#pragma once

#include "BrickCore.h"

// Texas Instruments TMS1100 (4-bit, 2048x8 ROM in 2 chapters x 16 pages x
// 64 words, 128x4 RAM, K1-K8 inputs, R0-R10 and O0-O7 outputs) with the
// Milton Bradley Microvision board around it: the cartridge is the CPU, the
// console the 16x16 LCD (a Hughes HLCD 0488 driver), the 12-key keypad,
// the rotary paddle and the piezo. Written for Yokoi from the chip's
// documented behaviour - not a port of another emulator's code.
//
// Container (".brk", CONVERT_ROM): core "TMS1100", clock = instruction
// cycles per second (the oscillator / 6). Blobs:
//   "rom"   2048 bytes, linear address = chapter*1024 + page*64 + pc
//   "opla"  32 bytes: O outputs for each (status latch << 4 | A) - the
//           cartridge's output PLA, as a lookup table
// Board wiring, per cartridge, as options. A pin is "R<n>" (R0-R10) or
// "O<n>" (O0-O7):
//   mv.lcd_lp=<pin>       HLCD 0488 latch pulse
//   mv.lcd_dc=<pin>       HLCD 0488 data clock
//   mv.lcd_data=<pin>     first of the 4 data lines (the next 3 pins follow)
//   mv.speaker=<pin>      piezo
//   mv.key.<n>=<pin>,<k>  keypad key n (0-11): while pressed, that R/O
//                         strobe shows on the K bits <k> (1, 2, 4, 8)
//   mv.butmask=<mask>     keys the cartridge leaves out (bit n = key n):
//                         they never read as pressed
//   mv.paddle=<pin>,<k>,<min_us>,<max_us>
//                         rotary paddle, an RC charge: after <pin> goes
//                         high, K bits <k> read 1 once min..max us have
//                         passed (paddle at 0..1); <pin> low discharges it.
//                         min > max turns it the other way. Pinball: R2,8,
//                         700,1300 (its paddle at column 0 at 700 us, 15 at
//                         1300, linearly in between)
// Unset LCD pins = a blank screen (the wiring is checked per game when the
// ROMs are imported).
//
// The 12 Microvision cartridges (MAME software list "microvision",
// CONVERT_ROM/microvision_carts.json: clock, PLA type, paddle and key mask
// of each, identified by the ROM's sha1). TMS1100 unless noted:
//   Block Buster       1979   Mindbuster         1979   Baseball      1980
//   Connect Four       1979   Star Trek: Phaser  1979   Sea Duel      1980
//   Bowling            1979     Strike                  Alien Raiders 1981
//   Pinball            1979   Vegas Slots        1979   Cosmic Hunter 1981
//                                                       Super Blockbuster 1982
// Connect Four also had an Intel 8021 version, and the first runs of Block
// Buster and Bowling were 8021s too (not dumped); the 8021 isn't supported.
// The output PLA isn't per cartridge: there are two types (0 and 1). Clock
// = the oscillator / 6 (350, 500 or 550 kHz oscillators).
//
// Display: the 16x16 dots as 32 bytes, row-major (dot r,c at byte r*2 +
// c/8, bit c%8), each integrated over 1/60 s frames since the games scan
// the rows. Inputs: pseudo-port "BTN", mask = key number 0-11, level 1 =
// pressed; pseudo-port "KNOB", mask 0 / 1 = turn the knob left / right
// while held (the whole way in kKnobFrames); or set_paddle() directly.
class TMS1100 : public BrickCore {
public:
    TMS1100() : BrickCore("TMS1100") {}

    // Knob position, 0 (fully left) .. 1 (fully right).
    void set_paddle(float position) {
        paddle_pos_ = position < 0 ? 0.0f : position > 1 ? 1.0f : position;
    }
    bool has_paddle() const { return paddle_.strobe.port >= 0; }

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override {
        return name == "BTN" ? 0 : name == "KNOB" ? 1 : -1;
    }
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t value) override;
    int peek_ram(uint32_t addr) const override;

private:
    static constexpr int kKeys = 12;
    static constexpr int kDots = 16;
    static constexpr int kKnobFrames = 45;  // end to end while held: 0.75 s

    struct Pin {
        int8_t port = -1;  // 0 = R, 1 = O, -1 = unconnected
        int8_t bit = 0;
    };
    struct Key {
        Pin strobe;
        uint8_t k = 0;
    };
    struct Paddle {
        Pin strobe;  // unconnected = no paddle
        uint8_t k = 0;
        uint32_t min_cycles = 0, max_cycles = 0;
    };

    void load_options();
    static Pin parse_pin(const std::string& s);
    bool pin_level(const Pin& p) const;

    // --- CPU ---
    static uint8_t next_pc(uint8_t pc);
    uint8_t& ram() { return ram_[((x_ & 7) << 4) | (y_ & 15)]; }
    uint8_t read_k();
    void write_r(uint16_t r);
    void write_o(uint8_t o);
    void run_op(uint8_t op);

    // --- board ---
    void outputs_changed();
    void lcd_update();
    void flush_display();
    void end_frame();

    bool options_loaded_ = false;
    std::vector<uint8_t> prg_ = std::vector<uint8_t>(2048, 0);
    uint8_t opla_[32] = {};
    Pin lcd_lp_, lcd_dc_, lcd_data_, speaker_pin_;
    Key keys_[kKeys];
    Paddle paddle_;
    uint32_t frame_cycles_len_ = 1;
    uint16_t butmask_ = 0;

    // CPU state
    uint8_t ram_[128] = {};
    uint8_t a_ = 0, x_ = 0, y_ = 0;
    uint8_t status_ = 1, sl_ = 0;
    uint8_t pc_ = 0, sr_ = 0, pa_ = 0xf, pb_ = 0xf;
    uint8_t ca_ = 0, cb_ = 0, cs_ = 0, cl_ = 0;
    uint16_t r_ = 0;
    uint8_t o_ = 0;
    uint64_t now_ = 0;  // instruction cycles since reset

    // HLCD 0488
    uint8_t lcd_lp_level_ = 0, lcd_dc_level_ = 0, lcd_addr_ = 0;
    uint8_t lcd_latch_[8] = {};
    uint16_t lcd_row_ = 0, lcd_col_ = 0;

    // paddle RC: when its strobe last went high (0 = discharged)
    uint64_t paddle_since_ = 0;
    uint8_t paddle_strobe_ = 0;
    float paddle_pos_ = 0.5f;

    // dot integration
    uint64_t last_flush_ = 0;
    uint32_t frame_cycles_ = 0;
    uint32_t acc_[kDots][kDots] = {};
    float bri_[kDots][kDots] = {};
    uint8_t shown_[kDots * 2] = {};

    uint16_t buttons_ = 0;
    uint8_t knob_ = 0;  // bit 0 turning left, bit 1 right
};
