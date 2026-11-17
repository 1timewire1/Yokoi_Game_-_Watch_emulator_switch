#pragma once

// On-screen keypad for the calculator-keypad games (Experimental Features,
// pause menu > Screen > On-Screen Keypad): Electronic Number Muncher,
// Uchitel' arifmetiki, and the Tronica calculator keypads of Space
// Adventure / Shuttle Voyage / Thief in Garden. The game is drawn on the
// left of the screen and the keypad on the right.
//   - Handheld: tap the keys on the touch screen.
//   - Any mode: R3 (L3 when R3 opens the pause menu) gives the keypad the
//     controller - D-pad / stick move, A presses, B or R3 again gives it
//     back to the game. While it has it, the game sees no buttons.
// The L/R button layers (modifier_layer_input.h) keep working as before.

class SM5XX;

// The current game has a keypad and it's switched on (and the game screen
// isn't rotated - the keypad is laid out for landscape only).
bool onscreen_keypad_visible();
// The loaded game is one of the keypad games (for showing the setting).
bool onscreen_keypad_game_has_one();
// How much of the screen width the game gets while the keypad shows.
float onscreen_keypad_game_area();

// Once per gameplay frame, after switch_input_poll(): touch, focus, cursor.
// While the keypad has the controller it clears the game's input.
void onscreen_keypad_update();
// After the game's own input code: presses the keys held on the keypad.
void onscreen_keypad_apply(SM5XX* cpu);
// Over the game screen (after switch_screen_render()).
void onscreen_keypad_render();
