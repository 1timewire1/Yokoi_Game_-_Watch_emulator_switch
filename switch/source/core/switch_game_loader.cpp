#include "switch_game_loader.h"

#include <cstdio>
#include <ctime>
#include <string>

#include "SM5XX/SM5XX.h"
#include "std/GW_ROM.h"
#include "std/debug_log.h"
#include "std/load_file.h"
#include "std/savestate.h"
#include "std/settings.h"

#define get_input_config get_input_config__switch_game_loader_tu
#include "virtual_i_o/virtual_input.h"
#include "virtual_i_o/rzone_io.h"
#undef get_input_config
#include "E0C6200/E0C6S46.h"

#include "audio/switch_sound.h"
#include "audio/lcdonald_sound.h"
#include "yokoi_cpu_utils.h"
#include "switch_layout.h"
#include "switch_runtime_state.h"
#include "switch_segments_state.h"
#include "switch_settings.h"
#include "high_scores.h"
#include "input/uchitari_clock_setup.h"
#include "input/vesarif_clock_setup.h"
#include "input/auto_confirm_press.h"
#include "input/e0c6200_clock_setup.h"
#include "input/brick_clock_setup.h"
#include "pet_time.h"

namespace {

uint16_t find_game_index_by_ref(const std::string& ref) {
    size_t n = get_nb_name();
    if (n > 0xFFFF) n = 0xFFFF;
    for (size_t i = 0; i < n; i++) {
        const GW_rom* g = load_game((uint16_t)i);
        if (g && g->ref == ref) {
            return (uint16_t)i;
        }
    }
    return 0;
}

struct ConfirmButton { const char* ref; int8_t group; int8_t line; bool not_multiplex = false; };

// Games whose clock is set correctly by the normal time_addresses.h
// RAM-injection path but which still need a single button auto-pressed on
// boot before they'll respond to anything else - see auto_confirm_press.h.
// Only ever consulted when g_time_set_mode is Automatic (see below).
//
// BASSMATE uses Compute (PART_SETUP/BUTTON_GAMEA in its Virtual_Input
// class). Every Tronica game except Treasure Island (TRTREISL, unaffected)
// needs the same treatment - Mode for the three with a hidden calculator
// keypad (MG_8/TG_18/SA_12, where Time re-enters the clock-set screen
// instead), Time for the rest. SG_21/FR_23 share SM_11's/MG_9's
// Virtual_Input class and button mapping respectively.
//
// Almost every Nintendo Game & Watch title behaves the same way - Time is
// their confirm button too - except Chef (FP_24), Egg (EG_26), and Mickey
// Mouse Wide Screen (MC_25), none of which gate anything on it (confirmed
// by hand; note EG_26 and MC_25 share MC_25's class exactly, so that
// exemption already covers Egg for free). Three Elektronika titles reuse a
// Nintendo class but were found to need their own confirm press despite
// that shared original not needing one: Tayny okeana (IM_03, OC_22/
// Octopus's class), Vinni-Pukh Panorama Screen (IM_12, CJ_93/DK Jr.
// Panorama's class), and Kosmicheskiy most (IM_09, FR_27/Fire Wide
// Screen's class) - every other Elektronika title was checked and needs no
// confirm press, matching its shared class's original.
//
// Every group/line target below comes straight from that game's own
// Virtual_Input class's PART_SETUP/BUTTON_TIME (or BUTTON_GAMEA, for the
// three Tronica calculator games) case in virtual_input.h - clone/rebadged
// refs (e.g. BF_107, CN_17, YM_105) reuse their parent class's target
// unchanged since get_input_config() maps them to the same class. AC_01/
// RC_04/MT_03 pass not_multiplex=true because those three classes' own
// BUTTON_TIME case does too (see auto_confirm_press.h's doc comment).
constexpr ConfirmButton kAutoConfirmButtons[] = {
    {"BASSMATE", 0, 0},
    {"MG_8", 7, 0}, {"TG_18", 7, 0}, {"SA_12", 7, 0},
    {"CC_38V", 1, 0}, {"DA_37", 1, 0},
    {"SK_10", 3, 0},
    {"SM_11", 3, 0}, {"SG_21", 3, 0},
    {"MG_9", 1, 3}, {"FR_23", 1, 3},
    // Nintendo Game & Watch (all but Chef/Egg/Mickey Mouse Wide Screen).
    {"AC_01", 0, 0, true},
    {"BF_803", 0, 0}, {"BF_107", 0, 0},
    {"BJ_60", 1, 0},
    {"BX_301", 6, 0}, {"AK_302", 6, 0}, {"HK_303", 6, 0},
    {"BD_62", 1, 0},
    {"UD_202", 1, 0},
    {"DR_802", 0, 0}, {"DR_106", 0, 0},
    {"DC_95", 1, 0}, {"MK_96", 1, 0},
    {"DJ_101", 2, 0},
    {"CJ_93", 2, 0},
    {"DK_52", 2, 0},
    {"JR_55", 2, 0},
    {"FR_27", 3, 0},
    {"ID_29", 1, 0},
    {"RC_04", 0, 0, true},
    {"FL_02", 3, 0},
    {"MV_64", 2, 0},
    {"GH_54", 2, 0},
    {"CN_07", 3, 0}, {"CN_17", 3, 0},
    {"IP_05", 3, 0}, {"IP_15", 3, 0},
    {"TC_58", 1, 0},
    {"LN_08", 3, 0},
    {"NH_103", 1, 0},
    {"MH_06", 3, 0},
    {"MW_56", 1, 0},
    {"ML_102", 1, 0},
    {"CM_72", 1, 0}, {"CM_72A", 1, 0},
    {"MB_108", 1, 0},
    {"TB_94", 1, 0},
    {"DM_53", 1, 0},
    {"OC_22", 3, 0},
    {"OP_51", 1, 0},
    {"PR_21", 3, 0},
    {"PB_59", 1, 0},
    {"PP_23", 3, 0},
    {"PG_92", 1, 0},
    {"LP_57", 1, 0},
    {"JB_63", 1, 0},
    {"YM_801", 0, 0}, {"YM_105", 0, 0},
    {"SM_91", 1, 0},
    {"MG_61", 1, 0},
    {"BU_201", 1, 0},
    {"SP_30", 1, 0},
    {"TL_28", 1, 0},
    {"TF_104", 1, 0},
    {"MT_03", 0, 0, true},
    {"ZL_65", 2, 0},
    // Elektronika titles needing their own confirm press despite sharing a
    // Nintendo class whose own original doesn't need one - see comment above.
    {"IM_03", 3, 0}, {"IM_09", 3, 0}, {"IM_12", 2, 0},
};

} // namespace

void switch_load_game_by_index_and_init(uint16_t idx) {
    YOKOI_LOG("switch_load_game_by_index_and_init: idx=%u begin", (unsigned)idx);

    g_game_index = idx;
    g_game = load_game(idx);
    if (!g_game) {
        YOKOI_LOG("switch_load_game_by_index_and_init: load_game(%u) failed", (unsigned)idx);
        return;
    }
    YOKOI_LOG("switch_load_game_by_index_and_init: got GW_rom '%s' ref=%s size_rom=%u mfr=%u",
              g_game->name.c_str(), g_game->ref.c_str(), (unsigned)g_game->size_rom, (unsigned)g_game->manufacturer);

    if (!yokoi_cpu_create_instance(g_cpu, g_game->rom, g_game->size_rom, g_game->size_melody > 0)) {
        YOKOI_LOG("switch_load_game_by_index_and_init: unsupported ROM size / CPU type for %s", g_game->name.c_str());
        return;
    }
    YOKOI_LOG("switch_load_game_by_index_and_init: cpu instance created");

    g_cpu->init();
    YOKOI_LOG("switch_load_game_by_index_and_init: cpu->init() done");

    // Tiger R-Zone cartridges read their buttons through a 74165 shift
    // register on the CPU's R/S/K pins instead of a direct button matrix -
    // see source/virtual_i_o/rzone_io.h. Every other game: no port device.
    g_rzone_io.reset();
    RZoneIO::Type rzone_type;
    if (RZoneIO::type_for_ref(g_game->ref, rzone_type)) {
        g_rzone_io = std::make_unique<RZoneIO>(rzone_type);
        g_cpu->port_device = g_rzone_io.get();
        YOKOI_LOG("switch_load_game_by_index_and_init: R-Zone I/O attached (type %d)", rzone_type == RZoneIO::Type::Type1 ? 1 : 2);
    }
    // E0C6S46/S48 boards differ in OSC3 / P-port wiring - see configure_for_ref().
    // (Type id check rather than dynamic_cast - this build is -fno-rtti.)
    if (g_cpu->get_cpu_type_id() == CPU_TYPE_E0C6S46) {
        static_cast<E0C6S46*>(g_cpu.get())->configure_for_ref(g_game->ref);
    }
    g_cpu->load_rom(g_game->rom, g_game->size_rom);
    YOKOI_LOG("switch_load_game_by_index_and_init: cpu->load_rom() done");
    if (g_game->is_lcdonald_game()) {
        // Drop any decoded .ogg samples cached for whichever McDonald's
        // game was previously loaded (see lcdonald_sound_reset()'s doc
        // comment) - harmless no-op the first time or for every other
        // manufacturer, so this is unconditional rather than diffing
        // against the previous game's ref.
        lcdonald_sound_reset();
    }
    g_cpu->load_rom_melody(g_game->melody, g_game->size_melody);
    g_cpu->load_rom_time_addresses(g_game->ref);

    g_cpu->time_set(false);
    yokoi_cpu_set_time_if_needed(g_cpu.get());

    g_input.reset(get_input_config__switch_game_loader_tu(g_cpu.get(), g_game->ref));
    // set_input_multiplexage() was removed from SM5XX (see SM5XX.h) - the
    // no-multiplex input path it toggled is now set per-call via
    // input_set(..., not_multiplex=true) from virtual_input.h itself.
    YOKOI_LOG("switch_load_game_by_index_and_init: input config %s", g_input ? "ok" : "NULL");

    g_segments.clear();
    if (g_game->segment && g_game->size_segment > 0) {
        g_segments.assign(g_game->segment, g_game->segment + g_game->size_segment);
    }
    g_seg_on.assign(g_segments.size(), 0);
    g_seg_raw_prev.assign(g_segments.size(), 0);
    YOKOI_LOG("switch_load_game_by_index_and_init: %zu segments", g_segments.size());

    rebuild_layout_from_game();
    switch_sound_reconfigure(g_cpu.get());
    YOKOI_LOG("switch_load_game_by_index_and_init: layout+sound reconfigured, entering warmup");

    int warmup_time_grace = kTimeSetGracePeriod;
    for (int i = 0; i < 2000; i++) {
        if (g_cpu->step()) {
            if (warmup_time_grace > 0) {
                warmup_time_grace--;
                g_cpu->time_set(false);
                yokoi_cpu_set_time_if_needed(g_cpu.get());
            }
            if (g_cpu->segments_state_are_update) {
                g_cpu->segments_state_are_update = false;
            }
        }
    }
    YOKOI_LOG("switch_load_game_by_index_and_init: warmup done");

    if (g_cpu) {
        g_cpu->time_set(false);
        yokoi_cpu_set_time_if_needed(g_cpu.get());
    }

    // Everything below is gated behind g_time_set_mode - see its doc
    // comment in switch_settings.h. Under Manual, none of these games get
    // anything pressed on the player's behalf: UCHITARI/VESARIF stay on
    // their default 00:00 boot clock and every confirm-press game stays on
    // its time-display screen until the player presses the button
    // themselves, exactly as un-automated hardware behaves.
    // E0C6S46/S48 games (Tamagotchi, Digital Monster): set the clock through
    // the game's own set screen a few seconds in - unless a save state
    // (loaded right after this function returns) already has it set. See
    // e0c6200_clock_setup.h. begin() also cancels any sequence left running
    // from the previous game, so it's called for every game. Like Pet Time,
    // this is an Experimental Feature.
    e0c6200_clock_setup_begin(g_game->ref,
                              g_time_set_mode == TimeSetMode::Automatic && g_experimental_features_enabled);
    // BrickEmuPy-core watches (Nelsonic Game Watches, Formel 1): leave the
    // boot test screen and set the clock a second in - brick_clock_setup.h.
    brick_clock_setup_begin(g_time_set_mode == TimeSetMode::Automatic &&
                            g_cpu->get_cpu_type_id() == CPU_TYPE_BRICK);
    pet_time_on_game_loaded();

    if (g_time_set_mode == TimeSetMode::Automatic) {
        // UCHITARI/VESARIF don't get a time_addresses.h entry (see that
        // file's comments on these two) - their clocks only respond to real
        // button presses, so they're set via a scripted sequence instead,
        // played out over real gameplay frames by main.cpp rather than
        // started here.
        if (g_game->ref == "UCHITARI" || g_game->ref == "VESARIF") {
            time_t now = time(nullptr);
            tm* t = localtime(&now);
            if (t) {
                if (g_game->ref == "UCHITARI") {
                    uchitari_clock_setup_begin((uint8_t)t->tm_hour, (uint8_t)t->tm_min);
                } else {
                    vesarif_clock_setup_begin((uint8_t)t->tm_hour, (uint8_t)t->tm_min);
                }
            }
        }
        // BASSMATE and the rest of kAutoConfirmButtons already get their
        // clocks set correctly via time_addresses.h - they just need one
        // confirm button auto-pressed afterward to leave the time-display
        // screen (see auto_confirm_press.h and kAutoConfirmButtons above).
        else {
            for (const ConfirmButton& e : kAutoConfirmButtons) {
                if (g_game->ref == e.ref) {
                    auto_confirm_press_begin(e.group, e.line, e.not_multiplex);
                    break;
                }
            }
        }
    }

    g_cpu->segments_state_are_update = true;
    update_segments_from_cpu(g_cpu.get());
    YOKOI_LOG("switch_load_game_by_index_and_init: initial segment state updated");

    g_rate_accu = 0;
    g_time_set_grace_counter = kTimeSetGracePeriod;
    g_hour_reassert_grace_frames = kHourReassertGraceFrames;
    g_texture_generation++;

    YOKOI_LOG("switch_load_game_by_index_and_init: done, loaded '%s' (%s)", g_game->name.c_str(), g_game->ref.c_str());
}

void switch_unload_current_game() {
    if (!g_cpu) {
        return;
    }

    if (g_debug_mode) {
        // Hidden debug mode disables save states entirely - see its doc
        // comment in switch_settings.h. g_last_game_played_ref deliberately
        // doesn't update either, since nothing was actually saved.
        YOKOI_LOG("switch_unload_current_game: debug mode, skipping save for idx=%u ref='%s'",
                  (unsigned)g_game_index, g_game ? g_game->ref.c_str() : "?");
    } else {
        YOKOI_LOG("switch_unload_current_game: saving state for idx=%u ref='%s'",
                  (unsigned)g_game_index, g_game ? g_game->ref.c_str() : "?");
        save_game_state(g_cpu.get(), g_game_index);
        high_score_flush();
        if (g_game) {
            g_last_game_played_ref = g_game->ref;
            switch_settings_save();
        }
    }

    g_cpu.reset();
    g_rzone_io.reset();  // after the CPU that points at it
    g_input.reset();
    g_game = nullptr;
    g_segments.clear();
    g_seg_on.clear();
    g_seg_raw_prev.clear();
}

uint16_t switch_get_default_game_index() {
    size_t n = get_nb_name();
    if (n == 0) {
        return 0;
    }

    // Prefer the actual last-saved game (see switch_unload_current_game())
    // over the per-manufacturer restoration below - it's a more direct
    // answer to "what was I playing", and unlike that mechanism, this one is
    // actually written by the Switch build. find_game_index_by_ref() returns
    // 0 for both "found at index 0" and "not found", so the match is
    // confirmed by re-checking the resolved game's own ref.
    if (!g_last_game_played_ref.empty()) {
        const uint16_t idx = find_game_index_by_ref(g_last_game_played_ref);
        const GW_rom* g = load_game(idx);
        if (g && g->ref == g_last_game_played_ref) {
            return idx;
        }
    }

    const uint8_t mfr = load_last_selected_manufacturer(GW_rom::MANUFACTURER_NINTENDO);
    uint16_t idx = 0;
    if (try_load_last_game_index_for_manufacturer(mfr, &idx)) {
        if (idx < n) {
            return idx;
        }
    }

    idx = find_game_index_by_ref("AC_01");
    if (idx >= n) {
        idx = 0;
    }
    return idx;
}
