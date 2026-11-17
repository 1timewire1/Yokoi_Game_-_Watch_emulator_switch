#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "BrickCore.h"

// National Semiconductor COP411 (COP410 instruction set, 512x8 ROM, 32x4
// RAM, G0-G2, D0-D1, no IN/timer) - a port of MAME's src/devices/cpu/
// cop400 (Curt Coder, BSD-3-Clause), just the parts a COP411 uses. Here
// as the sound CPU of Entex Stargate (HMCS40.*); step() runs one cycle, as
// MAME's execute_run() loop body does (a 2-byte instruction takes two).
class Cop411 {
public:
    std::function<uint8_t()> read_l;          // L0-L7 input (Q not enabled)
    std::function<uint8_t()> read_g;          // G0-G2 input
    std::function<void(uint8_t)> write_d;     // D0-D1
    std::function<void(uint8_t)> write_g;     // G0-G2
    std::function<void(int)> write_sk;        // SK

    void load(const std::vector<uint8_t>& rom);
    void reset();
    void step();
    void serialize(BrickStateIO& io);

private:
    uint8_t rom(uint16_t a) const { return rom_[a & 0x1ff]; }
    static int ram_index(uint8_t a) { return ((a >> 4) & 3) * 8 + (a & 7); }
    uint8_t ram_r(uint8_t a) const { return ram_[ram_index(a)]; }
    void ram_w(uint8_t a, uint8_t v) { ram_[ram_index(a)] = v & 0xf; }
    uint8_t in_g() { return (read_g ? read_g() : 0) & 0x7; }
    void out_sk(int v) { sk_ = (uint8_t)v; if (write_sk) write_sk(v); }
    void out_d(uint8_t v) { if (write_d) write_d(v & 0x3); }
    void out_l(uint8_t) {}
    void write_g_(uint8_t v) { g_ = v; if (write_g) write_g(v & 0x7); }
    void write_en(uint8_t v);
    void sk_update() { out_sk((en_ & 1) ? skl_ : 0); }
    void push(uint16_t v) { sb_ = sa_; sa_ = v; }
    void pop() { pc_ = sa_; sa_ = sb_; }
    void skip() { skip_ = 1; }
    void serial_tick();
    void exec(uint8_t operand);
    void exec33(uint8_t operand);
    void lbi(uint8_t operand);

    std::vector<uint8_t> rom_ = std::vector<uint8_t>(0x200, 0);
    uint8_t ram_[32] = {};
    uint16_t pc_ = 0, sa_ = 0, sb_ = 0;
    uint8_t a_ = 0, b_ = 0, c_ = 0, g_ = 0, q_ = 0, en_ = 0, sio_ = 0, si_ = 0, skl_ = 0, sk_ = 0;
    uint8_t skip_ = 0, skip_lbi_ = 0, halt_ = 0, opcode_ = 0x44, second_byte_ = 0;
};
