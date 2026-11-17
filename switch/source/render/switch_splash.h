#pragma once

#include <cstdint>

// Startup splash (Settings > Advanced > Splash Screen): the "YOKOI" logo
// printed on a brushed-metal handheld faceplate, and an LCD window where a
// little (original) character walks in step by step and juggles - drawn
// live with switch_ui's primitives, LCD-style (unlit segments faintly
// visible, lit ones leave a short ghosting trail), with piezo-style beeps
// timed to it (own voice on g_sound's driver). About 5.5 s, then it
// fades out and the game list takes over; + skips it at any time.
//
// switch_ui_init() must already have run (switch_menu_init() does it).

void switch_splash_start();
bool switch_splash_active();
// One frame: reads the skip button, draws, advances. Caller swaps buffers.
void switch_splash_frame(uint64_t kDown, int viewport_w, int viewport_h);
