#include "switch_splash.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

#include <malloc.h>

#include <switch.h>

#include "audio/switch_sound.h"
#include "core/switch_settings.h"
#include "render/switch_gl.h"
#include "render/switch_ui.h"

namespace {

// ---- timing ---------------------------------------------------------------
constexpr float kLetterStart = 0.15f;  // first logo letter
constexpr float kLetterStep  = 0.12f;  // between letters
constexpr float kLcdOnStart  = 0.80f;  // LCD "powers on"
constexpr float kLcdOnLen    = 0.30f;
constexpr float kAnimStart   = 1.30f;  // first LCD step
constexpr float kTick        = 0.25f;  // one LCD step (walk / juggle beat)
constexpr int   kJuggleSteps = 10;
constexpr int   kNumSteps    = 4 + kJuggleSteps;  // 3 walk + stop + juggling
constexpr float kHold        = 0.50f;
constexpr float kFadeLen     = 0.30f;
constexpr float kAnimEnd     = kAnimStart + kTick * kNumSteps;
constexpr float kTotal       = kAnimEnd + kHold + kFadeLen;

bool     s_active = false;
uint64_t s_start_tick = 0;
uint64_t s_last_tick = 0;

// ---- design space ---------------------------------------------------------
// Everything below is laid out on a 640x360 design canvas, scaled to fit
// the (logical) screen and centred; only the faceplate fills the whole
// screen.
constexpr float kDW = 640.0f, kDH = 360.0f;
float s_k = 2.0f, s_ox = 0.0f, s_oy = 0.0f;

struct Col { float r, g, b; };
constexpr Col c255(int r, int g, int b) { return Col{r / 255.0f, g / 255.0f, b / 255.0f}; }
constexpr Col kInk     = c255(34, 38, 32);
constexpr Col kLcdOn   = c255(197, 203, 172);
constexpr Col kLcdOff  = c255(150, 155, 130);
constexpr Col kBezel   = c255(46, 46, 50);
constexpr Col kBand    = c255(150, 30, 36);
constexpr Col kLogoRed = c255(196, 32, 40);
constexpr Col kLogoOut = c255(30, 30, 34);
constexpr Col kLogoSh  = c255(112, 112, 118);

Col mix(Col a, Col b, float t) { return Col{a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t}; }

float X(float x) { return s_ox + x * s_k; }
float Y(float y) { return s_oy + y * s_k; }

void rect(float x0, float y0, float x1, float y1, Col c, float a = 1.0f) {
    switch_ui_draw_rect(X(x0), Y(y0), (x1 - x0) * s_k, (y1 - y0) * s_k, c.r, c.g, c.b, a);
}

void quad(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy, Col c, float a) {
    const float p[8] = {X(ax), Y(ay), X(bx), Y(by), X(cx), Y(cy), X(dx), Y(dy)};
    switch_ui_draw_polygon(p, 4, c.r, c.g, c.b, a);
}

// Thick straight bar from A to B, `w` wide, ends extended by `ext`.
void bar(float ax, float ay, float bx, float by, float w, Col c, float a = 1.0f, float ext = 0.0f) {
    float dx = bx - ax, dy = by - ay;
    const float len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.0f) return;
    dx /= len; dy /= len;
    ax -= dx * ext; ay -= dy * ext; bx += dx * ext; by += dy * ext;
    const float nx = -dy * w * 0.5f, ny = dx * w * 0.5f;
    quad(ax + nx, ay + ny, bx + nx, by + ny, bx - nx, by - ny, ax - nx, ay - ny, c, a);
}

void ellipse(float cx, float cy, float rx, float ry, Col c, float a = 1.0f) {
    constexpr int kN = 24;
    float p[kN * 2];
    for (int i = 0; i < kN; ++i) {
        const float t = (float)i / kN * 6.2831853f;
        p[i * 2] = X(cx + std::cos(t) * rx);
        p[i * 2 + 1] = Y(cy + std::sin(t) * ry);
    }
    switch_ui_draw_polygon(p, kN, c.r, c.g, c.b, a);
}

// Elliptical ring: outer radii rx/ry, inner = outer - th.
void ring(float cx, float cy, float rx, float ry, float th, Col c, float a = 1.0f) {
    constexpr int kN = 32;
    for (int i = 0; i < kN; ++i) {
        const float t0 = (float)i / kN * 6.2831853f, t1 = (float)(i + 1) / kN * 6.2831853f;
        const float c0 = std::cos(t0), s0 = std::sin(t0), c1 = std::cos(t1), s1 = std::sin(t1);
        quad(cx + c0 * rx, cy + s0 * ry, cx + c1 * rx, cy + s1 * ry,
             cx + c1 * (rx - th), cy + s1 * (ry - th), cx + c0 * (rx - th), cy + s0 * (ry - th), c, a);
    }
}

void rounded_rect(float x0, float y0, float x1, float y1, float r, Col c, float a = 1.0f) {
    constexpr int kSeg = 6;
    float p[(kSeg + 1) * 4 * 2];
    int n = 0;
    const float cxs[4] = {x1 - r, x0 + r, x0 + r, x1 - r};
    const float cys[4] = {y1 - r, y1 - r, y0 + r, y0 + r};
    for (int corner = 0; corner < 4; ++corner) {
        for (int i = 0; i <= kSeg; ++i) {
            const float t = (corner + (float)i / kSeg) * 1.5707963f;
            p[n++] = X(cxs[corner] + std::cos(t) * r);
            p[n++] = Y(cys[corner] + std::sin(t) * r);
        }
    }
    switch_ui_draw_polygon(p, n / 2, c.r, c.g, c.b, a);
}

// ---- faceplate ------------------------------------------------------------
constexpr float LX0 = 160, LY0 = 104, LX1 = 480, LY1 = 300;  // LCD window
float s_row_shade[360];

void init_faceplate() {
    uint32_t seed = 12345u;
    for (int y = 0; y < 360; ++y) {
        seed = seed * 1664525u + 1013904223u;
        const float noise = ((float)(seed >> 8) / 16777216.0f - 0.5f) * 20.0f;
        s_row_shade[y] = (200.0f + 25.0f * std::cos((float)y / kDH * 3.14159f * 0.9f) + noise) / 255.0f;
    }
}

void draw_faceplate(float lw, float lh) {
    // Brushed metal, one strip per design row, stretched over the full screen.
    const float strip = lh / kDH;
    for (int y = 0; y < 360; ++y) {
        const float v = std::min(1.0f, s_row_shade[y]);
        switch_ui_draw_rect(0.0f, y * strip, lw, strip + 1.0f, v, v, std::min(1.0f, v * 1.02f), 1.0f);
    }
    // Coloured band along the bottom.
    const float band_y = Y(kDH - 46.0f);
    switch_ui_draw_rect(0.0f, band_y, lw, lh - band_y, kBand.r, kBand.g, kBand.b, 1.0f);
    // Dark bezel around the LCD.
    rounded_rect(LX0 - 16, LY0 - 14, LX1 + 16, LY1 + 14, 10, kBezel);
}

// ---- logo -----------------------------------------------------------------
// Chunky hand-built letters (no font needed): Y O K O I. Each letter is
// drawn three times - drop shadow, dark outline (shapes grown by `g`), red
// fill - like lettering printed on a handheld's faceplate.
constexpr float kLogoH = 56.0f, kLogoTop = 24.0f, kLogoGap = 0.10f;
constexpr float kLetterW[5] = {0.80f, 0.84f, 0.80f, 0.84f, 0.24f};

void draw_letter(int i, float lx, float g, Col c) {
    const float h = kLogoH, t = kLogoTop;
    auto U = [&](float u) { return lx + u * h; };
    auto V = [&](float v) { return t + v * h; };
    const float th = 0.24f * h;
    switch (i) {
        case 0:  // Y
            bar(U(0.12f), V(0.0f), U(0.40f), V(0.52f), th + 2 * g, c, 1.0f, g);
            bar(U(0.68f), V(0.0f), U(0.40f), V(0.52f), th + 2 * g, c, 1.0f, g);
            rect(U(0.28f) - g, V(0.45f) - g, U(0.52f) + g, V(1.0f) + g, c);
            break;
        case 1: case 3:  // O
            ring(U(0.42f), V(0.5f), 0.42f * h + g, 0.5f * h + g, th + 2 * g, c);
            break;
        case 2:  // K
            rect(U(0.0f) - g, V(0.0f) - g, U(0.24f) + g, V(1.0f) + g, c);
            bar(U(0.70f), V(0.08f), U(0.20f), V(0.56f), th + 2 * g, c, 1.0f, g);
            bar(U(0.36f), V(0.44f), U(0.72f), V(0.92f), th + 2 * g, c, 1.0f, g);
            break;
        case 4:  // I
            rect(U(0.0f) - g, V(0.0f) - g, U(0.24f) + g, V(1.0f) + g, c);
            break;
    }
}

void draw_logo(int letters) {
    float total = 0.0f;
    for (int i = 0; i < 5; ++i) total += kLetterW[i] + (i ? kLogoGap : 0.0f);
    float xs[5];
    float x = (kDW - total * kLogoH) * 0.5f;
    for (int i = 0; i < 5; ++i) { xs[i] = x; x += (kLetterW[i] + kLogoGap) * kLogoH; }
    const float g = 3.0f;
    for (int i = 0; i < letters; ++i) {
        s_ox += 3.0f * s_k; s_oy += 3.0f * s_k;
        draw_letter(i, xs[i], g, kLogoSh);
        s_ox -= 3.0f * s_k; s_oy -= 3.0f * s_k;
    }
    for (int i = 0; i < letters; ++i) draw_letter(i, xs[i], g, kLogoOut);
    for (int i = 0; i < letters; ++i) draw_letter(i, xs[i], 0.0f, kLogoRed);
}

// ---- LCD character --------------------------------------------------------
constexpr float kS = 0.85f, kGround = LY1 - 12.0f, kCX = (LX0 + LX1) * 0.5f;
enum Legs { LegsStand, LegsA, LegsB };
enum Arms { ArmsSwingA, ArmsSwingB, ArmsDown, ArmsJL, ArmsJM, ArmsJR };

void figure(float cx, Legs legs, Arms arms, Col bg, float a) {
    if (a <= 0.003f) return;
    auto PX = [&](float x) { return cx + x * kS; };
    auto PY = [&](float y) { return kGround + y * kS; };
    // head, cap + brim
    ellipse(PX(0), PY(-96), 14 * kS, 14 * kS, kInk, a);
    rect(PX(-15), PY(-114), PX(15), PY(-104), kInk, a);
    rect(PX(8), PY(-108), PX(24), PY(-104), kInk, a);
    // body
    rounded_rect(PX(-13), PY(-80), PX(13), PY(-40), 4 * kS, kInk, a);
    // legs + feet
    static const float kLegs[3][2][2] = {{{-7, 0}, {7, 0}}, {{-16, -2}, {12, 0}}, {{-10, 0}, {16, -2}}};
    for (int s = 0; s < 2; ++s) {
        const float fx = kLegs[legs][s][0], lift = kLegs[legs][s][1], hx = s ? 6.0f : -6.0f;
        bar(PX(hx), PY(-42), PX(fx), PY(-4 + lift), 7 * kS, kInk, a);
        rect(PX(fx - 5), PY(-6 + lift), PX(fx + 6), PY(lift), kInk, a);
    }
    // arms + hands
    static const float kArms[6][2][2] = {
        {{-20, -56}, {20, -44}}, {{-20, -44}, {20, -56}}, {{-20, -48}, {20, -48}},
        {{-30, -96}, {26, -62}}, {{-28, -72}, {28, -72}}, {{-26, -62}, {30, -96}},
    };
    for (int s = 0; s < 2; ++s) {
        const float ax = kArms[arms][s][0], ay = kArms[arms][s][1], sx = s ? 12.0f : -12.0f;
        bar(PX(sx), PY(-74), PX(ax), PY(ay), 6 * kS, kInk, a);
        ellipse(PX(ax), PY(ay), 5 * kS, 5 * kS, kInk, a);
    }
    // eyes: holes in the head, in the LCD's own colour
    ellipse(PX(-4), PY(-96), 2 * kS, 2 * kS, bg, a);
    ellipse(PX(5), PY(-96), 2 * kS, 2 * kS, bg, a);
}

constexpr float kBalls[5][2] = {{-50, -110}, {-38, -150}, {0, -170}, {38, -150}, {50, -110}};
void ball(int i, float a) {
    if (a <= 0.003f) return;
    ellipse(kCX + kBalls[i][0] * kS, kGround + kBalls[i][1] * kS, 7 * kS, 7 * kS, kInk, a);
}

// LCD "segments": every figure/ball position the screen can show. Each has
// a level that snaps to 1 when lit and decays when not (ghosting trail).
constexpr float kWalkX[3] = {kCX - 126.0f, kCX - 84.0f, kCX - 42.0f};
enum { kSegWalk = 0, kSegCenter = 6, kSegBall = 10, kNumSegs = 15 };
float s_level[kNumSegs];

void lit_segments(int step, bool lit[kNumSegs]) {
    std::fill(lit, lit + kNumSegs, false);
    if (step < 0) return;
    if (step < 3) { lit[kSegWalk + step * 2 + (step & 1)] = true; return; }
    if (step == 3) { lit[kSegCenter + 0] = true; return; }
    const int k = std::min(step, kNumSteps - 1) - 4;
    static const int kJ[4] = {1, 2, 3, 2};  // jL jM jR jM
    lit[kSegCenter + kJ[k % 4]] = true;
    for (int j = 0; j < 3; ++j) lit[kSegBall + (k + j * 2) % 5] = true;
}

void draw_segment(int s, Col bg, float a) {
    if (s < kSegCenter) {
        const int pos = (s - kSegWalk) / 2, pose = (s - kSegWalk) % 2;
        figure(kWalkX[pos], pose ? LegsB : LegsA, pose ? ArmsSwingB : ArmsSwingA, bg, a);
    } else if (s < kSegBall) {
        static const Arms kA[4] = {ArmsDown, ArmsJL, ArmsJM, ArmsJR};
        figure(kCX, LegsStand, kA[s - kSegCenter], bg, a);
    } else {
        ball(s - kSegBall, a);
    }
}

void draw_lcd(float t, float dt) {
    const float on = std::clamp((t - kLcdOnStart) / kLcdOnLen, 0.0f, 1.0f);
    const Col bg = mix(kLcdOff, kLcdOn, on);
    rect(LX0, LY0, LX1, LY1, bg);
    // faint pixel grid
    for (float x = LX0; x < LX1; x += 3.0f) rect(x, LY0, x + 0.5f, LY1, Col{0, 0, 0}, 0.05f);
    for (float y = LY0; y < LY1; y += 3.0f) rect(LX0, y, LX1, y + 0.5f, Col{0, 0, 0}, 0.05f);

    const int step = t >= kAnimStart ? (int)((t - kAnimStart) / kTick) : -1;
    bool lit[kNumSegs];
    lit_segments(step, lit);
    const float decay = std::pow(0.5f, dt * 25.0f);
    for (int s = 0; s < kNumSegs; ++s) {
        s_level[s] = lit[s] ? 1.0f : s_level[s] * decay;
    }
    // unlit segments, faintly visible once the LCD is on
    for (int s = 0; s < kNumSegs; ++s) draw_segment(s, bg, 0.07f * on);
    for (int s = 0; s < kNumSegs; ++s) {
        if (s_level[s] > 0.07f) draw_segment(s, bg, s_level[s]);
    }
}

// ---- sound ----------------------------------------------------------------
// Piezo-style beeps (square waves, like an LCD handheld's buzzer), timed to
// the animation above: a rising blip per logo letter, a low tick per walking
// step, a high/low beep per juggling beat. The whole clip is synthesized
// once at start into its own mempool and played as a single wavebuf on its
// own mono voice of g_sound's shared driver: 0 is the game tone synth, 1-8
// lcdonald_sound.cpp's stereo voices, 9-11 step_sound.cpp, 12-13
// menu_sound.cpp, 14 this one (switch_sound.cpp's num_voices has room).
// Plays at Settings > Sound > Volume, not at all while Mute is on.
constexpr int kSplashVoiceId = 14;
constexpr int kAudioRate = 48000;

void*   s_pool = nullptr;
size_t  s_pool_size = 0;
int     s_pool_id = -1;
bool    s_voice_on = false;
uint32_t s_voice_gen = 0;
AudioDriverWaveBuf s_wavebuf{};

void add_beep(int16_t* buf, size_t n, float t0, float freq, float dur, float vol) {
    const size_t i0 = (size_t)(t0 * kAudioRate);
    const size_t len = (size_t)(dur * kAudioRate);
    for (size_t i = 0; i < len && i0 + i < n; ++i) {
        const float phase = std::fmod((float)i * freq / kAudioRate, 1.0f);
        float env = 1.0f;
        if (i < 30) env = (float)i / 30.0f;                      // no clicks:
        if (i + 60 > len) env = std::min(env, (float)(len - i) / 60.0f);  // short ramps
        const int v = (int)buf[i0 + i] + (int)((phase < 0.5f ? 1.0f : -1.0f) * vol * env * 26000.0f);
        buf[i0 + i] = (int16_t)std::clamp(v, -32767, 32767);
    }
}

void splash_audio_start() {
    AudioDriver* drv = g_sound.driver();
    const float vol = switch_sound_volume();
    if (!drv || vol <= 0.0f) return;
    const size_t n = (size_t)(kTotal * kAudioRate);
    if (!s_pool) {
        s_pool_size = (n * sizeof(int16_t) + 0xFFF) & ~size_t(0xFFF);
        s_pool = memalign(0x1000, s_pool_size);
        if (!s_pool) return;
        int16_t* pcm = (int16_t*)s_pool;
        memset(pcm, 0, s_pool_size);
        for (int i = 0; i < 5; ++i)
            add_beep(pcm, n, kLetterStart + i * kLetterStep, 2000.0f + 300.0f * i, 0.05f, 0.25f);
        for (int k = 0; k < 4; ++k)
            add_beep(pcm, n, kAnimStart + k * kTick, 1200.0f, 0.025f, 0.18f);
        for (int k = 0; k < kJuggleSteps; ++k)
            add_beep(pcm, n, kAnimStart + (4 + k) * kTick, (k % 2) ? 2600.0f : 3200.0f, 0.04f, 0.20f);
        armDCacheFlush(s_pool, s_pool_size);
    }
    s_pool_id = audrvMemPoolAdd(drv, s_pool, s_pool_size);
    if (s_pool_id < 0) return;
    audrvMemPoolAttach(drv, s_pool_id);
    if (!audrvVoiceInit(drv, kSplashVoiceId, 1, PcmFormat_Int16, kAudioRate)) {
        audrvMemPoolDetach(drv, s_pool_id);
        audrvMemPoolRemove(drv, s_pool_id);
        s_pool_id = -1;
        audrvUpdate(drv);
        return;
    }
    audrvVoiceSetDestinationMix(drv, kSplashVoiceId, AUDREN_FINAL_MIX_ID);
    audrvVoiceSetMixFactor(drv, kSplashVoiceId, vol, 0, 0);
    audrvVoiceSetMixFactor(drv, kSplashVoiceId, vol, 0, 1);
    // Caller-owned fields only (see lcdonald_sound.cpp's play_cached()).
    s_wavebuf.data_pcm16 = (int16_t*)s_pool;
    s_wavebuf.size = (uint32_t)(n * sizeof(int16_t));
    s_wavebuf.start_sample_offset = 0;
    s_wavebuf.end_sample_offset = (int32_t)n;
    s_wavebuf.is_looping = false;
    audrvVoiceAddWaveBuf(drv, kSplashVoiceId, &s_wavebuf);
    audrvVoiceStart(drv, kSplashVoiceId);
    audrvUpdate(drv);
    s_voice_on = true;
    s_voice_gen = g_sound.reinit_generation();
}

// Stops (on skip) or retires (after it finished) the splash voice and gives
// its channel/mempool slots back. The pool memory itself is kept - ~0.5 MB,
// and never freeing it means the DSP can't possibly read freed memory.
void splash_audio_stop() {
    if (!s_voice_on) return;
    s_voice_on = false;
    AudioDriver* drv = g_sound.driver();
    // A full_reinit() in between already threw this voice/pool away with
    // the old driver - nothing left to tear down on the new one.
    if (!drv || g_sound.reinit_generation() != s_voice_gen) return;
    audrvVoiceStop(drv, kSplashVoiceId);
    audrvUpdate(drv);
    audrvVoiceDrop(drv, kSplashVoiceId);
    if (s_pool_id >= 0) {
        audrvMemPoolDetach(drv, s_pool_id);
        audrvMemPoolRemove(drv, s_pool_id);
        s_pool_id = -1;
    }
    audrvUpdate(drv);
}

}  // namespace

void switch_splash_start() {
    s_active = true;
    s_start_tick = s_last_tick = armGetSystemTick();
    std::fill(s_level, s_level + kNumSegs, 0.0f);
    init_faceplate();
    splash_audio_start();
}

bool switch_splash_active() { return s_active; }

void switch_splash_frame(uint64_t kDown, int viewport_w, int viewport_h) {
    if (!s_active) return;
    const uint64_t now = armGetSystemTick();
    const float t = (float)armTicksToNs(now - s_start_tick) / 1e9f;
    const float dt = std::min(0.1f, (float)armTicksToNs(now - s_last_tick) / 1e9f);
    s_last_tick = now;
    if ((kDown & HidNpadButton_Plus) || t >= kTotal) {
        s_active = false;
        splash_audio_stop();
    }

    int lw = viewport_w, lh = viewport_h;
    if (g_menu_rotation != ScreenRotation::Deg0) std::swap(lw, lh);
    switch_ui_set_rotation((int)g_menu_rotation);
    s_k = std::min(lw / kDW, lh / kDH);
    s_ox = (lw - kDW * s_k) * 0.5f;
    s_oy = (lh - kDH * s_k) * 0.5f;

    glViewport(0, 0, viewport_w, viewport_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    switch_ui_begin_frame(lw, lh);

    draw_faceplate((float)lw, (float)lh);
    const int letters = t < kLetterStart ? 0 : std::min(5, (int)((t - kLetterStart) / kLetterStep) + 1);
    draw_logo(letters);
    draw_lcd(t, dt);

    // fade to black at the end, so the cut to the game list isn't abrupt
    const float fade = std::clamp((t - (kTotal - kFadeLen)) / kFadeLen, 0.0f, 1.0f);
    if (fade > 0.0f) switch_ui_draw_rect(0.0f, 0.0f, (float)lw, (float)lh, 0.0f, 0.0f, 0.0f, fade);
}
