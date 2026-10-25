#include "e0c6200_input_map.h"

#include <switch.h>

#include "switch_input.h"
#include "E0C6200/E0C6S46.h"

namespace {

// One K pin and the Switch buttons (any of) that press it.
struct PinMap {
    int k_line;  // 0-3 = K00-K03, 4-7 = K10-K13
    uint64_t pads;
};

// Tamagotchi (Gen 1/2, Mothra, Angel) and Digital Monster: K02/K01/K00 are
// the left/middle/right buttons (MAME's Button1/2/3); Angel adds K03.
constexpr PinMap kTama[] = {
    {2, HidNpadButton_Y | HidNpadButton_Left},
    {1, HidNpadButton_A | HidNpadButton_Up | HidNpadButton_Down},
    {0, HidNpadButton_B | HidNpadButton_Right},
};
// K03 sensor on the E0C6S48 Tamagotchis (Tenshitchi, Umi/Mori de Hakken,
// Genjintchi - BrickEmuPy wires it to tapping the screen frame); K03 is
// unconnected on the others.
constexpr PinMap kTamaSensor[] = {{3, HidNpadButton_X}};

// Yasashii Tamagotchi: six icon buttons (top row Food / Smile / Heart,
// bottom row Clean / Bulb / Weight) plus Select / Cancel, per BrickEmuPy's
// TamagotchiYasashii.brick. Mapped by position: top row on the D-Pad, bottom
// row on L / D-Pad Down / R.
constexpr PinMap kYasashii[] = {
    {7, HidNpadButton_Left},   // Food
    {6, HidNpadButton_Up},     // Smile
    {5, HidNpadButton_Right},  // Heart
    {4, HidNpadButton_L},      // Clean
    {3, HidNpadButton_Down},   // Bulb
    {2, HidNpadButton_R},      // Weight
    {1, HidNpadButton_A},      // Select
    {0, HidNpadButton_B},      // Cancel
};

// Venus Diet Monogatari: same three buttons, on K12/K11/K10.
constexpr PinMap kVenus[] = {
    {6, HidNpadButton_Y | HidNpadButton_Left},
    {5, HidNpadButton_A | HidNpadButton_Up | HidNpadButton_Down},
    {4, HidNpadButton_B | HidNpadButton_Right},
};

constexpr PinMap kStackChallenge[] = {
    {0, HidNpadButton_Left},                      // Left / Level
    {1, HidNpadButton_Down},                      // Down / Start
    {2, HidNpadButton_Right},                     // Right / Height
    {3, HidNpadButton_A | HidNpadButton_Up},      // Rotate / Contrast
    {4, HidNpadButton_Plus},                      // On / Off
    {5, HidNpadButton_Minus},                     // Pause
    {6, HidNpadButton_R},                         // Sound
};

constexpr PinMap kAlienFever[] = {
    {0, HidNpadButton_X},  // Mode
    {1, HidNpadButton_Y},  // Select
    {2, HidNpadButton_R},  // Sound
    {3, HidNpadButton_A},  // Handle
};

template <size_t N>
void apply(E0C6S46* cpu, const PinMap (&map)[N], uint64_t held) {
    for (const PinMap& m : map) cpu->set_k_line(m.k_line, (held & m.pads) == 0);  // active low
}

}  // namespace

void apply_e0c6200_input(E0C6S46* cpu, const std::string& ref) {
    if (!cpu) return;
    const uint64_t held = switch_input_held();  // Already has left-stick folded into D-Pad bits.

    if (ref == "STACKCH") {
        apply(cpu, kStackChallenge, held);
    } else if (ref == "ALIENFEV") {
        apply(cpu, kAlienFever, held);
    } else if (ref == "VENUSDM") {
        apply(cpu, kVenus, held);
    } else if (ref == "TAMAYASA") {
        apply(cpu, kYasashii, held);
        // L and R are buttons here, so its reset pin is on + and - together.
        cpu->set_reset_line((held & HidNpadButton_Plus) && (held & HidNpadButton_Minus));
    } else {
        apply(cpu, kTama, held);
        if (ref == "TAMAANG" || ref == "TAMAUMI" || ref == "TAMAMORI" || ref == "TAMAGENJIN") {
            apply(cpu, kTamaSensor, held);
        }
        // Digital Monster's reset button is wired to the RESET pin; a
        // two-button chord so it can't be hit by accident mid-battle.
        if (ref.rfind("DIGIMON", 0) == 0) {
            cpu->set_reset_line((held & HidNpadButton_L) && (held & HidNpadButton_R));
        }
    }
}
