#include "switch_ui.h"

#include <cstdio>
#include <unordered_map>
#include <utility>
#include <vector>

#include <switch.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H

#include "switch_gl.h"

#include "std/debug_log.h"

#include "core/switch_settings.h"

namespace {

// V-flip is needed for any texture decoded/rasterized top-row-first and
// uploaded as-is (GL's V=0 samples the first row of the buffer, i.e. the
// bottom in GL's convention) - applies uniformly to stb_image-decoded PNGs
// and to our own FreeType-rasterized glyph bitmaps alike.
//
// X-flip corrects for segment/background/console PNGs being exported
// pre-mirrored by CONVERT_ROM's game-asset pipeline (see switch_screen.cpp);
// it's applied to switch_ui_draw_texture (menu thumbnails, which come from
// that same pipeline) but never to glyphs, which we rasterize ourselves.
constexpr bool kUvFlipV = true;

GlResources g_gl;
int g_viewport_w = 1280;
int g_viewport_h = 720;

FT_Library g_ft_library = nullptr;
FT_Face g_ft_face = nullptr;
// Nintendo's "extended" shared font - contains only button/icon glyphs (A/B/
// X/Y/L/R/dpad/+/-/...), no actual text. Same font Hekate-Toolbox uses for
// its own control hints (their gui.hpp calls it "fontIcons").
FT_Face g_ft_face_icons = nullptr;

struct GlyphEntry {
    GLuint tex = 0;
    int width = 0, height = 0;
    int bearing_x = 0, bearing_y = 0;
    int advance = 0;
};

// Keyed by (face selector, pixel size, codepoint) so the text face and icon
// face never collide even though their codepoint ranges shouldn't overlap.
std::unordered_map<uint64_t, GlyphEntry> g_glyph_cache;

// Codepoints in PlSharedFontType_NintendoExt for each button icon, taken
// from Hekate-Toolbox's source/utils/override_key.cpp (its HidNpadButton_*
// -> icon-string mapping) since Nintendo doesn't publish these anywhere else.
uint32_t icon_codepoint(SwitchButtonIcon icon) {
    switch (icon) {
        case SwitchButtonIcon::A:         return 0xE0E0;
        case SwitchButtonIcon::B:         return 0xE0E1;
        case SwitchButtonIcon::X:         return 0xE0E2;
        case SwitchButtonIcon::Y:         return 0xE0E3;
        case SwitchButtonIcon::L:         return 0xE0E4;
        case SwitchButtonIcon::R:         return 0xE0E5;
        case SwitchButtonIcon::ZL:        return 0xE0E6;
        case SwitchButtonIcon::ZR:        return 0xE0E7;
        case SwitchButtonIcon::DpadUp:    return 0xE0EB;
        case SwitchButtonIcon::DpadDown:  return 0xE0EC;
        case SwitchButtonIcon::DpadLeft:  return 0xE0ED;
        case SwitchButtonIcon::DpadRight: return 0xE0EE;
        case SwitchButtonIcon::Plus:      return 0xE0EF;
        case SwitchButtonIcon::Minus:     return 0xE0F0;
    }
    return 0;
}

void calc_uv_rect(float texW, float texH,
                   float u, float v, float w, float h,
                   bool flipX,
                   float& out_u0, float& out_v0, float& out_u1, float& out_v1) {
    if (texW <= 0.0f) texW = 1.0f;
    if (texH <= 0.0f) texH = 1.0f;

    out_u0 = u / texW;
    out_u1 = (u + w) / texW;
    if (flipX) {
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

void append_quad_px(std::vector<RenderVertex>& out, float x, float y, float w, float h,
                     float u0, float v0, float u1, float v1) {
    const float cw = g_viewport_w > 0 ? (float)g_viewport_w : 1.0f;
    const float ch = g_viewport_h > 0 ? (float)g_viewport_h : 1.0f;

    const float left = (x / cw) * 2.0f - 1.0f;
    const float right = ((x + w) / cw) * 2.0f - 1.0f;
    const float top = 1.0f - (y / ch) * 2.0f;
    const float bottom = 1.0f - ((y + h) / ch) * 2.0f;

    out.push_back({left, bottom, u0, v0});
    out.push_back({right, bottom, u1, v0});
    out.push_back({right, top, u1, v1});

    out.push_back({right, top, u1, v1});
    out.push_back({left, top, u0, v1});
    out.push_back({left, bottom, u0, v0});
}

void draw_quad(GLuint tex, const std::vector<RenderVertex>& verts) {
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

// codepoint needs 21 bits (Unicode max U+10FFFF); pixel_size gets a
// generous 18 bits above that; is_icon_face/bold are single flag bits on top.
uint64_t glyph_key(bool is_icon_face, bool bold, uint32_t codepoint, int pixel_size) {
    return ((uint64_t)(is_icon_face ? 1u : 0u) << 40)
         | ((uint64_t)(bold ? 1u : 0u) << 39)
         | ((uint64_t)(uint32_t)pixel_size << 21)
         | (uint64_t)(codepoint & 0x1FFFFFu);
}

// Rasterizes (and caches) one glyph at one exact pixel size, from either the
// text face or the icon face, optionally synthetically emboldened (the
// shared system font only has one real weight). Returns nullptr only if that
// face isn't available; a glyph with no visible bitmap (e.g. space) still
// returns a valid entry with tex=0 and a real advance.
const GlyphEntry* get_or_rasterize_glyph(FT_Face face, bool is_icon_face, uint32_t codepoint, int pixel_size, bool bold = false) {
    if (!face) {
        return nullptr;
    }

    const uint64_t key = glyph_key(is_icon_face, bold, codepoint, pixel_size);
    auto it = g_glyph_cache.find(key);
    if (it != g_glyph_cache.end()) {
        return &it->second;
    }

    FT_Set_Pixel_Sizes(face, 0, (FT_UInt)pixel_size);
    // Split load/embolden/render instead of the one-shot FT_LOAD_RENDER so
    // bold can embolden the outline before it gets rasterized.
    if (FT_Load_Char(face, codepoint, FT_LOAD_DEFAULT)) {
        return nullptr;
    }
    const FT_GlyphSlot slot = face->glyph;
    if (bold && slot->format == FT_GLYPH_FORMAT_OUTLINE) {
        FT_GlyphSlot_Embolden(slot);
    }
    if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL)) {
        return nullptr;
    }

    const FT_Bitmap& bmp = slot->bitmap;

    GlyphEntry entry;
    entry.width = (int)bmp.width;
    entry.height = (int)bmp.rows;
    entry.bearing_x = slot->bitmap_left;
    entry.bearing_y = slot->bitmap_top;
    entry.advance = (int)(slot->advance.x >> 6);

    if (entry.width > 0 && entry.height > 0 && bmp.pixel_mode == FT_PIXEL_MODE_GRAY) {
        std::vector<uint8_t> rgba((size_t)entry.width * entry.height * 4);
        for (int y = 0; y < entry.height; y++) {
            const uint8_t* src_row = bmp.buffer + (size_t)y * bmp.pitch;
            for (int x = 0; x < entry.width; x++) {
                uint8_t* dst = &rgba[((size_t)y * entry.width + x) * 4];
                // RGB=0, not 255: the shader's alpha-only tint mode computes
                // max(t.a, max(t.r,t.g,t.b)) as its coverage mask (designed
                // for segment art where shape can live in either channel).
                // RGB=255 made that max always 1.0 regardless of the real
                // alpha, rendering every glyph as a solid filled box instead
                // of its actual antialiased shape.
                dst[0] = dst[1] = dst[2] = 0;
                dst[3] = src_row[x];
            }
        }

        glGenTextures(1, &entry.tex);
        glBindTexture(GL_TEXTURE_2D, entry.tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, entry.width, entry.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    }

    auto res = g_glyph_cache.emplace(key, entry);
    return &res.first->second;
}

} // namespace

void switch_ui_init() {
    switch_gl_init_resources(g_gl);

    Result rc = plInitialize(PlServiceType_User);
    if (R_FAILED(rc)) {
        YOKOI_LOG("switch_ui_init: plInitialize failed: 0x%x", (unsigned)rc);
        return;
    }

    PlFontData font;
    rc = plGetSharedFontByType(&font, PlSharedFontType_Standard);
    if (R_FAILED(rc)) {
        YOKOI_LOG("switch_ui_init: plGetSharedFontByType failed: 0x%x", (unsigned)rc);
        return;
    }

    if (FT_Init_FreeType(&g_ft_library)) {
        YOKOI_LOG("switch_ui_init: FT_Init_FreeType failed");
        return;
    }

    if (FT_New_Memory_Face(g_ft_library, (const FT_Byte*)font.address, (FT_Long)font.size, 0, &g_ft_face)) {
        YOKOI_LOG("switch_ui_init: FT_New_Memory_Face failed");
        FT_Done_FreeType(g_ft_library);
        g_ft_library = nullptr;
        return;
    }

    PlFontData icon_font;
    rc = plGetSharedFontByType(&icon_font, PlSharedFontType_NintendoExt);
    if (R_FAILED(rc)) {
        YOKOI_LOG("switch_ui_init: plGetSharedFontByType(NintendoExt) failed: 0x%x - button icons won't render", (unsigned)rc);
    } else if (FT_New_Memory_Face(g_ft_library, (const FT_Byte*)icon_font.address, (FT_Long)icon_font.size, 0, &g_ft_face_icons)) {
        YOKOI_LOG("switch_ui_init: FT_New_Memory_Face(icons) failed - button icons won't render");
        g_ft_face_icons = nullptr;
    }

    YOKOI_LOG("switch_ui_init: FreeType + shared system font ready (program=%u)", (unsigned)g_gl.program);
}

void switch_ui_shutdown() {
    for (auto& kv : g_glyph_cache) {
        if (kv.second.tex) {
            GLuint t = kv.second.tex;
            glDeleteTextures(1, &t);
        }
    }
    g_glyph_cache.clear();

    if (g_ft_face) {
        FT_Done_Face(g_ft_face);
        g_ft_face = nullptr;
    }
    if (g_ft_face_icons) {
        FT_Done_Face(g_ft_face_icons);
        g_ft_face_icons = nullptr;
    }
    if (g_ft_library) {
        FT_Done_FreeType(g_ft_library);
        g_ft_library = nullptr;
    }
    plExit();

    if (g_gl.tex_white) glDeleteTextures(1, &g_gl.tex_white);
    if (g_gl.vbo) glDeleteBuffers(1, &g_gl.vbo);
    if (g_gl.vao) glDeleteVertexArrays(1, &g_gl.vao);
    if (g_gl.program) glDeleteProgram(g_gl.program);
    g_gl = GlResources{};
}

// viewport_w/viewport_h here are the LOGICAL dimensions to lay pixel-space
// UI out against - the caller (switch_menu.cpp) already swaps them at 90/270
// before calling this, so append_quad_px()'s pixel->NDC normalization below
// matches whatever orientation the menu is currently laying itself out in.
// The physical glViewport() call stays on the true (always-landscape) size -
// that's the caller's job, not this function's.
void switch_ui_begin_frame(int viewport_w, int viewport_h) {
    g_viewport_w = viewport_w;
    g_viewport_h = viewport_h;

    if (g_gl.program == 0) {
        return;
    }

    glUseProgram(g_gl.program);
    glBindVertexArray(g_gl.vao);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform2f(g_gl.uScale, 1.0f, 1.0f);
    if (g_gl.uOffset >= 0) glUniform2f(g_gl.uOffset, 0.0f, 0.0f);
    if (g_gl.uRotCosSin >= 0) {
        // Uninitialized GLSL uniforms default to 0, which would collapse
        // every vertex to the origin - see switch_screen.cpp for the full
        // story of why this must always be explicitly set, never skipped.
        // Signs verified against hardware - see the matching block in
        // switch_screen.cpp for why they're flipped from the naive rotation-
        // matrix math. Keep both in sync if this ever needs revisiting.
        float rotCos = 1.0f, rotSin = 0.0f;
        if (g_screen_rotation == ScreenRotation::Deg90) {
            rotCos = 0.0f; rotSin = -1.0f;
        } else if (g_screen_rotation == ScreenRotation::Deg270) {
            rotCos = 0.0f; rotSin = 1.0f;
        }
        glUniform2f(g_gl.uRotCosSin, rotCos, rotSin);
    }
    if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
}

void switch_ui_draw_rect(float x, float y, float w, float h, float r, float g, float b, float a) {
    if (g_gl.program == 0 || g_gl.tex_white == 0) return;
    glUniform4f(g_gl.uMul, r, g, b, a);
    std::vector<RenderVertex> verts;
    append_quad_px(verts, x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f);
    draw_quad(g_gl.tex_white, verts);
}

void switch_ui_draw_polygon(const float* xy, int n, float r, float g, float b, float a) {
    if (g_gl.program == 0 || g_gl.tex_white == 0 || !xy || n < 3) return;
    const float cw = g_viewport_w > 0 ? (float)g_viewport_w : 1.0f;
    const float ch = g_viewport_h > 0 ? (float)g_viewport_h : 1.0f;
    auto ndc = [&](int i) {
        return RenderVertex{(xy[2 * i] / cw) * 2.0f - 1.0f, 1.0f - (xy[2 * i + 1] / ch) * 2.0f, 0.5f, 0.5f};
    };
    glUniform4f(g_gl.uMul, r, g, b, a);
    std::vector<RenderVertex> verts;
    verts.reserve((size_t)(n - 2) * 3);
    for (int i = 1; i + 1 < n; i++) {
        verts.push_back(ndc(0));
        verts.push_back(ndc(i));
        verts.push_back(ndc(i + 1));
    }
    draw_quad(g_gl.tex_white, verts);
}

void switch_ui_draw_texture(uint32_t gl_tex, float x, float y, float w, float h,
                             float tex_w, float tex_h, float src_x, float src_y, float src_w, float src_h) {
    if (g_gl.program == 0 || gl_tex == 0) return;
    glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
    std::vector<RenderVertex> verts;
    float u0, v0, u1, v1;
    calc_uv_rect(tex_w, tex_h, src_x, src_y, src_w, src_h, /*flipX=*/true, u0, v0, u1, v1);
    append_quad_px(verts, x, y, w, h, u0, v0, u1, v1);
    draw_quad((GLuint)gl_tex, verts);
}

void switch_ui_draw_texture_plain(uint32_t gl_tex, float x, float y, float w, float h) {
    if (g_gl.program == 0 || gl_tex == 0) return;
    glUniform4f(g_gl.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
    std::vector<RenderVertex> verts;
    float u0, v0, u1, v1;
    // Full-texture UVs (0,0)-(1,1); only the V-flip every stb_image-decoded
    // texture needs applies here (see calc_uv_rect/kUvFlipV above) - no
    // cropping, no X-flip.
    calc_uv_rect(1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, /*flipX=*/false, u0, v0, u1, v1);
    append_quad_px(verts, x, y, w, h, u0, v0, u1, v1);
    draw_quad((GLuint)gl_tex, verts);
}

void switch_ui_draw_text(float x, float y, float pixel_size, float r, float g, float b, float a, const std::string& text, bool bold) {
    if (g_gl.program == 0 || !g_ft_face || pixel_size <= 0.0f) return;

    const int px = (int)(pixel_size + 0.5f);
    FT_Set_Pixel_Sizes(g_ft_face, 0, (FT_UInt)px);
    // `y` is the top of the line; approximate the baseline from the face's
    // own ascender metric at this pixel size so different pixel sizes still
    // line up sensibly when top-aligned.
    const float baseline_y = y + (float)(g_ft_face->size->metrics.ascender >> 6);

    if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 1.0f);
    glUniform4f(g_gl.uMul, r, g, b, a);

    float cursor_x = x;
    for (unsigned char c : text) {
        const GlyphEntry* ge = get_or_rasterize_glyph(g_ft_face, false, (uint32_t)c, px, bold);
        if (!ge) {
            cursor_x += px * 0.5f;
            continue;
        }
        if (ge->tex != 0) {
            std::vector<RenderVertex> verts;
            float u0, v0, u1, v1;
            calc_uv_rect((float)ge->width, (float)ge->height, 0.0f, 0.0f, (float)ge->width, (float)ge->height,
                         /*flipX=*/false, u0, v0, u1, v1);
            const float gx = cursor_x + (float)ge->bearing_x;
            const float gy = baseline_y - (float)ge->bearing_y;
            append_quad_px(verts, gx, gy, (float)ge->width, (float)ge->height, u0, v0, u1, v1);
            draw_quad(ge->tex, verts);
        }
        cursor_x += (float)ge->advance;
    }

    if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
}

float switch_ui_text_width(const std::string& text, float pixel_size, bool bold) {
    if (!g_ft_face || pixel_size <= 0.0f) return 0.0f;
    const int px = (int)(pixel_size + 0.5f);

    float width = 0.0f;
    for (unsigned char c : text) {
        const GlyphEntry* ge = get_or_rasterize_glyph(g_ft_face, false, (uint32_t)c, px, bold);
        width += ge ? (float)ge->advance : (float)px * 0.5f;
    }
    return width;
}

void switch_ui_draw_button_icon(float x, float y, float pixel_size, float r, float g, float b, float a, SwitchButtonIcon icon) {
    if (g_gl.program == 0 || !g_ft_face_icons || pixel_size <= 0.0f) return;

    const int px = (int)(pixel_size + 0.5f);
    const GlyphEntry* ge = get_or_rasterize_glyph(g_ft_face_icons, true, icon_codepoint(icon), px);
    if (!ge || ge->tex == 0) return;

    FT_Set_Pixel_Sizes(g_ft_face_icons, 0, (FT_UInt)px);
    const float baseline_y = y + (float)(g_ft_face_icons->size->metrics.ascender >> 6);

    if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 1.0f);
    glUniform4f(g_gl.uMul, r, g, b, a);

    std::vector<RenderVertex> verts;
    float u0, v0, u1, v1;
    calc_uv_rect((float)ge->width, (float)ge->height, 0.0f, 0.0f, (float)ge->width, (float)ge->height,
                 /*flipX=*/false, u0, v0, u1, v1);
    const float gx = x + (float)ge->bearing_x;
    const float gy = baseline_y - (float)ge->bearing_y;
    append_quad_px(verts, gx, gy, (float)ge->width, (float)ge->height, u0, v0, u1, v1);
    draw_quad(ge->tex, verts);

    if (g_gl.uAlphaOnly >= 0) glUniform1f(g_gl.uAlphaOnly, 0.0f);
}

float switch_ui_button_icon_width(float pixel_size, SwitchButtonIcon icon) {
    if (!g_ft_face_icons || pixel_size <= 0.0f) return 0.0f;
    const int px = (int)(pixel_size + 0.5f);
    const GlyphEntry* ge = get_or_rasterize_glyph(g_ft_face_icons, true, icon_codepoint(icon), px);
    return ge ? (float)ge->advance : (float)px;
}
