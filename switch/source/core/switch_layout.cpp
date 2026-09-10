#include "switch_layout.h"

#include <algorithm>

#include "std/GW_ROM.h"
#include "switch_runtime_state.h"
#include "switch_settings.h"

// Verbatim port of yokoi_layout.cpp::rebuild_layout_from_game (Android). The
// Switch renderer always draws in "combined" mode (see switch_screen.cpp), so
// this is the only layout function needed - rebuild_layout_for_menu (Android's
// menu-mode variant) has no Switch equivalent yet.
void rebuild_layout_from_game() {
    g_canvas_w = 1.0f;
    g_canvas_h = 1.0f;
    g_screen_off_x[0] = g_screen_off_x[1] = 0.0f;
    g_screen_off_y[0] = g_screen_off_y[1] = 0.0f;

    if (!g_game || !g_game->segment_info) {
        return;
    }

    uint16_t tex_w = g_game->segment_info[0];
    uint16_t tex_h = g_game->segment_info[1];
    uint16_t scale = g_game->segment_info[2] ? g_game->segment_info[2] : 1;
    uint16_t flags = g_game->segment_info[3];
    uint16_t scr0w = g_game->segment_info[4];
    uint16_t scr0h = g_game->segment_info[5];

    g_double_in_one_screen = (flags & 0x02) != 0;

    g_nb_screen = 1;
    for (const auto& s : g_segments) {
        if (s.screen == 1) {
            g_nb_screen = 2;
            break;
        }
    }

    uint16_t scr1w = scr0w;
    uint16_t scr1h = scr0h;
    if (g_nb_screen == 2) {
        scr1w = g_game->segment_info[6];
        scr1h = g_game->segment_info[7];
    }

    g_segment_info[0] = tex_w;
    g_segment_info[1] = tex_h;
    g_segment_info[2] = scale;
    g_segment_info[3] = flags;
    g_segment_info[4] = scr0w;
    g_segment_info[5] = scr0h;
    g_segment_info[6] = scr1w;
    g_segment_info[7] = scr1h;

    float w0 = (float)scr0w / (float)scale;
    float h0 = (float)scr0h / (float)scale;
    float w1 = (float)scr1w / (float)scale;
    float h1 = (float)scr1h / (float)scale;

    if (g_nb_screen == 1) {
        g_canvas_w = w0 > 1.0f ? w0 : 1.0f;
        g_canvas_h = h0 > 1.0f ? h0 : 1.0f;
    } else if (g_double_in_one_screen) {
        g_canvas_w = (w0 + w1) > 1.0f ? (w0 + w1) : 1.0f;
        g_canvas_h = (std::max(h0, h1)) > 1.0f ? (std::max(h0, h1)) : 1.0f;
        g_screen_off_x[0] = 0.0f;
        g_screen_off_x[1] = w0;
        g_screen_off_y[0] = (g_canvas_h - h0) * 0.5f;
        g_screen_off_y[1] = (g_canvas_h - h1) * 0.5f;
    } else {
        g_canvas_w = (std::max(w0, w1)) > 1.0f ? (std::max(w0, w1)) : 1.0f;
        g_canvas_h = (h0 + h1) > 1.0f ? (h0 + h1) : 1.0f;
        g_screen_off_x[0] = (g_canvas_w - w0) * 0.5f;
        g_screen_off_y[0] = 0.0f;
        g_screen_off_x[1] = (g_canvas_w - w1) * 0.5f;
        g_screen_off_y[1] = h0;
    }

    // Android policy for stacking two "clamshell" screens into vertical panels.
    // Switch always renders combined, but we still need g_top_/g_bottom_canvas_*
    // and g_split_two_screens_to_panels since switch_screen.cpp's console
    // placement and the combined-canvas math both key off them.
    g_split_two_screens_to_panels = (g_nb_screen == 2) && (!g_double_in_one_screen);

    if (g_split_two_screens_to_panels) {
        g_top_canvas_w = w0 > 1.0f ? w0 : 1.0f;
        g_top_canvas_h = h0 > 1.0f ? h0 : 1.0f;
        g_bottom_canvas_w = w1 > 1.0f ? w1 : 1.0f;
        g_bottom_canvas_h = h1 > 1.0f ? h1 : 1.0f;

        g_screen_off_x[0] = 0.0f;
        g_screen_off_y[0] = 0.0f;
        g_screen_off_x[1] = 0.0f;
        g_screen_off_y[1] = 0.0f;
    } else {
        g_top_canvas_w = g_canvas_w;
        g_top_canvas_h = g_canvas_h;

        g_bottom_canvas_w = 0.0f;
        g_bottom_canvas_h = 0.0f;
        // Only reserve canvas space for the console-art strip when it's
        // actually going to be drawn (switch_screen.cpp's matching check) -
        // g_hide_console_art lets the real game screen use that space
        // instead. Never reached for true dual-screen games (this whole
        // branch is g_split_two_screens_to_panels == false), so their real
        // second screen is untouched by this setting either way.
        if (!g_hide_console_art && g_game && g_game->console_info) {
            uint16_t scale2 = g_segment_info[2] ? g_segment_info[2] : 1;
            g_bottom_canvas_w = (float)g_game->console_info[4] / (float)scale2;
            g_bottom_canvas_h = (float)g_game->console_info[5] / (float)scale2;
        }
    }

    g_combined_canvas_w = std::max(g_top_canvas_w, g_bottom_canvas_w);
    if (g_combined_canvas_w < 1.0f) {
        g_combined_canvas_w = 1.0f;
    }

    // Real Game & Watch consoles almost always had a visible physical gap
    // between their two screens (or between a single screen and its
    // console-art strip below) - see g_panel_gap_percent's doc comment in
    // switch_settings.h. Only applied when there's actually something below
    // to space away from: g_bottom_canvas_h is already 0 whenever there
    // isn't (a single-screen game with Console Art hidden), so no separate
    // check is needed here.
    const float panel_gap = (g_bottom_canvas_h > 0.0f)
        ? g_top_canvas_h * ((float)g_panel_gap_percent / 100.0f)
        : 0.0f;

    g_combined_canvas_h = g_top_canvas_h + panel_gap + g_bottom_canvas_h;
    if (g_combined_canvas_h < 1.0f) {
        g_combined_canvas_h = 1.0f;
    }

    g_top_off_x = (g_combined_canvas_w - g_top_canvas_w) * 0.5f;
    g_bottom_off_x = (g_combined_canvas_w - g_bottom_canvas_w) * 0.5f;
    g_bottom_off_y = g_top_canvas_h + panel_gap;
}
