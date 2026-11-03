#pragma once

#include "BrickCore.h"

// Sunplus SPLB20 (a 6502 subset, the same 70 opcodes as the SPL0X family but with
// operands fetched one after another from the ROM) - a port of BrickEmuPy's
// cores/SPLB20.py: Apollo 2 in 1 virtual pet.
//
// Memory map: LCD RAM 0x00-0x3F (the display, blank while SYS_CTRL's LCD_ENBL is
// off), SFRs 0x40-0x7F, CPU RAM 0x80-0xFF (the stack lives in it, SP is an 8-bit
// address), data RAM 0x200-0x7FF, ROM at the top of the 64 KB space.
//
// Sound: port A's output register enables a square wave whose frequency is
// clock / 2^prescaler / (down-counter preset + 1) (SPLB20Sound.py), played as the
// Yokoi tone.
//
// Time is cycles * kTickHz ("scaled"): the sub clock is clock/32768 cycles, which
// scaled is exactly clock_hz, so nothing is lost to rounding.
class SPLB20 : public BrickCore {
public:
    SPLB20() : BrickCore("SPLB20") {}

protected:
    void reset() override;
    int execute() override { return (int)(execute_scaled() / kTickHz); }
    int64_t execute_scaled() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    // CPU RAM (0x80-0xFF) and data RAM (0x200-0x7FF and its mirrors, as in
    // read_mem()), by CPU address - for the clock writes / checks
    // (BrickCore::set_clock(), "clock.*" options).
    bool poke_ram(uint32_t addr, uint8_t v) override {
        if (addr >= 0x80 && addr < 0x100) { ram_[addr - 0x80] = v; return true; }
        if (addr >= 0x200 && (addr & 0x8FFF) < 0x800) { data_[addr & 0xFFF] = v; return true; }
        return false;
    }
    int peek_ram(uint32_t addr) const override {
        if (addr >= 0x80 && addr < 0x100) return ram_[addr - 0x80];
        if (addr >= 0x200 && (addr & 0x8FFF) < 0x800) return data_[addr & 0xFFF];
        return -1;
    }

private:
#define SPLB20_OPS(X) \
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


    typedef int (SPLB20::*OpFn)();
    static const OpFn kOps[256];
#define SPLB20_DECL(n) int op_##n();
    SPLB20_OPS(SPLB20_DECL)
#undef SPLB20_DECL

    // memory / I/O
    uint8_t rom_byte(int64_t addr) const {
        const int64_t n = (int64_t)rom().size();
        return rom()[(size_t)(((addr % n) + n) % n)];
    }
    uint8_t fetch8() { const uint8_t v = rom_byte(pc_); pc_ = (pc_ + 1) & 0xFFFF; return v; }
    uint32_t word_at_pc() const { return rom_byte(pc_) | ((uint32_t)rom_byte((int64_t)pc_ + 1) << 8); }
    uint8_t read_mem(uint32_t addr);
    void write_mem(uint32_t addr, uint8_t value);
    static bool io_present(uint32_t reg) { return (reg >= 0x70 && reg <= 0x7C) || reg == 0x7E || reg == 0x7F; }
    uint8_t io_read(uint8_t reg);
    void io_write(uint8_t reg, uint8_t value);
    uint8_t port_read() const;

    // CPU
    uint8_t ps() const;
    void set_ps(uint8_t p);
    void push(uint8_t v) { write_mem(sp_, v); sp_ = (uint8_t)(sp_ - 1); }
    uint8_t pull() { sp_ = (uint8_t)(sp_ + 1); return read_mem(sp_); }
    void go_vector(uint32_t addr) { pc_ = rom_byte(addr) | ((uint32_t)rom_byte((int64_t)addr + 1) << 8); }
    void irq();
    void nmi();
    void timers_clock(int64_t scaled_cycles);
    int64_t prescale_scaled() const { return ((int64_t)1 << (prescalar_ > 46 ? 46 : prescalar_)) * (int64_t)kTickHz; }
    void nz(uint8_t v) { nf_ = v >> 7; zf_ = v == 0; }
    int branch(bool taken);
    void adc(uint8_t operand);
    void sbc(uint8_t operand);
    void compare(uint8_t reg, uint8_t operand);
    void sound_update();

    // mask options
    bool options_loaded_ = false;
    uint8_t pullup_ext_ = 0;
    bool non_crystal_mode_ = false;
    int64_t sub_div_ = 0;   // scaled cycles per 32768 Hz sub-clock tick (= clock_hz)

    // CPU state
    uint32_t pc_ = 0;
    uint8_t a_ = 0, x_ = 0, sp_ = 0;
    uint8_t nf_ = 0, vf_ = 0, bf_ = 0, df_ = 0, if_ = 1, zf_ = 0, cf_ = 0;
    uint8_t cpu_enbl_ = 1, rosc_enbl_ = 1;
    std::vector<uint8_t> ram_ = std::vector<uint8_t>(0x80, 0), data_ = std::vector<uint8_t>(0x800, 0),
                         lcd_ = std::vector<uint8_t>(0x40, 0);
    int64_t timer_counter_ = 0, t2hz_counter_ = 0, t128hz_counter_ = 0;
    int tc_ = 0;

    // I/O registers
    uint8_t pdir_ = 0, pcfg_ = 0, platch_ = 0, in_low_ = 0, in_high_ = 0;
    uint8_t lcd_cfg_ = 0, lcd_bias_ = 0, sys_ctrl_ = 0, int_cfg_ = 0, ireq_ = 0, tc_preset_ = 0, prescalar_ = 0,
            keyscan_ctrl_ = 0;
    // sound (SPLB20Sound.py); these survive a reset like the Python's
    int tc_div_ = 1;
    int clock_div_log2_ = 0;
    bool sound_enable_ = false;
};
