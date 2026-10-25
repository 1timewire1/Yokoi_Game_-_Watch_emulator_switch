#include "pet_time.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <string>
#include <utility>

#include <switch.h>

#include "E0C6200/E0C6S46.h"
#include "std/debug_log.h"
#include "std/savestate.h"

#include "e0c6200_clock.h"
#include "input/e0c6200_clock_setup.h"
#include "render/switch_ui.h"
#include "switch_runtime_state.h"
#include "switch_segments_state.h"
#include "switch_settings.h"

namespace {

constexpr uint64_t kTicksPerSecond = 32768;   // E0C6S46 OSC1
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
};

CatchUp g_catch;
int64_t g_last_frame_wall = 0;  // time() of the last emulated frame, 0 = none yet

// The running pet and its clock layout, or nullptr if Pet Time doesn't apply.
E0C6S46* pet(const E0C6200ClockLayout** layout_out) {
    if (!g_cpu || !g_game || g_cpu->get_cpu_type_id() != CPU_TYPE_E0C6S46) return nullptr;
    const E0C6200ClockLayout* layout = e0c6200_clock_layout(g_game->ref);
    if (!layout) return nullptr;
    *layout_out = layout;
    return static_cast<E0C6S46*>(g_cpu.get());
}

void finish(E0C6S46* cpu, const E0C6200ClockLayout& layout) {
    e0c6200_clock_write(cpu, layout, time(nullptr));  // also covers the time the catch-up itself took
    cpu->segments_state_are_update = true;
    update_segments_from_cpu(cpu);
    g_catch = CatchUp{};
    g_last_frame_wall = (int64_t)time(nullptr);
}

// `away` seconds passed without the pet running.
void apply_time_away(int64_t away, const char* why) {
    if (away <= 0 || g_pet_time_mode == PetTimeMode::Paused) return;
    const E0C6200ClockLayout* layout = nullptr;
    E0C6S46* cpu = pet(&layout);
    if (!cpu || !e0c6200_clock_is_set(cpu, *layout)) return;  // pet never started - nothing to age

    const time_t now = time(nullptr);
    if (g_pet_time_mode == PetTimeMode::ClockSync) {
        e0c6200_clock_write(cpu, *layout, now);
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
        e0c6200_clock_write(cpu, *layout, now - (time_t)emulate);
    }
    // Nobody's holding anything while away.
    for (int line = 0; line < 8; line++) cpu->set_k_line(line, true);
    cpu->set_reset_line(false);
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

}  // namespace

void pet_time_on_game_loaded() {
    g_catch = CatchUp{};
    g_last_frame_wall = 0;
}

void pet_time_on_state_loaded() {
    const E0C6200ClockLayout* layout = nullptr;
    E0C6S46* cpu = pet(&layout);
    if (!cpu) return;
    const int64_t saved = cpu->saved_unix_time();
    if (saved == 0) return;  // older save without a timestamp - nothing to go on
    // Negative means the Switch clock moved backwards since - leave the pet alone.
    apply_time_away((int64_t)time(nullptr) - saved, "save state loaded");
}

bool pet_time_frame(uint64_t kDown) {
    const E0C6200ClockLayout* layout = nullptr;
    E0C6S46* cpu = pet(&layout);
    if (!cpu) return false;

    const int64_t now = (int64_t)time(nullptr);
    if (!g_catch.active) {
        // The automatic clock setup owns the first seconds of a fresh start.
        if (!e0c6200_clock_setup_active() && g_last_frame_wall != 0 && now - g_last_frame_wall >= kGapThresholdSeconds) {
            apply_time_away(now - g_last_frame_wall, "resumed after a pause");
        }
        g_last_frame_wall = now;
        if (!g_catch.active) return false;
    }

    if (kDown & HidNpadButton_B) {
        YOKOI_LOG("pet_time: catch-up stopped early at %llu of %llu s", (unsigned long long)(g_catch.done_ticks / kTicksPerSecond),
                  (unsigned long long)(g_catch.total_ticks / kTicksPerSecond));
        finish(cpu, *layout);
        return true;
    }

    const uint64_t start = armGetSystemTick();
    while (g_catch.done_ticks < g_catch.total_ticks && armTicksToNs(armGetSystemTick() - start) < kFrameBudgetNs) {
        const uint64_t n = std::min(kSliceTicks, g_catch.total_ticks - g_catch.done_ticks);
        cpu->fast_forward(n);
        g_catch.done_ticks += n;
    }
    if (g_pet_time_display == PetTimeDisplay::FastForward) update_segments_from_cpu(cpu);

    if (g_catch.done_ticks >= g_catch.total_ticks) {
        YOKOI_LOG("pet_time: caught up %llu s in %.1f s", (unsigned long long)(g_catch.total_ticks / kTicksPerSecond),
                  armTicksToNs(armGetSystemTick() - g_catch.started_tick) / 1e9);
        finish(cpu, *layout);
    }
    return true;
}

void pet_time_render(int viewport_w, int viewport_h) {
    if (!g_catch.active || armTicksToNs(armGetSystemTick() - g_catch.started_tick) < kOverlayDelayNs) return;

    int w = viewport_w, h = viewport_h;
    if (g_screen_rotation != ScreenRotation::Deg0) std::swap(w, h);
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

    // Progress screen: dim the (frozen) game, centred title, numbers and bar.
    switch_ui_draw_rect(0.0f, 0.0f, (float)w, (float)h, 0.0f, 0.0f, 0.0f, 0.72f);
    const float cx = (float)w * 0.5f;
    float y = (float)h * 0.5f - 90.0f;
    draw_centered(cx, y, 34.0f, 1.0f, "Catching up on time away", true);
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
