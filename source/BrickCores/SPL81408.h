#pragma once

#include "BrickCore.h"

// Sunplus SPL81408 (a cut-down 6502 with its own opcode map, 4 MHz) - a port
// of BrickEmuPy's cores/SPL81408.py + SPL81408sound.py
// (github.com/azya52/BrickEmuPy, CC0). McDonald's Chicken McNugget Tetris.
//
// Memory map: I/O registers 0x00-0x1F, LCD RAM 0x20-0x3C (the display -
// get_VRAM() is these 29 bytes), CPU RAM 0x60-0xFF (the stack lives in it:
// SP indexes page 0), ROM everywhere else. Inputs are port EF ("mask" = a
// bitmask); any change on it raises the external interrupt and can wake the
// CPU from sleep. Sound is one channel: a DAC level, or a square wave
// toggled by timer 0's overflow.
class SPL81408 : public BrickCore {
public:
    SPL81408() : BrickCore("SPL81408") {}

protected:
    void reset() override;
    int execute() override { return (int)(execute_scaled() / kTickHz); }
    int64_t execute_scaled() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;

private:
    enum Port { kCD = 0, kEF = 1, kAB = 2 };

    uint8_t rom_byte(uint32_t addr) const { return rom()[addr % rom().size()]; }
    uint16_t rom_word(uint32_t addr) const { return rom_byte(addr) | (rom_byte(addr + 1) << 8); }
    uint8_t read_mem(uint32_t addr);
    void write_mem(uint32_t addr, uint8_t value);
    uint8_t io_read(uint8_t reg);
    void io_write(uint8_t reg, uint8_t value);
    uint8_t port_read(int port) const;

    uint8_t ps() const;
    void set_ps(uint8_t ps);
    void push(uint8_t v);
    uint8_t pull();
    void irq();
    void adc(uint8_t operand);
    void sbc(uint8_t operand);
    void nz(uint8_t v) { nf_ = v >> 7; zf_ = v == 0; }
    void compare(int reg, uint8_t operand);
    int branch(bool taken);
    void timers_clock(int64_t scaled_cycles);
    void wake(uint8_t source);

    // sound (SPL81408sound.py)
    void sound_set_data(uint8_t ctrl, uint8_t data);
    void sound_toggle();

    // mask options
    bool options_loaded_ = false;
    uint8_t pullup_[3] = {};

    // CPU
    uint32_t pc_ = 0;
    uint8_t sp_ = 0, a_ = 0, x_ = 0;
    uint8_t nf_ = 0, vf_ = 0, bf_ = 0, df_ = 0, if_ = 0, zf_ = 0, cf_ = 0;
    uint32_t cpu_div_ = 2;
    uint8_t cpu_enbl_ = 1;
    std::vector<uint8_t> ram_ = std::vector<uint8_t>(0xA0, 0);
    std::vector<uint8_t> lcd_ = std::vector<uint8_t>(0x1D, 0);

    // I/O
    uint8_t rom_bank_ = 0, wakeup_ctrl_ = 0, wakeup_req_ = 0, time_base_ = 0, timer_ctrl_ = 0;
    uint8_t cpu_ctrl_ = 0, clk32768_disable_ = 0, int_ctrl_ = 0, ireq_ = 0, sys_ctrl_ = 0;
    uint8_t lcd_ctrl_ = 0, audio_ctrl_ = 0, audio_data_ = 0;
    uint8_t pdir_[3] = {}, platch_[3] = {};
    uint8_t input_low_[3] = {}, input_high_[3] = {};  // pins driven 0 / 1 from outside

    // timers, in cycles * kTickHz (sleep ticks are fractional cycles)
    int64_t t2khz_counter_ = 0;
    uint32_t t2khz_ = 0;
    int64_t tm0_ = 0;
    uint16_t tm0_reload_ = 0;

    // sound
    uint8_t envelope_ = 0;
    int8_t toggle_state_ = 1;
};
