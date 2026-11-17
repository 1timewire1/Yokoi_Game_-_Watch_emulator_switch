#include "HMCS40.h"

#include <cstring>

// CPU: MAME src/devices/cpu/hmcs40/hmcs40.cpp + hmcs40op.cpp (hap,
// BSD-3-Clause), HMCS44/HMCS45 PMOS parts only. Board wiring: MAME
// src/mame/handheld/hh_hmcs40.cpp (hap, Sean Riddle, BSD-3-Clause).

namespace {


inline uint8_t rev4(uint16_t v) {  // bitswap<4>(v, 0,1,2,3)
    return (uint8_t)(((v & 1) << 3) | ((v & 2) << 1) | ((v & 4) >> 1) | ((v & 8) >> 3));
}

inline uint8_t rev8(uint16_t v) {  // bitswap<8>(v, 0,1,...,7)
    uint8_t r = 0;
    for (int i = 0; i < 8; i++) r |= ((v >> i) & 1) << (7 - i);
    return r;
}

// Button numbers ("input.<name>=direct;BTN;<n>;1;<keys>") per game:
//   packmon:  0 start, 1 left, 2 right, 3 up, 4 down
//   cdkong:   0 jump, 1 up, 2 right, 3 down, 4 left
//   egalaxn2: 0 right, 1 left, 2 fire
//   einvader2: 0 right, 1 left, 2 fire
//   epacman2: 0 up, 1 right, 2 down, 3 left, 4 skill control (start), 5 demo/light test, 6 skill switch 1 <-> 2
//   tmtron:   0 down, 1 right, 2 up, 3 left, 4 fire
//   gdigdug:  0 select, 1 down, 2 right, 3 up, 4 left, 5 fire
//   gscobra:  0 bomb, 1 missile, 2 up, 3 down, 4 left, 5 right
//   bbtime:   0 right, 1 left, 2 down, 3 up, 4 attack/select, 5 pepper/start
//   cmspacmn: 0-3 P1 up/right/down/left, 4-7 P2 up/right/down/left
//   estargte: 0 inviso, 1 smart bomb, 2 fire, 3 change direction, 4 up, 5 down, 6 thrust
//   bfriskyt: 0 attack, 1 down, 2 right, 3 up, 4 left, 5 start
//   bzaxxon:  0 right, 1 up, 2 left, 3 down, 4 fire, 5 select
//   zackman:  0 up, 1 right, 2 down, 3 left, 4 fire
//   bpengo:   0 up, 1 down, 2 left, 3 right, 4 attack
//   alnattck: 0 left, 1 right, 2 up, 3 down, 4 move, 5 fire
//   pbqbert:  0 up-left, 1 up-right, 2 down-right, 3 down-left
//   cpacman:  0-3 P1 up/right/down/left, 4-7 P2 up/right/down/left
//   gckong:   0 right, 1 down, 2 left, 3 up, 4 jump
//   bdoramon: 0 left, 1 attack, 2 up, 3 right, 4 down
//   kingman:  0 up, 1 right, 2 down, 3 left, 4 jump
//   ghalien:  0 up, 1 down, 2 right, 3 left, 4 dig, 5 bury
//   machiman: 0 attack, 1 left, 2 right
//   bambball: 0-2 dribble low/medium/high, 3 shoot, 4 right, 5 left, 6 start, 7 display
//   vinvader: 0 left, 1 right, 2 fire
//   eturtles: 0 up, 1 down, 2 left, 3 right, 4 fire, 5/6 speed knob down/up
//   pairmtch: 0 right, 1 down, 2 up, 3 left, 4 button, 5 skill switch (1 -> 2 -> 3 -> 1)
//   bmboxing: 0-2 P1 punch high/medium/low, 3-5 P1 position normal/back/ducking, 6 start
inline bool btn(uint32_t buttons, int n) { return (buttons >> n) & 1; }

}  // namespace

void HMCS40::load_options() {
    const std::string g = options().has("hmcs40.game") ? options().str("hmcs40.game") : std::string("packmon");
    if (g == "cdkong") game_ = Game::Cdkong;
    else if (g == "egalaxn2") game_ = Game::Egalaxn2;
    else if (g == "epacman2" || g == "epacman2r") game_ = Game::Epacman2;
    else if (g == "einvader2") game_ = Game::Einvader2;
    else if (g == "tmtron") game_ = Game::Tmtron;
    else if (g == "gdigdug") game_ = Game::Gdigdug;
    else if (g == "gscobra") game_ = Game::Gscobra;
    else if (g == "bbtime") game_ = Game::Bbtime;
    else if (g == "cmspacmn") game_ = Game::Cmspacmn;
    else if (g == "estargte") game_ = Game::Estargte;
    else if (g == "bfriskyt") game_ = Game::Bfriskyt;
    else if (g == "bzaxxon") game_ = Game::Bzaxxon;
    else if (g == "zackman") game_ = Game::Zackman;
    else if (g == "bpengo") game_ = Game::Bpengo;
    else if (g == "alnattck") game_ = Game::Alnattck;
    else if (g == "pbqbert") game_ = Game::Pbqbert;
    else if (g == "cpacman" || g == "cpacmanr1") game_ = Game::Cpacman;
    else if (g == "gckong") game_ = Game::Gckong;
    else if (g == "bdoramon") game_ = Game::Bdoramon;
    else if (g == "kingman") game_ = Game::Kingman;
    else if (g == "ghalien") game_ = Game::Ghalien;
    else if (g == "machiman") game_ = Game::Machiman;
    else if (g == "bambball") game_ = Game::Bambball;
    else if (g == "vinvader") game_ = Game::Vinvader;
    else if (g == "eturtles") game_ = Game::Eturtles;
    else if (g == "bmboxing") game_ = Game::Bmboxing;
    else if (g == "pairmtch") game_ = Game::Pairmtch;
    else game_ = Game::Packmon;
    const long family = options().num("hmcs40.family", 44);
    family45_ = family == 45;
    family43_ = family == 43;
    pcmask_ = family43_ ? 0x3ff : 0x7ff;
    prgmask_ = family43_ ? 0x7ff : 0xfff;
    stack_levels_ = family43_ ? 3 : 4;

    // MAME: ROM_LOAD(0, 0x1000) + ROM_CONTINUE(0x1e80, 0x100) into a 0x2000
    // byte region (HMCS43: 0x800 + 0x80 at 0xf00), read as little-endian
    // 16-bit words.
    load_prg(rom());
    if (game_ == Game::Pairmtch) {
        // pairmtch_state: a second HD38820 (the audio CPU, its ROM is the
        // "sound_rom" blob), same 400 kHz, run in lockstep (perfect quantum)
        sub_ = std::make_unique<HMCS40>();
        sub_->game_ = Game::PairmtchAudio;
        sub_->parent_ = this;
        sub_->family45_ = true;
        sub_->options_loaded_ = true;
        if (const std::vector<uint8_t>* snd = rom_info().blob("sound_rom")) sub_->load_prg(*snd);
    }
    if (game_ == Game::Eturtles) {
        // eturtles_state: L0-L3 = maincpu R0x latch, G0 = maincpu D0 (ack);
        // D0 = maincpu INT0 (active low); SK = speaker (inverted). 215 kHz / 4.
        if (const std::vector<uint8_t>* snd = rom_info().blob("sound_rom")) cop_.load(*snd);
        cop_hz_ = 53750;
        cop_.read_l = [this] { return (uint8_t)(r_[0] & 0xf); };
        cop_.read_g = [this] { return (uint8_t)(d_ & 1); };
        cop_.write_d = [this](uint8_t v) {
            cop_irq_ = ~v & 1;
            update_estargte_int();
        };
        cop_.write_sk = [this](int v) {
            speaker_level_ = !v;
            set_speaker(speaker_level_);
        };
    }
    if (game_ == Game::Estargte) {
        if (const std::vector<uint8_t>* snd = rom_info().blob("sound_rom")) cop_.load(*snd);
        // estargte_state: L0-L3 = maincpu R0x latch, L7 = maincpu D0 (ack);
        // D0 = maincpu INT0 (active low); SK = speaker (inverted); G0-G2 = volume
        cop_.read_l = [this] { return (uint8_t)((r_[0] & 0xf) | ((d_ & 1) << 7)); };
        cop_.write_d = [this](uint8_t v) {
            cop_irq_ = ~v & 1;
            update_estargte_int();
        };
        cop_.write_sk = [this](int v) {
            speaker_level_ = !v;
            set_speaker(speaker_level_);
        };
    }
    options_loaded_ = true;
}

// data_160x4: 0x00-0x7f, 0x80-0x8f (mirrored over 0x80-0xbf), 0xc0-0xcf
// (mirrored over 0xc0-0xff).
// data_80x4 (HMCS43): 0x00-0x3f, 0x40-0x4f (mirrored over 0x40-0x7f).
int HMCS40::ram_index(uint32_t addr) const {
    if (family43_) {
        addr &= 0x7f;
        return addr < 0x40 ? (int)addr : 0x40 + (int)(addr & 0xf);
    }
    addr &= 0xff;
    if (addr < 0x80) return (int)addr;
    if (addr < 0xc0) return 0x80 + (int)(addr & 0xf);
    return 0x90 + (int)(addr & 0xf);
}

void HMCS40::load_prg(const std::vector<uint8_t>& r) {
    std::vector<uint8_t> region(0x2000, 0);
    const size_t main = family43_ ? 0x800 : 0x1000, extra = family43_ ? 0x80 : 0x100;
    const size_t extra_at = family43_ ? 0xf00 : 0x1e80;
    for (size_t i = 0; i < r.size() && i < main; i++) region[i] = r[i];
    for (size_t i = main; i < r.size() && i < main + extra; i++) region[extra_at + (i - main)] = r[i];
    for (int w = 0; w < 0x1000; w++) prg_[w] = (uint16_t)(region[w * 2] | (region[w * 2 + 1] << 8));
}

bool HMCS40::poke_ram(uint32_t addr, uint8_t value) {
    ram_[ram_index(addr)] = value & 0xf;
    return true;
}

int HMCS40::peek_ram(uint32_t addr) const { return ram_[ram_index(addr)]; }

void HMCS40::reset() {
    if (!options_loaded_) load_options();
    // device_start
    memset(stack_, 0, sizeof(stack_));
    op_ = prev_op_ = 0;
    i_ = eint_line_ = 0;
    prescaler_ = 0;
    block_int_ = 0;
    pc_ = pc_upper_ = 0;
    a_ = b_ = x_ = spx_ = y_ = spy_ = 0;
    s_ = 1;
    c_ = 0;
    tc_ = cf_ = ie_ = iri_ = irt_ = 0;
    if_[0] = if_[1] = tf_ = 0;
    int_[0] = int_[1] = 0;
    memset(r_, 0, sizeof(r_));
    d_ = 0;
    memset(ram_, 0, sizeof(ram_));
    // board
    board_int_[0] = board_int_[1] = 0;
    inp_mux_ = 0;
    grid_ = 0;
    plate_ = 0;
    volume_ = 0;
    decay_cycles_ = 0;
    speaker_level_ = false;
    set_speaker(false);
    now_ = last_flush_ = 0;
    frame_cycles_ = 0;
    memset(acc_, 0, sizeof(acc_));
    memset(bri_, 0, sizeof(bri_));
    memset(shown_, 0, sizeof(shown_));
    cycles_ = 0;
    cop_irq_ = 0;
    cop_debt_ = 0;
    rplate_ = 0;
    if (has_cop()) cop_.reset();
    latch_[0] = latch_[1] = 0;
    sub_held_ = 0;
    sub_debt_ = 0;
    if (sub_) sub_->reset();
    cpu_reset();
    // machine_reset: refresh_interrupts(), then the inputs as they stand
    for (int i = 0; i < 2; i++) set_input_line(i, board_int_[i]);
    update_board_interrupts();
}

// device_reset
void HMCS40::cpu_reset() {
    pc_ = pcmask_;
    prev_op_ = op_ = 0;
    cf_ = ie_ = 0;
    iri_ = irt_ = 0;
    if_[0] = if_[1] = tf_ = 1;
    reset_io();
}

// --- CPU helpers ---------------------------------------------------------

void HMCS40::push_stack() {
    for (int i = stack_levels_ - 1; i >= 1; i--) stack_[i] = stack_[i - 1];
    stack_[0] = pc_;
}

void HMCS40::pop_stack() {
    pc_ = stack_[0] & pcmask_;
    for (int i = 0; i < stack_levels_ - 1; i++) stack_[i] = stack_[i + 1];
}

void HMCS40::increment_pc() {
    // the low 6 bits are a polynomial counter
    const uint8_t mask = 0x3f;
    const uint8_t low = pc_ & mask;
    int fb = (low << 1 & 0x20) == (low & 0x20);
    if (low == (mask >> 1)) fb = 1;
    else if (low == mask) fb = 0;
    pc_ = (uint16_t)((pc_ & ~mask) | ((pc_ << 1 | fb) & mask));
}

void HMCS40::clock_timer() {
    tc_ = (tc_ + 1) & 0xf;
    if (tc_ == 0 && !tf_) {
        irt_ = 1;
        tf_ = 1;
    }
}

void HMCS40::clock_prescaler() {
    prescaler_ = (prescaler_ + 1) & 0x3f;
    if (prescaler_ == 0 && !cf_) clock_timer();
}

void HMCS40::cycle() {
    cycles_++;
    now_++;
    clock_prescaler();
    // cdkong: speaker volume decays every 1 ms (speaker_decay_sim)
    if (game_ == Game::Cdkong && ++decay_cycles_ >= 100) {
        decay_cycles_ = 0;
        if (r_[1] & 8) volume_ = 1.0;
        volume_ /= 1.02;
        set_speaker(speaker_level_ && volume_ > 0.2);
    }
    if (++frame_cycles_ >= (uint32_t)kFrameCycles) end_frame();
}

void HMCS40::take_interrupt() {
    push_stack();
    ie_ = 0;
    pc_ = 0x3f | (iri_ ? 0x40 : 0);
    if (iri_) iri_ = 0;
    else irt_ = 0;
    cycle();
}

void HMCS40::set_input_line(int line, int state) {
    state = state ? 1 : 0;
    if (line != 0 && line != 1) return;
    if (state && !int_[line]) {
        if (!if_[line]) {
            eint_line_ = (uint8_t)line;
            iri_ = 1;
            if_[line] = 1;
        }
        if (cf_ && line == 1) clock_timer();
    }
    int_[line] = (uint8_t)state;
}

// PMOS (polarity 0): inputs are OR-ed with the output latches.
uint8_t HMCS40::read_r(uint8_t index) {
    index &= 7;
    return (board_read_r(index) | r_[index]) & 0xf;
}

void HMCS40::write_r(uint8_t index, uint8_t data) {
    index &= 7;
    if (family43_ ? (index == 0 || index >= 4) : family45_ ? index == 7 : index >= 6) return;  // no such output
    r_[index] = data & 0xf;
    board_write_r(index, data & 0xf);
}

int HMCS40::read_d(uint8_t index) {
    index &= 0xf;
    return ((board_read_d() | d_) >> index) & 1;
}

void HMCS40::write_d(uint8_t index, int state) {
    index &= 0xf;
    const uint16_t mask = (uint16_t)(1 << index);
    d_ = (uint16_t)((d_ & ~mask) | (state ? mask : 0));
    board_write_d();
}

void HMCS40::reset_io() {
    d_ = 0;
    board_write_d();
    for (int i = 0; i < 8; i++) {  // hmcs40_cpu_device::write_r, unrestricted
        r_[i] = 0;
        board_write_r((uint8_t)i, 0);
    }
}

void HMCS40::op_xsp() {
    if (op_ & 1) {
        const uint8_t t = x_;
        x_ = spx_;
        spx_ = t;
    }
    if (op_ & 2) {
        const uint8_t t = y_;
        y_ = spy_;
        spy_ = t;
    }
}

// --- execute -------------------------------------------------------------

int HMCS40::execute() {
    cycles_ = 0;
    // LPU is delayed by one instruction
    if ((prev_op_ & 0x7e0) == 0x340) pc_ = (uint16_t)(((pc_upper_ << 6) | (pc_ & 0x3f)) & pcmask_);
    prev_op_ = op_;
    if (ie_ && (iri_ || irt_) && !block_int_) take_interrupt();
    block_int_ = 0;
    op_ = rom_word(pc_) & 0x3ff;
    i_ = rev4(op_);  // reversed bit order for the 4-bit immediate (except XAMR)
    increment_pc();
    cycle();
    run_op();
    if (sub_) {  // pairmtch's audio CPU, held while D7 asserts its reset line
        const int c = cycles_;
        sub_debt_ += c;
        if (sub_held_) sub_debt_ = 0;
        while (sub_debt_ > 0) sub_debt_ -= sub_->execute();
        cycles_ = c;
    }
    if (has_cop()) {
        cop_debt_ += (int64_t)cycles_ * cop_hz_;
        const int64_t main_hz = speed_hz();
        while (cop_debt_ >= main_hz) {
            cop_.step();
            cop_debt_ -= main_hz;
        }
    }
    return cycles_;
}

void HMCS40::run_op() {
    auto iy = [&] { y_ = (y_ + 1) & 0xf; s_ = (y_ != 0); };
    auto dy = [&] { y_ = (y_ - 1) & 0xf; s_ = (y_ != 0xf); };
    auto rtn = [&] { pop_stack(); };

    switch (op_ & 0x3f0) {
        case 0x1c0: case 0x1d0: case 0x1e0: case 0x1f0:  // BR
            if (s_) pc_ = (uint16_t)((pc_ & ~0x3f) | (op_ & 0x3f));
            else s_ = 1;
            return;
        case 0x3c0: case 0x3d0: case 0x3e0: case 0x3f0:  // CAL
            if (s_) {
                block_int_ = 1;
                push_stack();
                pc_ = op_ & 0x3f;  // short calls default to page 0
            } else {
                s_ = 1;
            }
            return;
        case 0x340: case 0x350:  // LPU
            if (s_) {
                block_int_ = 1;
                pc_upper_ = op_ & 0x1f;
            } else {
                op_ |= 0x400;  // unhandled LPU
            }
            return;
        case 0x010: ram_w(i_); iy(); return;                             // LMIIY
        case 0x070: a_ = i_; return;                                     // LAI
        case 0x080: a_ += i_; s_ = (a_ >> 4) & 1; a_ &= 0xf; return;     // AI
        case 0x0f0: {                                                    // XAMR
            // HMCS43: MR0-MR3 on file 0-3, MR4-MR15 on file 4; others: last file
            uint8_t mr = op_ & 0xf;
            mr |= family43_ ? ((mr < 4) ? (uint8_t)(mr << 4) : 0x40) : 0xf0;
            const int addr = ram_index(mr);
            const uint8_t old = a_;
            a_ = ram_[addr];
            ram_[addr] = old & 0xf;
            return;
        }
        case 0x140: x_ = i_; return;                                     // LXI
        case 0x150: y_ = i_; return;                                     // LYI
        case 0x160: b_ = i_; return;                                     // LBI
        case 0x170: tc_ = i_; prescaler_ = 0; return;                    // LTI
        case 0x210: s_ = (i_ != ram_r()); return;                        // MNEI
        case 0x270: s_ = (a_ <= i_); return;                             // ALEI
        case 0x280: s_ = (y_ != i_); return;                             // YNEI
        default: break;
    }

    switch (op_ & 0x3fc) {
        case 0x0c0: case 0x0c4: a_ = read_r(op_ & 7); return;            // LAR
        case 0x0e0: case 0x0e4: b_ = read_r(op_ & 7); return;            // LBR
        case 0x2c0: case 0x2c4: write_r(op_ & 7, a_); return;            // LRA
        case 0x2e0: case 0x2e4: write_r(op_ & 7, b_); return;            // LRB
        case 0x360: case 0x364:                                          // TBR
            pc_ = (uint16_t)((a_ | b_ << 4 | c_ << 8 | (op_ & 7) << 9 | (pc_ & ~0x3f)) & pcmask_);
            return;
        case 0x368: case 0x36c: {                                        // P
            cycle();
            const uint16_t address = (uint16_t)(a_ | b_ << 4 | c_ << 8 | (op_ & 7) << 9 | (pc_ & ~0x3f));
            uint16_t data = rom_word(address & prgmask_);
            if (data & 0x100) {
                a_ = rev4(data);
                b_ = (data >> 4) & 0xf;
            }
            if (data & 0x200) {
                const uint8_t sw = rev8(data);
                write_r(2, sw & 0xf);
                write_r(3, (sw >> 4) & 0xf);
            }
            return;
        }
        case 0x000: op_xsp(); return;                                    // XSP
        case 0x004: ram_w(ram_r() | (1 << (op_ & 3))); return;           // SEM
        case 0x008: a_ = ram_r(); op_xsp(); return;                      // LAM
        case 0x020: b_ = ram_r(); op_xsp(); return;                      // LBM
        case 0x0d0: write_d(op_ & 3, 1); return;                         // SEDD
        case 0x200: s_ = (ram_r() >> (op_ & 3)) & 1; return;             // TM
        case 0x204: ram_w(ram_r() & ~(1 << (op_ & 3))); return;          // REM
        case 0x208: { const uint8_t o = a_; a_ = ram_r(); ram_w(o); op_xsp(); return; }  // XMA
        case 0x220: { const uint8_t o = b_; b_ = ram_r(); ram_w(o); op_xsp(); return; }  // XMB
        case 0x2d0: write_d(op_ & 3, 0); return;                         // REDD
        default: break;
    }

    switch (op_) {
        case 0x024: s_ = (b_ <= ram_r()); return;                                               // BLEM
        case 0x030: a_ += ram_r() + c_; c_ = (a_ >> 4) & 1; s_ = c_; a_ &= 0xf; return;         // AMC
        case 0x034: a_ += ram_r(); s_ = (a_ >> 4) & 1; a_ &= 0xf; return;                       // AM
        case 0x03c: tc_ = a_; prescaler_ = 0; return;                                           // LTA
        case 0x040: x_ = a_; return;                                                            // LXA
        case 0x045: if (!c_ || a_ > 9) { a_ = (a_ + 10) & 0xf; c_ = 0; } return;                // DAS
        case 0x046: if (c_ || a_ > 9) { a_ = (a_ + 6) & 0xf; c_ = 1; } return;                  // DAA
        case 0x04c: c_ = 0; return;                                                             // REC
        case 0x04f: c_ = 1; return;                                                             // SEC
        case 0x050: y_ = a_; return;                                                            // LYA
        case 0x054: iy(); return;                                                               // IY
        case 0x058: y_ += a_; s_ = (y_ >> 4) & 1; y_ &= 0xf; return;                            // AYY
        case 0x060: b_ = a_; return;                                                            // LBA
        case 0x064: b_ = (b_ + 1) & 0xf; s_ = (b_ != 0); return;                                // IB
        case 0x090: write_d(y_, 1); return;                                                     // SED
        case 0x094: s_ = (uint8_t)read_d(y_); return;                                           // TD
        case 0x0a0: if_[1] = 1; return;                                                         // SEIF1
        case 0x0a1: cf_ = 1; return;                                                            // SECF
        case 0x0a2: if_[0] = 1; return;                                                         // SEIF0
        case 0x0a4: ie_ = 1; return;                                                            // SEIE
        case 0x0a5: tf_ = 1; return;                                                            // SETF
        case 0x110: case 0x111: ram_w(a_); iy(); op_xsp(); return;                              // LMAIY
        case 0x114: case 0x115: ram_w(a_); dy(); op_xsp(); return;                              // LMADY
        case 0x118: a_ = y_; return;                                                            // LAY
        case 0x120: a_ |= b_; return;                                                           // OR
        case 0x124: s_ = (a_ != ram_r()); return;                                               // ANEM
        case 0x1a0: s_ = if_[1]; return;                                                        // TIF1
        case 0x1a1: s_ = int_[1]; return;                                                       // TI1
        case 0x1a2: s_ = if_[0]; return;                                                        // TIF0
        case 0x1a3: s_ = int_[0]; return;                                                       // TI0
        case 0x1a5: s_ = tf_; return;                                                           // TTF
        case 0x224: { const uint8_t c = a_ & 1; a_ = (uint8_t)(a_ >> 1 | c_ << 3); c_ = c; return; }  // ROTR
        case 0x225: a_ = (uint8_t)(a_ << 1 | c_); c_ = (a_ >> 4) & 1; a_ &= 0xf; return;        // ROTL
        case 0x230: a_ = (uint8_t)(ram_r() - a_ - (c_ ^ 1)); c_ = ((~a_) >> 4) & 1; s_ = c_; a_ &= 0xf; return;  // SMC
        case 0x234: s_ = (a_ <= ram_r()); return;                                               // ALEM
        case 0x23c: a_ = tc_; return;                                                           // LAT
        case 0x240: a_ = spx_; return;                                                          // LASPX
        case 0x244: a_ = (uint8_t)((0 - a_) & 0xf); return;                                     // NEGA
        case 0x24f: s_ = c_; return;                                                            // TC
        case 0x250: a_ = spy_; return;                                                          // LASPY
        case 0x254: dy(); return;                                                               // DY
        case 0x258: y_ = (uint8_t)(y_ - a_); s_ = ((~y_) >> 4) & 1; y_ &= 0xf; return;          // SYY
        case 0x260: a_ = b_; return;                                                            // LAB
        case 0x267: b_ = (b_ - 1) & 0xf; s_ = (b_ != 0xf); return;                              // DB
        case 0x290: write_d(y_, 0); return;                                                     // RED
        case 0x2a0: if_[1] = 0; return;                                                         // REIF1
        case 0x2a1: cf_ = 0; return;                                                            // RECF
        case 0x2a2: if_[0] = 0; return;                                                         // REIF0
        case 0x2a4: ie_ = 0; return;                                                            // REIE
        case 0x2a5: tf_ = 0; return;                                                            // RETF
        case 0x320: b_ ^= 0xf; return;                                                          // COMB
        case 0x324: s_ = (b_ != ram_r()); return;                                               // BNEM
        case 0x3a4: ie_ = 1; rtn(); return;                                                     // RTNI
        case 0x3a7: rtn(); return;                                                              // RTN
        default: return;                                                                        // illegal
    }
}

// --- board ---------------------------------------------------------------

uint16_t HMCS40::read_inputs(int columns) const {
    uint16_t ret = 0;
    for (int i = 0; i < columns; i++) {
        if (!((inp_mux_ >> i) & 1)) continue;
        uint16_t row = 0;
        switch (game_) {
            case Game::Packmon: row = btn(buttons_, i) ? 0x20 : 0; break;
            case Game::Tmtron:
            case Game::Gdigdug:
            case Game::Gscobra:
            case Game::Bbtime:
            case Game::Bfriskyt:
            case Game::Bzaxxon:
            case Game::Zackman:
            case Game::Bpengo:
            case Game::Gckong:
            case Game::Kingman: row = btn(buttons_, i) ? 1 : 0; break;
            case Game::Ghalien: row = (i < 6 && btn(buttons_, i)) ? 0x8000 : 0; break;  // row 6: difficulty 1
            case Game::Pairmtch:  // row 0: player 2
                if (i == 1) row = (uint16_t)(buttons_ & 0xf);
                break;
            case Game::Bmboxing:  // rows 2/3: player 2 (1-player mode)
                if (i == 0) row = (uint16_t)(buttons_ & 7);
                else if (i == 1) row = (uint16_t)((buttons_ >> 3) & 7);
                break;
            case Game::Bambball:
                if (i == 0) row = (uint16_t)(buttons_ & 0xf);
                else if (i == 1) row = (uint16_t)((buttons_ >> 4) & 3);
                else if (i == 2) row = (uint16_t)(((buttons_ >> 6) & 3) << 2);
                else if (i == 3) row = 0x09;  // difficulty 1, 1 player
                break;
            case Game::Alnattck: row = (i >= 1 && btn(buttons_, i - 1)) ? 0x20 : 0; break;  // row 0: difficulty 1
            case Game::Cmspacmn:
                if (i == 1) row = (uint16_t)((buttons_ >> 4) & 0xf);   // P2
                else if (i == 2) row = (uint16_t)(buttons_ & 0xf);     // P1 (row 0: difficulty 1)
                break;
            case Game::Cpacman:
                if (i == 1) row = (uint16_t)(buttons_ & 0xf);          // P1
                else if (i == 2) row = (uint16_t)((buttons_ >> 4) & 0xf);  // P2 (row 0: difficulty 1)
                break;
            case Game::Estargte:
                if (i == 0) row = (btn(buttons_, 0) ? 1 : 0) | (btn(buttons_, 1) ? 2 : 0);
                else if (i == 1) row = (btn(buttons_, 2) ? 1 : 0) | (btn(buttons_, 3) ? 2 : 0);
                else if (i == 2) row = (btn(buttons_, 4) ? 1 : 0) | (btn(buttons_, 5) ? 2 : 0);
                else if (i == 3) row = btn(buttons_, 6) ? 1 : 0;
                break;  // rows 4/5: 1 player, difficulty 1
            case Game::Eturtles:
                if (i < 4) row = btn(buttons_, i) ? 1 : 0;
                else if (i == 4) row = 0x01 | (btn(buttons_, 4) ? 2 : 0);  // difficulty 1
                break;  // row 5: 1 player
            case Game::Einvader2:  // D3: right + "1 player"; D5: left (demo off); D6: fire
                if (i == 0) row = (uint16_t)((btn(buttons_, 0) ? 1 : 0) | 0x04);
                else if (i == 1) row = btn(buttons_, 1) ? 1 : 0;
                else if (i == 2) row = btn(buttons_, 2) ? 1 : 0;
                break;
            case Game::Epacman2:  // Galaxian 2's board, MAME's own input rows
                if (i == 0) row = (uint16_t)(buttons_ & 0xf);  // up, right, down, left
                else if (i == 2) row = (btn(buttons_, 4) ? 0x01 : 0) | (btn(buttons_, 5) ? 0x02 : 0);
                else if (i == 3) row = (uint16_t)(0x04 | (skill_ ? 0x02 : 0));  // 1 player; difficulty 1 = 0
                break;
            case Game::Egalaxn2:
                if (i == 0) row = (btn(buttons_, 0) ? 0x02 : 0) | (btn(buttons_, 1) ? 0x08 : 0);
                else if (i == 2) row = btn(buttons_, 2) ? 0x01 : 0;
                else if (i == 3) row = 0x02;  // difficulty 1, 1 player
                break;
            default: break;
        }
        ret |= row;
    }
    return ret;
}

uint8_t HMCS40::board_read_r(uint8_t index) const {
    if ((game_ == Game::Egalaxn2 || game_ == Game::Epacman2) && index == 0) return (uint8_t)read_inputs(4);
    if ((game_ == Game::Cmspacmn || game_ == Game::Cpacman) && index == 0) return (uint8_t)read_inputs(3);
    if ((game_ == Game::Bambball || game_ == Game::Bmboxing) && index == 0) return (uint8_t)read_inputs(4);
    if (game_ == Game::Pairmtch && index == 4) return (uint8_t)read_inputs(2);
    if (game_ == Game::Pairmtch && index == 5) return latch_[1];           // from the audio CPU
    if (game_ == Game::PairmtchAudio && index == 2) return parent_->latch_[0];  // from the main CPU
    if (game_ == Game::Vinvader && index == 0) return (uint8_t)((btn(buttons_, 0) ? 1 : 0) | (btn(buttons_, 1) ? 2 : 0));
    return 0;
}

uint16_t HMCS40::board_read_d() const {
    switch (game_) {
        case Game::Packmon: return read_inputs(5) & 0x20;
        case Game::Cdkong:
            return (uint16_t)((btn(buttons_, 1) ? 0x0001 : 0) | (btn(buttons_, 2) ? 0x0002 : 0) |
                              (btn(buttons_, 3) ? 0x0004 : 0) | (btn(buttons_, 4) ? 0x8000 : 0));
        case Game::Alnattck: return read_inputs(7) & 0x20;
        case Game::Ghalien: return read_inputs(7) & 0x8000;
        case Game::Einvader2: return (uint16_t)(read_inputs(3) << 13);  // D13-D15
        case Game::Vinvader: return btn(buttons_, 2) ? 0x0008 : 0;  // fire (D1: difficulty 1)
        case Game::Bmboxing: return btn(buttons_, 6) ? 0x0004 : 0;
        case Game::Pairmtch: {  // 1 player; skill switch D12/D13: 1 = 0x2000, 2 = 0x1000, 3 = 0
            static const uint16_t kSkill[3] = {0x2000, 0x1000, 0x0000};
            return (uint16_t)(0x0800 | kSkill[skill_ % 3] | (btn(buttons_, 4) ? 0x4000 : 0));
        }  // start (D0/D1: 1 player, difficulty 1)
        case Game::Bdoramon:
            return (uint16_t)((btn(buttons_, 2) ? 0x10 : 0) | (btn(buttons_, 3) ? 0x20 : 0) | (btn(buttons_, 4) ? 0x40 : 0));
        case Game::Machiman: return (uint16_t)((btn(buttons_, 1) ? 0x4000 : 0) | (btn(buttons_, 2) ? 0x8000 : 0));
        case Game::Bzaxxon: return btn(buttons_, 5) ? 0x0008 : 0;  // Select
        case Game::Pbqbert:
            return (uint16_t)((btn(buttons_, 0) ? 0x0200 : 0) | (btn(buttons_, 1) ? 0x0400 : 0) |
                              (btn(buttons_, 2) ? 0x0800 : 0) | (btn(buttons_, 3) ? 0x1000 : 0));
        default: return 0;
    }
}

void HMCS40::set_interrupt(int line, int state) {
    line = line ? 1 : 0;
    state = state ? 1 : 0;
    if (state != board_int_[line]) {
        set_input_line(line, state);
        board_int_[line] = (uint8_t)state;
    }
}

void HMCS40::update_board_interrupts() {
    switch (game_) {
        case Game::Cdkong: set_interrupt(0, btn(buttons_, 0)); break;
        case Game::Tmtron:
            set_interrupt(0, btn(buttons_, 4));
            set_interrupt(1, read_inputs(4) != 0);
            break;
        case Game::Gdigdug:
            set_interrupt(0, btn(buttons_, 5));
            set_interrupt(1, read_inputs(5) != 0);
            break;
        case Game::Gscobra: set_interrupt(0, read_inputs(6) != 0); break;
        case Game::Bbtime:
            set_interrupt(0, read_inputs(5) != 0);
            set_interrupt(1, btn(buttons_, 5));
            break;
        case Game::Estargte:
        case Game::Eturtles: update_estargte_int(); break;
        case Game::Bfriskyt:
            set_interrupt(0, btn(buttons_, 5));
            set_interrupt(1, read_inputs(5) != 0);
            break;
        case Game::Bzaxxon:
            set_interrupt(0, btn(buttons_, 4));
            set_interrupt(1, read_inputs(4) != 0);
            break;
        case Game::Zackman:
        case Game::Bpengo:
        case Game::Kingman:
            set_interrupt(0, read_inputs(4) != 0);
            set_interrupt(1, btn(buttons_, 4));
            break;
        case Game::Gckong:
            set_interrupt(0, btn(buttons_, 4));
            set_interrupt(1, read_inputs(4) != 0);
            break;
        case Game::Bdoramon:
            set_interrupt(0, btn(buttons_, 0));
            set_interrupt(1, btn(buttons_, 1));
            break;
        case Game::Machiman: set_interrupt(0, btn(buttons_, 0)); break;
        default: break;
    }
}

void HMCS40::update_estargte_int() {
    // eturtles_state::update_int: INT0/1 on the multiplexed inputs, INT0 also from COP D0
    const uint16_t inp = read_inputs(6);
    set_interrupt(0, (inp & 1) | cop_irq_);
    set_interrupt(1, inp & 2);
}

void HMCS40::port_input(int port, uint8_t mask, int level) {
    if (port != 0 || mask >= 32) return;
    // eturtles/estargte: the game speed knob (MAME's "Game Speed" adjuster,
    // 0-100, 50 = normal), turned a notch (10) per press of its buttons
    if (game_ == Game::Pairmtch && mask == 5 && level > 0 && !btn(buttons_, mask)) skill_ = (uint8_t)((skill_ + 1) % 3);
    if (game_ == Game::Epacman2 && mask == 6 && level > 0 && !btn(buttons_, mask)) skill_ ^= 1;
    if (has_cop() && level > 0 && !btn(buttons_, mask)) {
        const int down = game_ == Game::Eturtles ? 5 : 7;
        if (mask == down && speed_knob_ > 0) speed_knob_ -= 10;
        if (mask == down + 1 && speed_knob_ < 100) speed_knob_ += 10;
    }
    if (level > 0) buttons_ |= 1u << mask;
    else buttons_ &= ~(1u << mask);
    update_board_interrupts();
}

void HMCS40::board_write_r(uint8_t index, uint8_t data) {
    uint64_t plate = plate_;
    auto nibble = [&](int shift) { plate = (plate & ~(0xfull << shift)) | ((uint64_t)data << shift); };
    switch (game_) {
        case Game::Packmon:
        case Game::Tmtron:
        case Game::Bfriskyt:
        case Game::Bzaxxon:
        case Game::Alnattck:
        case Game::Gckong:
        case Game::Bdoramon:
        case Game::Kingman:
        case Game::Ghalien:
            if (index > 3) return;
            nibble(index * 4);
            break;
        case Game::Einvader2:  // R0x-R3x: plates
            if (index > 3) return;
            nibble(index * 4);
            break;
        case Game::Machiman:  // R0x-R3x, R6x at 16
            if (index > 3 && index != 6) return;
            nibble(index == 6 ? 16 : index * 4);
            break;
        case Game::Eturtles: {  // R0x-R6x; D10 also drives plate 7 (with R13)
            if (index > 6) return;
            const int shift = index * 4;
            rplate_ = (rplate_ & ~(0xfull << shift)) | ((uint64_t)data << shift);
            plate = rplate_ | ((uint64_t)((d_ >> 10) & 1) << 7);
            break;
        }
        case Game::Pairmtch:  // R2x,R3x,R6x: plates; R5x: sound latch
            if (index == 5) { latch_[0] = rev4(data); return; }
            if (index != 2 && index != 3 && index != 6) return;
            nibble(index == 6 ? 8 : (index - 2) * 4);
            break;
        case Game::PairmtchAudio:  // R2x: latch back to the main CPU
            if (index == 2) parent_->latch_[1] = rev4(data);
            return;
        case Game::Bambball:  // R1x-R3x
        case Game::Vinvader:
        case Game::Bmboxing:
            if (index < 1 || index > 3) return;
            nibble((index - 1) * 4);
            break;
        case Game::Cdkong:
            if (index > 6) return;
            if (r_[1] & 8) volume_ = 1.0;  // R13: speaker on
            nibble(index * 4);
            break;
        case Game::Gdigdug:
        case Game::Gscobra:
        case Game::Bbtime:
        case Game::Estargte:
        case Game::Zackman:
        case Game::Bpengo:
        case Game::Pbqbert:
            if (index > 6) return;
            nibble(index * 4);
            break;
        case Game::Egalaxn2:
        case Game::Epacman2:
        case Game::Cmspacmn:
        case Game::Cpacman:
            if (index < 1 || index > 6) return;
            nibble((index - 1) * 4);
            break;
    }
    set_display(grid_, plate);
}

void HMCS40::board_write_d() {
    const uint16_t d = d_;
    uint32_t grid = grid_;
    uint64_t plate = plate_;
    switch (game_) {
        case Game::Packmon:
            speaker_level_ = (d >> 4) & 1;
            inp_mux_ = (d >> 11) & 0x1f;
            grid = (d >> 6) & 0x3ff;
            plate = (plate & ~(0xfull << 16)) | ((uint64_t)(d & 0xf) << 16);
            break;
        case Game::Cdkong:
            speaker_level_ = (d >> 3) & 1;
            grid = (d >> 4) & 0x7ff;
            break;
        case Game::Einvader2:
            speaker_level_ = d & 1;
            inp_mux_ = (uint16_t)(((d >> 3) & 1) | ((d >> 4) & 6));  // D3,D5,D6
            grid = (d >> 1) & 0xfff;                                // D1-D12
            break;
        case Game::Egalaxn2:
        case Game::Epacman2:
            speaker_level_ = d & 1;
            inp_mux_ = (d >> 1) & 0xf;
            grid = (d >> 1) & 0x7fff;
            break;
        case Game::Tmtron: {
            speaker_level_ = (d >> 4) & 1;
            const uint16_t mux = (d >> 12) & 0xf;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(1, read_inputs(4) != 0);
            }
            grid = (d >> 6) & 0x3ff;
            plate = (plate & 0x00ffff) | (((uint64_t)d << 16) & 0x2f0000);
            break;
        }
        case Game::Gdigdug: {
            speaker_level_ = (d >> 6) & 1;
            const uint16_t mux = (d >> 11) & 0x1f;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(1, read_inputs(5) != 0);
            }
            grid = (d >> 7) & 0x1ff;
            plate = (plate & ~(0xfull << 28)) | ((uint64_t)(d & 0xf) << 28);  // D0-D3: plates 28-31
            break;
        }
        case Game::Gscobra: {
            speaker_level_ = d & 1;
            const uint16_t mux = (d >> 10) & 0x3f;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(0, read_inputs(6) != 0);
            }
            grid = (d >> 7) & 0x1ff;
            plate = (plate & ~(0xfull << 28)) | ((uint64_t)((d >> 1) & 7) << 28);  // D1-D3: plates 28-30
            break;
        }
        case Game::Bfriskyt: {
            speaker_level_ = (d >> 6) & 1;
            const uint16_t mux = (d >> 11) & 0x1f;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(1, read_inputs(5) != 0);
            }
            grid = (d >> 8) & 0xff;
            plate = (plate & 0x00ffff) | (((uint64_t)d << 16) & 0x3f0000);  // D0-D5: plates 16-21
            break;
        }
        case Game::Bzaxxon: {
            speaker_level_ = (d >> 4) & 1;
            const uint16_t mux = (d >> 7) & 0xf;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(1, read_inputs(4) != 0);
            }
            grid = (d >> 5) & 0x7ff;
            plate = (plate & ~(0xfull << 16)) | ((uint64_t)(d & 7) << 16);  // D0-D2: plates 16-18
            break;
        }
        case Game::Zackman: {
            speaker_level_ = (d >> 2) & 1;
            const uint16_t mux = (d >> 11) & 0xf;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(0, read_inputs(4) != 0);
            }
            grid = (d >> 8) & 0xff;
            plate = (plate & ~(0xfull << 28)) | ((uint64_t)(d & 3) << 28);  // D0,D1: plates 28-29
            break;
        }
        case Game::Bpengo: {
            speaker_level_ = (d >> 10) & 1;
            const uint16_t mux = (d >> 12) & 0xf;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(0, read_inputs(4) != 0);
            }
            grid = d & 0xff;
            break;
        }
        case Game::Alnattck:
            speaker_level_ = (d >> 4) & 1;
            inp_mux_ = (d >> 7) & 0x7f;
            grid = (d >> 6) & 0x3ff;
            plate = (plate & ~(0xfull << 16)) | ((uint64_t)(d & 0xf) << 16);  // D0-D3: plates 16-19
            break;
        case Game::Pbqbert:
            speaker_level_ = (d >> 14) & 1;
            grid = d & 0xff;
            plate = (plate & ~(0xfull << 28)) | ((uint64_t)((d >> 8) & 1) << 28);  // D8: plate 28
            break;
        case Game::Gckong: {
            speaker_level_ = (d >> 2) & 1;
            const uint16_t mux = (d >> 5) & 0xf;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(1, read_inputs(4) != 0);
            }
            grid = (d >> 5) & 0x7ff;
            plate = (plate & ~(0xfull << 16)) | ((uint64_t)(d & 3) << 16);  // D0,D1: plates 16-17
            break;
        }
        case Game::Bdoramon:
            speaker_level_ = (d >> 7) & 1;
            grid = (d >> 8) & 0xff;
            plate = (plate & ~(0xfull << 16)) | ((uint64_t)(d & 0xf) << 16);  // D0-D3: plates 16-19
            break;
        case Game::Kingman: {
            speaker_level_ = (d >> 6) & 1;
            const uint16_t mux = (d >> 12) & 0xf;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(0, read_inputs(4) != 0);
            }
            grid = (d >> 7) & 0x1ff;
            plate = (plate & 0x00ffff) | (((uint64_t)d << 16) & 0x1f0000);  // D0-D4: plates 16-20
            break;
        }
        case Game::Ghalien:
            speaker_level_ = (d >> 14) & 1;
            inp_mux_ = d & 0x7f;
            grid = d & 0x3ff;
            plate = (plate & ~(0xfull << 16)) | ((uint64_t)((d >> 10) & 0xf) << 16);  // D10-D13: plates 16-19
            break;
        case Game::Machiman:
            speaker_level_ = (d >> 13) & 1;
            grid = d & 0x1f;
            break;
        case Game::Vinvader:
            speaker_level_ = d & 1;
            grid = (d >> 7) & 0x1ff;
            plate = (plate & ~(0xfull << 12)) | ((uint64_t)((d >> 4) & 7) << 12);  // D4-D6: plates 12-14
            break;
        case Game::Pairmtch: {
            const uint8_t held = (d >> 7) & 1;  // D7: audio CPU reset
            if (held && !sub_held_) sub_->cpu_reset();
            sub_held_ = held;
            sub_->set_input_line(0, (d >> 9) & 1);  // D9: sound start -> audio INT0
            inp_mux_ = (uint16_t)(((d >> 10) & 1) | ((d >> 14) & 2));
            grid = d & 0x3f;
            break;
        }
        case Game::PairmtchAudio:
            parent_->speaker_level_ = d & 1;
            parent_->set_speaker(d & 1);
            parent_->set_interrupt(0, (d >> 1) & 1);  // D1: sound ack -> main INT0
            return;
        case Game::Bmboxing:
            speaker_level_ = (d >> 13) & 1;
            inp_mux_ = (d >> 9) & 0xf;
            grid = (d >> 4) & 0x1ff;
            break;
        case Game::Bambball:
            speaker_level_ = (d >> 4) & 1;
            inp_mux_ = (d >> 7) & 0xf;
            grid = (d >> 7) & 0x1ff;
            plate = (plate & ~(0xfull << 12)) | ((uint64_t)(d & 0xf) << 12);  // D0-D3: plates 12-15
            break;
        case Game::Cpacman:
            speaker_level_ = d & 1;
            inp_mux_ = (d >> 13) & 7;
            grid = (d >> 5) & 0x7ff;
            plate = (plate & ~(0xfull << 24)) | ((uint64_t)((d >> 1) & 3) << 24);  // D1,D2: plates 24-25
            break;
        case Game::Cmspacmn:
            speaker_level_ = (d >> 2) & 1;
            inp_mux_ = (d >> 13) & 7;
            grid = (d >> 5) & 0x7ff;
            plate = (plate & ~(0xfull << 24)) | ((uint64_t)(d & 3) << 24);  // D0,D1: plates 24-25
            break;
        case Game::Eturtles: {
            const uint16_t mux = (d >> 1) & 0x3f;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                update_estargte_int();
            }
            grid = (d >> 1) & 0x7fff;
            plate = rplate_ | ((uint64_t)((d >> 10) & 1) << 7);
            break;
        }
        case Game::Estargte: {
            const uint16_t mux = (d >> 1) & 0x3f;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                update_estargte_int();
            }
            grid = (d >> 2) & 0x3fff;  // (data >> 1 & 0x7fff) >> 1
            break;
        }
        case Game::Bbtime: {
            speaker_level_ = (d >> 3) & 1;
            const uint16_t mux = (d >> 10) & 0x1f;
            if (mux != inp_mux_) {
                inp_mux_ = mux;
                set_interrupt(0, read_inputs(5) != 0);
            }
            grid = (d >> 4) & 0x3f;
            break;
        }
    }
    if (!has_cop())  // its speaker is the COP's
        set_speaker(game_ == Game::Cdkong ? (speaker_level_ && volume_ > 0.2) : speaker_level_);
    set_display(grid, plate);
}

// --- display (pwm_display_device) ----------------------------------------

void HMCS40::flush_display() {
    const uint32_t dt = (uint32_t)(now_ - last_flush_);
    last_flush_ = now_;
    if (!dt || !grid_) return;
    for (int g = 0; g < kGrids; g++) {
        if (!((grid_ >> g) & 1)) continue;
        acc_[g][kCols - 1] += dt;
        uint64_t p = plate_;
        for (int c = 0; p && c < kCols - 1; c++, p >>= 1) {
            if (p & 1) acc_[g][c] += dt;
        }
    }
}

void HMCS40::set_display(uint32_t grid, uint64_t plate) {
    if (grid == grid_ && plate == plate_) return;
    flush_display();
    grid_ = grid;
    plate_ = plate;
}

void HMCS40::end_frame() {
    flush_display();
    const float frame = (float)frame_cycles_;
    frame_cycles_ = 0;
    // pwm.cpp frame_tick: interpolation 0.5, levels {0.01}, cutoff 4 * 0.01
    for (int g = 0; g < kGrids; g++) {
        uint8_t* out = &shown_[g * 5];
        memset(out, 0, 5);
        for (int c = 0; c < kCols; c++) {
            float bri = bri_[g][c] * 0.5f + (acc_[g][c] / frame) * 0.5f;
            if (bri > 0.04f) bri = 0.04f;
            bri_[g][c] = bri;
            acc_[g][c] = 0;
            if (bri > 0.01f) {
                if (c == kCols - 1) out[4] |= 1;
                else out[c >> 3] |= (uint8_t)(1 << (c & 7));
            }
        }
    }
}

uint8_t HMCS40::display_byte(int index) const {
    if (index < 0 || index >= kGrids * 5) return 0;
    return shown_[index];
}

void HMCS40::serialize(BrickStateIO& io) {
    io.bytes(stack_, sizeof(stack_));
    io.pod(op_);
    io.pod(prev_op_);
    io.pod(i_);
    io.pod(eint_line_);
    io.pod(prescaler_);
    io.pod(block_int_);
    io.pod(pc_);
    io.pod(pc_upper_);
    io.pod(a_);
    io.pod(b_);
    io.pod(x_);
    io.pod(spx_);
    io.pod(y_);
    io.pod(spy_);
    io.pod(s_);
    io.pod(c_);
    io.pod(tc_);
    io.pod(cf_);
    io.pod(ie_);
    io.pod(iri_);
    io.pod(irt_);
    io.bytes(if_, sizeof(if_));
    io.pod(tf_);
    io.bytes(int_, sizeof(int_));
    io.bytes(r_, sizeof(r_));
    io.pod(d_);
    io.bytes(ram_, sizeof(ram_));
    io.pod(board_int_[0]);
    io.pod(board_int_[1]);
    io.pod(inp_mux_);
    io.pod(grid_);
    io.pod(plate_);
    io.pod(volume_);
    io.pod(decay_cycles_);
    io.pod(speaker_level_);
    io.pod(now_);
    io.pod(last_flush_);
    io.pod(frame_cycles_);
    io.bytes(acc_, sizeof(acc_));
    io.bytes(bri_, sizeof(bri_));
    io.bytes(shown_, sizeof(shown_));
    if (game_ == Game::Eturtles) io.pod(rplate_);
    if (has_cop()) io.pod(speed_knob_);
    if (sub_) {
        io.bytes(latch_, sizeof(latch_));
        io.pod(sub_held_);
        io.pod(sub_debt_);
        io.pod(skill_);
        sub_->serialize(io);
    }
    if (has_cop()) {
        cop_.serialize(io);
        io.pod(cop_irq_);
        io.pod(cop_debt_);
    }
    // Buttons aren't state: whatever is held now stays held.
    if (!io.writing()) update_board_interrupts();
}
