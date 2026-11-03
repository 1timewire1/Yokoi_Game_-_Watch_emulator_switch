#pragma once

#include "BrickCore.h"

// Toshiba T7741 and T6770S, 4-bit LCD game MCUs (1981-85) - a port of
// BrickEmuPy's cores/T7741.py and cores/T6770S.py (github.com/azya52/BrickEmuPy,
// CC0). The Popy Animest, Gakken and Bandai LCD games, Spica's D'Artagnan, and
// (T6770S) Bandai's Gundam Space Combat 3 in 1. The two chips share most of
// their opcode map, so one class does both - t6770s_ picks the differences.
//
// Memory: 128 nibbles of RAM, H:L addressed (H is 3 bits). T7741 keeps its two
// return addresses in RAM 0x70-0x75; T6770S has registers for them. Program
// words are 10 bits, stored big-endian as 16.
//
// Display: get_VRAM() is 36 nibbles of GRAM, filled through a shift register
// (outm_lcd) one com at a time, ids "<nibble>_<bit>" in the face SVG.
//
// Inputs: ports INP and IOP. Most games scan a matrix by writing OUTP/IOP, so
// those go out through port_output(). Sound is a buzzer pin against a
// mask-option level, a signal while the two differ.
//
// Time: BrickEmuPy counts prescaler and LCD time in doubles, in sub-clock ticks
// (a .brick's sub_clock may be 32768 or the main clock, and wait_frame/wait_com
// return a fractional number of cycles), and this does the same.
class T7741 : public BrickCore {
public:
    T7741() : T7741("T7741", false) {}

protected:
    T7741(const char* name, bool t6770s) : BrickCore(name), t6770s_(t6770s) {}

    void reset() override;
    int execute() override { return (int)(execute_scaled() / kTickHz); }
    int64_t execute_scaled() override;
    uint8_t display_byte(int index) const override {
        return (index >= 0 && index < kGramSize) ? (uint8_t)gram_[index] : 0;
    }
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t v) override {
        if (addr >= kRamSize) return false;
        ram_[addr] = v;
        return true;
    }
    int peek_ram(uint32_t addr) const override { return addr < kRamSize ? ram_[addr] : -1; }

private:
    static constexpr int kRamSize = 128;
    static constexpr int kSegCount = 36;
    static constexpr int kComCount = 4;
    static constexpr int kGramSize = (kSegCount / 4) * kComCount;

    double op(uint32_t opcode);     // runs one opcode, returns its cycles
    double op_t7741(uint32_t opcode);
    double op_t6770s(uint32_t opcode);
    double op_shared(uint32_t opcode);
    void load_options();
    uint32_t word_at(uint32_t pc) const;
    int& mhl() { return ram_[(h_ << 4) | l_]; }
    void set_out(int port, int value);
    void buzzer(int level);
    double nop2() { nsf_ = 0; return 32; }

    const bool t6770s_;

    // mask options
    bool options_loaded_ = false;
    double sub_clock_div_ = 1, px_div_ = 0, py_div_ = 0, pz_div_ = 0, com_div_ = 768, frame_div_ = 3072;
    int sound_gnd_ = 0;

    // Registers. RAM and A stay ints: T6770S's rotate-left doesn't mask to a
    // nibble, in BrickEmuPy, so a cell can briefly hold more than 4 bits.
    uint32_t pc_ = 0xF00, pca_ = 0, pcd_ = 0;
    int a_ = 0, b_ = 0, h_ = 0, l_ = 0, cf_ = 0, nsf_ = 0;
    int pzf_ = 0, pyf_ = 0, pxf_ = 0;
    int inp_ = 0, outp_ = 0, iop_ = 0, bz_ = 0;
    int gram_offset_ = 0, scan_ = 1, pchtmp_ = -1, clc_src_ = 0;
    int ram_[kRamSize] = {};
    int gram_[kGramSize] = {};
    double counter_ = 0, cycle_counter_ = 0;
};

class T6770S : public T7741 {
public:
    T6770S() : T7741("T6770S", true) {}
};
