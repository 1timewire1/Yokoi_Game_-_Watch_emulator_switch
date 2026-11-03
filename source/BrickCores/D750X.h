#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "BrickCore.h"

// NEC uPD750x, Epoch Monster Panic.
// Generated from BrickEmuPy's cores/D750X.py (github.com/azya52/BrickEmuPy, CC0)
// by CONVERT_ROM/utils/brick_core_gen.py - edit that, not this.
class D750X : public BrickCore {
public:
    D750X() : BrickCore("D750X") {}

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
    enum Ports { kPort_4 = 4, kPort_5 = 5, kPort_A = 1000, kPort_CY = 1001, kPort_DE = 1002, kPort_DL = 1003, kPort_HL = 1004, kPort_IE1 = 1005, kPort_IET = 1006, kPort_IME = 1007, kPort_MEMORY = 1008, kPort_PC = 1009, kPort_PORT = 1010, kPort_RAM = 1011, kPort_SIO = 1012, kPort_SIOCR = 1013, kPort_SP = 1014, kPort_TCR = 1015, kPort_TMR = 1016, kPort_RES = 1999 };
    struct OpEntry { int64_t (D750X::*fn)(int64_t); int size; };
    struct IoEntry { int64_t (D750X::*get)(); int64_t (D750X::*set)(int64_t); };
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
    void snd_toggle(int64_t a, int64_t b) { set_speaker(a != b && a); }
    void py_reset();
    void pyreset();
    void pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level);
    void py_interrupt(int64_t v_addr);
    int64_t pyclock();
    int64_t py_instruction_00111110(int64_t v_opcode);
    int64_t py_instruction_00111111(int64_t v_opcode);
    int64_t py_dummy(int64_t v_opcode);
    int64_t py_skip_next();
    int64_t py_aisc_data(int64_t v_opcode);
    int64_t py_lai_data(int64_t v_opcode);
    int64_t py_jmp_addr(int64_t v_opcode);
    int64_t py_call_addr(int64_t v_opcode);
    int64_t py_ladr_addr(int64_t v_opcode);
    int64_t py_xadr(int64_t v_opcode);
    int64_t py_xhdr(int64_t v_opcode);
    int64_t py_xldr(int64_t v_opcode);
    int64_t py_ddrs_addr(int64_t v_opcode);
    int64_t py_idrs_addr(int64_t v_opcode);
    int64_t py_lei_data(int64_t v_opcode);
    int64_t py_lli_data(int64_t v_opcode);
    int64_t py_ldi_data(int64_t v_opcode);
    int64_t py_lhi_data(int64_t v_opcode);
    int64_t py_skeei_data(int64_t v_opcode);
    int64_t py_sklei_data(int64_t v_opcode);
    int64_t py_skdei_data(int64_t v_opcode);
    int64_t py_skhei_data(int64_t v_opcode);
    int64_t py_tae(int64_t v_opcode);
    int64_t py_pshde(int64_t v_opcode);
    int64_t py_popde(int64_t v_opcode);
    int64_t py_tea(int64_t v_opcode);
    int64_t py_tal(int64_t v_opcode);
    int64_t py_tla(int64_t v_opcode);
    int64_t py_pshhl(int64_t v_opcode);
    int64_t py_pophl(int64_t v_opcode);
    int64_t py_tad(int64_t v_opcode);
    int64_t py_tda(int64_t v_opcode);
    int64_t py_tah(int64_t v_opcode);
    int64_t py_tha(int64_t v_opcode);
    int64_t py_jam_addr(int64_t v_opcode);
    int64_t py_tamsp(int64_t v_opcode);
    int64_t py_tspam(int64_t v_opcode);
    int64_t py_halt(int64_t v_opcode);
    int64_t py_stop(int64_t v_opcode);
    int64_t py_timer(int64_t v_opcode);
    int64_t py_sio(int64_t v_opcode);
    int64_t py_tsioam(int64_t v_opcode);
    int64_t py_tcntam(int64_t v_opcode);
    int64_t py_tamsio(int64_t v_opcode);
    int64_t py_tammod(int64_t v_opcode);
    int64_t py_ski_data(int64_t v_opcode);
    int64_t py_skaei_data(int64_t v_opcode);
    int64_t py_ip54(int64_t v_opcode);
    int64_t py_op54(int64_t v_opcode);
    int64_t py_anl(int64_t v_opcode);
    int64_t py_rar(int64_t v_opcode);
    int64_t py_orl(int64_t v_opcode);
    int64_t py_ip_addr(int64_t v_opcode);
    int64_t py_op_addr(int64_t v_opcode);
    int64_t py_di_data(int64_t v_opcode);
    int64_t py_ei_data(int64_t v_opcode);
    int64_t py_lam_dl(int64_t v_opcode);
    int64_t py_lam_de(int64_t v_opcode);
    int64_t py_rtpsw(int64_t v_opcode);
    int64_t py_xam_dl(int64_t v_opcode);
    int64_t py_xam_de(int64_t v_opcode);
    int64_t py_des(int64_t v_opcode);
    int64_t py_ies(int64_t v_opcode);
    int64_t py_xad(int64_t v_opcode);
    int64_t py_xae(int64_t v_opcode);
    int64_t py_anp_data(int64_t v_opcode);
    int64_t py_orp_data(int64_t v_opcode);
    int64_t py_lhli_data(int64_t v_opcode);
    int64_t py_ldei_data(int64_t v_opcode);
    int64_t py_lam_hld(int64_t v_opcode);
    int64_t py_lam_hli(int64_t v_opcode);
    int64_t py_lam_hl(int64_t v_opcode);
    int64_t py_rt(int64_t v_opcode);
    int64_t py_xam_hld(int64_t v_opcode);
    int64_t py_xam_hli(int64_t v_opcode);
    int64_t py_xam_hl(int64_t v_opcode);
    int64_t py_st(int64_t v_opcode);
    int64_t py_dls(int64_t v_opcode);
    int64_t py_ils(int64_t v_opcode);
    int64_t py_skc(int64_t v_opcode);
    int64_t py_rts(int64_t v_opcode);
    int64_t py_lamt(int64_t v_opcode);
    int64_t py_skaem(int64_t v_opcode);
    int64_t py_skmbf_data(int64_t v_opcode);
    int64_t py_skmbt_data(int64_t v_opcode);
    int64_t py_rmb(int64_t v_opcode);
    int64_t py_smb(int64_t v_opcode);
    int64_t py_ipl(int64_t v_opcode);
    int64_t py_ip1(int64_t v_opcode);
    int64_t py_opl(int64_t v_opcode);
    int64_t py_op3(int64_t v_opcode);
    int64_t py_skabt_data(int64_t v_opcode);
    int64_t py_rc(int64_t v_opcode);
    int64_t py_sc(int64_t v_opcode);
    int64_t py_xah(int64_t v_opcode);
    int64_t py_xal(int64_t v_opcode);
    int64_t py_acsc(int64_t v_opcode);
    int64_t py_asc(int64_t v_opcode);
    int64_t py_exl(int64_t v_opcode);
    int64_t py_cma(int64_t v_opcode);
    int64_t py_jcp_addr(int64_t v_opcode);
    int64_t py_lhlt_addr(int64_t v_opcode);
    int64_t py_calt_addr(int64_t v_opcode);
    static const OpEntry T_execute[256];
    static const OpEntry T_instruction_00111110_tbl[256];
    static const OpEntry T_instruction_00111111_tbl[256];
    int64_t f_rom_mask = 0;
    int64_t f_instr_counter = 0;
    double f_timer_counter = 0;
    int64_t f_sound_port = 0;
    std::vector<int64_t> f_sound_mask;
    double f_crystal_clock_div = 0;
    std::vector<double> f_counter_div;
    int64_t f_A = 0;
    int64_t f_HL = 0;
    int64_t f_DE = 0;
    int64_t f_SP = 0;
    int64_t f_PC = 0;
    int64_t f_CF = 0;
    int64_t f_SK = 0;
    int64_t f_HALT = 0;
    int64_t f_STOP = 0;
    int64_t f_IER = 0;
    int64_t f_RQF = 0;
    int64_t f_IME = 0;
    std::vector<int64_t> f_P;
    int64_t f_SIO = 0;
    int64_t f_SIOCR = 0;
    int64_t f_TCR = 0;
    int64_t f_TMR = 0;
    std::vector<int64_t> f_RAM;
    int64_t f_rom = 0;
};
