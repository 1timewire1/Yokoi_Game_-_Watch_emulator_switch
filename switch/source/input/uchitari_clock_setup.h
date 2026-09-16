#pragma once
#include <cstdint>

class SM5XX;

// Drives UCHITARI's physical "set the clock" button sequence automatically
// on load, standing in for time_addresses.h's usual direct-RAM-write
// approach: investigation this session proved that approach has zero
// effect on this particular game's display (the RAM cells it would write
// tick correctly in isolation but the display driver never reads them -
// only real Hour/Minute button presses do). So instead, this simulates the
// exact manual procedure a player would use: press Mode once (dismisses
// the boot test-pattern screen), tap Hour once per hour (0-23, wrapping
// like the real device), tap Minute once per minute (0-59), then press
// Alarm once - confirmed necessary to actually exit the set-time screen
// afterward (a second Mode press with no prior Alarm press does nothing at
// all; the device is otherwise stuck).
//
// Each tap must be held/released for several frames to register reliably
// (see this session's timing sweep - anything faster starts dropping
// presses), so the full sequence can take up to ~20 seconds worst case
// (23 hour-taps + 59 minute-taps). It plays out over real gameplay frames
// rather than blocking on load, so the player watches the digits climb to
// the current time. Call begin() once right after the game loads, then
// advance() once per frame while is_active() is true - and skip every
// other input path (Virtual_Input, the modifier-layer scheme, etc.)
// entirely during that window, so a stray player button press can't get
// miscounted as one of the scripted taps.
void uchitari_clock_setup_begin(uint8_t hour, uint8_t minute);
bool uchitari_clock_setup_active();
void uchitari_clock_setup_advance(SM5XX* cpu);
