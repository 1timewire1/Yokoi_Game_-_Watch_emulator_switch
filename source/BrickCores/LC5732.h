#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "BrickCore.h"

// Sanyo LC5732, Hiro Pocket Boy No.2 and Edla's Space Rescue.
// Generated from BrickEmuPy's cores/LC5732.py (github.com/azya52/BrickEmuPy, CC0)
// by CONVERT_ROM/utils/brick_core_gen.py - edit that, not this.
class LC5732 : public BrickCore {
public:
    LC5732() : BrickCore("LC5732") {}

protected:
    void reset() override;
    int execute() override { return (int)(execute_scaled() / kTickHz); }
    int64_t execute_scaled() override;
    uint8_t display_byte(int i) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t a, uint8_t v) override {
        if (a >= f_RAM.size()) return false;
        f_RAM[a] = v;
        return true;
    }
    int peek_ram(uint32_t a) const override { return a < f_RAM.size() ? (int)f_RAM[a] : -1; }

private:
    enum Ports { kPort_AC = 1000, kPort_ALM = 1001, kPort_CF = 1002, kPort_CSTF = 1003, kPort_DP = 1004, kPort_EDP = 1005, kPort_HALT = 1006, kPort_HEF = 1007, kPort_LCDRAM = 1008, kPort_LGT = 1009, kPort_MEMORY = 1010, kPort_PAGE = 1011, kPort_PC = 1012, kPort_PDF = 1013, kPort_PM = 1014, kPort_PS = 1015, kPort_RAM = 1016, kPort_SP = 1017, kPort_STACK = 1018, kPort_STS = 1019, kPort_TREG = 1020, kPort_RES = 1999 };
    struct OpEntry { int64_t (LC5732::*fn)(int64_t); int size; };
    struct IoEntry { int64_t (LC5732::*get)(); int64_t (LC5732::*set)(int64_t); };
    void init_();
    bool inited_ = false;
    int64_t rom_get_byte(int64_t a) const;
    int64_t rom_get_word(int64_t a) const { return (rom_get_byte(a) << 8) | rom_get_byte(a + 1); }
    int64_t rom_get_word_LSB(int64_t a) const { return rom_get_byte(a) | (rom_get_byte(a + 1) << 8); }
    int64_t rom_get_bytes(int64_t a, int64_t n) const {
        int64_t r = 0;
        for (int64_t i = 0; i < n; i++) r |= rom_get_byte(a + i) << (8 * (n - i - 1));
        return r;
    }
    int64_t rom_size() const { return (int64_t)rom().size(); }
    int64_t rom_get_mask() const {
        int64_t bits = 0;
        for (int64_t n = rom_size(); n; n >>= 1) bits++;
        return ((int64_t)1 << bits) - 1;
    }
    std::vector<std::vector<double>> snd_tbl_;
    void snd_set_alm(int64_t alm);
    void py_reset();
    void pyreset();
    void pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level);
    void py_timers_clock(int64_t v_exec_cycles);
    int64_t pyclock();
    int64_t py_get_ram();
    void py_set_ram(int64_t v_value);
    int64_t py_halt(int64_t v_opcode);
    int64_t py_taat(int64_t v_opcode);
    int64_t py_twrt(int64_t v_opcode);
    int64_t py_tmel(int64_t v_opcode);
    int64_t py_csp(int64_t v_opcode);
    int64_t py_cst(int64_t v_opcode);
    int64_t py_rc5(int64_t v_opcode);
    int64_t py_sc5(int64_t v_opcode);
    int64_t py_jmp_x(int64_t v_opcode);
    int64_t py_jmp_p(int64_t v_opcode);
    int64_t py_page(int64_t v_opcode);
    int64_t py_mtr(int64_t v_opcode);
    int64_t py_rts(int64_t v_opcode);
    int64_t py_mpcl(int64_t v_opcode);
    int64_t py_mpcm(int64_t v_opcode);
    int64_t py_mpch(int64_t v_opcode);
    int64_t py_in(int64_t v_opcode);
    int64_t py_asr0(int64_t v_opcode);
    int64_t py_asr1(int64_t v_opcode);
    int64_t py_asl0(int64_t v_opcode);
    int64_t py_asl1(int64_t v_opcode);
    int64_t py_sdpl(int64_t v_opcode);
    int64_t py_sdph(int64_t v_opcode);
    int64_t py_edpl(int64_t v_opcode);
    int64_t py_edph(int64_t v_opcode);
    int64_t py_mvi_x(int64_t v_opcode);
    int64_t py_ldi_x(int64_t v_opcode);
    int64_t py_baz_x(int64_t v_opcode);
    int64_t py_bab0_x(int64_t v_opcode);
    int64_t py_banz_x(int64_t v_opcode);
    int64_t py_bab1_x(int64_t v_opcode);
    int64_t py_bcnh_x(int64_t v_opcode);
    int64_t py_bab2_x(int64_t v_opcode);
    int64_t py_bch_x(int64_t v_opcode);
    int64_t py_bab3_x(int64_t v_opcode);
    int64_t py_adc(int64_t v_opcode);
    int64_t py_sbc(int64_t v_opcode);
    int64_t py_add(int64_t v_opcode);
    int64_t py_sub(int64_t v_opcode);
    int64_t py_adn(int64_t v_opcode);
    int64_t py_and(int64_t v_opcode);
    int64_t py_eor(int64_t v_opcode);
    int64_t py_or(int64_t v_opcode);
    int64_t py_adc_m(int64_t v_opcode);
    int64_t py_sbc_m(int64_t v_opcode);
    int64_t py_add_m(int64_t v_opcode);
    int64_t py_sub_m(int64_t v_opcode);
    int64_t py_adn_m(int64_t v_opcode);
    int64_t py_and_m(int64_t v_opcode);
    int64_t py_eor_m(int64_t v_opcode);
    int64_t py_or_m(int64_t v_opcode);
    int64_t py_adci(int64_t v_opcode);
    int64_t py_sbci(int64_t v_opcode);
    int64_t py_addi(int64_t v_opcode);
    int64_t py_subi(int64_t v_opcode);
    int64_t py_adni(int64_t v_opcode);
    int64_t py_andi(int64_t v_opcode);
    int64_t py_eori(int64_t v_opcode);
    int64_t py_ori(int64_t v_opcode);
    int64_t py_inc(int64_t v_opcode);
    int64_t py_dec(int64_t v_opcode);
    int64_t py_idpl(int64_t v_opcode);
    int64_t py_ddpl(int64_t v_opcode);
    int64_t py_idph(int64_t v_opcode);
    int64_t py_ddph(int64_t v_opcode);
    int64_t py_isp(int64_t v_opcode);
    int64_t py_dsp(int64_t v_opcode);
    int64_t py_jsr_x(int64_t v_opcode);
    int64_t py_ipm(int64_t v_opcode);
    int64_t py_lda(int64_t v_opcode);
    int64_t py_lsp(int64_t v_opcode);
    int64_t py_lhlt(int64_t v_opcode);
    int64_t py_l500(int64_t v_opcode);
    int64_t py_sta(int64_t v_opcode);
    int64_t py_ssp(int64_t v_opcode);
    int64_t py_ips(int64_t v_opcode);
    int64_t py_mdpl_x(int64_t v_opcode);
    int64_t py_mdph_x(int64_t v_opcode);
    int64_t py_sic_x(int64_t v_opcode);
    int64_t py_msp_x(int64_t v_opcode);
    int64_t py_rcf(int64_t v_opcode);
    int64_t py_scf(int64_t v_opcode);
    int64_t py_rlgt(int64_t v_opcode);
    int64_t py_slgt(int64_t v_opcode);
    int64_t py_spdr_x(int64_t v_opcode);
    int64_t py_rbak(int64_t v_opcode);
    int64_t py_sbak(int64_t v_opcode);
    int64_t py_sas_x(int64_t v_opcode);
    int64_t py_csec(int64_t v_opcode);
    int64_t py_out(int64_t v_opcode);
    int64_t py_ldpl(int64_t v_opcode);
    int64_t py_ldph(int64_t v_opcode);
    int64_t py_nop(int64_t v_opcode);
    static const OpEntry T_execute[256];
    int64_t f_cycle_counter = 0;
    int64_t f_instr_counter = 0;
    double f_500ms_div = 0;
    double f_100ms_div = 0;
    double f_32ms_div = 0;
    int64_t f_AC = 0;
    int64_t f_DP = 0;
    int64_t f_EDP = 0;
    int64_t f_SP = 0;
    int64_t f_PC = 0;
    int64_t f_CF = 0;
    int64_t f_STACK = 0;
    int64_t f_TREG = 0;
    int64_t f_PAGE = 0;
    int64_t f_HALT = 0;
    int64_t f_ALM = 0;
    int64_t f_CSTF = 0;
    int64_t f_HEF = 0;
    int64_t f_STS = 0;
    int64_t f_PS = 0;
    int64_t f_PM = 0;
    int64_t f_LIGHT = 0;
    int64_t f_PDF = 0;
    std::vector<int64_t> f_RAM;
    std::vector<int64_t> f_LCDRAM;
    int64_t f_BACKUP = 0;
    int64_t f_self = 0;
    int64_t f_LGT = 0;
    int64_t f_rom = 0;
};
