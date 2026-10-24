#pragma once

#include <cstdint>

class LCDonald_fake_cpu;

// Overlay for McDonald's (LCDonald) games, drawn in the screen margins
// around the LCD after switch_screen_render() has drawn the game itself:
//
//   - Top right: the Endless-mode score, in an italic seven-segment style
//     matching the original app's counter (a plain "000" readout beside the
//     LCD in LCDonald's own UI - see MainView.axaml). Shown once an Endless
//     run has started and kept after it ends (the final score) until the
//     next run starts. Below it, "HI" + the best Endless score for this
//     game (core/high_scores.h), also shown on the start screen.
//   - Bottom right: a "+ Normal / - Endless" prompt while the game is
//     stopped (freshly loaded, or a finished run), since Plus/Minus are
//     what start it - see main.cpp.
//
// Laid out in the same rotation-aware logical space the menu uses.
void lcdonald_hud_render(int viewport_w, int viewport_h, LCDonald_fake_cpu* cpu, uint32_t endless_best);
