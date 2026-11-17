#pragma once

#include <cstdint>

class LCDonald_fake_cpu;

// Overlay for McDonald's (LCDonald) games, drawn in the screen margins
// around the LCD after switch_screen_render() has drawn the game itself:
//
//   - Top right: the run's score (Normal or Endless), in an italic
//     seven-segment style matching the original app's counter (a plain
//     "000" readout beside the LCD in LCDonald's own UI - see
//     MainView.axaml, which showed it in Endless only). Shown once a run has
//     started and kept after it ends (the final score) until the next one.
//     Below it the record (core/score_tracker.h): during a run "HI" + that
//     mode's; on the start screen both modes', "HI Normal" / "HI Endless".
//     0 = no line (none yet, or hidden by Settings > Game List).
//   - Bottom right: a "+ Normal / - Endless" prompt while the game is
//     stopped (freshly loaded, or a finished run), since Plus/Minus are
//     what start it - see main.cpp.
//
// Laid out in the same rotation-aware logical space the menu uses.
void lcdonald_hud_render(int viewport_w, int viewport_h, LCDonald_fake_cpu* cpu, uint32_t hi_normal, uint32_t hi_endless);
