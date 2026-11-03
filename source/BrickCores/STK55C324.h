#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "BrickCore.h"

// Sitronix STK55C324 (a 6502), Stadlbauer's Mini Classics Soccer.
// Generated from BrickEmuPy's cores/STK55C324.py (github.com/azya52/BrickEmuPy, CC0)
// by CONVERT_ROM/utils/brick_core_gen.py - edit that, not this.
class STK55C324 : public BrickCore {
public:
    STK55C324() : BrickCore("STK55C324") {}

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
    enum Ports { kPort_A = 1000, kPort_BF = 1001, kPort_CF = 1002, kPort_DF = 1003, kPort_IF = 1004, kPort_LCDRAM = 1005, kPort_NF = 1006, kPort_P1 = 1007, kPort_PC = 1008, kPort_RAM = 1009, kPort_SFR = 1010, kPort_SP = 1011, kPort_TF = 1012, kPort_VF = 1013, kPort_X = 1014, kPort_Y = 1015, kPort_ZF = 1016, kPort_RES = 1999 };
    struct OpEntry { int64_t (STK55C324::*fn)(int64_t); int size; };
    struct IoEntry { int64_t (STK55C324::*get)(); int64_t (STK55C324::*set)(int64_t); };
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
    // STK55C324sound.py has two channels, Yokoi one tone: the first that's playing wins.
    int64_t snd_clock_[2] = {0, 0}, snd_data_[2] = {0, 0}, snd_volume_[2] = {0, 0}, snd_enable_[2] = {0, 0};
    void snd_update() {
        for (int c = 0; c < 2; c++) {
            if (snd_enable_[c] && snd_volume_[c] > 0 && snd_data_[c] > 0) {
                set_tone((double)clock_hz() / (2 << snd_clock_[c]) / snd_data_[c] / 2);
                return;
            }
        }
        set_tone(0);
    }
    void snd_set_volume(int64_t c, int64_t v) { snd_volume_[c] = v; snd_update(); }
    void snd_set_data(int64_t c, int64_t v) { snd_data_[c] = v; snd_update(); }
    void snd_set_clock(int64_t v) { snd_clock_[0] = v & 7; snd_clock_[1] = (v >> 4) & 7; snd_update(); }
    void snd_set_control(int64_t v) { snd_enable_[0] = (v & 1) != 0; snd_enable_[1] = (v & 4) != 0; snd_update(); }
    void pyreset();
    int64_t py_port_read(int64_t v_port);
    void pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level);
    void py_irq();
    void py_nmi();
    void py_timers_clock(int64_t v_exec_cycles);
    double pyclock();
    int64_t py_get_sfr_dummy();
    void py_set_sfr_dummy(int64_t v_value);
    int64_t py_set_sfr_standby_mode(int64_t v_value);
    int64_t py_set_sfr_sleep_mode(int64_t v_value);
    int64_t py_set_sfr_watch_timer_ctrl(int64_t v_value);
    int64_t py_get_sfr_irq_flag();
    int64_t py_set_sfr_irq_flag(int64_t v_value);
    int64_t py_get_sfr_port1_data();
    int64_t py_set_sfr_port1_data(int64_t v_value);
    int64_t py_set_sfr_port1_dir(int64_t v_value);
    int64_t py_set_sfr_port1_int(int64_t v_value);
    int64_t py_get_sfr_timer1_data();
    int64_t py_set_sfr_timer1_data(int64_t v_value);
    int64_t py_set_sfr_timer1_div_contrast(int64_t v_value);
    int64_t py_set_sfr_ctrl(int64_t v_value);
    int64_t py_set_sfr_volume_ch1(int64_t v_value);
    int64_t py_set_sfr_volume_ch2(int64_t v_value);
    int64_t py_set_sfr_sound_clock(int64_t v_value);
    int64_t py_set_sfr_data_ch1(int64_t v_value);
    int64_t py_set_sfr_data_ch2(int64_t v_value);
    int64_t py_set_sfr_sound_ctrl(int64_t v_value);
    void py_write_mem(int64_t v_addr, int64_t v_value);
    int64_t py_read_mem(int64_t v_addr);
    int64_t py_get_ps();
    void py_set_ps(int64_t v_ps);
    void py_adc(int64_t v_operand);
    void py_sbc(int64_t v_operand);
    int64_t py_brk(int64_t = 0);
    int64_t py_ora_ind_x(int64_t = 0);
    int64_t py_tsb_zp(int64_t = 0);
    int64_t py_ora_zp(int64_t = 0);
    int64_t py_asl_zp(int64_t = 0);
    int64_t py_php(int64_t = 0);
    int64_t py_ora_imm(int64_t = 0);
    int64_t py_asl_a(int64_t = 0);
    int64_t py_tsb_abs(int64_t = 0);
    int64_t py_ora_abs(int64_t = 0);
    int64_t py_asl_abs(int64_t = 0);
    int64_t py_bpl(int64_t = 0);
    int64_t py_ora_ind_y(int64_t = 0);
    int64_t py_ora_zp_ind(int64_t = 0);
    int64_t py_trb_zp(int64_t = 0);
    int64_t py_ora_zp_x(int64_t = 0);
    int64_t py_asl_zp_x(int64_t = 0);
    int64_t py_ora_abs_y(int64_t = 0);
    int64_t py_inc_a(int64_t = 0);
    int64_t py_and_ind_x(int64_t = 0);
    int64_t py_trb_abs(int64_t = 0);
    int64_t py_ora_abs_x(int64_t = 0);
    int64_t py_asl_abs_x(int64_t = 0);
    int64_t py_clc(int64_t = 0);
    int64_t py_jsr_abs(int64_t = 0);
    int64_t py_and_abs_y(int64_t = 0);
    int64_t py_dec_a(int64_t = 0);
    int64_t py_bit_abs_x(int64_t = 0);
    int64_t py_and_abs_x(int64_t = 0);
    int64_t py_rol_abs_x(int64_t = 0);
    int64_t py_bit_zp(int64_t = 0);
    int64_t py_and_zp(int64_t = 0);
    int64_t py_rol_zp(int64_t = 0);
    int64_t py_plp(int64_t = 0);
    int64_t py_and_imm(int64_t = 0);
    int64_t py_rol_a(int64_t = 0);
    int64_t py_bit_abs(int64_t = 0);
    int64_t py_and_abs(int64_t = 0);
    int64_t py_rol_abs(int64_t = 0);
    int64_t py_bmi(int64_t = 0);
    int64_t py_and_ind_y(int64_t = 0);
    int64_t py_and_zp_ind(int64_t = 0);
    int64_t py_bit_zp_x(int64_t = 0);
    int64_t py_and_zp_x(int64_t = 0);
    int64_t py_rol_zp_x(int64_t = 0);
    int64_t py_sec(int64_t = 0);
    int64_t py_rti(int64_t = 0);
    int64_t py_eor_ind_x(int64_t = 0);
    int64_t py_eor_zp(int64_t = 0);
    int64_t py_lsr_zp(int64_t = 0);
    int64_t py_pha(int64_t = 0);
    int64_t py_eor_imm(int64_t = 0);
    int64_t py_lsr_a(int64_t = 0);
    int64_t py_jmp_abs(int64_t = 0);
    int64_t py_eor_abs(int64_t = 0);
    int64_t py_lsr_abs(int64_t = 0);
    int64_t py_bvc(int64_t = 0);
    int64_t py_eor_ind_y(int64_t = 0);
    int64_t py_eor_zp_ind(int64_t = 0);
    int64_t py_eor_zp_x(int64_t = 0);
    int64_t py_lsr_zp_x(int64_t = 0);
    int64_t py_cli(int64_t = 0);
    int64_t py_eor_abs_y(int64_t = 0);
    int64_t py_phy(int64_t = 0);
    int64_t py_eor_abs_x(int64_t = 0);
    int64_t py_lsr_abs_x(int64_t = 0);
    int64_t py_rts(int64_t = 0);
    int64_t py_adc_ind_x(int64_t = 0);
    int64_t py_stz_zp(int64_t = 0);
    int64_t py_adc_zp(int64_t = 0);
    int64_t py_ror_zp(int64_t = 0);
    int64_t py_pla(int64_t = 0);
    int64_t py_adc_imm(int64_t = 0);
    int64_t py_ror_a(int64_t = 0);
    int64_t py_jmp_ind(int64_t = 0);
    int64_t py_adc_abs(int64_t = 0);
    int64_t py_ror_abs(int64_t = 0);
    int64_t py_bvs(int64_t = 0);
    int64_t py_adc_ind_y(int64_t = 0);
    int64_t py_adc_zp_ind(int64_t = 0);
    int64_t py_stz_zp_x(int64_t = 0);
    int64_t py_adc_zp_x(int64_t = 0);
    int64_t py_ror_zp_x(int64_t = 0);
    int64_t py_sei(int64_t = 0);
    int64_t py_adc_abs_y(int64_t = 0);
    int64_t py_ply(int64_t = 0);
    int64_t py_jmp_abs_ind_x(int64_t = 0);
    int64_t py_adc_abs_x(int64_t = 0);
    int64_t py_ror_abs_x(int64_t = 0);
    int64_t py_bra(int64_t = 0);
    int64_t py_sta_ind_x(int64_t = 0);
    int64_t py_sty_zp(int64_t = 0);
    int64_t py_sta_zp(int64_t = 0);
    int64_t py_stx_zp(int64_t = 0);
    int64_t py_dey(int64_t = 0);
    int64_t py_bit_imm(int64_t = 0);
    int64_t py_txa(int64_t = 0);
    int64_t py_sty_abs(int64_t = 0);
    int64_t py_sta_abs(int64_t = 0);
    int64_t py_stx_abs(int64_t = 0);
    int64_t py_bcc(int64_t = 0);
    int64_t py_sta_ind_y(int64_t = 0);
    int64_t py_sta_zp_ind(int64_t = 0);
    int64_t py_sty_zp_x(int64_t = 0);
    int64_t py_sta_zp_x(int64_t = 0);
    int64_t py_stx_zp_y(int64_t = 0);
    int64_t py_tya(int64_t = 0);
    int64_t py_sta_abs_y(int64_t = 0);
    int64_t py_txs(int64_t = 0);
    int64_t py_stz_abs(int64_t = 0);
    int64_t py_sta_abs_x(int64_t = 0);
    int64_t py_stz_abs_x(int64_t = 0);
    int64_t py_ldy_imm(int64_t = 0);
    int64_t py_lda_ind_x(int64_t = 0);
    int64_t py_ldx_imm(int64_t = 0);
    int64_t py_ldy_zp(int64_t = 0);
    int64_t py_lda_zp(int64_t = 0);
    int64_t py_ldx_zp(int64_t = 0);
    int64_t py_tay(int64_t = 0);
    int64_t py_lda_imm(int64_t = 0);
    int64_t py_tax(int64_t = 0);
    int64_t py_ldy_abs(int64_t = 0);
    int64_t py_lda_abs(int64_t = 0);
    int64_t py_ldx_abs(int64_t = 0);
    int64_t py_bcs(int64_t = 0);
    int64_t py_lda_ind_y(int64_t = 0);
    int64_t py_lda_zp_ind(int64_t = 0);
    int64_t py_ldy_zp_x(int64_t = 0);
    int64_t py_lda_zp_x(int64_t = 0);
    int64_t py_ldx_zp_y(int64_t = 0);
    int64_t py_clv(int64_t = 0);
    int64_t py_lda_abs_y(int64_t = 0);
    int64_t py_tsx(int64_t = 0);
    int64_t py_ldy_abs_x(int64_t = 0);
    int64_t py_lda_abs_x(int64_t = 0);
    int64_t py_ldx_abs_y(int64_t = 0);
    int64_t py_cpy_imm(int64_t = 0);
    int64_t py_cmp_ind_x(int64_t = 0);
    int64_t py_cpy_zp(int64_t = 0);
    int64_t py_cmp_zp(int64_t = 0);
    int64_t py_dec_zp(int64_t = 0);
    int64_t py_iny(int64_t = 0);
    int64_t py_cmp_imm(int64_t = 0);
    int64_t py_cpy_abs(int64_t = 0);
    int64_t py_cmp_abs(int64_t = 0);
    int64_t py_dec_abs(int64_t = 0);
    int64_t py_dex(int64_t = 0);
    int64_t py_bne(int64_t = 0);
    int64_t py_cmp_ind_y(int64_t = 0);
    int64_t py_cmp_zp_ind(int64_t = 0);
    int64_t py_cmp_zp_x(int64_t = 0);
    int64_t py_dec_zp_x(int64_t = 0);
    int64_t py_cld(int64_t = 0);
    int64_t py_cmp_abs_y(int64_t = 0);
    int64_t py_phx(int64_t = 0);
    int64_t py_cmp_abs_x(int64_t = 0);
    int64_t py_dec_abs_x(int64_t = 0);
    int64_t py_cpx_imm(int64_t = 0);
    int64_t py_sbc_ind_x(int64_t = 0);
    int64_t py_cpx_zp(int64_t = 0);
    int64_t py_sbc_zp(int64_t = 0);
    int64_t py_inc_zp(int64_t = 0);
    int64_t py_inx(int64_t = 0);
    int64_t py_sbc_imm(int64_t = 0);
    int64_t py_cpx_abs(int64_t = 0);
    int64_t py_sbc_abs(int64_t = 0);
    int64_t py_inc_abs(int64_t = 0);
    int64_t py_nop(int64_t = 0);
    int64_t py_beq(int64_t = 0);
    int64_t py_sbc_ind_y(int64_t = 0);
    int64_t py_sbc_zp_ind(int64_t = 0);
    int64_t py_sbc_zp_x(int64_t = 0);
    int64_t py_inc_zp_x(int64_t = 0);
    int64_t py_sed(int64_t = 0);
    int64_t py_sbc_abs_y(int64_t = 0);
    int64_t py_plx(int64_t = 0);
    int64_t py_sbc_abs_x(int64_t = 0);
    int64_t py_inc_abs_x(int64_t = 0);
    int64_t py_dummy(int64_t = 0);
    static const OpEntry T_execute[256];
    static const IoEntry* find_sfr_tbl(int64_t addr);
    int64_t f_rom_offset = 0;
    std::map<int64_t, int64_t> f_pullup;
    std::map<int64_t, std::vector<int64_t>> f_port_input;
    int64_t f_instr_counter = 0;
    double f_64Hz_counter = 0;
    int64_t f_timer1_counter = 0;
    double f_fixtime_counter = 0;
    double f_sub_clock_div = 0;
    int64_t f_PC = 0;
    int64_t f_A = 0;
    int64_t f_X = 0;
    int64_t f_Y = 0;
    int64_t f_NF = 0;
    int64_t f_VF = 0;
    int64_t f_TF = 0;
    int64_t f_BF = 0;
    int64_t f_DF = 0;
    int64_t f_IF = 0;
    int64_t f_ZF = 0;
    int64_t f_CF = 0;
    int64_t f_SP = 0;
    int64_t f_CONTROL = 0;
    int64_t f_nSTANDBY = 0;
    int64_t f_SLEEP_MODE = 0;
    std::vector<int64_t> f_RAM;
    std::vector<int64_t> f_LCDRAM;
    std::map<int64_t, int64_t> f_PDIR;
    std::map<int64_t, int64_t> f_PLATCH;
    int64_t f_P1INT = 0;
    int64_t f_TIMER1 = 0;
    int64_t f_TIMER1_LATCH = 0;
    int64_t f_TIMER_CLK_LCD_CONTRAST = 0;
    int64_t f_TIMER1_DIV = 0;
    int64_t f_WATCH_TIMER_CTRL = 0;
    int64_t f_IREQ = 0;
    int64_t f_SOUND_CH1_VOL = 0;
    int64_t f_SOUND_CH2_VOL = 0;
    int64_t f_SOUND_CLK = 0;
    int64_t f_SOUND_CH1_DATA = 0;
    int64_t f_SOUND_CH2_DATA = 0;
    int64_t f_SOUND_CTRL = 0;
    int64_t f_addr_reset = 0;
    int64_t f_addr_irq = 0;
    int64_t f_addr_nmi = 0;
};
