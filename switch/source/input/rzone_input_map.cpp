#include "rzone_input_map.h"

#include <switch.h>

#include "switch_input.h"
#include "virtual_i_o/rzone_io.h"

void apply_rzone_input(RZoneIO* io) {
    if (!io) return;

    const uint64_t held = switch_input_held();  // Already has left-stick folded into D-Pad bits.
    struct Map { uint64_t pad; uint16_t rzone; };
    static const Map kMap[] = {
        {HidNpadButton_Up, RZoneIO::Up},
        {HidNpadButton_Down, RZoneIO::Down},
        {HidNpadButton_Left, RZoneIO::Left},
        {HidNpadButton_Right, RZoneIO::Right},
        {HidNpadButton_A, RZoneIO::Button1},
        {HidNpadButton_B, RZoneIO::Button2},
        {HidNpadButton_X, RZoneIO::Button3},
        {HidNpadButton_Y, RZoneIO::Button4},
        {HidNpadButton_Plus, RZoneIO::Start},
        {HidNpadButton_Minus, RZoneIO::Pause},
        {HidNpadButton_L, RZoneIO::Select},
        {HidNpadButton_R, RZoneIO::Sound},
    };
    uint16_t buttons = 0;
    for (const Map& m : kMap) {
        if (held & m.pad) buttons |= m.rzone;
    }
    io->set_buttons(buttons);
}
