#include "time_control.h"

#include <cstdio>
#include <cstdlib>
#include <deque>
#include <string>
#include <vector>
#include <switch.h>

#include "SM5XX/SM5XX.h"
#include "control_remap.h"
#include "switch_input.h"
#include "core/switch_runtime_state.h"
#include "core/switch_settings.h"
#include "render/switch_ui.h"

namespace {

const uint64_t kChoiceMask[kNumHotkeyChoices] = {
    0, HidNpadButton_ZL, HidNpadButton_ZR, HidNpadButton_StickL, HidNpadButton_StickR,
};

constexpr int kSnapEvery = 15;    // frames between snapshots (4 a second)
constexpr int kRewindEvery = 4;   // frames per snapshot stepped back (~3.75x speed)

int g_speed = 1;
bool g_rewinding = false;
int g_frame = 0;
int g_rewind_frame = 0;
std::deque<std::vector<uint8_t>> g_snaps;
const void* g_snap_game = nullptr;   // the game/CPU the ring belongs to
const void* g_snap_cpu = nullptr;

uint64_t mask_of(int choice) { return (choice > 0 && choice < kNumHotkeyChoices) ? kChoiceMask[choice] : 0; }

bool save_snapshot(SM5XX* cpu, std::vector<uint8_t>& out) {
    char* buf = nullptr;
    size_t len = 0;
    FILE* f = open_memstream(&buf, &len);
    if (!f) return false;
    const bool ok = cpu->save_state(f);
    fclose(f);
    if (ok && buf && len) out.assign(buf, buf + len);
    free(buf);
    return ok && !out.empty();
}

bool load_snapshot(SM5XX* cpu, std::vector<uint8_t>& in) {
    FILE* f = fmemopen(in.data(), in.size(), "rb");
    if (!f) return false;
    const bool ok = cpu->load_state(f);
    fclose(f);
    return ok;
}

size_t max_snaps() {
    static const int kSeconds[4] = {5, 10, 30, 60};
    return (size_t)kSeconds[g_rewind_length < 4 ? g_rewind_length : 2] * 60 / kSnapEvery;
}

}  // namespace

uint64_t time_control_hotkey_mask() { return mask_of(g_fastforward_button) | mask_of(g_rewind_button); }

const char* time_control_button_label(int choice) {
    static const char* const kLabels[kNumHotkeyChoices] = {"Off", "ZL", "ZR", "L3", "R3"};
    return kLabels[(choice >= 0 && choice < kNumHotkeyChoices) ? choice : 0];
}

int time_control_next_choice(int current, int dir, int other_choice) {
    const uint64_t blocked = pause_combo_mask() | mask_of(other_choice);
    int c = current;
    for (int i = 0; i < kNumHotkeyChoices; i++) {
        c = (c + dir + kNumHotkeyChoices) % kNumHotkeyChoices;
        if (c == 0 || !(mask_of(c) & blocked)) return c;
    }
    return 0;
}

void time_control_update() {
    const uint64_t held = switch_input_raw_held();
    const uint64_t pause = pause_combo_mask();
    const uint64_t ff = mask_of(g_fastforward_button) & ~pause;
    const uint64_t rw = mask_of(g_rewind_button) & ~pause;
    g_rewinding = rw && (held & rw);
    if (!g_rewinding) g_rewind_frame = 0;
    static const int kSpeeds[3] = {2, 4, 8};
    g_speed = (!g_rewinding && ff && (held & ff)) ? kSpeeds[g_fastforward_speed < 3 ? g_fastforward_speed : 1] : 1;
}

int time_control_speed() { return g_speed; }
bool time_control_rewinding() { return g_rewinding; }

bool time_control_rewind_step(SM5XX* cpu) {
    if (!cpu || g_snap_cpu != cpu || g_snaps.empty()) return false;
    if (g_rewind_frame++ % kRewindEvery != 0) return false;
    // Keep the oldest one so holding on never runs out of a picture.
    std::vector<uint8_t> snap = g_snaps.back();
    if (g_snaps.size() > 1) g_snaps.pop_back();
    return load_snapshot(cpu, snap);
}

void time_control_after_frame(SM5XX* cpu) {
    if (!cpu || g_rewind_button == 0) {
        g_snaps.clear();
        g_snap_cpu = nullptr;
        return;
    }
    // A different game (or a reload) starts a fresh ring.
    if (g_snap_cpu != cpu || g_snap_game != (const void*)g_game) {
        g_snaps.clear();
        g_snap_cpu = cpu;
        g_snap_game = g_game;
        g_frame = 0;
    }
    if (g_frame++ % kSnapEvery != 0) return;
    std::vector<uint8_t> snap;
    if (!save_snapshot(cpu, snap)) return;  // a CPU that can't save (McDonald's): no rewind
    g_snaps.push_back(std::move(snap));
    while (g_snaps.size() > max_snaps()) g_snaps.pop_front();
}

void time_control_render() {
    if (g_speed <= 1 && !g_rewinding) return;
    char text[16];
    if (g_rewinding) snprintf(text, sizeof(text), "<< REW");
    else snprintf(text, sizeof(text), ">> %dx", g_speed);
    switch_ui_set_rotation(0);
    switch_ui_begin_frame(1280, 720);
    const float size = 26.0f, w = switch_ui_text_width(text, size, true);
    const float x = 1280.0f - 28.0f - w, y = 22.0f;
    switch_ui_draw_rect(x - 14.0f, y - 8.0f, w + 28.0f, size + 18.0f, 0.0f, 0.0f, 0.0f, 0.55f);
    switch_ui_draw_text(x, y, size, 1.0f, 1.0f, 1.0f, 1.0f, text, true);
}
