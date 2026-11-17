#include "switch_transition.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include <switch.h>

#include "core/switch_settings.h"
#include "render/switch_gl.h"
#include "render/switch_ui.h"

namespace {

constexpr int kW = 1280, kH = 720;
constexpr int kBufs = 4;  // 3 frames (current, last, old) + 1 scratch for Pixelate
GLuint g_fbo[kBufs] = {}, g_tex[kBufs] = {};
bool g_ready = false, g_failed = false;

int g_cur = -1;   // being drawn this frame
int g_last = -1;  // last frame shown (pure, no transition on it)
int g_old = -1;   // frozen "from" frame of the running transition
uint64_t g_last_key = 0;
bool g_have_last = false;

bool g_running = false;
uint8_t g_style = kTrSlide;
int g_dir = 1;
float g_t0 = 0.0f;
float g_dur = 0.28f;

bool make_buffers() {
    if (g_ready) return true;
    if (g_failed) return false;
    glGenFramebuffers(kBufs, g_fbo);
    glGenTextures(kBufs, g_tex);
    for (int i = 0; i < kBufs; ++i) {
        glBindTexture(GL_TEXTURE_2D, g_tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kW, kH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindFramebuffer(GL_FRAMEBUFFER, g_fbo[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_tex[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) g_failed = true;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (g_failed) {
        glDeleteFramebuffers(kBufs, g_fbo);
        glDeleteTextures(kBufs, g_tex);
        return false;
    }
    g_ready = true;
    return true;
}

float now_s() { return (float)armTicksToNs(armGetSystemTick()) / 1e9f; }
float smooth(float t) { return t * t * (3.0f - 2.0f * t); }
float back_out(float t) {
    const float c1 = 1.70158f, c3 = c1 + 1.0f;
    return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

// Part (sx,sy,sw,sh - logical px of the 1280x720 frame) of buffer `i`, drawn at (x,y,w,h).
void blit(int i, float x, float y, float w, float h, float sx, float sy, float sw, float sh, float alpha = 1.0f,
          bool nearest = false) {
    if (i < 0 || w <= 0.5f || h <= 0.5f) return;
    switch_ui_draw_texture_uv(g_tex[i], x, y, w, h, sx / kW, sy / kH, (sx + sw) / kW, (sy + sh) / kH, alpha, nearest);
}

void composite(float t, float y0, float y1, const float bg[3], const float hl[3]) {
    const float W = (float)kW, H = y1 - y0;
    const float e = smooth(t);
    const int o = g_old, n = g_cur;
    switch_ui_draw_rect(0.0f, y0, W, H, bg[0], bg[1], bg[2], 1.0f);
    // 1:1 copies are sampled unfiltered: with filtering, the row just
    // outside the content area (the top bar's separator line) bleeds into
    // the edge and slides along with the picture as a faint ghost bar.
    // Scaled ones (Zoom, Flip, Cube) sample half a pixel inside instead.
    const float in = 0.5f;
    auto full = [&](int i, float a = 1.0f) { blit(i, 0.0f, y0, W, H, 0.0f, y0, W, H, a, true); };
    auto scaled = [&](int i, float x, float y, float w, float h, float a = 1.0f) {
        blit(i, x, y, w, h, 0.0f, y0 + in, W, H - 2.0f * in, a);
    };
    switch (g_style) {
        case kTrSlide:
        case kTrBounce: {
            const float p = g_style == kTrBounce ? back_out(t) : e;
            blit(o, std::round(-g_dir * W * p), y0, W, H, 0.0f, y0, W, H, 1.0f, true);
            blit(n, std::round(g_dir * W * (1.0f - p)), y0, W, H, 0.0f, y0, W, H, 1.0f, true);
            break;
        }
        case kTrFade:
            full(o);
            full(n, e);
            break;
        case kTrZoom: {
            const float s = 1.0f - 0.15f * std::sin(3.14159f * t);
            const float a = 0.25f + 0.75f * std::fabs(1.0f - 2.0f * t);
            const float w = W * s, h = H * s;
            scaled(t < 0.5f ? o : n, (W - w) * 0.5f, y0 + (H - h) * 0.5f, w, h, a);
            break;
        }
        case kTrFlip: {
            const float w = W * std::fabs(std::cos(3.14159f * e));
            scaled(e < 0.5f ? o : n, (W - w) * 0.5f, y0, w, H);
            break;
        }
        case kTrCube: {
            const float wa = W * (1.0f - e), wb = W * e;
            if (g_dir > 0) {
                scaled(o, 0.0f, y0, wa, H);
                scaled(n, wa, y0, wb, H);
            } else {
                scaled(n, 0.0f, y0, wb, H);
                scaled(o, wb, y0, wa, H);
            }
            break;
        }
        case kTrWipe: {
            full(o);
            const float xw = W * e;
            const float x = g_dir > 0 ? W - xw : 0.0f;  // from the side you're heading to
            blit(n, std::round(x), y0, std::round(xw), H, std::round(x), y0, std::round(xw), H, 1.0f, true);
            if (xw > 1.0f && xw < W - 1.0f) {
                switch_ui_draw_rect(g_dir > 0 ? x - 2.0f : xw - 2.0f, y0, 4.0f, H, hl[0], hl[1], hl[2], 1.0f);
            }
            break;
        }
        case kTrBlinds: {
            full(o);
            constexpr int kStrips = 8;
            const float sh = H / kStrips;
            for (int k = 0; k < kStrips; ++k) {
                const float h = sh * std::clamp(e * 1.6f - k * 0.08f, 0.0f, 1.0f);
                const float sy = y0 + k * sh;
                blit(n, 0.0f, std::round(sy), W, std::round(h), 0.0f, std::round(sy), W, std::round(h), 1.0f, true);
            }
            break;
        }
        case kTrPixelate: {
            const float p = 1.0f - std::fabs(1.0f - 2.0f * t);
            const float blk = 1.0f + p * 30.0f;
            const float sw = std::max(1.0f, std::floor(W / blk)), sh = std::max(1.0f, std::floor(H / blk));
            // Shrink into the scratch buffer, then blow it back up unfiltered.
            glBindFramebuffer(GL_FRAMEBUFFER, g_fbo[3]);
            blit(t < 0.5f ? o : n, 0.0f, 0.0f, sw, sh, 0.0f, y0 + in, W, H - 2.0f * in);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            blit(3, 0.0f, y0, W, H, 0.0f, 0.0f, sw, sh, 1.0f, /*nearest=*/true);
            break;
        }
        case kTrLcd: {
            const float lcd[3] = {197 / 255.0f, 203 / 255.0f, 172 / 255.0f};
            if (t < 0.5f) {
                full(o);
                switch_ui_draw_rect(0.0f, y0, W, H, lcd[0], lcd[1], lcd[2], std::min(1.0f, t * 2.2f));
            } else {
                const float k = (t - 0.5f) * 2.0f;
                switch_ui_draw_rect(0.0f, y0, W, H, lcd[0], lcd[1], lcd[2], 1.0f);
                full(n, std::min(1.0f, k * 1.4f));
                if (((int)(k * 8.0f)) % 3 == 0) switch_ui_draw_rect(0.0f, y0, W, H, lcd[0], lcd[1], lcd[2], 0.4f);
            }
            break;
        }
        default:
            full(n);
            break;
    }
}

}  // namespace

const char* transition_name(uint8_t t) {
    static const char* const kNames[kNumTransitions] = {"Off",    "Slide",  "Fade",     "Zoom",         "Flip",  "Cube",
                                                        "Wipe",   "Blinds", "Bounce",   "Pixelate",     "LCD Power-On",
                                                        "Random"};
    return kNames[t < kNumTransitions ? t : 0];
}

bool transition_capture_begin(int viewport_w, int viewport_h) {
    if (viewport_w != kW || viewport_h != kH || !make_buffers()) return false;
    g_cur = 0;
    while (g_cur == g_last || g_cur == g_old) g_cur++;  // one of 0..2 is free
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo[g_cur]);
    return true;
}

void transition_capture_end(int viewport_w, int viewport_h, uint64_t key, int dir, float y0, float y1, const float bg[3],
                            const float hl[3]) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    const float t_now = now_s();
    if (g_have_last && key != g_last_key && g_transition != kTrOff) {
        // Quick repeats (a direction held) get a shorter one.
        const bool quick = g_running && (t_now - g_t0) < g_dur;
        g_old = g_last;
        g_style = g_transition == kTrRandom ? (uint8_t)(kTrSlide + rand() % (kTrLcd - kTrSlide + 1)) : g_transition;
        g_dir = dir >= 0 ? 1 : -1;
        g_dur = quick ? 0.08f : 0.28f;  // held direction (key repeat): keep up with it
        g_t0 = t_now;
        g_running = true;
    }

    glViewport(0, 0, viewport_w, viewport_h);
    switch_ui_set_rotation(0);  // only used unrotated
    switch_ui_begin_frame(viewport_w, viewport_h);
    blit(g_cur, 0.0f, 0.0f, (float)kW, (float)kH, 0.0f, 0.0f, (float)kW, (float)kH);
    if (g_running) {
        const float t = (t_now - g_t0) / g_dur;
        if (t >= 1.0f) {
            g_running = false;
            g_old = -1;
        } else {
            composite(std::max(0.0f, t), y0, y1, bg, hl);
        }
    }
    g_last = g_cur;
    g_last_key = key;
    g_have_last = true;
}

void transition_reset() {
    g_have_last = false;
    g_running = false;
    g_old = -1;
    g_last = -1;
}
