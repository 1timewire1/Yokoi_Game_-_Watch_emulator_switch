#include "lcdonald_input_map.h"

#include <switch.h>

#include "LCDonald/LCDonald_fake_cpu.h"
#include "switch_input.h"

void apply_lcdonald_input(LCDonald_fake_cpu* cpu) {
    if (!cpu) return;

    const uint64_t held = switch_input_held();  // Already has left-stick folded into D-Pad bits.

    for (const std::string& name : cpu->program()->get_available_inputs()) {
        bool pressed;
        if (name == "Left") {
            pressed = (held & HidNpadButton_Left) != 0;
        } else if (name == "Right") {
            pressed = (held & HidNpadButton_Right) != 0;
        } else if (name == "Up") {
            pressed = (held & HidNpadButton_Up) != 0;
        } else if (name == "Down") {
            pressed = (held & HidNpadButton_Down) != 0;
        } else {
            // The game's one action button, whatever LCDonald calls it.
            pressed = (held & HidNpadButton_A) != 0;
        }
        cpu->set_input_pressed(name, pressed);
    }
}
