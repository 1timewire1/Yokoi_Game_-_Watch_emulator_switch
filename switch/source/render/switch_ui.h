#pragma once

#include <cstdint>
#include <string>

// Small immediate-mode 2D helper for the menu (switch_menu.cpp): filled
// rects, textured quads, and text, all in absolute pixel space (top-left
// origin, matching Hekate-Toolbox's SCREEN_WIDTH/SCREEN_HEIGHT convention)
// rather than switch_screen.cpp's letterboxed "canvas" space.
//
// Text is rendered with FreeType against Switch's own shared system font
// (plGetSharedFontByType) - not a bundled bitmap font - so it renders
// correctly regardless of what characters a game/manufacturer name needs.
//
// Deliberately owns its own GL program/VAO/texture, separate from
// switch_screen.cpp's - the in-game renderer is proven working on hardware
// and this keeps the menu from being able to disturb its GL state.

void switch_ui_init();
void switch_ui_shutdown();

// Call once per frame before any switch_ui_draw_* calls.
void switch_ui_begin_frame(int viewport_w, int viewport_h);

void switch_ui_draw_rect(float x, float y, float w, float h, float r, float g, float b, float a);

// Blits a sub-rect (src_x/src_y/src_w/src_h, in source pixel coordinates) of
// an already-uploaded GL texture into a pixel-space destination rect,
// preserving aspect is the caller's job - this just stretches src into dst.
// tex_w/tex_h are the texture's actual decoded pixel dimensions (needed to
// normalize the crop into UV space). Applies the same X-mirror correction
// switch_screen.cpp uses for CONVERT_ROM-pipeline assets (segments/
// background/console art) - this is only meant for those, not arbitrary textures.
void switch_ui_draw_texture(uint32_t gl_tex, float x, float y, float w, float h,
                             float tex_w, float tex_h, float src_x, float src_y, float src_w, float src_h);

// Draws the *whole* texture into a pixel-space destination rect, stretching
// to fit (aspect preservation is the caller's job) - unlike
// switch_ui_draw_texture() above, this does NOT apply the CONVERT_ROM-
// pipeline's X-mirror correction, so it's the right one for a generic UI
// texture (e.g. a header icon) that was never run through that pipeline.
void switch_ui_draw_texture_plain(uint32_t gl_tex, float x, float y, float w, float h);

// Draws `text` in pixel space, (x,y) = top-left of the line. `pixel_size` is
// the requested glyph height in pixels (glyphs are individually cached per
// exact pixel size + bold combination the first time they're drawn).
// `bold` is synthetic (FT_GlyphSlot_Embolden) - the shared system font only
// has one weight, there's no real bold face to select instead.
void switch_ui_draw_text(float x, float y, float pixel_size, float r, float g, float b, float a, const std::string& text, bool bold = false);
float switch_ui_text_width(const std::string& text, float pixel_size, bool bold = false);

// Nintendo button-prompt glyphs (A/B/X/Y/L/R/dpad/+/-), rendered from
// Switch's PlSharedFontType_NintendoExt shared font - the same one
// Hekate-Toolbox itself uses for its own control hints (see
// override_key.cpp's HidNpadButton_* -> "" etc. mapping there, which
// this mirrors). A separate font/face from switch_ui_draw_text's - this one
// only has these icons, not any actual text.
enum class SwitchButtonIcon {
    A, B, X, Y, L, R, ZL, ZR, DpadUp, DpadDown, DpadLeft, DpadRight, Plus, Minus,
};

// Draws one icon, (x,y) = top-left, matching switch_ui_draw_text's convention.
void switch_ui_draw_button_icon(float x, float y, float pixel_size, float r, float g, float b, float a, SwitchButtonIcon icon);
float switch_ui_button_icon_width(float pixel_size, SwitchButtonIcon icon);
