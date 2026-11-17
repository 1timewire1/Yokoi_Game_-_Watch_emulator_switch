#pragma once

#include <cstdint>

// Fast-forward and rewind (Settings > Saves & Clock, and the pause menu's
// Game Session): each is Off by default or held on a button of the
// player's choice (ZL, ZR, L3 or R3 - never the ones that open the pause
// menu). That button stops reaching the game while it's assigned.
//  - Fast-forward: the game runs 2x / 4x / 8x while held (sound muted).
//  - Rewind: a snapshot is kept every quarter second for the last 5 / 10
//    / 30 / 60 s; holding the button steps back through them.

class SM5XX;

// The buttons assigned (HidNpadButton_* bits), so the game doesn't see them.
uint64_t time_control_hotkey_mask();
// Button choices for the settings rows: 0 Off, 1 ZL, 2 ZR, 3 L3, 4 R3.
constexpr int kNumHotkeyChoices = 5;
const char* time_control_button_label(int choice);
// The next allowed choice from `current` in direction dir (skips the pause
// combo's buttons and the other hotkey's).
int time_control_next_choice(int current, int dir, int other_choice);

// Once per gameplay frame, before the CPU is stepped.
void time_control_update();
// How many times faster than normal to run this frame (1 = normal).
int time_control_speed();
bool time_control_rewinding();
// Rewinding: goes back one snapshot every few frames; true when it did
// (the caller refreshes the screen from the CPU).
bool time_control_rewind_step(SM5XX* cpu);
// After a normally stepped frame: keeps the snapshot ring going.
void time_control_after_frame(SM5XX* cpu);
// ">> 4x" / "<< REW" in the corner while either is active.
void time_control_render();
