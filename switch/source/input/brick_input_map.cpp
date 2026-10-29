#include "brick_input_map.h"

#include <map>
#include <string>
#include <vector>

#include <switch.h>

#include "switch_input.h"
#include "BrickCores/BrickCore.h"

namespace {

uint64_t key_mask(const std::string& name) {
    static const std::map<std::string, uint64_t> kKeys = {
        {"A", HidNpadButton_A},       {"B", HidNpadButton_B},         {"X", HidNpadButton_X},
        {"Y", HidNpadButton_Y},       {"L", HidNpadButton_L},         {"R", HidNpadButton_R},
        {"ZL", HidNpadButton_ZL},     {"ZR", HidNpadButton_ZR},       {"PLUS", HidNpadButton_Plus},
        {"MINUS", HidNpadButton_Minus}, {"UP", HidNpadButton_Up},     {"DOWN", HidNpadButton_Down},
        {"LEFT", HidNpadButton_Left}, {"RIGHT", HidNpadButton_Right},
    };
    auto it = kKeys.find(name);
    return it == kKeys.end() ? 0 : it->second;
}

// One binding: pressed while any alternative ("|") has all its keys ("+") held.
struct Resolved {
    std::string name;
    std::vector<uint64_t> alternatives;
};

bool is_chord(uint64_t mask) { return (mask & (mask - 1)) != 0; }  // more than one key

// Rebuilt whenever a different core or binding list shows up (a new game
// can land at the same address as the last one, so the list is compared too).
const BrickCore* g_core = nullptr;
std::vector<BrickCore::Binding> g_bindings;
std::vector<Resolved> g_resolved;
std::map<std::string, bool> g_last;

void resolve(const BrickCore* core) {
    g_core = core;
    g_bindings = core->bindings();
    g_resolved.clear();
    g_last.clear();
    for (const BrickCore::Binding& b : core->bindings()) {
        Resolved r{b.name, {}};
        uint64_t mask = 0;
        bool bad = false;
        std::string key;
        for (size_t i = 0; i <= b.switch_keys.size(); i++) {
            const char c = i < b.switch_keys.size() ? b.switch_keys[i] : '|';
            if (c != '+' && c != '|') {
                key += c;
                continue;
            }
            const uint64_t m = key_mask(key);
            bad |= (m == 0);
            mask |= m;
            key.clear();
            if (c == '|') {
                if (mask && !bad) r.alternatives.push_back(mask);
                mask = 0;
                bad = false;
            }
        }
        if (!r.alternatives.empty()) g_resolved.push_back(r);
    }
}

}  // namespace

void apply_brick_input(BrickCore* core) {
    if (!core) return;
    const auto& bindings = core->bindings();
    bool same = core == g_core && bindings.size() == g_bindings.size();
    for (size_t i = 0; same && i < bindings.size(); i++) {
        same = bindings[i].name == g_bindings[i].name && bindings[i].switch_keys == g_bindings[i].switch_keys;
    }
    if (!same) resolve(core);

    const uint64_t held = switch_input_held();  // Already has left-stick folded into D-Pad bits.
    uint64_t chorded = 0;
    for (const Resolved& r : g_resolved) {
        for (uint64_t m : r.alternatives) {
            if (is_chord(m) && (held & m) == m) chorded |= m;
        }
    }
    for (const Resolved& r : g_resolved) {
        bool pressed = false;
        for (uint64_t m : r.alternatives) {
            if ((held & m) == m && (is_chord(m) || !(m & chorded))) pressed = true;
        }
        auto last = g_last.find(r.name);
        if (last != g_last.end() && last->second == pressed) continue;
        g_last[r.name] = pressed;
        core->set_button(r.name, pressed);
    }
}
