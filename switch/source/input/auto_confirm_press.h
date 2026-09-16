#pragma once
#include <cstdint>

class SM5XX;

// Auto-presses a single confirm button once, shortly after load, for games
// whose clock is set correctly by the normal time_addresses.h RAM-injection
// path but which still won't respond to anything else until that button is
// pressed once to leave the just-booted time-display screen - confirmed by
// hand on real hardware for BASSMATE, every Tronica game except Treasure
// Island (TRTREISL, which needs no confirm press at all), almost every
// Nintendo Game & Watch title (all but Chef/Egg/Mickey Mouse Wide Screen,
// none of which gate anything on Time), and three Elektronika titles that
// reuse a Nintendo class but still need their own confirm press (Tayny
// okeana/OC_22's class, Vinni-Pukh Panorama/CJ_93's, Kosmicheskiy most/
// FR_27's - see kAutoConfirmButtons in switch_game_loader.cpp for the full
// per-game list). For the three Tronica games with a hidden calculator
// keypad (MG_8/TG_18/SA_12, see apply_calculator_modifier_input()) the
// confirm button is "Mode" instead of "Time", since Time re-enters the
// clock-set screen on those three the same way it does on UCHITARI/VESARIF.
// Gated behind g_time_set_mode (see switch_settings.h) - only ever begun
// when that's Automatic; callers skip begin() entirely under Manual.
//
// Call begin(group, line) once right after the game loads with that game's
// confirm-button target (see each Virtual_Input class's PART_SETUP case for
// its mapping), then advance() once per frame while active() is true,
// suppressing the normal input path in the meantime - this is a single
// ~0.2s press, but a same-frame player press on the same line could still
// race it. `not_multiplex` mirrors SM5XX::input_set()'s own parameter of
// the same name - pass true when the target class's own BUTTON_TIME case
// does (AC_01/RC_04/MT_03's Time button routes through the CPU's separate
// non-multiplexed K-register bit rather than the normal per-group one;
// using the wrong path there silently no-ops the press).
void auto_confirm_press_begin(int8_t group, int8_t line, bool not_multiplex = false);
bool auto_confirm_press_active();
void auto_confirm_press_advance(SM5XX* cpu);
