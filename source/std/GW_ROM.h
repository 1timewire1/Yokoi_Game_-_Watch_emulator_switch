
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "segment.h"

struct GW_rom {
    // Manufacturer buckets - what the menu groups by. Ids are stored in the
    // rompack (GameEntryV2's manufacturer_id) so they must never be
    // renumbered without bumping the pack's content_version (see
    // CONVERT_ROM/convert_switch.py's ROMPACK_CONTENT_VERSION), and they
    // don't encode display order (switch_menu.cpp's mfr_rank() does) or
    // which ones the "Experimental Games" setting hides
    // (switch_menu.cpp's is_experimental_games_manufacturer()).
    static constexpr uint8_t MANUFACTURER_NINTENDO = 0;
    static constexpr uint8_t MANUFACTURER_TRONICA = 1;
    static constexpr uint8_t MANUFACTURER_ELEKTRONIKA = 2;
    static constexpr uint8_t MANUFACTURER_TIGER = 3;
    static constexpr uint8_t MANUFACTURER_DAVID_AND_JOHN = 4;
    // 5 is retired: it used to be MANUFACTURER_EXPERIMENTAL, a catch-all
    // bucket for post-launch games (Tiger, Konami, Nelsonic, VTech, ...)
    // gated behind their own toggle. Every game in it now lives under its
    // real manufacturer below (or an existing one), and the number is
    // deliberately not reused so a stale pack can never be misread.
    // McDonald's Happy Meal LCD games (2003-2005), ported from the LCDonald
    // project (github.com/Difegue/LCDonald) rather than emulated from a
    // MAME-dumped ROM - see source/LCDonald/.
    static constexpr uint8_t MANUFACTURER_MCDONALDS = 6;
    // "Burger Bard" - LCDonald's white-label re-skin of the McDonald's games
    // (its `#if BURGER` build, "Burger Bard Crystalectrics"; the manufacturer
    // named on its fictional instruction sheets) - see source/LCDonald/.
    static constexpr uint8_t MANUFACTURER_BURGER_BARD = 7;
    static constexpr uint8_t MANUFACTURER_KONAMI = 8;
    static constexpr uint8_t MANUFACTURER_NELSONIC = 9;
    // VTech currently has a single supported device (Electronic Number
    // Muncher); more are expected.
    static constexpr uint8_t MANUFACTURER_VTECH = 10;
    // Seiko Epson E0C6S46/E0C6S48 keychain games (MAME's hh_e0c6200 driver,
    // source/E0C6200/): Tamagotchi / Digital Monster, Alien Fever, Venus
    // Diet Monogatari and Stack Challenge respectively.
    static constexpr uint8_t MANUFACTURER_BANDAI = 11;
    static constexpr uint8_t MANUFACTURER_EPOCH = 12;
    static constexpr uint8_t MANUFACTURER_NIKKO = 13;
    static constexpr uint8_t MANUFACTURER_TANDY = 14;
    // Makers of the games ported from BrickEmuPy (source/BrickCores/,
    // CONVERT_ROM/brick_games.json). Popy is Bandai's toy brand (merged into
    // Bandai in 1983) - kept apart because its 1982-83 units only carry the
    // Popy mark. Brick Game = the unbranded/OEM Chinese and Taiwanese brick
    // games and keychains; Unbranded = everything else with no traceable
    // maker. One-title makers each have a group for now.
    static constexpr uint8_t MANUFACTURER_POPY = 15;
    static constexpr uint8_t MANUFACTURER_GAKKEN = 16;
    static constexpr uint8_t MANUFACTURER_SPICA = 17;
    static constexpr uint8_t MANUFACTURER_APOLLO = 18;
    static constexpr uint8_t MANUFACTURER_BRICK_GAME = 19;
    static constexpr uint8_t MANUFACTURER_HIRO = 20;
    static constexpr uint8_t MANUFACTURER_NATURE_LAND = 21;
    static constexpr uint8_t MANUFACTURER_SPARKLE_CRYSTAL = 22;
    static constexpr uint8_t MANUFACTURER_GAMETECH = 23;
    static constexpr uint8_t MANUFACTURER_COMPILE = 24;
    static constexpr uint8_t MANUFACTURER_VGS = 25;
    static constexpr uint8_t MANUFACTURER_COOL_TEC = 26;
    static constexpr uint8_t MANUFACTURER_HARTUNG = 27;
    static constexpr uint8_t MANUFACTURER_EDLA = 28;
    static constexpr uint8_t MANUFACTURER_STADLBAUER = 29;
    static constexpr uint8_t MANUFACTURER_UNBRANDED = 30;
    static constexpr uint8_t MANUFACTURER_COUNT = 31;

    // Whether this game runs on LCDonald_fake_cpu (source/LCDonald/): its
    // ROM is the 4-byte dummy FF FF FE <game id> (see GW_ROM_RGDS/lcd_*.cpp).
    // The Switch layer's LCDonald-specific handling (named-input model, .ogg
    // playback, Endless-mode HUD) keys off this. Keyed on the ROM, not the
    // manufacturer, since McDonald's also has real-ROM games (Chicken
    // McNugget Tetris, a BrickEmuPy core).
    bool is_lcdonald_game() const {
        return rom && size_rom == 4 && rom[0] == 0xFF && rom[1] == 0xFF && rom[2] == 0xFE;
    }

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
