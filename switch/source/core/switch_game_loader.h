#pragma once

#include <cstdint>

// Ported from android/app/src/main/cpp/core/yokoi_game_loader.cpp, trimmed to
// single-threaded (no g_cpu_mutex - nothing else touches g_cpu concurrently in
// the Switch build) and with __android_log_* swapped for printf.
void switch_load_game_by_index_and_init(uint8_t idx);

// Saves the currently-loaded game's state (if any) and fully unloads it -
// g_cpu/g_input are reset and g_game is cleared. This is what actually
// leaving a game for the list means (the in-game settings screen's "Return
// to Main Menu" row) - NOT the same as merely pausing to show that screen in
// the first place, which keeps the session alive for an instant B-to-resume
// (see switch_menu_open_ingame_settings()). No-op if nothing is loaded, so
// it's safe to call speculatively as a belt-and-suspenders guard too - e.g.
// right before quitting to hbmenu, in case a future change ever lets the
// game list be reached with something still loaded in the background.
void switch_unload_current_game();

// Prefers g_last_game_played_ref (switch_settings.h - the last game a save
// was actually written for) when it resolves to a real game; otherwise falls
// back to Android's last-selected-game-per-manufacturer heuristic, then
// "AC_01" if present, then game 0.
uint8_t switch_get_default_game_index();
