#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

#include "SM5XX/SM5XX.h"

// Seiko Epson E0C6S46 / E0C6S48 MCU (E0C6200 4-bit CPU core + the S46 family's
// clock timer, stopwatch, programmable timer, buzzer and 16-COM LCD driver),
// as used by the first Tamagotchi / Digital Monster virtual pets and a few
// other late-90s keychain games (MAME's hh_e0c6200 driver).
//
// A port of MAME's src/devices/cpu/e0c6200/{e0c6200,e0c6200op,e0c6s46}.cpp
// (license: BSD-3-Clause, copyright-holders: hap), kept as close to the
// original as possible - the opcode semantics, cycle counts, interrupt
// priorities and timer/buzzer edge rules are all MAME's. What differs is only
// the scheduling: MAME drives its peripherals off emu_timers, this runs them
// off a plain OSC1 tick counter instead.
//
// Fitting it into the engine: like the fake CPUs (David_And_John_fake_cpu,
// LCDonald_fake_cpu), this subclasses SM5XX purely so the loader, renderer,
// sound and save-state code can keep treating every game as "some SM5XX".
// SM5XX::step() isn't virtual, but it calls update_sound() exactly once per
// call, so that's where the real work happens: with `frequency` = OSC1
// (32768 Hz), main.cpp calls step() 32768 times per emulated second and each
// call advances the chip by one OSC1 tick - peripherals first, then however
// many CPU cycles fit in that tick (1 on OSC1, ~30.5 on the 1 MHz OSC3 some
// games switch to). That also gives get_active_sound() a fresh buzzer level
// for every audio sample. execute_curr_opcode() is a no-op.
//
// Segments: MAME's write_segs callback names each LCD dot "COM.SEG" (16 COMs
// x up to 51 SEGs); the pack stores those as (col, line, word) = (COM, SEG, 0)
// - see CONVERT_ROM/prepare_e0c6200_svgs.py.
class E0C6S46 : public SM5XX {
public:
    enum class Model : uint8_t { S46 = 0, S48 = 1 };

    explicit E0C6S46(Model model);

    void init() override;
    void load_rom(const uint8_t* file_hex, size_t size_hex) override;

    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override;
    bool screen_is_on() override { return true; }
    bool get_active_sound() override { return s_.bz_pulse != 0; }

    bool save_state(FILE* file) override;
    bool load_state(FILE* file) override;
    uint8_t get_cpu_type_id() override { return 5; }  // CPU_TYPE_E0C6S46 (see std/savestate.h)

    // Per-board wiring MAME sets in each machine config: whether the OSC3
    // resistor is fitted (1 MHz fast clock) and what the P ports read back
    // when set to input. Call after init(), before running.
    void configure_for_ref(const std::string& ref);

    // K input pins (0-3 = K00-K03, 4-7 = K10-K13) - `level` is the pin level,
    // and these boards' buttons are active-low (pressed = false). Port of
    // e0c6s46_device::execute_set_input(): edges raise the K0/K1 interrupts.
    void set_k_line(int line, bool level);

    // A button wired straight to the RESET pin (Digital Monster): asserting
    // resets the chip and holds the CPU until released.
    void set_reset_line(bool asserted);

    Model model() const { return model_; }

    // Advances the chip by `ticks` OSC1 ticks (32768 per emulated second) as
    // fast as possible: the same per-tick work as step(), minus SM5XX::step()'s
    // bookkeeping (~2.3x faster on host). For catching a virtual pet up on
    // time that passed while the app was closed (switch/source/core/
    // pet_time.cpp). Buttons should be released first; the buzzer and LCD
    // keep running, so the caller decides whether to show/hear them.
    void fast_forward(uint64_t ticks) {
        while (ticks-- > 0) tick_osc1();
    }

    // Wall-clock time (Unix seconds) the last loaded save state was written
    // at, or 0 if unknown (none loaded, or a version-1 state from before this
    // was recorded). Set by load_state(); save_state() stamps time(nullptr).
    int64_t saved_unix_time() const { return saved_unix_time_; }

    // Direct access to a data-RAM nibble (0x000 - RAM size), for host-side
    // helpers that set a game's clock (switch/source/input/
    // e0c6200_clock_setup.cpp). Out-of-range reads return 0, writes are
    // ignored.
    uint8_t ram_nibble(int addr) const { return (addr >= 0 && addr < ram_size_) ? s_.ram[addr] : 0; }
    void set_ram_nibble(int addr, uint8_t v) {
        if (addr >= 0 && addr < ram_size_) s_.ram[addr] = v & 0xf;
    }

    // For host-side tests: the 16x51 LCD as last driven.
    bool lcd_dot(int com, int seg) const { return s_.lcd[com][seg] != 0; }
    uint16_t debug_pc() const { return s_.pc; }
    uint8_t debug_irqmask(int reg) const { return s_.irqmask[reg]; }
    uint8_t debug_ram(int addr) const { return s_.ram[addr]; }
    void debug_set_ram(int addr, uint8_t v) { s_.ram[addr] = v & 0xf; }
    uint32_t debug_cpu_clock() const { return s_.cpu_clock; }
    bool debug_halted() const { return s_.halt || s_.sleep; }

private:
    // --- SM5XX plumbing (see the class comment) ---
    void update_sound() override { tick_osc1(); }
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

    static constexpr uint32_t kOsc1 = 32768;
    static constexpr int kMaxRomWords = 0x2000;
    static constexpr int kMaxRam = 0x300;
    static constexpr int kMaxVram = 0x66;

    enum { C_FLAG = 1, Z_FLAG = 2, D_FLAG = 4, I_FLAG = 8 };
    enum { IRQREG_CLKTIMER = 0, IRQREG_STOPWATCH, IRQREG_PRGTIMER, IRQREG_SERIAL, IRQREG_INPUT0, IRQREG_INPUT1 };

    // Everything that changes while running, in one POD block so a save
    // state is a single versioned fwrite (see E0C6S46_savestate.cpp).
    struct State {
        // e0c6200 core
        uint16_t op, prev_op;
        uint16_t pc, prev_pc, npc, jpc;
        uint8_t a, b, xp, xh, xl, yp, yh, yl, sp, f;
        uint8_t irq_vector;
        uint8_t possible_irq, halt, sleep;
        int32_t icount;

        // memory
        uint8_t ram[kMaxRam];
        uint8_t vram[2][kMaxVram];
        uint8_t lcd[16][51];

        // interrupt controller / misc i/o
        uint8_t irqflag[6], irqmask[6];
        uint8_t osc, svd, lcd_control, lcd_contrast;
        uint8_t port_r[5], r_dir, port_p[4], p_dir, p_pullup, port_k[2], dfk0;

        // clocks
        uint32_t osc3;        // OSC3 frequency (== kOsc1 when not fitted)
        uint32_t cpu_clock;   // current instruction clock (OSC1 or OSC3)
        uint32_t cycle_accu;  // OSC3 cycles-per-tick remainder
        uint8_t reset_held;

        // 256 Hz core timer, LCD driver (in OSC1 ticks until next event)
        uint8_t src_256_pulse;
        int32_t ticks_to_256, ticks_to_lcd;

        int32_t watchdog_count;
        uint8_t clktimer_count;

        uint8_t stopwatch_on;
        int32_t swl_cur_pulse, swl_slice, swl_count, swh_count;

        uint8_t prgtimer_select, prgtimer_on;
        int32_t prgtimer_src_pulse, prgtimer_cur_pulse;
        uint8_t prgtimer_count, prgtimer_reload;
        uint8_t prgtimer_active;
        int32_t prgtimer_remaining;  // OSC1 ticks

        uint8_t bz_43_on, bz_freq, bz_envelope, bz_envelope_count, bz_duty_ratio;
        uint8_t bz_1shot_on, bz_1shot_running, bz_1shot_count;
        uint8_t bz_pulse;
        uint8_t bz_active;
        int32_t bz_remaining;  // half OSC1 ticks (1/65536 s)
    };

    void power_on();
    void device_reset();
    void tick_osc1();
    void run_cycles(int cycles);

    // e0c6200 core
    void execute_one();
    void do_interrupt();
    bool check_interrupt();
    uint16_t read_rom_word(uint16_t pc) const;

    uint8_t data_read(uint16_t addr);
    void data_write(uint16_t addr, uint8_t data);
    uint8_t read_mx();
    uint8_t read_my();
    uint8_t read_mn();
    void write_mx(uint8_t data);
    void write_my(uint8_t data);
    void write_mn(uint8_t data);
    uint8_t read_rq(int rq);
    void write_rq(int rq, uint8_t data);

    void push(uint8_t data);
    uint8_t pop();
    void push_pc();
    void pop_pc();

    void set_cf(uint8_t data);
    void set_zf(uint8_t data);
    void inc_x();
    void inc_y();
    void do_branch(int condition = 1);

    uint8_t op_inc(uint8_t x);
    uint8_t op_dec(uint8_t x);
    uint8_t op_add(uint8_t x, uint8_t y, int decimal = 0);
    uint8_t op_adc(uint8_t x, uint8_t y, int decimal = 0);
    uint8_t op_sub(uint8_t x, uint8_t y, int decimal = 0);
    uint8_t op_sbc(uint8_t x, uint8_t y, int decimal = 0);
    uint8_t op_and(uint8_t x, uint8_t y);
    uint8_t op_or(uint8_t x, uint8_t y);
    uint8_t op_xor(uint8_t x, uint8_t y);
    uint8_t op_rlc(uint8_t x);
    uint8_t op_rrc(uint8_t x);

    // e0c6s46 peripherals
    uint8_t io_r(uint8_t offset);
    void io_w(uint8_t offset, uint8_t data);
    void write_r(uint8_t port, uint8_t data);
    void write_p(uint8_t port, uint8_t data);
    uint8_t read_p(uint8_t port);

    void core_256_cb();
    void lcd_driver_cb();
    void clock_watchdog();
    void clock_clktimer();
    void clock_stopwatch();
    void clock_prgtimer();
    bool prgtimer_reset_prescaler();
    void prgtimer_cb();
    void schedule_buzzer();
    void buzzer_cb();
    void reset_buzzer();
    void clock_bz_1shot();
    void clock_bz_envelope();
    void reset_bz_envelope();

    Model model_;
    int rom_words_;   // 0x1800 (S46) / 0x2000 (S48)
    int ram_size_;    // 0x280 / 0x300
    int vram_size_;   // 0x50 / 0x66 per bank
    uint8_t p_input_[4] = {0, 0, 0, 0};  // what each P port reads when set to input

    uint16_t rom_[kMaxRomWords] = {};
    State s_{};
    int64_t saved_unix_time_ = 0;
};
