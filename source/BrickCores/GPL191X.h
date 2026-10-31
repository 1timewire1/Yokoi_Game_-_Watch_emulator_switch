#pragma once

#include "BrickCore.h"

// GeneralPlus GPL191X / Sunplus SPL61X (6502-family CPU, banked ROM, LCD
// driver with display RAM, two timers, two-channel tone generator) - a port
// of BrickEmuPy's cores/GPL191X.py + GPL191Xsound.py
// (github.com/azya52/BrickEmuPy, CC0). BrickEmuPy runs its "SPL61X" games
// (Bandai's D-Power Digivice) and "GPL191X" ones (Tamagotchi Mini 2017) on
// this same core.
//
// The opcode handlers (GPL191X_ops.inc) are generated mechanically from the
// Python source and keep its exact behaviour, cycle counts and quirks; the
// rest is hand-ported. State uses 64-bit ints / doubles where the Python
// does (Python ints are unbounded, and the timer dividers are floats:
// clock / 32768), so the two stay in lock-step.
//
// Matches BrickEmuPy exactly (CONVERT_ROM/tmp/review/compare.py) as of its
// "fix GPL191X" commit (2026-10-28), which made five changes angel_hp's port
// first had for the D-Power Digivice the behaviour for every game:
//   - an IRQ is held off for one instruction after CLI (a real 6502 trait);
//   - port EF direction writes apply (bit 0 forced to input);
//   - three ROM bank bits (the D-Tector's 256 KB dump uses banks 5-7);
//   - timer 0 in counter mode counts pulses on pin CD0 - the D-Tector's Step
//     button (without it, steps did nothing);
//   - any port CD input change re-evaluates the timers, not just pin CD1.
//
// Display: get_VRAM() is the 128-byte display RAM at 0x300 while the LCD and
// the 32 kHz clock are on. Sound: each channel's tone (TONE mode) becomes
// BrickCore's square tone - BrickEmuPy's direct (non-tone) writes are
// cancelled by an immediate stop in the same instant, so they're silent
// there too.
class GPL191X : public BrickCore {
public:
    explicit GPL191X(const char* name = "GPL191X") : BrickCore(name) {}

protected:
    void reset() override;
    int execute() override { return (int)clock_step(); }
    int64_t execute_scaled() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;

public:  // (public for the verification harness)
    enum Port { kAB = 0, kCD = 1, kEF = 2 };
    static constexpr int64_t kStack = 0x100;

    double clock_step();
    int64_t rd(int64_t addr);
    void wr(int64_t addr, int64_t value);
    int64_t get_ps() const;
    void set_ps(int64_t ps);
    void adc(int64_t operand);
    void sbc(int64_t operand);
    void irq();
    void nmi();
    int64_t port_read(int port) const;
    void update_audio_div();
    void update_timers_div();
    void timers_clock(double exec_cycles);
    int64_t sfr_get(int64_t addr);
    bool sfr_set(int64_t addr, int64_t value);
    void emit_port(int port);

    // sound (GPL191Xsound.py)
    void snd_set_data(int ch, int64_t ctrl, int64_t data);
    void snd_tone(int ch, double div);
    void snd_stop(int ch);
    void snd_apply();

    typedef int (GPL191X::*OpFn)();
    static const OpFn kOps[256];
#include "GPL191X_decl.inc"

    std::vector<uint8_t> mem_;
    size_t mem_size_ = 1;
    double sub_clock_div_ = 0;
    int64_t pullup_[3] = {0, 0, 0};
    int64_t port_in_[3][2] = {{0, 0}, {0, 0}, {0, 0}};

    // CPU
    int64_t A_ = 0, X_ = 0, Y_ = 0, SP_ = 0, PC_ = 0;
    int64_t NF_ = 0, VF_ = 0, DF_ = 0, BF_ = 0, IF_ = 0, ZF_ = 0, CF_ = 0;
    int64_t BANK_ = 0;
    int64_t CPU_ENBL_ = 1, CPU_DIV_ = 2;

    // SFRs
    int64_t WAKEUP_CTRL_ = 0, WAKEUPREQ_ = 0, ENTER_SLEEP_ = 0, TIME_BASE_ = 0, TIMER_CTRL_ = 0;
    int64_t CLK_CPU_CTRL_ = 0, CLK32K_ENABLE_ = 0x80;
    int64_t PDIR_[3] = {0, 0, 0}, PLATCH_[3] = {0, 0, 0};
    int64_t INT_CTRL_ = 0, INT_CLEAR_ = 0, IREQ_ = 0, NMI_CTRL_ = 0;
    int64_t AUDIO_CTRL_[2] = {0, 0}, AUDIO_DATA_[2] = {0, 0};
    int64_t LCD_CTRL_ = 0;

    // timers
    double TM0_counter_ = 0, TM0_div_ = 0, TM1_counter_ = 0, TM1_div_ = 0;
    double T64HZ_counter_ = 0, T128HZ_counter_ = 0, T2K_counter_ = 0;
    int64_t T64HZ_ = 0;
    int64_t TM0_ = 0, TM0_PRELOAD_ = 0, TM0_LOAD_ = 0, TM1_ = 0, TM1_PRELOAD_ = 0, TM1_LOAD_ = 0;

    // sound
    double envelope_[2] = {0, 0};
    double ch_freq_[2] = {0, 0};   // what each channel is emitting, 0 = silent
};

// BrickEmuPy's "SPL61X" core name (the D-Power Digivice) - the same core.
class SPL61X : public GPL191X {
public:
    SPL61X() : GPL191X("SPL61X") {}
};
