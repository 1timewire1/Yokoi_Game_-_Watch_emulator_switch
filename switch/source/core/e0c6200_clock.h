#pragma once

#include <ctime>
#include <string>

class E0C6S46;

// Where each E0C6S46/S48 virtual pet keeps its clock in RAM, found with the
// host harness (CONVERT_ROM/tmp/e0c_clock_findings.md). Shared by the
// automatic clock setup (input/e0c6200_clock_setup.cpp) and Pet Time
// (core/pet_time.cpp) - a game is in scope for either only if it's listed
// here, i.e. its clock layout is known.
//
// From `base`: seconds units, seconds tens, minutes units, minutes tens
// (BCD), then the hour:
//   Tama1   - hour 0-23 binary, low nibble then high nibble; >= 24 means
//             unset (0x20 at power-on). Tamagotchi Gen. 1 and its relatives.
//   Digimon - hour 1-12, then a PM flag nibble; hour 0 means unset (the
//             clock stops). Digital Monster, Mothra, Genjintchi.
//   Tama1Zero - hour as Tama1, but the cells boot as all zeros, so "unset"
//             is all six zero with the game's clock-running flag
//             (set_flag_addr/mask) clear. The unidentified "Tamaotch".
// The games read these cells as the live time once the clock is running, so
// writing them (clock_write) changes the time the pet sees, same as using
// its own set-clock screen.
struct E0C6200ClockLayout {
    enum class Kind { Tama1, Digimon, Tama1Zero };
    const char* ref;
    Kind kind;
    int base;
    int open_line;     // K pin of the button that opens the clock-set screen
    int confirm_line;  // K pin of the button that confirms it
    // K pin pressed once on the set screen before the time is written, for
    // games whose confirm is ignored until the time has been edited by hand
    // (Tamaotch: an hour+ press). -1 = none.
    int nudge_line = -1;
    int set_flag_addr = -1;  // Tama1Zero: RAM nibble whose set_flag_mask bit means "clock running"
    int set_flag_mask = 0;
};

// nullptr if `ref` isn't a pet with a known clock layout.
const E0C6200ClockLayout* e0c6200_clock_layout(const std::string& ref);

bool e0c6200_clock_is_set(const E0C6S46* cpu, const E0C6200ClockLayout& layout);

// Writes the local time of `when` into the clock cells.
void e0c6200_clock_write(E0C6S46* cpu, const E0C6200ClockLayout& layout, time_t when);
