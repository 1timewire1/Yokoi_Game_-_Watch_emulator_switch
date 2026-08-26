#pragma once

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

void switch_screen_shutdown();
