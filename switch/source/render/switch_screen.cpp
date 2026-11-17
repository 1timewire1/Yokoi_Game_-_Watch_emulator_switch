#include "switch_screen.h"

#include <cstring>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "switch_gl.h"

// Declarations only - the implementation (STB_IMAGE_IMPLEMENTATION) is compiled
// once in third_party/stb_image.cpp.
#include "../third_party/stb_image.h"

#include "std/GW_ROM.h"
#include "std/debug_log.h"
#include "std/gw_pack.h"
#include "std/segment.h"
#include "std/settings.h"

#include "core/switch_runtime_state.h"
#include "core/switch_settings.h"
#include "BrickCores/GameCom.h"
#include "BrickCores/AdvVision.h"
#include "BrickCores/Tamago.h"

namespace {

// Same verified-correct UV convention as Android (see yokoi_jni.cpp):
// atlas is mirrored on X and needs a V flip for OpenGL's bottom-left origin.
constexpr bool kUvFlipX = true;
constexpr bool kUvFlipV = true;
constexpr bool kRenderConsoleOverlay = true;

GlResources g_gl;
uint32_t g_loaded_texture_generation = 0xFFFFFFFFu;

// LCD ghosting (g_ghosting): each segment's displayed level, eased toward
// its on/off state once per rendered frame. Turning off is the slow side
// (a real LCD's segment lingers); turning on is quicker. Partly-lit
// segments are drawn in kGhostBins alpha steps (RenderVertex has no colour,
// so each step is its own batch).
std::vector<float> g_seg_level;
constexpr int kGhostBins = 6;
constexpr float kGhostFall[4] = {1.0f, 0.34f, 0.17f, 0.085f};  // level lost per frame when off
constexpr float kGhostRise[4] = {1.0f, 1.0f, 0.5f, 0.34f};     // level gained per frame when on


std::string t3x_path_to_png_asset(const std::string& p) {
    if (p.empty()) return std::string();
    std::string base = p;
    size_t slash = base.find_last_of('/');
    if (slash != std::string::npos) {
        base = base.substr(slash + 1);
    }
    const std::string t3x = ".t3x";
    if (base.size() >= t3x.size() && base.compare(base.size() - t3x.size(), t3x.size(), t3x) == 0) {
        base.replace(base.size() - t3x.size(), t3x.size(), ".png");
    }
    return base;
}

GLuint upload_png_from_pack(const std::string& asset_name, int& out_w, int& out_h) {
    out_w = out_h = 0;
    if (asset_name.empty()) {
        return 0;
    }

    const uint8_t* data = nullptr;
    size_t size = 0;
    if (!gw_pack::get_file_bytes(asset_name, data, size) || !data || size == 0) {
        YOKOI_LOG("switch_screen: pack entry not found: %s", asset_name.c_str());
        return 0;
    }
    YOKOI_LOG("switch_screen: pack entry '%s' = %u bytes", asset_name.c_str(), (unsigned)size);

    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load_from_memory(data, (int)size, &w, &h, &channels, 4);
    if (!pixels) {
        YOKOI_LOG("switch_screen: PNG decode failed for %s: %s", asset_name.c_str(), stbi_failure_reason());
        return 0;
    }
    YOKOI_LOG("switch_screen: decoded %s -> %dx%d (src channels=%d)", asset_name.c_str(), w, h, channels);

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    const GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        YOKOI_LOG("switch_screen: glTexImage2D error 0x%04x for %s (tex=%u)", (unsigned)err, asset_name.c_str(), (unsigned)tex);
    }

    stbi_image_free(pixels);

    out_w = w;
    out_h = h;
    return tex;
}

void append_quad_ndc_uv_canvas(std::vector<RenderVertex>& out,
                                float canvas_w, float canvas_h,
                                float x, float y, float w, float h,
                                float u0, float v0, float u1, float v1) {
    if (canvas_w <= 0.0f) canvas_w = 1.0f;
    if (canvas_h <= 0.0f) canvas_h = 1.0f;

    float left = (x / canvas_w) * 2.0f - 1.0f;
    float right = ((x + w) / canvas_w) * 2.0f - 1.0f;
    float top = 1.0f - (y / canvas_h) * 2.0f;
    float bottom = 1.0f - ((y + h) / canvas_h) * 2.0f;

    out.push_back({left, bottom, u0, v0});
    out.push_back({right, bottom, u1, v0});
    out.push_back({right, top, u1, v1});

    out.push_back({right, top, u1, v1});
    out.push_back({left, top, u0, v1});
    out.push_back({left, bottom, u0, v0});
}

void calc_uv_rect(float texW, float texH,
                   float u, float v, float w, float h,
                   float& out_u0, float& out_v0, float& out_u1, float& out_v1) {
    if (texW <= 0.0f) texW = 1.0f;
    if (texH <= 0.0f) texH = 1.0f;

    out_u0 = u / texW;
    out_u1 = (u + w) / texW;
    if (kUvFlipX) {
        std::swap(out_u0, out_u1);
    }

    if (kUvFlipV) {
        out_v0 = 1.0f - (v / texH);
        out_v1 = 1.0f - ((v + h) / texH);
    } else {
        out_v0 = v / texH;
        out_v1 = (v + h) / texH;
    }
}

// g_split_two_screens_to_panels marks "clamshell" games (two screens, not
// double_in_one_screen) that Android stacks as top/bottom panels with no
// shared offset math; other games (single screen, or two screens side-by-side)
// use g_screen_off_x/y computed relative to a single combined canvas. Even
// though Switch always renders "combined", it still needs both branches -
// rebuild_layout_from_game() zeroes g_screen_off_x/y for the clamshell case
// precisely because that positioning is meant to come from here instead.
void get_screen_base_global(uint8_t screen, float& outX, float& outY) {
    if (g_split_two_screens_to_panels) {
        if (screen == 0) {
            outX = g_top_off_x;
            outY = 0.0f;
        } else {
            outX = g_bottom_off_x;
            outY = g_bottom_off_y;
        }
    } else {
        outX = g_top_off_x + g_screen_off_x[screen];
        outY = g_screen_off_y[screen];
    }
}

void draw_vertices(GLuint tex, const std::vector<RenderVertex>& verts) {
    if (tex == 0 || verts.empty()) return;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(g_gl.uTex, 0);
    glBindBuffer(GL_ARRAY_BUFFER, g_gl.vbo);
    const size_t bytes = verts.size() * sizeof(RenderVertex);
    if (bytes > g_gl.vbo_capacity_bytes) {
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)bytes, verts.data(), GL_DYNAMIC_DRAW);
        g_gl.vbo_capacity_bytes = bytes;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)bytes, verts.data());
    }
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
}

} // namespace

namespace {
// Screen filters (g_screen_filter / g_filter_strength), drawn over each game
// screen rect after everything but the console art. Plain blending only (no
// shader change): line/grid patterns are thin dark quads, tints a multiply
// blend, the vignette a few nested translucent frames.
enum ScreenFilter : uint8_t {
    kFilterNone, kFilterScanlines, kFilterLcdGrid, kFilterDotMatrix, kFilterCrt, kFilterVignette,
    kFilterGreenPhosphor, kFilterAmber, kFilterSepia, kFilterCool, kFilterWarm, kFilterNight,
    kFilterVirtualBoy, kFilterMagnifier, kFilterCount
};

// The game picture's NDC transform this frame (uScale / uOffset, no
// rotation) - for the Magnifier filter, which re-reads what was drawn.
float s_lens_scale[2] = {1.0f, 1.0f}, s_lens_off[2] = {0.0f, 0.0f};
bool s_lens_ok = false;  // not rotated

void draw_screen_filter(float contentW, float contentH, float px_per_unit) {
    const int f = g_screen_filter;
    if (f <= kFilterNone || f >= kFilterCount || g_gl.tex_white == 0) return;
    const float strength = 0.6f + 0.4f * (float)std::min<uint8_t>(g_filter_strength, 2);  // 0.6 / 0.8 / 1.0
    const float unit = 1.0f / std::max(0.01f, px_per_unit);  // one screen pixel, in canvas units

    // Each game screen's rect, in canvas units.
    struct Rect { float x, y, w, h; };
    std::vector<Rect> rects;
    const uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
    for (uint8_t s = 0; s < g_nb_screen; s++) {
        float gx = 0.0f, gy = 0.0f;
        get_screen_base_global(s, gx, gy);
        rects.push_back({gx, gy, (float)g_segment_info[4 + s * 2] / (float)scale, (float)g_segment_info[5 + s * 2] / (float)scale});
    }

    if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
    if (g_gl.uMultiplyDarken >= 0) glUniform1f(g_gl.uMultiplyDarken, 0.0f);

    auto lines = [&](float spacing_px, float thick_px, bool horizontal, bool vertical, float alpha) {
        std::vector<RenderVertex> v;
        const float step = spacing_px * unit, t = thick_px * unit;
        for (const Rect& r : rects) {
            if (horizontal)
                for (float y = r.y; y < r.y + r.h; y += step)
                    append_quad_ndc_uv_canvas(v, contentW, contentH, r.x, y, r.w, std::min(t, r.y + r.h - y), 0, 0, 1, 1);
            if (vertical)
                for (float x = r.x; x < r.x + r.w; x += step)
                    append_quad_ndc_uv_canvas(v, contentW, contentH, x, r.y, std::min(t, r.x + r.w - x), r.h, 0, 0, 1, 1);
        }
        glUniform4f(g_gl.uMul, 0.0f, 0.0f, 0.0f, alpha * strength);
        draw_vertices(g_gl.tex_white, v);
    };
    auto tint = [&](uint32_t rgb) {  // multiply, faded toward white by strength
        std::vector<RenderVertex> v;
        for (const Rect& r : rects) append_quad_ndc_uv_canvas(v, contentW, contentH, r.x, r.y, r.w, r.h, 0, 0, 1, 1);
        const float k = 0.45f + 0.55f * strength;
        auto ch = [&](int sh) { return 1.0f - k * (1.0f - ((rgb >> sh) & 0xFF) / 255.0f); };
        glUniform4f(g_gl.uMul, ch(16), ch(8), ch(0), 1.0f);
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        draw_vertices(g_gl.tex_white, v);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    };
    auto vignette = [&](float alpha) {
        const int rings = 10;
        for (const Rect& r : rects) {
            const float edge = std::min(r.w, r.h) * 0.18f;
            for (int i = 0; i < rings; i++) {
                const float in = edge * (float)i / rings, th = edge / rings;
                std::vector<RenderVertex> v;
                append_quad_ndc_uv_canvas(v, contentW, contentH, r.x + in, r.y + in, r.w - 2 * in, th, 0, 0, 1, 1);
                append_quad_ndc_uv_canvas(v, contentW, contentH, r.x + in, r.y + r.h - in - th, r.w - 2 * in, th, 0, 0, 1, 1);
                append_quad_ndc_uv_canvas(v, contentW, contentH, r.x + in, r.y + in + th, th, r.h - 2 * in - 2 * th, 0, 0, 1, 1);
                append_quad_ndc_uv_canvas(v, contentW, contentH, r.x + r.w - in - th, r.y + in + th, th, r.h - 2 * in - 2 * th, 0, 0, 1, 1);
                const float a = alpha * strength * (1.0f - (float)i / rings) * (1.0f - (float)i / rings);
                glUniform4f(g_gl.uMul, 0.0f, 0.0f, 0.0f, a);
                draw_vertices(g_gl.tex_white, v);
            }
        }
    };

    switch (f) {
        case kFilterScanlines:     lines(3.0f, 1.0f, true, false, 0.35f); break;
        case kFilterLcdGrid:       lines(3.0f, 1.0f, true, true, 0.18f); break;
        case kFilterDotMatrix:     tint(0x9BBC0F); lines(4.0f, 1.0f, true, true, 0.22f); break;
        case kFilterCrt:           lines(3.0f, 1.0f, true, false, 0.3f); vignette(0.55f); break;
        case kFilterVignette:      vignette(0.6f); break;
        case kFilterGreenPhosphor: tint(0x66FF66); lines(3.0f, 1.0f, true, false, 0.2f); break;
        case kFilterAmber:         tint(0xFFB000); lines(3.0f, 1.0f, true, false, 0.2f); break;
        case kFilterSepia:         tint(0xE0C090); break;
        case kFilterCool:          tint(0xC8DCFF); break;
        case kFilterWarm:          tint(0xFFE0B8); break;
        case kFilterNight:         tint(0x8A96C8); break;
        case kFilterVirtualBoy:    tint(0xFF2020); break;
        // Magnifier: as through a lens (the R-Zone XPG's) - the edges darken;
        // the soft glow round lit segments is drawn with them.
        case kFilterMagnifier: {
            // 1) Fisheye: what's been drawn so far is copied out of the
            // framebuffer and drawn back over each screen on a bent grid -
            // the middle swollen, the edges where they were - a little soft.
            if (s_lens_ok && g_gl.viewport_w > 0 && g_gl.viewport_h > 0) {
                static GLuint s_fb = 0;
                static int s_fb_w = 0, s_fb_h = 0;
                const int vw = g_gl.viewport_w, vh = g_gl.viewport_h;
                if (s_fb == 0) glGenTextures(1, &s_fb);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, s_fb);
                if (s_fb_w != vw || s_fb_h != vh) {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, vw, vh, 0);
                    s_fb_w = vw;
                    s_fb_h = vh;
                } else {
                    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, vw, vh);
                }
                // Canvas point -> texture coordinate of where it is on screen.
                auto to_uv = [&](float cx, float cy, float& u, float& v) {
                    const float nx = cx / contentW * 2.0f - 1.0f, ny = 1.0f - cy / contentH * 2.0f;
                    u = (nx * s_lens_scale[0] + s_lens_off[0] + 1.0f) * 0.5f;
                    v = (ny * s_lens_scale[1] + s_lens_off[1] + 1.0f) * 0.5f;
                };
                constexpr int G = 32;              // grid cells per side
                constexpr float kBulge = 0.09f;    // CRT-Geom-like barrel curvature
                for (int pass = 0; pass < 3; pass++) {
                    // pass 0 sharp; 1-2 nudged copies, faint, for the lens's softness
                    const float nudge = pass == 0 ? 0.0f : (pass == 1 ? 1.0f : -1.0f) * 1.5f * unit;
                    std::vector<RenderVertex> v;
                    for (const Rect& r : rects) {
                        auto src = [&](float qx, float qy, float& u, float& vv) {  // q in [-1,1]
                            // Barrel: points toward the edges read from further out,
                            // so the picture bulges and its edges curve (past the
                            // screen = the black corners, covered by the mask below).
                            float bx = qx * (1.0f + kBulge * (qy * qy + 0.25f * qx * qx));
                            float by = qy * (1.0f + kBulge * (qx * qx + 0.25f * qy * qy));
                            bx = std::max(-1.0f, std::min(1.0f, bx));
                            by = std::max(-1.0f, std::min(1.0f, by));
                            const float sx2 = r.x + (bx + 1.0f) * 0.5f * r.w + nudge;
                            const float sy2 = r.y + (by + 1.0f) * 0.5f * r.h + nudge;
                            to_uv(sx2, sy2, u, vv);
                        };
                        for (int j = 0; j < G; j++)
                            for (int i = 0; i < G; i++) {
                                const float qx0 = -1.0f + 2.0f * i / G, qx1 = -1.0f + 2.0f * (i + 1) / G;
                                const float qy0 = -1.0f + 2.0f * j / G, qy1 = -1.0f + 2.0f * (j + 1) / G;
                                const float x0 = r.x + (qx0 + 1.0f) * 0.5f * r.w, x1 = r.x + (qx1 + 1.0f) * 0.5f * r.w;
                                const float y0 = r.y + (qy0 + 1.0f) * 0.5f * r.h, y1 = r.y + (qy1 + 1.0f) * 0.5f * r.h;
                                float u00, v00, u10, v10, u01, v01, u11, v11;
                                src(qx0, qy0, u00, v00); src(qx1, qy0, u10, v10);
                                src(qx0, qy1, u01, v01); src(qx1, qy1, u11, v11);
                                auto P = [&](float cx, float cy, float u, float vv) {
                                    v.push_back({(cx / contentW) * 2.0f - 1.0f, 1.0f - (cy / contentH) * 2.0f, u, vv});
                                };
                                P(x0, y1, u01, v01); P(x1, y1, u11, v11); P(x1, y0, u10, v10);
                                P(x1, y0, u10, v10); P(x0, y0, u00, v00); P(x0, y1, u01, v01);
                            }
                    }
                    glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, pass == 0 ? 1.0f : 0.3f);
                    draw_vertices(s_fb, v);
                }
            }
            // 2) The lid darkens the top edge and the corners, smoothly.
            // A smooth round falloff (a small texture made once, stretched
            // over each screen) - stepped rings show as bands here.
            static GLuint s_lens = 0;
            if (s_lens == 0) {
                constexpr int N = 256;  // large enough that the fade has no steps
                static uint8_t px[N * N * 4];
                for (int y = 0; y < N; y++)
                    for (int x = 0; x < N; x++) {
                        const float dx = (x + 0.5f) / N * 2.0f - 1.0f, dy = (y + 0.5f) / N * 2.0f - 1.0f;
                        // Black where the bent picture runs out (the curved
                        // edges and corners - same curve as the grid above),
                        // soft at the edge, and a slight darkening toward corners.
                        constexpr float kB = 0.09f;
                        const float bx = dx * (1.0f + kB * (dy * dy + 0.25f * dx * dx));
                        const float by = dy * (1.0f + kB * (dx * dx + 0.25f * dy * dy));
                        const float e = std::min(1.0f - std::fabs(bx), 1.0f - std::fabs(by));
                        float in = e / 0.02f;
                        in = in <= 0.0f ? 0.0f : in >= 1.0f ? 1.0f : in * in * (3.0f - 2.0f * in);
                        const float vig = 0.25f * std::sqrt(std::fabs(dx * dy));
                        const float a = 1.0f - in * (1.0f - vig);
                        uint8_t* p = &px[(y * N + x) * 4];
                        p[0] = p[1] = p[2] = 0;
                        p[3] = (uint8_t)(a * 255.0f);
                    }
                glGenTextures(1, &s_lens);
                glBindTexture(GL_TEXTURE_2D, s_lens);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
            }
            std::vector<RenderVertex> v;
            for (const Rect& r : rects) append_quad_ndc_uv_canvas(v, contentW, contentH, r.x, r.y, r.w, r.h, 0, 0, 1, 1);
            glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);  // corners fully black
            draw_vertices(s_lens, v);
            break;
        }
        default: break;
    }
}
}  // namespace

void switch_screen_init(int viewport_w, int viewport_h) {
    switch_gl_init_resources(g_gl);
    g_gl.viewport_w = viewport_w;
    g_gl.viewport_h = viewport_h;
}

// Dot-matrix cartridge systems drawn with pixel-snapped dots: Epoch Game
// Pocket Computer (refs "GPC...", 75x64) and Hartung Game Master ("HGM...", 61x64).
static bool is_dot_matrix_game() {
    return g_game && (g_game->ref.compare(0, 3, "GPC") == 0 || g_game->ref.compare(0, 3, "HGM") == 0);
}
static int dot_matrix_width() {
    return (g_game && g_game->ref.compare(0, 3, "HGM") == 0) ? 61 : 75;
}

void switch_screen_load_textures() {
    if (!g_game) {
        return;
    }
    if (g_loaded_texture_generation == g_texture_generation) {
        return;
    }

    if (g_gl.tex_segments) glDeleteTextures(1, &g_gl.tex_segments);
    if (g_gl.tex_background) glDeleteTextures(1, &g_gl.tex_background);
    if (g_gl.tex_console) glDeleteTextures(1, &g_gl.tex_console);
    if (g_gl.tex_overlay) glDeleteTextures(1, &g_gl.tex_overlay);

    const std::string seg_asset = t3x_path_to_png_asset(g_game->path_segment);
    const std::string bg_asset = t3x_path_to_png_asset(g_game->path_background);
    const std::string cs_asset = t3x_path_to_png_asset(g_game->path_console);
    // Overlay has no path_overlay field of its own (see GW_rom::background_info_count's
    // comment) - derived from the background asset name by the same "background_"/
    // "overlay_" naming convention overlay_data_file() writes its file under. Games
    // without an overlay layer just don't have this file in the pack; upload_png_from_pack
    // already degrades gracefully (logs, returns 0) when an asset name isn't found.
    std::string ov_asset = bg_asset;
    {
        const std::string from = "background_";
        const size_t pos = ov_asset.find(from);
        if (pos != std::string::npos) {
            ov_asset.replace(pos, from.size(), "overlay_");
        } else {
            ov_asset.clear();
        }
    }
    YOKOI_LOG("switch_screen: loading textures for '%s' (gen=%u): seg='%s' (raw '%s'), bg='%s' (raw '%s'), console='%s' (raw '%s'), overlay='%s'",
              g_game->name.c_str(), (unsigned)g_texture_generation,
              seg_asset.c_str(), g_game->path_segment.c_str(),
              bg_asset.c_str(), g_game->path_background.c_str(),
              cs_asset.c_str(), g_game->path_console.c_str(),
              ov_asset.c_str());

    g_gl.tex_segments = upload_png_from_pack(seg_asset, g_gl.tex_segments_w, g_gl.tex_segments_h);
    // Epoch Game Pocket Computer (refs "GPC..."): its 4800 dots all share one
    // 16 px tile, drawn at ~8-11 px. Plain linear filtering then samples it
    // unevenly - dots and gaps of different widths. Mipmaps keep every dot
    // the same at any size.
    if (g_gl.tex_segments && is_dot_matrix_game()) {
        glBindTexture(GL_TEXTURE_2D, g_gl.tex_segments);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    g_gl.tex_background = upload_png_from_pack(bg_asset, g_gl.tex_background_w, g_gl.tex_background_h);
    g_gl.tex_console = upload_png_from_pack(cs_asset, g_gl.tex_console_w, g_gl.tex_console_h);
    g_gl.tex_overlay = ov_asset.empty() ? 0 : upload_png_from_pack(ov_asset, g_gl.tex_overlay_w, g_gl.tex_overlay_h);

    YOKOI_LOG("switch_screen: tex ids seg=%u (%dx%d) bg=%u (%dx%d) console=%u (%dx%d) overlay=%u (%dx%d)",
              (unsigned)g_gl.tex_segments, g_gl.tex_segments_w, g_gl.tex_segments_h,
              (unsigned)g_gl.tex_background, g_gl.tex_background_w, g_gl.tex_background_h,
              (unsigned)g_gl.tex_console, g_gl.tex_console_w, g_gl.tex_console_h,
              (unsigned)g_gl.tex_overlay, g_gl.tex_overlay_w, g_gl.tex_overlay_h);

    // Prefer actual decoded texture sizes for UV normalization, same as Android.
    if (g_gl.tex_segments_w > 0 && g_gl.tex_segments_h > 0) {
        g_segment_info[0] = (uint16_t)g_gl.tex_segments_w;
        g_segment_info[1] = (uint16_t)g_gl.tex_segments_h;
    }

    g_loaded_texture_generation = g_texture_generation;
}

// s_panel: the slice comes from switch_screen_set_panel() (the local link) -
// at Game Rotation 90/270 it's a slice of the rotated screen's height, so
// the two devices stack top to bottom. The keypad's set_area() only applies
// at 0.
namespace { float s_area_frac = 1.0f; float s_area_off = 0.0f; bool s_clear = true; bool s_panel = false; }
void switch_screen_set_area(float width_frac) {
    s_area_frac = std::min(1.0f, std::max(0.2f, width_frac));
    s_area_off = 0.0f;
    s_clear = true;
    s_panel = false;
}

void switch_screen_set_panel(float width_frac, float offset_frac, bool clear) {
    s_area_frac = std::min(1.0f, std::max(0.2f, width_frac));
    s_area_off = std::min(1.0f - s_area_frac, std::max(0.0f, offset_frac));
    s_clear = clear;
    s_panel = true;
}

void switch_screen_swap_game_state(SwitchScreenGameState& o) {
    GLuint t;
    t = g_gl.tex_segments; g_gl.tex_segments = o.tex_segments; o.tex_segments = t;
    t = g_gl.tex_background; g_gl.tex_background = o.tex_background; o.tex_background = t;
    t = g_gl.tex_console; g_gl.tex_console = o.tex_console; o.tex_console = t;
    t = g_gl.tex_overlay; g_gl.tex_overlay = o.tex_overlay; o.tex_overlay = t;
    std::swap(g_gl.tex_segments_w, o.tex_segments_w);
    std::swap(g_gl.tex_segments_h, o.tex_segments_h);
    std::swap(g_gl.tex_background_w, o.tex_background_w);
    std::swap(g_gl.tex_background_h, o.tex_background_h);
    std::swap(g_gl.tex_console_w, o.tex_console_w);
    std::swap(g_gl.tex_console_h, o.tex_console_h);
    std::swap(g_gl.tex_overlay_w, o.tex_overlay_w);
    std::swap(g_gl.tex_overlay_h, o.tex_overlay_h);
    std::swap(g_loaded_texture_generation, o.loaded_generation);
    std::swap(g_seg_level, o.seg_level);
}

void switch_screen_free_game_state(SwitchScreenGameState& s) {
    GLuint ids[4] = {(GLuint)s.tex_segments, (GLuint)s.tex_background, (GLuint)s.tex_console, (GLuint)s.tex_overlay};
    for (GLuint id : ids) {
        if (id) glDeleteTextures(1, &id);
    }
    s = SwitchScreenGameState();
}

// Pixel Style (switch_settings.h): the dots of a Brick Game's dot-matrix
// grid, worked out once per game (kept for the two a local link can show).
// Only games of the Brick Game maker - other makers' grids (Microvision)
// have plain dots, as on the real LCDs. A dot is a squarish segment of about
// the size most of them share (within 25%: the art's dots come out a few
// pixels apart in size) that sits on the grid - one grid step from another.
// Each is drawn at its grid cell's centre with the cell's dot size, so all
// look the same.
struct GridDots {
    bool on = false;
    std::vector<uint8_t> is_dot;
    float dot = 0.0f, step = 0.0f, ox = 0.0f, oy = 0.0f;  // segment units
};
std::vector<std::pair<std::string, GridDots>> g_grid_cache;

bool switch_screen_pixel_style_applies() {
    // The pack keeps refs in capitals. Since CONVERT_ROM/fix_brick_dots.py
    // these three have the frame-and-centre dot their SVG always meant (the
    // importer had filled it in), so Pixel Style now goes the other way:
    // Classic leaves the art alone, Filled paints each lit dot solid.
    static const char* const kRefs[] = {"E23PLUSMARKII96IN1", "E332IN1", "E888IN1"};
    if (!g_game) return false;
    std::string ref = g_game->ref;
    for (char& c : ref) c = (char)toupper((unsigned char)c);
    if (ref.compare(0, 3, "GPC") == 0 || ref.compare(0, 3, "HGM") == 0) return true;  // Game Pocket Computer: Filled / Solid only
    for (const char* r : kRefs)
        if (ref == r) return true;
    return false;
}

const GridDots& grid_dots() {
    static const GridDots kNone;
    if (!switch_screen_pixel_style_applies()) return kNone;
    for (auto& e : g_grid_cache)
        if (e.first == g_game->ref && e.second.is_dot.size() == g_segments.size()) return e.second;
    GridDots gd;
    gd.is_dot.assign(g_segments.size(), 0);
    // The most common square size.
    struct Sz { int s, n; };
    std::vector<Sz> sizes;
    for (const Segment& sg : g_segments) {
        const int w = sg.size_tex[0], h = sg.size_tex[1];
        if (w < 4 || h < 4 || std::abs(w - h) > std::max(2, w / 8)) continue;
        const int sz = (w + h) / 2;
        bool found = false;
        for (Sz& z : sizes)
            if (std::abs(z.s - sz) <= std::max(2, z.s / 8)) { z.n++; found = true; break; }
        if (!found) sizes.push_back({sz, 1});
    }
    int best = 0, dsz = 0;
    for (const Sz& z : sizes)
        if (z.n > best) { best = z.n; dsz = z.s; }
    if (best >= 40) {
        std::vector<size_t> cand;
        for (size_t i = 0; i < g_segments.size(); i++) {
            const Segment& sg = g_segments[i];
            const int w = sg.size_tex[0], h = sg.size_tex[1];
            if (std::abs(w - dsz) <= dsz / 4 && std::abs(h - dsz) <= dsz / 4) cand.push_back(i);
        }
        auto cxof = [&](size_t i) { return (float)g_segments[i].pos_scr[0] + g_segments[i].size_tex[0] * 0.5f; };
        auto cyof = [&](size_t i) { return (float)g_segments[i].pos_scr[1] + g_segments[i].size_tex[1] * 0.5f; };
        // Grid step: the usual centre-to-centre distance between neighbours in a row.
        std::vector<float> steps;
        for (size_t a = 0; a < cand.size(); a++)
            for (size_t b = 0; b < cand.size(); b++) {
                if (g_segments[cand[a]].screen != g_segments[cand[b]].screen) continue;
                if (std::fabs(cyof(cand[a]) - cyof(cand[b])) > dsz * 0.25f) continue;
                const float dx = cxof(cand[b]) - cxof(cand[a]);
                if (dx >= dsz * 0.9f && dx <= dsz * 1.8f) steps.push_back(dx);
            }
        if (!steps.empty()) {
            std::sort(steps.begin(), steps.end());
            gd.step = steps[steps.size() / 2];
            gd.dot = (float)dsz;
            const float tol = gd.step * 0.2f;
            // Origin: any dot's centre; cells are origin + k * step.
            gd.ox = cxof(cand[0]);
            gd.oy = cyof(cand[0]);
            // Any grid counts (the big one and NEXT's small one sit at
            // different offsets): a dot just needs a neighbour one step away.
            for (size_t a = 0; a < cand.size(); a++) {
                bool neighbour = false;
                for (size_t b = 0; b < cand.size() && !neighbour; b++) {
                    if (a == b || g_segments[cand[a]].screen != g_segments[cand[b]].screen) continue;
                    const float dx = std::fabs(cxof(cand[b]) - cxof(cand[a])), dy = std::fabs(cyof(cand[b]) - cyof(cand[a]));
                    neighbour = (std::fabs(dx - gd.step) <= tol && dy <= tol) || (std::fabs(dy - gd.step) <= tol && dx <= tol);
                }
                if (neighbour) gd.is_dot[cand[a]] = 1;
            }
            // Only real grids: a group of at least 12 dots joined by
            // one-step neighbours (the 10x20 field, NEXT's 4x4). Two icons
            // of about a dot's size one step apart - the E-88's ROTATE
            // arrows - are not a grid and keep their own art.
            std::vector<uint8_t> seen(g_segments.size(), 0);
            for (size_t a : cand) {
                if (!gd.is_dot[a] || seen[a]) continue;
                std::vector<size_t> comp, stack{a};
                seen[a] = 1;
                while (!stack.empty()) {
                    const size_t k = stack.back();
                    stack.pop_back();
                    comp.push_back(k);
                    for (size_t b : cand) {
                        if (seen[b] || !gd.is_dot[b] || g_segments[b].screen != g_segments[k].screen) continue;
                        const float dx = std::fabs(cxof(b) - cxof(k)), dy = std::fabs(cyof(b) - cyof(k));
                        if ((std::fabs(dx - gd.step) <= tol && dy <= tol) || (std::fabs(dy - gd.step) <= tol && dx <= tol)) {
                            seen[b] = 1;
                            stack.push_back(b);
                        }
                    }
                }
                if (comp.size() < 12)
                    for (size_t k : comp) gd.is_dot[k] = 0;
            }
            gd.on = true;
        }
    }
    if (g_grid_cache.size() >= 2) g_grid_cache.erase(g_grid_cache.begin());
    g_grid_cache.push_back({g_game->ref, gd});
    return g_grid_cache.back().second;
}

void switch_screen_render() {
    glViewport(0, 0, g_gl.viewport_w, g_gl.viewport_h);
    if (s_clear) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    if (g_gl.program == 0) {
        YOKOI_LOG("switch_screen_render: no shader program - bailing before any draw");
        return;
    }

    switch_screen_load_textures();

    // Throttled one-time-ish dump of the state that decides where/whether
    // anything actually draws, for troubleshooting a blank screen.
    static int s_frames_logged = 0;
    if (s_frames_logged < 3) {
        s_frames_logged++;
        YOKOI_LOG("switch_screen_render[%d]: viewport=%dx%d canvas=%.1fx%.1f combined=%.1fx%.1f "
                  "nb_screen=%u split=%d seg_info=[%u,%u,scale=%u,flags=%u] segments=%zu program=%u vao=%u vbo=%u white=%u",
                  s_frames_logged, g_gl.viewport_w, g_gl.viewport_h,
                  g_canvas_w, g_canvas_h, g_combined_canvas_w, g_combined_canvas_h,
                  (unsigned)g_nb_screen, (int)g_split_two_screens_to_panels,
                  (unsigned)g_segment_info[0], (unsigned)g_segment_info[1], (unsigned)g_segment_info[2], (unsigned)g_segment_info[3],
                  g_segments.size(), (unsigned)g_gl.program, (unsigned)g_gl.vao, (unsigned)g_gl.vbo, (unsigned)g_gl.tex_white);
    }

    glUseProgram(g_gl.program);
    glBindVertexArray(g_gl.vao);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (g_gl.uAlphaOnly >= 0) {
        glUniform1f(g_gl.uAlphaOnly, 0.0f);
    }

    const float contentW = g_combined_canvas_w > 0.0f ? g_combined_canvas_w : 1.0f;
    const float contentH = g_combined_canvas_h > 0.0f ? g_combined_canvas_h : 1.0f;
    const float contentAspect = contentW / contentH;

    // At 90/270, the content should be aspect-fit against a viewport with
    // width/height swapped, not the true (always-landscape) physical one:
    // NDC is a symmetric [-1,1] square regardless of aspect, so fitting
    // against the swapped aspect here and then rotating the already-fit NDC
    // quad below is exactly equivalent to fitting a rotated viewport
    // directly, with no extra distortion - see switch_ui.cpp's
    // switch_ui_begin_frame() for the same trick applied to the menu.
    int effViewportW = g_gl.viewport_w;
    int effViewportH = g_gl.viewport_h;
    if (g_screen_rotation != ScreenRotation::Deg0) {
        std::swap(effViewportW, effViewportH);
    }
    // On-screen keypad: fit into the left part only. Local link rotated:
    // a slice of the (rotated) height instead - player 1 on top.
    const bool rotated = g_screen_rotation != ScreenRotation::Deg0;
    const bool vslice = rotated && s_panel;
    const float area = (!rotated || vslice) ? s_area_frac : 1.0f;
    if (vslice) effViewportH = (int)((float)effViewportH * area);
    else effViewportW = (int)((float)effViewportW * area);
    const float viewAspect = (effViewportW > 0 && effViewportH > 0)
        ? (float)effViewportW / (float)effViewportH
        : 1.0f;

    float sx = 1.0f, sy = 1.0f;
    if (viewAspect > contentAspect) {
        sx = contentAspect / viewAspect;
    } else {
        sy = viewAspect / contentAspect;
    }
    // Screen Size (switch_settings.h). The quad is in NDC, so scaling sx/sy
    // scales the whole picture; anything pushed past the edges is clipped.
    switch (g_screen_size) {
        case 0: sx *= 0.70f; sy *= 0.70f; break;
        case 1: sx *= 0.85f; sy *= 0.85f; break;
        case 3: sx *= 1.15f; sy *= 1.15f; break;
        case 4: sx = 1.0f; sy = 1.0f; break;  // Stretched
        case 5: {
            // Pixel Exact: the largest whole multiple (or, for art bigger
            // than the screen, whole fraction) of the canvas size that fits.
            const float fit = std::min((float)effViewportW / contentW, (float)effViewportH / contentH);
            const float s = fit >= 1.0f ? std::floor(fit) : 1.0f / std::ceil(1.0f / fit);
            sx = s * contentW / (float)effViewportW;
            sy = s * contentH / (float)effViewportH;
            break;
        }
        default: break;  // Normal
    }
    glUniform2f(g_gl.uScale, vslice ? sx : sx * area, vslice ? sy * area : sy);
    s_lens_scale[0] = vslice ? sx : sx * area;
    s_lens_scale[1] = vslice ? sy * area : sy;
    s_lens_off[0] = vslice ? 0.0f : 2.0f * (rotated ? 0.0f : s_area_off) + area - 1.0f;
    s_lens_off[1] = vslice ? 1.0f - 2.0f * s_area_off - area : 0.0f;
    s_lens_ok = !rotated;
    if (g_gl.uOffset >= 0) {
        // Centre of the slice in NDC: -1 + 2 * (offset + area / 2) - across
        // from the left, or (rotated link) down from the top, where NDC y = 1.
        if (vslice) {
            glUniform2f(g_gl.uOffset, 0.0f, 1.0f - 2.0f * s_area_off - area);
        } else {
            const float off = rotated ? 0.0f : s_area_off;
            glUniform2f(g_gl.uOffset, 2.0f * off + area - 1.0f, 0.0f);
        }
    }
    if (g_gl.uRotCosSin >= 0) {
        // Signs verified against hardware - they're the opposite of what the
        // rotation matrix's math would suggest at a glance, likely because
        // this shader's NDC y-axis convention is already flipped somewhere
        // upstream (kUvFlipV-style). Flip both here and in switch_ui.cpp's
        // matching block together if this ever needs revisiting.
        float rotCos = 1.0f, rotSin = 0.0f;
        if (g_screen_rotation == ScreenRotation::Deg90) {
            rotCos = 0.0f; rotSin = -1.0f;
        } else if (g_screen_rotation == ScreenRotation::Deg270) {
            rotCos = 0.0f; rotSin = 1.0f;
        }
        glUniform2f(g_gl.uRotCosSin, rotCos, rotSin);
    }

    auto to_local_x = [&](float gx) { return gx; };
    auto to_local_y = [&](float gy) { return gy; };

    constexpr uint32_t kMaskFondRgb = 0x080908;
    const bool is_mask_game = (g_segment_info[3] & 0x01) != 0;
    // Tiger R-Zone (refs "RZ..."): red screen in the dark XPG - a black
    // canvas, the screen itself dark red, lit segments bright red, unlit ones
    // a faint darker red (as photos of the real console).
    // R-Zone Screen "LCD" (g_rz_style 2): no colours of its own - the plain
    // LCD, for the usual palette / colour settings.
    const bool rzone = g_game && g_game->ref.compare(0, 2, "RZ") == 0 && !is_mask_game && g_rz_style != 2;
    const uint32_t bg = is_mask_game ? kMaskFondRgb : rzone ? 0x050303u : effective_background_color();
    const float br = ((bg >> 16) & 0xFF) / 255.0f;
    const float bgc = ((bg >> 8) & 0xFF) / 255.0f;
    const float bb = (bg & 0xFF) / 255.0f;

    // Base fill.
    if (g_gl.tex_white != 0) {
        glUniform4f(g_gl.uMul, br, bgc, bb, 1.0f);
        std::vector<RenderVertex> fill_verts;
        append_quad_ndc_uv_canvas(fill_verts, contentW, contentH, 0.0f, 0.0f, contentW, contentH, 0.0f, 0.0f, 1.0f, 1.0f);
        draw_vertices(g_gl.tex_white, fill_verts);
        if (rzone) {
            const uint16_t sc = g_segment_info[2] ? g_segment_info[2] : 1;
            float gx = 0.0f, gy = 0.0f;
            get_screen_base_global(0, gx, gy);
            std::vector<RenderVertex> red;
            append_quad_ndc_uv_canvas(red, contentW, contentH, to_local_x(gx), to_local_y(gy),
                                      (float)g_segment_info[4] / sc, (float)g_segment_info[5] / sc, 0.0f, 0.0f, 1.0f, 1.0f);
            // R-Zone Screen (g_rz_style): 0 White - crimson, lit segments
            // near white; 1 Red - dark screen, lit segments bright red.
            if (g_rz_style == 1) glUniform4f(g_gl.uMul, 40.0f / 255.0f, 0.0f, 4.0f / 255.0f, 1.0f);
            else glUniform4f(g_gl.uMul, 178.0f / 255.0f, 4.0f / 255.0f, 20.0f / 255.0f, 1.0f);
            draw_vertices(g_gl.tex_white, red);
        }
    }

    // Printed art (Background + Overlay) unless Background Art is Hidden -
    // masked games always keep it (see g_hide_background_art).
    const bool draw_art = !g_hide_background_art || is_mask_game;

    // Background layer (per-screen rect from GW_rom::background_info).
    if (draw_art && g_gl.tex_background != 0 && g_game && g_game->background_info && g_gl.tex_background_w > 0 && g_gl.tex_background_h > 0) {
        const uint16_t* bi = g_game->background_info;
        // A Screen Palette tints the printed art with its background colour.
        if (g_screen_palette > 0 && !is_mask_game) {
            glUniform4f(g_gl.uMul, br, bgc, bb, 1.0f);
        } else {
            glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        std::vector<RenderVertex> bg_verts;
        bg_verts.reserve(g_nb_screen * 6);
        const float texW = (float)g_gl.tex_background_w;
        const float texH = (float)g_gl.tex_background_h;
        const uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;

        for (uint8_t screen = 0; screen < g_nb_screen; screen++) {
            uint16_t id = (uint16_t)(2 + screen * 4);
            float u = (float)bi[id + 0];
            float v = (float)bi[id + 1];
            float w = (float)bi[id + 2];
            float h = (float)bi[id + 3];

            float u0, v0, u1, v1;
            calc_uv_rect(texW, texH, u, v, w, h, u0, v0, u1, v1);

            float dw = (float)g_segment_info[4 + screen * 2] / (float)scale;
            float dh = (float)g_segment_info[5 + screen * 2] / (float)scale;

            float gx = 0.0f, gy = 0.0f;
            get_screen_base_global(screen, gx, gy);
            append_quad_ndc_uv_canvas(bg_verts, contentW, contentH, to_local_x(gx), to_local_y(gy), dw, dh, u0, v0, u1, v1);
        }
        draw_vertices(g_gl.tex_background, bg_verts);
    }

    // Segments layer.
    if (g_segment_info[0] > 0 && g_segment_info[1] > 0) {
        const bool is_mask = (g_segment_info[3] & 0x01) != 0;

        // 1-4: Spitball Sparky / Crab Grab's coloured segments. 5-41: the
        // VFD phosphor / colour-overlay colours of the HMCS40 tabletop games,
        // as their MAME SVGs draw them - CONVERT_ROM/prepare_hmcs40.py's PALETTE.
        static constexpr uint32_t kSegmentColorCount = 42;
        static constexpr uint32_t kSegmentColorRgb[kSegmentColorCount] = {
            0x080908u, 0x9992e7u, 0x58b9a0u, 0xff677cu, 0x3db8e4u,
            0x80fff8u, 0xff4820u, 0x50ff30u, 0xe6e6e6u, 0xf60000u, 0xb49dffu,
            0xb0ffe0u, 0xe0ffd0u, 0xff2c00u, 0xfff0c0u, 0xff5b00u, 0xffc75fu,
            0xffe45fu, 0xffd55fu, 0x64f0f0u, 0xffff48u,
            0x4840ffu, 0xff9429u, 0xff1700u, 0x64dfffu, 0xff4e30u,
            0xff0000u, 0x9765ffu, 0x008fffu, 0x00bb00u, 0x000000u,
            0x488f8bu, 0x8f2812u, 0x28248fu,  // Zaxxon, seen through its mirror
            0x9eeae8u, 0x64ff71u, 0xff6600u,  // VTech Invaders' overlay
            0x0d66e0u,                        // Knock-Em Out Boxing, through its blue window
            0x00ffcau, 0x26c9ffu, 0x00b84eu, 0xff6820u,  // Coleco Frogger (MELPS4)
        };

        std::vector<RenderVertex> seg_verts;
        std::vector<RenderVertex> seg_color_verts[kSegmentColorCount];
        std::vector<RenderVertex> seg_mark_verts;
        std::vector<RenderVertex> seg_shadow_verts;
        bool has_colored_segments = false;

        const size_t seg_count = g_segments.size();
        seg_verts.reserve(seg_count * 6);
        if (!is_mask) {
            seg_mark_verts.reserve(seg_count * 6);
            seg_shadow_verts.reserve(seg_count * 6);
        }

        const uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        const float texW = (float)g_segment_info[0];
        const float texH = (float)g_segment_info[1];

        // Ghosting is skipped for the mask and multiply-darken passes (their
        // blending can't take a partial alpha) - those keep the instant look.
        const bool multiply_darken_pass = (g_segment_info[3] & 0x04) != 0;
        const int ghost = (!is_mask && !multiply_darken_pass && g_ghosting <= 3) ? g_ghosting : 0;
        if (g_seg_level.size() != seg_count) g_seg_level.assign(seg_count, 0.0f);
        std::vector<RenderVertex> ghost_verts[kGhostBins];
        std::vector<std::pair<uint32_t, std::vector<RenderVertex>>> ghost_color_verts[kGhostBins];

        // Pixel Style (switch_settings.h) = Filled: which segments are the
        // dots of a dot-matrix grid - worked out once per game (grid_dots()).
        static const GridDots kNoGrid;
        const GridDots& grid = (g_pixel_style >= 1 && !is_mask && !(g_game && is_dot_matrix_game())) ? grid_dots() : kNoGrid;

        // Epoch Game Pocket Computer (refs "GPC..."): 75x64 dots, each segment
        // one dot - an 18x18 cell, its 16x16 dot at +1 (CONVERT_ROM/
        // build_gamepock_pack.py). A dot is ~8 or ~11 screen pixels, never a
        // whole number, so drawn as is its edges land mid-pixel and dots come
        // out of different darkness and sharpness. Here every dot is snapped
        // to whole screen pixels instead: Pixel Style Filled (the default -
        // Classic counts as Filled here) a square with a 1 px gap, Solid the
        // whole cell, so lit dots join.
        const bool gpc = g_game && is_dot_matrix_game() && !is_mask;
        const int dm_w = dot_matrix_width();  // 75 Game Pocket, 61 Game Master
        const float px_per_x = sx * (float)effViewportW / contentW;   // screen pixels per canvas unit
        const float px_per_y = sy * (float)effViewportH / contentH;
        const float px_x0 = (float)effViewportW * (1.0f - sx) * 0.5f; // canvas x 0 on screen
        const float px_y0 = (float)effViewportH * (1.0f - sy) * 0.5f;
        auto snap_x = [&](float cx) { return (std::round(px_x0 + cx * px_per_x) - px_x0) / px_per_x; };
        auto snap_y = [&](float cy) { return (std::round(px_y0 + cy * px_per_y) - px_y0) / px_per_y; };
        // Which dots are lit, by position (Solid joins lit neighbours) - by
        // the segments' ids, whatever order the loader keeps them in.
        static uint8_t s_gpc_lit[64][75];
        if (gpc) {
            memset(s_gpc_lit, 0, sizeof(s_gpc_lit));
            for (size_t i = 0; i < seg_count && i < g_seg_on.size(); i++)
                if (g_seg_on[i] && g_segments[i].id[0] < dm_w && g_segments[i].id[1] < 64) s_gpc_lit[g_segments[i].id[1]][g_segments[i].id[0]] = 1;
        }
        std::vector<RenderVertex> dot_gap_verts;  // Filled style: the solid squares

        for (size_t si = 0; si < seg_count; si++) {
            const Segment& seg = g_segments[si];
            const bool raw_on = (si < g_seg_on.size()) ? (g_seg_on[si] != 0) : false;
            float& level = g_seg_level[si];
            if (ghost == 0) {
                level = raw_on ? 1.0f : 0.0f;
            } else if (raw_on) {
                level = std::min(1.0f, level + kGhostRise[ghost]);
            } else {
                level = std::max(0.0f, level - kGhostFall[ghost]);
            }
            const bool seg_on = level >= 0.999f;
            const bool seg_fading = !seg_on && level > 0.001f;

            float base_gx = 0.0f, base_gy = 0.0f;
            get_screen_base_global(seg.screen, base_gx, base_gy);

            float sx2 = (float)seg.pos_scr[0] / (float)scale + base_gx;
            float sy2 = (float)seg.pos_scr[1] / (float)scale + base_gy;
            float sw = (float)seg.size_tex[0] / (float)scale;
            float sh = (float)seg.size_tex[1] / (float)scale;

            float sx_local = to_local_x(sx2);
            float sy_local = to_local_y(sy2);
            // Solid: the 1 px gap to a lit neighbour on the right / below is
            // filled too (and the corner between four lit dots), so a block's
            // outer edges stay on the same grid lines as every other dot.
            bool bridge_r = false, bridge_d = false, bridge_c = false;
            float gap_x = 0.0f, gap_y = 0.0f;
            if (gpc && px_per_x > 0.0f && px_per_y > 0.0f) {
                const float cx0 = sx_local - 1.0f / (float)scale, cy0 = sy_local - 1.0f / (float)scale;
                const float cell = 18.0f / (float)scale;
                gap_x = 1.0f / px_per_x;
                gap_y = 1.0f / px_per_y;
                const float x0 = snap_x(cx0), x1 = snap_x(cx0 + cell), y0 = snap_y(cy0), y1 = snap_y(cy0 + cell);
                sx_local = x0;
                sy_local = y0;
                sw = std::max(x1 - x0 - gap_x, 1.0f / px_per_x);
                sh = std::max(y1 - y0 - gap_y, 1.0f / px_per_y);
                if (g_pixel_style == 2 && raw_on) {
                    const int dx = seg.id[0], dy = seg.id[1];
                    if (dx < dm_w && dy < 64) {
                        bridge_r = dx < dm_w - 1 && s_gpc_lit[dy][dx + 1];
                        bridge_d = dy < 63 && s_gpc_lit[dy + 1][dx];
                        bridge_c = bridge_r && bridge_d && s_gpc_lit[dy + 1][dx + 1];
                    }
                }
            }

            float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
            if (g_gl.tex_segments != 0) {
                if (gpc) {
                    // One texel in the middle of the (solid) dot tile for the
                    // whole quad: the quad's own pixel-snapped edges give the
                    // shape, and a constant UV means no mipmap blur - the 1 px
                    // Solid bridges, sampled across the tile, came out nearly
                    // transparent from the smallest mip level.
                    calc_uv_rect(texW, texH, (float)seg.pos_tex[0] + (float)seg.size_tex[0] * 0.5f,
                                 (float)seg.pos_tex[1] + (float)seg.size_tex[1] * 0.5f, 0.0f, 0.0f, u0, v0, u1, v1);
                } else
                calc_uv_rect(texW, texH, (float)seg.pos_tex[0], (float)seg.pos_tex[1],
                             (float)seg.size_tex[0], (float)seg.size_tex[1], u0, v0, u1, v1);
            }

            if (!is_mask) {
                append_quad_ndc_uv_canvas(seg_mark_verts, contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                if (seg_on && !gpc) {
                    append_quad_ndc_uv_canvas(seg_shadow_verts, contentW, contentH, sx_local + 2.0f, sy_local + 2.0f, sw, sh, u0, v0, u1, v1);
                }
            }

            if (seg_fading) {
                const int bin = std::min(kGhostBins - 1, (int)(level * kGhostBins));
                if (seg.color_index > 0 && seg.color_index < kSegmentColorCount) {
                    has_colored_segments = true;
                    auto& lst = ghost_color_verts[bin];
                    auto it = std::find_if(lst.begin(), lst.end(), [&](const std::pair<uint32_t, std::vector<RenderVertex>>& p) {
                        return p.first == seg.color_index;
                    });
                    if (it == lst.end()) {
                        lst.emplace_back(seg.color_index, std::vector<RenderVertex>());
                        it = lst.end() - 1;
                    }
                    append_quad_ndc_uv_canvas(it->second, contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                } else {
                    append_quad_ndc_uv_canvas(ghost_verts[bin], contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                }
            }

            if (seg_on && grid.on && si < grid.is_dot.size() && grid.is_dot[si]) {
                // Pixel Style = Filled: a solid square over the dot's frame
                // and centre, the same size on every dot of the grid.
                const float cx = sx_local + sw * 0.5f, cy = sy_local + sh * 0.5f;
                // 82% of the grid step, like the LCD's own dots: a 2-unit
                // inset merged neighbours once the art came in at 3x.
                const float d = (g_pixel_style == 2 ? grid.step + 0.5f : std::min(grid.dot, grid.step * 0.82f)) / (float)scale;
                append_quad_ndc_uv_canvas(dot_gap_verts, contentW, contentH, cx - d * 0.5f, cy - d * 0.5f, d, d, 0, 0, 1, 1);
            }
            if (seg_on) {
                if (!is_mask && seg.color_index > 0 && seg.color_index < kSegmentColorCount) {
                    has_colored_segments = true;
                    append_quad_ndc_uv_canvas(seg_color_verts[seg.color_index], contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                } else {
                    append_quad_ndc_uv_canvas(seg_verts, contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                    if (bridge_r) append_quad_ndc_uv_canvas(seg_verts, contentW, contentH, sx_local + sw, sy_local, gap_x, sh, u0, v0, u1, v1);
                    if (bridge_d) append_quad_ndc_uv_canvas(seg_verts, contentW, contentH, sx_local, sy_local + sh, sw, gap_y, u0, v0, u1, v1);
                    if (bridge_c) append_quad_ndc_uv_canvas(seg_verts, contentW, contentH, sx_local + sw, sy_local + sh, gap_x, gap_y, u0, v0, u1, v1);
                }
            }
        }

        // Lit-segment colour - the Segment Color setting, or (Auto) a tint
        // of the background colour; see effective_segment_color().
        const uint32_t seg_rgb = rzone ? (g_rz_style == 1 ? 0xFF282Du : 0xFFBE8Cu) : effective_segment_color(bg);
        const float seg_r = ((seg_rgb >> 16) & 0xFF) / 255.0f;
        const float seg_g = ((seg_rgb >> 8) & 0xFF) / 255.0f;
        const float seg_b = (seg_rgb & 0xFF) / 255.0f;
        const GLuint seg_tex = (g_gl.tex_segments != 0) ? g_gl.tex_segments : g_gl.tex_white;

        if (!is_mask) {
            const float mark_a = (float)g_settings.segment_marking_alpha / 255.0f;
            if (rzone && !seg_mark_verts.empty()) {
                // R-Zone: the unlit drawings always show, a darker red.
                if (g_rz_style == 1) glUniform4f(g_gl.uMul, 70.0f / 255.0f, 0.0f, 6.0f / 255.0f, 0.6f);
                else glUniform4f(g_gl.uMul, 120.0f / 255.0f, 0.0f, 6.0f / 255.0f, 0.55f);
                draw_vertices(seg_tex, seg_mark_verts);
            } else if (mark_a > 0.0f && !seg_mark_verts.empty()) {
                // Unlit-segment "ghost" marks, in the lit colour at low
                // alpha (it used to be a fixed near-black, which matched the
                // old always-dark segments but disappears on dark backgrounds).
                glUniform4f(g_gl.uMul, seg_r, seg_g, seg_b, mark_a);
                draw_vertices(seg_tex, seg_mark_verts);
            }
            if (!seg_shadow_verts.empty()) {
                const float s = 17.0f / 255.0f;
                glUniform4f(g_gl.uMul, s, s, s, (float)0x18 / 255.0f);
                draw_vertices(seg_tex, seg_shadow_verts);
            }
        }

        // Games whose "on" segments are large filled rectangles sitting
        // directly over printed colour/label art (e.g. Bassmate's option
        // grid) need those segments to darken the art in place rather than
        // replace it outright - standard alpha blending on a ~fully-opaque
        // fill just erases whatever's underneath, which is fine for a thin
        // digit stroke but wrong for a cell-sized block. multiply_darken
        // switches to a multiply blend for exactly that "on" draw: outside
        // the shape it multiplies the destination by 1.0 (no-op), inside it
        // by the tint colour (darken-in-place), matching how MAME's own
        // artwork compositing (and real LCD ink over a printed backdrop)
        // looks. See GW_rom::segment_info's flags byte / MULTIPLY_DARKEN_BIT.
        const bool multiply_darken = (g_segment_info[3] & 0x04) != 0;

        if (is_mask) {
            if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
            glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
            draw_vertices(seg_tex, seg_verts);
        } else if (multiply_darken) {
            // Darkens whatever's already drawn (the blank NS backdrop) in
            // place instead of replacing it - same seg_r/g/b tint every
            // other game's segments use, since label legibility is no
            // longer this pass's job: the overlay layer below (MAME's own
            // TEXT-Top/Bottom, drawn after segments at a fixed alpha) is
            // what keeps labels visible regardless of how dark the "on"
            // tint is, exactly matching the real device's LCD-ink-over-
            // printed-backdrop construction.
            if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
            if (g_gl.uMultiplyDarken >= 0) glUniform1f(g_gl.uMultiplyDarken, 1.0f);
            glUniform4f(g_gl.uMul, seg_r, seg_g, seg_b, 1.0f);
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            draw_vertices(seg_tex, seg_verts);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            if (g_gl.uMultiplyDarken >= 0) glUniform1f(g_gl.uMultiplyDarken, 0.0f);
        } else {
            if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
            glUniform4f(g_gl.uMul, seg_r, seg_g, seg_b, 1.0f);
            draw_vertices(seg_tex, seg_verts);
            // Screen Filter "Magnifier": a soft glow round lit segments, as
            // through a lens (copies nudged a few units each way, faint).
            if (g_screen_filter == kFilterMagnifier && !gpc && !seg_verts.empty()) {
                static const float kOff[8][2] = {{3, 0}, {-3, 0}, {0, 3}, {0, -3}, {2, 2}, {-2, 2}, {2, -2}, {-2, -2}};
                std::vector<RenderVertex> glow(seg_verts.size());
                glUniform4f(g_gl.uMul, seg_r, seg_g, seg_b, 0.16f);
                for (const auto& o : kOff) {
                    const float dx = 2.0f * o[0] / contentW, dy = 2.0f * o[1] / contentH;
                    for (size_t k = 0; k < seg_verts.size(); k++) {
                        glow[k] = seg_verts[k];
                        glow[k].x += dx;
                        glow[k].y += dy;
                    }
                    draw_vertices(seg_tex, glow);
                }
            }
            // Fading (ghosting) segments, one alpha step per batch.
            for (int b = 0; b < kGhostBins; b++) {
                if (ghost_verts[b].empty()) continue;
                glUniform4f(g_gl.uMul, seg_r, seg_g, seg_b, ((float)b + 0.5f) / (float)kGhostBins);
                draw_vertices(seg_tex, ghost_verts[b]);
            }
        }

        if (!is_mask && has_colored_segments) {
            // Deliberately NOT alpha-only mode here (unlike text/glyphs in
            // switch_ui.cpp, which genuinely need it). Every segment texture
            // this pipeline produces (CONVERT_ROM/source/img_manipulation.py's
            // convert_to_only_Alpha()) is RGB-white with only alpha carrying
            // real shape data - a deliberate trick so plain `t * uMul`
            // multiplication already tints correctly (white is the
            // multiplicative identity) while alpha still gates transparency.
            // The shader's alpha-only path instead computes
            // max(t.a, max(t.r,t.g,t.b)) - since t.rgb is *always* 1.0 for
            // these textures regardless of actual alpha, that max is always
            // 1.0, making the whole quad register as "in shape" - a solid
            // block. Only ever surfaced by Spitball Sparky/Crab Grab, the
            // only two games with color_segment=true (every other game's
            // segments never take this branch at all).
            for (uint32_t ci = 1; ci < kSegmentColorCount; ci++) {
                if (seg_color_verts[ci].empty()) continue;
                const uint32_t rgb = kSegmentColorRgb[ci];
                glUniform4f(g_gl.uMul, ((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, 1.0f);
                draw_vertices(seg_tex, seg_color_verts[ci]);
            }
            for (int b = 0; b < kGhostBins; b++) {
                for (const auto& p : ghost_color_verts[b]) {
                    const uint32_t rgb = kSegmentColorRgb[p.first];
                    glUniform4f(g_gl.uMul, ((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f,
                                ((float)b + 0.5f) / (float)kGhostBins);
                    draw_vertices(seg_tex, p.second);
                }
            }
        }
        if (!dot_gap_verts.empty() && g_gl.tex_white != 0) {
            glUniform4f(g_gl.uMul, seg_r, seg_g, seg_b, 1.0f);
            draw_vertices(g_gl.tex_white, dot_gap_verts);
        }
    }

    // Tiger Game.com (refs "TGC..."): no segments - the core keeps the
    // 200x160 LCD as shades (source/BrickCores/GameCom.h), drawn here as a
    // picture over game screen 0, in the real LCD's greys (taken from
    // photos of the console: pale grey-green to near black).
    if (g_game && g_game->ref.compare(0, 3, "TGC") == 0 && g_cpu) {
        GameCom* gc = static_cast<GameCom*>(g_cpu.get());  // core "SM8521" (only TGC games use it)
        // The Game.com options (pause menu > Sound), applied every frame.
        gc->set_sound_options(g_tgc_sound == 1, g_tgc_noise == 1);
        gc->set_speed_percent(g_tgc_speed ? 120 : 100);
        // Screen Palette Original: the real LCD's greys (from photos of the
        // console - the "Game.com" palette). Any other palette: its background
        // to its segment colour, in the same five steps.
        static const uint8_t kReal[5][3] = {{178, 192, 170}, {130, 140, 126}, {90, 97, 91}, {54, 58, 58}, {32, 34, 38}};
        uint8_t kPal[5][3];
        const uint8_t (*kShade)[3] = kReal;
        if (g_screen_palette > 0 && g_screen_palette < kNumScreenPalettes) {
            static const float kT[5] = {0.0f, 0.33f, 0.6f, 0.85f, 1.0f};
            const uint32_t pb = kScreenPalettes[g_screen_palette].bg, ps = kScreenPalettes[g_screen_palette].seg;
            for (int s = 0; s < 5; s++)
                for (int k = 0; k < 3; k++) {
                    const float b = (float)((pb >> (16 - 8 * k)) & 0xFF), f = (float)((ps >> (16 - 8 * k)) & 0xFF);
                    kPal[s][k] = (uint8_t)(b + (f - b) * kT[s] + 0.5f);
                }
            kShade = kPal;
            // "Game.com Green": its own five shades, not a straight blend.
            if (strcmp(kScreenPalettes[g_screen_palette].name, "Game.com Green") == 0) {
                static const uint8_t kGreen[5][3] = {{0xdf, 0xff, 0x8f}, {0x8f, 0xcf, 0x8f}, {0x6f, 0x8f, 0x4f}, {0x0f, 0x4f, 0x2f}, {0x00, 0x00, 0x00}};
                kShade = kGreen;
            }
        }
        static GLuint s_tex = 0;
        static uint8_t s_rgba[GameCom::kW * GameCom::kH * 4];
        const uint8_t* sh = gc->shades();
        // LCD Trails (g_tgc_ghost): the real LCD is slow, a pixel takes
        // several frames to change - each frame moves only part of the way
        // to the new shade.
        static float s_acc[GameCom::kW * GameCom::kH * 3];
        static bool s_acc_valid = false;
        const float keep = g_tgc_ghost == 2 ? 0.72f : g_tgc_ghost == 1 ? 0.5f : 0.0f;
        if (keep == 0.0f) s_acc_valid = false;
        for (int i = 0; i < GameCom::kW * GameCom::kH; i++) {
            const uint8_t* c = kShade[sh[i] <= 4 ? sh[i] : 4];
            for (int k = 0; k < 3; k++) {
                float v = (float)c[k];
                if (keep > 0.0f) {
                    v = s_acc_valid ? s_acc[i * 3 + k] * keep + v * (1.0f - keep) : v;
                    s_acc[i * 3 + k] = v;
                }
                s_rgba[i * 4 + k] = (uint8_t)(v + 0.5f);
            }
            s_rgba[i * 4 + 3] = 255;
        }
        if (keep > 0.0f) s_acc_valid = true;
        glActiveTexture(GL_TEXTURE0);
        if (s_tex == 0) {
            glGenTextures(1, &s_tex);
            glBindTexture(GL_TEXTURE_2D, s_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GameCom::kW, GameCom::kH, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_rgba);
        } else {
            glBindTexture(GL_TEXTURE_2D, s_tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GameCom::kW, GameCom::kH, GL_RGBA, GL_UNSIGNED_BYTE, s_rgba);
        }
        const uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        float gx = 0.0f, gy = 0.0f;
        get_screen_base_global(0, gx, gy);
        if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
        glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        std::vector<RenderVertex> v;
        append_quad_ndc_uv_canvas(v, contentW, contentH, to_local_x(gx), to_local_y(gy),
                                  (float)g_segment_info[4] / (float)scale, (float)g_segment_info[5] / (float)scale, 0.0f, 1.0f, 1.0f, 0.0f);  // row 0 at the top, not mirrored (pack art is stored mirrored, this is not)
        draw_vertices(s_tex, v);
    }

    // Entex Adventure Vision (refs "EAV..."): 40 red LEDs and a spinning
    // mirror - the core keeps the picture (source/BrickCores/AdvVision.h,
    // 480x40, 0/1), drawn here over game screen 0. Each LED row is three
    // texture rows, the last one dark, so the rows read as separate LEDs.
    if (g_game && g_game->ref.compare(0, 3, "EAV") == 0 && g_cpu) {
        AdvVision* av = static_cast<AdvVision*>(g_cpu.get());  // core "I8048" (only EAV games use it)
        constexpr int kRows = 3;
        static GLuint s_tex = 0;
        static uint8_t s_rgba[AdvVision::kW * AdvVision::kH * kRows * 4];
        const uint8_t* pic = av->picture();
        for (int y = 0; y < AdvVision::kH * kRows; y++) {
            const bool gap = g_eav_screen == 0 && (y % kRows) == kRows - 1;  // Settings > Adventure Vision Screen
            const uint8_t* row = pic + (y / kRows) * AdvVision::kW;
            uint8_t* o = s_rgba + (size_t)y * AdvVision::kW * 4;
            for (int x = 0; x < AdvVision::kW; x++, o += 4) {
                const bool on = row[x] != 0;
                o[0] = on ? (gap ? 90 : 255) : (gap ? 6 : 22);
                o[1] = on ? (gap ? 4 : 28) : 2;
                o[2] = on ? (gap ? 4 : 20) : 2;
                o[3] = 255;
            }
        }
        const int tw = AdvVision::kW, th = AdvVision::kH * kRows;
        glActiveTexture(GL_TEXTURE0);
        if (s_tex == 0) {
            glGenTextures(1, &s_tex);
            glBindTexture(GL_TEXTURE_2D, s_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_rgba);
        } else {
            glBindTexture(GL_TEXTURE_2D, s_tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, s_rgba);
        }
        const uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        float gx = 0.0f, gy = 0.0f;
        get_screen_base_global(0, gx, gy);
        if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
        glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        std::vector<RenderVertex> v;
        append_quad_ndc_uv_canvas(v, contentW, contentH, to_local_x(gx), to_local_y(gy),
                                  (float)g_segment_info[4] / (float)scale, (float)g_segment_info[5] / (float)scale, 0.0f, 1.0f, 1.0f, 0.0f);
        draw_vertices(s_tex, v);
    }

    // Tamagotchi TamaTown Tama-Go / Friends (refs "TMG..."): the core lays
    // out the icon rows and the dot matrix as ink (source/BrickCores/
    // Tamago.h), coloured here from the bare LCD to its darkest shade - the
    // port's default LCD colours, or the chosen Screen Palette - over game
    // screen 0. Settings > Gotchi Figure is applied every frame.
    if (g_game && g_game->ref.compare(0, 3, "TMG") == 0 && g_cpu) {
        Tamago* tm = static_cast<Tamago*>(g_cpu.get());  // core "TAMAGO" (only TMG games use it)
        tm->set_figure(g_tmg_figure, tmg_contacts_of(g_tmg_figure));
        uint32_t bgc = 0xA6B398u, inkc = 0x1E261Cu;
        if (g_screen_palette > 0 && g_screen_palette < kNumScreenPalettes) {
            bgc = kScreenPalettes[g_screen_palette].bg;
            inkc = kScreenPalettes[g_screen_palette].seg;
        }
        constexpr int tw = Tamago::kPicW, th = Tamago::kPicH;
        static GLuint s_tex = 0;
        static uint8_t s_rgba[tw * th * 4];
        const uint8_t* ink = tm->ink();
        uint8_t lut[256][3];
        for (int i = 0; i < 256; i++)
            for (int k = 0; k < 3; k++) {
                const int b = (int)((bgc >> (16 - 8 * k)) & 0xFF), f = (int)((inkc >> (16 - 8 * k)) & 0xFF);
                lut[i][k] = (uint8_t)(b + (f - b) * i / 255);
            }
        for (int i = 0; i < tw * th; i++) {
            const uint8_t* c = lut[ink[i]];
            s_rgba[i * 4 + 0] = c[0]; s_rgba[i * 4 + 1] = c[1]; s_rgba[i * 4 + 2] = c[2]; s_rgba[i * 4 + 3] = 255;
        }
        glActiveTexture(GL_TEXTURE0);
        if (s_tex == 0) {
            glGenTextures(1, &s_tex);
            glBindTexture(GL_TEXTURE_2D, s_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, s_rgba);
        } else {
            glBindTexture(GL_TEXTURE_2D, s_tex);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, s_rgba);
        }
        const uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        float gx = 0.0f, gy = 0.0f;
        get_screen_base_global(0, gx, gy);
        if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
        glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        std::vector<RenderVertex> v;
        append_quad_ndc_uv_canvas(v, contentW, contentH, to_local_x(gx), to_local_y(gy),
                                  (float)g_segment_info[4] / (float)scale, (float)g_segment_info[5] / (float)scale, 0.0f, 1.0f, 1.0f, 0.0f);
        draw_vertices(s_tex, v);
    }

    // Hartung Game Master (refs "HGM..."): Settings > Game Master Speed.
    if (g_game && g_game->ref.compare(0, 3, "HGM") == 0 && g_cpu)
        static_cast<BrickCore*>(g_cpu.get())->set_speed_percent(g_hgm_speed ? 200 : 100);  // core "UPD78C11"

    // Label/text overlay, drawn AFTER the segments at a fixed alpha - see
    // GW_rom::background_info_count's comment and overlay_data_file() in
    // CONVERT_ROM/convert_switch.py. Mirrors MAME's own compositing order
    // for games built this way (background -> <screen blend="multiply"> ->
    // a translucent label layer on top of everything), so labels stay
    // legible over a darkened "on" cell instead of only existing in the
    // static backdrop where a lit segment would otherwise erase them.
    // Presence is entirely data-driven: background_info simply has more
    // than the normal number of elements for a game with an overlay
    // (nothing else validates its length), so every other game's
    // rendering is untouched.
    if (draw_art && g_gl.tex_overlay != 0 && g_game && g_game->background_info &&
        g_gl.tex_overlay_w > 0 && g_gl.tex_overlay_h > 0) {
        const uint16_t* bi = g_game->background_info;
        const size_t base_len = (size_t)2 + (size_t)4 * g_nb_screen + 3;
        if (g_game->background_info_count >= base_len + (size_t)2 + (size_t)4 * g_nb_screen) {
            const uint16_t* ov = bi + base_len;
            const float ovTexW = (float)ov[0];
            const float ovTexH = (float)ov[1];
            const uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;

            std::vector<RenderVertex> ov_verts;
            ov_verts.reserve(g_nb_screen * 6);

            for (uint8_t screen = 0; screen < g_nb_screen; screen++) {
                uint16_t id = (uint16_t)(2 + screen * 4);
                float u = (float)ov[id + 0];
                float v = (float)ov[id + 1];
                float w = (float)ov[id + 2];
                float h = (float)ov[id + 3];

                float u0, v0, u1, v1;
                calc_uv_rect(ovTexW, ovTexH, u, v, w, h, u0, v0, u1, v1);

                float dw = (float)g_segment_info[4 + screen * 2] / (float)scale;
                float dh = (float)g_segment_info[5 + screen * 2] / (float)scale;

                float gx = 0.0f, gy = 0.0f;
                get_screen_base_global(screen, gx, gy);
                append_quad_ndc_uv_canvas(ov_verts, contentW, contentH, to_local_x(gx), to_local_y(gy), dw, dh, u0, v0, u1, v1);
            }

            if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
            if (g_gl.uMultiplyDarken >= 0) glUniform1f(g_gl.uMultiplyDarken, 0.0f);
            // MAME's own default.lay: <element ref="TEXT-Top"><color alpha="0.7" />,
            // unless the pack carries an explicit alpha (percent) right after the
            // overlay rects - BrickEmuPy faces set 100, their overlay art having
            // its own transparency (convert_switch.py's 'overlay_alpha').
            float overlay_alpha = 0.7f;
            const size_t alpha_at = base_len + (size_t)2 + (size_t)4 * g_nb_screen;
            if (g_game->background_info_count > alpha_at) {
                overlay_alpha = std::min(bi[alpha_at], (uint16_t)100) / 100.0f;
            }
            glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, overlay_alpha);
            draw_vertices(g_gl.tex_overlay, ov_verts);
        }
    }

    // Screen filter over the game screens (not the console art below).
    // Not on the Game Pocket Computer: its dots are drawn on whole screen
    // pixels with their own gaps (above) - a filter's lines on top fall
    // across them at a different spacing and break up Solid's blocks.
    if (!(g_game && is_dot_matrix_game()))
        draw_screen_filter(contentW, contentH, (sy * (float)effViewportH) / contentH);

    // Console overlay. Clamshell (split-panel) games have no console strip -
    // see rebuild_layout_from_game(), which only sizes g_bottom_canvas_* from
    // console_info in the non-split branch - so g_hide_console_art can never
    // affect them; that same function already skips reserving canvas space
    // for this strip when the setting is on, this just skips drawing it too.
    if (!g_split_two_screens_to_panels && !g_hide_console_art && kRenderConsoleOverlay && g_gl.tex_console != 0 && g_game && g_game->console_info && g_gl.tex_console_w > 0 && g_gl.tex_console_h > 0) {
        if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
        glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);

        const uint16_t* ci = g_game->console_info;
        const float texW = (float)g_gl.tex_console_w;
        const float texH = (float)g_gl.tex_console_h;
        const float u = (float)ci[2], v = (float)ci[3], w = (float)ci[4], h = (float)ci[5];

        float u0, v0, u1, v1;
        calc_uv_rect(texW, texH, u, v, w, h, u0, v0, u1, v1);

        const uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        const float dst_w = w / (float)scale;
        const float dst_h = h / (float)scale;

        // Game Pocket Computer: its console photo sits on white, so the
        // whole strip under the screen is white, edge to edge.
        if ((is_dot_matrix_game() || g_game->ref.compare(0, 3, "TGC") == 0 || g_game->ref.compare(0, 3, "EAV") == 0 || g_game->ref.compare(0, 3, "TMG") == 0) && g_gl.tex_white != 0) {
            const float sy0 = to_local_y(g_bottom_off_y);
            std::vector<RenderVertex> white;
            append_quad_ndc_uv_canvas(white, contentW, contentH, 0.0f, sy0, contentW, contentH - sy0, 0, 0, 1, 1);
            draw_vertices(g_gl.tex_white, white);
        }
        std::vector<RenderVertex> cs_verts;
        append_quad_ndc_uv_canvas(cs_verts, contentW, contentH, to_local_x(g_bottom_off_x), to_local_y(g_bottom_off_y), dst_w, dst_h, u0, v0, u1, v1);
        draw_vertices(g_gl.tex_console, cs_verts);
    }

    glBindVertexArray(0);
}

void switch_screen_shutdown() {
    if (g_gl.tex_segments) glDeleteTextures(1, &g_gl.tex_segments);
    if (g_gl.tex_background) glDeleteTextures(1, &g_gl.tex_background);
    if (g_gl.tex_console) glDeleteTextures(1, &g_gl.tex_console);
    if (g_gl.tex_overlay) glDeleteTextures(1, &g_gl.tex_overlay);
    if (g_gl.tex_white) glDeleteTextures(1, &g_gl.tex_white);
    if (g_gl.vbo) glDeleteBuffers(1, &g_gl.vbo);
    if (g_gl.vao) glDeleteVertexArrays(1, &g_gl.vao);
    if (g_gl.program) glDeleteProgram(g_gl.program);
    g_gl = GlResources{};
}

