#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "BrickCore.h"

// EMC EM73000, Brick Game E-33 2 in 1.
// Generated from BrickEmuPy's cores/EM73000.py (github.com/azya52/BrickEmuPy, CC0)
// by CONVERT_ROM/utils/brick_core_gen.py - edit that, not this.
class EM73000 : public BrickCore {
public:
    EM73000() : BrickCore("EM73000") {}

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
    enum Ports { kPort_ACC = 1000, kPort_CF = 1001, kPort_DP = 1002, kPort_EI = 1003, kPort_HL = 1004, kPort_IL = 1005, kPort_IMASK = 1006, kPort_IO = 1007, kPort_MEMORY = 1008, kPort_P0 = 1009, kPort_P7 = 1010, kPort_P8 = 1011, kPort_PC = 1012, kPort_RAM0 = 1013, kPort_RAM1 = 1014, kPort_SF = 1015, kPort_SP = 1016, kPort_TIMERA = 1017, kPort_TIMERB = 1018, kPort_ZF = 1019, kPort_RES = 1999 };
    struct OpEntry { int64_t (EM73000::*fn)(int64_t); int size; };
    struct IoEntry { int64_t (EM73000::*get)(); int64_t (EM73000::*set)(int64_t); };
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
    int64_t snd_basic_div_ = 0, snd_div_ = 0, snd_mode_ = 0;
    void snd_update() {
        if (snd_mode_ != 0 && snd_div_ > 1 && snd_basic_div_ > 0)
            set_tone((double)clock_hz() / snd_basic_div_ / snd_div_ / 2, snd_mode_ != 1);
        else set_tone(0);
    }
    void snd_set_basic_freq_div(int64_t v) { snd_basic_div_ = v; }
    void snd_set_freq_div(int64_t v) { snd_div_ = v; snd_update(); }
    void snd_set_mode(int64_t v) { snd_mode_ = v; snd_update(); }
    void py_reset();
    void pyreset();
    int64_t py_set_io_dummy(int64_t v_value);
    int64_t py_get_io_dummy();
    int64_t py_get_io_P0();
    void py_set_io_P0(int64_t v_value);
    int64_t py_get_io_P4();
    int64_t py_set_io_P5(int64_t v_value);
    int64_t py_set_io_P6(int64_t v_value);
    int64_t py_set_io_P4(int64_t v_value);
    int64_t py_get_io_P7();
    int64_t py_get_io_P8();
    int64_t py_set_io_P8(int64_t v_value);
    int64_t py_set_io_P9(int64_t v_value);
    int64_t py_get_io_P9();
    int64_t py_set_io_P16(int64_t v_value);
    int64_t py_set_io_P23(int64_t v_value);
    int64_t py_set_io_P24(int64_t v_value);
    int64_t py_set_io_P25(int64_t v_value);
    int64_t py_set_io_P27(int64_t v_value);
    int64_t py_set_io_P28(int64_t v_value);
    int64_t py_set_io_P29(int64_t v_value);
    int64_t py_set_io_P30(int64_t v_value);
    void pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level);
    void py_pin_set(int64_t v_port, int64_t v_pin, int64_t v_level);
    void py_pin_release(int64_t v_port, int64_t v_pin);
    void py_process_timer(int64_t v_exec_cycles);
    void py_interrupt();
    int64_t pyclock();
    void py_write_ram(int64_t v_i, int64_t v_value);
    int64_t py_read_ram(int64_t v_i);
    int64_t py_sbr_a(int64_t v_opcode);
    int64_t py_lcall_a(int64_t v_opcode);
    int64_t py_std_k_y(int64_t v_opcode);
    int64_t py_add_k_y(int64_t v_opcode);
    int64_t py_out_k_p(int64_t v_opcode);
    int64_t py_cmp_k_y(int64_t v_opcode);
    int64_t py_exhl_x(int64_t v_opcode);
    int64_t py_rti(int64_t v_opcode);
    int64_t py_ldhl_x(int64_t v_opcode);
    int64_t py_ret(int64_t v_opcode);
    int64_t py_rlca(int64_t v_opcode);
    int64_t py_rrca(int64_t v_opcode);
    int64_t py_ttcfs(int64_t v_opcode);
    int64_t py_tfcfc(int64_t v_opcode);
    int64_t py_slbr1_a(int64_t v_opcode);
    int64_t py_slbr0_a(int64_t v_opcode);
    int64_t py_nop(int64_t v_opcode);
    int64_t py_exam(int64_t v_opcode);
    int64_t py_stam(int64_t v_opcode);
    int64_t py_ldam(int64_t v_opcode);
    int64_t py_tzs(int64_t v_opcode);
    int64_t py_deca(int64_t v_opcode);
    int64_t py_decm(int64_t v_opcode);
    int64_t py_inca(int64_t v_opcode);
    int64_t py_incm(int64_t v_opcode);
    int64_t py_clpl(int64_t v_opcode);
    int64_t py_tfpl(int64_t v_opcode);
    int64_t py_sepl(int64_t v_opcode);
    int64_t py_cil(int64_t v_opcode);
    int64_t py_cil_r(int64_t v_opcode);
    int64_t py_dicil_r(int64_t v_opcode);
    int64_t py_eicil_r(int64_t v_opcode);
    int64_t py_exal(int64_t v_opcode);
    int64_t py_ldax(int64_t v_opcode);
    int64_t py_exah(int64_t v_opcode);
    int64_t py_ldaxi(int64_t v_opcode);
    int64_t py_exa_x(int64_t v_opcode);
    int64_t py_sta_x(int64_t v_opcode);
    int64_t py_lda_x(int64_t v_opcode);
    int64_t py_cmpa_x(int64_t v_opcode);
    int64_t py_bit_y_b(int64_t v_opcode);
    int64_t py_tf_y_b(int64_t v_opcode);
    int64_t py_set_y_b(int64_t v_opcode);
    int64_t py_tt_y_b(int64_t v_opcode);
    int64_t py_clr_y_b(int64_t v_opcode);
    int64_t py_bit_p_b(int64_t v_opcode);
    int64_t py_tfp_p_b(int64_t v_opcode);
    int64_t py_sep_p_b(int64_t v_opcode);
    int64_t py_ttp_p_b(int64_t v_opcode);
    int64_t py_clp_p_b(int64_t v_opcode);
    int64_t py_math_k(int64_t v_opcode);
    int64_t py_addl_k(int64_t v_opcode);
    int64_t py_cmpl_k(int64_t v_opcode);
    int64_t py_ora_k(int64_t v_opcode);
    int64_t py_adda_k(int64_t v_opcode);
    int64_t py_anda_k(int64_t v_opcode);
    int64_t py_suba_k(int64_t v_opcode);
    int64_t py_addh_k(int64_t v_opcode);
    int64_t py_cmph_k(int64_t v_opcode);
    int64_t py_orm_k(int64_t v_opcode);
    int64_t py_addm_k(int64_t v_opcode);
    int64_t py_andm_k(int64_t v_opcode);
    int64_t py_subm_k(int64_t v_opcode);
    int64_t py_io_p(int64_t v_opcode);
    int64_t py_ina_p(int64_t v_opcode);
    int64_t py_inm_p(int64_t v_opcode);
    int64_t py_outa_p(int64_t v_opcode);
    int64_t py_outm_p(int64_t v_opcode);
    int64_t py_adcam(int64_t v_opcode);
    int64_t py_addam(int64_t v_opcode);
    int64_t py_sbcam(int64_t v_opcode);
    int64_t py_cmpam(int64_t v_opcode);
    int64_t py_tla(int64_t v_opcode);
    int64_t py_exae(int64_t v_opcode);
    int64_t py_tha(int64_t v_opcode);
    int64_t py_oram(int64_t v_opcode);
    int64_t py_xoram(int64_t v_opcode);
    int64_t py_andam(int64_t v_opcode);
    int64_t py_decl(int64_t v_opcode);
    int64_t py_stamd(int64_t v_opcode);
    int64_t py_incl(int64_t v_opcode);
    int64_t py_stami(int64_t v_opcode);
    int64_t py_ldl_k(int64_t v_opcode);
    int64_t py_ldh_k(int64_t v_opcode);
    int64_t py_stdmi_k(int64_t v_opcode);
    int64_t py_cmpia_k(int64_t v_opcode);
    int64_t py_lbr_a(int64_t v_opcode);
    int64_t py_ldia_k(int64_t v_opcode);
    int64_t py_scall_a(int64_t v_opcode);
    int64_t py_clm_b(int64_t v_opcode);
    int64_t py_sem_b(int64_t v_opcode);
    int64_t py_tfa_b(int64_t v_opcode);
    int64_t py_tfm_b(int64_t v_opcode);
    int64_t py_dummy(int64_t v_opcode);
    static const OpEntry T_execute[256];
    static const OpEntry T_execute_cil[4];
    static const OpEntry T_execute_bit_y_b[4];
    static const OpEntry T_execute_bit_p_b[4];
    static const OpEntry T_execute_math_k[16];
    static const OpEntry T_execute_io_p[4];
    static const IoEntry* find_io_tbl(int64_t addr);
    int64_t f_P0_pullup_mask = 0;
    int64_t f_P7_pullup_mask = 0;
    int64_t f_P8_pullup_mask = 0;
    int64_t f_P0_wakeup_mask = 0;
    int64_t f_P8_wakeup_mask = 0;
    int64_t f_ACC = 0;
    int64_t f_PC = 0;
    int64_t f_SP = 0;
    int64_t f_DP = 0;
    int64_t f_HL = 0;
    int64_t f_EI = 0;
    int64_t f_CF = 0;
    int64_t f_ZF = 0;
    int64_t f_SF = 0;
    int64_t f_TIMERA = 0;
    int64_t f_TIMERB = 0;
    int64_t f_IL = 0;
    int64_t f_IMASK = 0;
    std::vector<int64_t> f_RAM;
    int64_t f_P0 = 0;
    int64_t f_P4 = 0;
    int64_t f_P5 = 0;
    int64_t f_P6 = 0;
    int64_t f_P7 = 0;
    int64_t f_P8 = 0;
    int64_t f_P9 = 0;
    int64_t f_P16 = 0;
    int64_t f_P23 = 0;
    int64_t f_P24 = 0;
    int64_t f_P25 = 0;
    int64_t f_P27 = 0;
    int64_t f_P28 = 0;
    int64_t f_P29 = 0;
    int64_t f_P30 = 0;
    int64_t f_timerA_counter = 0;
    int64_t f_timerB_counter = 0;
    int64_t f_time_base_counter = 0;
    int64_t f_ram_bank = 0;
    int64_t f_instr_counter = 0;
};
