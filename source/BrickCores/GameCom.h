#pragma once

#include <memory>

#include "BrickCore.h"

class sm8500_cpu;

// Tiger Game.com (1997): Sharp SM8521 (an SM8500 CPU, 5.5296 MHz) with its
// own LCD controller, DMA blitter, timers and sound; 200x160 LCD with four
// shades, 256 KB kernel ROM, cartridges up to 2 MB, a touch screen.
// Written for Yokoi after MAME's src/mame/tiger/gamecom*.cpp (license:
// BSD-3-Clause, Wilbert Pol, Robbbert); the CPU is MAME's sm8500 core.
//
// Container (".brk", CONVERT_ROM/build_gamecom_pack.py): core "SM8521",
// clock 5529600. Blobs: "rom" internal.bin (4 KB), "kernel" external.bin
// (256 KB), "cart" the cartridge (absent = the built-in programs).
// Inputs: ports "IN0", "IN1", "IN2" (MAME's rows, mask = bit, active low).
// Display: shade(x, y) 0 (white) .. 4 (black) - the renderer draws it as a
// picture; segments (x, y, s) are lit when the pixel is at least shade s.
class GameCom : public BrickCore {
public:
    static constexpr int kW = 200, kH = 160;

    GameCom();
    ~GameCom();

    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override {
        return col < kW && line < kH && word >= 1 && shade_[line][col] >= word;
    }
    // 0 = white .. 4 = black (MAME's palette index, reversed).
    uint8_t shade(int x, int y) const { return shade_[y][x]; }
    const uint8_t* shades() const { return &shade_[0][0]; }
    // Touch screen: stylus at (x, y) in LCD pixels, or released.
    void set_stylus(int x, int y, bool down);
    // Sound: the DAC and the two waveform channels mixed, as MAME's routes
    // (8-bit DAC at 0.5, each 4-bit channel at 0.25 times its volume).
    bool has_analog_sound() override { return !basic_sound_; }
    // Optional settings: the old one-bit buzzer instead, and an approximate
    // SG2 noise channel (MAME has none).
    void set_sound_options(bool basic, bool noise) { basic_sound_ = basic; noise_on_ = noise; }
    float get_sound_level() override { return sound_level_; }
    bool cart_mapped() const { for (int i = 1; i < 5; i++) if (bank_[i] & 0x80000000u) return true; return false; }

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int) const override { return 0; }
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t value) override;
    int peek_ram(uint32_t addr) const override;

private:
    uint8_t read(uint16_t a);
    void write(uint16_t a, uint8_t v);
    void pio_w(uint16_t off, uint8_t data);
    void internal_w(uint16_t off, uint8_t data);
    void set_mmu(int mmu, uint8_t data);
    void handle_input_press(uint16_t mux);
    void handle_stylus_press(int column);
    void handle_dma();
    void update_timers(int cycles);
    void draw_column(int x);
    void sound_tick(int cycles);
    const uint8_t* bank_ptr(uint32_t off) const;

    std::unique_ptr<sm8500_cpu> cpu_;
    std::vector<uint8_t> irom_, kernel_, cart_;  // cart_ mirrored to 2 MB
    uint8_t ram_[0x400] = {};      // 0000-03ff (registers, I/O)
    uint8_t vram_[0x4000] = {};    // a000-dfff, write only
    uint8_t nvram_[0x2000] = {};   // e000-ffff
    // bank bases as offsets: bit 31 = cartridge, else kernel
    uint32_t bank_[5] = {};
    bool cart_sel_ = false;
    uint8_t in_[3] = {0xff, 0xff, 0xff};
    int stylus_x_ = -1, stylus_y_ = -1;
    // Straight into the cartridge: taps the menu's CARTRIDGE icon until the
    // cartridge has been running for a second (autoboot_ in frames).
    uint32_t autoboot_frame_ = 0, autoboot_mapped_ = 0;
    bool autoboot_done_ = false;
    void autoboot_frame();

    struct Timer { uint8_t enabled = 0; uint32_t prescale_count = 0, prescale_max = 1; uint8_t upcounter_max = 0; } timer_[2];
    struct Sound { uint8_t sgc = 0, sg0l = 0, sg1l = 0, sg2l = 0, sgda = 0; uint16_t sg0t = 0, sg1t = 0, sg2t = 0;
                   uint8_t sg0w[16] = {}, sg1w[16] = {}; } snd_;
    uint32_t snd_acc_[2] = {}, snd_cnt_[2] = {0x40, 0x40};
    uint8_t snd_out_[2] = {}, dac_ = 0x80;
    float sound_level_ = 0.0f;
    bool basic_sound_ = false, noise_on_ = false;
    uint32_t noise_acc_ = 0, noise_lfsr_ = 1;
    uint8_t noise_out_ = 0;
    uint32_t clock_acc_ = 0, clock_period_ = 0;   // CLKT 1 s / 1 min timer, in cycles
    uint32_t line_acc_ = 0, line_len_ = 600, frame_lines_ = 167, line_ = 0;  // LCD timing, in CPU cycles
    uint8_t shade_[kH][kW] = {};
};
