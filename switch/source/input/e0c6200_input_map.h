#pragma once

#include <string>

class E0C6S46;

// Feeds Switch input into an E0C6S46/S48 game's K input pins (see
// source/E0C6200/E0C6S46.h). These boards wire every button straight to a K
// pin (active low, edge-triggered interrupts), not a col/line keypad, so
// they aren't in virtual_input.h's get_input_config() - g_input stays null -
// and main.cpp calls this instead. Per MAME's hh_e0c6200 input ports:
//
//   Tamagotchi / Digital Monster / Venus Diet (3 buttons, left to right):
//     Y or D-Pad Left  -> left button   (A: select / cycle)
//     A or D-Pad Up/Down -> middle button (B: confirm)
//     B or D-Pad Right -> right button  (C: cancel)
//     X                -> K03 sensor (Tenshitchi, Umi/Mori de Hakken, Genjintchi)
//     L + R held       -> RESET pin (Digital Monster only)
//   Yasashii Tamagotchi: D-Pad Left/Up/Right = Food/Smile/Heart,
//     L / D-Pad Down / R = Clean/Bulb/Weight, A = Select, B = Cancel,
//     + and - held = reset
//   Stack Challenge: D-Pad Left/Right/Down, A or D-Pad Up = Rotate,
//     + = On/Off, - = Pause, R = Sound
//   Alien Fever: A = Handle, X = Mode, Y = Select, R = Sound
void apply_e0c6200_input(E0C6S46* cpu, const std::string& ref);
