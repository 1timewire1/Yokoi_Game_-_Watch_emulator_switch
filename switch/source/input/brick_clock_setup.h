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
// just gets the plain write, i.e. its time synced - unless it's a virtual
// pet ("clock.pet": Pocket Pikachu, Apollo 2 in 1), whose time away is
// Pet Time's to handle (core/pet_time.h).
//
// Games whose set-time screen the player reaches by themselves
// ("clock.set_screen" - Apollo 2 in 1: language and pet choice first) are
// watched instead, for the whole session, without taking the inputs: each
// time that screen comes up, the setup waits half a second, writes the real
// time into it and presses "clock.confirm" (again, up to a few times, if
// the screen is still there, as on Super Gyaoppi and Big Cat, whose confirm
// goes through their alarm screen first), then writes the time once more,
// since a confirm can zero the seconds. Nothing is pressed at boot.
//
// Call begin() once per game load (Time Set Mode = Automatic), then each
// frame: advance() instead of apply_brick_input() while owns_input(),
// otherwise apply_brick_input() and then advance() while active().
void brick_clock_setup_begin(bool enabled);
bool brick_clock_setup_active();
bool brick_clock_setup_owns_input();
void brick_clock_setup_advance(BrickCore* core);
