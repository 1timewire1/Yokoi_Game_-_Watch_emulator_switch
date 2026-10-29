#pragma once

#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "SM5XX/SM5XX.h"
#include "BrickRom.h"

// Symmetric save/load for a core's state: the same serialize() call writes
// on save and reads on load, so the two can't drift apart. Any short
// read/write latches ok() false and every further call becomes a no-op.
class BrickStateIO {
public:
    BrickStateIO(FILE* file, bool writing) : file_(file), writing_(writing) {}
    bool ok() const { return ok_; }
    bool writing() const { return writing_; }

    void bytes(void* p, size_t n) {
        if (!ok_) return;
        if ((writing_ ? fwrite(p, 1, n, file_) : fread(p, 1, n, file_)) != n) ok_ = false;
    }
    template <typename T>
    void pod(T& v) { bytes(&v, sizeof(T)); }
    // Fixed-size on both sides (the core sizes it before loading).
    void vec(std::vector<uint8_t>& v) {
        uint32_t n = (uint32_t)v.size();
        pod(n);
        if (ok_ && n != v.size()) ok_ = false;  // layout mismatch - reject
        if (ok_ && n) bytes(v.data(), n);
    }

private:
    FILE* file_;
    bool writing_;
    bool ok_ = true;
};

// Base for the cores ported from BrickEmuPy (github.com/azya52/BrickEmuPy,
// CC0). A BrickEmuPy core is a class whose clock() runs one instruction
// (plus its timers) and returns the cycles that took, at the .brick's
// "clock" rate; get_VRAM() returns the display memory its face SVG's
// "<byte>_<bit>" segment ids index into; port_handler() takes input from
// the button peripherals. Subclasses implement those same three things
// (execute(), display_byte(), port_input()), and this class does the rest:
//
//  - SM5XX plumbing, like E0C6S46: frequency = kTickHz, and SM5XX::step()'s
//    once-per-call update_sound() runs one tick of emulated time - as many
//    execute() calls as fit, carrying the remainder - then samples the
//    speaker, so audio gets a fresh level for every sample.
//  - Segments: pack ids are (byte, bit, 0) - see CONVERT_ROM/split_brick_svg.py.
//  - Inputs: the .brick's direct/matrix bindings ("input.*" options) by
//    button name, with BrickEmuPy's semantics (peripherals/direct_input.py,
//    matrix_input.py) - set_button() is all the platform layer calls.
//  - Sound: a speaker pin (set_speaker) for cores that toggle one, or a
//    square-wave tone (set_tone) for chips with a tone generator - what
//    BrickEmuPy's emit_audio((freq, noise, ...)) tuples describe.
//  - Save states: a versioned, timestamped header, then serialize().
class BrickCore : public SM5XX {
public:
    static constexpr uint32_t kTickHz = 32768;

    explicit BrickCore(const char* name) : SM5XX(name) {
        frequency = kTickHz;
        time_per_cycle_us = 1'000'000.0 / kTickHz;
    }

    // Parses the container and wires up inputs; returns false if it's not
    // one or it's for a different core. Call before init().
    bool configure(const uint8_t* data, size_t size);
    const BrickRom& rom_info() const { return rom_; }

    // --- SM5XX ---
    void init() override;
    void load_rom(const uint8_t* file_hex, size_t size_hex) override;
    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override {
        return word == 0 && line < 8 && ((display_byte(col) >> line) & 1);
    }
    bool screen_is_on() override { return true; }
    bool get_active_sound() override { return sound_level_; }
    bool save_state(FILE* file) override;
    bool load_state(FILE* file) override;
    uint8_t get_cpu_type_id() override { return 6; }  // CPU_TYPE_BRICK (std/savestate.h)

    // Button names as in the .brick ("btnLeft", ...), with the Switch keys
    // the importer assigned them ("input.<name>" option's last field).
    struct Binding {
        std::string name;
        std::string switch_keys;  // e.g. "LEFT", "A", "PLUS+MINUS"
    };
    const std::vector<Binding>& bindings() const { return bindings_; }
    void set_button(const std::string& name, bool pressed);

    // Wall-clock time the last loaded state was saved at (0 = unknown).
    int64_t saved_unix_time() const { return saved_unix_time_; }

protected:
    // --- what a ported core implements ---
    virtual void reset() = 0;                               // power-on/reset state (ROM already loaded)
    virtual int execute() = 0;                              // one instruction, returns cycles (>= 1)
    // The same, in cycles * kTickHz - for cores whose steps take fractional
    // cycles (SPL81408 asleep: one 32768 Hz tick = clock/32768 cycles).
    virtual int64_t execute_scaled() { return (int64_t)execute() * kTickHz; }
    virtual uint8_t display_byte(int index) const = 0;      // get_VRAM()[index], 0 past the end
    virtual int port_id(const std::string& name) const = 0; // .brick port name -> core port id, -1 = unknown
    // level 0/1, -1 = released. `mask` is passed through from the .brick as
    // is, and its meaning is the core's: a bitmask on most (E0C6200's K0
    // "mask": 4), a pin number on the Holtek ones (HT943's port_handler
    // does 1 << mask).
    virtual void port_input(int port, uint8_t mask, int level) = 0;
    virtual void serialize(BrickStateIO& io) = 0;           // all mutable core state
    // Called by the core when it drives an output port, so matrix-wired
    // buttons (input pin <- output pin while pressed) follow it.
    void port_output(int port, uint8_t value);

    void set_speaker(bool level) { speaker_ = level; }
    // Square wave at freq_hz, 0 = off. `noise` gives each half-wave a random
    // sign instead (BrickEmuPy's audio_engine.py: a "noise" tone flips its
    // sine's sign at random on every zero crossing).
    void set_tone(double freq_hz, bool noise = false) {
        if (freq_hz <= 0) {
            tone_step_ = 0;
            tone_phase_ = 0;
            tone_half_ = false;
            return;
        }
        tone_step_ = freq_hz / kTickHz;
        tone_noise_ = noise;
    }
    // The "RES" pin is held (execution is stopped; most cores blank the LCD).
    bool reset_held() const { return reset_held_; }

    const std::vector<uint8_t>& rom() const { return rom_.blobs.at("rom"); }
    const BrickRom::Options& options() const { return rom_.options; }
    uint32_t clock_hz() const { return rom_.clock_hz; }

private:
    void update_sound() override;   // one engine tick - see the class comment
    void execute_curr_opcode() override {}
    bool no_pc_increase(uint8_t) override { return true; }
    bool is_on_double_octet(uint8_t) override { return false; }
    void update_segment() override {}
    bool condition_to_update_segment() override { return false; }
    void wake_up() override {}
    uint8_t read_rom_value() override { return 0; }
    uint8_t read_ram_value() override { return 0; }
    void write_ram_value(uint8_t) override {}
    void set_ram_value(uint8_t, uint8_t, uint8_t) override {}

    struct DirectInput { int port; uint8_t mask; int level; };
    struct MatrixInput { int port; uint8_t mask; int out_port; uint8_t out_mask; };
    void update_matrix();

    BrickRom rom_;
    std::vector<Binding> bindings_;
    std::map<std::string, DirectInput> direct_;
    std::map<std::string, MatrixInput> matrix_;
    std::map<std::string, bool> matrix_pressed_;
    std::map<int, uint8_t> port_out_;
    bool reset_held_ = false;

    int64_t cycle_debt_ = 0;     // in cycles * kTickHz
    uint32_t ticks_to_refresh_ = 0;
    bool speaker_ = false;
    double tone_step_ = 0;       // tone periods per tick, 0 = no tone
    double tone_phase_ = 0;      // within the current half-wave, 0..0.5
    bool tone_half_ = false;     // which half-wave
    bool tone_noise_ = false;
    bool noise_flip_ = false;    // current half-wave's random sign
    uint32_t noise_lfsr_ = 1;
    bool sound_level_ = false;
    int64_t saved_unix_time_ = 0;
};

// Creates the right BrickCore subclass for a .brk container (by its core
// name), configured and ready for init(). nullptr if it isn't a container,
// or no ported core matches yet.
std::unique_ptr<SM5XX> brick_core_create(const uint8_t* data, size_t size);
