#pragma once

#include <cstdint>

// Pet Time: makes time pass for virtual pets while they aren't running - the
// E0C6S46/S48 ones with a known clock layout (core/e0c6200_clock.h) and the
// BrickEmuPy-core ones marked "clock.pet" (Apollo 2 in 1 - BrickCore.h,
// core/brick_pet_clock.h) - per
// g_pet_time_mode (switch_settings.h):
//   - after a save state loads: the time since that state was saved (its
//     timestamp - E0C6S46/BrickCore::saved_unix_time()),
//   - mid-play: any real-time gap between two emulated frames (HOME menu,
//     console sleep, the in-game settings screen - anything that stops the
//     main loop from stepping the CPU).
// Real Time fast-forwards the pet through that time (up to the cap; the rest
// is skipped by jumping its clock first), a slice per frame so the app stays
// responsive, then sets its clock to now (covering the time the catch-up
// itself took). B stops early - the rest is skipped the same way. Clock Sync
// only sets the clock. Nothing happens to a pet whose clock was never set.
//
// Wiring: pet_time_on_game_loaded() from the loader (resets everything),
// pet_time_on_state_loaded() after a save state loads, and pet_time_frame()
// once per gameplay frame before input/stepping - while it returns true it
// has consumed the frame (catching up), and the caller should just render
// and draw pet_time_render() on top.
void pet_time_on_game_loaded();
void pet_time_on_state_loaded();
bool pet_time_frame(uint64_t kDown);
void pet_time_render(int viewport_w, int viewport_h);

// Pause menu extras (switch_menu.cpp):
// - The running game is a pet Pet Time handles, with its clock set - only
//   then are Simulate Time / Undo Simulate offered.
bool pet_time_current_is_pet();
// - Simulate Time: the pet lives `seconds` right now, fast-forwarded like a
//   catch-up (same overlay/display setting, B stops early; the Real Time cap
//   doesn't apply). Afterwards the clock is put back to the real time,
//   except under Paused (or Experimental Features off), where it stays
//   that far ahead - like winding a real one's clock forward.
void pet_time_simulate(int64_t seconds);
// - After Undo Simulate restored the pre-simulation copy: clock back to
//   the real time (same rule as above), nothing caught up.
void pet_time_after_undo();
// - Loading an older slot: how long ago the loaded state was saved (0 if
//   not a pet / unknown), whether Pet Time would catch that up (Real Time),
//   and the "load as is" choice - only the clock is set.
int64_t pet_time_state_age();
bool pet_time_would_catch_up();
void pet_time_sync_clock();
