#pragma once

// Per-game button remapping (pause menu > Controls) and the "Open Pause
// Menu" combo (Settings > Menu). Works one level below every game's own
// input code: switch_input_poll() runs the pad through
// control_remap_apply() while a game is being played, so everything that
// reads switch_input_held() / the CTL_* mask (SM5xx Virtual_Input, the
// BrickEmuPy/HMCS40 bindings, E0C6200 pins, LCDonald, R-Zone, the
// calculator keypad layers) sees the remapped buttons without knowing.
//
// The 16 remappable buttons are "logical" (what the game's input code
// expects) and "physical" (what the player presses) at once: a game's map
// says, for each logical button, which physical one drives it. It is always
// a permutation (changing one swaps it with whoever had that button), so no
// function is left without a button - unless the player leaves one
// unassigned (control_remap_unassign(): no button presses it, kNoButton).
// Stored per game ref in
// yokoi_config/controls.cfg; games without a line use the default.

#include <cstdint>
#include <string>

enum RemapButton : int {
    RB_Up, RB_Down, RB_Left, RB_Right, RB_A, RB_B, RB_X, RB_Y,
    RB_L, RB_R, RB_ZL, RB_ZR, RB_Plus, RB_Minus, RB_L3, RB_R3,
    kNumRemapButtons
};

uint64_t remap_button_mask(int b);       // HidNpadButton_* bit
const char* remap_button_label(int b);   // "A", "ZL", "L3", "Up"...
// The first remappable button in a HidNpadButton_* mask, or -1.
int remap_button_from_mask(uint64_t mask);

// Settings > Menu > Open Pause Menu: 0 ZL + ZR, 1 L3 + R3, 2 L3, 3 R3.
constexpr int kNumPauseCombos = 4;
const char* pause_combo_label(int combo);
uint64_t pause_combo_mask();             // for the current setting

// The game whose map applies, or "" while the menus are up (no remap).
void control_remap_set_active(const std::string& ref);
uint64_t control_remap_apply(uint64_t raw);

// kNoButton: the function is unassigned.
constexpr int kNoButton = 0xFF;
int control_remap_source(const std::string& ref, int logical);
// Leaves `logical` without a button (Controls: no press within 3 s). Its
// button then does nothing until something is assigned to it.
void control_remap_unassign(const std::string& ref, int logical);
void control_remap_assign(const std::string& ref, int logical, int physical);
void control_remap_reset(const std::string& ref, int logical);
void control_remap_reset_all(const std::string& ref);
bool control_remap_customized(const std::string& ref);
