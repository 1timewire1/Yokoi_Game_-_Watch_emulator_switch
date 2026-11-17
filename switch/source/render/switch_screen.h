#pragma once

#include <cstdint>
#include <vector>

// Ported from the render_frame() function in android/app/src/main/cpp/jni/yokoi_jni.cpp,
// trimmed to what a no-menu, single-screen, single-threaded v1 needs:
//  - always renders in Android's "combined" panel mode (both G&W screens +
//    console stacked into one canvas - this is what a single Switch screen
//    needs anyway, so there's no dual-panel splitting to do)
//  - no menu navigation state machine, no touch, no menu UI texture
//  - segment/background/console PNGs are decoded on-device via stb_image
//    (Android instead offloads PNG decode to Java's BitmapFactory - there's
//    no equivalent shortcut in a Switch homebrew binary)

void switch_screen_init(int viewport_w, int viewport_h);

// Reloads segment/background/console textures from the currently loaded
// g_game via gw_pack::get_file_bytes(). Call after switch_load_game_by_index_and_init().
void switch_screen_load_textures();

void switch_screen_render();

// Pause menu > Screen > Pixel Style (switch_settings.h' g_pixel_style) only
// applies to the loaded game when this is true: the Brick Games whose 10x20
// dot grid has filled dots in the art (96 in 1 E-23 Plus Mark II and
// E-33 2 in 1; the E-88's art already has the frame and centre). The others either aren't grid games or already draw a frame and a
// centre square.
bool switch_screen_pixel_style_applies();

// Share of the screen width the game is fitted into, from the left (1 =
// all of it). The on-screen keypad (input/onscreen_keypad.h) takes the
// rest. Only applied at Game Rotation 0.
void switch_screen_set_area(float width_frac);

// Local link (core/link_session.h): the game is fitted into a slice of the
// screen width starting at offset_frac (0 = left edge), and the render
// doesn't clear the screen first when clear is false - so a second game can
// be drawn next to the first. switch_screen_set_area() resets both.
void switch_screen_set_panel(float width_frac, float offset_frac, bool clear);

// What switch_screen.cpp keeps per loaded game (its textures and their
// generation, and each segment's ghosting level) - swapped with the second
// game's around its render, together with the core/ globals it reads.
struct SwitchScreenGameState {
    unsigned tex_segments = 0, tex_background = 0, tex_console = 0, tex_overlay = 0;
    int tex_segments_w = 0, tex_segments_h = 0, tex_background_w = 0, tex_background_h = 0;
    int tex_console_w = 0, tex_console_h = 0, tex_overlay_w = 0, tex_overlay_h = 0;
    uint32_t loaded_generation = 0xFFFFFFFFu;
    std::vector<float> seg_level;
};
void switch_screen_swap_game_state(SwitchScreenGameState& other);
void switch_screen_free_game_state(SwitchScreenGameState& s);  // deletes its textures

void switch_screen_shutdown();
