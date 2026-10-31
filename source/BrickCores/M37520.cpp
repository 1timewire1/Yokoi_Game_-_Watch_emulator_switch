#include "M37520.h"

#include <cstring>

namespace {

constexpr int kVectorReset = 4;
constexpr int kVectorBrk = 34;
constexpr int kAddressSpace = 0x10000;

constexpr uint32_t kSfrSize = 0x40;
constexpr uint32_t kRamSize = 0x200, kRamOffset = 0x40;
constexpr uint32_t kLcdSize = 0xB0, kLcdOffsetB0 = 0x40, kLcdOffsetB3 = 0x340;

// IREQ1 / ICON1
constexpr uint8_t kIreq1Int0 = 0x01, kIreq1Int1 = 0x02, kIreq1Tx = 0x10, kIreq1Ty = 0x20, kIreq1T2 = 0x40,
                  kIreq1T3 = 0x80;
// IREQ2 / ICON2
constexpr uint8_t kIreq2T1 = 0x10, kIreq2KeyOn = 0x20;
constexpr uint8_t kIntEdgeInt0 = 0x01, kIntEdgeInt1 = 0x02;
constexpr uint8_t kTxmWCtrl = 0x01, kTxmStop = 0x80, kTymStop = 0x80;
constexpr uint8_t kT123mT2WCtrl = 0x04, kT123mT2Src = 0x08, kT123mT3Src = 0x10, kT123mT1Src = 0x20;
constexpr uint8_t kLmPage0 = 0x04, kLmEnable = 0x08;
constexpr uint8_t kCpumSPage1 = 0x04, kCpumMainDiv8 = 0x40, kCpumClkLow = 0x80;

}  // namespace

const M37520::OpEntry M37520::kOps[256] = {
    {&M37520::op_brk, 1}, {&M37520::op_ora_ind_x, 2}, {&M37520::op_jsr_zp_ind, 2}, {&M37520::op_bbs_bit_a, 2},
    {&M37520::op_dummy, 1}, {&M37520::op_ora_zp, 2}, {&M37520::op_asl_zp, 2}, {&M37520::op_bbs_bit_zp, 3},
    {&M37520::op_php, 1}, {&M37520::op_ora_imm, 2}, {&M37520::op_asl_a, 1}, {&M37520::op_seb_bit_a, 1},
    {&M37520::op_dummy, 1}, {&M37520::op_ora_abs, 3}, {&M37520::op_asl_abs, 3}, {&M37520::op_seb_bit_zp, 2},
    {&M37520::op_bpl, 2}, {&M37520::op_ora_ind_y, 2}, {&M37520::op_clt, 1}, {&M37520::op_bbc_bit_a, 2},
    {&M37520::op_dummy, 1}, {&M37520::op_ora_zp_x, 2}, {&M37520::op_asl_zp_x, 2}, {&M37520::op_bbc_bit_zp, 3},
    {&M37520::op_clc, 1}, {&M37520::op_ora_abs_y, 3}, {&M37520::op_dec_a, 1}, {&M37520::op_clb_bit_a, 1},
    {&M37520::op_dummy, 1}, {&M37520::op_ora_abs_x, 3}, {&M37520::op_asl_abs_x, 3}, {&M37520::op_clb_bit_zp, 2},
    {&M37520::op_jsr_abs, 3}, {&M37520::op_and_ind_x, 2}, {&M37520::op_jsr_sp, 2}, {&M37520::op_bbs_bit_a, 2},
    {&M37520::op_bit_zp, 2}, {&M37520::op_and_zp, 2}, {&M37520::op_rol_zp, 2}, {&M37520::op_bbs_bit_zp, 3},
    {&M37520::op_plp, 1}, {&M37520::op_and_imm, 2}, {&M37520::op_rol_a, 1}, {&M37520::op_seb_bit_a, 1},
    {&M37520::op_bit_abs, 3}, {&M37520::op_and_abs, 3}, {&M37520::op_rol_abs, 3}, {&M37520::op_seb_bit_zp, 2},
    {&M37520::op_bmi, 2}, {&M37520::op_and_ind_y, 2}, {&M37520::op_set, 1}, {&M37520::op_bbc_bit_a, 2},
    {&M37520::op_dummy, 1}, {&M37520::op_and_zp_x, 2}, {&M37520::op_rol_zp_x, 2}, {&M37520::op_bbc_bit_zp, 3},
    {&M37520::op_sec, 1}, {&M37520::op_and_abs_y, 3}, {&M37520::op_inc_a, 1}, {&M37520::op_clb_bit_a, 1},
    {&M37520::op_ldm_zp, 3}, {&M37520::op_and_abs_x, 3}, {&M37520::op_rol_abs_x, 3}, {&M37520::op_clb_bit_zp, 2},
    {&M37520::op_rti, 1}, {&M37520::op_eor_ind_x, 2}, {&M37520::op_stp, 1}, {&M37520::op_bbs_bit_a, 2},
    {&M37520::op_com_zp, 2}, {&M37520::op_eor_zp, 2}, {&M37520::op_lsr_zp, 2}, {&M37520::op_bbs_bit_zp, 3},
    {&M37520::op_pha, 1}, {&M37520::op_eor_imm, 2}, {&M37520::op_lsr_a, 1}, {&M37520::op_seb_bit_a, 1},
    {&M37520::op_jmp_abs, 3}, {&M37520::op_eor_abs, 3}, {&M37520::op_lsr_abs, 3}, {&M37520::op_seb_bit_zp, 2},
    {&M37520::op_bvc, 2}, {&M37520::op_eor_ind_y, 2}, {&M37520::op_dummy, 1}, {&M37520::op_bbc_bit_a, 2},
    {&M37520::op_dummy, 1}, {&M37520::op_eor_zp_x, 2}, {&M37520::op_lsr_zp_x, 2}, {&M37520::op_bbc_bit_zp, 3},
    {&M37520::op_cli, 1}, {&M37520::op_eor_abs_y, 3}, {&M37520::op_dummy, 1}, {&M37520::op_clb_bit_a, 1},
    {&M37520::op_dummy, 1}, {&M37520::op_eor_abs_x, 3}, {&M37520::op_lsr_abs_x, 3}, {&M37520::op_clb_bit_zp, 2},
    {&M37520::op_rts, 1}, {&M37520::op_adc_ind_x, 2}, {&M37520::op_mul_zp_x, 2}, {&M37520::op_bbs_bit_a, 2},
    {&M37520::op_tst_zp, 2}, {&M37520::op_adc_zp, 2}, {&M37520::op_ror_zp, 2}, {&M37520::op_bbs_bit_zp, 3},
    {&M37520::op_pla, 1}, {&M37520::op_adc_imm, 2}, {&M37520::op_ror_a, 1}, {&M37520::op_seb_bit_a, 1},
    {&M37520::op_jmp_ind, 3}, {&M37520::op_adc_abs, 3}, {&M37520::op_ror_abs, 3}, {&M37520::op_seb_bit_zp, 2},
    {&M37520::op_bvs, 2}, {&M37520::op_adc_ind_y, 2}, {&M37520::op_dummy, 1}, {&M37520::op_bbc_bit_a, 2},
    {&M37520::op_dummy, 1}, {&M37520::op_adc_zp_x, 2}, {&M37520::op_ror_zp_x, 2}, {&M37520::op_bbc_bit_zp, 3},
    {&M37520::op_sei, 1}, {&M37520::op_adc_abs_y, 3}, {&M37520::op_dummy, 1}, {&M37520::op_clb_bit_a, 1},
    {&M37520::op_dummy, 1}, {&M37520::op_adc_abs_x, 3}, {&M37520::op_ror_abs_x, 3}, {&M37520::op_clb_bit_zp, 2},
    {&M37520::op_bra, 2}, {&M37520::op_sta_ind_x, 2}, {&M37520::op_rrf_zp, 2}, {&M37520::op_bbs_bit_a, 2},
    {&M37520::op_sty_zp, 2}, {&M37520::op_sta_zp, 2}, {&M37520::op_stx_zp, 2}, {&M37520::op_bbs_bit_zp, 3},
    {&M37520::op_dey, 1}, {&M37520::op_dummy, 1}, {&M37520::op_txa, 1}, {&M37520::op_seb_bit_a, 1},
    {&M37520::op_sty_abs, 3}, {&M37520::op_sta_abs, 3}, {&M37520::op_stx_abs, 3}, {&M37520::op_seb_bit_zp, 2},
    {&M37520::op_bcc, 2}, {&M37520::op_sta_ind_y, 2}, {&M37520::op_dummy, 1}, {&M37520::op_bbc_bit_a, 2},
    {&M37520::op_sty_zp_x, 2}, {&M37520::op_sta_zp_x, 2}, {&M37520::op_stx_zp_y, 2}, {&M37520::op_bbc_bit_zp, 3},
    {&M37520::op_tya, 1}, {&M37520::op_sta_abs_y, 3}, {&M37520::op_txs, 1}, {&M37520::op_clb_bit_a, 1},
    {&M37520::op_dummy, 1}, {&M37520::op_sta_abs_x, 3}, {&M37520::op_dummy, 1}, {&M37520::op_clb_bit_zp, 2},
    {&M37520::op_ldy_imm, 2}, {&M37520::op_lda_ind_x, 2}, {&M37520::op_ldx_imm, 2}, {&M37520::op_bbs_bit_a, 2},
    {&M37520::op_ldy_zp, 2}, {&M37520::op_lda_zp, 2}, {&M37520::op_ldx_zp, 2}, {&M37520::op_bbs_bit_zp, 3},
    {&M37520::op_tay, 1}, {&M37520::op_lda_imm, 2}, {&M37520::op_tax, 1}, {&M37520::op_seb_bit_a, 1},
    {&M37520::op_ldy_abs, 3}, {&M37520::op_lda_abs, 3}, {&M37520::op_ldx_abs, 3}, {&M37520::op_seb_bit_zp, 2},
    {&M37520::op_bcs, 2}, {&M37520::op_lda_ind_y, 2}, {&M37520::op_jmp_zp_ind, 2}, {&M37520::op_bbc_bit_a, 2},
    {&M37520::op_ldy_zp_x, 2}, {&M37520::op_lda_zp_x, 2}, {&M37520::op_ldx_zp_y, 2}, {&M37520::op_bbc_bit_zp, 3},
    {&M37520::op_clv, 1}, {&M37520::op_lda_abs_y, 3}, {&M37520::op_tsx, 1}, {&M37520::op_clb_bit_a, 1},
    {&M37520::op_ldy_abs_x, 3}, {&M37520::op_lda_abs_x, 3}, {&M37520::op_ldx_abs_y, 3}, {&M37520::op_clb_bit_zp, 2},
    {&M37520::op_cpy_imm, 2}, {&M37520::op_cmp_ind_x, 2}, {&M37520::op_wit, 1}, {&M37520::op_bbs_bit_a, 2},
    {&M37520::op_cpy_zp, 2}, {&M37520::op_cmp_zp, 2}, {&M37520::op_dec_zp, 2}, {&M37520::op_bbs_bit_zp, 3},
    {&M37520::op_iny, 1}, {&M37520::op_cmp_imm, 2}, {&M37520::op_dex, 1}, {&M37520::op_seb_bit_a, 1},
    {&M37520::op_cpy_abs, 3}, {&M37520::op_cmp_abs, 3}, {&M37520::op_dec_abs, 3}, {&M37520::op_seb_bit_zp, 2},
    {&M37520::op_bne, 2}, {&M37520::op_cmp_ind_y, 2}, {&M37520::op_dummy, 1}, {&M37520::op_bbc_bit_a, 2},
    {&M37520::op_dummy, 1}, {&M37520::op_cmp_zp_x, 2}, {&M37520::op_dec_zp_x, 2}, {&M37520::op_bbc_bit_zp, 3},
    {&M37520::op_cld, 1}, {&M37520::op_cmp_abs_y, 3}, {&M37520::op_dummy, 1}, {&M37520::op_clb_bit_a, 1},
    {&M37520::op_dummy, 1}, {&M37520::op_cmp_abs_x, 3}, {&M37520::op_dec_abs_x, 3}, {&M37520::op_clb_bit_zp, 2},
    {&M37520::op_cpx_imm, 2}, {&M37520::op_sbc_ind_x, 2}, {&M37520::op_div_zp_x, 2}, {&M37520::op_bbs_bit_a, 2},
    {&M37520::op_cpx_zp, 2}, {&M37520::op_sbc_zp, 2}, {&M37520::op_inc_zp, 2}, {&M37520::op_bbs_bit_zp, 3},
    {&M37520::op_inx, 1}, {&M37520::op_sbc_imm, 2}, {&M37520::op_nop, 1}, {&M37520::op_seb_bit_a, 1},
    {&M37520::op_cpx_abs, 3}, {&M37520::op_sbc_abs, 3}, {&M37520::op_inc_abs, 3}, {&M37520::op_seb_bit_zp, 2},
    {&M37520::op_beq, 2}, {&M37520::op_sbc_ind_y, 2}, {&M37520::op_dummy, 1}, {&M37520::op_bbc_bit_a, 2},
    {&M37520::op_dummy, 1}, {&M37520::op_sbc_zp_x, 2}, {&M37520::op_inc_zp_x, 2}, {&M37520::op_bbc_bit_zp, 3},
    {&M37520::op_sed, 1}, {&M37520::op_sbc_abs_y, 3}, {&M37520::op_dummy, 1}, {&M37520::op_clb_bit_a, 1},
    {&M37520::op_dummy, 1}, {&M37520::op_sbc_abs_x, 3}, {&M37520::op_inc_abs_x, 3}, {&M37520::op_clb_bit_zp, 2},
};

// --- reset / ports ----------------------------------------------------------

void M37520::reset() {
    if (!options_loaded_) {
        static const char* kNames[5] = {"P0", "P1", "P2", "P3", "P4"};
        for (int i = 0; i < 5; i++) pullup_ext_[i] = (uint8_t)options().num(std::string("port_pullup.") + kNames[i], 0);
        rom_offset_ = kAddressSpace - (int64_t)rom().size();
        options_loaded_ = true;
    }
    // (Like M37520.reset(): the port inputs and the timers' fractional
    // counters survive a reset.)
    pc_ = 0;
    a_ = x_ = y_ = 0;
    set_ps(0x04);
    sp_ = 0;
    spp_ = 0x100;
    nwait_ = nstop_ = 1;
    ram_.assign(kRamSize, 0);
    lcd_.assign(kLcdSize, 0);
    lcd_offset_ = kLcdOffsetB3;
    memset(pdir_, 0, sizeof(pdir_));
    memset(pullup_, 0, sizeof(pullup_));
    memset(platch_, 0, sizeof(platch_));
    tx_ = tx_latch_ = 0xFFFF;
    ty_ = ty_latch_ = 0xFFFF;
    t1_ = t1_latch_ = 0xFF;
    t2_ = t2_latch_ = 0x01;
    t3_ = t3_latch_ = 0xFF;
    txm_ = tym_ = t123m_ = lc_ = intedge_ = 0;
    ireq1_ = ireq2_ = icon1_ = icon2_ = 0;
    set_lm(0x00);
    set_cpum(0x4C);
    go_vector(kVectorReset);
}

void M37520::go_vector(int addr) {
    const int64_t at = (kAddressSpace - addr) - rom_offset_;
    pc_ = (uint32_t)(rom_byte(at) | (rom_byte(at + 1) << 8)) % kAddressSpace;
}

uint8_t M37520::display_byte(int index) const {
    if (index < 0 || index >= (int)kLcdSize) return 0;
    return (lm_ & kLmEnable) ? lcd_[index] : 0;
}

int M37520::port_id(const std::string& name) const {
    if (name.size() == 2 && name[0] == 'P' && name[1] >= '0' && name[1] <= '4') return name[1] - '0';
    return -1;
}

uint8_t M37520::port_read(int port) const {
    return (uint8_t)((pdir_[port] & platch_[port]) |
                     (~pdir_[port] & ~in_low_[port] & (in_high_[port] | pullup_[port] | pullup_ext_[port])));
}

void M37520::port_input(int port, uint8_t mask, int level) {
    if (port < 0 || port > 4) return;
    const uint8_t prev = port_read(port);
    in_low_[port] &= ~mask;
    in_high_[port] &= ~mask;
    if (level == 0) in_low_[port] |= mask;
    else if (level > 0) in_high_[port] |= mask;
    if (prev == port_read(port)) return;
    if (port == 2 && level == 0) {
        ireq2_ |= kIreq2KeyOn;
    } else if (port == 4) {
        if ((mask & 0x1) && level >= 0 && ((intedge_ & kIntEdgeInt0) != 0) == (level != 0)) ireq1_ |= kIreq1Int0;
        else if ((mask & 0x2) && level >= 0 && ((intedge_ & kIntEdgeInt1) != 0) == (level != 0)) ireq1_ |= kIreq1Int1;
    }
}

void M37520::set_lm(uint8_t v) {
    lm_ = v;
    lcd_offset_ = (v & kLmPage0) ? kLcdOffsetB0 : kLcdOffsetB3;
}

void M37520::set_cpum(uint8_t v) {
    cpum_ = v;
    if (cpum_ & kCpumClkLow) {
        // Running from the 32768 Hz sub clock: one tick = clock/32768 cycles.
        cpu_div_ = 2 * (int64_t)clock_hz();
        main_div_ = (int64_t)clock_hz();
    } else {
        cpu_div_ = 2 * (int64_t)kTickHz;
        main_div_ = (int64_t)kTickHz;
        if (cpum_ & kCpumMainDiv8) cpu_div_ *= 4;
    }
    spp_ = (cpum_ & kCpumSPage1) ? 0x100 : 0;
}

// --- memory and I/O ---------------------------------------------------------

uint8_t M37520::io_read(uint8_t reg) {
    switch (reg) {
        case 0x00: return port_read(0);
        case 0x01: return pdir_[0];
        case 0x02: return port_read(1);
        case 0x03: return pdir_[1];
        case 0x04: return port_read(2);
        case 0x05: return pdir_[2];
        case 0x06: return port_read(3);
        case 0x07: return pdir_[3];
        case 0x08: return port_read(4);
        case 0x09: return pdir_[4];
        case 0x0B: return pullup_[0];
        case 0x0C: return pullup_[1];
        case 0x0D: return pullup_[2];
        case 0x0E: return pullup_[3];
        case 0x0F: return pullup_[4];
        case 0x20: return (uint8_t)(tx_ & 0xFF);
        case 0x21: return (uint8_t)(tx_ >> 8);
        case 0x22: return (uint8_t)(ty_ & 0xFF);
        case 0x23: return (uint8_t)(ty_ >> 8);
        case 0x24: return (uint8_t)t1_;
        case 0x25: return (uint8_t)t2_;
        case 0x26: return (uint8_t)t3_;
        case 0x27: return txm_;
        case 0x28: return tym_;
        case 0x29: return t123m_;
        case 0x37: return lc_;
        case 0x39: return lm_;
        case 0x3A: return intedge_;
        case 0x3B: return cpum_;
        case 0x3C: return ireq1_;
        case 0x3D: return ireq2_;
        case 0x3E: return icon1_;
        case 0x3F: return icon2_;
        default: return 0;  // 0x0A, 0x10-0x1F, 0x2A-0x34, 0x35, 0x36, 0x38: unmapped
    }
}

void M37520::io_write(uint8_t reg, uint8_t v) {
    switch (reg) {
        case 0x00: platch_[0] = v; break;
        case 0x01: pdir_[0] = v; break;
        case 0x02: platch_[1] = v; break;
        case 0x03: pdir_[1] = v; break;
        case 0x04: platch_[2] = v; break;
        case 0x05: pdir_[2] = v; break;
        case 0x06: {
            // P3 bits 6/5 drive the speaker pins: opposite levels = a signal.
            const bool h1 = (v & 0x40) != 0, h2 = (v & 0x20) != 0;
            set_speaker(h1 != h2 && h1);
            platch_[3] = v;
            break;
        }
        case 0x07: pdir_[3] = v; break;
        case 0x08: platch_[4] = v & 0xFE; break;
        case 0x09: pdir_[4] = v & 0xFE; break;
        case 0x0B: pullup_[0] = v; break;
        case 0x0C: pullup_[1] = v; break;
        case 0x0D: pullup_[2] = v; break;
        case 0x0E: pullup_[3] = v; break;
        case 0x0F: pullup_[4] = v; break;
        case 0x20:
            tx_latch_ = (tx_ & 0xFF00) | v;
            if (!(txm_ & kTxmWCtrl)) tx_ = tx_latch_;
            break;
        case 0x21:
            tx_latch_ = (tx_ & 0xFF) | (v << 8);
            if (!(txm_ & kTxmWCtrl)) tx_ = tx_latch_;
            break;
        case 0x22: ty_ = (ty_ & 0xFF00) | v; ty_latch_ = ty_; break;
        case 0x23: ty_ = (ty_ & 0xFF) | (v << 8); ty_latch_ = ty_; break;
        case 0x24: t1_ = t1_latch_ = v; break;
        case 0x25:
            if (!(t123m_ & kT123mT2WCtrl)) t2_ = v;
            t2_latch_ = v;
            break;
        case 0x26: t3_ = t3_latch_ = v; break;
        case 0x27: txm_ = v; break;
        case 0x28: tym_ = v; break;
        case 0x29: t123m_ = v; break;
        case 0x37: lc_ = v; break;
        case 0x39: set_lm(v); break;
        case 0x3A: intedge_ = v; break;
        case 0x3B: set_cpum(v); break;
        case 0x3C: ireq1_ &= v; break;  // write 0 to clear
        case 0x3D: ireq2_ &= v; break;
        case 0x3E: icon1_ = v; break;
        case 0x3F: icon2_ = v; break;
        default: break;
    }
    if (reg <= 0x09 && !(reg & 1)) port_output(reg >> 1, (uint8_t)(platch_[reg >> 1] & pdir_[reg >> 1]));
}

uint8_t M37520::read_mem(uint32_t addr) {
    if (addr >= lcd_offset_ && addr < lcd_offset_ + kLcdSize) return lcd_[addr - lcd_offset_];
    if (addr >= kRamOffset && addr < kRamOffset + kRamSize) return ram_[addr - kRamOffset];
    if (addr < kSfrSize) return io_read((uint8_t)addr);
    if (addr >= rom_offset_) return rom_byte((int64_t)addr - rom_offset_);
    return 0;
}

// set_clock()'s access: CPU RAM only (0x40-0x23F), never the SFRs/LCD.
bool M37520::poke_ram(uint32_t addr, uint8_t v) {
    if (addr < kRamOffset || addr >= kRamOffset + kRamSize) return false;
    ram_[addr - kRamOffset] = v;
    return true;
}

int M37520::peek_ram(uint32_t addr) const {
    if (addr < kRamOffset || addr >= kRamOffset + kRamSize) return -1;
    return ram_[addr - kRamOffset];
}

void M37520::write_mem(uint32_t addr, uint8_t value) {
    if (addr >= lcd_offset_ && addr < lcd_offset_ + kLcdSize) lcd_[addr - lcd_offset_] = value;
    else if (addr >= kRamOffset && addr < kRamOffset + kRamSize) ram_[addr - kRamOffset] = value;
    else if (addr < kSfrSize) io_write((uint8_t)addr, value);
}

// --- interrupts and timers ----------------------------------------------------

void M37520::interrupt(int addr) {
    nwait_ = 1;
    push((uint8_t)(pc_ >> 8));
    push((uint8_t)pc_);
    push(ps());
    if_ = 1;
    go_vector(addr);
}

void M37520::interrupt_process() {
    if (ireq1_) {
        for (int i = 0; i < 8; i++) {
            if (ireq1_ & icon1_ & (1 << i)) {
                ireq1_ &= ~(1 << i);
                return interrupt(6 + (i << 1));
            }
        }
    }
    if (ireq2_) {
        for (int i = 0; i < 6; i++) {
            if (ireq2_ & icon2_ & (1 << i)) {
                ireq2_ &= ~(1 << i);
                return interrupt(22 + (i << 1));
            }
        }
    }
}

void M37520::timers_clock(int64_t e) {
    const int64_t sub = (int64_t)clock_hz();   // one 32768 Hz sub-clock tick, scaled
    const int64_t sub32 = sub * 32;
    // (A zero period would never leave the loops - BrickEmuPy would hang the
    // same way - so a period is at least one scaled unit.)
    auto period = [](int64_t p) { return p > 0 ? p : 1; };

    if (!(txm_ & kTxmStop)) {
        tx_counter_ -= e;
        while (tx_counter_ <= 0) {
            tx_counter_ += period(main_div_);
            if (--tx_ < 0) {
                tx_ = tx_latch_;
                ireq1_ |= kIreq1Tx;
            }
        }
    }
    if (!(tym_ & kTymStop)) {
        ty_counter_ -= e;
        while (ty_counter_ <= 0) {
            ty_counter_ += period(main_div_);
            if (--ty_ < 0) {
                ty_ = ty_latch_;
                ireq1_ |= kIreq1Ty;
            }
        }
    }

    t1_counter_ -= e;
    while (t1_counter_ <= 0) {
        t1_counter_ += period((t123m_ & kT123mT1Src) ? sub32 : main_div_);
        if (--t1_ < 0) {
            t1_ = t1_latch_;
            ireq2_ |= kIreq2T1;
        }
    }

    t2_counter_ -= e;
    while (t2_counter_ <= 0) {
        if (t123m_ & kT123mT2Src) t2_counter_ += period(main_div_);
        else if (t123m_ & kT123mT1Src) t2_counter_ += period(sub32 * t1_latch_);
        else t2_counter_ += period(main_div_ * t1_latch_);
        if (--t2_ < 0) {
            t2_ = t2_latch_;
            ireq1_ |= kIreq1T2;
        }
    }

    t3_counter_ -= e;
    while (t3_counter_ <= 0) {
        if (t123m_ & kT123mT3Src) t3_counter_ += period(sub32);
        else if (t123m_ & kT123mT1Src) t3_counter_ += period(sub32 * t1_latch_);
        else t3_counter_ += period(main_div_ * t1_latch_);
        if (--t3_ < 0) {
            t3_ = t3_latch_;
            ireq1_ |= kIreq1T3;
        }
    }
}

// --- CPU core -------------------------------------------------------------------

uint8_t M37520::ps() const {
    return (uint8_t)((nf_ << 7) | (vf_ << 6) | (tf_ << 5) | (bf_ << 4) | (df_ << 3) | (if_ << 2) | (zf_ << 1) | cf_);
}

void M37520::set_ps(uint8_t p) {
    nf_ = p >> 7;
    vf_ = (p & 0x40) != 0;
    tf_ = (p & 0x20) != 0;
    bf_ = (p & 0x10) != 0;
    df_ = (p & 0x08) != 0;
    if_ = (p & 0x04) != 0;
    zf_ = (p & 0x02) != 0;
    cf_ = p & 0x01;
}

int64_t M37520::execute_scaled() {
    int64_t exec = cpu_div_;  // taken before the instruction: it may change CPUM
    if (nstop_) {
        if (nwait_) {
            const uint8_t b = rom_byte((int64_t)pc_ - rom_offset_);
            const OpEntry& e = kOps[b];
            uint32_t op = 0;
            for (int i = 0; i < e.len; i++) op = (op << 8) | rom_byte((int64_t)pc_ - rom_offset_ + i);
            pc_ += e.len;
            exec *= (this->*e.fn)(op);
        }
        timers_clock(exec);
    }
    if (!if_) interrupt_process();
    return exec;
}

// ALU operations. With the T flag set they work on memory[X] instead of A
// and cost extra cycles (the return value).
int M37520::alu_adc(uint8_t operand) {
    const int a = tf_ ? read_mem(x_) : a_;
    int r = a + operand + cf_;
    if (df_ && (a & 0x0F) + (operand & 0x0F) + cf_ > 9) r += 6;
    vf_ = ((~(a ^ operand) & (a ^ r)) >> 7) & 1;
    nf_ = (r >> 7) & 1;
    if (df_ && r > 0x99) r += 0x60;
    zf_ = (r & 0xFF) == 0;
    cf_ = r > 255;
    if (tf_) { write_mem(x_, (uint8_t)r); return 3; }
    a_ = (uint8_t)r;
    return 0;
}

int M37520::alu_sbc(uint8_t operand) {
    const int a = tf_ ? read_mem(x_) : a_;
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
    if (tf_) { write_mem(x_, (uint8_t)r); return 3; }
    a_ = (uint8_t)r;
    return 0;
}

#define M37520_LOGIC(NAME, EXPR)                                  \
    int M37520::alu_##NAME(uint8_t operand) {                     \
        if (tf_) {                                                \
            const uint8_t v = (uint8_t)(EXPR(read_mem(x_), operand)); \
            write_mem(x_, v);                                     \
            nz(v);                                                \
            return 3;                                             \
        }                                                         \
        a_ = (uint8_t)(EXPR(a_, operand));                        \
        nz(a_);                                                   \
        return 0;                                                 \
    }
#define OR_(l, r) ((l) | (r))
#define AND_(l, r) ((l) & (r))
#define EOR_(l, r) ((l) ^ (r))
M37520_LOGIC(ora, OR_)
M37520_LOGIC(and, AND_)
M37520_LOGIC(eor, EOR_)

int M37520::alu_lda(uint8_t operand) {
    nz(operand);
    if (tf_) { write_mem(x_, operand); return 2; }
    a_ = operand;
    return 0;
}

int M37520::alu_cmp(uint8_t operand) {
    const int t = (tf_ ? read_mem(x_) : a_) - operand;
    nf_ = (t >> 7) & 1;
    zf_ = t == 0;
    cf_ = t >= 0;
    return tf_;
}

void M37520::compare(uint8_t reg, uint8_t operand) {
    const int t = reg - operand;
    nf_ = (t >> 7) & 1;
    zf_ = t == 0;
    cf_ = t >= 0;
}

uint8_t M37520::sh_asl(uint8_t v) {
    const int n = v << 1;
    nf_ = (n & 0x80) != 0;
    zf_ = (n & 0xFF) == 0;
    cf_ = n >> 8;
    return (uint8_t)n;
}

uint8_t M37520::sh_rol(uint8_t v) {
    const int n = (v << 1) | cf_;
    nf_ = (n & 0x80) != 0;
    zf_ = (n & 0xFF) == 0;
    cf_ = n >> 8;
    return (uint8_t)n;
}

uint8_t M37520::sh_lsr(uint8_t v) {
    nf_ = 0;
    zf_ = v <= 1;
    cf_ = v & 1;
    return v >> 1;
}

uint8_t M37520::sh_ror(uint8_t v) {
    const uint8_t r = (uint8_t)((v >> 1) | (cf_ << 7));
    nf_ = cf_;
    zf_ = ((v & 0xFE) | cf_) == 0;
    cf_ = v & 1;
    return r;
}

int M37520::branch(bool taken, uint32_t op) {
    if (!taken) return 2;
    pc_ = (pc_ + (op & 0xFF) - ((op & 0x80) << 1)) & 0xFFFF;
    return 4;
}

// --- instructions -----------------------------------------------------------------
// `op` is the instruction's bytes packed big-endian (opcode first), as in
// BrickEmuPy: a 3-byte "abs" instruction is op<<16 | lo<<8 | hi.

#define ALU(NAME, FN, ZPX)                                                                \
    int M37520::op_##NAME##_ind_x(uint32_t op) { return 6 + FN(read_mem(m_ind_x(op))); }  \
    int M37520::op_##NAME##_zp(uint32_t op) { return 3 + FN(read_mem(op & 0xFF)); }       \
    int M37520::op_##NAME##_imm(uint32_t op) { return 2 + FN(op & 0xFF); }                \
    int M37520::op_##NAME##_abs(uint32_t op) { return 4 + FN(read_mem(m_abs(op))); }      \
    int M37520::op_##NAME##_ind_y(uint32_t op) { return 6 + FN(read_mem(m_ind_y(op))); }  \
    int M37520::op_##NAME##_zp_x(uint32_t op) { return ZPX + FN(read_mem(m_zp_x(op))); }  \
    int M37520::op_##NAME##_abs_y(uint32_t op) { return 5 + FN(read_mem(m_abs_y(op))); }  \
    int M37520::op_##NAME##_abs_x(uint32_t op) { return 5 + FN(read_mem(m_abs_x(op))); }

ALU(adc, alu_adc, 4)
ALU(sbc, alu_sbc, 4)
ALU(ora, alu_ora, 3)  // (BrickEmuPy: ORA zp,X is 3 cycles, the others 4)
ALU(and, alu_and, 4)
ALU(eor, alu_eor, 4)
ALU(lda, alu_lda, 4)
ALU(cmp, alu_cmp, 4)

#define SHIFT(NAME, FN)                                                                                    \
    int M37520::op_##NAME##_zp(uint32_t op) { const uint32_t a = op & 0xFF; write_mem(a, FN(read_mem(a))); return 5; } \
    int M37520::op_##NAME##_a(uint32_t) { a_ = FN(a_); return 2; }                                          \
    int M37520::op_##NAME##_abs(uint32_t op) { const uint32_t a = m_abs(op); write_mem(a, FN(read_mem(a))); return 6; } \
    int M37520::op_##NAME##_zp_x(uint32_t op) { const uint32_t a = m_zp_x(op); write_mem(a, FN(read_mem(a))); return 6; } \
    int M37520::op_##NAME##_abs_x(uint32_t op) { const uint32_t a = m_abs_x(op); write_mem(a, FN(read_mem(a))); return 7; }
SHIFT(asl, sh_asl)
SHIFT(rol, sh_rol)
SHIFT(lsr, sh_lsr)
SHIFT(ror, sh_ror)

#define INCDEC(NAME, DELTA)                                                                                \
    int M37520::op_##NAME##_zp(uint32_t op) { const uint32_t a = op & 0xFF; const uint8_t v = (uint8_t)(read_mem(a) + DELTA); write_mem(a, v); nz(v); return 5; } \
    int M37520::op_##NAME##_abs(uint32_t op) { const uint32_t a = m_abs(op); const uint8_t v = (uint8_t)(read_mem(a) + DELTA); write_mem(a, v); nz(v); return 6; } \
    int M37520::op_##NAME##_zp_x(uint32_t op) { const uint32_t a = m_zp_x(op); const uint8_t v = (uint8_t)(read_mem(a) + DELTA); write_mem(a, v); nz(v); return 6; } \
    int M37520::op_##NAME##_abs_x(uint32_t op) { const uint32_t a = m_abs_x(op); const uint8_t v = (uint8_t)(read_mem(a) + DELTA); write_mem(a, v); nz(v); return 7; }
INCDEC(inc, 1)
INCDEC(dec, -1)

int M37520::op_inc_a(uint32_t) { a_++; nz(a_); return 2; }
int M37520::op_dec_a(uint32_t) { a_--; nz(a_); return 2; }
int M37520::op_inx(uint32_t) { x_++; nz(x_); return 2; }
int M37520::op_iny(uint32_t) { y_++; nz(y_); return 2; }
int M37520::op_dex(uint32_t) { x_--; nz(x_); return 2; }
int M37520::op_dey(uint32_t) { y_--; nz(y_); return 2; }

// loads / stores / transfers
int M37520::op_ldy_imm(uint32_t op) { y_ = op & 0xFF; nz(y_); return 2; }
int M37520::op_ldx_imm(uint32_t op) { x_ = op & 0xFF; nz(x_); return 2; }
int M37520::op_ldy_zp(uint32_t op) { y_ = read_mem(op & 0xFF); nz(y_); return 3; }
int M37520::op_ldx_zp(uint32_t op) { x_ = read_mem(op & 0xFF); nz(x_); return 3; }
int M37520::op_ldy_abs(uint32_t op) { y_ = read_mem(m_abs(op)); nz(y_); return 4; }
int M37520::op_ldx_abs(uint32_t op) { x_ = read_mem(m_abs(op)); nz(x_); return 4; }
int M37520::op_ldy_zp_x(uint32_t op) { y_ = read_mem(m_zp_x(op)); nz(y_); return 4; }
int M37520::op_ldx_zp_y(uint32_t op) { x_ = read_mem(m_zp_y(op)); nz(x_); return 4; }
int M37520::op_ldy_abs_x(uint32_t op) { y_ = read_mem(m_abs_x(op)); nz(y_); return 5; }
int M37520::op_ldx_abs_y(uint32_t op) { x_ = read_mem(m_abs_y(op)); nz(x_); return 5; }
int M37520::op_ldm_zp(uint32_t op) { write_mem(op & 0xFF, (op >> 8) & 0xFF); return 4; }

int M37520::op_sta_ind_x(uint32_t op) { write_mem(m_ind_x(op), a_); return 7; }
int M37520::op_sta_zp(uint32_t op) { write_mem(op & 0xFF, a_); return 4; }
int M37520::op_sta_abs(uint32_t op) { write_mem(m_abs(op), a_); return 7; }
int M37520::op_sta_ind_y(uint32_t op) { write_mem(m_ind_y(op), a_); return 7; }
int M37520::op_sta_zp_x(uint32_t op) { write_mem(m_zp_x(op), a_); return 5; }
int M37520::op_sta_abs_y(uint32_t op) { write_mem(m_abs_y(op), a_); return 6; }
int M37520::op_sta_abs_x(uint32_t op) { write_mem(m_abs_x(op), a_); return 6; }
int M37520::op_sty_zp(uint32_t op) { write_mem(op & 0xFF, y_); return 4; }
int M37520::op_stx_zp(uint32_t op) { write_mem(op & 0xFF, x_); return 4; }
int M37520::op_sty_abs(uint32_t op) { write_mem(m_abs(op), y_); return 5; }
int M37520::op_stx_abs(uint32_t op) { write_mem(m_abs(op), x_); return 5; }
int M37520::op_sty_zp_x(uint32_t op) { write_mem(m_zp_x(op), y_); return 5; }
int M37520::op_stx_zp_y(uint32_t op) { write_mem(m_zp_y(op), x_); return 5; }

int M37520::op_tax(uint32_t) { x_ = a_; nz(x_); return 2; }
int M37520::op_tay(uint32_t) { y_ = a_; nz(y_); return 2; }
int M37520::op_txa(uint32_t) { a_ = x_; nz(a_); return 2; }
int M37520::op_tya(uint32_t) { a_ = y_; nz(a_); return 2; }
int M37520::op_tsx(uint32_t) { x_ = sp_; nz(x_); return 2; }
int M37520::op_txs(uint32_t) { sp_ = x_; return 2; }

// compares / bit tests
int M37520::op_cpx_imm(uint32_t op) { compare(x_, op & 0xFF); return 2; }
int M37520::op_cpx_zp(uint32_t op) { compare(x_, read_mem(op & 0xFF)); return 3; }
int M37520::op_cpx_abs(uint32_t op) { compare(x_, read_mem(m_abs(op))); return 4; }
int M37520::op_cpy_imm(uint32_t op) { compare(y_, op & 0xFF); return 2; }
int M37520::op_cpy_zp(uint32_t op) { compare(y_, read_mem(op & 0xFF)); return 3; }
int M37520::op_cpy_abs(uint32_t op) { compare(y_, read_mem(m_abs(op))); return 4; }

int M37520::op_bit_zp(uint32_t op) {
    const uint8_t m = read_mem(op & 0xFF);
    nf_ = m >> 7;
    vf_ = (m & 0x40) != 0;
    zf_ = (a_ & m) == 0;
    return 3;
}

int M37520::op_bit_abs(uint32_t op) {
    const uint8_t m = read_mem(m_abs(op));
    nf_ = m >> 7;
    vf_ = (m & 0x40) != 0;
    zf_ = (a_ & m) == 0;
    return 4;
}

int M37520::op_tst_zp(uint32_t op) { nz(read_mem(op & 0xFF)); return 3; }
int M37520::op_com_zp(uint32_t op) {
    const uint32_t a = op & 0xFF;
    const uint8_t v = read_mem(a) ^ 0xFF;
    write_mem(a, v);
    nz(v);
    return 5;
}
int M37520::op_rrf_zp(uint32_t op) {
    const uint32_t a = op & 0xFF;
    const uint8_t v = read_mem(a);
    write_mem(a, (uint8_t)((v >> 4) | (v << 4)));
    return 8;
}

// bit set / clear / branch on bit
int M37520::op_seb_bit_a(uint32_t op) { a_ |= 1 << ((op >> 5) & 7); return 2; }
int M37520::op_clb_bit_a(uint32_t op) { a_ &= ~(1 << ((op >> 5) & 7)); return 2; }
int M37520::op_seb_bit_zp(uint32_t op) {
    write_mem(op & 0xFF, read_mem(op & 0xFF) | (1 << ((op >> 13) & 7)));
    return 5;
}
int M37520::op_clb_bit_zp(uint32_t op) {
    write_mem(op & 0xFF, read_mem(op & 0xFF) & ~(1 << ((op >> 13) & 7)));
    return 5;
}
int M37520::op_bbs_bit_a(uint32_t op) {
    if (a_ & (1 << ((op >> 13) & 7))) { branch(true, op); return 6; }
    return 4;
}
int M37520::op_bbc_bit_a(uint32_t op) {
    if (!(a_ & (1 << ((op >> 13) & 7)))) { branch(true, op); return 6; }
    return 4;
}
int M37520::op_bbs_bit_zp(uint32_t op) {
    if (read_mem((op >> 8) & 0xFF) & (1 << ((op >> 21) & 7))) { branch(true, op); return 7; }
    return 5;
}
int M37520::op_bbc_bit_zp(uint32_t op) {
    if (!(read_mem((op >> 8) & 0xFF) & (1 << ((op >> 21) & 7)))) { branch(true, op); return 7; }
    return 5;
}

// branches
int M37520::op_bpl(uint32_t op) { return branch(!nf_, op); }
int M37520::op_bmi(uint32_t op) { return branch(nf_, op); }
int M37520::op_bvc(uint32_t op) { return branch(!vf_, op); }
int M37520::op_bvs(uint32_t op) { return branch(vf_, op); }
int M37520::op_bcc(uint32_t op) { return branch(!cf_, op); }
int M37520::op_bcs(uint32_t op) { return branch(cf_, op); }
int M37520::op_bne(uint32_t op) { return branch(!zf_, op); }
int M37520::op_beq(uint32_t op) { return branch(zf_, op); }
int M37520::op_bra(uint32_t op) { branch(true, op); return 4; }

// jumps / calls / returns
int M37520::op_jmp_abs(uint32_t op) { pc_ = m_abs(op); return 3; }
int M37520::op_jmp_ind(uint32_t op) {
    const uint32_t a = m_abs(op);
    pc_ = read_mem(a) | ((uint32_t)read_mem((a + 1) & 0xFFFF) << 8);
    return 5;
}
int M37520::op_jmp_zp_ind(uint32_t op) {
    pc_ = read_mem(op & 0xFF) | ((uint32_t)read_mem((op + 1) & 0xFF) << 8);
    return 4;
}
int M37520::op_jsr_abs(uint32_t op) {
    push((uint8_t)(pc_ >> 8));
    push((uint8_t)pc_);
    pc_ = m_abs(op);
    return 6;
}
int M37520::op_jsr_zp_ind(uint32_t op) {
    push((uint8_t)(pc_ >> 8));
    push((uint8_t)pc_);
    pc_ = read_mem(op & 0xFF) | ((uint32_t)read_mem((op + 1) & 0xFF) << 8);
    return 7;
}
int M37520::op_jsr_sp(uint32_t op) {
    push((uint8_t)(pc_ >> 8));
    push((uint8_t)pc_);
    pc_ = 0xFF00 | (op & 0xFF);
    return 5;
}
int M37520::op_rts(uint32_t) {
    pc_ = pull();
    pc_ |= (uint32_t)pull() << 8;
    return 6;
}
int M37520::op_rti(uint32_t) {
    set_ps(pull());
    pc_ = pull();
    pc_ |= (uint32_t)pull() << 8;
    return 6;
}
int M37520::op_brk(uint32_t) {
    bf_ = 1;
    pc_ = (pc_ + 1) & 0xFFFF;
    interrupt(kVectorBrk);
    return 7;
}

// stack
int M37520::op_pha(uint32_t) { push(a_); return 3; }
int M37520::op_php(uint32_t) { push(ps()); return 3; }
int M37520::op_pla(uint32_t) { a_ = pull(); nz(a_); return 4; }
int M37520::op_plp(uint32_t) { set_ps(pull()); return 4; }

// flags / control
int M37520::op_clt(uint32_t) { tf_ = 0; return 2; }
int M37520::op_set(uint32_t) { tf_ = 1; return 2; }
int M37520::op_clc(uint32_t) { cf_ = 0; return 2; }
int M37520::op_sec(uint32_t) { cf_ = 1; return 2; }
int M37520::op_cli(uint32_t) { if_ = 0; return 2; }
int M37520::op_sei(uint32_t) { if_ = 1; return 2; }
int M37520::op_cld(uint32_t) { df_ = 0; return 2; }
int M37520::op_sed(uint32_t) { df_ = 1; return 2; }
int M37520::op_clv(uint32_t) { vf_ = 0; return 2; }
int M37520::op_nop(uint32_t) { return 2; }
int M37520::op_dummy(uint32_t) { return 2; }
int M37520::op_wit(uint32_t) { nwait_ = 0; return 2; }
int M37520::op_stp(uint32_t) { nstop_ = 0; return 2; }

// multiply / divide: the high byte / remainder goes on the stack
int M37520::op_mul_zp_x(uint32_t op) {
    const uint32_t r = (uint32_t)a_ * read_mem(m_zp_x(op));
    push((uint8_t)(r >> 8));
    a_ = (uint8_t)r;
    return 15;
}
int M37520::op_div_zp_x(uint32_t op) {
    const uint32_t dividend = read_mem((op + x_) & 0xFF) | ((uint32_t)read_mem((op + x_ + 1) & 0xFF) << 8);
    if (a_) {
        push((uint8_t)(dividend % a_));
        a_ = (uint8_t)(dividend / a_);
    }
    return 16;
}

// --- save states ---------------------------------------------------------------------

void M37520::serialize(BrickStateIO& io) {
    io.pod(pc_);
    io.pod(a_); io.pod(x_); io.pod(y_); io.pod(sp_);
    uint8_t flags[10] = {nf_, vf_, tf_, bf_, df_, if_, zf_, cf_, nwait_, nstop_};
    io.bytes(flags, sizeof(flags));
    nf_ = flags[0]; vf_ = flags[1]; tf_ = flags[2]; bf_ = flags[3]; df_ = flags[4];
    if_ = flags[5]; zf_ = flags[6]; cf_ = flags[7]; nwait_ = flags[8]; nstop_ = flags[9];
    io.vec(ram_);
    io.vec(lcd_);
    io.bytes(pdir_, sizeof(pdir_));
    io.bytes(platch_, sizeof(platch_));
    io.bytes(pullup_, sizeof(pullup_));
    io.bytes(in_low_, sizeof(in_low_));
    io.bytes(in_high_, sizeof(in_high_));
    io.pod(tx_); io.pod(tx_latch_); io.pod(ty_); io.pod(ty_latch_);
    io.pod(t1_); io.pod(t1_latch_); io.pod(t2_); io.pod(t2_latch_); io.pod(t3_); io.pod(t3_latch_);
    io.pod(tx_counter_); io.pod(ty_counter_); io.pod(t1_counter_); io.pod(t2_counter_); io.pod(t3_counter_);
    uint8_t regs[9] = {txm_, tym_, t123m_, lc_, lm_, intedge_, cpum_, ireq1_, ireq2_};
    io.bytes(regs, sizeof(regs));
    txm_ = regs[0]; tym_ = regs[1]; t123m_ = regs[2]; lc_ = regs[3]; lm_ = regs[4];
    intedge_ = regs[5]; cpum_ = regs[6]; ireq1_ = regs[7]; ireq2_ = regs[8];
    io.pod(icon1_);
    io.pod(icon2_);
    // Derived state.
    set_lm(lm_);
    set_cpum(cpum_);
}
