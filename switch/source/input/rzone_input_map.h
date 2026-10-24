#pragma once

class RZoneIO;

// Feeds Switch input into a Tiger R-Zone cartridge's 74165 button shift
// register (see source/virtual_i_o/rzone_io.h). R-Zone games aren't in
// virtual_input.h's get_input_config() - g_input stays null for them - so
// main.cpp calls this instead of the standard controller mapping:
//   D-Pad / left stick -> joystick     A / B / X / Y -> buttons 1-4 (A-D)
//   +                  -> Start        -             -> Pause (button 5)
//   L                  -> Select       R             -> Sound (volume)
// Power On/Off are left unmapped: the games run from boot without them.
void apply_rzone_input(RZoneIO* io);
