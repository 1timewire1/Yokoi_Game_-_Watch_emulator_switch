#include "control_remap.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <switch.h>

#include "core/switch_settings.h"
#include "std/platform_paths.h"
#include "time_control.h"

namespace {

struct Map { uint8_t src[kNumRemapButtons]; };

const uint64_t kMasks[kNumRemapButtons] = {
    HidNpadButton_Up, HidNpadButton_Down, HidNpadButton_Left, HidNpadButton_Right,
    HidNpadButton_A, HidNpadButton_B, HidNpadButton_X, HidNpadButton_Y,
    HidNpadButton_L, HidNpadButton_R, HidNpadButton_ZL, HidNpadButton_ZR,
    HidNpadButton_Plus, HidNpadButton_Minus, HidNpadButton_StickL, HidNpadButton_StickR,
};
const char* const kLabels[kNumRemapButtons] = {
    "Up", "Down", "Left", "Right", "A", "B", "X", "Y", "L", "R", "ZL", "ZR", "+", "-", "L3", "R3",
};

std::map<std::string, Map> g_maps;
bool g_loaded = false;
std::string g_active_ref;
const Map* g_active = nullptr;

Map identity() {
    Map m;
    for (int i = 0; i < kNumRemapButtons; i++) m.src[i] = (uint8_t)i;
    return m;
}

bool is_identity(const Map& m) {
    for (int i = 0; i < kNumRemapButtons; i++)
        if (m.src[i] != i) return false;
    return true;
}

// Only accepted when it is a real permutation, so a hand-edited or damaged
// line can never leave a function unreachable.
bool valid(const Map& m) {
    bool seen[kNumRemapButtons] = {};
    for (int i = 0; i < kNumRemapButtons; i++) {
        if (m.src[i] == kNoButton) continue;  // unassigned
        if (m.src[i] >= kNumRemapButtons || seen[m.src[i]]) return false;
        seen[m.src[i]] = true;
    }
    return true;
}

void load() {
    if (g_loaded) return;
    g_loaded = true;
    FILE* f = fopen(config_path("controls.cfg").c_str(), "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char ref[128];
        int v[kNumRemapButtons];
        const int n = sscanf(line, "%127s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", ref,
                             &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10],
                             &v[11], &v[12], &v[13], &v[14], &v[15]);
        if (n != kNumRemapButtons + 1) continue;
        Map m;
        for (int i = 0; i < kNumRemapButtons; i++) m.src[i] = (uint8_t)(v[i] >= 0 && v[i] < kNumRemapButtons ? v[i] : 0xFF);
        if (valid(m) && !is_identity(m)) g_maps[ref] = m;
    }
    fclose(f);
}

void save() {
    FILE* f = fopen(config_path("controls.cfg").c_str(), "w");
    if (!f) return;
    for (const auto& it : g_maps) {
        fprintf(f, "%s", it.first.c_str());
        for (int i = 0; i < kNumRemapButtons; i++) fprintf(f, " %d", (int)it.second.src[i]);
        fprintf(f, "\n");
    }
    fclose(f);
}

void refresh_active() {
    auto it = g_maps.find(g_active_ref);
    g_active = (g_active_ref.empty() || it == g_maps.end()) ? nullptr : &it->second;
}

Map current(const std::string& ref) {
    load();
    auto it = g_maps.find(ref);
    return it == g_maps.end() ? identity() : it->second;
}

void store(const std::string& ref, const Map& m) {
    if (is_identity(m)) g_maps.erase(ref);
    else g_maps[ref] = m;
    save();
    refresh_active();
}

}  // namespace

uint64_t remap_button_mask(int b) { return (b >= 0 && b < kNumRemapButtons) ? kMasks[b] : 0; }
const char* remap_button_label(int b) { return (b >= 0 && b < kNumRemapButtons) ? kLabels[b] : "?"; }

int remap_button_from_mask(uint64_t mask) {
    for (int i = 0; i < kNumRemapButtons; i++)
        if (mask & kMasks[i]) return i;
    return -1;
}

const char* pause_combo_label(int combo) {
    switch (combo) {
        case 1:  return "L3 + R3";
        case 2:  return "L3";
        case 3:  return "R3";
        default: return "ZL + ZR";
    }
}

uint64_t pause_combo_mask() {
    switch (g_pause_combo) {
        case 1:  return HidNpadButton_StickL | HidNpadButton_StickR;
        case 2:  return HidNpadButton_StickL;
        case 3:  return HidNpadButton_StickR;
        default: return HidNpadButton_ZL | HidNpadButton_ZR;
    }
}

void control_remap_set_active(const std::string& ref) {
    if (ref == g_active_ref) return;
    load();
    g_active_ref = ref;
    refresh_active();
}

uint64_t control_remap_apply(uint64_t raw) {
    // While a game is played, the fast-forward / rewind buttons are theirs.
    if (!g_active_ref.empty()) raw &= ~time_control_hotkey_mask();
    if (!g_active) return raw;
    uint64_t all = 0;
    for (int i = 0; i < kNumRemapButtons; i++) all |= kMasks[i];
    const uint64_t pause = pause_combo_mask();
    uint64_t out = raw & ~all;
    for (int i = 0; i < kNumRemapButtons; i++) {
        const int src = g_active->src[i];
        if (src >= kNumRemapButtons) continue;  // unassigned: never pressed
        // A button that opens the pause menu never drives anything else.
        if (src != i && (kMasks[src] & pause)) continue;
        if (raw & kMasks[src]) out |= kMasks[i];
    }
    return out;
}

int control_remap_source(const std::string& ref, int logical) {
    if (logical < 0 || logical >= kNumRemapButtons) return logical;
    return current(ref).src[logical];
}

void control_remap_assign(const std::string& ref, int logical, int physical) {
    if (ref.empty() || logical < 0 || logical >= kNumRemapButtons || physical < 0 || physical >= kNumRemapButtons) return;
    Map m = current(ref);
    for (int j = 0; j < kNumRemapButtons; j++) {
        if (j != logical && m.src[j] == physical) {
            m.src[j] = m.src[logical];  // swap: whoever had it gets this one's old button
            break;
        }
    }
    m.src[logical] = (uint8_t)physical;
    store(ref, m);
}

void control_remap_unassign(const std::string& ref, int logical) {
    if (ref.empty() || logical < 0 || logical >= kNumRemapButtons) return;
    Map m = current(ref);
    m.src[logical] = (uint8_t)kNoButton;
    store(ref, m);
}

void control_remap_reset(const std::string& ref, int logical) {
    control_remap_assign(ref, logical, logical);
}

void control_remap_reset_all(const std::string& ref) {
    load();
    if (g_maps.erase(ref)) {
        save();
        refresh_active();
    }
}

bool control_remap_customized(const std::string& ref) {
    load();
    return g_maps.count(ref) != 0;
}
