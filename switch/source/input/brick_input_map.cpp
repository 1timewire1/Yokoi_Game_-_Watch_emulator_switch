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
// An alternative written with a leading '*' ("A|*X") is a turbo: while it's
// held the button is pressed and released over and over (kTurboFrames each)
// instead of staying down - e.g. the D-Power Digivice's Step button, whose
// pedometer counts one step per press.
struct Resolved {
    std::string name;
    std::vector<uint64_t> alternatives;
    std::vector<bool> turbo;  // per alternative
    int turbo_frame = 0;      // frames since the turbo started, for the press/release cadence
};

// 3 frames down + 3 up at 60 fps = 10 presses a second - as fast as the
// D-Power Digivice still counts every one (it drops half of them at 12/s).
constexpr int kTurboFrames = 3;

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
        Resolved r{b.name, {}, {}};
        uint64_t mask = 0;
        bool bad = false;
        bool turbo = false;
        std::string key;
        for (size_t i = 0; i <= b.switch_keys.size(); i++) {
            const char c = i < b.switch_keys.size() ? b.switch_keys[i] : '|';
            if (c == '*' && key.empty() && mask == 0) {
                turbo = true;
                continue;
            }
            if (c != '+' && c != '|') {
                key += c;
                continue;
            }
            const uint64_t m = key_mask(key);
            bad |= (m == 0);
            mask |= m;
            key.clear();
            if (c == '|') {
                if (mask && !bad) {
                    r.alternatives.push_back(mask);
                    r.turbo.push_back(turbo);
                }
                mask = 0;
                bad = false;
                turbo = false;
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
    for (Resolved& r : g_resolved) {
        bool pressed = false, turbo_held = false;
        for (size_t i = 0; i < r.alternatives.size(); i++) {
            const uint64_t m = r.alternatives[i];
            if ((held & m) != m || (!is_chord(m) && (m & chorded))) continue;
            if (r.turbo[i]) turbo_held = true;
            else pressed = true;
        }
        if (turbo_held && !pressed) {
            pressed = (r.turbo_frame / kTurboFrames) % 2 == 0;
            r.turbo_frame++;
        } else {
            r.turbo_frame = 0;
        }
        auto last = g_last.find(r.name);
        if (last != g_last.end() && last->second == pressed) continue;
        g_last[r.name] = pressed;
        core->set_button(r.name, pressed);
    }
}
