#ifndef TIME_ADDRESSES_H
#define TIME_ADDRESSES_H
#include <stdint.h>
#include <string>

#include <unordered_map>

struct TimeAddress {
    uint8_t col_hour_tens,    line_hour_tens;
    uint8_t col_hour_units,   line_hour_units;
    uint8_t col_minute_tens,  line_minute_tens;
    uint8_t col_minute_units, line_minute_units;
    uint8_t col_second_tens,  line_second_tens;
    uint8_t col_second_units, line_second_units;
    uint8_t pm_bit;                                 // PM bit for 12-hour format
    // Combined-hour games only (see SM5XX::set_time()'s "col_hour_tens ==
    // col_hour_units" sentinel): a dedicated cell for the PM flag, since
    // there's no spare bit left in a single-cell 1-12 hour value to OR
    // pm_bit into directly. Defaults to 99,99 (unused) for every other
    // entry in this file, same "out of range = ignored" sentinel
    // set_ram_value() already uses for an absent seconds field.
    uint8_t col_pm = 99, line_pm = 99;
};

// Some of the address mappings in this file are derived from https://github.com/bzhxx/LCD-Game-Shrinker
// The rest came from inspection of ram dumps using debug_dump_ram_state() 

inline const TimeAddress* get_time_addresses(const std::string& ref_game) {
    // --- SM5A ---
    static const std::unordered_map<std::string, TimeAddress> sm5a_map = {
        {"AC_01", {4,8, 4,9, 4,10, 4,11, 99,99, 99,99, 0}}, // Ball
        {"FL_02", {4,4, 4,5, 4,6, 4,7, 99,99, 99,99, 0}}, // Flagman
        {"MT_03", {4,0, 4,1, 4,2, 4,3, 99,99, 99,99, 0}}, // Vermin
        {"RC_04", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Fire (Silver)
        {"MH_06", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Manhole (Gold)
        {"CN_07", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Helmet (Rev.1 / CN-07 original)
        {"CN_17", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Helmet (Rev.2 / CN-17 revised)
        {"LN_08", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Lion
        {"PR_21", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Parachute
        {"OC_22", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Octopus
        {"IM_03", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Tayny okeana
        {"PP_23", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Popeye (Wide Screen)
        {"IP_05", {1,2, 1,3, 1,6, 1,7, 1,4, 1,5, 0}}, // Judge (Green / Original)
        {"IP_15", {1,2, 1,3, 1,6, 1,7, 1,4, 1,5, 0}}, // Judge (Purple / Revised)
        {"FP_24", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Chef
        {"IM_04", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Vesyolyy povar
        {"MC_25", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Mickey Mouse (Wide Screen)
        {"EG_26", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Egg
        {"IM_53", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Ataka asteroidov
        {"IM_19", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Biathlon 
        {"IM_11", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Circus 
        {"IM_10", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Hockey 
        {"IM_50", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Kosmicheskiy polyot 
        {"IM_32", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Kot-rybolov 
        {"IM_33", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Kvaka-zadavaka 
        {"IM_51", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Morskaja ataka 
        {"IM_49", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Nochnye vorishki 
        {"IM_02", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Nu, pogodi! 
        {"IM_16", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Okhota 
        {"IM_13", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Razvedchiki kosmosa 
        {"IM_22", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Vesyolye futbolisty 
        {"FR_27", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Fire (Wide Screen)
        {"IM_09", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Kosmicheskiy most
        {"SK_10", {0,3, 0,2, 0,1, 0,0, 99,99, 99,99, 8}}, // Super Goal Keeper
        {"SM_11", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Space Mission (Tronica)
        {"SG_21", {3,6, 3,7, 3,8, 3,9, 3,10, 3,11, 8}}, // Spider (Tronica)
        {"IM_23", {2,4, 2,5, 2,6, 2,7, 99,99, 99,99, 0}}, // Autoslalom (Elektronika)

        // Altair (Nauchpribor bootleg, ref "naltair"). Not independently
        // verified - MAME's own driver comment says this unit "uses the
        // same screen as IM-22" (Vesyolye futbolisty), and naltair.svg
        // confirms it: byte-for-byte identical viewBox/docname to
        // vfutbol.svg, i.e. the same LCD scan, not a redrawn approximation.
        // Copied straight from IM_22 below on that basis; needs real-
        // hardware/MAME confirmation like every other unverified entry in
        // this file.
        {"NALTAIR", {2,4, 2,5, 2,6, 2,7, 2,8, 2,9, 2}}, // Altair (Nauchpribor)
    };

    // --- SM510 ---
    static const std::unordered_map<std::string, TimeAddress> sm510_map = {
        {"TL_28", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Turtle Bridge
        {"ID_29", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Fire Attack
        {"SP_30", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Snoopy Tennis
        {"OP_51", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Oil Panic
        {"GH_54", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Green House
        {"MW_56", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario Bros.
        {"LP_57", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Rain Shower
        {"MG_61", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Squish
        {"DJ_101", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Donkey Kong Jr. (New Wide Screen)
        {"ML_102", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario's Cement Factory (New Wide Screen)
        {"TF_104", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Tropical Fish
        {"UD_202", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Crab Grab
        {"SM_91", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Snoopy (Panorama Screen)
        {"PG_92", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Popeye (Panorama Screen)
        {"CJ_93", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Donkey Kong Jr. (Panorama Screen)
        {"IM_12", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Vinni-Pukh (Panorama Screen)
        {"TB_94", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario's Bombs Away
        {"DC_95", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Donkey Kong Circus
        {"MK_96", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mickey Mouse (Panorama Screen)
        {"CM_72", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario's Cement Factory (Table Top, CM-72)
        {"CM_72A", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Mario's Cement Factory (Table Top, CM-72A)
        {"DK_52", {4,5, 4,4, 4,3, 4,2, 4,1, 4,0, 2}}, // Donkey Kong
        {"DM_53", {4,9, 4,10, 4,11, 4,12, 4,7, 4,8, 2}}, // Mickey & Donald
        {"JR_55", {4,4, 4,3, 4,2, 4,1, 4,6, 4,5, 2}}, // Donkey Kong II
        {"TC_58", {0,10, 0,11, 0,12, 0,13, 0,14, 0,15, 8}}, // Life Boat
        {"NH_103", {0,10, 0,11, 0,12, 0,13, 0,14, 0,15, 8}}, // Manhole (New Wide Screen)
        {"BU_201", {0,10, 0,11, 0,12, 0,13, 0,14, 0,15, 8}}, // Spitball Sparky
        {"MG_8", {3,0, 3,1, 3,3, 3,4, 3,6, 3,7, 2}}, // Shuttle Voyage
        {"TG_18", {3,0, 3,1, 3,3, 3,4, 3,6, 3,7, 2}}, // Thief in Garden
        {"CC_38V", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Clever Chicken
        {"DA_37", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Diver's Adventure
        {"MG_9", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Space Rescue
        {"FR_23", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Thunder Ball (Tronica)
    };

    // --- SM511/SM512 ---
    static const std::unordered_map<std::string, TimeAddress> sm511_map = {
        {"PB_59", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Pinball
        {"BJ_60", {0,5, 0,4, 0,3, 0,2, 0,1, 0,0, 24}}, // Black Jack (Only game with 24hr clock)
        {"BD_62", {1,0, 1,1, 1,2, 1,3, 1,4, 1,5, 2}}, // Bomb Sweeper
        {"JB_63", {1,0, 1,1, 1,2, 1,3, 1,4, 1,5, 2}}, // Safe Buster
        {"MV_64", {1,0, 1,1, 1,2, 1,3, 1,4, 1,5, 2}}, // Gold Cliff
        {"ZL_65", {1,3, 1,2, 1,1, 1,0, 1,5, 1,4, 2}}, // Zelda
        {"YM_801", {2,2, 2,3, 2,4, 2,5, 2,6, 2,7, 2}}, // Super Mario Bros. (Crystal Screen)
        {"YM_105", {2,2, 2,3, 2,4, 2,5, 2,6, 2,7, 2}}, // Super Mario Bros. (New Wide Screen)
        {"DR_802", {2,3, 2,4, 2,5, 2,6, 2,7, 2,8, 2}}, // Climber (Crystal Screen)
        {"DR_106", {2,3, 2,4, 2,5, 2,6, 2,7, 2,8, 2}}, // Climber (New Wide Screen)
        {"BF_803", {2,3, 2,4, 2,5, 2,6, 2,1, 2,2, 2}}, // Balloon Fight (Crystal Screen)
        {"BF_107", {2,3, 2,4, 2,5, 2,6, 2,1, 2,2, 2}}, // Balloon Fight (New Wide Screen)
        {"MB_108", {1,8, 1,9, 1,10, 1,11, 1,12, 1,13, 2}}, // Mario The Juggler
        {"BX_301", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Micro Vs. System: Boxing
        {"AK_302", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Micro Vs. System: Donkey Kong 3
        {"HK_303", {1,4, 1,5, 1,6, 1,7, 1,8, 1,9, 8}}, // Micro Vs. System: Donkey Kong Hockey
        {"SA_12", {5,4, 5,5, 5,7, 5,8, 5,10, 5,11, 8}}, // Space Adventure (Tronica)
        // Bassmate Computer (Telko/Nintendo). Unlike every other entry here,
        // found via live RAM inspection in MAME rather than debug_dump_ram_state():
        // dumped the SM511 "data" address space at boot (showing "12:00 AM"),
        // spotted addr 0x2C incrementing by exactly 1 per real second across six
        // 1-second-apart samples (the seconds-units digit, ticking even though
        // never displayed), and read the five cells immediately before it as
        // 01 02 00 00 00 - an exact match for hour_tens=1, hour_units=2,
        // minute_tens=0, minute_units=0, second_tens=0 at "12:00". Addresses
        // 0x27-0x2C convert to (col,line) via this core's ram[8][16] layout
        // (col = addr>>4, line = addr&0xF), confirmed against SM511_2_RAM_COL/LINE.
        // pm_bit=8 is NOT independently verified - hour_tens only ever needs bit0
        // for valid BCD (0 or 1), so any of bits 1-3 is a safe non-colliding
        // choice, matching several other entries above, but confirming which
        // bit actually flips the AM/PM indicator would need the clock to reach
        // noon; the LCD only refreshes from RAM on its own periodic divider
        // tick (see condition_to_update_segment()), not on demand, so poking
        // hour_tens directly and screenshotting immediately after (tried first)
        // never shows a change even when the write demonstrably lands in RAM.
        {"BASSMATE", {2,7, 2,8, 2,9, 2,10, 2,11, 2,12, 8}}, // Bassmate Computer

        // Treasure Island (Tronica, "Super Tri-Screen"). Found via live RAM
        // inspection in MAME, same method as Bassmate above. The center
        // screen shows "12:00" by default (no AM/PM shown, no visible
        // seconds) - dumped the SM511 "data" space right after entering
        // Time mode and found addresses 0x50-0x53 holding 00 02 01 00
        // (minute_units, minute_tens, hour_units, hour_tens at ascending
        // addresses - i.e. DESCENDING significance, the opposite direction
        // from Bassmate's block), an exact match for "12:00" read back to
        // front: hour_tens=1, hour_units=2, minute_tens=0, minute_units=0.
        // second_units is much less certain: address 0x06 was the only
        // cell that ticked at roughly 1/sec once in Time mode (noisy -
        // MAME's own real-time pacing plus this game's continuous
        // background animation made a clean read hard, unlike Bassmate's
        // quiet screen), and it is NOT adjacent to the 0x50-0x53 block, so
        // it may belong to an unrelated counter. second_tens=0x05 is a
        // guess by adjacency to that cell (stayed 0 - a valid tens-of-
        // seconds value - through every sample taken, but never observed
        // long enough to see it actually carry). pm_bit=8 is unverified
        // the same way Bassmate's was, for the same reason (hour_tens only
        // ever needs bit0 for valid BCD, so any of bits 1-3 is safe) -
        // confirming any of this needs real-hardware testing.
        {"TRTREISL", {5,3, 5,2, 5,1, 5,0, 0,5, 0,6, 8}}, // Treasure Island (Tronica)

        // Uchitel' arifmetiki (Elektronika, "uchitari") does NOT get an entry
        // here, unlike every other game in this file. RAM(0,4)-(0,9) does hold
        // a genuine, cleanly-ticking 24-hour clock (found the same way as every
        // other entry: booted the game, watched general RAM once a second for
        // 26 emulated hours) - but a follow-up check proved the display never
        // reads it: two different injected times produced byte-identical lit
        // segments after boot. Whatever that RAM tracks, it isn't wired to the
        // screen. The only thing that DOES move the visible digits is the real
        // Hour/Minute buttons, confirmed by tapping them and watching the
        // segments change in lockstep - so this game's clock is set by
        // switch/source/input/uchitari_clock_setup.h instead, which scripts
        // that exact button sequence on load rather than writing RAM directly.
        //
        // Vesolaya arifmetika ("vesarif") doesn't get an entry either, for
        // the exact same reason - same SM511 family, same symptom (its own
        // general RAM has a cleanly-ticking clock at col=2 that two
        // different injected times proved has zero effect on the display,
        // identical segments either way). Set via the button sequence in
        // switch/source/input/vesarif_clock_setup.h instead.
    };

    // --- SM530 ---
    // All three Nelsonic games share the exact same time-keeping RAM layout
    // (same firmware base, different game ROM code on top) - found via the
    // standalone SM530 harness rather than MAME: booted each game, then
    // watched general RAM (cols 0/3 - mirrored duplicates of each other, cols
    // 4/5 are the LCD segment banks) once a second for 2.2 emulated hours,
    // then a coarser 25-hour pass (every simulated 30 minutes, all 8 columns
    // this time) to catch anything on a longer period. col=0 line=1 rolled
    // 12->1 at exactly the 1-hour mark - a single combined-hour cell (see
    // SM5XX::set_time()'s "col_hour_tens == col_hour_units" sentinel), not
    // split BCD tens/units like every other core here. Lines 2-5 are
    // ordinary BCD digits, each confirmed by its own clean period (600s/60s/
    // 10s/1s) and 0-5 or 0-9 wrap. The 25-hour pass found the AM/PM flag
    // living in its own separate cell, col=0 line=0: bit3 (value 8) flips on
    // at t=43200s (12h - noon) and back off at t=86400s (24h - midnight),
    // confirmed identical across all three games - hence col_pm/line_pm
    // below instead of OR'ing pm_bit into the hour cell (no spare bit left
    // in a single-cell 1-12 value).
    static const std::unordered_map<std::string, TimeAddress> sm530_map = {
        {"NSTARFOX", {0,1, 0,1, 0,2, 0,3, 0,4, 0,5, 8, 0,0}}, // Star Fox (Nelsonic)
        {"NSMB3",    {0,1, 0,1, 0,2, 0,3, 0,4, 0,5, 8, 0,0}}, // Super Mario Bros. 3 (Nelsonic)
        {"NSMW",     {0,1, 0,1, 0,2, 0,3, 0,4, 0,5, 8, 0,0}}, // Super Mario World (Nelsonic)
    };

    auto it = sm5a_map.find(ref_game);
    if (it != sm5a_map.end()) return &it->second;
    it = sm510_map.find(ref_game);
    if (it != sm510_map.end()) return &it->second;
    it = sm511_map.find(ref_game);
    if (it != sm511_map.end()) return &it->second;
    it = sm530_map.find(ref_game);
    if (it != sm530_map.end()) return &it->second;
    return nullptr; // Return nullptr if no specific mapping exists
}

#endif // TIME_ADDRESSES_H
