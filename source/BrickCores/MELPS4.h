#pragma once

#include "BrickCore.h"

// Mitsubishi MELPS 4 family, M58846 only - a port of MAME's
// src/devices/cpu/melps4 (melps4.cpp, melps4op.cpp, m58846.cpp; hap,
// BSD-3-Clause) together with the board wiring of the hh_melps4.cpp VFD
// tabletop games, selected by the container's "melps4.game" option:
//   cfrogger  Coleco Frogger   M58846-701P  12 grids x 16 plates
//
// Runs on the BrickCore plumbing (a ".brk" container, CONVERT_ROM/
// prepare_hmcs40.py, which builds these sets too): clock = MAME's oscillator
// / 6 (6 t-states per machine cycle) - 100000 instruction cycles per second
// for Frogger's 600 kHz. The 4096-byte MAME dump is the "rom" blob, read as
// 2048 little-endian 16-bit words (9 bits used).
//
// Display: like the HMCS40 core, the VFD is a grid x plate matrix integrated
// over 1/60 s frames, kept as 5 bytes per grid (plate p at byte grid*5 + p/8,
// bit p%8), so MAME's SVG segment "g.p" is Yokoi's (g*5 + p/8, p%8, 0).
//
// Inputs: one pseudo-port "BTN", mask = button number, level 1 = pressed:
//   cfrogger: 0 up, 1 right, 2 left, 3 down, 4 skill switch (1 <-> 2)
// The unit's RESET button is BrickCore's "RES" pin.
class MELPS4 : public BrickCore {
public:
    MELPS4() : BrickCore("MELPS4") {}

    enum class Game : uint8_t { Cfrogger };

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override { return name == "BTN" ? 0 : -1; }
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t value) override;
    int peek_ram(uint32_t addr) const override;

private:
    static constexpr int kGrids = 16;
    static constexpr int kCols = 16;
    static constexpr int kFrameCycles = 1667;  // 1/60 s at 100 kHz
    static constexpr int kStackLevels = 3;
    static constexpr uint16_t kPrgMask = 0x7ff;   // prgwidth 11
    static constexpr uint8_t kDataMask = 0x7f;    // datawidth 7
    static constexpr uint16_t kDMask = 0xfff;     // 12 D pins
    static constexpr uint8_t kSmPage = 2;
    static constexpr uint8_t kIntPage = 1;
    static constexpr uint16_t kSpMask = 0x7 << 4;
    static constexpr uint16_t kBaOp = 0x01;
    static constexpr uint8_t kXamiMask = 0xf;

    enum Port : uint8_t { PortS = 0, PortF, PortG, PortU };

    void load_options();

    // --- CPU ---
    uint8_t ram_r() const { return ram_[(z_ << 6 | x_ << 4 | y_) & kDataMask] & 0xf; }
    void ram_w(uint8_t v) { ram_[(z_ << 6 | x_ << 4 | y_) & kDataMask] = v & 0xf; }
    void push_pc();
    void pop_pc();
    void tick_timer();
    void write_v(uint8_t data);
    void do_interrupt(int which);
    void check_interrupt();
    uint8_t read_gen_port(int port);
    void write_gen_port(int port, uint8_t data);
    int read_d_pin(int bit);
    void write_d_pin(int bit, int state);  // bit 16 = clear all
    void write_t(int state);
    void run_op();
    void op_lcps();
    void op_b();
    void op_bm();
    void op_rt();
    void op_xam();
    void op_dey();
    void op_amc();

    // --- board (hh_melps4.cpp) ---
    void board_write_port(int port, uint8_t data);
    void board_write_d(uint16_t data);
    uint16_t board_read_k() const;
    void set_display(uint32_t grid, uint32_t plate);
    void flush_display();
    void end_frame();

    Game game_ = Game::Cfrogger;
    bool options_loaded_ = false;
    std::vector<uint16_t> prg_ = std::vector<uint16_t>(0x800, 0);

    // CPU state (melps4.h)
    uint16_t pc_ = 0, prev_pc_ = 0;
    uint16_t stack_[kStackLevels] = {};
    uint16_t op_ = 0, prev_op_ = 0;
    uint8_t bitmask_ = 0;
    uint16_t port_d_ = 0;
    uint8_t port_s_ = 0, port_f_ = 0, port_t_ = 0;
    uint8_t sm_ = 0, sms_ = 0, ba_flag_ = 0;
    uint8_t sp_param_ = 0, cps_ = 0, skip_ = 0;
    uint8_t inte_ = 0, intp_ = 1;
    uint8_t irqflag_[3] = {};
    uint8_t int_state_ = 0;
    uint8_t prohibit_irq_ = 0, possible_irq_ = 0;
    uint8_t tmr_count_[2] = {};
    uint8_t tmr_reload_ = 0;
    uint8_t tmr_irq_enabled_[2] = {};
    uint8_t a_ = 0, b_ = 0, e_ = 0;
    uint8_t y_ = 0, y2_ = 0, x_ = 0, x2_ = 0, z_ = 0, z2_ = 0, cy_ = 0, cy2_ = 0;
    uint8_t h_ = 0, l_ = 0, c_ = 7, v_ = 0, w_ = 0;
    uint8_t ram_[128] = {};
    int cycles_ = 0;

    // board state
    uint32_t buttons_ = 0;
    uint8_t skill_ = 0;      // cfrogger: Skill 1/2 slide switch on K3 (kept over resets)
    uint16_t inp_mux_ = 0;
    uint32_t grid_ = 0;
    uint32_t plate_ = 0;

    // display integration
    uint64_t now_ = 0;
    uint64_t last_flush_ = 0;
    uint32_t frame_cycles_ = 0;
    uint32_t acc_[kGrids][kCols] = {};
    float bri_[kGrids][kCols] = {};
    uint8_t shown_[kGrids * 5] = {};
};
