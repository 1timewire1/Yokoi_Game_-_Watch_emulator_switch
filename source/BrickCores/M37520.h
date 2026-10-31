#pragma once

#include "BrickCore.h"

// Mitsubishi M37520 (740 family: a 6502 with bit-test/set/clear opcodes, the T
// flag, MUL/DIV, WIT/STP, 4 MHz) - a port of BrickEmuPy's cores/M37520.py
// (github.com/azya52/BrickEmuPy, CC0). Pocket Pikachu (Nintendo, 1998).
//
// Memory map: SFRs 0x00-0x3F, CPU RAM 0x40-0x23F (the stack lives in it:
// page 0 or 1 per CPUM), LCD RAM 0xB0 bytes at 0x340 (or at 0x40 when LM's
// PAGE0 bit is set - that overlay wins over RAM), ROM at the top of the 64 KB
// space. The display is the LCD RAM itself: get_VRAM() is those 176 bytes,
// blank while the LCD is disabled. Inputs are ports P0-P4 (a bitmask in
// "mask") with mask-option pull-ups; P2 going low raises KEYON, P4's INT0/INT1
// pins raise the edge interrupts. Sound is a pin pair on P3 (bits 6/5).
//
// Timekeeping is in cycles * kTickHz ("scaled"), like SPL81408: when the CPU
// runs from the 32768 Hz sub clock one instruction cycle is clock/32768 base
// cycles, so nothing is lost to rounding.
class M37520 : public BrickCore {
public:
    M37520() : BrickCore("M37520") {}

protected:
    void reset() override;
    int execute() override { return (int)(execute_scaled() / kTickHz); }
    int64_t execute_scaled() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t v) override;
    int peek_ram(uint32_t addr) const override;

private:
#define M37520_OPS(X) \
    X(adc_abs) \
    X(adc_abs_x) \
    X(adc_abs_y) \
    X(adc_imm) \
    X(adc_ind_x) \
    X(adc_ind_y) \
    X(adc_zp) \
    X(adc_zp_x) \
    X(and_abs) \
    X(and_abs_x) \
    X(and_abs_y) \
    X(and_imm) \
    X(and_ind_x) \
    X(and_ind_y) \
    X(and_zp) \
    X(and_zp_x) \
    X(asl_a) \
    X(asl_abs) \
    X(asl_abs_x) \
    X(asl_zp) \
    X(asl_zp_x) \
    X(bbc_bit_a) \
    X(bbc_bit_zp) \
    X(bbs_bit_a) \
    X(bbs_bit_zp) \
    X(bcc) \
    X(bcs) \
    X(beq) \
    X(bit_abs) \
    X(bit_zp) \
    X(bmi) \
    X(bne) \
    X(bpl) \
    X(bra) \
    X(brk) \
    X(bvc) \
    X(bvs) \
    X(clb_bit_a) \
    X(clb_bit_zp) \
    X(clc) \
    X(cld) \
    X(cli) \
    X(clt) \
    X(clv) \
    X(cmp_abs) \
    X(cmp_abs_x) \
    X(cmp_abs_y) \
    X(cmp_imm) \
    X(cmp_ind_x) \
    X(cmp_ind_y) \
    X(cmp_zp) \
    X(cmp_zp_x) \
    X(com_zp) \
    X(cpx_abs) \
    X(cpx_imm) \
    X(cpx_zp) \
    X(cpy_abs) \
    X(cpy_imm) \
    X(cpy_zp) \
    X(dec_a) \
    X(dec_abs) \
    X(dec_abs_x) \
    X(dec_zp) \
    X(dec_zp_x) \
    X(dex) \
    X(dey) \
    X(div_zp_x) \
    X(dummy) \
    X(eor_abs) \
    X(eor_abs_x) \
    X(eor_abs_y) \
    X(eor_imm) \
    X(eor_ind_x) \
    X(eor_ind_y) \
    X(eor_zp) \
    X(eor_zp_x) \
    X(inc_a) \
    X(inc_abs) \
    X(inc_abs_x) \
    X(inc_zp) \
    X(inc_zp_x) \
    X(inx) \
    X(iny) \
    X(jmp_abs) \
    X(jmp_ind) \
    X(jmp_zp_ind) \
    X(jsr_abs) \
    X(jsr_sp) \
    X(jsr_zp_ind) \
    X(lda_abs) \
    X(lda_abs_x) \
    X(lda_abs_y) \
    X(lda_imm) \
    X(lda_ind_x) \
    X(lda_ind_y) \
    X(lda_zp) \
    X(lda_zp_x) \
    X(ldm_zp) \
    X(ldx_abs) \
    X(ldx_abs_y) \
    X(ldx_imm) \
    X(ldx_zp) \
    X(ldx_zp_y) \
    X(ldy_abs) \
    X(ldy_abs_x) \
    X(ldy_imm) \
    X(ldy_zp) \
    X(ldy_zp_x) \
    X(lsr_a) \
    X(lsr_abs) \
    X(lsr_abs_x) \
    X(lsr_zp) \
    X(lsr_zp_x) \
    X(mul_zp_x) \
    X(nop) \
    X(ora_abs) \
    X(ora_abs_x) \
    X(ora_abs_y) \
    X(ora_imm) \
    X(ora_ind_x) \
    X(ora_ind_y) \
    X(ora_zp) \
    X(ora_zp_x) \
    X(pha) \
    X(php) \
    X(pla) \
    X(plp) \
    X(rol_a) \
    X(rol_abs) \
    X(rol_abs_x) \
    X(rol_zp) \
    X(rol_zp_x) \
    X(ror_a) \
    X(ror_abs) \
    X(ror_abs_x) \
    X(ror_zp) \
    X(ror_zp_x) \
    X(rrf_zp) \
    X(rti) \
    X(rts) \
    X(sbc_abs) \
    X(sbc_abs_x) \
    X(sbc_abs_y) \
    X(sbc_imm) \
    X(sbc_ind_x) \
    X(sbc_ind_y) \
    X(sbc_zp) \
    X(sbc_zp_x) \
    X(seb_bit_a) \
    X(seb_bit_zp) \
    X(sec) \
    X(sed) \
    X(sei) \
    X(set) \
    X(sta_abs) \
    X(sta_abs_x) \
    X(sta_abs_y) \
    X(sta_ind_x) \
    X(sta_ind_y) \
    X(sta_zp) \
    X(sta_zp_x) \
    X(stp) \
    X(stx_abs) \
    X(stx_zp) \
    X(stx_zp_y) \
    X(sty_abs) \
    X(sty_zp) \
    X(sty_zp_x) \
    X(tax) \
    X(tay) \
    X(tst_zp) \
    X(tsx) \
    X(txa) \
    X(txs) \
    X(tya) \
    X(wit)


    typedef int (M37520::*OpFn)(uint32_t);
    struct OpEntry { OpFn fn; uint8_t len; };
    static const OpEntry kOps[256];
#define M37520_DECL(n) int op_##n(uint32_t op);
    M37520_OPS(M37520_DECL)
#undef M37520_DECL

    // memory / I/O
    uint8_t rom_byte(int64_t addr) const {
        const int64_t n = (int64_t)rom().size();
        return rom()[(size_t)(((addr % n) + n) % n)];
    }
    uint8_t read_mem(uint32_t addr);
    void write_mem(uint32_t addr, uint8_t value);
    uint8_t io_read(uint8_t reg);
    void io_write(uint8_t reg, uint8_t value);
    uint8_t port_read(int port) const;
    void set_lm(uint8_t v);
    void set_cpum(uint8_t v);

    // CPU helpers
    uint8_t ps() const;
    void set_ps(uint8_t p);
    void push(uint8_t v) { write_mem(spp_ | sp_, v); sp_ = (uint8_t)(sp_ - 1); }
    uint8_t pull() { sp_ = (uint8_t)(sp_ + 1); return read_mem(spp_ | sp_); }
    void go_vector(int addr);
    void interrupt(int addr);
    void interrupt_process();
    void timers_clock(int64_t scaled_cycles);
    void nz(uint8_t v) { nf_ = v >> 7; zf_ = v == 0; }
    int branch(bool taken, uint32_t op);
    int alu_adc(uint8_t operand);
    int alu_sbc(uint8_t operand);
    int alu_ora(uint8_t operand);
    int alu_and(uint8_t operand);
    int alu_eor(uint8_t operand);
    int alu_lda(uint8_t operand);
    int alu_cmp(uint8_t operand);
    void compare(uint8_t reg, uint8_t operand);
    uint8_t sh_asl(uint8_t v);
    uint8_t sh_rol(uint8_t v);
    uint8_t sh_lsr(uint8_t v);
    uint8_t sh_ror(uint8_t v);

    // addressing modes, from the packed big-endian instruction bytes
    static uint32_t m_abs(uint32_t op) { return ((op & 0xFF) << 8) | ((op >> 8) & 0xFF); }
    uint32_t m_abs_x(uint32_t op) const { return (m_abs(op) + x_) & 0xFFFF; }
    uint32_t m_abs_y(uint32_t op) const { return (m_abs(op) + y_) & 0xFFFF; }
    uint32_t m_zp_x(uint32_t op) const { return (op + x_) & 0xFF; }
    uint32_t m_zp_y(uint32_t op) const { return (op + y_) & 0xFF; }
    uint32_t m_ind_x(uint32_t op) {
        return read_mem((op + x_) & 0xFF) | ((uint32_t)read_mem((op + x_ + 1) & 0xFF) << 8);
    }
    uint32_t m_ind_y(uint32_t op) {
        return ((read_mem(op & 0xFF) | ((uint32_t)read_mem((op + 1) & 0xFF) << 8)) + y_) & 0xFFFF;
    }

    // mask options
    bool options_loaded_ = false;
    uint8_t pullup_ext_[5] = {};
    int64_t rom_offset_ = 0;

    // CPU
    uint32_t pc_ = 0;
    uint8_t a_ = 0, x_ = 0, y_ = 0, sp_ = 0;
    uint32_t spp_ = 0x100;
    uint8_t nf_ = 0, vf_ = 0, tf_ = 0, bf_ = 0, df_ = 0, if_ = 1, zf_ = 0, cf_ = 0;
    uint8_t nwait_ = 1, nstop_ = 1;
    std::vector<uint8_t> ram_ = std::vector<uint8_t>(0x200, 0);
    std::vector<uint8_t> lcd_ = std::vector<uint8_t>(0xB0, 0);
    uint32_t lcd_offset_ = 0x340;
    int64_t cpu_div_ = 2 * (int64_t)kTickHz;   // scaled cycles per instruction cycle
    int64_t main_div_ = (int64_t)kTickHz;      // scaled cycles per main-clock timer tick

    // ports: direction, output latch, pull-up register, pins driven low/high from outside
    uint8_t pdir_[5] = {}, platch_[5] = {}, pullup_[5] = {};
    uint8_t in_low_[5] = {}, in_high_[5] = {};

    // timers (X, Y 16-bit; 1, 2, 3 8-bit) and their fractional-cycle counters
    int32_t tx_ = 0xFFFF, tx_latch_ = 0xFFFF, ty_ = 0xFFFF, ty_latch_ = 0xFFFF;
    int32_t t1_ = 0xFF, t1_latch_ = 0xFF, t2_ = 1, t2_latch_ = 1, t3_ = 0xFF, t3_latch_ = 0xFF;
    int64_t tx_counter_ = 0, ty_counter_ = 0, t1_counter_ = 0, t2_counter_ = 0, t3_counter_ = 0;
    uint8_t txm_ = 0, tym_ = 0, t123m_ = 0, lc_ = 0, lm_ = 0, intedge_ = 0, cpum_ = 0;
    uint8_t ireq1_ = 0, ireq2_ = 0, icon1_ = 0, icon2_ = 0;
};
