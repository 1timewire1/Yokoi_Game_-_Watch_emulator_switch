#pragma once

#include "BrickCore.h"
#include "Cop411.h"

#include <memory>

// Hitachi HMCS40 family (HMCS44 = HD38800, HMCS45 = HD38820; PMOS) - a port
// of MAME's src/devices/cpu/hmcs40 (hap, BSD-3-Clause) together with the
// board wiring of these hh_hmcs40.cpp tabletop VFD games, selected by
// the container's "hmcs40.game" option:
//   packmon   Bandai Packri Monster      HD38800A27  10 grids x 20 plates
//   cdkong    Coleco Donkey Kong         HD38820A45  11 grids x 28 plates
//   egalaxn2  Entex Galaxian 2           HD38820A13  15 grids x 24 plates
//   epacman2  Entex Pac Man 2            HD38820A23  same board as Galaxian 2
//             (epacman2r: the same ROM, the VFD with a red Pac-Man)
//   einvader2 Entex Super Space Invader 2  HD38800A31  12 grids x 14 plates (black version, 450 kHz)
//   tmtron    Tomy Tron                  HD38800A88  10 grids x 22 plates
//   gdigdug   Gakken Dig Dug             HD38820A69   9 grids x 32 plates
//   gscobra   Gakken Super Cobra         HD38820A32   9 grids x 31 plates
//   bbtime    Bandai Burger Time         HD38820A65   6 grids x 28 plates
//   cmspacmn  Coleco Ms. Pac-Man         HD38820A61  11 grids x 26 plates
//   estargte  Entex Stargate             HD38820A42  14 grids x 28 plates, + COP411 sound CPU
//             (its ROM is the container's "sound_rom" blob)
//   bfriskyt  Bandai Frisky Tom          HD38800A77   8 grids x 22 plates
//   bzaxxon   Bandai Zaxxon              HD38800B19  11 grids x 19 plates (450 kHz)
//   zackman   Bandai Zackman             HD38820A49   8 grids x 30 plates
//   bpengo    Bandai Pengo               HD38820A63   8 grids x 28 plates
//   alnattck  Coleco Alien Attack        HD38800A25  10 grids x 20 plates
//   pbqbert   Parker Brothers Q*Bert     HD38820A70   8 grids x 29 plates
//   cpacman   Coleco Pac-Man             HD38820A28/29 11 grids x 26 plates (rev. 1 / 2)
//   gckong    Gakken Crazy Kong          HD38800B01  11 grids x 18 plates
//   bdoramon  Bandai Dokodemo Dorayaki Doraemon  HD38800B43   8 grids x 20 plates
//   kingman   Tomy Kingman               HD38800B23   9 grids x 21 plates
//   ghalien   Gakken Heiankyo Alien      HD38800A04  10 grids x 20 plates
//   machiman  Bandai Machine Man         HD38820A85   5 grids x 19 plates
//   bambball  Bambino Dribble Away Basketball  HD38750A08 (HMCS43)  9 grids x 16 plates
//   vinvader  VTech Invaders             HD38750A45 (HMCS43)  9 grids x 15 plates (300 kHz)
//   eturtles  Entex Turtles              HD38820A43  15 grids x 28 plates, + COP411 sound CPU
//   pairmtch  Bandai Pair Match          HD38820A88 + HD38820A89 (audio, "sound_rom")  6 grids x 12 plates
//   bmboxing  Bambino Knock-Em Out Boxing  HD38750A07 (HMCS43)  9 grids x 12 plates
//
// "hmcs40.family": 43 (HD3875x: 1K+64 ROM, 80x4 RAM, 3 stack levels, R1-R3
// out), 44 (HD3880x) or 45 (HD3882x). The 43's 2176-byte dump loads as MAME's
// ROM_LOAD(0, 0x800) + ROM_CONTINUE(0xf00, 0x80).
//
// Runs on the BrickCore plumbing (a ".brk" container, CONVERT_ROM/
// prepare_hmcs40.py): clock = MAME's oscillator / 4 - 100000 instruction
// cycles per second for its 400 kHz. The 4352-byte MAME dump is the "rom" blob,
// laid out like MAME's ROM_LOAD + ROM_CONTINUE(0x1e80, 0x100).
//
// Display: the VFD is a grid x plate matrix strobed by the program; like
// MAME's pwm_display it is integrated over 1/60 s frames (interpolation 0.5,
// lit above 1% on-time). Display memory is 5 bytes per grid: bytes 0-3 =
// plates 0-31 (bit = plate & 7), byte 4 bit 0 = the grid itself ("<grid>.a"
// in MAME's SVGs - drawn while that grid is strobed at all). So MAME's
// segment "g.p" is Yokoi's (g*5 + p/8, p%8, 0) and "g.a" is (g*5 + 4, 0, 0).
//
// Inputs: one pseudo-port "BTN", mask = button number (see the .cpp's game
// tables), level 1 = pressed.
class HMCS40 : public BrickCore {
public:
    HMCS40() : BrickCore("HMCS40") {}

    enum class Game : uint8_t { Packmon, Cdkong, Egalaxn2, Tmtron, Gdigdug, Gscobra, Bbtime, Cmspacmn, Estargte, Bfriskyt, Bzaxxon, Zackman, Bpengo, Alnattck, Pbqbert, Cpacman, Gckong, Bdoramon, Kingman, Ghalien, Machiman, Bambball, Vinvader, Eturtles, Bmboxing, Pairmtch, PairmtchAudio, Epacman2, Einvader2 };

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
    static constexpr int kCols = 33;          // plates 0-31 + the grid's own ".a"
    static constexpr int kFrameCycles = 1667; // 1/60 s at 100 kHz

    void load_options();
    void load_prg(const std::vector<uint8_t>& rom);
    void cpu_reset();

    // --- CPU (hmcs40.cpp / hmcs40op.cpp) ---
    uint16_t rom_word(uint32_t addr) const { return prg_[addr & prgmask_]; }
    int ram_index(uint32_t addr) const;
    uint8_t ram_r() const { return ram_[ram_index((x_ << 4 | y_) & 0xff)]; }
    void ram_w(uint8_t v) { ram_[ram_index((x_ << 4 | y_) & 0xff)] = v & 0xf; }
    void push_stack();
    void pop_stack();
    void increment_pc();
    void cycle();
    void take_interrupt();
    void clock_timer();
    void clock_prescaler();
    void set_input_line(int line, int state);  // execute_set_input
    uint8_t read_r(uint8_t index);
    void write_r(uint8_t index, uint8_t data);
    int read_d(uint8_t index);
    void write_d(uint8_t index, int state);
    void reset_io();
    void op_xsp();
    void run_op();

    // --- board (hh_hmcs40.cpp) ---
    void board_write_r(uint8_t index, uint8_t data);
    void board_write_d();
    uint8_t board_read_r(uint8_t index) const;
    uint16_t board_read_d() const;
    uint16_t read_inputs(int columns) const;
    void update_board_interrupts();
    void set_interrupt(int line, int state);
    void set_display(uint32_t grid, uint64_t plate);
    void flush_display();
    void end_frame();

    Game game_ = Game::Packmon;
    bool family45_ = false;
    bool family43_ = false;
    uint16_t pcmask_ = 0x7ff;   // pcwidth 11 (10 on the HMCS43)
    uint16_t prgmask_ = 0xfff;  // prgwidth 12 (11)
    int stack_levels_ = 4;      // (3)
    bool options_loaded_ = false;
    std::vector<uint16_t> prg_ = std::vector<uint16_t>(0x1000, 0);

    // CPU state
    uint16_t stack_[4] = {};
    uint16_t op_ = 0, prev_op_ = 0;
    uint8_t i_ = 0;
    uint8_t eint_line_ = 0;
    uint8_t prescaler_ = 0;
    uint8_t block_int_ = 0;
    uint16_t pc_ = 0, pc_upper_ = 0;
    uint8_t a_ = 0, b_ = 0, x_ = 0, spx_ = 0, y_ = 0, spy_ = 0;
    uint8_t s_ = 1, c_ = 0;
    uint8_t tc_ = 0, cf_ = 0, ie_ = 0, iri_ = 0, irt_ = 0;
    uint8_t if_[2] = {}, tf_ = 0, int_[2] = {};
    uint8_t r_[8] = {};
    uint16_t d_ = 0;
    uint8_t ram_[160] = {};
    int cycles_ = 0;  // this execute()'s count

    // board state
    uint32_t buttons_ = 0;
    uint8_t board_int_[2] = {};
    uint16_t inp_mux_ = 0;
    uint32_t grid_ = 0;
    uint64_t plate_ = 0;
    double volume_ = 0;        // cdkong: speaker volume (RC decay)
    int32_t decay_cycles_ = 0;
    bool speaker_level_ = false;

    // estargte: the COP411 sound CPU (195 kHz / 4 = 48750 cycles a second)
    Cop411 cop_;
    int64_t cop_hz_ = 48750;    // eturtles: 215 kHz / 4 = 53750
    uint64_t rplate_ = 0;       // eturtles: R0x-R6x (plate 7 also gets D10)
    // pairmtch: the audio CPU (another HMCS40, game PairmtchAudio) and the two
    // 4-bit sound latches between them (0: main -> audio, 1: audio -> main)
    std::unique_ptr<HMCS40> sub_;
    HMCS40* parent_ = nullptr;
    uint8_t latch_[2] = {};
    uint8_t sub_held_ = 0;
    int32_t sub_debt_ = 0;
    uint8_t skill_ = 0;  // pairmtch: its SKILL switch, 0-2 = level 1-3; epacman2: SKILL 1/2 (slide switches: kept over resets)

    bool has_cop() const { return game_ == Game::Estargte || game_ == Game::Eturtles; }

    // The speed knob of eturtles/estargte: maincpu clock = knob * 1500 + 325000
    // Hz (MAME's game_speed), i.e. 81250-118750 instruction cycles a second,
    // 100000 at the default 50. Applied by stretching each instruction's time.
    int speed_knob_ = 50;
    int64_t speed_hz() const { return has_cop() ? (speed_knob_ * 1500 + 325000) / 4 : 100000; }
    int64_t execute_scaled() override {
        const int64_t c = execute();
        return has_cop() ? c * kTickHz * (int64_t)clock_hz() / speed_hz() : c * kTickHz;
    }
    uint8_t cop_irq_ = 0;
    int64_t cop_debt_ = 0;
    void update_estargte_int();

    // display integration
    uint64_t now_ = 0;          // total cycles
    uint64_t last_flush_ = 0;
    uint32_t frame_cycles_ = 0;
    uint32_t acc_[kGrids][kCols] = {};
    float bri_[kGrids][kCols] = {};
    uint8_t shown_[kGrids * 5] = {};
};
