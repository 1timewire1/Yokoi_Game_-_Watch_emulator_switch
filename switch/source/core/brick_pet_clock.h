#pragma once

#include <string>

class BrickCore;

// Built-in "clock.*" options (BrickCore.h) for BrickEmuPy-core virtual pets,
// so their clock works with an existing pack, without re-importing the game:
// older .brk containers have no clock options, or only sec/min/hour/pm.
// Options the container already has win; this only fills in what's missing.
//
//   APOLLO2IN1 (Apollo 2 in 1, SPLB20) - found with the host harness
//   against BrickEmuPy-main's apollo2in1.bin. 12-hour clock, one digit per
//   RAM cell: seconds 0x9C/0x9D, minutes 0x9E/0x9F, hour 0xA0/0xA1 (1-12),
//   PM flag 0xA2 (1 = PM). Its set-time screen (reached after the language
//   and pet choices on a first boot; Enter = minute+, Mode = hour+, Exit =
//   done) is told apart by its fixed pixels - no RAM byte does it - (the
//   "SET" label etc.; checked over ~70 emulated minutes of play with button
//   presses, they never all show anywhere else); the clock has never been
//   set while RAM[0x9B] == 0.
//   POCKETPIKACHU (Pocket Pikachu, M37520) - brick_games.json's "clock"
//   entry: one BCD byte each for seconds 0x57, minutes 0x58, hour 0x59
//   (0-11), PM flag 0x5A bit 0; first boot sits on a set screen while
//   RAM[0x7D] == 3, which A confirms.
//   TAMAGOTCHIMINI2017 (Tamagotchi Mini 2017, GPL191X) - found with the host
//   harness against BrickEmuPy-main's TamagotchiMini2017.bin: one binary
//   byte each for half-seconds 0x97 (0-119), minutes 0x98, hour 0x99
//   (0-23). It boots stopped at 00:00:00 (egg), and three Center presses
//   (egg -> set screen -> its fields) start it at the default 12:00 AM;
//   the real time is written once it runs. Shown on the game's clock
//   screen as expected (20:07:33 written -> "PM 8:12" five minutes later).
// All three are marked "clock.pet", so Pet Time applies to them.
//
// Refs are matched without case (the pack upper-cases them). Call once per
// game load, after the core is created (switch_game_loader.cpp).
void brick_pet_clock_apply_builtin(BrickCore* core, const std::string& ref);
