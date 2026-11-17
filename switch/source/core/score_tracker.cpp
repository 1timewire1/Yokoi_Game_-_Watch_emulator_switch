#include "score_tracker.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

#include "high_scores.h"
#include "SM5XX/SM5XX.h"
#include "std/GW_ROM.h"
#include "render/switch_i18n.h"
#include "render/switch_ui.h"
#include "switch_runtime_state.h"
#include "switch_settings.h"

void menu_theme_alert_colors(float& ar, float& ag, float& ab, float& pr, float& pg, float& pb);  // switch_menu.cpp

namespace {

struct SegId {
    uint8_t col, line, word;
};
// One game: the four digits (thousands .. units), each as seven segments in
// the order of the units digit's own list, so every digit reads with the
// same pattern table; 255 = no such segment. `extra`: segments the clock
// lit in a digit that the geometry didn't place - lit, the digit isn't a
// number (a game graphic crossing it), so that frame is skipped.
struct ScoreLayout {
    const char* ref;
    SegId digit[4][7];
    uint8_t table[10];  // bit i = digit[..][i] lit, for 0..9
    // What only the clock lights - AM / PM, the colon (which blinks, so the
    // clock counts as up for a second after any of these was lit).
    SegId clock[4];
    SegId extra[4];
    // Other shapes of 6 / 7 / 9 (a 6 without its top bar...) for the layouts
    // whose segments are in a..g order (found by shape); 0 = none.
    uint8_t alt[3];
    uint8_t modes;  // 2: Game A and Game B; 1: one score
};

const ScoreLayout kLayouts[] = {
#include "score_layouts.inc"
};

const ScoreLayout* g_layout = nullptr;
std::string g_ref;
int g_mode = 0;              // 0 Game A, 1 Game B
int g_last = -1, g_stable = 0;
bool g_in_game = false;
uint32_t g_best_at_start = 0;
bool g_banner_done = false;
int g_banner_frames = 0;
uint32_t g_banner_score = 0;
int g_flush_frames = 0;
int g_clock_hold = 0;

// Achievements: got since loading; waiting for their banner; whether the
// waiting ones may show now (always during the game, or once it ends with
// "When the Game Ends"); the one on screen and its frames left.
const uint32_t kAchPoints[5] = {25, 50, 100, 200, 500};
uint8_t g_ach_session = 0;
uint8_t g_ach_pending = 0;
bool g_ach_release = false;
int g_ach_showing = -1;
int g_ach_frames = 0;
int g_prev_score = -1;  // the last value that counted, to see a new game start

const ScoreLayout* layout_of(const std::string& ref) {
    for (const ScoreLayout& l : kLayouts)
        if (ref == l.ref) return &l;
    return nullptr;
}

bool lit(const SegId& s) {
    if (s.col == 255 || !g_cpu) return false;
    return g_cpu->get_segments_state(s.col, s.line, s.word);
}

const char* mode_key(int m) { return m == 1 ? "b" : "a"; }

// The VFD tabletop games (one score, found by shape) score in tens.
uint32_t ach_scale(const std::string& ref) {
    const ScoreLayout* l = layout_of(ref);
    return l && l->modes == 1 ? 10u : 1u;
}

void ach_award(const std::string& ref, int i) {
    const uint32_t have = high_score_get(ref, "ach");
    const uint32_t bit = 1u << i;
    if (have & bit) return;
    high_score_note(ref, "ach", have | bit);  // only adds bits: always "beats" it
    g_ach_session |= (uint8_t)bit;
    if (g_ach_notify != 2) g_ach_pending |= (uint8_t)bit;
}

void ach_check(const std::string& ref, uint32_t score, bool beat_record) {
    const uint32_t scale = ach_scale(ref);
    for (int i = 0; i < 5; i++)
        if (score >= kAchPoints[i] * scale) ach_award(ref, i);
    if (beat_record) ach_award(ref, 5);
}

// Once a frame: the next waiting achievement, after any record banner.
void ach_tick() {
    if (g_ach_frames > 0 && --g_ach_frames == 0) g_ach_showing = -1;
    if (g_ach_notify == 2) g_ach_pending = 0;
    if (g_ach_showing >= 0 || g_ach_pending == 0 || g_banner_frames > 0) return;
    if (g_ach_notify == 1 && !g_ach_release) return;
    for (int i = 0; i < kAchievementCount; i++) {
        if (g_ach_pending & (1u << i)) {
            g_ach_pending &= (uint8_t)~(1u << i);
            g_ach_showing = i;
            g_ach_frames = 180;  // 3 s
            break;
        }
    }
    if (g_ach_pending == 0) g_ach_release = false;
}

// The number on the four digits, -1 if they aren't one (a blank leading
// digit counts as nothing, a blank units digit as 0).
int read_score(const ScoreLayout& l) {
    for (const SegId& e : l.extra)
        if (lit(e)) return -1;
    int value = 0;
    bool started = false;
    for (int d = 0; d < 4; d++) {
        uint8_t pat = 0;
        for (int i = 0; i < 7; i++)
            if (lit(l.digit[d][i])) pat |= (uint8_t)(1u << i);
        if (pat == 0) {
            if (started || d == 3) value *= 10;
            started = started || d == 3;
            continue;
        }
        int digit = -1;
        for (int k = 0; k < 10; k++)
            if (l.table[k] == pat) digit = k;
        static const int kAltDigit[3] = {6, 7, 9};
        for (int k = 0; k < 3 && digit < 0; k++)
            if (l.alt[k] && l.alt[k] == pat) digit = kAltDigit[k];
        if (digit < 0) return -1;
        value = value * 10 + digit;
        started = true;
    }
    return value;
}

}  // namespace

bool score_tracker_has_records(const std::string& ref) {
    return layout_of(ref) != nullptr || high_score_get(ref, "endless") > 0 || high_score_get(ref, "normal") > 0;
}

std::vector<ScoreRecord> score_tracker_records(const std::string& ref) {
    std::vector<ScoreRecord> out;
    if (const ScoreLayout* l = layout_of(ref)) {
        if (l->modes == 1) {
            out.push_back({"Score", high_score_get(ref, "a")});
        } else {
            out.push_back({"Game A", high_score_get(ref, "a")});
            out.push_back({"Game B", high_score_get(ref, "b")});
        }
    }
    // McDonald's / Burger Bard: both modes once either has a record.
    const uint32_t normal = high_score_get(ref, "normal"), endless = high_score_get(ref, "endless");
    if (normal > 0 || endless > 0) {
        out.push_back({"Normal", normal});
        out.push_back({"Endless", endless});
    }
    return out;
}

void score_tracker_clear(const std::string& ref) {
    high_score_clear(ref);
    high_score_flush();
    if (ref == g_ref) {
        g_best_at_start = 0;
        g_banner_done = true;
        g_ach_session = 0;
        g_ach_pending = 0;
        g_ach_showing = -1;
        g_ach_frames = 0;
    }
}

uint8_t score_tracker_achievements(const std::string& ref) {
    return (uint8_t)high_score_get(ref, "ach");
}

uint8_t score_tracker_session_achievements() { return g_ach_session; }

std::string score_tracker_achievement_name(const std::string& ref, int i) {
    if (i >= 5) return switch_tr("Beat your record");
    char num[16];
    snprintf(num, sizeof(num), "%u ", (unsigned)(kAchPoints[i] * ach_scale(ref)));
    return num + switch_tr("points");
}

void score_tracker_run_ended() {
    if (g_ach_pending) g_ach_release = true;
}

void score_tracker_game_loaded(const std::string& ref) {
    g_ref = ref;
    g_layout = layout_of(ref);
    g_mode = 0;
    g_last = -1;
    g_stable = 0;
    g_in_game = false;
    g_banner_frames = 0;
    g_clock_hold = 0;
    g_ach_session = 0;
    g_ach_pending = 0;
    g_ach_release = false;
    g_ach_showing = -1;
    g_ach_frames = 0;
    g_prev_score = -1;
}

void score_tracker_set_mode(int mode) {
    // Game A / Game B starts a new game: the last one ended.
    if (g_ach_pending) g_ach_release = true;
    if (g_layout && g_layout->modes == 1) return;  // one score whatever is pressed
    if (mode != g_mode) {
        g_mode = mode ? 1 : 0;
        g_in_game = false;  // a new game: its record to beat is this mode's
    }
}

void score_tracker_lcd_start(const std::string& ref, bool endless) {
    g_best_at_start = high_score_get(ref, endless ? "endless" : "normal");
    g_banner_done = false;
}

void score_tracker_note_lcd(const std::string& ref, bool endless, uint32_t score) {
    if (!g_records_enabled) return;
    const bool beat = high_score_note(ref, endless ? "endless" : "normal", score) && g_best_at_start > 0;
    if (beat && !g_banner_done) {
        g_banner_done = true;
        g_banner_frames = 120;  // 2 s
        g_banner_score = score;
    }
    ach_check(ref, score, beat);
}

uint32_t score_tracker_lcd_hi(const std::string& ref, bool endless, bool on_start_screen) {
    if (!g_records_enabled || g_record_hud == 0) return 0;
    if (g_record_hud == 1 && !on_start_screen) return 0;  // When Beaten: only before a run
    return high_score_get(ref, endless ? "endless" : "normal");
}

void score_tracker_update() {
    if (g_banner_frames > 0) g_banner_frames--;
    ach_tick();
    if (!g_records_enabled || !g_layout || !g_cpu || !g_game || g_game->ref != g_ref) return;
    const ScoreLayout& l = *g_layout;
    // The clock is up (AM / PM / colon lit lately): no game. Also the
    // all-lit test screen.
    for (const SegId& c : l.clock)
        if (lit(c)) g_clock_hold = 60;
    if (g_clock_hold > 0) {
        g_clock_hold--;
        if (g_in_game) {
            high_score_flush();
            if (g_ach_pending) g_ach_release = true;  // the game ended
        }
        g_in_game = false;
        g_last = -1;
        g_stable = 0;
        return;
    }
    const int v = read_score(l);
    if (v < 0) {
        g_stable = 0;
        return;
    }
    if (v != g_last) {
        g_last = v;
        g_stable = 1;
        return;
    }
    if (++g_stable != 6) return;  // ~0.1 s on screen: not a frame mid-redraw
    // The score went down: a new game started, the last one ended.
    if (v < g_prev_score && g_ach_pending) g_ach_release = true;
    g_prev_score = v;
    if (!g_in_game) {
        g_in_game = true;
        g_best_at_start = high_score_get(g_ref, mode_key(g_mode));
        g_banner_done = false;
    }
    const bool beat = high_score_note(g_ref, mode_key(g_mode), (uint32_t)v);
    ach_check(g_ref, (uint32_t)v, beat && g_best_at_start > 0);
    if (beat) {
        if (g_best_at_start > 0 && !g_banner_done) {
            g_banner_done = true;
            g_banner_frames = 120;  // 2 s
        }
        g_banner_score = (uint32_t)v;
        if (g_flush_frames <= 0) {
            high_score_flush();
            g_flush_frames = 600;
        }
    }
    if (g_flush_frames > 0) g_flush_frames--;
}

void score_tracker_render(int viewport_w, int viewport_h) {
    if (!g_records_enabled || !g_game || g_game->ref != g_ref) return;
    // The McDonald's games draw their own record (render/lcdonald_hud.cpp):
    // only the banner here.
    const bool box = g_record_hud == 2 && g_layout;
    const bool banner = g_record_hud != 0 && g_banner_frames > 0;
    const bool ach = g_ach_showing >= 0 && g_ach_frames > 0;
    if (!box && !banner && !ach) return;
    int logical_w = viewport_w, logical_h = viewport_h;
    // Same rotation-aware space as render/lcdonald_hud.cpp.
    if (g_screen_rotation != ScreenRotation::Deg0) std::swap(logical_w, logical_h);
    switch_ui_set_rotation((int)g_screen_rotation);
    switch_ui_begin_frame(logical_w, logical_h);
    if (box) {
        const uint32_t best = high_score_get(g_ref, mode_key(g_mode));
        char num[16];
        snprintf(num, sizeof(num), "%04u", (unsigned)best);
        const std::string title = switch_tr(!g_layout || g_layout->modes == 1 ? "HIGH SCORE" : g_mode ? "HIGH SCORE - GAME B" : "HIGH SCORE - GAME A");
        const float ts = 15.0f, ns = 32.0f;
        const float w = std::max(switch_ui_text_width(title, ts, true), switch_ui_text_width(num, ns, true)) + 32.0f;
        const float x = (float)logical_w - 24.0f - w, y = 20.0f, h = ts + ns + 26.0f;
        switch_ui_draw_rect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, 1.0f, 0.77f, 0.31f, 0.9f);
        switch_ui_draw_rect(x, y, w, h, 0.0f, 0.0f, 0.0f, 0.78f);
        switch_ui_draw_text(x + w - 16.0f - switch_ui_text_width(title, ts, true), y + 8.0f, ts, 1.0f, 0.77f, 0.31f, 1.0f, title, true);
        switch_ui_draw_text(x + w - 16.0f - switch_ui_text_width(num, ns, true), y + 12.0f + ts, ns, 1.0f, 1.0f, 1.0f, 1.0f, num, true);
    }
    // Record / achievement alerts: gold, or the menu theme's accent and
    // panel (Settings > Alert Colour).
    float ar = 1.0f, ag = 0.82f, ab = 0.36f, pr = 0.08f, pg = 0.08f, pb = 0.09f;
    if (g_alert_color == 1) menu_theme_alert_colors(ar, ag, ab, pr, pg, pb);
    // A card: icon, small grey heading, big name; star = the record's trophy.
    auto alert_card = [&](const std::string& head, const std::string& name, float y, float a, bool star) {
        const float hs = 15.0f, ns = 28.0f, medal = hs + ns + 4.0f;
        const float tw = std::max(switch_ui_text_width(head, hs, true), switch_ui_text_width(name, ns, true));
        const float w = 20.0f + medal + 16.0f + tw + 24.0f, h = medal + 24.0f;
        const float x = ((float)logical_w - w) * 0.5f;
        switch_ui_draw_rect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, ar, ag, ab, 0.95f * a);
        switch_ui_draw_rect(x, y, w, h, pr, pg, pb, 0.92f * a);
        const float mx = x + 20.0f, my = y + 12.0f;
        if (star) {
            // A trophy, all rectangles: cup with its handles, stem, base.
            const float m = medal;
            switch_ui_draw_rect(mx + m * 0.2f, my, m * 0.6f, m * 0.46f, ar, ag, ab, a);                 // cup
            switch_ui_draw_rect(mx + m * 0.3f, my + m * 0.46f, m * 0.4f, m * 0.08f, ar, ag, ab, a);     // cup bottom
            switch_ui_draw_rect(mx, my + m * 0.06f, m * 0.2f, m * 0.08f, ar, ag, ab, a);               // left handle
            switch_ui_draw_rect(mx, my + m * 0.06f, m * 0.08f, m * 0.26f, ar, ag, ab, a);
            switch_ui_draw_rect(mx, my + m * 0.26f, m * 0.2f, m * 0.08f, ar, ag, ab, a);
            switch_ui_draw_rect(mx + m * 0.8f, my + m * 0.06f, m * 0.2f, m * 0.08f, ar, ag, ab, a);    // right handle
            switch_ui_draw_rect(mx + m * 0.92f, my + m * 0.06f, m * 0.08f, m * 0.26f, ar, ag, ab, a);
            switch_ui_draw_rect(mx + m * 0.8f, my + m * 0.26f, m * 0.2f, m * 0.08f, ar, ag, ab, a);
            switch_ui_draw_rect(mx + m * 0.44f, my + m * 0.54f, m * 0.12f, m * 0.22f, ar, ag, ab, a);  // stem
            switch_ui_draw_rect(mx + m * 0.22f, my + m * 0.76f, m * 0.56f, m * 0.1f, ar, ag, ab, a);   // base
            switch_ui_draw_rect(mx + m * 0.14f, my + m * 0.86f, m * 0.72f, m * 0.14f, ar, ag, ab, a);
        } else {
            // A medal on its ribbon, all rectangles (same pixel style as the trophy).
            static const float kMedal[][5] = {
                {.06f, 0, .24f, .12f, 1}, {.7f, 0, .24f, .12f, 1}, {.16f, .12f, .22f, .12f, 1}, {.62f, .12f, .22f, .12f, 1},
                {.28f, .24f, .2f, .1f, 1}, {.52f, .24f, .2f, .1f, 1},                                        // ribbon
                {.34f, .36f, .32f, .08f, 1}, {.24f, .44f, .52f, .08f, 1}, {.18f, .52f, .64f, .32f, 1},
                {.24f, .84f, .52f, .08f, 1}, {.34f, .92f, .32f, .08f, 1},                                     // disc
                {.34f, .56f, .32f, .24f, 0}, {.42f, .6f, .16f, .16f, 1},                                      // its ring
            };
            const float m = medal;
            for (const auto& r : kMedal) {
                if (r[4] != 0) switch_ui_draw_rect(mx + m * r[0], my + m * r[1], m * r[2], m * r[3], ar, ag, ab, a);
                else switch_ui_draw_rect(mx + m * r[0], my + m * r[1], m * r[2], m * r[3], pr, pg, pb, a);
            }
        }
        const float tx = mx + medal + 16.0f;
        switch_ui_draw_text(tx, y + 10.0f, hs, 0.75f, 0.75f, 0.75f, a, head, true);
        switch_ui_draw_text(tx, y + 14.0f + hs, ns, ar, ag, ab, a, name, true);
    };
    if (banner) {
        // Top centre, small: "NEW HIGH SCORE!" and the score (2 s).
        char num[16];
        snprintf(num, sizeof(num), "%04u", (unsigned)g_banner_score);
        const float a = g_banner_frames < 30 ? (float)g_banner_frames / 30.0f : 1.0f;
        alert_card(switch_tr("NEW HIGH SCORE!"), num, 24.0f, a, true);
    }
    if (ach) {
        // Bottom centre: a medal, "ACHIEVEMENT UNLOCKED" and its name.
        const float a = g_ach_frames < 30 ? (float)g_ach_frames / 30.0f : 1.0f;
        alert_card(switch_tr("ACHIEVEMENT UNLOCKED"), score_tracker_achievement_name(g_ref, g_ach_showing), (float)logical_h - 71.0f - 28.0f, a, false);
    }
}
