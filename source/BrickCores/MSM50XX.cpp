#include "MSM50XX.h"

#include <cstdlib>
#include <cstring>

namespace {

constexpr int kMclockDiv = 4;  // cycles per instruction (MSM50XX.py MCLOCK_DIV)

// Nelsonic Zelda's table - used if a container has none.
constexpr double kDefaultFreqDiv[16] = {0, 6, 8, 10, 6, 7, 0, 6, 0, 5.5, 0, 6.5, 0, 7.5, 0, 8.5};

// Generated from BrickEmuPy cores/MSM50XX.py's _execute table (opcode >> 4).
enum MsmOp : uint8_t {
    k_adc_acc_a0,
    k_adc_acc_ap,
    k_add_acc_a0,
    k_add_acc_ap,
    k_add_d_a0,
    k_add_d_ap,
    k_adjust_n_a0,
    k_adjust_n_ap,
    k_adrs_a0,
    k_adrs_ap,
    k_adrs_n,
    k_asl_a0,
    k_asl_ap,
    k_asr_a0,
    k_asr_ap,
    k_bcc_n,
    k_bcs_n,
    k_bgt_n,
    k_bic_acc_a0,
    k_bic_acc_ap,
    k_bic_d_a0,
    k_bic_d_ap,
    k_bis_acc_a0,
    k_bis_acc_ap,
    k_bis_d_a0,
    k_bis_d_ap,
    k_bit_acc_a0,
    k_bit_acc_ap,
    k_bit_d_a0,
    k_bit_d_ap,
    k_ble_n,
    k_bnz_n,
    k_buzzer_freq_sound,
    k_bze_n,
    k_chg_ap,
    k_chg_ax,
    k_cla,
    k_clc,
    k_clz,
    k_cmp_acc_a0,
    k_cmp_acc_ap,
    k_cmp_d_a0,
    k_cmp_d_ap,
    k_dsp_digit_a0,
    k_dsp_digit_ap,
    k_dspf_digit_ap,
    k_dummy,
    k_format_a0,
    k_format_ap,
    k_format_n,
    k_freq_n,
    k_halt,
    k_int16,
    k_int32,
    k_intmode_a0,
    k_intmode_ap,
    k_jmp_a0,
    k_jmp_adrs,
    k_jmp_ap,
    k_jmpio_a0,
    k_jmpio_ap,
    k_kswitch_a0,
    k_kswitch_ap,
    k_lmp_bkp_on_off,
    k_matrix_a0,
    k_matrix_ap,
    k_matrix_mn,
    k_mov_acc_ap,
    k_mov_acc_ax,
    k_mov_ap_acc,
    k_mov_ax_acc,
    k_mov_d_a0,
    k_mov_d_ap,
    k_nop,
    k_page_a0,
    k_page_n,
    k_rate_a0,
    k_rate_ap,
    k_ror_a0,
    k_ror_ap,
    k_rstrate,
    k_sbc_acc_a0,
    k_sbc_acc_ap,
    k_sea,
    k_sec,
    k_sez,
    k_sub_acc_a0,
    k_sub_acc_ap,
    k_sub_d_a0,
    k_sub_d_ap,
    k_switch_a0,
    k_switch_ap,
    k_xor_acc_a0,
    k_xor_acc_ap,
    k_xor_d_a0,
    k_xor_d_ap,
};
static const uint8_t kMsmOps[1024] = {
    k_nop, k_dummy, k_ror_a0, k_asr_a0, k_add_acc_a0, k_adc_acc_a0, k_bis_acc_a0, k_xor_acc_a0,
    k_dummy, k_clc, k_clz, k_cla, k_dummy, k_jmp_a0, k_bit_acc_a0, k_dummy,
    k_dummy, k_dummy, k_ror_ap, k_asr_ap, k_add_acc_ap, k_adc_acc_ap, k_bis_acc_ap, k_xor_acc_ap,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_jmp_ap, k_bit_acc_ap, k_dummy,
    k_dummy, k_dummy, k_dummy, k_asl_a0, k_sub_acc_a0, k_sbc_acc_a0, k_bic_acc_a0, k_dummy,
    k_dummy, k_sec, k_sez, k_sea, k_dummy, k_jmpio_a0, k_cmp_acc_a0, k_dummy,
    k_dummy, k_dummy, k_dummy, k_asl_ap, k_sub_acc_ap, k_sbc_acc_ap, k_bic_acc_ap, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_jmpio_ap, k_cmp_acc_ap, k_dummy,
    k_halt, k_lmp_bkp_on_off, k_matrix_mn, k_format_n, k_int16, k_page_n, k_adrs_n, k_dummy,
    k_rstrate, k_dummy, k_dummy, k_int32, k_buzzer_freq_sound, k_freq_n, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_bcs_n, k_bcs_n, k_dummy, k_dummy, k_bze_n, k_bze_n, k_ble_n, k_ble_n,
    k_bcc_n, k_bcc_n, k_dummy, k_dummy, k_bnz_n, k_bnz_n, k_bgt_n, k_bgt_n,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0,
    k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0,
    k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap,
    k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap,
    k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0,
    k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0, k_dsp_digit_a0,
    k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap,
    k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap, k_dsp_digit_ap,
    k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap,
    k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap,
    k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap,
    k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap,
    k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap,
    k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap,
    k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap,
    k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap, k_dspf_digit_ap,
    k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0,
    k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0, k_bis_d_a0,
    k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap,
    k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap, k_bis_d_ap,
    k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0,
    k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0, k_bic_d_a0,
    k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap,
    k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap, k_bic_d_ap,
    k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0,
    k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0, k_bit_d_a0,
    k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap,
    k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap, k_bit_d_ap,
    k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0,
    k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0, k_cmp_d_a0,
    k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap,
    k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap, k_cmp_d_ap,
    k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0,
    k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0, k_add_d_a0,
    k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap,
    k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap, k_add_d_ap,
    k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0,
    k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0, k_sub_d_a0,
    k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap,
    k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap, k_sub_d_ap,
    k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0,
    k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0, k_mov_d_a0,
    k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap,
    k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap, k_mov_d_ap,
    k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0,
    k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0, k_xor_d_a0,
    k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap,
    k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap, k_xor_d_ap,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs, k_jmp_adrs,
    k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0,
    k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0, k_adjust_n_a0,
    k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap,
    k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap, k_adjust_n_ap,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_switch_a0, k_kswitch_a0, k_dummy, k_intmode_a0, k_dummy, k_dummy, k_dummy,
    k_dummy, k_rate_a0, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_switch_ap, k_kswitch_ap, k_dummy, k_intmode_ap, k_dummy, k_dummy, k_dummy,
    k_dummy, k_rate_ap, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_matrix_a0, k_format_a0, k_dummy, k_page_a0, k_adrs_a0, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_matrix_ap, k_format_ap, k_dummy, k_dummy, k_adrs_ap, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_chg_ax, k_chg_ax, k_chg_ax, k_chg_ax, k_chg_ax, k_chg_ax, k_chg_ax, k_chg_ax,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_chg_ap, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_mov_acc_ax, k_mov_acc_ax, k_mov_acc_ax, k_mov_acc_ax, k_mov_acc_ax, k_mov_acc_ax, k_mov_acc_ax, k_mov_acc_ax,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_mov_acc_ap, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_mov_ax_acc, k_mov_ax_acc, k_mov_ax_acc, k_mov_ax_acc, k_mov_ax_acc, k_mov_ax_acc, k_mov_ax_acc, k_mov_ax_acc,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_mov_ap_acc, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
    k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy, k_dummy,
};

}  // namespace

void MSM50XX::load_options() {
    // "sound_freq_div_tbl" may hold fractions (5.5, 6.5 ...), so it's parsed
    // here as doubles rather than through Options::list().
    for (int i = 0; i < 16; i++) freq_div_[i] = kDefaultFreqDiv[i];
    if (options().has("sound_freq_div_tbl")) {
        const std::string& s = options().str("sound_freq_div_tbl");
        size_t start = 0;
        for (int i = 0; i < 16 && start <= s.size(); i++) {
            size_t comma = s.find(',', start);
            if (comma == std::string::npos) comma = s.size();
            freq_div_[i] = strtod(s.substr(start, comma - start).c_str(), nullptr);
            start = comma + 1;
        }
    }
    format_tbl_offset_ = (uint32_t)(rom().size() / 2) - 64;
    options_loaded_ = true;
}

void MSM50XX::reset() {
    if (!options_loaded_) load_options();
    acc_ = 0;
    pc_ = 0;
    cf_ = zf_ = halt_ = 0;
    tc_ = int16_ = int32_ = int_ = 0;
    pm_ = ps_ = pk_ = ld_ = 0;
    fmt_ = backup_ = page_ = 0;
    areg_ = 0xF;
    ram_.assign(kRamSize, 0);
    gram_.assign(kGramSize, 0);
    // (MSM50XX._reset leaves the timer dividers and the melody counters
    // alone - only the sound register is cleared.)
    sound_set(0);
}

uint8_t MSM50XX::display_byte(int index) const {
    if (index < 0 || index >= kGramSize) return 0;
    if ((gram_[15] & 1) || (gram_[31] & 1)) return 0xFF;
    return gram_[index];
}

int MSM50XX::port_id(const std::string& name) const {
    if (name == "PS") return kPS;
    if (name == "PK") return kPK;
    return -1;
}

void MSM50XX::port_input(int port, uint8_t mask, int level) {
    uint8_t* p = (port == kPS) ? &ps_ : (port == kPK) ? &pk_ : nullptr;
    if (!p) return;
    *p &= (uint8_t)~mask;
    if (level > 0) {
        *p |= mask;
        halt_ = 0;
    }
}

uint16_t MSM50XX::rom_word(uint32_t byte_addr) const {
    const std::vector<uint8_t>& r = rom();
    const size_t n = r.size();
    if (!n) return 0;
    return (uint16_t)((r[byte_addr % n] << 8) | r[(byte_addr + 1) % n]);
}

uint32_t MSM50XX::ap(uint16_t op) const {
    const uint32_t a = op & 0xF;
    return ((uint32_t)page_ << 4) | (a == 0xF ? areg_ : a);
}

uint32_t MSM50XX::ax(uint16_t op) const {
    const uint32_t a = op & 0xF;
    return (op & 0x70) | (a == 0xF ? areg_ : a);
}

// --- sound -----------------------------------------------------------------

void MSM50XX::sound_set(uint8_t value) {
    freg_ = value & 0x3;
    mreg_ = value >> 2;
    if (freg_ == 2) freg_ = freg4_;
}

void MSM50XX::sound_clock(int cycles) {
    if (!freg_) {
        snd_state_ = 0;
        melody_div2_ = melody_div1_ = 0;
        set_tone(0);
        return;
    }
    melody_div2_ -= cycles;
    if (melody_div2_ > 0) return;
    const int32_t clk = (int32_t)clock_hz();
    melody_div2_ += clk / 16;
    snd_state_ ^= 2;
    melody_div1_ -= melody_div2_;
    if (melody_div1_ <= 0) {
        melody_div1_ += clk / 2;
        snd_state_ ^= 1;
    }
    const double div = freq_div_[freg_ & 0xF];
    if (mreg_ == 0 && !(snd_state_ & 0x2)) {
        set_tone(0);
        freg_ = 0;
    } else if ((snd_state_ & mreg_) == mreg_ && div != 0) {
        set_tone(clk / div);
    } else {
        set_tone(0);
    }
}

// --- CPU ---------------------------------------------------------------------

int MSM50XX::execute() {
    if (!halt_) {
        const uint16_t op = rom_word(pc_ << 1);
        pc_++;
        exec(op);
    }
    if (int16_) {
        timer16_ -= kMclockDiv;
        if (timer16_ <= 0) {
            timer16_ += 2048;
            tc_ = (tc_ + 1) & 0xF;
            if (tc_ == 0) int_ |= 0x8;
            halt_ = 0;
        }
    }
    if (int32_) {
        timer32_ -= kMclockDiv;
        if (timer32_ <= 0) {
            timer32_ += 1024;
            tc_ = (tc_ + 1) & 0xF;
            if (tc_ == 0) int_ |= 0x8;
            halt_ = 0;
        }
    }
    sound_clock(kMclockDiv);
    return kMclockDiv;
}

void MSM50XX::exec(uint16_t op) {
    const uint8_t d = (op >> 4) & 0xF;
    switch (kMsmOps[(op >> 4) & 0x3FF]) {
        case k_nop:
        case k_dummy:
            break;

        // --- shifts / ALU with ACC ---
        case k_ror_a0: case k_ror_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_ror_a0 ? (op & 0xF) : ap(op));
            const uint8_t c = m & 1;
            acc_ = m = (uint8_t)((cf_ << 3) | (m >> 1));
            cf_ = c;
            zf_ = !acc_;
            break;
        }
        case k_asr_a0: case k_asr_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_asr_a0 ? (op & 0xF) : ap(op));
            cf_ = m & 1;
            acc_ = m = m >> 1;
            zf_ = !acc_;
            break;
        }
        case k_asl_a0: case k_asl_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_asl_a0 ? (op & 0xF) : ap(op));
            cf_ = m > 7;
            acc_ = m = (m << 1) & 0xF;
            zf_ = !acc_;
            break;
        }
        case k_add_acc_a0: case k_add_acc_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_add_acc_a0 ? (op & 0xF) : ap(op));
            const int r = m + acc_;
            acc_ = m = r & 0xF;
            cf_ = r > 15;
            zf_ = !acc_;
            break;
        }
        case k_adc_acc_a0: case k_adc_acc_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_adc_acc_a0 ? (op & 0xF) : ap(op));
            const int r = m + acc_ + cf_;
            acc_ = m = (uint8_t)(r % 10);
            cf_ = r >= 10;
            zf_ = !acc_;
            break;
        }
        case k_sub_acc_a0: case k_sub_acc_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_sub_acc_a0 ? (op & 0xF) : ap(op));
            cf_ = m < acc_;
            acc_ = m = (m - acc_) & 0xF;
            zf_ = !acc_;
            break;
        }
        case k_sbc_acc_a0: case k_sbc_acc_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_sbc_acc_a0 ? (op & 0xF) : ap(op));
            const int r = m - acc_ - cf_;
            cf_ = r < 0;
            acc_ = m = (uint8_t)(((r % 10) + 10) % 10);  // Python's modulo
            zf_ = !acc_;
            break;
        }
        case k_bis_acc_a0: case k_bis_acc_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_bis_acc_a0 ? (op & 0xF) : ap(op));
            acc_ = m = m | acc_;
            zf_ = !acc_;
            break;
        }
        case k_bic_acc_a0: case k_bic_acc_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_bic_acc_a0 ? (op & 0xF) : ap(op));
            acc_ = m = m & (uint8_t)~acc_;
            zf_ = !acc_;
            break;
        }
        case k_xor_acc_a0: case k_xor_acc_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_xor_acc_a0 ? (op & 0xF) : ap(op));
            acc_ = m = m ^ acc_;
            zf_ = !acc_;
            break;
        }
        case k_bit_acc_a0: zf_ = ((~ram_at(op & 0xF) & acc_) & 0xFF) > 0; break;
        case k_bit_acc_ap: zf_ = ((~ram_at(ap(op)) & acc_) & 0xFF) > 0; break;
        case k_cmp_acc_a0: zf_ = ram_at(op & 0xF) == acc_; break;
        case k_cmp_acc_ap: zf_ = ram_at(ap(op)) == acc_; break;

        // --- flags ---
        case k_clc: cf_ = 0; break;
        case k_clz: zf_ = 0; break;
        case k_cla: cf_ = zf_ = 0; break;
        case k_sec: cf_ = 1; break;
        case k_sez: zf_ = 1; break;
        case k_sea: cf_ = zf_ = 1; break;

        // --- jumps ---
        case k_jmp_a0: pc_ += ram_at(op & 0xF); break;
        case k_jmp_ap: pc_ += ram_at(ap(op)); break;
        case k_jmpio_a0: pc_ += ram_at(op & 0xF) & 0x7; break;
        case k_jmpio_ap: pc_ += ram_at(ap(op)) & 0x7; break;
        case k_jmp_adrs: pc_ = op & 0x7FF; break;
        case k_bcs_n: if (cf_) pc_ += op & 0x1F; break;
        case k_bze_n: if (zf_) pc_ += op & 0x1F; break;
        case k_ble_n: if (zf_ | cf_) pc_ += op & 0x1F; break;
        case k_bcc_n: if (!cf_) pc_ += op & 0x1F; break;
        case k_bnz_n: if (!zf_) pc_ += op & 0x1F; break;
        case k_bgt_n: if (!(zf_ | cf_)) pc_ += op & 0x1F; break;

        // --- control registers ---
        case k_halt: halt_ = 1; break;
        case k_lmp_bkp_on_off:
            if (op & 0x2) ld_ = 1;
            else if (op & 0x1) ld_ = 0;
            if (op & 0x8) backup_ = 1;
            else if (op & 0x4) backup_ = 0;
            break;
        case k_matrix_mn: pm_ = op & 0xF; break;
        case k_format_n: fmt_ = op & 0xF; break;
        case k_int16: int16_ = (op & 0x2) > 0; break;
        case k_page_n: page_ = op & 0xF; break;
        case k_adrs_n: areg_ = op & 0xF; break;
        case k_rstrate: tc_ = 0; break;
        case k_int32: int32_ = (op & 0x8) > 0; break;
        case k_buzzer_freq_sound: sound_set(op & 0xF); break;
        case k_freq_n: freg4_ = op & 0xF; break;
        case k_matrix_a0: pm_ = ram_at(op & 0xF); break;
        case k_matrix_ap: pm_ = ram_at(ap(op)); break;
        case k_format_a0: fmt_ = ram_at(op & 0xF); break;
        case k_format_ap: fmt_ = ram_at(ap(op)); break;
        case k_page_a0: page_ = ram_at(op & 0xF); break;
        case k_adrs_a0: areg_ = ram_at(op & 0xF); break;
        case k_adrs_ap: areg_ = ram_at(ap(op)); break;

        // --- display ---
        case k_dsp_digit_a0: case k_dsp_digit_ap: {
            const uint32_t a = kMsmOps[(op >> 4) & 0x3FF] == k_dsp_digit_a0 ? (op & 0xF) : ap(op);
            const uint32_t dg = ((op >> 5) & 0x10) | ((op >> 4) & 0xF);
            gram_[dg & (kGramSize - 1)] = (uint8_t)((ram_at(a) << 4) | acc_);
            break;
        }
        case k_dspf_digit_ap: {
            const uint32_t a = (op & 0x0100) ? ap(op) : (op & 0xF);
            const uint32_t dg = ((op >> 5) & 0x10) | ((op >> 4) & 0xF);
            const uint32_t offset = format_tbl_offset_ + (((fmt_ & 0x6u) << 3) | ram_at(a));
            uint16_t word = rom_word(offset * 2);
            if (fmt_ & 0x1) word >>= 7;
            gram_[dg & (kGramSize - 1)] = (uint8_t)((word << 1) & 0xFE);
            break;
        }

        // --- ALU with immediate ---
        case k_bis_d_a0: case k_bis_d_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_bis_d_a0 ? (op & 0xF) : ap(op));
            acc_ = m = m | d;
            zf_ = !acc_;
            break;
        }
        case k_bic_d_a0: case k_bic_d_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_bic_d_a0 ? (op & 0xF) : ap(op));
            acc_ = m = m & (uint8_t)~d;
            zf_ = !acc_;
            break;
        }
        case k_bit_d_a0: zf_ = ((~ram_at(op & 0xF) & d) & 0xFF) > 0; break;
        case k_bit_d_ap: zf_ = ((~ram_at(ap(op)) & d) & 0xFF) > 0; break;
        case k_cmp_d_a0: zf_ = ram_at(op & 0xF) == d; break;
        case k_cmp_d_ap: zf_ = ram_at(ap(op)) == d; break;
        case k_add_d_a0: case k_add_d_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_add_d_a0 ? (op & 0xF) : ap(op));
            acc_ = m = (m + d) & 0xF;
            cf_ = acc_ < d;
            zf_ = !acc_;
            break;
        }
        case k_sub_d_a0: case k_sub_d_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_sub_d_a0 ? (op & 0xF) : ap(op));
            cf_ = m < d;
            acc_ = m = (m - d) & 0xF;
            zf_ = !acc_;
            break;
        }
        case k_mov_d_a0: case k_mov_d_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_mov_d_a0 ? (op & 0xF) : ap(op));
            acc_ = m = d;
            zf_ = !d;
            break;
        }
        case k_xor_d_a0: case k_xor_d_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_xor_d_a0 ? (op & 0xF) : ap(op));
            acc_ = m = m ^ d;
            zf_ = !acc_;
            break;
        }
        case k_adjust_n_a0: case k_adjust_n_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_adjust_n_a0 ? (op & 0xF) : ap(op));
            const uint8_t value = m;
            const uint8_t mod = (uint8_t)((~d & 0xF) + 1);
            acc_ = m = value % mod;
            cf_ = value >= mod;
            zf_ = !acc_;
            break;
        }

        // --- inputs / status into RAM ---
        case k_switch_a0: case k_switch_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_switch_a0 ? (op & 0xF) : ap(op));
            acc_ = m = ps_;
            zf_ = !acc_;
            break;
        }
        case k_kswitch_a0: case k_kswitch_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_kswitch_a0 ? (op & 0xF) : ap(op));
            acc_ = m = pk_;
            zf_ = !acc_;
            break;
        }
        case k_intmode_a0: case k_intmode_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_intmode_a0 ? (op & 0xF) : ap(op));
            acc_ = m = int_;
            int_ = 0;
            zf_ = !acc_;
            break;
        }
        case k_rate_a0: case k_rate_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_rate_a0 ? (op & 0xF) : ap(op));
            acc_ = m = tc_;
            zf_ = !acc_;
            break;
        }

        // --- moves ---
        case k_chg_ax: case k_chg_ap: {
            uint8_t& m = ram_at(kMsmOps[(op >> 4) & 0x3FF] == k_chg_ax ? ax(op) : ap(op));
            const uint8_t b = acc_;
            acc_ = m;
            m = b;
            break;
        }
        case k_mov_acc_ax: ram_at(ax(op)) = acc_; break;
        case k_mov_acc_ap: ram_at(ap(op)) = acc_; break;
        case k_mov_ax_acc: acc_ = ram_at(ax(op)); zf_ = !acc_; break;
        case k_mov_ap_acc: acc_ = ram_at(ap(op)); zf_ = !acc_; break;
    }
}

void MSM50XX::serialize(BrickStateIO& io) {
    io.pod(acc_);
    io.pod(pc_);
    io.pod(cf_);
    io.pod(zf_);
    io.pod(halt_);
    io.pod(tc_);
    io.pod(int16_);
    io.pod(int32_);
    io.pod(int_);
    io.pod(pm_);
    io.pod(ps_);
    io.pod(pk_);
    io.pod(ld_);
    io.pod(fmt_);
    io.pod(backup_);
    io.pod(page_);
    io.pod(areg_);
    io.vec(ram_);
    io.vec(gram_);
    io.pod(timer16_);
    io.pod(timer32_);
    io.pod(freg_);
    io.pod(freg4_);
    io.pod(mreg_);
    io.pod(snd_state_);
    io.pod(melody_div1_);
    io.pod(melody_div2_);
}
