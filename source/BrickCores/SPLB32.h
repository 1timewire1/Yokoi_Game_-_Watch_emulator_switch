#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "BrickCore.h"

// Sunplus SPLB32 (a 6502) with its HT24LC08 EEPROM on port D, Tamagotchi Connection V3.
// Generated from BrickEmuPy's cores/SPLB32.py (github.com/azya52/BrickEmuPy, CC0)
// by CONVERT_ROM/utils/brick_core_gen.py - edit that, not this.
class SPLB32 : public BrickCore {
public:
    SPLB32() : BrickCore("SPLB32") {}

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
    enum Ports { kPort_A = 1000, kPort_BF = 1001, kPort_CF = 1002, kPort_DF = 1003, kPort_IF = 1004, kPort_LCDRAM = 1005, kPort_NF = 1006, kPort_PA = 1007, kPort_PB = 1008, kPort_PC = 1009, kPort_PC16 = 1010, kPort_PD = 1011, kPort_RAM = 1012, kPort_SFR = 1013, kPort_SP = 1014, kPort_VF = 1015, kPort_X = 1016, kPort_Y = 1017, kPort_ZF = 1018, kPort_RES = 1999 };
    struct OpEntry { int64_t (SPLB32::*fn)(int64_t); int size; };
    struct IoEntry { int64_t (SPLB32::*get)(); int64_t (SPLB32::*set)(int64_t); };
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
    // SPLB32sound.py keeps two channels; Yokoi's speaker is one pin.
    int64_t snd_envelope_[2] = {0, 0}, snd_state_[2] = {1, 1};
    void snd_set_data(int64_t ch, int64_t ctrl, int64_t data) {
        if (ctrl & 0x40) {
            snd_envelope_[ch] = data & 0xFF;
            if (data == 0) set_speaker(false);
        } else {
            set_speaker((data & 0xFF) > 128);
        }
    }
    void snd_toggle(int64_t ch) {
        if (!snd_envelope_[ch]) return;
        snd_state_[ch] = -snd_state_[ch];
        set_speaker(snd_state_[ch] > 0);
    }
    // peripherals/HT24LC08.py. The .eeprom file's job is done by save states.
    void eeprom_port(int64_t port, int64_t value);
    std::vector<uint8_t> ee_mem_ = std::vector<uint8_t>(1024, 0);
    int64_t ee_state_ = 0, ee_bits_ = 0, ee_addr_ = 0, ee_scl_ = 1, ee_sda_ = 1, ee_ack_ = 0, ee_rx_ = 0, ee_tx_ = 0;
    int64_t py_get_rom_addr(int64_t v_addr);
    void pyreset();
    int64_t py_port_read(int64_t v_port);
    void pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level);
    void py_IRQ();
    void py_NMI();
    void py_timers_clock(double v_exec_cycles);
    double pyclock();
    int64_t py_set_sfr_io_portA_attr(int64_t v_value);
    int64_t py_set_sfr_io_portA_dir(int64_t v_value);
    int64_t py_get_sfr_io_portA_data();
    int64_t py_set_sfr_io_portA_data(int64_t v_value);
    int64_t py_set_sfr_io_portB_attr(int64_t v_value);
    int64_t py_set_sfr_io_portB_dir(int64_t v_value);
    int64_t py_get_sfr_io_portB_data();
    int64_t py_set_sfr_io_portB_data(int64_t v_value);
    int64_t py_set_sfr_io_portC_attr(int64_t v_value);
    int64_t py_set_sfr_io_portC_dir(int64_t v_value);
    int64_t py_get_sfr_io_portC_data();
    int64_t py_set_sfr_io_portC_data(int64_t v_value);
    int64_t py_set_sfr_io_portD_dir(int64_t v_value);
    int64_t py_set_sfr_io_portD_attr(int64_t v_value);
    int64_t py_get_sfr_io_portD_data();
    int64_t py_set_sfr_io_portD_data(int64_t v_value);
    int64_t py_get_sfr_audio_ch0_ctrl();
    int64_t py_set_sfr_audio_ch0_ctrl(int64_t v_value);
    int64_t py_get_sfr_audio_ch0_data();
    int64_t py_set_sfr_audio_ch0_data(int64_t v_value);
    int64_t py_get_sfr_audio_ch1_ctrl();
    int64_t py_set_sfr_audio_ch1_ctrl(int64_t v_value);
    int64_t py_get_sfr_audio_ch1_data();
    int64_t py_set_sfr_audio_ch1_data(int64_t v_value);
    int64_t py_get_sfr_bank_sel();
    int64_t py_set_sfr_bank_sel(int64_t v_value);
    int64_t py_get_sfr_wakeup_ctrl();
    int64_t py_set_sfr_wakeup_ctrl(int64_t v_value);
    int64_t py_get_sfr_timer_timebase_sel();
    int64_t py_set_sfr_timer_timebase_sel(int64_t v_value);
    int64_t py_get_sfr_timer_ctrl();
    int64_t py_set_sfr_timer_ctrl(int64_t v_value);
    int64_t py_set_sfr_clk_32768_en(int64_t v_value);
    int64_t py_get_sfr_nmi_ctrl();
    int64_t py_set_sfr_nmi_ctrl(int64_t v_value);
    int64_t py_get_sfr_int_ctrl();
    int64_t py_set_sfr_int_ctrl(int64_t v_value);
    int64_t py_get_sfr_clk_cpu_ctrl();
    int64_t py_set_sfr_clk_cpu_ctrl(int64_t v_value);
    int64_t py_set_sfr_clk_int_clear(int64_t v_value);
    int64_t py_get_sfr_timer_TM1L();
    int64_t py_set_sfr_timer_TM1L(int64_t v_value);
    int64_t py_get_sfr_timer_TM1H();
    int64_t py_set_sfr_timer_TM1H(int64_t v_value);
    int64_t py_get_sfr_timer_TM0L();
    int64_t py_set_sfr_timer_TM0L(int64_t v_value);
    int64_t py_get_sfr_timer_TM0H();
    int64_t py_set_sfr_timer_TM0H(int64_t v_value);
    int64_t py_get_sfr_lcd_ctrl1();
    int64_t py_set_sfr_lcd_ctrl1(int64_t v_value);
    int64_t py_get_sfr_specialfunc_config();
    int64_t py_set_sfr_specialfunc_config(int64_t v_value);
    int64_t py_get_sfr_aux_byte_mirror();
    int64_t py_set_sfr_aux_byte_mirror(int64_t v_value);
    int64_t py_get_sfr_aux_nibble_swap();
    int64_t py_set_sfr_aux_nibble_swap(int64_t v_value);
    int64_t py_get_sfr_dummy();
    void py_set_sfr_dummy(int64_t v_value);
    void py_write_mem(int64_t v_addr, int64_t v_value);
    int64_t py_read_mem(int64_t v_addr);
    int64_t py_get_ps();
    void py_set_ps(int64_t v_ps);
    void py_adc(int64_t v_operand);
    void py_sbc(int64_t v_operand);
    int64_t py_brk(int64_t = 0);
    int64_t py_ora_ind_x(int64_t = 0);
    int64_t py_ora_zp(int64_t = 0);
    int64_t py_asl_zp(int64_t = 0);
    int64_t py_php(int64_t = 0);
    int64_t py_ora_imm(int64_t = 0);
    int64_t py_asl_a(int64_t = 0);
    int64_t py_ora_abs(int64_t = 0);
    int64_t py_asl_abs(int64_t = 0);
    int64_t py_bpl(int64_t = 0);
    int64_t py_ora_ind_y(int64_t = 0);
    int64_t py_ora_zp_x(int64_t = 0);
    int64_t py_asl_zp_x(int64_t = 0);
    int64_t py_ora_abs_y(int64_t = 0);
    int64_t py_and_ind_x(int64_t = 0);
    int64_t py_ora_abs_x(int64_t = 0);
    int64_t py_asl_abs_x(int64_t = 0);
    int64_t py_clc(int64_t = 0);
    int64_t py_jsr_abs(int64_t = 0);
    int64_t py_and_abs_y(int64_t = 0);
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
    int64_t py_eor_zp_x(int64_t = 0);
    int64_t py_lsr_zp_x(int64_t = 0);
    int64_t py_cli(int64_t = 0);
    int64_t py_eor_abs_y(int64_t = 0);
    int64_t py_eor_abs_x(int64_t = 0);
    int64_t py_lsr_abs_x(int64_t = 0);
    int64_t py_rts(int64_t = 0);
    int64_t py_adc_ind_x(int64_t = 0);
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
    int64_t py_adc_zp_x(int64_t = 0);
    int64_t py_ror_zp_x(int64_t = 0);
    int64_t py_sei(int64_t = 0);
    int64_t py_adc_abs_y(int64_t = 0);
    int64_t py_adc_abs_x(int64_t = 0);
    int64_t py_ror_abs_x(int64_t = 0);
    int64_t py_sta_ind_x(int64_t = 0);
    int64_t py_sty_zp(int64_t = 0);
    int64_t py_sta_zp(int64_t = 0);
    int64_t py_stx_zp(int64_t = 0);
    int64_t py_dey(int64_t = 0);
    int64_t py_txa(int64_t = 0);
    int64_t py_sty_abs(int64_t = 0);
    int64_t py_sta_abs(int64_t = 0);
    int64_t py_stx_abs(int64_t = 0);
    int64_t py_bcc(int64_t = 0);
    int64_t py_sta_ind_y(int64_t = 0);
    int64_t py_sty_zp_x(int64_t = 0);
    int64_t py_sta_zp_x(int64_t = 0);
    int64_t py_stx_zp_y(int64_t = 0);
    int64_t py_tya(int64_t = 0);
    int64_t py_sta_abs_y(int64_t = 0);
    int64_t py_txs(int64_t = 0);
    int64_t py_sta_abs_x(int64_t = 0);
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
    int64_t py_cmp_zp_x(int64_t = 0);
    int64_t py_dec_zp_x(int64_t = 0);
    int64_t py_cld(int64_t = 0);
    int64_t py_cmp_abs_y(int64_t = 0);
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
    int64_t py_sbc_zp_x(int64_t = 0);
    int64_t py_inc_zp_x(int64_t = 0);
    int64_t py_sed(int64_t = 0);
    int64_t py_sbc_abs_y(int64_t = 0);
    int64_t py_sbc_abs_x(int64_t = 0);
    int64_t py_inc_abs_x(int64_t = 0);
    int64_t py_dummy(int64_t = 0);
    static const OpEntry T_execute[256];
    static const IoEntry* find_sfr_tbl(int64_t addr);
    int64_t f_instr_counter = 0;
    std::map<int64_t, int64_t> f_pullup_ext;
    std::map<int64_t, std::vector<int64_t>> f_port_input;
    int64_t f_clock = 0;
    double f_sub_clock_div = 0;
    double f_TM0_counter = 0;
    double f_TM1_counter = 0;
    double f_T1KHZ_counter = 0;
    int64_t f_T1KHZ = 0;
    double f_TM0 = 0;
    int64_t f_TM0_RELOAD = 0;
    double f_TM1 = 0;
    int64_t f_TM1_RELOAD = 0;
    int64_t f_PC = 0;
    int64_t f_A = 0;
    int64_t f_X = 0;
    int64_t f_Y = 0;
    int64_t f_SP = 0;
    int64_t f_NF = 0;
    int64_t f_VF = 0;
    int64_t f_BF = 0;
    int64_t f_DF = 0;
    int64_t f_IF = 0;
    int64_t f_ZF = 0;
    int64_t f_CF = 0;
    int64_t f_CPU_ENBL = 0;
    std::vector<int64_t> f_RAM;
    std::vector<int64_t> f_DPRAM;
    int64_t f_ROM_BANK = 0;
    int64_t f_WAKEUP_CTRL = 0;
    int64_t f_WAKEUPREQ = 0;
    int64_t f_TIME_BASE = 0;
    int64_t f_TIMER_CTRL = 0;
    int64_t f_CPU_DIV = 0;
    int64_t f_CLK32768_ENABLE = 0;
    std::map<int64_t, int64_t> f_PDIR;
    std::map<int64_t, int64_t> f_ATTR;
    std::map<int64_t, int64_t> f_PLATCH;
    int64_t f_IO_CTRL = 0;
    int64_t f_INT_CTRL = 0;
    int64_t f_IREQ = 0;
    int64_t f_NMI_CTRL = 0;
    int64_t f_NMIREQ = 0;
    int64_t f_AUDIO_CH0_CTRL = 0;
    int64_t f_AUDIO_CH0_DATA = 0;
    int64_t f_AUDIO_CH1_CTRL = 0;
    int64_t f_AUDIO_CH1_DATA = 0;
    int64_t f_BYTE_MIRROR = 0;
    int64_t f_NIBBLE_SWAP = 0;
    int64_t f_LCD_CTRL1 = 0;
    int64_t f_addr_reset = 0;
    int64_t f_addr_irq = 0;
    int64_t f_addr_nmi = 0;
};
