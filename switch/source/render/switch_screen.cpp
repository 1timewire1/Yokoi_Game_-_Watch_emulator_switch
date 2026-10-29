#include "switch_screen.h"

#include <algorithm>
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

namespace {

// Same verified-correct UV convention as Android (see yokoi_jni.cpp):
// atlas is mirrored on X and needs a V flip for OpenGL's bottom-left origin.
constexpr bool kUvFlipX = true;
constexpr bool kUvFlipV = true;
constexpr bool kRenderConsoleOverlay = true;

GlResources g_gl;
uint32_t g_loaded_texture_generation = 0xFFFFFFFFu;

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

void switch_screen_init(int viewport_w, int viewport_h) {
    switch_gl_init_resources(g_gl);
    g_gl.viewport_w = viewport_w;
    g_gl.viewport_h = viewport_h;
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

void switch_screen_render() {
    glViewport(0, 0, g_gl.viewport_w, g_gl.viewport_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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
    const float viewAspect = (effViewportW > 0 && effViewportH > 0)
        ? (float)effViewportW / (float)effViewportH
        : 1.0f;

    float sx = 1.0f, sy = 1.0f;
    if (viewAspect > contentAspect) {
        sx = contentAspect / viewAspect;
    } else {
        sy = viewAspect / contentAspect;
    }
    glUniform2f(g_gl.uScale, sx, sy);
    if (g_gl.uOffset >= 0) {
        glUniform2f(g_gl.uOffset, 0.0f, 0.0f);
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
    const uint32_t bg = is_mask_game ? kMaskFondRgb : g_settings.background_color;
    const float br = ((bg >> 16) & 0xFF) / 255.0f;
    const float bgc = ((bg >> 8) & 0xFF) / 255.0f;
    const float bb = (bg & 0xFF) / 255.0f;

    // Base fill.
    if (g_gl.tex_white != 0) {
        glUniform4f(g_gl.uMul, br, bgc, bb, 1.0f);
        std::vector<RenderVertex> fill_verts;
        append_quad_ndc_uv_canvas(fill_verts, contentW, contentH, 0.0f, 0.0f, contentW, contentH, 0.0f, 0.0f, 1.0f, 1.0f);
        draw_vertices(g_gl.tex_white, fill_verts);
    }

    // Printed art (Background + Overlay) unless Background Art is Hidden -
    // masked games always keep it (see g_hide_background_art).
    const bool draw_art = !g_hide_background_art || is_mask_game;

    // Background layer (per-screen rect from GW_rom::background_info).
    if (draw_art && g_gl.tex_background != 0 && g_game && g_game->background_info && g_gl.tex_background_w > 0 && g_gl.tex_background_h > 0) {
        const uint16_t* bi = g_game->background_info;
        glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
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

        static constexpr uint32_t kSegmentColorRgb[5] = {
            0x080908u, 0x9992e7u, 0x58b9a0u, 0xff677cu, 0x3db8e4u,
        };

        std::vector<RenderVertex> seg_verts;
        std::vector<RenderVertex> seg_color_verts[5];
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

        for (size_t si = 0; si < seg_count; si++) {
            const Segment& seg = g_segments[si];
            const bool seg_on = (si < g_seg_on.size()) ? (g_seg_on[si] != 0) : false;

            float base_gx = 0.0f, base_gy = 0.0f;
            get_screen_base_global(seg.screen, base_gx, base_gy);

            float sx2 = (float)seg.pos_scr[0] / (float)scale + base_gx;
            float sy2 = (float)seg.pos_scr[1] / (float)scale + base_gy;
            float sw = (float)seg.size_tex[0] / (float)scale;
            float sh = (float)seg.size_tex[1] / (float)scale;

            float sx_local = to_local_x(sx2);
            float sy_local = to_local_y(sy2);

            float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
            if (g_gl.tex_segments != 0) {
                calc_uv_rect(texW, texH, (float)seg.pos_tex[0], (float)seg.pos_tex[1],
                             (float)seg.size_tex[0], (float)seg.size_tex[1], u0, v0, u1, v1);
            }

            if (!is_mask) {
                append_quad_ndc_uv_canvas(seg_mark_verts, contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                if (seg_on) {
                    append_quad_ndc_uv_canvas(seg_shadow_verts, contentW, contentH, sx_local + 2.0f, sy_local + 2.0f, sw, sh, u0, v0, u1, v1);
                }
            }

            if (seg_on) {
                if (!is_mask && seg.color_index > 0 && seg.color_index < 5) {
                    has_colored_segments = true;
                    append_quad_ndc_uv_canvas(seg_color_verts[seg.color_index], contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                } else {
                    append_quad_ndc_uv_canvas(seg_verts, contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                }
            }
        }

        // Lit-segment colour - the Segment Color setting, or (Auto) a tint
        // of the background colour; see effective_segment_color().
        const uint32_t seg_rgb = effective_segment_color(bg);
        const float seg_r = ((seg_rgb >> 16) & 0xFF) / 255.0f;
        const float seg_g = ((seg_rgb >> 8) & 0xFF) / 255.0f;
        const float seg_b = (seg_rgb & 0xFF) / 255.0f;
        const GLuint seg_tex = (g_gl.tex_segments != 0) ? g_gl.tex_segments : g_gl.tex_white;

        if (!is_mask) {
            const float mark_a = (float)g_settings.segment_marking_alpha / 255.0f;
            if (mark_a > 0.0f && !seg_mark_verts.empty()) {
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
            for (uint32_t ci = 1; ci < 5; ci++) {
                if (seg_color_verts[ci].empty()) continue;
                const uint32_t rgb = kSegmentColorRgb[ci];
                glUniform4f(g_gl.uMul, ((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, 1.0f);
                draw_vertices(seg_tex, seg_color_verts[ci]);
            }
        }
    }

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
