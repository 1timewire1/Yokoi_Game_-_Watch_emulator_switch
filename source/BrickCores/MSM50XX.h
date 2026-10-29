#pragma once

#include "BrickCore.h"

// OKI MSM50XX (MSM5052/MSM5055 family, 4-bit, 14-bit opcodes) with its
// buzzer/melody generator - a port of BrickEmuPy's cores/MSM50XX.py +
// MSM50XXsound.py (github.com/azya52/BrickEmuPy, CC0). Nelsonic Game
// Watches (The Legend of Zelda, Super Mario Bros.) and Hartung's Formel 1.
//
// One clock() of the Python core = one instruction = MCLOCK_DIV (4) cycles
// of the .brick's 32768 Hz clock. The display is the 32-byte GRAM (segment
// ids "<byte>_<bit>"), forced all-on while GRAM[15] or GRAM[31] bit 0 is set
// (the chip's "all segments" test bits). Inputs are the PS/PK pins; any
// pressed pin wakes the CPU from HALT.
class MSM50XX : public BrickCore {
public:
    MSM50XX() : BrickCore("MSM50XX") {}

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t v) override { ram_at(addr) = v & 0xF; return true; }
    int peek_ram(uint32_t addr) const override { return ram_[addr & (kRamSize - 1)]; }

private:
    enum Port { kPS = 0, kPK = 1 };
    static constexpr int kRamSize = 128;
    static constexpr int kGramSize = 32;

    uint16_t rom_word(uint32_t byte_addr) const;
    uint8_t& ram_at(uint32_t addr) { return ram_[addr & (kRamSize - 1)]; }
    uint32_t ap(uint16_t op) const;
    uint32_t ax(uint16_t op) const;
    void exec(uint16_t op);
    void load_options();

    // sound (MSM50XXsound.py)
    void sound_clock(int cycles);
    void sound_set(uint8_t value);

    bool options_loaded_ = false;
    double freq_div_[16] = {};
    uint32_t format_tbl_offset_ = 0;

    // CPU
    uint8_t acc_ = 0;
    uint32_t pc_ = 0;
    uint8_t cf_ = 0, zf_ = 0, halt_ = 0;
    uint8_t tc_ = 0, int16_ = 0, int32_ = 0, int_ = 0;
    uint8_t pm_ = 0, ps_ = 0, pk_ = 0, ld_ = 0;
    uint8_t fmt_ = 0, backup_ = 0, page_ = 0, areg_ = 0xF;
    std::vector<uint8_t> ram_ = std::vector<uint8_t>(kRamSize, 0);
    std::vector<uint8_t> gram_ = std::vector<uint8_t>(kGramSize, 0);
    int32_t timer16_ = 0, timer32_ = 0;

    // sound
    uint8_t freg_ = 0, freg4_ = 2, mreg_ = 0, snd_state_ = 0;
    int32_t melody_div1_ = 0, melody_div2_ = 0;
};
