#include "modifier_layer_input.h"

#include <switch.h>

#include "SM5XX/SM5XX.h"
#include "switch_input.h"

namespace {

void read_slot_state(uint64_t held, bool out[12]) {
    out[0] = (held & HidNpadButton_Up) != 0;
    out[1] = (held & HidNpadButton_Down) != 0;
    out[2] = (held & HidNpadButton_Left) != 0;
    out[3] = (held & HidNpadButton_Right) != 0;
    out[4] = (held & HidNpadButton_A) != 0;
    out[5] = (held & HidNpadButton_B) != 0;
    out[6] = (held & HidNpadButton_X) != 0;
    out[7] = (held & HidNpadButton_Y) != 0;
    out[8] = (held & HidNpadButton_L) != 0;
    out[9] = (held & HidNpadButton_R) != 0;
    out[10] = (held & HidNpadButton_Plus) != 0;
    out[11] = (held & HidNpadButton_Minus) != 0;
}

} // namespace

void apply_modifier_layer_input(SM5XX* cpu, const ModLayerTarget table[36]) {
    if (!cpu) return;

    const uint64_t held = switch_input_held();
    const bool zl = (held & HidNpadButton_ZL) != 0;
    const bool zr = (held & HidNpadButton_ZR) != 0;
    const int layer = (zl && !zr) ? 1 : (zr && !zl) ? 2 : 0;

    bool slot_state[12];
    read_slot_state(held, slot_state);

    // Force every OTHER layer's targets low first, so switching layers
    // mid-hold (e.g. releasing ZL while still holding D-Up) can't leave a
    // button from the no-longer-active layer stuck held forever.
    for (int l = 0; l < 3; l++) {
        if (l == layer) continue;
        for (int s = 0; s < 12; s++) {
            const ModLayerTarget& t = table[l * 12 + s];
            if (t.group >= 0) cpu->input_set(t.group, t.line, false);
        }
    }

    // OR together this layer's slots per (group,line) target before
    // applying - some targets are intentionally shared by two slots (e.g.
    // UCHITARI's Alarm/Exam Level, wired to the same line on real
    // hardware), and a naive per-slot input_set() call would let whichever
    // slot happens to be processed last clobber the other's press.
    bool combined[8][4] = {};
    bool touched[8][4] = {};
    for (int s = 0; s < 12; s++) {
        const ModLayerTarget& t = table[layer * 12 + s];
        if (t.group < 0) continue;
        combined[t.group][t.line] = combined[t.group][t.line] || slot_state[s];
        touched[t.group][t.line] = true;
    }
    for (int g = 0; g < 8; g++) {
        for (int l = 0; l < 4; l++) {
            if (touched[g][l]) cpu->input_set(g, l, combined[g][l]);
        }
    }
}

void apply_calculator_modifier_input(SM5XX* cpu, const ModLayerTarget table[24]) {
    if (!cpu) return;

    const uint64_t held = switch_input_held();
    const bool zl = (held & HidNpadButton_ZL) != 0;
    const bool zr = (held & HidNpadButton_ZR) != 0;
    const int active_layer = (zl && !zr) ? 1 : (zr && !zl) ? 2 : 0; // 0 = neither - normal input owns everything

    bool slot_state[12];
    read_slot_state(held, slot_state);

    // Clear whichever of the two layers isn't active (both, if neither is
    // held) - unlike apply_modifier_layer_input() above there's no base
    // layer to fall back to here, since the caller's own Virtual_Input
    // call already drives the unmodified controls every frame regardless.
    for (int l = 1; l <= 2; l++) {
        if (l == active_layer) continue;
        int base = (l - 1) * 12;
        for (int s = 0; s < 12; s++) {
            const ModLayerTarget& t = table[base + s];
            if (t.group >= 0) cpu->input_set(t.group, t.line, false);
        }
    }
    if (active_layer == 0) return;

    int base = (active_layer - 1) * 12;
    bool combined[8][4] = {};
    bool touched[8][4] = {};
    for (int s = 0; s < 12; s++) {
        const ModLayerTarget& t = table[base + s];
        if (t.group < 0) continue;
        combined[t.group][t.line] = combined[t.group][t.line] || slot_state[s];
        touched[t.group][t.line] = true;
    }
    for (int g = 0; g < 8; g++) {
        for (int l = 0; l < 4; l++) {
            if (touched[g][l]) cpu->input_set(g, l, combined[g][l]);
        }
    }
}
