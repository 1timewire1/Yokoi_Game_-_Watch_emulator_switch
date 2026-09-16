
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "segment.h"

struct GW_rom {
    static constexpr uint8_t MANUFACTURER_NINTENDO = 0;
    static constexpr uint8_t MANUFACTURER_TRONICA = 1;
    static constexpr uint8_t MANUFACTURER_ELEKTRONIKA = 2;
    static constexpr uint8_t MANUFACTURER_TIGER = 3;
    static constexpr uint8_t MANUFACTURER_DAVID_AND_JOHN = 4;
    // Games added post-launch, gated behind the Switch-only "Experimental
    // Features" toggle (switch_menu.cpp) rather than the Beta toggle - not
    // yet verified to the same degree as the other manufacturer buckets
    // above. Originally Tiger-only (hence the old name,
    // MANUFACTURER_TIGER_EXPERIMENTAL), but now also covers other
    // manufacturers (Konami, Tronica, Elektronika, Telko/Nintendo, ...)
    // that don't otherwise have their own bucket - their real manufacturer
    // is recorded in the display name itself ("Title (Manufacturer)"),
    // since this field only has room for one bucket per game and none of
    // these have enough titles yet to justify their own.
    static constexpr uint8_t MANUFACTURER_EXPERIMENTAL = 5;
    static constexpr uint8_t MANUFACTURER_COUNT = 6;

    const std::string name;
    const std::string ref;
    const std::string date;

    // Manufacturer id used by the menu to group navigation (no extra rompack metadata).
    const uint8_t manufacturer;

    const uint8_t* rom; const size_t size_rom;
    const uint8_t* melody; const size_t size_melody;

    const std::string path_segment;
    const Segment* segment; const size_t size_segment;
    const uint16_t* segment_info; 

    const std::string path_background;
    const uint16_t* background_info;

    const std::string path_console;
    const uint16_t* console_info;

    // Element count of background_info, when known. 0 for every existing
    // call site (3DS/Android's compiled-in per-game .cpp files, which pass
    // a bare static array and don't know or care about this). The Switch
    // runtime pack loader (gw_pack.cpp) is the only caller that supplies
    // it, since it's the only consumer that needs to detect the optional
    // per-game "overlay" data some games (e.g. Bassmate) append after
    // background_info's normal fields - see overlay_data_file() in
    // CONVERT_ROM/convert_switch.py and switch_screen.cpp's overlay draw.
    const size_t background_info_count = 0;

    GW_rom(const std::string& n, const std::string& r, const std::string& d,
        const uint8_t* rom_, size_t rom_size,
        const uint8_t* melody_, size_t melody_size,
        const std::string& path_seg,
        const Segment* seg_, size_t seg_size,
        const uint16_t* seg_info,
        const std::string& path_bg,
        const uint16_t* bg_info,
        const std::string& path_cs,
        const uint16_t* cs_info,
        uint8_t manufacturer_ = MANUFACTURER_NINTENDO,
        size_t bg_info_count = 0
        )
            : name(n), ref(r), date(d),
            manufacturer(manufacturer_),
            rom(rom_), size_rom(rom_size),
            melody(melody_), size_melody(melody_size),
            path_segment(path_seg),
            segment(seg_), size_segment(seg_size),
            segment_info(seg_info),
            path_background(path_bg),
            background_info(bg_info),
            path_console(path_cs),
            console_info(cs_info),
            background_info_count(bg_info_count)
            {}
};
