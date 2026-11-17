#include "brick_input_map.h"

#include <map>
#include <string>
#include <vector>

#include <switch.h>

#include "switch_input.h"
#include "BrickCores/BrickCore.h"
#include "audio/step_sound.h"

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

// 8 frames down + 8 up at 60 fps = 3.75 presses a second: the pace of the
// real pedometers walking (the user's recordings of a D-Tector, a D-Power
// Digivice and a Pocket Pikachu all step every ~0.25 s), so each step's
// sound (step_sound.cpp) follows the next like on the real thing. The
// D-Power Digivice would still count up to 10 a second (3 + 3), but that
// rattled its step sound off like a machine gun.
constexpr int kTurboFrames = 8;

bool is_chord(uint64_t mask) { return (mask & (mask - 1)) != 0; }  // more than one key

// Rebuilt whenever a different core or binding list shows up (a new game
// can land at the same address as the last one, so the list is compared too).
const BrickCore* g_core = nullptr;
std::vector<BrickCore::Binding> g_bindings;
std::vector<Resolved> g_resolved;
std::map<std::string, bool> g_last;
int g_motion_frames = 0;  // frames the step button stays pressed for a shake
int g_ir_wave_frames = 0;  // same for the IR camera's Wave
bool g_ir_sensor = false;  // the IR camera's Sensor, held

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
        if (g_motion_frames > 0 && (r.name == "btnStep" || r.name == "reflector")) pressed = true;
        if (g_ir_wave_frames > 0 && r.name == "btnWave") pressed = true;
        if (g_ir_sensor && r.name == "btnSensor") pressed = true;
        if (turbo_held && !pressed) {
            pressed = (r.turbo_frame / kTurboFrames) % 2 == 0;
            r.turbo_frame++;
        } else {
            r.turbo_frame = 0;
        }
        auto last = g_last.find(r.name);
        if (last != g_last.end() && last->second == pressed) continue;
        g_last[r.name] = pressed;
        if (pressed && r.name == "btnStep") {
            // A pedometer that counts a step only every few pulses (D-Tector:
            // 3): send the extra ones first, so one press = one step.
            for (int i = core->step_pulses(); i > 1; i--) {
                core->set_button(r.name, true);
                core->set_button(r.name, false);
            }
        }
        core->set_button(r.name, pressed);
        // Each device's own pedometer sound on every counted step - the only
        // games with a step sensor: D-Tector (GPL191X) and D-Power Digivice
        // (SPL61X) "btnStep", Pocket Pikachu "reflector".
        if (pressed && r.name == "btnStep") {
            step_sound_play(core->rom_info().core == "GPL191X" ? StepSound::DTector : StepSound::DPower);
        } else if (pressed && r.name == "reflector") {
            step_sound_play(StepSound::PocketPikachu);
        }
    }
    if (g_motion_frames > 0) g_motion_frames--;
    if (g_ir_wave_frames > 0) g_ir_wave_frames--;
}

bool brick_input_has_ir() {
    for (const Resolved& r : g_resolved)
        if (r.name == "btnWave" || r.name == "btnSensor") return true;
    return false;
}

void brick_input_ir_wave() {
    if (g_ir_wave_frames <= 0) g_ir_wave_frames = kTurboFrames;
}

void brick_input_ir_sensor(bool held) { g_ir_sensor = held; }

bool brick_input_has_step() {
    for (const Resolved& r : g_resolved)
        if (r.name == "btnStep" || r.name == "reflector") return true;
    return false;
}

void brick_input_motion_step() {
    if (g_motion_frames <= 0) g_motion_frames = kTurboFrames;
}

bool brick_input_take_motion_frame() {
    if (g_motion_frames <= 0) return false;
    g_motion_frames--;
    return true;
}
