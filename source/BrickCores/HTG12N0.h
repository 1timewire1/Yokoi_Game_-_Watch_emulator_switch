#pragma once

#include "HT943.h"

// Holtek HTG12N0 (HT4BIT family, as HT943) - a port of BrickEmuPy's
// cores/HTG12N0.py (github.com/azya52/BrickEmuPy, CC0). Mickey Deluxe Virtual
// Game virtual pet.
//
// Differences from HT943: ROM banks (port B's low bits select PC 13:12), 256
// nibbles of RAM in two banks behind R1R0 < 128, a separate 128-nibble display
// RAM at R1R0 >= 128 (the display, blank while port C's LCD bit is off), and an
// RTC tick that raises the external interrupt and ends HALT.
class HTG12N0 : public HT943 {
public:
    HTG12N0() : HT943("HTG12N0") {}

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int index) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    uint8_t& ram(int rp) override;
    void io_op(uint8_t op) override;
    // Both banks, by raw index - clock cells are in RAM bank 0 or 1.
    bool poke_ram(uint32_t addr, uint8_t v) override {
        if (addr >= ram_.size()) return false;
        ram_[addr] = v;
        return true;
    }
    int peek_ram(uint32_t addr) const override { return addr < ram_.size() ? ram_[addr] : -1; }

private:
    static constexpr uint8_t kRamBank1 = 0x01;
    static constexpr uint8_t kLcdOn = 0x02;

    std::vector<uint8_t> vram_ = std::vector<uint8_t>(128, 0);
    double rtc_div_ = 0, rtc_counter_ = 0;
    uint8_t pb_ = 0, port_c_ = 0;
};
