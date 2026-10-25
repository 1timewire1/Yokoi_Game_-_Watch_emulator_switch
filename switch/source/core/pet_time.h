#pragma once

#include <cstdint>

// Pet Time: makes time pass for E0C6S46/S48 virtual pets (only those with a
// known clock layout - core/e0c6200_clock.h) while they aren't running, per
// g_pet_time_mode (switch_settings.h):
//   - after a save state loads: the time since that state was saved (its
//     timestamp - E0C6S46::saved_unix_time()),
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
