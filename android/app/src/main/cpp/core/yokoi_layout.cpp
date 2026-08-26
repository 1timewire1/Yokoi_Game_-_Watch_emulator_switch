#include "yokoi_layout.h"

#include <algorithm>

#include "std/GW_ROM.h"
#include "yokoi_runtime_state.h"

void rebuild_layout_from_game() {
    g_canvas_w = 1.0f;
    g_canvas_h = 1.0f;
    g_screen_off_x[0] = g_screen_off_x[1] = 0.0f;
    g_screen_off_y[0] = g_screen_off_y[1] = 0.0f;

    if (!g_game || !g_game->segment_info) {
        return;
    }

    // segment_info layout (from 3DS renderer):
    // - Single-screen games commonly provide 6 entries:
    //   [0]=texW, [1]=texH, [2]=scaleDiv, [3]=flags, [4]=scr0W, [5]=scr0H
    // - Two-screen games provide 8 entries (adds scr1W/scr1H).
    // We MUST not read beyond what's present, so we only read [6]/[7] if we detect 2 screens.

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

    // Store the values we may re-use elsewhere.
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
        // Default: stack vertically.
        g_canvas_w = (std::max(w0, w1)) > 1.0f ? (std::max(w0, w1)) : 1.0f;
        g_canvas_h = (h0 + h1) > 1.0f ? (h0 + h1) : 1.0f;
        g_screen_off_x[0] = (g_canvas_w - w0) * 0.5f;
        g_screen_off_y[0] = 0.0f;
        g_screen_off_x[1] = (g_canvas_w - w1) * 0.5f;
        g_screen_off_y[1] = h0;
    }

    // Android layout policy:
    // - Always render exactly two vertical panels.
    // - Single-screen games: top = game screen, bottom = console.
    // - Two-screen games that are "double_in_one_screen" (left/right): top = both screens combined, bottom = console.
    // - Other two-screen games (clamshell top/bottom): top = screen0, bottom = screen1 (no console panel).
    g_split_two_screens_to_panels = (g_nb_screen == 2) && (!g_double_in_one_screen);

    if (g_split_two_screens_to_panels) {
        // Top panel is screen 0.
        g_top_canvas_w = w0 > 1.0f ? w0 : 1.0f;
        g_top_canvas_h = h0 > 1.0f ? h0 : 1.0f;
        // Bottom panel is screen 1.
        g_bottom_canvas_w = w1 > 1.0f ? w1 : 1.0f;
        g_bottom_canvas_h = h1 > 1.0f ? h1 : 1.0f;

        // For this mode we treat each screen as its own panel origin.
        g_screen_off_x[0] = 0.0f;
        g_screen_off_y[0] = 0.0f;
        g_screen_off_x[1] = 0.0f;
        g_screen_off_y[1] = 0.0f;
    } else {
        // Top panel holds the whole game canvas (1 screen or both combined).
        g_top_canvas_w = g_canvas_w;
        g_top_canvas_h = g_canvas_h;

        // Bottom panel holds the console (if present).
        g_bottom_canvas_w = 0.0f;
        g_bottom_canvas_h = 0.0f;
        if (g_game && g_game->console_info) {
            uint16_t scale2 = g_segment_info[2] ? g_segment_info[2] : 1;
            // console_info: [0]=texW [1]=texH [2]=u [3]=v [4]=w [5]=h
            g_bottom_canvas_w = (float)g_game->console_info[4] / (float)scale2;
            g_bottom_canvas_h = (float)g_game->console_info[5] / (float)scale2;
        }
    }

    g_combined_canvas_w = std::max(g_top_canvas_w, g_bottom_canvas_w);
    if (g_combined_canvas_w < 1.0f) {
        g_combined_canvas_w = 1.0f;
    }
    g_combined_canvas_h = g_top_canvas_h + g_bottom_canvas_h;
    if (g_combined_canvas_h < 1.0f) {
        g_combined_canvas_h = 1.0f;
    }

    g_top_off_x = (g_combined_canvas_w - g_top_canvas_w) * 0.5f;
    g_bottom_off_x = (g_combined_canvas_w - g_bottom_canvas_w) * 0.5f;
    g_bottom_off_y = g_top_canvas_h;
}

void rebuild_layout_for_menu() {
    // Menu always renders as two stacked panels:
    // - Top: game name/year (provided by Java UI texture)
    // - Bottom: console image
    g_split_two_screens_to_panels = false;

    // RGDS/Android UI canvas: treat each panel as a 640x480 surface.
    g_top_canvas_w = 640.0f;
    g_top_canvas_h = 480.0f;

    // Bottom size based on the selected game's console image.
    g_bottom_canvas_w = 0.0f;
    g_bottom_canvas_h = 0.0f;
    if (g_game && g_game->segment_info) {
        uint16_t scale = g_game->segment_info[2] ? g_game->segment_info[2] : 1;
        g_segment_info[2] = scale;
        if (g_game->console_info) {
            g_bottom_canvas_w = (float)g_game->console_info[4] / (float)scale;
            g_bottom_canvas_h = (float)g_game->console_info[5] / (float)scale;
        }
    }

    g_combined_canvas_w = std::max(g_top_canvas_w, g_bottom_canvas_w);
    if (g_combined_canvas_w < 1.0f) {
        g_combined_canvas_w = 1.0f;
    }
    g_combined_canvas_h = g_top_canvas_h + g_bottom_canvas_h;
    if (g_combined_canvas_h < 1.0f) {
        g_combined_canvas_h = 1.0f;
    }

    g_top_off_x = (g_combined_canvas_w - g_top_canvas_w) * 0.5f;
    g_bottom_off_x = (g_combined_canvas_w - g_bottom_canvas_w) * 0.5f;
    g_bottom_off_y = g_top_canvas_h;
}
