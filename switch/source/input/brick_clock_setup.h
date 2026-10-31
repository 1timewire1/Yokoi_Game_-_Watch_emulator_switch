#pragma once

class BrickCore;

// Sets a BrickEmuPy-core watch's clock to the real time a second after it
// loads (BrickCore::set_clock(), laid out by the game's "clock.*" options -
// see brick_games.json). The plain write already happens during load, like
// every other clock game (yokoi_cpu_set_time_if_needed()), but that isn't
// enough on the Nelsonic Game Watches: they boot to an all-segments test
// screen with the clock stopped, and leaving it (any direction button)
// resets the hour. So, over real frames:
//   1. let the game settle - long enough for a save state to load too,
//   2. if the screen is that test pattern and the game names a button that
//      leaves it ("clock.start"), press it,
//   3. write the current local time.
// Games that boot into a clock-set screen instead (Pocket Pikachu:
// "clock.unset" + "clock.confirm") get the confirm button pressed, again
// every second or so - intro screens swallow some presses - until the
// game's own "clock set" flag clears, then the time written, seconds
// included. A game that's already running (restored from a save state)
// just gets the plain write, i.e. its time synced. Call begin() once per
// game load, then advance() once per frame while active(), instead of
// apply_brick_input().
void brick_clock_setup_begin(bool enabled);
bool brick_clock_setup_active();
void brick_clock_setup_advance(BrickCore* core);
