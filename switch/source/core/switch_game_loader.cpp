#include "switch_game_loader.h"

#include <cstdio>
#include <string>

#include "SM5XX/SM5XX.h"
#include "std/GW_ROM.h"
#include "std/debug_log.h"
#include "std/load_file.h"
#include "std/savestate.h"
#include "std/settings.h"

#define get_input_config get_input_config__switch_game_loader_tu
#include "virtual_i_o/virtual_input.h"
#undef get_input_config

#include "audio/switch_sound.h"
#include "yokoi_cpu_utils.h"
#include "switch_layout.h"
#include "switch_runtime_state.h"
#include "switch_segments_state.h"
#include "switch_settings.h"

namespace {

uint8_t find_game_index_by_ref(const std::string& ref) {
    size_t n = get_nb_name();
    if (n > 255) n = 255;
    for (size_t i = 0; i < n; i++) {
        const GW_rom* g = load_game((uint8_t)i);
        if (g && g->ref == ref) {
            return (uint8_t)i;
        }
    }
    return 0;
}

} // namespace

void switch_load_game_by_index_and_init(uint8_t idx) {
    YOKOI_LOG("switch_load_game_by_index_and_init: idx=%u begin", (unsigned)idx);

    g_game_index = idx;
    g_game = load_game(idx);
    if (!g_game) {
        YOKOI_LOG("switch_load_game_by_index_and_init: load_game(%u) failed", (unsigned)idx);
        return;
    }
    YOKOI_LOG("switch_load_game_by_index_and_init: got GW_rom '%s' ref=%s size_rom=%u mfr=%u",
              g_game->name.c_str(), g_game->ref.c_str(), (unsigned)g_game->size_rom, (unsigned)g_game->manufacturer);

    if (!yokoi_cpu_create_instance(g_cpu, g_game->rom, (uint16_t)g_game->size_rom)) {
        YOKOI_LOG("switch_load_game_by_index_and_init: unsupported ROM size / CPU type for %s", g_game->name.c_str());
        return;
    }
    YOKOI_LOG("switch_load_game_by_index_and_init: cpu instance created");

    g_cpu->init();
    YOKOI_LOG("switch_load_game_by_index_and_init: cpu->init() done");
    g_cpu->load_rom(g_game->rom, g_game->size_rom);
    YOKOI_LOG("switch_load_game_by_index_and_init: cpu->load_rom() done");
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

    g_cpu->segments_state_are_update = true;
    update_segments_from_cpu(g_cpu.get());
    YOKOI_LOG("switch_load_game_by_index_and_init: initial segment state updated");

    g_rate_accu = 0;
    g_time_set_grace_counter = kTimeSetGracePeriod;
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
        if (g_game) {
            g_last_game_played_ref = g_game->ref;
            switch_settings_save();
        }
    }

    g_cpu.reset();
    g_input.reset();
    g_game = nullptr;
    g_segments.clear();
    g_seg_on.clear();
    g_seg_raw_prev.clear();
}

uint8_t switch_get_default_game_index() {
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
        const uint8_t idx = find_game_index_by_ref(g_last_game_played_ref);
        const GW_rom* g = load_game(idx);
        if (g && g->ref == g_last_game_played_ref) {
            return idx;
        }
    }

    const uint8_t mfr = load_last_selected_manufacturer(GW_rom::MANUFACTURER_NINTENDO);
    uint8_t idx = 0;
    if (try_load_last_game_index_for_manufacturer(mfr, &idx)) {
        if (idx < (uint8_t)n) {
            return idx;
        }
    }

    idx = find_game_index_by_ref("AC_01");
    if (idx >= (uint8_t)n) {
        idx = 0;
    }
    return idx;
}
