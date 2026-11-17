#include "pet_time.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <string>
#include <utility>

#include <switch.h>

#include "BrickCores/BrickCore.h"
#include "E0C6200/E0C6S46.h"
#include "std/debug_log.h"
#include "std/savestate.h"

#include "e0c6200_clock.h"
#include "input/brick_clock_setup.h"
#include "input/e0c6200_clock_setup.h"
#include "render/switch_ui.h"
#include "render/switch_i18n.h"
#include "switch_runtime_state.h"
#include "switch_segments_state.h"
#include "switch_settings.h"

namespace {

constexpr uint64_t kTicksPerSecond = 32768;   // E0C6S46 OSC1, BrickCore::kTickHz
static_assert(kTicksPerSecond == BrickCore::kTickHz, "Pet Time slices both kinds of pet in the same ticks");
constexpr uint64_t kSliceTicks = kTicksPerSecond / 4;
constexpr uint64_t kFrameBudgetNs = 12'000'000;  // leave ~4.6 ms of each 60 fps frame for rendering
constexpr int64_t kGapThresholdSeconds = 3;      // a mid-play stall at least this long counts as time away
constexpr uint64_t kOverlayDelayNs = 400'000'000;  // short catch-ups finish before any overlay shows

struct CatchUp {
    bool active = false;
    uint64_t total_ticks = 0;
    uint64_t done_ticks = 0;
    int64_t skipped_seconds = 0;  // beyond the cap - jumped over, not emulated
    uint64_t started_tick = 0;    // armGetSystemTick() when it began
    bool simulate = false;        // Simulate Time rather than time away
    bool keep_clock = false;      // ...under Paused: leave the clock ahead
};

CatchUp g_catch;
int64_t g_last_frame_wall = 0;  // time() of the last emulated frame, 0 = none yet

// The running pet, if Pet Time applies to it: an E0C6S46/S48 with a known
// clock layout (core/e0c6200_clock.h), or a BrickEmuPy-core game marked as
// a pet ("clock.pet" - Apollo 2 in 1, see brick_pet_clock.h).
struct Pet {
    E0C6S46* e0c = nullptr;
    const E0C6200ClockLayout* layout = nullptr;
    BrickCore* brick = nullptr;

    explicit operator bool() const { return e0c || brick; }
    SM5XX* cpu() const { return e0c ? static_cast<SM5XX*>(e0c) : static_cast<SM5XX*>(brick); }
    bool clock_is_set() const { return e0c ? e0c6200_clock_is_set(e0c, *layout) : brick->clock_is_set(); }
    void clock_write(time_t when) const {
        if (e0c) {
            e0c6200_clock_write(e0c, *layout, when);
            return;
        }
        const tm* t = localtime(&when);
        if (t) {
            brick->set_clock(t->tm_hour, t->tm_min, t->tm_sec);
            brick->set_date(t->tm_mon + 1, t->tm_mday);
        }
    }
    int64_t saved_unix_time() const { return e0c ? e0c->saved_unix_time() : brick->saved_unix_time(); }
    void fast_forward(uint64_t ticks) const {
        if (e0c) e0c->fast_forward(ticks);
        else brick->fast_forward(ticks);
    }
    // Nobody's holding anything while away.
    void release_inputs() const {
        if (e0c) {
            for (int line = 0; line < 8; line++) e0c->set_k_line(line, true);
            e0c->set_reset_line(false);
            e0c->set_sensor_chatter(false);
        } else {
            brick->release_all_buttons();
        }
    }
    // The automatic clock setup owns the first seconds of a fresh start.
    bool setup_busy() const { return e0c ? e0c6200_clock_setup_active() : brick_clock_setup_owns_input(); }
};

Pet current_pet() {
    Pet p;
    if (!g_cpu || !g_game) return p;
    if (g_cpu->get_cpu_type_id() == CPU_TYPE_E0C6S46) {
        p.layout = e0c6200_clock_layout(g_game->ref);
        if (p.layout) p.e0c = static_cast<E0C6S46*>(g_cpu.get());
    } else if (g_cpu->get_cpu_type_id() == CPU_TYPE_BRICK) {
        BrickCore* brick = static_cast<BrickCore*>(g_cpu.get());
        if (brick->is_pet()) p.brick = brick;
    }
    return p;
}

void finish(const Pet& pet) {
    if (!(g_catch.simulate && g_catch.keep_clock)) {
        pet.clock_write(time(nullptr));  // also covers the time the catch-up itself took
    }
    pet.cpu()->segments_state_are_update = true;
    update_segments_from_cpu(pet.cpu());
    g_catch = CatchUp{};
    g_last_frame_wall = (int64_t)time(nullptr);
}

// `away` seconds passed without the pet running.
void apply_time_away(int64_t away, const char* why) {
    // Experimental Features off = as if Paused (the stored mode is kept).
    if (away <= 0 || !g_experimental_features_enabled || g_pet_time_mode == PetTimeMode::Paused) return;
    const Pet pet = current_pet();
    if (!pet || !pet.clock_is_set()) return;  // pet never started - nothing to age

    const time_t now = time(nullptr);
    if (g_pet_time_mode == PetTimeMode::ClockSync) {
        pet.clock_write(now);
        YOKOI_LOG("pet_time: %s, %lld s away - clock synced", why, (long long)away);
        return;
    }

    const int64_t cap = g_pet_time_cap_hours ? (int64_t)g_pet_time_cap_hours * 3600 : away;
    const int64_t emulate = std::min(away, cap);
    g_catch = CatchUp{};
    g_catch.skipped_seconds = away - emulate;
    if (g_catch.skipped_seconds > 0) {
        // Jump over the part beyond the cap first, so the emulated stretch
        // ends exactly at now with the pet's day lined up with the real one.
        pet.clock_write(now - (time_t)emulate);
    }
    pet.release_inputs();
    g_catch.total_ticks = (uint64_t)emulate * kTicksPerSecond;
    g_catch.started_tick = armGetSystemTick();
    g_catch.active = true;
    YOKOI_LOG("pet_time: %s, %lld s away - catching up %lld s (%lld s beyond the cap skipped)", why,
              (long long)away, (long long)emulate, (long long)g_catch.skipped_seconds);
}

std::string duration_text(uint64_t seconds) {
    const uint64_t d = seconds / 86400, h = seconds / 3600 % 24, m = seconds / 60 % 60, s = seconds % 60;
    char buf[48];
    if (d) snprintf(buf, sizeof(buf), "%llud %lluh %llum", (unsigned long long)d, (unsigned long long)h, (unsigned long long)m);
    else if (h) snprintf(buf, sizeof(buf), "%lluh %02llum", (unsigned long long)h, (unsigned long long)m);
    else snprintf(buf, sizeof(buf), "%llum %02llus", (unsigned long long)m, (unsigned long long)s);
    return buf;
}

void draw_centered(float cx, float y, float size, float a, const std::string& text, bool bold = false) {
    switch_ui_draw_text(cx - switch_ui_text_width(text, size, bold) * 0.5f, y, size, 1.0f, 1.0f, 1.0f, a, text, bold);
}

void draw_skip_hint(float x_right, float y, float size) {
    const std::string text = "Skip";
    const float tw = switch_ui_text_width(text, size);
    const float iw = switch_ui_button_icon_width(size, SwitchButtonIcon::B);
    switch_ui_draw_button_icon(x_right - tw - 8.0f - iw, y, size, 1.0f, 1.0f, 1.0f, 0.95f, SwitchButtonIcon::B);
    switch_ui_draw_text(x_right - tw, y, size, 1.0f, 1.0f, 1.0f, 0.95f, text);
}

// ---- drawing helpers for the Clock / Day & Night / Screen & Bar views ----
void fill_circle(float cx, float cy, float r, float red, float green, float blue, float a) {
    float xy[2 * 40];
    for (int i = 0; i < 40; i++) {
        const float t = 6.2831853f * (float)i / 40.0f;
        xy[2 * i] = cx + std::cos(t) * r;
        xy[2 * i + 1] = cy + std::sin(t) * r;
    }
    switch_ui_draw_polygon(xy, 40, red, green, blue, a);
}

void thick_line(float x0, float y0, float x1, float y1, float w, float red, float green, float blue, float a) {
    const float dx = x1 - x0, dy = y1 - y0, len = std::sqrt(dx * dx + dy * dy);
    if (len <= 0.0f) return;
    const float nx = -dy / len * w * 0.5f, ny = dx / len * w * 0.5f;
    const float xy[8] = {x0 + nx, y0 + ny, x1 + nx, y1 + ny, x1 - nx, y1 - ny, x0 - nx, y0 - ny};
    switch_ui_draw_polygon(xy, 4, red, green, blue, a);
}

// An arc as short segments (a progress ring).
void arc_line(float cx, float cy, float r, float from, float to, float w, float red, float green, float blue, float a) {
    const int n = std::max(2, (int)((to - from) / 0.08f));
    for (int i = 0; i < n; i++) {
        const float t0 = from + (to - from) * i / n, t1 = from + (to - from) * (i + 1) / n;
        thick_line(cx + std::cos(t0) * r, cy + std::sin(t0) * r, cx + std::cos(t1) * r, cy + std::sin(t1) * r, w, red, green, blue, a);
    }
}

// The pet's own time right now in the catch-up: time away ends at "now",
// Simulate Time starts there.
time_t pet_time_now(uint64_t done_s, uint64_t total_s) {
    const double real_elapsed = armTicksToNs(armGetSystemTick() - g_catch.started_tick) / 1e9;
    const time_t began = time(nullptr) - (time_t)real_elapsed;
    return g_catch.simulate ? began + (time_t)done_s : began - (time_t)total_s + (time_t)done_s;
}

void draw_progress_line(float cx, float y, uint64_t done_s, uint64_t total_s) {
    const float frac = total_s ? (float)done_s / (float)total_s : 1.0f;
    char pct[16];
    snprintf(pct, sizeof(pct), "  -  %d %%", (int)(frac * 100.0f + 0.5f));
    draw_centered(cx, y, 24.0f, 1.0f, duration_text(done_s) + " / " + duration_text(total_s) + pct, true);
}

void draw_header(float cx, float y) {
    draw_centered(cx, y, 34.0f, 1.0f, g_catch.simulate ? "Simulating time" : "Catching up on time away", true);
}

void render_clock_view(int w, int h, uint64_t done_s, uint64_t total_s) {
    switch_ui_draw_rect(0.0f, 0.0f, (float)w, (float)h, 0.11f, 0.11f, 0.125f, 1.0f);
    const float cx = (float)w * 0.5f;
    draw_header(cx, 40.0f);
    const float cy = (float)h * 0.5f + 6.0f, R = std::min(170.0f, (float)h * 0.27f);
    const float frac = total_s ? (float)done_s / (float)total_s : 1.0f;
    fill_circle(cx, cy, R + 14.0f, 0.2f, 0.2f, 0.22f, 1.0f);
    arc_line(cx, cy, R + 8.0f, -1.5707963f, -1.5707963f + 6.2831853f * frac, 9.0f, 0.04f, 0.75f, 1.0f, 1.0f);
    fill_circle(cx, cy, R, 0.925f, 0.91f, 0.86f, 1.0f);
    for (int i = 0; i < 12; i++) {
        const float a = 6.2831853f * i / 12.0f - 1.5707963f;
        const float r2 = R - (i % 3 == 0 ? 30.0f : 22.0f);
        thick_line(cx + std::cos(a) * (R - 12), cy + std::sin(a) * (R - 12), cx + std::cos(a) * r2, cy + std::sin(a) * r2,
                   i % 3 == 0 ? 5.0f : 3.0f, 0.16f, 0.16f, 0.16f, 1.0f);
    }
    const time_t t = pet_time_now(done_s, total_s);
    const tm* lt = localtime(&t);
    const float hours = lt ? (float)(lt->tm_hour % 12) + lt->tm_min / 60.0f : 0.0f;
    const float mins = lt ? (float)lt->tm_min + lt->tm_sec / 60.0f : 0.0f;
    const float ha = 6.2831853f * hours / 12.0f - 1.5707963f, ma = 6.2831853f * mins / 60.0f - 1.5707963f;
    thick_line(cx, cy, cx + std::cos(ha) * R * 0.55f, cy + std::sin(ha) * R * 0.55f, 9.0f, 0.12f, 0.12f, 0.12f, 1.0f);
    thick_line(cx, cy, cx + std::cos(ma) * R * 0.82f, cy + std::sin(ma) * R * 0.82f, 6.0f, 0.12f, 0.12f, 0.12f, 1.0f);
    fill_circle(cx, cy, 10.0f, 0.8f, 0.16f, 0.16f, 1.0f);
    if (lt) {
        char hm[16];
        snprintf(hm, sizeof(hm), "%02d:%02d", lt->tm_hour, lt->tm_min);
        const float tw = switch_ui_text_width(hm, 24.0f, true);
        switch_ui_draw_text(cx - tw * 0.5f, cy + R * 0.38f, 24.0f, 0.3f, 0.3f, 0.3f, 1.0f, hm, true);
    }
    draw_progress_line(cx, cy + R + 34.0f, done_s, total_s);
}

void render_daynight_view(int w, int h, uint64_t done_s, uint64_t total_s) {
    switch_ui_draw_rect(0.0f, 0.0f, (float)w, (float)h, 0.11f, 0.11f, 0.125f, 1.0f);
    const float cx = (float)w * 0.5f;
    draw_header(cx, 40.0f);
    const time_t t = pet_time_now(done_s, total_s);
    const tm* lt = localtime(&t);
    const float hour = lt ? (float)lt->tm_hour + lt->tm_min / 60.0f : 12.0f;
    // Sky colour: night blue -> dawn orange -> day blue -> dusk orange.
    const bool day = hour >= 6.0f && hour < 18.0f;
    const float sun = day ? std::sin((hour - 6.0f) / 12.0f * 3.1415926f) : 0.0f;  // 0 at the horizon, 1 at noon
    float r1, g1, b1, r2, g2, b2;
    if (day) {
        r1 = 0.95f - 0.55f * sun; g1 = 0.6f + 0.15f * sun; b1 = 0.4f + 0.55f * sun;
        r2 = 0.98f - 0.3f * sun;  g2 = 0.75f + 0.1f * sun; b2 = 0.6f + 0.35f * sun;
    } else {
        r1 = 0.05f; g1 = 0.07f; b1 = 0.18f;
        r2 = 0.12f; g2 = 0.14f; b2 = 0.3f;
    }
    const float x0 = std::max(40.0f, cx - 500.0f), x1 = std::min((float)w - 40.0f, cx + 500.0f);
    const float y0 = 110.0f, y1 = (float)h - 200.0f;
    const int bands = 24;
    for (int i = 0; i < bands; i++) {
        const float t2 = (float)i / (bands - 1);
        switch_ui_draw_rect(x0, y0 + (y1 - y0) * i / bands, x1 - x0, (y1 - y0) / bands + 1.0f,
                            r1 + (r2 - r1) * t2, g1 + (g2 - g1) * t2, b1 + (b2 - b1) * t2, 1.0f);
    }
    // Sun by day (6-18 h), moon by night (18-6 h), on an arc over the ground.
    const float ground = y1 - 60.0f;
    const float p = day ? (hour - 6.0f) / 12.0f : std::fmod(hour + 6.0f, 24.0f) / 12.0f;
    const float ang = 3.1415926f * (1.0f + p);
    const float ax = (x0 + x1) * 0.5f, rx = (x1 - x0) * 0.5f - 70.0f, ry = ground - y0 - 50.0f;
    const float bx = ax + std::cos(ang) * rx, by = ground + std::sin(ang) * ry;
    if (day) {
        fill_circle(bx, by, 46.0f, 1.0f, 0.9f, 0.6f, 0.45f);
        fill_circle(bx, by, 26.0f, 1.0f, 0.98f, 0.86f, 1.0f);
    } else {
        fill_circle(bx, by, 24.0f, 0.93f, 0.93f, 0.85f, 1.0f);
        fill_circle(bx + 10.0f, by - 6.0f, 20.0f, r1, g1, b1, 1.0f);  // crescent
        for (int i = 0; i < 14; i++) {  // a few fixed stars
            const float sx = x0 + 40.0f + std::fmod(i * 173.0f, x1 - x0 - 80.0f), sy = y0 + 20.0f + std::fmod(i * 97.0f, ground - y0 - 80.0f);
            fill_circle(sx, sy, 2.0f, 1.0f, 1.0f, 1.0f, 0.8f);
        }
    }
    switch_ui_draw_rect(x0, ground, x1 - x0, y1 - ground, day ? 0.16f : 0.06f, day ? 0.24f : 0.1f, day ? 0.18f : 0.08f, 1.0f);
    // One dot per day (up to 10), the current one highlighted.
    const int days = (int)std::min<uint64_t>(10, total_s / 86400 + 1);
    const int today = (int)std::min<uint64_t>((uint64_t)days - 1, done_s / 86400);
    for (int i = 0; i < days; i++) {
        const float dx = cx + (i - (days - 1) * 0.5f) * 36.0f;
        if (i == today) fill_circle(dx, y1 + 30.0f, 9.0f, 0.04f, 0.75f, 1.0f, 1.0f);
        else fill_circle(dx, y1 + 30.0f, 9.0f, 0.3f, 0.3f, 0.34f, 1.0f);
    }
    const char* part = !day ? "Night" : hour < 12.0f ? "Morning" : hour < 16.0f ? "Afternoon" : "Evening";
    char line[64];
    snprintf(line, sizeof(line), " %d / %d  -  ", today + 1, days);
    draw_centered(cx, y1 + 52.0f, 20.0f, 0.75f, switch_tr("Day") + line + switch_tr(part));
    draw_progress_line(cx, y1 + 90.0f, done_s, total_s);
}

void render_screenbar_view(int w, int h, uint64_t done_s, uint64_t total_s) {
    // The pet's own LCD stays visible (updated every frame); a panel at the
    // bottom carries the title, the bar and the speed.
    const float cx = (float)w * 0.5f;
    const float ph = 150.0f, py = (float)h - ph;
    switch_ui_draw_rect(0.0f, py, (float)w, ph, 0.0f, 0.0f, 0.0f, 0.72f);
    draw_centered(cx, py + 14.0f, 26.0f, 1.0f, g_catch.simulate ? "Simulating time" : "Catching up on time away", true);
    const float bar_w = std::min(720.0f, (float)w - 80.0f), bar_h = 20.0f, by = py + 58.0f;
    const float frac = total_s ? (float)done_s / (float)total_s : 1.0f;
    switch_ui_draw_rect(cx - bar_w * 0.5f, by, bar_w, bar_h, 1.0f, 1.0f, 1.0f, 0.2f);
    switch_ui_draw_rect(cx - bar_w * 0.5f, by, bar_w * frac, bar_h, 0.04f, 0.75f, 1.0f, 1.0f);
    draw_progress_line(cx, by + 34.0f, done_s, total_s);
    // How many times faster than real time it's running.
    const double real_elapsed = armTicksToNs(armGetSystemTick() - g_catch.started_tick) / 1e9;
    if (real_elapsed > 0.5) {
        char sp[24];
        snprintf(sp, sizeof(sp), "x%d", (int)(done_s / real_elapsed + 0.5));
        const float tw = switch_ui_text_width(sp, 26.0f, true);
        switch_ui_draw_rect((float)w - 34.0f - tw, 14.0f, tw + 20.0f, 40.0f, 0.0f, 0.0f, 0.0f, 0.6f);
        switch_ui_draw_text((float)w - 24.0f - tw, 20.0f, 26.0f, 1.0f, 1.0f, 1.0f, 1.0f, sp, true);
    }
}

}  // namespace

void pet_time_on_game_loaded() {
    g_catch = CatchUp{};
    g_last_frame_wall = 0;
}

void pet_time_on_state_loaded() {
    const Pet pet = current_pet();
    if (!pet) return;
    const int64_t saved = pet.saved_unix_time();
    if (saved == 0) return;  // older save without a timestamp - nothing to go on
    // Negative means the Switch clock moved backwards since - leave the pet alone.
    apply_time_away((int64_t)time(nullptr) - saved, "save state loaded");
}

bool pet_time_current_is_pet() {
    const Pet pet = current_pet();
    return pet && pet.clock_is_set();
}

void pet_time_simulate(int64_t seconds) {
    const Pet pet = current_pet();
    if (!pet || !pet.clock_is_set() || seconds <= 0) return;
    pet.release_inputs();
    g_catch = CatchUp{};
    g_catch.total_ticks = (uint64_t)seconds * kTicksPerSecond;
    g_catch.started_tick = armGetSystemTick();
    g_catch.simulate = true;
    g_catch.keep_clock = !g_experimental_features_enabled || g_pet_time_mode == PetTimeMode::Paused;
    g_catch.active = true;
    YOKOI_LOG("pet_time: simulating %lld s (clock %s afterwards)", (long long)seconds,
              g_catch.keep_clock ? "left ahead" : "back to real time");
}

void pet_time_after_undo() {
    g_catch = CatchUp{};
    g_last_frame_wall = (int64_t)time(nullptr);
    const Pet pet = current_pet();
    if (!pet || !pet.clock_is_set()) return;
    if (g_experimental_features_enabled && g_pet_time_mode != PetTimeMode::Paused) pet.clock_write(time(nullptr));
    pet.cpu()->segments_state_are_update = true;
    update_segments_from_cpu(pet.cpu());
}

int64_t pet_time_state_age() {
    const Pet pet = current_pet();
    if (!pet) return 0;
    const int64_t saved = pet.saved_unix_time();
    if (saved == 0) return 0;
    return std::max<int64_t>(0, (int64_t)time(nullptr) - saved);
}

bool pet_time_would_catch_up() {
    return g_experimental_features_enabled && g_pet_time_mode == PetTimeMode::RealTime;
}

void pet_time_sync_clock() {
    g_catch = CatchUp{};
    g_last_frame_wall = (int64_t)time(nullptr);
    const Pet pet = current_pet();
    if (pet && pet.clock_is_set()) pet.clock_write(time(nullptr));
}

bool pet_time_frame(uint64_t kDown) {
    const Pet pet = current_pet();
    if (!pet) return false;

    const int64_t now = (int64_t)time(nullptr);
    if (!g_catch.active) {
        if (!pet.setup_busy() && g_last_frame_wall != 0 && now - g_last_frame_wall >= kGapThresholdSeconds) {
            apply_time_away(now - g_last_frame_wall, "resumed after a pause");
        }
        g_last_frame_wall = now;
        if (!g_catch.active) return false;
    }

    if (kDown & HidNpadButton_B) {
        YOKOI_LOG("pet_time: catch-up stopped early at %llu of %llu s", (unsigned long long)(g_catch.done_ticks / kTicksPerSecond),
                  (unsigned long long)(g_catch.total_ticks / kTicksPerSecond));
        finish(pet);
        return true;
    }

    const uint64_t start = armGetSystemTick();
    while (g_catch.done_ticks < g_catch.total_ticks && armTicksToNs(armGetSystemTick() - start) < kFrameBudgetNs) {
        const uint64_t n = std::min(kSliceTicks, g_catch.total_ticks - g_catch.done_ticks);
        pet.fast_forward(n);
        g_catch.done_ticks += n;
    }
    if (g_pet_time_display == PetTimeDisplay::FastForward || g_pet_time_display == PetTimeDisplay::ScreenBar)
        update_segments_from_cpu(pet.cpu());

    if (g_catch.done_ticks >= g_catch.total_ticks) {
        YOKOI_LOG("pet_time: caught up %llu s in %.1f s", (unsigned long long)(g_catch.total_ticks / kTicksPerSecond),
                  armTicksToNs(armGetSystemTick() - g_catch.started_tick) / 1e9);
        finish(pet);
    }
    return true;
}

void pet_time_render(int viewport_w, int viewport_h) {
    if (!g_catch.active || armTicksToNs(armGetSystemTick() - g_catch.started_tick) < kOverlayDelayNs) return;

    int w = viewport_w, h = viewport_h;
    if (g_screen_rotation != ScreenRotation::Deg0) std::swap(w, h);
    switch_ui_set_rotation((int)g_screen_rotation);  // drawn over the game
    switch_ui_begin_frame(w, h);

    const uint64_t done_s = g_catch.done_ticks / kTicksPerSecond;
    const uint64_t total_s = g_catch.total_ticks / kTicksPerSecond;
    const uint64_t left_s = total_s - std::min(done_s, total_s);

    if (g_pet_time_display == PetTimeDisplay::FastForward) {
        // Small plate, top right - the LCD itself shows the time flying by.
        const std::string text = ">> " + duration_text(left_s) + " left";
        const float size = 24.0f, pad = 12.0f;
        const float tw = switch_ui_text_width(text, size, true);
        const float right = (float)w - 20.0f;
        switch_ui_draw_rect(right - tw - pad, 18.0f - pad, tw + 2 * pad, size + 2 * pad, 0.0f, 0.0f, 0.0f, 0.6f);
        switch_ui_draw_text(right - tw, 18.0f, size, 1.0f, 1.0f, 1.0f, 1.0f, text, true);
        draw_skip_hint(right, (float)h - 52.0f, 22.0f);
        return;
    }

    if (g_pet_time_display == PetTimeDisplay::Clock || g_pet_time_display == PetTimeDisplay::DayNight ||
        g_pet_time_display == PetTimeDisplay::ScreenBar) {
        if (g_pet_time_display == PetTimeDisplay::Clock) render_clock_view(w, h, done_s, total_s);
        else if (g_pet_time_display == PetTimeDisplay::DayNight) render_daynight_view(w, h, done_s, total_s);
        else render_screenbar_view(w, h, done_s, total_s);
        draw_skip_hint((float)w - 20.0f, (float)h - 52.0f - (g_pet_time_display == PetTimeDisplay::ScreenBar ? 150.0f : 0.0f), 22.0f);
        return;
    }

    // Progress screen: dim the (frozen) game, centred title, numbers and bar.
    switch_ui_draw_rect(0.0f, 0.0f, (float)w, (float)h, 0.0f, 0.0f, 0.0f, 0.72f);
    const float cx = (float)w * 0.5f;
    float y = (float)h * 0.5f - 90.0f;
    draw_centered(cx, y, 34.0f, 1.0f, g_catch.simulate ? "Simulating time" : "Catching up on time away", true);
    y += 58.0f;
    draw_centered(cx, y, 26.0f, 0.9f, duration_text(done_s) + " of " + duration_text(total_s));
    y += 48.0f;
    const float bar_w = std::min(560.0f, (float)w - 80.0f), bar_h = 18.0f;
    const float frac = total_s ? (float)done_s / (float)total_s : 1.0f;
    switch_ui_draw_rect(cx - bar_w * 0.5f, y, bar_w, bar_h, 1.0f, 1.0f, 1.0f, 0.2f);
    switch_ui_draw_rect(cx - bar_w * 0.5f, y, bar_w * frac, bar_h, 1.0f, 1.0f, 1.0f, 0.9f);
    y += 40.0f;
    if (g_catch.skipped_seconds > 0) {
        draw_centered(cx, y, 20.0f, 0.7f, "(" + duration_text((uint64_t)g_catch.skipped_seconds) + " beyond the cap skipped)");
    }
    draw_skip_hint((float)w - 20.0f, (float)h - 52.0f, 22.0f);
}
