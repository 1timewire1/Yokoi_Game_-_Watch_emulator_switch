#include "e0c6200_clock.h"

#include "E0C6200/E0C6S46.h"

namespace {

using Kind = E0C6200ClockLayout::Kind;

// B (K01) opens the set screen, C (K00) confirms, on every set verified so
// far. Yasashii Tamagotchi and Venus Diet are left out until solved; Alien
// Fever's counter is uptime, and Stack Challenge has no clock.
constexpr E0C6200ClockLayout kLayouts[] = {
    {"TAMA", Kind::Tama1, 0x010, 1, 0},        {"TAMAG2", Kind::Tama1, 0x010, 1, 0},
    {"TAMAJ", Kind::Tama1, 0x010, 1, 0},       {"TAMAANG", Kind::Tama1, 0x010, 1, 0},
    {"TAMAMORI", Kind::Tama1, 0x010, 1, 0},    {"TAMAUMI", Kind::Tama1, 0x011, 1, 0},
    {"DIGIMON", Kind::Digimon, 0x01A, 1, 0},   {"DIGIMONV2", Kind::Digimon, 0x01A, 1, 0},
    {"DIGIMONV3", Kind::Digimon, 0x01A, 1, 0}, {"DIGIMONES", Kind::Digimon, 0x01A, 1, 0},
    {"DIGIMONV4", Kind::Digimon, 0x01A, 1, 0}, {"TAMAMOT", Kind::Digimon, 0x01A, 1, 0},
    {"TAMAGENJIN", Kind::Digimon, 0x01A, 1, 0}, {"DIGIMONJA", Kind::Digimon, 0x01A, 1, 0},
    // Unidentified S48 Tamagotchi: Gen. 1-style cells at 0x102, confirm only
    // takes after an A (hour+) press; 0x101 bit 3 is set while the clock runs.
    {"TAMAOTCH", Kind::Tama1Zero, 0x102, 1, 0, 2, 0x101, 0x8},
};

}  // namespace

const E0C6200ClockLayout* e0c6200_clock_layout(const std::string& ref) {
    for (const E0C6200ClockLayout& l : kLayouts) {
        if (ref == l.ref) return &l;
    }
    return nullptr;
}

bool e0c6200_clock_is_set(const E0C6S46* cpu, const E0C6200ClockLayout& l) {
    if (l.kind == Kind::Tama1) {
        return (cpu->ram_nibble(l.base + 5) << 4 | cpu->ram_nibble(l.base + 4)) < 24;
    }
    if (l.kind == Kind::Tama1Zero) {
        if (l.set_flag_addr >= 0 && (cpu->ram_nibble(l.set_flag_addr) & l.set_flag_mask)) return true;
        for (int i = 0; i < 6; i++) {
            if (cpu->ram_nibble(l.base + i)) return true;  // (both clear only at exactly midnight)
        }
        return false;
    }
    return cpu->ram_nibble(l.base + 4) != 0;
}

void e0c6200_clock_write(E0C6S46* cpu, const E0C6200ClockLayout& l, time_t when) {
    const tm* t = localtime(&when);
    if (!t) return;
    const int h = t->tm_hour, m = t->tm_min, s = t->tm_sec;
    const uint8_t digits[4] = {(uint8_t)(s % 10), (uint8_t)(s / 10), (uint8_t)(m % 10), (uint8_t)(m / 10)};
    for (int i = 0; i < 4; i++) cpu->set_ram_nibble(l.base + i, digits[i]);
    if (l.kind == Kind::Tama1 || l.kind == Kind::Tama1Zero) {
        cpu->set_ram_nibble(l.base + 4, h & 0xf);
        cpu->set_ram_nibble(l.base + 5, h >> 4);
    } else {
        cpu->set_ram_nibble(l.base + 4, (h % 12) ? (h % 12) : 12);
        cpu->set_ram_nibble(l.base + 5, h >= 12 ? 1 : 0);
    }
}
