#pragma once
#include <cstdint>

class SM5XX;

// A 3-layer (base / ZL-held / ZR-held) x 12-slot input scheme for games
// with more distinct physical buttons (~20-26) than the standard 12-input
// layout (D-pad x4, A/B/X/Y, L/R, +/-) can cover on its own - see NUMMUNCH
// and UCHITARI in main.cpp, both calculator-style keypads that don't fit
// this codebase's usual PART_LEFT/PART_RIGHT/Virtual_Input scheme at all.
// Not ideal control-wise (three key-presses to reach some buttons), but
// enough to actually play and test these two on real hardware rather than
// leaving them unported over a button-count mismatch alone.
//
// Slot order within a layer, matching switch_input.h's button set:
// D-Up, D-Down, D-Left, D-Right, A, B, X, Y, L, R, +, -.
// ZL+ZR held together is the existing pause-menu combo (see main.cpp) and
// is deliberately treated as "no modifier" here (falls back to the base
// layer) so it doesn't leave a stray input held while the pause menu is
// open - it never reaches this function while the menu is actually up
// anyway, since main.cpp's gameplay branch is skipped whenever the menu is
// active.
struct ModLayerTarget {
    int8_t group; // cpu->input_set() group; -1 = this slot is unused on this layer
    int8_t line;  // cpu->input_set() line
};

// table[layer*12 + slot], layer 0 = base, 1 = ZL held, 2 = ZR held.
// Reads the current physical button/ZL/ZR state itself (via
// switch_input_held()) - callers just pass the CPU and the game's table.
void apply_modifier_layer_input(SM5XX* cpu, const ModLayerTarget table[36]);

// Additive variant for games that already have a normal Virtual_Input
// control scheme that must keep working unmodified (unlike NUMMUNCH/
// UCHITARI above, which have none) - see SPACE_ADVENTURE/SHUTTLE_VOYAGE/
// THIEF_IN_GARDEN in main.cpp. Those three are ordinary joystick+action
// games in the main library, but their real hardware also has a full
// calculator keypad (digits, operators, %, memory, etc. - ~19 buttons)
// that nothing in this codebase has ever mapped. Exposing that keypad
// changes the game's own input behavior in a way the original port
// didn't (holding a math operator can, e.g., feed a "cheat"-like value
// into gameplay), so unlike the two calculator-only games above this is
// gated behind Experimental Features (see g_experimental_features_enabled) rather
// than always on - main.cpp checks that before calling this at all.
//
// table[(layer-1)*12 + slot], layer 1 = ZL held, 2 = ZR held - no base
// layer slot 0 here, since the caller's normal Virtual_Input-driven call
// already owns the unmodified controls; this only ever touches the
// (group, line) targets listed in the table, clearing them when neither
// ZL nor ZR is held rather than leaving a calculator key stuck once the
// modifier is released.
void apply_calculator_modifier_input(SM5XX* cpu, const ModLayerTarget table[24]);
