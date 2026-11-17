#pragma once

#include <memory>

#include "BrickCore.h"

class upd7810_device;

// Epoch Game Pocket Computer (1984): NEC uPD78C06 (4 KB internal ROM - the
// BIOS, with the built-in Puzzle and Graphics programs), 2 KB RAM, up to
// 32 KB cartridge ROM, a 75x64 LCD on three Hitachi HD44102 column drivers,
// a 1-bit speaker on the CPU's TO pin. After MAME's src/mame/epoch/gamepock.cpp
// (license:BSD-3-Clause, copyright-holders:Wilbert Pol, hap); the CPU is
// MAME's uPD7810 core (upd7810.cpp/h, upd7810_opcodes.cpp, upd7810_table.cpp)
// and the LCD drivers follow MAME's hd44102.cpp (Curt Coder).
//
// Container (".brk", CONVERT_ROM/prepare_gamepock.py): core "UPD78C06",
// clock = 1500000 (MAME: 6 MHz XTAL, 4 clocks per CPU cycle). Blobs:
//   "rom"   the 4 KB BIOS (egpcboot.bin)
//   "cart"  the cartridge ROM, if any (none = the built-in programs)
// Inputs: pseudo-ports "IN0"/"IN1", mask = the bit (MAME's two input rows),
// level 1 = pressed. Segments: (x, y, 0), x 0-74, y 0-63, one per dot.
class GamePocket : public BrickCore {
public:
    GamePocket();
    ~GamePocket();

    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override {
        return word == 0 && col < kW && line < kH && fb_[line][col];
    }

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override {
        return name == "IN0" ? 0 : name == "IN1" ? 1 : -1;
    }
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t value) override;
    int peek_ram(uint32_t addr) const override;

private:
    static constexpr int kW = 75, kH = 64;

    struct HD44102 {
        uint8_t ram[4][50] = {};
        uint8_t status = 0x20 | 0x40;  // display off, count up
        uint8_t output = 0, page = 0;
        int x = 0, y = 0;
        void control_w(uint8_t data);
        void data_w(uint8_t data);
        void count();
        bool dot(int sx, int sy) const;  // sx 0-49, sy 0-31, as MAME's render()
    };

    uint8_t read(uint32_t addr) const;
    void write(uint32_t addr, uint8_t data);
    void control_w(uint8_t data);
    uint8_t input_r() const;
    void render();

    std::unique_ptr<upd7810_device> cpu_;
    std::vector<uint8_t> bios_, cart_;
    uint8_t ram_[0x800] = {};
    uint8_t iram_[0x80] = {};
    HD44102 lcd_[3];
    uint8_t control_ = 0, lcd_data_ = 0;
    uint8_t inputs_[2] = {0xff, 0xff};
    uint32_t frame_cycles_ = 0, frame_len_ = 25000;
    bool fb_[kH][kW] = {};
};
