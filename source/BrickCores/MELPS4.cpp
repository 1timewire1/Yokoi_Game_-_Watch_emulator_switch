#include "MELPS4.h"

#include <cstring>

// CPU: MAME src/devices/cpu/melps4/melps4.cpp + melps4op.cpp + m58846.cpp
// (hap, BSD-3-Clause). Board wiring: MAME src/mame/handheld/hh_melps4.cpp
// (hap, Kevin Horton, BSD-3-Clause).

void MELPS4::load_options() {
    // only Frogger so far ("melps4.game=cfrogger")
    game_ = Game::Cfrogger;
    // MAME: ROM_LOAD(0, 0x1000) into the 16-bit little-endian program space
    const std::vector<uint8_t>& r = rom();
    for (size_t i = 0; i < prg_.size(); i++) {
        const size_t at = i * 2;
        prg_[i] = (uint16_t)((at < r.size() ? r[at] : 0) | ((at + 1 < r.size() ? r[at + 1] : 0) << 8));
    }
    options_loaded_ = true;
}

bool MELPS4::poke_ram(uint32_t addr, uint8_t value) {
    if (addr > kDataMask) return false;
    ram_[addr] = value & 0xf;
    return true;
}

int MELPS4::peek_ram(uint32_t addr) const { return addr > kDataMask ? -1 : ram_[addr]; }

void MELPS4::reset() {
    if (!options_loaded_) load_options();
    // device_start
    pc_ = prev_pc_ = 0;
    memset(stack_, 0, sizeof(stack_));
    op_ = prev_op_ = 0;
    bitmask_ = 0;
    port_d_ = 0;
    port_s_ = port_f_ = port_t_ = 0;
    sm_ = sms_ = ba_flag_ = 0;
    sp_param_ = cps_ = skip_ = 0;
    inte_ = 0;
    intp_ = 1;
    irqflag_[0] = irqflag_[1] = irqflag_[2] = 0;
    int_state_ = 0;
    prohibit_irq_ = possible_irq_ = 0;
    tmr_count_[0] = tmr_count_[1] = 0;
    tmr_reload_ = 0;
    tmr_irq_enabled_[0] = tmr_irq_enabled_[1] = 0;
    a_ = b_ = e_ = 0;
    y_ = y2_ = x_ = x2_ = z_ = z2_ = cy_ = cy2_ = 0;
    h_ = l_ = 0;
    c_ = 7;
    v_ = w_ = 0;
    memset(ram_, 0, sizeof(ram_));
    cycles_ = 0;
    // board
    inp_mux_ = 0;
    grid_ = plate_ = 0;
    set_speaker(false);
    now_ = last_flush_ = 0;
    frame_cycles_ = 0;
    memset(acc_, 0, sizeof(acc_));
    memset(bri_, 0, sizeof(bri_));
    memset(shown_, 0, sizeof(shown_));

    // device_reset (melps4 + m58846)
    sm_ = sms_ = 0;
    ba_flag_ = 0;
    skip_ = 0;
    op_ = prev_op_ = 0;
    pc_ = prev_pc_ = 0;
    op_lcps();  // CPS=0
    inte_ = 0;
    intp_ = 1;
    write_v(0);
    w_ = 0;
    irqflag_[0] = irqflag_[1] = irqflag_[2] = 0;
    prohibit_irq_ = 0;
    possible_irq_ = 0;
    write_d_pin(16, 0);
    write_gen_port(PortS, 0);
    write_gen_port(PortF, 0);
    write_gen_port(PortG, 0);
    write_gen_port(PortU, 0);
    write_t(0);
}

// --- CPU helpers -----------------------------------------------------------

void MELPS4::pop_pc() {
    pc_ = stack_[0];
    for (int i = 0; i < kStackLevels - 1; i++) stack_[i] = stack_[i + 1];
}

void MELPS4::push_pc() {
    for (int i = kStackLevels - 1; i >= 1; i--) stack_[i] = stack_[i - 1];
    stack_[0] = pc_;
}

// m58846_device::timer_update - runs once per machine cycle (6 t-states)
void MELPS4::tick_timer() {
    cycles_++;
    now_++;
    // timer 1: 7-bit fixed counter (manual specifically says 127)
    if (++tmr_count_[0] == 127) {
        tmr_count_[0] = 0;
        irqflag_[1] = 1;
        possible_irq_ = 1;
    }
    // timer 2: 8-bit user defined counter with auto-reload
    if ((v_ & 8) && ++tmr_count_[1] == 0) {
        tmr_count_[1] = tmr_reload_;
        irqflag_[2] = 1;
        possible_irq_ = 1;
        write_t(port_t_ ^ 1);
    }
    if (++frame_cycles_ >= (uint32_t)kFrameCycles) end_frame();
}

void MELPS4::write_v(uint8_t data) {
    // d0: enable timer 1 irq, d3: timer 2 enable
    tmr_irq_enabled_[0] = data & 1;
    possible_irq_ = 1;
    v_ = data;
}

void MELPS4::write_t(int state) {
    port_t_ = state & 1;
    set_speaker(port_t_ != 0);  // T pin -> speaker
}

void MELPS4::do_interrupt(int which) {
    inte_ = 0;
    irqflag_[which] = 0;
    tick_timer();  // m_icount--
    push_pc();
    sms_ = sm_;
    sm_ = 0;
    op_ = 0;  // fake nop
    pc_ = (uint16_t)(kIntPage << 7 | (which * 2));
}

void MELPS4::check_interrupt() {
    if (!inte_) return;
    int which;
    // assume that lower irq vectors have higher priority
    if (irqflag_[0]) which = 0;
    else if (irqflag_[1] && tmr_irq_enabled_[0]) which = 1;
    else if (irqflag_[2] && tmr_irq_enabled_[1]) which = 2;
    else return;
    do_interrupt(which);
}

uint8_t MELPS4::read_gen_port(int port) {
    switch (port) {
        case PortS: return port_s_;  // nothing drives S/F on these boards
        case PortF: return port_f_;
        default: return 0;
    }
}

void MELPS4::write_gen_port(int port, uint8_t data) {
    switch (port) {
        case PortS: port_s_ = data; board_write_port(PortS, data); break;
        case PortF: port_f_ = data & 0xf; board_write_port(PortF, data & 0xf); break;
        case PortG: board_write_port(PortG, data & 0xf); break;
        case PortU: board_write_port(PortU, data & 1); break;
        default: break;
    }
}

int MELPS4::read_d_pin(int bit) {
    bit &= 0xf;
    return ((port_d_ & kDMask) >> bit) & 1;
}

void MELPS4::write_d_pin(int bit, int state) {
    if (bit == 16) {
        port_d_ = 0;
    } else {
        bit &= 0xf;
        port_d_ = (uint16_t)(((port_d_ & ~(1 << bit)) | (state << bit)) & kDMask);
    }
    board_write_d(port_d_);
}

// --- execute ----------------------------------------------------------------

int MELPS4::execute() {
    cycles_ = 0;
    // remember previous state
    prev_op_ = op_;
    prev_pc_ = pc_;
    // Interrupts are not accepted during skips or LXY, LA, EI, DI, RT/RTS/RTI or any branch.
    if (possible_irq_ && !prohibit_irq_ && !skip_) {
        possible_irq_ = 0;
        check_interrupt();
    }
    prohibit_irq_ = 0;
    // fetch next opcode
    tick_timer();
    op_ = prg_[pc_ & kPrgMask] & 0x1ff;
    bitmask_ = (uint8_t)(1 << (op_ & 3));
    pc_ = (uint16_t)((pc_ & ~0x7f) | ((pc_ + 1) & 0x7f));  // stays in the same page
    if (skip_) {
        // if it's a long jump, skip next one as well
        if (op_ != kBaOp && (op_ & ~0xf) != kSpMask) {
            skip_ = 0;
            op_ = 0;  // fake nop
        }
    } else {
        run_op();
    }
    return cycles_;
}

void MELPS4::op_lcps() {
    // LCPS i: choose active DP,CY or DP',CY'
    if ((op_ & 1) != cps_) {
        cps_ = op_ & 1;
        std::swap(x_, x2_);
        std::swap(y_, y2_);
        std::swap(z_, z2_);
        std::swap(cy_, cy2_);
    }
}

void MELPS4::op_b() {
    // B xy: branch
    prohibit_irq_ = 1;
    uint8_t page = (uint8_t)(pc_ >> 7);
    if ((prev_op_ & ~0xf) == kSpMask) {
        sm_ = 0;
        page = sp_param_;
    } else if (sm_) {
        page = (uint8_t)(kSmPage | (op_ >> 7 & 1));
    }
    pc_ = (uint16_t)(page << 7 | (op_ & 0x7f));
    // if BA opcode was executed, set PC low 4 bits to A
    if (ba_flag_) {
        ba_flag_ = 0;
        pc_ = (uint16_t)((pc_ & ~0xf) | a_);
    }
}

void MELPS4::op_bm() {
    // BM xy: call subroutine; don't push on short calls in subroutine mode
    if (!sm_ || (prev_op_ & ~0xf) == kSpMask) push_pc();
    sm_ = 1;
    op_b();
}

void MELPS4::op_rt() {
    prohibit_irq_ = 1;
    sm_ = 0;
    pop_pc();
}

void MELPS4::op_xam() {
    const uint8_t a = a_;
    a_ = ram_r();
    ram_w(a);
    x_ ^= op_ & 3;
}

void MELPS4::op_dey() {
    y_ = (y_ - 1) & 0xf;
    skip_ = (y_ == 0xf);
}

void MELPS4::op_amc() {
    a_ = (uint8_t)(a_ + ram_r() + cy_);
    cy_ = a_ >> 4 & 1;
    a_ &= 0xf;
}

// m58846_device::execute_one
void MELPS4::run_op() {
    switch (op_ & 0x1f0) {
        case 0x30: skip_ = (y_ == (op_ & 0xf)); return;  // SEY y
        case 0x70: prohibit_irq_ = 1; sp_param_ = op_ & 0xf; return;  // SP
        case 0xa0: {  // A n: add immediate, skip on no carry (except n=6)
            const uint8_t n = op_ & 0xf;
            a_ += n;
            skip_ = !((a_ & 0x10) || n == 6);
            a_ &= 0xf;
            return;
        }
        case 0xb0:  // LA n: skip any next LA
            prohibit_irq_ = 1;
            if ((op_ & ~0xf) != (prev_op_ & ~0xf)) a_ = op_ & 0xf;
            return;
        case 0xc0: case 0xd0: case 0xe0: case 0xf0:  // LXY x,y: skip any next LXY
            prohibit_irq_ = 1;
            if ((op_ & ~0x3f) != (prev_op_ & ~0x3f)) {
                x_ = op_ >> 4 & 3;
                y_ = op_ & 0xf;
            }
            return;
        default: break;
    }
    switch (op_ & 0x1fc) {
        case 0x20: skip_ = !(ram_r() & bitmask_); return;  // SZB j
        case 0x4c: ram_w(ram_r() | bitmask_); return;      // SB j
        case 0x58: skip_ = !(board_read_k() & bitmask_); return;  // SZK j
        case 0x5c: ram_w(ram_r() & ~bitmask_); return;     // RB j
        case 0x60: op_xam(); return;                       // XAM j
        case 0x64: a_ = ram_r(); x_ ^= op_ & 3; return;    // TAM j
        case 0x68: op_xam(); op_dey(); return;             // XAMD j
        case 0x6c:                                         // XAMI j
            op_xam();
            skip_ = ((y_ & kXamiMask) == kXamiMask);
            y_ = (y_ + 1) & 0xf;
            return;
        default: break;
    }
    switch (op_) {
        case 0x06: case 0x07: write_gen_port(PortU, op_ & 1); return;  // SU/RU
        case 0x40: case 0x41: op_lcps(); return;
        case 0x4a: case 0x4b: z_ = op_ & 1; return;  // LZ
        case 0x54: case 0x55: a_ = read_gen_port(PortS) >> ((op_ & 1) ? 0 : 4) & 0xf; return;  // IAS

        case 0x00: return;  // NOP
        case 0x01: prohibit_irq_ = 1; ba_flag_ = 1; return;  // BA
        case 0x02: y_ = (y_ + 1) & 0xf; skip_ = (y_ == 0); return;  // INY
        case 0x03: op_dey(); return;
        case 0x04: prohibit_irq_ = 1; inte_ = 0; return;  // DI
        case 0x05: prohibit_irq_ = 1; possible_irq_ = 1; inte_ = 1; return;  // EI
        case 0x09: a_ = e_ & 0xf; b_ = e_ >> 4; return;  // TABE
        case 0x0a: a_ = (a_ + ram_r()) & 0xf; return;    // AM
        case 0x0b: write_gen_port(PortS, e_); return;    // OSE
        case 0x0c: y_ = a_; return;                      // TYA
        case 0x0f: a_ ^= 0xf; return;                    // CMA

        case 0x10: write_gen_port(PortS, 0); return;     // CLS
        case 0x11: write_d_pin(16, 0); write_gen_port(PortS, 0); return;  // CLDS
        case 0x13: write_d_pin(16, 0); return;           // CLD
        case 0x14: write_d_pin(y_, 0); return;           // RD
        case 0x15: write_d_pin(y_, 1); return;           // SD
        case 0x16: case 0x17: return;                    // TEPA/OSPA: PLA, not emulated in MAME
        case 0x18: {  // RL
            const uint8_t c = a_ >> 3 & 1;
            a_ = (uint8_t)((a_ << 1 | cy_) & 0xf);
            cy_ = c;
            return;
        }
        case 0x19: {  // RR
            const uint8_t c = a_ & 1;
            a_ = (uint8_t)(a_ >> 1 | cy_ << 3);
            cy_ = c;
            return;
        }
        case 0x1a: e_ = (uint8_t)(b_ << 4 | a_); return;  // TEAB
        case 0x1b: write_gen_port(PortS, (uint8_t)(b_ << 4 | a_)); return;  // OSAB
        case 0x1c: b_ = a_; return;  // TBA
        case 0x1d: a_ = y_; return;  // TAY
        case 0x1e: a_ = b_; return;  // TAB

        case 0x26: skip_ = (a_ == ram_r()); return;  // SEAM
        case 0x2b: skip_ = !read_d_pin(y_); return;   // SZD
        case 0x2f: skip_ = !cy_; return;              // SZC

        case 0x43: op_amc(); return;
        case 0x44: op_rt(); return;
        case 0x45: op_rt(); skip_ = 1; return;        // RTS
        case 0x46: op_rt(); sm_ = sms_; return;       // RTI
        case 0x48: cy_ = 0; return;                   // RC
        case 0x49: cy_ = 1; return;                   // SC

        case 0x53: op_amc(); skip_ = (cy_ != 0); return;  // AMCS
        case 0x57: a_ = board_read_k() & 0xf; return;     // IAK

        case 0x81: write_gen_port(PortF, a_); return;     // OFA
        case 0x82: skip_ = irqflag_[1]; irqflag_[1] = 0; return;  // SNZ1
        case 0x83: skip_ = irqflag_[2]; irqflag_[2] = 0; return;  // SNZ2
        case 0x84: write_gen_port(PortG, a_); return;     // OGA
        case 0x85: tmr_reload_ = tmr_count_[1] = (uint8_t)(b_ << 4 | a_); return;  // T2AB
        case 0x86: write_v(a_); return;                   // TVA
        case 0x8a: a_ = tmr_count_[1] & 0xf; b_ = tmr_count_[1] >> 4; return;  // TAB2
        case 0x8c: a_ = read_gen_port(PortF); return;     // IAF
        default: break;
    }
    // B is at $18x and BM is at $10x for all MCU types
    if (op_ >= 0x180) op_b();
    else if (op_ >= 0x100) op_bm();
    // else: illegal opcode, ignored
}

// --- board (hh_melps4.cpp) ---------------------------------------------------

uint16_t MELPS4::board_read_k() const {
    // cfrogger: K0,K1 multiplexed (F0: up/right, F1: left/down), K3 skill switch
    uint16_t r = 0;
    if (inp_mux_ & 1) r |= (uint16_t)(((buttons_ >> 0) & 1) | (((buttons_ >> 1) & 1) << 1));
    if (inp_mux_ & 2) r |= (uint16_t)(((buttons_ >> 2) & 1) | (((buttons_ >> 3) & 1) << 1));
    return (uint16_t)((r & 3) | (skill_ ? 8 : 0));
}

void MELPS4::board_write_port(int port, uint8_t data) {
    if (port == PortU) return;  // not connected on Frogger
    // F0,F1: input mux
    if (port == PortF) inp_mux_ = data & 3;
    // Sx,Fx,Gx: vfd plate
    const uint32_t mask = (port == PortS) ? 0xff : 0xf;  // port S is 8-bit
    const int shift = (port == PortS) ? 0 : (port + 1) * 4;
    set_display(grid_, (plate_ & ~(mask << shift)) | ((uint32_t)data << shift));
}

void MELPS4::board_write_d(uint16_t data) {
    // D0-D11: vfd grid
    set_display(data, plate_);
}

void MELPS4::port_input(int port, uint8_t mask, int level) {
    if (port != 0 || mask >= 32) return;
    // the Skill 1/2 slide switch, flipped by each press of its button
    if (mask == 4 && level > 0 && !((buttons_ >> 4) & 1)) skill_ ^= 1;
    if (level > 0) buttons_ |= 1u << mask;
    else buttons_ &= ~(1u << mask);
}

// --- display: pwm_display integrated over 1/60 s frames ------------------------

void MELPS4::flush_display() {
    const uint32_t dt = (uint32_t)(now_ - last_flush_);
    last_flush_ = now_;
    if (!dt || !grid_) return;
    for (int g = 0; g < kGrids; g++) {
        if (!((grid_ >> g) & 1)) continue;
        uint32_t p = plate_;
        for (int c = 0; p && c < kCols; c++, p >>= 1) {
            if (p & 1) acc_[g][c] += dt;
        }
    }
}

void MELPS4::set_display(uint32_t grid, uint32_t plate) {
    if (grid == grid_ && plate == plate_) return;
    flush_display();
    grid_ = grid;
    plate_ = plate;
}

void MELPS4::end_frame() {
    flush_display();
    const float frame = (float)frame_cycles_;
    frame_cycles_ = 0;
    for (int g = 0; g < kGrids; g++) {
        uint8_t* out = &shown_[g * 5];
        memset(out, 0, 5);
        for (int c = 0; c < kCols; c++) {
            float bri = bri_[g][c] * 0.5f + (acc_[g][c] / frame) * 0.5f;
            if (bri > 0.04f) bri = 0.04f;
            bri_[g][c] = bri;
            acc_[g][c] = 0;
            if (bri > 0.01f) out[c >> 3] |= (uint8_t)(1 << (c & 7));
        }
    }
}

uint8_t MELPS4::display_byte(int index) const {
    if (index < 0 || index >= kGrids * 5) return 0;
    return shown_[index];
}

void MELPS4::serialize(BrickStateIO& io) {
    io.pod(pc_);
    io.pod(prev_pc_);
    io.bytes(stack_, sizeof(stack_));
    io.pod(op_);
    io.pod(prev_op_);
    io.pod(bitmask_);
    io.pod(port_d_);
    io.pod(port_s_);
    io.pod(port_f_);
    io.pod(port_t_);
    io.pod(sm_);
    io.pod(sms_);
    io.pod(ba_flag_);
    io.pod(sp_param_);
    io.pod(cps_);
    io.pod(skip_);
    io.pod(inte_);
    io.pod(intp_);
    io.bytes(irqflag_, sizeof(irqflag_));
    io.pod(int_state_);
    io.pod(prohibit_irq_);
    io.pod(possible_irq_);
    io.bytes(tmr_count_, sizeof(tmr_count_));
    io.pod(tmr_reload_);
    io.bytes(tmr_irq_enabled_, sizeof(tmr_irq_enabled_));
    io.pod(a_);
    io.pod(b_);
    io.pod(e_);
    io.pod(y_);
    io.pod(y2_);
    io.pod(x_);
    io.pod(x2_);
    io.pod(z_);
    io.pod(z2_);
    io.pod(cy_);
    io.pod(cy2_);
    io.pod(h_);
    io.pod(l_);
    io.pod(c_);
    io.pod(v_);
    io.pod(w_);
    io.bytes(ram_, sizeof(ram_));
    io.pod(skill_);
    io.pod(inp_mux_);
    io.pod(grid_);
    io.pod(plate_);
    io.pod(now_);
    io.pod(last_flush_);
    io.pod(frame_cycles_);
    io.bytes(acc_, sizeof(acc_));
    io.bytes(bri_, sizeof(bri_));
    io.bytes(shown_, sizeof(shown_));
    // Buttons aren't state: whatever is held now stays held.
    if (!io.writing()) set_speaker(port_t_ != 0);
}
