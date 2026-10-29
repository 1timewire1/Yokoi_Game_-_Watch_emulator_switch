#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

// The ".brk" ROM container for games whose core is ported from BrickEmuPy
// (github.com/azya52/BrickEmuPy, CC0) - written by CONVERT_ROM/brick_import.py
// from a .brick config and stored in the pack as the game's ROM. BrickEmuPy
// configures each game with a JSON .brick file (core, clock, mask options,
// input wiring) next to its ROM(s); this carries all of that, so the loader
// can pick the core from the ROM alone (same idea as LCDonald's dummy ROM)
// and nothing per game needs to be compiled in.
//
// Layout (little-endian):
//   "BRK1"
//   u8  core id length, core id ("HT943", "T7741", ... - BrickEmuPy's names)
//   u32 clock (Hz, the .brick "clock": the unit core cycles are counted in)
//   u32 options length, options text: "key=value" lines (see Options)
//   u32 blob count, then per blob: u8 name length, name, u32 size, bytes
//       "rom" is the program ROM; others are core-specific ("sound_rom", ...)
struct BrickRom {
    // Flat view of the .brick's mask options + input wiring. Nested keys are
    // dotted ("port_pullup.K0=15"), lists comma-separated ("1,2,3"), and the
    // input bindings are "input.<button>=..." lines (see brick_input_map.h).
    class Options {
    public:
        bool has(const std::string& key) const { return values_.count(key) != 0; }
        const std::string& str(const std::string& key) const;
        long num(const std::string& key, long fallback = 0) const;
        std::vector<long> list(const std::string& key) const;
        // Every key starting with `prefix` (e.g. "input."), prefix stripped.
        std::map<std::string, std::string> with_prefix(const std::string& prefix) const;
        void parse(const char* text, size_t size);

    private:
        std::map<std::string, std::string> values_;
    };

    std::string core;
    uint32_t clock_hz = 0;
    Options options;
    std::map<std::string, std::vector<uint8_t>> blobs;

    const std::vector<uint8_t>* blob(const std::string& name) const;

    static bool is_container(const uint8_t* data, size_t size);
    // False (leaving *this partly filled) on a truncated or malformed container.
    bool parse(const uint8_t* data, size_t size);
};
