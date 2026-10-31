#include "BrickCore.h"

#include "GPL191X.h"
#include "HT943.h"
#include "M37520.h"
#include "MSM50XX.h"
#include "SPL81408.h"

#include <cstdlib>
#include <cstring>
#include <ctime>

namespace {

constexpr uint32_t kStateMagic = 0x534B5242;  // "BRKS"
constexpr uint8_t kStateVersion = 2;  // 2: fractional/noise tone
constexpr uint32_t kRefreshTicks = 512;       // segment refresh: 64 Hz
constexpr int kResetPort = -2;                // BrickEmuPy's "RES" pseudo-port

std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        const size_t at = s.find(sep, start);
        out.push_back(s.substr(start, at == std::string::npos ? std::string::npos : at - start));
        if (at == std::string::npos) return out;
        start = at + 1;
    }
}

}  // namespace

bool BrickCore::configure(const uint8_t* data, size_t size) {
    if (!rom_.parse(data, size)) return false;
    // "input.<button>" = direct;<port>;<mask>;<level>;<keys>
    //                  | matrix;<port>;<mask>;<out port>;<out mask>;<keys>
    bindings_.clear();
    direct_.clear();
    matrix_.clear();
    for (const auto& kv : rom_.options.with_prefix("input.")) {
        const std::vector<std::string> f = split(kv.second, ';');
        if (f.size() == 5 && f[0] == "direct") {
            const int port = (f[1] == "RES") ? kResetPort : port_id(f[1]);
            direct_[kv.first] = {port, (uint8_t)strtol(f[2].c_str(), nullptr, 0), atoi(f[3].c_str())};
            bindings_.push_back({kv.first, f[4]});
        } else if (f.size() == 6 && f[0] == "matrix") {
            matrix_[kv.first] = {port_id(f[1]), (uint8_t)strtol(f[2].c_str(), nullptr, 0), port_id(f[3]),
                                 (uint8_t)strtol(f[4].c_str(), nullptr, 0)};
            bindings_.push_back({kv.first, f[5]});
        }
    }
    return true;
}

void BrickCore::init() {
    // SM5XX bookkeeping step() relies on (it never touches anything else).
    cycle_curr_opcode = 0;
    cpu_frequency_divider = 1;
    is_sleep = false;
    f_clock_divider = 0;
    gamma_flag_second = false;
    time_addresses = nullptr;
    sound_divide_frequency = 1;
    segments_state_are_update = true;

    cycle_debt_ = 0;
    ticks_to_refresh_ = kRefreshTicks;
    speaker_ = sound_level_ = false;
    tone_step_ = tone_phase_ = 0;
    tone_half_ = tone_noise_ = noise_flip_ = false;
    noise_lfsr_ = 1;
    reset_held_ = false;
    matrix_pressed_.clear();
    port_out_.clear();
}

void BrickCore::load_rom(const uint8_t*, size_t) {
    // The ROM came in through configure(); this is where the core starts.
    reset();
}

void BrickCore::update_sound() {
    // One tick = 1/kTickHz s = clock_hz / kTickHz cycles (debt kept scaled
    // by kTickHz so nothing is lost to rounding).
    if (!reset_held_) {
        cycle_debt_ += rom_.clock_hz;
        while (cycle_debt_ > 0) cycle_debt_ -= execute_scaled();
    }
    if (tone_step_ > 0) {
        tone_phase_ += tone_step_;
        while (tone_phase_ >= 0.5) {
            tone_phase_ -= 0.5;
            tone_half_ = !tone_half_;
            if (tone_noise_) {
                // 32-bit Galois LFSR - any decent random bit will do.
                noise_lfsr_ = (noise_lfsr_ >> 1) ^ (-(noise_lfsr_ & 1u) & 0x80200003u);
                noise_flip_ = noise_lfsr_ & 1;
            }
        }
        sound_level_ = tone_half_ != (tone_noise_ && noise_flip_);
    } else {
        sound_level_ = speaker_;
    }
    if (--ticks_to_refresh_ == 0) {
        ticks_to_refresh_ = kRefreshTicks;
        segments_state_are_update = true;
    }
}

bool BrickCore::set_clock(int hour, int minute, int second) {
    const std::vector<long> sec = rom_.options.list("clock.sec");
    const std::vector<long> min = rom_.options.list("clock.min");
    const std::vector<long> hr = rom_.options.list("clock.hour");
    const std::vector<long> pm = rom_.options.list("clock.pm");
    const bool bcd = rom_.options.num("clock.bcd", 0) != 0;
    const size_t cells = bcd ? 1 : 2;
    if (sec.size() != cells || min.size() != cells || hr.empty() || hr.size() > 2) return false;
    int h = hour;
    if (pm.size() == 3) {
        h = hour % 12;
        if (h == 0 && !rom_.options.num("clock.hour_zero", 0)) h = 12;
        const int cell = peek_ram((uint32_t)pm[0]);
        if (cell < 0) return false;
        const bool set = (hour >= 12) == (pm[2] != 0);
        if (!poke_ram((uint32_t)pm[0], (uint8_t)(set ? (cell | pm[1]) : (cell & ~pm[1])))) return false;
    }
    if (bcd) {
        auto to_bcd = [](int v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); };
        const bool ok = poke_ram((uint32_t)sec[0], to_bcd(second)) && poke_ram((uint32_t)min[0], to_bcd(minute)) &&
                        poke_ram((uint32_t)hr[0], to_bcd(h));
        if (ok) segments_state_are_update = true;
        return ok;
    }
    bool ok = poke_ram((uint32_t)sec[0], (uint8_t)(second % 10)) && poke_ram((uint32_t)sec[1], (uint8_t)(second / 10)) &&
              poke_ram((uint32_t)min[0], (uint8_t)(minute % 10)) && poke_ram((uint32_t)min[1], (uint8_t)(minute / 10));
    if (hr.size() == 2) {
        ok = ok && poke_ram((uint32_t)hr[0], (uint8_t)(h % 10)) && poke_ram((uint32_t)hr[1], (uint8_t)(h / 10));
    } else {
        ok = ok && poke_ram((uint32_t)hr[0], (uint8_t)h);
    }
    if (ok) segments_state_are_update = true;
    return ok;
}

void BrickCore::set_button(const std::string& name, bool pressed) {
    auto d = direct_.find(name);
    if (d != direct_.end()) {
        if (d->second.port == kResetPort) {
            // The reset pin ("RES" in BrickEmuPy): held = reset and stop.
            if (pressed && !reset_held_) reset();
            reset_held_ = pressed;
        } else if (d->second.port >= 0) {
            port_input(d->second.port, d->second.mask, pressed ? d->second.level : -1);
        }
        return;
    }
    if (matrix_.count(name)) {
        if (matrix_pressed_[name] != pressed) {
            matrix_pressed_[name] = pressed;
            update_matrix();
        }
    }
}

void BrickCore::port_output(int port, uint8_t value) {
    auto it = port_out_.find(port);
    if (it != port_out_.end() && it->second == value) return;
    port_out_[port] = value;
    if (!matrix_.empty()) update_matrix();
}

void BrickCore::update_matrix() {
    // peripherals/matrix_input.py: each (port, mask) input is driven with the
    // OR of its pressed buttons' output-pin levels, or released (-1) if none.
    std::map<std::pair<int, uint8_t>, int> state;
    for (const auto& kv : matrix_) {
        const MatrixInput& m = kv.second;
        const auto key = std::make_pair(m.port, m.mask);
        if (!state.count(key)) state[key] = -1;
        auto pressed = matrix_pressed_.find(kv.first);
        if (pressed == matrix_pressed_.end() || !pressed->second) continue;
        auto out = port_out_.find(m.out_port);
        const int level = (out != port_out_.end() && (out->second & m.out_mask)) ? 1 : 0;
        state[key] = (state[key] == -1) ? level : (state[key] | level);
    }
    for (const auto& kv : state) {
        if (kv.first.first >= 0) port_input(kv.first.first, kv.first.second, kv.second);
    }
}

bool BrickCore::save_state(FILE* file) {
    if (!file) return false;
    BrickStateIO io(file, true);
    uint32_t magic = kStateMagic;
    uint8_t version = kStateVersion;
    uint8_t core_len = (uint8_t)rom_.core.size();
    int64_t now = (int64_t)time(nullptr);
    io.pod(magic);
    io.pod(version);
    io.pod(core_len);
    io.bytes(&rom_.core[0], core_len);
    io.pod(now);
    io.pod(cycle_debt_);
    io.pod(speaker_);
    io.pod(tone_step_);
    io.pod(tone_phase_);
    io.pod(tone_half_);
    io.pod(tone_noise_);
    io.pod(noise_flip_);
    io.pod(noise_lfsr_);
    serialize(io);
    return io.ok();
}

bool BrickCore::load_state(FILE* file) {
    if (!file) return false;
    BrickStateIO io(file, false);
    uint32_t magic = 0;
    uint8_t version = 0, core_len = 0;
    io.pod(magic);
    io.pod(version);
    io.pod(core_len);
    std::string core(core_len, '\0');
    if (core_len) io.bytes(&core[0], core_len);
    if (!io.ok() || magic != kStateMagic || version != kStateVersion || core != rom_.core) return false;
    int64_t saved = 0;
    io.pod(saved);
    io.pod(cycle_debt_);
    io.pod(speaker_);
    io.pod(tone_step_);
    io.pod(tone_phase_);
    io.pod(tone_half_);
    io.pod(tone_noise_);
    io.pod(noise_flip_);
    io.pod(noise_lfsr_);
    serialize(io);
    if (!io.ok()) {
        reset();  // a half-applied state isn't worth keeping
        return false;
    }
    saved_unix_time_ = saved;
    segments_state_are_update = true;
    return true;
}

// --- factory -------------------------------------------------------------
// One entry per ported core, by BrickEmuPy's core name. Several of its
// names share one implementation (SPL61X runs on GPL191X, for instance).
namespace {

struct CoreEntry {
    const char* name;
    BrickCore* (*create)();
};

template <class T>
BrickCore* make_core() { return new T(); }

constexpr CoreEntry kCores[] = {
    {"GPL191X", make_core<GPL191X>},
    {"HT943", make_core<HT943>},
    {"M37520", make_core<M37520>},
    {"MSM50XX", make_core<MSM50XX>},
    {"SPL61X", make_core<SPL61X>},
    {"SPL81408", make_core<SPL81408>},
};

}  // namespace

std::unique_ptr<SM5XX> brick_core_create(const uint8_t* data, size_t size) {
    BrickRom probe;
    if (!probe.parse(data, size)) return nullptr;
    for (const CoreEntry& e : kCores) {
        if (!e.name || probe.core != e.name) continue;
        std::unique_ptr<BrickCore> core(e.create());
        if (!core->configure(data, size)) return nullptr;
        return core;
    }
    return nullptr;
}
