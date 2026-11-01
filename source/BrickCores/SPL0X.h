#pragma once

#include "BrickCore.h"

// Sunplus SPL0X family (a 6502 subset: 70 opcodes, 13-bit address space with a
// 4 KB ROM bank window, 2 Hz / 256 Hz NMI timers, ports PA/PB, tone + noise + speech
// DAC) - a port of BrickEmuPy's cores/SPL0X.py with its two variants SPL02
// (Apollo 126 in 1 B0202) and SPL03 (Apollo 18 in 1 B0302).
//
// Memory map: LCD RAM 0x00-0x2F (the display is these bytes), CPU RAM from 0x30
// (0x90 bytes on SPL02, 0x50 on SPL03; the stack lives in it, SP is an 8-bit
// address), SFRs 0xC0-0xFF, ROM above (addresses >= 0x1000 go through the bank
// window: (addr % 0x2000) + (ROM_BANK << 12), all modulo the ROM size).
//
// Sound: BrickEmuPy mixes tone A, tone B, noise and a speech DAC. The Yokoi
// audio engine has one tone and a 1-bit speaker, so the first running channel of
// tone A / tone B / noise plays as the tone, and the speech DAC level is the
// speaker pin (level > 50 %): effects play, voice is crude.
//
// Time is cycles * kTickHz ("scaled"): the sub clock is non_crystal_div cycles
// (or clock/32768 when that option is 0), so nothing is lost to rounding.
class SPL0X : public BrickCore {
protected:
    SPL0X(const char* name, uint32_t cpu_ram_size, bool has_tone_b, uint8_t speech_data_mask, uint8_t bank_mask)
        : BrickCore(name), ram_size_(cpu_ram_size), has_tone_b_(has_tone_b), speech_mask_(speech_data_mask),
          bank_mask_(bank_mask) {}

    void reset() override;
    int execute() override { return (int)(execute_scaled() / kTickHz); }
    int64_t execute_scaled() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;

private:
#define SPL0X_OPS(X) \
    X(adc_imm) \
    X(adc_zp) \
    X(and_imm) \
    X(and_zp) \
    X(bcc) \
    X(bcs) \
    X(beq) \
    X(bit_abs) \
    X(bit_zp) \
    X(bmi) \
    X(bne) \
    X(bpl) \
    X(brk) \
    X(bvc) \
    X(bvs) \
    X(clc) \
    X(cli) \
    X(clv) \
    X(cmp_imm) \
    X(cmp_zp) \
    X(cmp_zp_x) \
    X(cpx_imm) \
    X(cpx_zp) \
    X(dec_zp) \
    X(dec_zp_x) \
    X(dex) \
    X(dummy) \
    X(eor_imm) \
    X(eor_zp) \
    X(eor_zp_x) \
    X(inc_zp) \
    X(inx) \
    X(jmp_abs) \
    X(jmp_ind) \
    X(jsr_abs) \
    X(lda_abs) \
    X(lda_abs_x) \
    X(lda_imm) \
    X(lda_ind_x) \
    X(lda_zp) \
    X(lda_zp_x) \
    X(ldx_abs) \
    X(ldx_imm) \
    X(ldx_zp) \
    X(nop) \
    X(ora_imm) \
    X(ora_zp) \
    X(pha) \
    X(php) \
    X(pla) \
    X(plp) \
    X(rol_a) \
    X(rol_zp) \
    X(ror_a) \
    X(ror_zp) \
    X(rti) \
    X(rts) \
    X(sbc_imm) \
    X(sbc_zp) \
    X(sec) \
    X(sed) \
    X(sei) \
    X(sta_ind_x) \
    X(sta_zp) \
    X(sta_zp_x) \
    X(stx_abs) \
    X(stx_zp) \
    X(tax) \
    X(tsx) \
    X(txa) \
    X(txs)


    typedef int (SPL0X::*OpFn)(uint32_t);
    struct OpEntry { OpFn fn; uint8_t len; };
    static const OpEntry kOps[256];
#define SPL0X_DECL(n) int op_##n(uint32_t op);
    SPL0X_OPS(SPL0X_DECL)
#undef SPL0X_DECL

    // memory / I/O
    uint8_t rom_byte(int64_t addr) const {
        const int64_t n = (int64_t)rom().size();
        return rom()[(size_t)(((addr % n) + n) % n)];
    }
    uint32_t get_pc() const { return pc_ > 0xFFF ? (pc_ % 0x2000) + ((uint32_t)rom_bank_ << 12) : pc_; }
    uint8_t read_mem(uint32_t addr);
    void write_mem(uint32_t addr, uint8_t value);
    bool io_present(uint8_t reg) const;
    uint8_t io_read(uint8_t reg);
    void io_write(uint8_t reg, uint8_t value);
    uint8_t port_read(int port) const;

    // CPU
    uint8_t ps() const;
    void set_ps(uint8_t p);
    void push(uint8_t v) { write_mem(sp_, v); sp_ = (uint8_t)(sp_ - 1); }
    uint8_t pull() { sp_ = (uint8_t)(sp_ + 1); return read_mem(sp_); }
    void go_vector(uint32_t addr);
    void irq();
    void nmi();
    void timers_clock(int64_t scaled_cycles);
    void nz(uint8_t v) { nf_ = v >> 7; zf_ = v == 0; }
    int branch(bool taken, uint32_t op);
    void adc(uint8_t operand);
    void sbc(uint8_t operand);
    void compare(uint8_t reg, uint8_t operand);

    static uint32_t m_abs(uint32_t op) { return ((op & 0xFF) << 8) | ((op >> 8) & 0xFF); }
    uint32_t m_zp_x(uint32_t op) const { return (op + x_) & 0xFF; }
    uint32_t m_ind_x(uint32_t op) {
        return read_mem((op + x_) & 0xFF) | ((uint32_t)read_mem((op + x_ + 1) & 0xFF) << 8);
    }

    // sound
    void sound_tone(int which, uint8_t ctrl1, uint8_t ctrl2);
    void sound_noise(uint8_t ctrl1, uint8_t ctrl2);
    void sound_speech();
    void sound_mix();

    // variant parameters
    uint32_t ram_size_;
    bool has_tone_b_;
    uint8_t speech_mask_, bank_mask_;

    // mask options
    bool options_loaded_ = false;
    uint8_t pullup_ext_[2] = {}, pkey_[2] = {1, 0};
    int64_t sub_div_ = 16 * (int64_t)kTickHz;   // scaled cycles per 32768 Hz sub-clock tick
    double freq_factor_ = 1.0;

    // CPU state
    uint32_t pc_ = 0;
    uint8_t a_ = 0, x_ = 0, sp_ = 0;
    uint8_t nf_ = 0, vf_ = 0, bf_ = 0, df_ = 0, if_ = 1, zf_ = 0, cf_ = 0;
    uint8_t cpu_enbl_ = 1, rosc_enbl_ = 1;
    int rom_bank_ = 0;
    std::vector<uint8_t> ram_, lcd_ = std::vector<uint8_t>(0x30, 0);
    int64_t cycle_counter_ = 0;
    int64_t t2hz_counter_ = 0, t256hz_counter_ = 0;

    // I/O registers
    uint8_t pdir_[2] = {}, platch_[2] = {}, in_low_[2] = {}, in_high_[2] = {};
    uint8_t io_ctrl_ = 0, int_cfg_ = 0, ireq_ = 0, sys_ctrl_ = 0, speech_ctrl_ = 0, speech_data_ = 0;
    uint8_t tonea1_ = 0, tonea2_ = 0, toneb1_ = 0, toneb2_ = 0, noise1_ = 0, noise2_ = 0;
    // sound channels: frequency in Hz (0 = silent)
    double tone_hz_[2] = {0, 0}, noise_hz_ = 0;
};

// The two chips (see the table above for what differs).
class SPL02 : public SPL0X {
public:
    SPL02() : SPL0X("SPL02", 0x90, true, 0xFF, 0xFF) {}
};

class SPL03 : public SPL0X {
public:
    // SPL03.py overrides _set_io_Bank_Select (& 0x1) and _set_io_Speech_Data_Port (& 0xBE, 3-argument
    // set_speech), but its _io_tbl points at the SPL0X base functions, so those overrides are never
    // reached: BrickEmuPy really runs SPL03 with the plain base behaviour (no masks), and so do we.
    SPL03() : SPL0X("SPL03", 0x50, false, 0xFF, 0xFF) {}
};
