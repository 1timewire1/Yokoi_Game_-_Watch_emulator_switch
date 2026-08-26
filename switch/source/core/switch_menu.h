#pragma once

#include <cstdint>

// Game/manufacturer select menu, styled after Hekate-Toolbox (dark theme,
// centered card, top/bottom bars with control hints) but with this project's
// own navigation semantics, ported from the menu-navigation half of
// android/app/src/main/cpp/jni/yokoi_jni.cpp's render_frame():
//   D-pad Left/Right -> next/prev game within the current manufacturer
//   D-pad Up/Down    -> next/prev manufacturer (restores its last-played game)
//   A                -> start the selected game
//   L+R+Plus (while in-game) -> pause and return to the menu (game stays loaded)
//
// B is deliberately NOT a back-out-to-menu shortcut: several G&W games use
// it as a real gameplay input (see switch_menu_update()'s in-game branch).

void switch_menu_init();
void switch_menu_shutdown();

bool switch_menu_is_active();

// Pauses and returns to the menu without unloading the current game. No-op
// if already in the menu. Reached via the in-game settings screen's "Return
// to Main Menu" row now, rather than directly from the L+R+Plus combo - see
// switch_menu_open_ingame_settings() below.
void switch_menu_return_to_menu();

// Opens the settings screen without unloading or unpausing the current game -
// called from main.cpp on the L+R+Plus combo *during gameplay* (from the game
// list, that combo still quits to hbmenu instead). Same settings screen the
// game list's Plus button opens, minus the Console Art row (it only takes
// effect on a game's next load, so it'd be a dead control mid-session) plus
// a "Return to Main Menu" row in its place. B backs out and resumes the game
// in place instead of returning to the game list.
void switch_menu_open_ingame_settings();

// kDown = buttons newly pressed this frame, kHeld = buttons currently held
// (both HidNpadButton_* bits, as returned by switch_input_poll()/switch_input_held()).
// Handles menu navigation; does nothing during gameplay (see switch_menu_return_to_menu()).
void switch_menu_update(uint64_t kDown, uint64_t kHeld);

void switch_menu_render(int viewport_w, int viewport_h);
