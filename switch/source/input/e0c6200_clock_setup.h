#pragma once

#include <string>

class E0C6S46;

// Sets an E0C6S46/S48 game's clock to the real time on a fresh start, the
// way a player would - through the game's own clock-set screen - for the
// sets whose clock layout is known (core/e0c6200_clock.h).
//
// Writing the time cells alone isn't enough on these games: the clock only
// starts (and a Tamagotchi egg only starts hatching) when the set screen is
// confirmed, which is also when the game initialises the pet. So this plays
// out over real frames, like uchitari_clock_setup.h:
//   1. wait for the boot animation to settle,
//   2. if the clock is already set (a pet restored from a save state - the
//      state loads right after the game does), stop here and touch nothing,
//   3. press the button that opens the "0:00 AM SET" screen,
//   4. write the current local time into the cells that screen edits,
//   5. press the game's confirm button.
// Takes about 5 seconds. Call begin() once per game load (it cancels any
// sequence still running from a previous game), then advance() once per
// frame while active() - and skip the normal E0C6200 input mapping during
// that window, so a stray press can't land mid-sequence.
void e0c6200_clock_setup_begin(const std::string& ref, bool enabled);
bool e0c6200_clock_setup_active();
void e0c6200_clock_setup_advance(E0C6S46* cpu);
