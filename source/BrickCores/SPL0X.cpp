#include "SPL0X.h"

#include <cstring>

namespace {

constexpr uint32_t kVectorNmi = 0x1FFA, kVectorReset = 0x1FFC, kVectorIrq = 0x1FFE;
constexpr uint32_t kSfrOffset = 0xC0, kSfrSize = 0x40, kRamOffset = 0x30, kLcdSize = 0x30;
constexpr int64_t kSubClock = 32768;

constexpr uint8_t kIoCtrlAlDir = 0x01, kIoCtrlAhDir = 0x02, kIoCtrlBDir = 0x04, kIoCtrlRosc = 0x10;
constexpr uint8_t kIntT2Hz = 0x01, kIntT256Hz = 0x02, kIntPowerKey = 0x04, kIntNmiEnable = 0x80;
constexpr uint8_t kSysCtrl32kEnable = 0x20, kSysCtrlCpuStop = 0x40, kSysCtrlRoscStop = 0x80;
constexpr uint8_t kSpeechEnable = 0x01;

}  // namespace

const SPL0X::OpEntry SPL0X::kOps[256] = {
    {&SPL0X::op_brk, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_ora_zp, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_php, 1}, {&SPL0X::op_ora_imm, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bpl, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_clc, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_jsr_abs, 3}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bit_zp, 2}, {&SPL0X::op_and_zp, 2}, {&SPL0X::op_rol_zp, 2}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_plp, 1}, {&SPL0X::op_and_imm, 2}, {&SPL0X::op_rol_a, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bit_abs, 3}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bmi, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_sec, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_rti, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_eor_zp, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_pha, 1}, {&SPL0X::op_eor_imm, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_jmp_abs, 3}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bvc, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_eor_zp_x, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_cli, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_rts, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_adc_zp, 2}, {&SPL0X::op_ror_zp, 2}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_pla, 1}, {&SPL0X::op_adc_imm, 2}, {&SPL0X::op_ror_a, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_jmp_ind, 3}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bvs, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_sei, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_sta_ind_x, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_sta_zp, 2}, {&SPL0X::op_stx_zp, 2}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_txa, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_stx_abs, 3}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bcc, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_sta_zp_x, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_txs, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_lda_ind_x, 2}, {&SPL0X::op_ldx_imm, 2}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_lda_zp, 2}, {&SPL0X::op_ldx_zp, 2}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_lda_imm, 2}, {&SPL0X::op_tax, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_lda_abs, 3}, {&SPL0X::op_ldx_abs, 3}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bcs, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_lda_zp_x, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_clv, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_tsx, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_lda_abs_x, 3}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_cmp_zp, 2}, {&SPL0X::op_dec_zp, 2}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_cmp_imm, 2}, {&SPL0X::op_dex, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_bne, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_cmp_zp_x, 2}, {&SPL0X::op_dec_zp_x, 2}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_cpx_imm, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_cpx_zp, 2}, {&SPL0X::op_sbc_zp, 2}, {&SPL0X::op_inc_zp, 2}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_inx, 1}, {&SPL0X::op_sbc_imm, 2}, {&SPL0X::op_nop, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_beq, 2}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_sed, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
    {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1}, {&SPL0X::op_dummy, 1},
};

// --- reset / ports --------------------------------------------------------------

void SPL0X::reset() {
    if (!options_loaded_) {
        static const char* kNames[2] = {"PA", "PB"};
        for (int i = 0; i < 2; i++) {
            pullup_ext_[i] = (uint8_t)options().num(std::string("port_pullup.") + kNames[i], 0);
            pkey_[i] = (uint8_t)options().num(std::string("port_pkey.") + kNames[i], i == 0 ? 1 : 0);
        }
        const int64_t div = (int64_t)options().num("non_crystal_div", 0);
        if (div > 0) {
            sub_div_ = div * (int64_t)kTickHz;
            freq_factor_ = (double)clock_hz() / (double)div / (double)kSubClock;
        } else {
            sub_div_ = (int64_t)clock_hz();     // clock/32768 cycles, scaled
            freq_factor_ = 1.0;
        }
        options_loaded_ = true;
    }
    // (The port inputs survive a reset; the timers' counters do not.)
    t2hz_counter_ = t256hz_counter_ = 0;
    pc_ = 0;
    sp_ = 0;
    a_ = x_ = 0;
    set_ps(0x04);
    cpu_enbl_ = rosc_enbl_ = 1;
    ram_.assign(ram_size_, 0);
    lcd_.assign(kLcdSize, 0);
    rom_bank_ = 0;
    memset(pdir_, 0, sizeof(pdir_));
    memset(platch_, 0, sizeof(platch_));
    io_ctrl_ = int_cfg_ = ireq_ = sys_ctrl_ = speech_ctrl_ = speech_data_ = 0;
    tonea1_ = tonea2_ = toneb1_ = toneb2_ = noise1_ = noise2_ = 0;
    tone_hz_[0] = tone_hz_[1] = noise_hz_ = 0;
    sound_mix();
    set_speaker(false);
    go_vector(kVectorReset);
}

void SPL0X::go_vector(uint32_t addr) {
    const int64_t at = (int64_t)addr + ((int64_t)rom_bank_ << 12);
    pc_ = (uint32_t)(rom_byte(at) | (rom_byte(at + 1) << 8));
}

uint8_t SPL0X::display_byte(int index) const {
    return (index >= 0 && index < (int)kLcdSize) ? lcd_[index] : 0;
}

int SPL0X::port_id(const std::string& name) const {
    if (name == "PA") return 0;
    if (name == "PB") return 1;
    return -1;
}

uint8_t SPL0X::port_read(int port) const {
    return (uint8_t)((~pdir_[port] & platch_[port]) | (pdir_[port] & (~in_low_[port] & (in_high_[port] | pullup_ext_[port]))));
}

void SPL0X::port_input(int port, uint8_t mask, int level) {
    if (port < 0 || port > 1) return;
    const uint8_t prev = port_read(port);
    in_low_[port] &= ~mask;
    in_high_[port] &= ~mask;
    if (level == 0) in_low_[port] |= mask;
    else if (level > 0) in_high_[port] |= mask;
    if ((prev & pkey_[port]) < (port_read(port) & pkey_[port])) {
        if (int_cfg_ & kIntPowerKey) {
            ireq_ |= kIntPowerKey;
            nmi();
        }
    }
}

// --- interrupts and timers ----------------------------------------------------------

void SPL0X::irq() {
    push((uint8_t)(pc_ >> 8));
    push((uint8_t)pc_);
    push(ps());
    if_ = 1;
    go_vector(kVectorIrq);
}

void SPL0X::nmi() {
    if (!(int_cfg_ & kIntNmiEnable)) return;
    if (rosc_enbl_ && cpu_enbl_) {
        push((uint8_t)(pc_ >> 8));
        push((uint8_t)pc_);
        push(ps());
        go_vector(kVectorNmi);
    } else {
        cpu_enbl_ = rosc_enbl_ = 1;   // wake from stop through the reset vector
        go_vector(kVectorReset);
    }
}

void SPL0X::timers_clock(int64_t e) {
    t2hz_counter_ -= e;
    while (t2hz_counter_ <= 0) {
        t2hz_counter_ += sub_div_ * (kSubClock / 2);
        if (int_cfg_ & kIntT2Hz) {
            ireq_ |= kIntT2Hz;
            nmi();
        }
    }
    t256hz_counter_ -= e;
    while (t256hz_counter_ <= 0) {
        t256hz_counter_ += sub_div_ * (kSubClock / 256);
        if (int_cfg_ & kIntT256Hz) {
            ireq_ |= kIntT256Hz;
            nmi();
        }
    }
}

int64_t SPL0X::execute_scaled() {
    int64_t exec = (int64_t)kTickHz;   // MCLOCK_DIV = 1 cycle
    if (rosc_enbl_) {
        if (cpu_enbl_) {
            const int64_t at = get_pc();
            const OpEntry& e = kOps[rom_byte(at)];
            uint32_t op = 0;
            for (int i = 0; i < e.len; i++) op = (op << 8) | rom_byte(at + i);
            pc_ += e.len;
            exec = (int64_t)(this->*e.fn)(op) * (int64_t)kTickHz;
        }
        timers_clock(exec);
    } else if (sys_ctrl_ & kSysCtrl32kEnable) {
        exec = sub_div_;
        timers_clock(exec);
    }
    cycle_counter_ += exec / (int64_t)kTickHz;
    return exec;
}

// --- memory and I/O ---------------------------------------------------------------------

bool SPL0X::io_present(uint8_t reg) const {
    switch (reg) {
        case 0xC0: case 0xC1: case 0xC3: case 0xC4: case 0xC6: case 0xCC: case 0xCE:
        case 0xD0: case 0xD2: case 0xD4: case 0xD5: case 0xD7:
            return true;
        case 0xC8: case 0xCA:
            return has_tone_b_;
        default:
            return false;
    }
}

uint8_t SPL0X::io_read(uint8_t reg) {
    switch (reg) {
        case 0xC0: return io_ctrl_;
        case 0xC1: return port_read(0);
        case 0xC3: return port_read(1);
        case 0xD0: return sys_ctrl_;
        case 0xD2: {   // reading the interrupt register clears the requests
            const uint8_t buf = ireq_ | (int_cfg_ & 0x80);
            ireq_ = 0;
            return buf;
        }
        case 0xD4: return speech_ctrl_;
        case 0xD5: return (uint8_t)((cycle_counter_ >> 3) & 1);
        case 0xD7: return (uint8_t)rom_bank_;
        default: return 0;   // the tone / noise registers are write-only
    }
}

void SPL0X::io_write(uint8_t reg, uint8_t v) {
    switch (reg) {
        case 0xC0:
            if (v & kIoCtrlAlDir) pdir_[0] |= 0x0F;
            if (v & kIoCtrlAhDir) pdir_[0] |= 0xF0;
            if (v & kIoCtrlBDir) pdir_[1] |= 0x03;
            rosc_enbl_ = ((io_ctrl_ | ~v) & kIoCtrlRosc) != 0;
            io_ctrl_ = v;
            break;
        case 0xC1: platch_[0] = v; port_output(0, (uint8_t)(~pdir_[0] & platch_[0])); break;
        case 0xC3: platch_[1] = v; port_output(1, (uint8_t)(~pdir_[1] & platch_[1])); break;
        case 0xC4: tonea1_ = v; sound_tone(0, tonea1_, tonea2_); break;
        case 0xC6: tonea2_ = v; sound_tone(0, tonea1_, tonea2_); break;
        case 0xC8: toneb1_ = v; sound_tone(1, toneb1_, toneb2_); break;
        case 0xCA: toneb2_ = v; sound_tone(1, toneb1_, toneb2_); break;
        case 0xCC: noise1_ = v; sound_noise(noise1_, noise2_); break;
        case 0xCE: noise2_ = v; sound_noise(noise1_, noise2_); break;
        case 0xD0:
            rosc_enbl_ = ((sys_ctrl_ | ~v) & kSysCtrlRoscStop) != 0;
            cpu_enbl_ = ((sys_ctrl_ | ~v) & kSysCtrlCpuStop) != 0;
            sys_ctrl_ = v;
            break;
        case 0xD2: int_cfg_ = v; break;
        case 0xD4: speech_ctrl_ = v; sound_speech(); break;
        case 0xD5: speech_data_ = v & speech_mask_; sound_speech(); break;
        case 0xD7: rom_bank_ = v & bank_mask_; break;
        default: break;
    }
}

uint8_t SPL0X::read_mem(uint32_t addr) {
    if (addr < kLcdSize) return lcd_[addr];
    if (addr >= kRamOffset && addr < kRamOffset + ram_size_) return ram_[addr - kRamOffset];
    if (addr >= kSfrOffset && addr < kSfrOffset + kSfrSize) return io_present((uint8_t)addr) ? io_read((uint8_t)addr) : 0;
    if (addr >= 0x1000) addr = (addr % 0x2000) + ((uint32_t)rom_bank_ << 12);
    return rom_byte(addr);
}

void SPL0X::write_mem(uint32_t addr, uint8_t value) {
    if (addr < kLcdSize) lcd_[addr] = value;
    else if (addr >= kRamOffset && addr < kRamOffset + ram_size_) ram_[addr - kRamOffset] = value;
    else if (addr < 0x100 && io_present((uint8_t)addr)) io_write((uint8_t)addr, value);
}

// --- sound ------------------------------------------------------------------------------------

void SPL0X::sound_tone(int which, uint8_t c1, uint8_t c2) {
    const double vol = (c2 >> 6) / 3.0;
    const int freq32 = (((c2 & 0x3) << 8) | c1) << 1;
    tone_hz_[which] = (vol > 0 && freq32 > 0) ? freq_factor_ * freq32 : 0;
    sound_mix();
}

void SPL0X::sound_noise(uint8_t c1, uint8_t c2) {
    const double vol = (c2 >> 4) / 15.0;
    const int freq32 = (16 - (c1 & 0xF)) << 7;
    noise_hz_ = (vol > 0 && freq32 > 0) ? freq_factor_ * freq32 : 0;
    sound_mix();
}

void SPL0X::sound_mix() {
    if (tone_hz_[0] > 0) set_tone(tone_hz_[0], false);
    else if (tone_hz_[1] > 0) set_tone(tone_hz_[1], false);
    else if (noise_hz_ > 0) set_tone(noise_hz_, true);
    else set_tone(0);
}

void SPL0X::sound_speech() {
    double amplitude = ((speech_data_ & 0x3F) / 63.0);
    if (speech_data_ & 0x80) amplitude = 0;
    if (speech_data_ == 0 || !(speech_ctrl_ & kSpeechEnable)) set_speaker(false);
    else set_speaker(amplitude > 0.5);   // the DAC level as a 1-bit speaker
}

// --- CPU core -------------------------------------------------------------------------------------

uint8_t SPL0X::ps() const {
    return (uint8_t)((nf_ << 7) | (vf_ << 6) | (bf_ << 4) | (df_ << 3) | (if_ << 2) | (zf_ << 1) | cf_);
}

void SPL0X::set_ps(uint8_t p) {
    nf_ = p >> 7;
    vf_ = (p & 0x40) != 0;
    bf_ = (p & 0x10) != 0;
    df_ = (p & 0x08) != 0;
    if_ = (p & 0x04) != 0;
    zf_ = (p & 0x02) != 0;
    cf_ = p & 0x01;
}

void SPL0X::adc(uint8_t operand) {
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

void SPL0X::sbc(uint8_t operand) {
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

void SPL0X::compare(uint8_t reg, uint8_t operand) {
    const int t = reg - operand;
    nf_ = (t >> 7) & 1;
    zf_ = t == 0;
    cf_ = t >= 0;
}

int SPL0X::branch(bool taken, uint32_t op) {
    if (!taken) return 2;
    const uint32_t prev = pc_;
    pc_ = (pc_ + (op & 0xFF) - ((op & 0x80) << 1)) & 0xFFFF;
    return 3 + (((pc_ ^ prev) > 255) ? 1 : 0);
}

// --- instructions (`op` = the bytes packed big-endian, opcode first) -----------------------------------
int SPL0X::op_dummy(uint32_t) { return 2; }   // illegal opcode (BrickEmuPy raises)
int SPL0X::op_nop(uint32_t) { return 2; }
int SPL0X::op_brk(uint32_t) { bf_ = 1; pc_ = (pc_ + 1) & 0xFFFF; irq(); return 7; }

int SPL0X::op_ora_zp(uint32_t op) { a_ |= read_mem(op & 0xFF); nz(a_); return 3; }
int SPL0X::op_ora_imm(uint32_t op) { a_ |= op & 0xFF; nz(a_); return 2; }
int SPL0X::op_and_zp(uint32_t op) { a_ &= read_mem(op & 0xFF); nz(a_); return 3; }
int SPL0X::op_and_imm(uint32_t op) { a_ &= op & 0xFF; nz(a_); return 2; }
int SPL0X::op_eor_zp(uint32_t op) { a_ ^= read_mem(op & 0xFF); nz(a_); return 3; }
int SPL0X::op_eor_imm(uint32_t op) { a_ ^= op & 0xFF; nz(a_); return 2; }
int SPL0X::op_eor_zp_x(uint32_t op) { a_ ^= read_mem(m_zp_x(op)); nz(a_); return 4; }
int SPL0X::op_adc_zp(uint32_t op) { adc(read_mem(op & 0xFF)); return 3; }
int SPL0X::op_adc_imm(uint32_t op) { adc(op & 0xFF); return 2; }
int SPL0X::op_sbc_zp(uint32_t op) { sbc(read_mem(op & 0xFF)); return 3; }
int SPL0X::op_sbc_imm(uint32_t op) { sbc(op & 0xFF); return 2; }

int SPL0X::op_php(uint32_t) { push(ps()); return 3; }
int SPL0X::op_pha(uint32_t) { push(a_); return 3; }
int SPL0X::op_plp(uint32_t) { set_ps(pull()); return 4; }
int SPL0X::op_pla(uint32_t) { a_ = pull(); nz(a_); return 4; }

int SPL0X::op_bpl(uint32_t op) { return branch(!nf_, op); }
int SPL0X::op_bmi(uint32_t op) { return branch(nf_, op); }
int SPL0X::op_bvc(uint32_t op) { return branch(!vf_, op); }
int SPL0X::op_bvs(uint32_t op) { return branch(vf_, op); }
int SPL0X::op_bcc(uint32_t op) { return branch(!cf_, op); }
int SPL0X::op_bcs(uint32_t op) { return branch(cf_, op); }
int SPL0X::op_bne(uint32_t op) { return branch(!zf_, op); }
int SPL0X::op_beq(uint32_t op) { return branch(zf_, op); }

int SPL0X::op_clc(uint32_t) { cf_ = 0; return 2; }
int SPL0X::op_sec(uint32_t) { cf_ = 1; return 2; }
int SPL0X::op_cli(uint32_t) { if_ = 0; return 2; }
int SPL0X::op_sei(uint32_t) { if_ = 1; return 2; }
int SPL0X::op_clv(uint32_t) { vf_ = 0; return 2; }
int SPL0X::op_sed(uint32_t) { df_ = 1; return 2; }

int SPL0X::op_jsr_abs(uint32_t op) {
    const uint32_t pc = pc_ - 1;
    push((uint8_t)((pc >> 8) & 0xFF));
    push((uint8_t)(pc & 0xFF));
    pc_ = m_abs(op);
    return 6;
}
int SPL0X::op_jmp_abs(uint32_t op) { pc_ = m_abs(op); return 3; }
int SPL0X::op_jmp_ind(uint32_t op) {
    const uint32_t a = m_abs(op);
    pc_ = read_mem(a) | ((uint32_t)read_mem((a + 1) & 0xFFFF) << 8);
    return 6;
}
int SPL0X::op_rts(uint32_t) {
    pc_ = pull();
    pc_ |= (uint32_t)pull() << 8;
    pc_ += 1;
    return 6;
}
int SPL0X::op_rti(uint32_t) {
    set_ps(pull());
    pc_ = pull();
    pc_ |= (uint32_t)pull() << 8;
    return 6;
}

int SPL0X::op_bit_zp(uint32_t op) {
    const uint8_t m = read_mem(op & 0xFF);
    nf_ = m >> 7; vf_ = (m & 0x40) != 0; zf_ = (a_ & m) == 0;
    return 3;
}
int SPL0X::op_bit_abs(uint32_t op) {
    const uint8_t m = read_mem(m_abs(op));
    nf_ = m >> 7; vf_ = (m & 0x40) != 0; zf_ = (a_ & m) == 0;
    return 4;
}

int SPL0X::op_rol_zp(uint32_t op) {
    const uint32_t a = op & 0xFF;
    const int n = (read_mem(a) << 1) | cf_;
    write_mem(a, (uint8_t)n);
    nf_ = (n & 0x80) != 0; zf_ = (n & 0xFF) == 0; cf_ = n > 0xFF;
    return 5;
}
int SPL0X::op_rol_a(uint32_t) {
    const int n = (a_ << 1) | cf_;
    a_ = (uint8_t)n;
    nf_ = (n & 0x80) != 0; zf_ = (n & 0xFF) == 0; cf_ = n > 0xFF;
    return 2;
}
int SPL0X::op_ror_zp(uint32_t op) {
    const uint32_t a = op & 0xFF;
    const uint8_t prev = read_mem(a);
    write_mem(a, (uint8_t)((prev >> 1) | (cf_ << 7)));
    nf_ = cf_; zf_ = ((prev & 0xFE) | cf_) == 0; cf_ = prev & 1;
    return 5;
}
int SPL0X::op_ror_a(uint32_t) {
    const uint8_t prev = a_;
    a_ = (uint8_t)((prev >> 1) | (cf_ << 7));
    nf_ = cf_; zf_ = ((prev & 0xFE) | cf_) == 0; cf_ = prev & 1;
    return 2;
}

int SPL0X::op_inc_zp(uint32_t op) { const uint32_t a = op & 0xFF; const uint8_t v = read_mem(a) + 1; write_mem(a, v); nz(v); return 5; }
int SPL0X::op_dec_zp(uint32_t op) { const uint32_t a = op & 0xFF; const uint8_t v = read_mem(a) - 1; write_mem(a, v); nz(v); return 5; }
int SPL0X::op_dec_zp_x(uint32_t op) { const uint32_t a = m_zp_x(op); const uint8_t v = read_mem(a) - 1; write_mem(a, v); nz(v); return 6; }
int SPL0X::op_inx(uint32_t) { x_++; nz(x_); return 2; }
int SPL0X::op_dex(uint32_t) { x_--; nz(x_); return 2; }

int SPL0X::op_sta_ind_x(uint32_t op) { write_mem(m_ind_x(op), a_); return 6; }
int SPL0X::op_sta_zp(uint32_t op) { write_mem(op & 0xFF, a_); return 3; }
int SPL0X::op_sta_zp_x(uint32_t op) { write_mem(m_zp_x(op), a_); return 4; }
int SPL0X::op_stx_zp(uint32_t op) { write_mem(op & 0xFF, x_); return 3; }
int SPL0X::op_stx_abs(uint32_t op) { write_mem(m_abs(op), x_); return 4; }

int SPL0X::op_lda_ind_x(uint32_t op) { a_ = read_mem(m_ind_x(op)); nz(a_); return 6; }
int SPL0X::op_lda_zp(uint32_t op) { a_ = read_mem(op & 0xFF); nz(a_); return 3; }
int SPL0X::op_lda_imm(uint32_t op) { a_ = op & 0xFF; nz(a_); return 2; }
int SPL0X::op_lda_abs(uint32_t op) { a_ = read_mem(m_abs(op)); nz(a_); return 4; }
int SPL0X::op_lda_zp_x(uint32_t op) { a_ = read_mem(m_zp_x(op)); nz(a_); return 4; }
int SPL0X::op_lda_abs_x(uint32_t op) {
    const uint32_t addr = (m_abs(op) + x_) & 0xFFFF;
    a_ = read_mem(addr);
    nz(a_);
    return 4 + (((pc_ ^ addr) > 255) ? 1 : 0);
}
int SPL0X::op_ldx_imm(uint32_t op) { x_ = op & 0xFF; nz(x_); return 2; }
int SPL0X::op_ldx_zp(uint32_t op) { x_ = read_mem(op & 0xFF); nz(x_); return 3; }
int SPL0X::op_ldx_abs(uint32_t op) { x_ = read_mem(m_abs(op)); nz(x_); return 4; }

int SPL0X::op_tax(uint32_t) { x_ = a_; nz(x_); return 2; }
int SPL0X::op_txa(uint32_t) { a_ = x_; nz(a_); return 2; }
int SPL0X::op_tsx(uint32_t) { x_ = sp_; nz(x_); return 2; }
int SPL0X::op_txs(uint32_t) { sp_ = x_; return 2; }

int SPL0X::op_cmp_zp(uint32_t op) { compare(a_, read_mem(op & 0xFF)); return 3; }
int SPL0X::op_cmp_imm(uint32_t op) { compare(a_, op & 0xFF); return 2; }
int SPL0X::op_cmp_zp_x(uint32_t op) { compare(a_, read_mem(m_zp_x(op))); return 4; }
int SPL0X::op_cpx_imm(uint32_t op) { compare(x_, op & 0xFF); return 2; }
int SPL0X::op_cpx_zp(uint32_t op) { compare(x_, read_mem(op & 0xFF)); return 3; }

// --- save states --------------------------------------------------------------------------------------------

void SPL0X::serialize(BrickStateIO& io) {
    io.pod(pc_);
    io.pod(a_); io.pod(x_); io.pod(sp_);
    uint8_t flags[9] = {nf_, vf_, bf_, df_, if_, zf_, cf_, cpu_enbl_, rosc_enbl_};
    io.bytes(flags, sizeof(flags));
    nf_ = flags[0]; vf_ = flags[1]; bf_ = flags[2]; df_ = flags[3]; if_ = flags[4];
    zf_ = flags[5]; cf_ = flags[6]; cpu_enbl_ = flags[7]; rosc_enbl_ = flags[8];
    io.pod(rom_bank_);
    io.vec(ram_);
    io.vec(lcd_);
    io.pod(cycle_counter_);
    io.pod(t2hz_counter_);
    io.pod(t256hz_counter_);
    io.bytes(pdir_, sizeof(pdir_));
    io.bytes(platch_, sizeof(platch_));
    io.bytes(in_low_, sizeof(in_low_));
    io.bytes(in_high_, sizeof(in_high_));
    uint8_t regs[12] = {io_ctrl_, int_cfg_, ireq_, sys_ctrl_, speech_ctrl_, speech_data_,
                        tonea1_, tonea2_, toneb1_, toneb2_, noise1_, noise2_};
    io.bytes(regs, sizeof(regs));
    io_ctrl_ = regs[0]; int_cfg_ = regs[1]; ireq_ = regs[2]; sys_ctrl_ = regs[3]; speech_ctrl_ = regs[4];
    speech_data_ = regs[5]; tonea1_ = regs[6]; tonea2_ = regs[7]; toneb1_ = regs[8]; toneb2_ = regs[9];
    noise1_ = regs[10]; noise2_ = regs[11];
    // Derived state: the sound channels follow the registers.
    sound_tone(0, tonea1_, tonea2_);
    sound_tone(1, toneb1_, toneb2_);
    sound_noise(noise1_, noise2_);
    sound_speech();
}
