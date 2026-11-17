#pragma once

#include <memory>

#include "BrickCore.h"

class upd7810_device;

// Hartung Game Master (1990, also sold as Impel / Systema 2000 / Videojet /
// Prodis PDJ-10...): NEC uPD78C11 at 12 MHz (4 KB internal ROM - the boot
// code, 256 bytes internal RAM), 2 KB external RAM, cartridge ROM at
// 0x8000, a 61x64 LCD on two Epson SED1520 drivers (top and bottom half),
// a 1-bit speaker. Written for Yokoi from the hardware as MAME's
// src/mame/handheld/gmaster.cpp documents it; the CPU is MAME's uPD7810
// core (upd7810.cpp/h - license:BSD-3-Clause, Juergen Buchmueller).
//
// Container (".brk", CONVERT_ROM/build_gamemaster_pack.py): core
// "UPD78C11", clock = 4000000 (12 MHz XTAL, 3 clocks per CPU cycle).
// Blobs: "rom" the 4 KB internal ROM (d78c11agf_e19.u1), "cart" the
// cartridge. Inputs: pseudo-port "JOY", mask = the bit (MAME's JOY row,
// active low on port A), level 1 = pressed. Segments: (x, y, 0), x 0-60,
// y 0-63, one per dot.
class GameMaster : public BrickCore {
public:
    GameMaster();
    ~GameMaster();

    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override {
        return word == 0 && col < kW && line < kH && fb_[line][col];
    }

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override { return name == "JOY" ? 0 : -1; }
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t value) override;
    int peek_ram(uint32_t addr) const override;

private:
    static constexpr int kW = 61, kH = 64;

    // Epson SED1520: 4 pages x 80 columns of display RAM, as MAME's sed1520.cpp.
    struct SED1520 {
        uint8_t ddr[320] = {};
        uint8_t lcd_on = 0, page = 3, column = 0, old_column = 0, start_line = 0;
        uint8_t adc = 1, duty = 0, data = 0, modify_write = 0, static_drive = 0;
        void control_write(uint8_t d);
        void data_write(uint8_t d);
        uint8_t data_read();
        uint8_t status_read() const { return (uint8_t)((adc << 6) | (lcd_on << 5)); }
    };

    uint8_t read(uint32_t addr);
    void write(uint32_t addr, uint8_t data);
    void render();

    std::unique_ptr<upd7810_device> cpu_;
    std::vector<uint8_t> bios_, cart_;
    uint8_t ram_[0x800] = {};
    uint8_t iram_[0x100] = {};
    SED1520 lcd_[2];
    uint8_t chipsel_ = 0;
    uint8_t joy_ = 0xff;
    uint32_t frame_cycles_ = 0, frame_len_ = 66666;
    bool fb_[kH][kW] = {};
};
