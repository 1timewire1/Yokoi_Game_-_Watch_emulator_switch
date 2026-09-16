#pragma once
#include <cstdint>

class SM5XX;

// Drives VESARIF's physical "set the clock" button sequence automatically
// on load - same rationale and mechanism as uchitari_clock_setup.h (RAM
// injection via time_addresses.h has zero effect on this game's display
// either, confirmed the same way: two different injected times produced
// byte-identical lit segments). VESARIF's own UI differs from uchitari's
// though: there's no dedicated Hour/Minute button, just Time/Game/Start/
// Alarm plus four directional buttons. Confirmed empirically (tapping each
// button up to its full wrap length and checking the display returns to
// its exact starting pattern): Left-Up and Left-Down are electrically
// identical and increment the hour (0-23, 24-tap cycle); Right-Up and
// Right-Down are likewise identical and increment the minute (0-59,
// 60-tap cycle). The sequence is Time (enters set mode) -> Hour taps ->
// Minute taps -> Time again (confirms and exits) - confirmed by direct
// A/B comparison: pressing Game twice with no time set produces identical,
// unresponsive segments both times, but following this exact sequence
// first makes the second Game press transition to a genuinely different
// screen. Without it, Game/Start don't respond at all.
//
// Same tap-timing requirement as uchitari (6 frames held/10 released per
// repeated tap, confirmed by sweep - anything faster starts dropping
// presses), so a worst case (23 hour-taps + 59 minute-taps) takes ~22
// seconds. Call begin() once right after the game loads, then advance()
// once per frame while is_active() is true, suppressing every other input
// path for this game in the meantime (see uchitari_clock_setup.h's doc
// comment on why that matters - a stray press could get miscounted as one
// of the scripted taps).
void vesarif_clock_setup_begin(uint8_t hour, uint8_t minute);
bool vesarif_clock_setup_active();
void vesarif_clock_setup_advance(SM5XX* cpu);
