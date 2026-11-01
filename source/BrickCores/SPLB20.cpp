#include "SPLB20.h"

#include <cmath>
#include <cstring>

namespace {

constexpr uint32_t kVectorNmi = 0xFFFA, kVectorReset = 0xFFFC;
constexpr int64_t kSubClock = 32768;
constexpr uint32_t kAddressSpace = 0x10000;

constexpr uint8_t kIntT2Hz = 0x01, kIntT128Hz = 0x02, kIntPowerKey = 0x04, kIntNormalKey = 0x08, kIntCounter = 0x10,
                  kIntNmiEnable = 0x80;
constexpr uint8_t kSysLcdEnable = 0x04, kSysTimerEnable = 0x10, kSys32kEnable = 0x20, kSysCpuStop = 0x40,
                  kSysRoscStop = 0x80;

}  // namespace

const SPLB20::OpFn SPLB20::kOps[256] = {
    &SPLB20::op_brk, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_ora_zp, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_php, &SPLB20::op_ora_imm, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_bpl, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_clc, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_jsr_abs, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_bit_zp, &SPLB20::op_and_zp, &SPLB20::op_rol_zp, &SPLB20::op_dummy,
    &SPLB20::op_plp, &SPLB20::op_and_imm, &SPLB20::op_rol_a, &SPLB20::op_dummy,
    &SPLB20::op_bit_abs, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_bmi, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_sec, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_rti, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_eor_zp, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_pha, &SPLB20::op_eor_imm, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_jmp_abs, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_bvc, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_eor_zp_x, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_cli, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_rts, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_adc_zp, &SPLB20::op_ror_zp, &SPLB20::op_dummy,
    &SPLB20::op_pla, &SPLB20::op_adc_imm, &SPLB20::op_ror_a, &SPLB20::op_dummy,
    &SPLB20::op_jmp_ind, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_bvs, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_sei, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_sta_ind_x, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_sta_zp, &SPLB20::op_stx_zp, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_txa, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_stx_abs, &SPLB20::op_dummy,
    &SPLB20::op_bcc, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_sta_zp_x, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_txs, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_lda_ind_x, &SPLB20::op_ldx_imm, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_lda_zp, &SPLB20::op_ldx_zp, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_lda_imm, &SPLB20::op_tax, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_lda_abs, &SPLB20::op_ldx_abs, &SPLB20::op_dummy,
    &SPLB20::op_bcs, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_lda_zp_x, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_clv, &SPLB20::op_dummy, &SPLB20::op_tsx, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_lda_abs_x, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_cmp_zp, &SPLB20::op_dec_zp, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_cmp_imm, &SPLB20::op_dex, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_bne, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_cmp_zp_x, &SPLB20::op_dec_zp_x, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_cpx_imm, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_cpx_zp, &SPLB20::op_sbc_zp, &SPLB20::op_inc_zp, &SPLB20::op_dummy,
    &SPLB20::op_inx, &SPLB20::op_sbc_imm, &SPLB20::op_nop, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_beq, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_sed, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
    &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy, &SPLB20::op_dummy,
};

// --- reset / ports ---------------------------------------------------------------

void SPLB20::reset() {
    if (!options_loaded_) {
        pullup_ext_ = (uint8_t)options().num("port_pullup.PA", 0);
        non_crystal_mode_ = options().num("non_crystal_mode", 0) != 0;
        sub_div_ = (int64_t)clock_hz();
        options_loaded_ = true;
    }
    // (The port input survives a reset, as does the sound's divider state.)
    timer_counter_ = t2hz_counter_ = t128hz_counter_ = 0;
    pc_ = 0;
    sp_ = 0;
    a_ = x_ = 0;
    set_ps(0x04);
    cpu_enbl_ = rosc_enbl_ = 1;
    ram_.assign(0x80, 0);
    data_.assign(0x800, 0);
    lcd_.assign(0x40, 0);
    pcfg_ = pdir_ = platch_ = 0;
    lcd_cfg_ = lcd_bias_ = sys_ctrl_ = int_cfg_ = ireq_ = 0;
    tc_ = 0;
    tc_preset_ = prescalar_ = keyscan_ctrl_ = 0;
    sound_enable_ = false;
    sound_update();
    go_vector(kVectorReset);
}

uint8_t SPLB20::display_byte(int index) const {
    if (index < 0 || index >= 0x40 || !(sys_ctrl_ & kSysLcdEnable)) return 0;
    return lcd_[index];
}

int SPLB20::port_id(const std::string& name) const { return name == "PA" ? 0 : -1; }

uint8_t SPLB20::port_read() const {
    return (uint8_t)((~pdir_ & platch_) | (pdir_ & (~in_low_ & (in_high_ | pullup_ext_))));
}

void SPLB20::port_input(int port, uint8_t mask, int level) {
    if (port != 0) return;
    const uint8_t prev = port_read();
    in_low_ &= ~mask;
    in_high_ &= ~mask;
    if (level == 0) in_low_ |= mask;
    else if (level > 0) in_high_ |= mask;
    if (prev != port_read() && level == 0) {
        if (mask & 0xFE) {
            if (int_cfg_ & kIntNormalKey) {
                ireq_ |= kIntNormalKey;
                nmi();
            }
        }
        if (mask & 0x01) {
            if (int_cfg_ & kIntPowerKey) {
                ireq_ |= kIntPowerKey;
                nmi();
            }
        }
    }
}

// --- interrupts and timers --------------------------------------------------------------

void SPLB20::irq() {
    push((uint8_t)(pc_ >> 8));
    push((uint8_t)pc_);
    push(ps());
    if_ = 1;
    pc_ = rom_byte(0xFFFE) | ((uint32_t)rom_byte(0xFFFF) << 8);
}

void SPLB20::nmi() {
    if (!(int_cfg_ & kIntNmiEnable)) return;
    if (rosc_enbl_ && cpu_enbl_) {
        push((uint8_t)(pc_ >> 8));
        push((uint8_t)pc_);
        push(ps());
        go_vector(kVectorNmi);
        sys_ctrl_ &= ~0x3;
    } else {
        if (sys_ctrl_ & kSys32kEnable) sys_ctrl_ |= 0x1;
        else sys_ctrl_ |= 0x3;
        cpu_enbl_ = rosc_enbl_ = 1;
        int_cfg_ &= ~kIntNmiEnable;
        go_vector(kVectorReset);
    }
}

void SPLB20::timers_clock(int64_t e) {
    if (sys_ctrl_ & kSysTimerEnable) {
        timer_counter_ -= e;
        while (timer_counter_ <= 0) {
            timer_counter_ += prescale_scaled();
            if (--tc_ < 0) {
                tc_ = tc_preset_;
                if (int_cfg_ & kIntCounter) {
                    ireq_ |= kIntCounter;
                    nmi();
                }
            }
        }
    }
    t2hz_counter_ -= e;
    while (t2hz_counter_ <= 0) {
        t2hz_counter_ += non_crystal_mode_ ? prescale_scaled() * (kSubClock / 2) : sub_div_ * (kSubClock / 2);
        if (int_cfg_ & kIntT2Hz) {
            ireq_ |= kIntT2Hz;
            nmi();
        }
    }
    t128hz_counter_ -= e;
    while (t128hz_counter_ <= 0) {
        t128hz_counter_ += non_crystal_mode_ ? prescale_scaled() * (kSubClock / 128) : sub_div_ * (kSubClock / 128);
        if (int_cfg_ & kIntT128Hz) {
            ireq_ |= kIntT128Hz;
            nmi();
        }
    }
}

int64_t SPLB20::execute_scaled() {
    int64_t exec = (int64_t)kTickHz;   // MCLOCK_DIV = 1 cycle
    if (rosc_enbl_) {
        if (cpu_enbl_) {
            const uint8_t opcode = fetch8();
            exec = (int64_t)(this->*kOps[opcode])() * (int64_t)kTickHz;
        }
        timers_clock(exec);
    } else if (sys_ctrl_ & kSys32kEnable) {
        exec = sub_div_;
        timers_clock(exec);
    }
    return exec;
}

// --- memory and I/O ----------------------------------------------------------------------------

uint8_t SPLB20::io_read(uint8_t reg) {
    switch (reg) {
        case 0x70: return lcd_cfg_;
        case 0x71: return pdir_;
        case 0x72: return pcfg_;
        case 0x73: return port_read();
        case 0x76: return lcd_bias_;
        case 0x79: {   // reading the interrupt register clears the requests
            const uint8_t buf = ireq_ | (int_cfg_ & 0x80);
            ireq_ = 0;
            return buf;
        }
        case 0x7A: return sys_ctrl_;
        case 0x7B: return tc_preset_;
        case 0x7C: return prescalar_;
        case 0x7E: return keyscan_ctrl_;
        default: return 0;
    }
}

void SPLB20::io_write(uint8_t reg, uint8_t v) {
    switch (reg) {
        case 0x70: lcd_cfg_ = v; break;
        case 0x71: pdir_ = v; break;
        case 0x72: pcfg_ = v; sound_enable_ = (v & 0xC0) == 0xC0; sound_update(); break;
        case 0x73: platch_ = v; port_output(0, (uint8_t)(~pdir_ & platch_)); break;
        case 0x76: lcd_bias_ = v; break;
        case 0x79: int_cfg_ = v; break;
        case 0x7A:
            rosc_enbl_ = ((sys_ctrl_ | ~v) & kSysRoscStop) != 0;
            cpu_enbl_ = ((sys_ctrl_ | ~v) & kSysCpuStop) != 0;
            sys_ctrl_ = v;
            if ((!rosc_enbl_ && non_crystal_mode_) || !(v & kSysTimerEnable)) sound_enable_ = false;
            else sound_enable_ = (pcfg_ & 0xC0) == 0xC0;
            sound_update();
            break;
        case 0x7B: tc_preset_ = v; tc_div_ = v + 1; sound_update(); break;
        case 0x7C: prescalar_ = v; clock_div_log2_ = v; sound_update(); break;
        case 0x7E: keyscan_ctrl_ = v; break;
        default: break;   // 0x74, 0x75, 0x77, 0x78, 0x7F: nothing happens
    }
}

uint8_t SPLB20::read_mem(uint32_t addr) {
    if (addr < 0x40) return lcd_[addr];
    if (addr >= 0x80 && addr < 0x100) return ram_[addr - 0x80];
    if (addr >= 0x200 && (addr & 0x8FFF) < 0x800) return data_[addr & 0xFFF];
    if (addr >= 0x40 && addr < 0x80) return io_present(addr) ? io_read((uint8_t)addr) : 0;
    const int64_t rom_offset = (int64_t)kAddressSpace - (int64_t)rom().size();
    if ((int64_t)addr >= rom_offset) return rom_byte((int64_t)addr - rom_offset);
    return 0;
}

void SPLB20::write_mem(uint32_t addr, uint8_t value) {
    if (addr < 0x40) lcd_[addr] = value;
    else if (addr >= 0x80 && addr < 0x100) ram_[addr - 0x80] = value;
    else if (addr >= 0x200 && (addr & 0x8FFF) < 0x800) data_[addr & 0xFFF] = value;
    else if (addr < 0x100 && io_present(addr)) io_write((uint8_t)addr, value);
}

void SPLB20::sound_update() {
    if (tc_div_ > 1 && sound_enable_)
        set_tone((double)clock_hz() / std::ldexp(1.0, clock_div_log2_) / (double)tc_div_);
    else
        set_tone(0);
}

// --- CPU core ----------------------------------------------------------------------------------------

uint8_t SPLB20::ps() const {
    return (uint8_t)((nf_ << 7) | (vf_ << 6) | (bf_ << 4) | (df_ << 3) | (if_ << 2) | (zf_ << 1) | cf_);
}

void SPLB20::set_ps(uint8_t p) {
    nf_ = p >> 7;
    vf_ = (p & 0x40) != 0;
    bf_ = (p & 0x10) != 0;
    df_ = (p & 0x08) != 0;
    if_ = (p & 0x04) != 0;
    zf_ = (p & 0x02) != 0;
    cf_ = p & 0x01;
}

void SPLB20::adc(uint8_t operand) {
    const int a = a_;
    int r = a + operand + cf_;
    if (df_ && (a & 0x0F) + (operand & 0x0F) + cf_ > 9) r += 6;
    vf_ = ((~(a ^ operand) & (a ^ r)) >> 7) & 1;
    nf_ = (r >> 7) & 1;
    if (df_ && r > 0x99) r += 0x60;
    zf_ = (r & 0xFF) == 0;
    cf_ = r > 255;
    a_ = (uint8_t)r;
}

void SPLB20::sbc(uint8_t operand) {
    const int a = a_;
    const int borrow = !cf_;
    int r = a - operand - borrow;
    if (df_) {
        if ((a & 0x0F) - (operand & 0x0F) - borrow < 0) r -= 6;
        if (r < 0) r -= 0x60;
    }
    vf_ = (((a ^ operand) & (a ^ r)) >> 7) & 1;
    nf_ = (r >> 7) & 1;
    zf_ = (r & 0xFF) == 0;
    cf_ = r >= 0;
    a_ = (uint8_t)r;
}

void SPLB20::compare(uint8_t reg, uint8_t operand) {
    const int t = reg - operand;
    nf_ = (t >> 7) & 1;
    zf_ = t == 0;
    cf_ = t >= 0;
}

int SPLB20::branch(bool taken) {
    if (!taken) { pc_ = (pc_ + 1) & 0xFFFF; return 2; }
    const uint32_t off = fetch8();
    const uint32_t prev = pc_;
    pc_ = (pc_ + off - ((off & 0x80) << 1)) & 0xFFFF;
    return 3 + (((pc_ ^ prev) > 255) ? 1 : 0);
}

// --- instructions (operands are fetched from the ROM at PC, one byte after another) ----------------------
int SPLB20::op_dummy() { return 2; }   // illegal opcode (BrickEmuPy raises)
int SPLB20::op_nop() { return 2; }
int SPLB20::op_brk() { bf_ = 1; pc_ = (pc_ + 1) & 0xFFFF; irq(); return 7; }

int SPLB20::op_ora_zp() { a_ |= read_mem(fetch8()); nz(a_); return 3; }
int SPLB20::op_ora_imm() { a_ |= fetch8(); nz(a_); return 2; }
int SPLB20::op_and_zp() { a_ &= read_mem(fetch8()); nz(a_); return 3; }
int SPLB20::op_and_imm() { a_ &= fetch8(); nz(a_); return 2; }
int SPLB20::op_eor_zp() { a_ ^= read_mem(fetch8()); nz(a_); return 3; }
int SPLB20::op_eor_imm() { a_ ^= fetch8(); nz(a_); return 2; }
int SPLB20::op_eor_zp_x() { a_ ^= read_mem((fetch8() + x_) & 0xFF); nz(a_); return 4; }
int SPLB20::op_adc_zp() { adc(read_mem(rom_byte(pc_))); pc_ = (pc_ + 1) & 0xFFFF; return 3; }
int SPLB20::op_adc_imm() { adc(rom_byte(pc_)); pc_ = (pc_ + 1) & 0xFFFF; return 2; }
int SPLB20::op_sbc_zp() { sbc(read_mem(rom_byte(pc_))); pc_ = (pc_ + 1) & 0xFFFF; return 3; }
int SPLB20::op_sbc_imm() { sbc(rom_byte(pc_)); pc_ = (pc_ + 1) & 0xFFFF; return 2; }

int SPLB20::op_php() { push(ps()); return 3; }
int SPLB20::op_pha() { push(a_); return 3; }
int SPLB20::op_plp() { set_ps(pull()); return 4; }
int SPLB20::op_pla() { a_ = pull(); nz(a_); return 4; }

int SPLB20::op_bpl() { return branch(!nf_); }
int SPLB20::op_bmi() { return branch(nf_); }
int SPLB20::op_bvc() { return branch(!vf_); }
int SPLB20::op_bvs() { return branch(vf_); }
int SPLB20::op_bcc() { return branch(!cf_); }
int SPLB20::op_bcs() { return branch(cf_); }
int SPLB20::op_bne() { return branch(!zf_); }
int SPLB20::op_beq() { return branch(zf_); }

int SPLB20::op_clc() { cf_ = 0; return 2; }
int SPLB20::op_sec() { cf_ = 1; return 2; }
int SPLB20::op_cli() { if_ = 0; return 2; }
int SPLB20::op_sei() { if_ = 1; return 2; }
int SPLB20::op_clv() { vf_ = 0; return 2; }
int SPLB20::op_sed() { df_ = 1; return 2; }

int SPLB20::op_jsr_abs() {
    const uint32_t pc = (pc_ + 1) & 0xFFFF;
    push((uint8_t)(pc >> 8));
    push((uint8_t)pc);
    pc_ = word_at_pc();
    return 6;
}
int SPLB20::op_jmp_abs() { pc_ = word_at_pc(); return 3; }
int SPLB20::op_jmp_ind() {
    const uint32_t addr = word_at_pc();
    pc_ = read_mem(addr);
    pc_ |= (uint32_t)read_mem((addr + 1) & 0xFFFF) << 8;
    return 6;
}
int SPLB20::op_rts() {
    pc_ = pull();
    pc_ |= (uint32_t)pull() << 8;
    pc_ += 1;
    return 6;
}
int SPLB20::op_rti() {
    set_ps(pull());
    pc_ = pull();
    pc_ |= (uint32_t)pull() << 8;
    return 6;
}

int SPLB20::op_bit_zp() {
    const uint8_t m = read_mem(fetch8());
    nf_ = m >> 7; vf_ = (m & 0x40) != 0; zf_ = (a_ & m) == 0;
    return 3;
}
int SPLB20::op_bit_abs() {
    const uint8_t m = read_mem(word_at_pc());
    pc_ = (pc_ + 2) & 0xFFFF;
    nf_ = m >> 7; vf_ = (m & 0x40) != 0; zf_ = (a_ & m) == 0;
    return 4;
}

int SPLB20::op_rol_zp() {
    const uint32_t a = fetch8();
    const int n = (read_mem(a) << 1) | cf_;
    write_mem(a, (uint8_t)n);
    nf_ = (n & 0x80) != 0; zf_ = (n & 0xFF) == 0; cf_ = n > 0xFF;
    return 5;
}
int SPLB20::op_rol_a() {
    const int n = (a_ << 1) | cf_;
    a_ = (uint8_t)n;
    nf_ = (n & 0x80) != 0; zf_ = (n & 0xFF) == 0; cf_ = n > 0xFF;
    return 2;
}
int SPLB20::op_ror_zp() {
    const uint32_t a = fetch8();
    const uint8_t prev = read_mem(a);
    write_mem(a, (uint8_t)((prev >> 1) | (cf_ << 7)));
    nf_ = cf_; zf_ = ((prev & 0xFE) | cf_) == 0; cf_ = prev & 1;
    return 5;
}
int SPLB20::op_ror_a() {
    const uint8_t prev = a_;
    a_ = (uint8_t)((prev >> 1) | (cf_ << 7));
    nf_ = cf_; zf_ = ((prev & 0xFE) | cf_) == 0; cf_ = prev & 1;
    return 2;
}

int SPLB20::op_inc_zp() { const uint32_t a = fetch8(); const uint8_t v = read_mem(a) + 1; write_mem(a, v); nz(v); return 5; }
int SPLB20::op_dec_zp() { const uint32_t a = fetch8(); const uint8_t v = read_mem(a) - 1; write_mem(a, v); nz(v); return 5; }
int SPLB20::op_dec_zp_x() { const uint32_t a = (rom_byte(pc_) + x_) & 0xFF; pc_ = (pc_ + 1) & 0xFFFF; const uint8_t v = read_mem(a) - 1; write_mem(a, v); nz(v); return 6; }
int SPLB20::op_inx() { x_++; nz(x_); return 2; }
int SPLB20::op_dex() { x_--; nz(x_); return 2; }

int SPLB20::op_sta_ind_x() {
    const uint32_t op = fetch8();
    const uint32_t addr = read_mem((op + x_) & 0xFF) | ((uint32_t)read_mem((op + x_ + 1) & 0xFF) << 8);
    write_mem(addr, a_);
    return 6;
}
int SPLB20::op_sta_zp() { write_mem(rom_byte(pc_), a_); pc_ = (pc_ + 1) & 0xFFFF; return 3; }
int SPLB20::op_sta_zp_x() { write_mem((rom_byte(pc_) + x_) & 0xFF, a_); pc_ = (pc_ + 1) & 0xFFFF; return 4; }
int SPLB20::op_stx_zp() { write_mem(rom_byte(pc_), x_); pc_ = (pc_ + 1) & 0xFFFF; return 3; }
int SPLB20::op_stx_abs() { write_mem(word_at_pc(), x_); pc_ = (pc_ + 2) & 0xFFFF; return 4; }

int SPLB20::op_lda_ind_x() {
    const uint32_t value = (uint32_t)fetch8() + x_;
    a_ = read_mem(read_mem(value & 0xFF) | ((uint32_t)read_mem((value + 1) & 0xFF) << 8));
    nz(a_);
    return 6;
}
int SPLB20::op_lda_zp() { a_ = read_mem(rom_byte(pc_)); pc_ = (pc_ + 1) & 0xFFFF; nz(a_); return 3; }
int SPLB20::op_lda_imm() { a_ = fetch8(); nz(a_); return 2; }
int SPLB20::op_lda_abs() { a_ = read_mem(word_at_pc()); pc_ = (pc_ + 2) & 0xFFFF; nz(a_); return 4; }
int SPLB20::op_lda_zp_x() { a_ = read_mem((rom_byte(pc_) + x_) & 0xFF); pc_ = (pc_ + 1) & 0xFFFF; nz(a_); return 4; }
int SPLB20::op_lda_abs_x() {
    const uint32_t base = word_at_pc();
    pc_ = (pc_ + 2) & 0xFFFF;
    const uint32_t addr = (base + x_) & 0xFFFF;
    a_ = read_mem(addr);
    nz(a_);
    return 4 + (((base ^ addr) > 255) ? 1 : 0);
}
int SPLB20::op_ldx_imm() { x_ = fetch8(); nz(x_); return 2; }
int SPLB20::op_ldx_zp() { x_ = read_mem(rom_byte(pc_)); pc_ = (pc_ + 1) & 0xFFFF; nz(x_); return 3; }
int SPLB20::op_ldx_abs() { x_ = read_mem(word_at_pc()); pc_ = (pc_ + 2) & 0xFFFF; nz(x_); return 4; }

int SPLB20::op_tax() { x_ = a_; nz(x_); return 2; }
int SPLB20::op_txa() { a_ = x_; nz(a_); return 2; }
int SPLB20::op_tsx() { x_ = sp_; nz(x_); return 2; }
int SPLB20::op_txs() { sp_ = x_; return 2; }

int SPLB20::op_cmp_zp() { compare(a_, read_mem(rom_byte(pc_))); pc_ = (pc_ + 1) & 0xFFFF; return 3; }
int SPLB20::op_cmp_imm() { compare(a_, rom_byte(pc_)); pc_ = (pc_ + 1) & 0xFFFF; return 2; }
int SPLB20::op_cmp_zp_x() { compare(a_, read_mem((rom_byte(pc_) + x_) & 0xFF)); pc_ = (pc_ + 1) & 0xFFFF; return 4; }
int SPLB20::op_cpx_imm() { compare(x_, rom_byte(pc_)); pc_ = (pc_ + 1) & 0xFFFF; return 2; }
int SPLB20::op_cpx_zp() { compare(x_, read_mem(rom_byte(pc_))); pc_ = (pc_ + 1) & 0xFFFF; return 3; }

// --- save states --------------------------------------------------------------------------------------------

void SPLB20::serialize(BrickStateIO& io) {
    io.pod(pc_);
    io.pod(a_); io.pod(x_); io.pod(sp_);
    uint8_t flags[9] = {nf_, vf_, bf_, df_, if_, zf_, cf_, cpu_enbl_, rosc_enbl_};
    io.bytes(flags, sizeof(flags));
    nf_ = flags[0]; vf_ = flags[1]; bf_ = flags[2]; df_ = flags[3]; if_ = flags[4];
    zf_ = flags[5]; cf_ = flags[6]; cpu_enbl_ = flags[7]; rosc_enbl_ = flags[8];
    io.vec(ram_);
    io.vec(data_);
    io.vec(lcd_);
    io.pod(timer_counter_);
    io.pod(t2hz_counter_);
    io.pod(t128hz_counter_);
    io.pod(tc_);
    uint8_t regs[13] = {pdir_, pcfg_, platch_, in_low_, in_high_, lcd_cfg_, lcd_bias_, sys_ctrl_, int_cfg_, ireq_,
                        tc_preset_, prescalar_, keyscan_ctrl_};
    io.bytes(regs, sizeof(regs));
    pdir_ = regs[0]; pcfg_ = regs[1]; platch_ = regs[2]; in_low_ = regs[3]; in_high_ = regs[4]; lcd_cfg_ = regs[5];
    lcd_bias_ = regs[6]; sys_ctrl_ = regs[7]; int_cfg_ = regs[8]; ireq_ = regs[9]; tc_preset_ = regs[10];
    prescalar_ = regs[11]; keyscan_ctrl_ = regs[12];
    io.pod(tc_div_);
    io.pod(clock_div_log2_);
    uint8_t se = sound_enable_;
    io.pod(se);
    sound_enable_ = se != 0;
    sound_update();
}
