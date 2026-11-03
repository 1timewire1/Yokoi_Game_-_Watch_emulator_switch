#include "EM73000.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {

inline int64_t py_mod(int64_t a, int64_t b) { const int64_t r = a % b; return (r != 0 && ((r < 0) != (b < 0))) ? r + b : r; }
inline double py_mod(double a, double b) { const double r = std::fmod(a, b); return (r != 0 && ((r < 0) != (b < 0))) ? r + b : r; }
inline double py_mod(int64_t a, double b) { return py_mod((double)a, b); }
inline double py_mod(double a, int64_t b) { return py_mod(a, (double)b); }
inline int64_t py_div(int64_t a, int64_t b) { const int64_t q = a / b; return ((a % b != 0) && ((a < 0) != (b < 0))) ? q - 1 : q; }

template <class T>
void ser_vec(BrickStateIO& io, std::vector<T>& v) {
    uint32_t n = (uint32_t)v.size();
    io.pod(n);
    if (!io.ok()) return;
    if (!io.writing()) v.resize(n);
    if (n) io.bytes(v.data(), n * sizeof(T));
}

const int64_t k_TIMER_DIV[] = {1024, 16384, 262144, 4194304};
const int64_t k_TIME_BASE_DIV[] = {0, 0, 0, 0, 1024, 2048, 4096, 8192, 0, 0, 0, 0, 512, 256, 32768, 131072};
const int64_t k_MASK2IL[] = {32, 33, 38, 39, 40, 41, 46, 47, 48, 49, 54, 55, 56, 57, 62, 63};
const int64_t k_INT_ENTRYS[] = {12, 10, 8, 6, 4, 2};
const int64_t k_WARMUP_TIME[] = {262144, 16384, 65536, 8};

}  // namespace

int64_t EM73000::rom_get_byte(int64_t a) const {
    const int64_t n = rom_size();
    return rom()[(size_t)(((a % n) + n) % n)];
}

void EM73000::reset() {
    if (!inited_) {
        inited_ = true;
        init_();
    }
    py_reset();
}

void EM73000::init_() {
    f_P0_pullup_mask = options().num("port_pullup.P0", 0);
    f_P7_pullup_mask = options().num("port_pullup.P7", 0);
    f_P8_pullup_mask = 15;
    f_P0_wakeup_mask = options().num("port_wakeup.P0", 0);
    f_P8_wakeup_mask = options().num("port_wakeup.P8", 0);
    f_instr_counter = 0;
}

int64_t EM73000::execute_scaled() {
    const int64_t scaled = std::llround((double)pyclock() * kTickHz);
    return scaled > 0 ? scaled : 1;
}

uint8_t EM73000::display_byte(int i) const {
    if ((f_P16 & 4) || i < 0 || i >= 384) return 0;
    return (uint8_t)f_RAM[i];
}

int EM73000::port_id(const std::string& name) const {
    if (name == "ACC") return kPort_ACC;
    if (name == "CF") return kPort_CF;
    if (name == "DP") return kPort_DP;
    if (name == "EI") return kPort_EI;
    if (name == "HL") return kPort_HL;
    if (name == "IL") return kPort_IL;
    if (name == "IMASK") return kPort_IMASK;
    if (name == "IO") return kPort_IO;
    if (name == "MEMORY") return kPort_MEMORY;
    if (name == "P0") return kPort_P0;
    if (name == "P7") return kPort_P7;
    if (name == "P8") return kPort_P8;
    if (name == "PC") return kPort_PC;
    if (name == "RAM0") return kPort_RAM0;
    if (name == "RAM1") return kPort_RAM1;
    if (name == "SF") return kPort_SF;
    if (name == "SP") return kPort_SP;
    if (name == "TIMERA") return kPort_TIMERA;
    if (name == "TIMERB") return kPort_TIMERB;
    if (name == "ZF") return kPort_ZF;
    return -1;
}

void EM73000::port_input(int port, uint8_t mask, int level) {
    pypin_handler(port, mask, level);
}

void EM73000::serialize(BrickStateIO& io) {
    io.pod(f_P0_pullup_mask);
    io.pod(f_P7_pullup_mask);
    io.pod(f_P8_pullup_mask);
    io.pod(f_P0_wakeup_mask);
    io.pod(f_P8_wakeup_mask);
    io.pod(f_ACC);
    io.pod(f_PC);
    io.pod(f_SP);
    io.pod(f_DP);
    io.pod(f_HL);
    io.pod(f_EI);
    io.pod(f_CF);
    io.pod(f_ZF);
    io.pod(f_SF);
    io.pod(f_TIMERA);
    io.pod(f_TIMERB);
    io.pod(f_IL);
    io.pod(f_IMASK);
    ser_vec(io, f_RAM);
    io.pod(f_P0);
    io.pod(f_P4);
    io.pod(f_P5);
    io.pod(f_P6);
    io.pod(f_P7);
    io.pod(f_P8);
    io.pod(f_P9);
    io.pod(f_P16);
    io.pod(f_P23);
    io.pod(f_P24);
    io.pod(f_P25);
    io.pod(f_P27);
    io.pod(f_P28);
    io.pod(f_P29);
    io.pod(f_P30);
    io.pod(f_timerA_counter);
    io.pod(f_timerB_counter);
    io.pod(f_time_base_counter);
    io.pod(f_ram_bank);
    io.pod(f_instr_counter);
    io.pod(snd_basic_div_);
    io.pod(snd_div_);
    io.pod(snd_mode_);
}


void EM73000::py_reset() {
    f_ACC = 0;
    f_PC = 0;
    f_SP = 12;
    f_DP = 0;
    f_HL = 0;
    f_EI = 0;
    f_CF = 0;
    f_ZF = 0;
    f_SF = 1;
    f_TIMERA = 0;
    f_TIMERB = 0;
    f_IL = 0;
    f_IMASK = 0;
    f_RAM.assign((size_t)(384), 0);
    f_P0 = f_P0_pullup_mask;
    f_P4 = 15;
    f_P5 = 0;
    f_P6 = 0;
    f_P7 = f_P7_pullup_mask;
    f_P8 = f_P8_pullup_mask;
    f_P9 = 0;
    f_P16 = 0;
    f_P23 = 15;
    f_P24 = 15;
    f_P25 = 0;
    f_P27 = 0;
    f_P28 = 0;
    f_P29 = 0;
    f_P30 = 0;
    f_timerA_counter = 0;
    f_timerB_counter = 0;
    f_time_base_counter = 0;
    f_ram_bank = 0;
}

void EM73000::pyreset() {
    py_reset();
}

int64_t EM73000::py_set_io_dummy(int64_t v_value) {
    return 0;
}

int64_t EM73000::py_get_io_dummy() {
    return 0;
    return 0;
}

int64_t EM73000::py_get_io_P0() {
    return f_P0;
    return 0;
}

void EM73000::py_set_io_P0(int64_t v_value) {
}

int64_t EM73000::py_get_io_P4() {
    return f_P4;
    return 0;
}

int64_t EM73000::py_set_io_P5(int64_t v_value) {
    f_P5 = v_value;
    return 0;
}

int64_t EM73000::py_set_io_P6(int64_t v_value) {
    f_P6 = v_value;
    return 0;
}

int64_t EM73000::py_set_io_P4(int64_t v_value) {
    f_P4 = v_value;
    return 0;
}

int64_t EM73000::py_get_io_P7() {
    return f_P7;
    return 0;
}

int64_t EM73000::py_get_io_P8() {
    return f_P8;
    return 0;
}

int64_t EM73000::py_set_io_P8(int64_t v_value) {
    return 0;
}

int64_t EM73000::py_set_io_P9(int64_t v_value) {
    f_ram_bank = (256 * (((v_value & 8) > 0)));
    f_P9 = v_value;
    return 0;
}

int64_t EM73000::py_get_io_P9() {
    return f_P9;
    return 0;
}

int64_t EM73000::py_set_io_P16(int64_t v_value) {
    f_P16 = v_value;
    if ((v_value & 4)) {
        f_time_base_counter = 0;
    }
    return 0;
}

int64_t EM73000::py_set_io_P23(int64_t v_value) {
    snd_set_freq_div((((f_P24 << 4) | v_value) + 1));
    f_P23 = v_value;
    return 0;
}

int64_t EM73000::py_set_io_P24(int64_t v_value) {
    f_P24 = v_value;
    return 0;
}

int64_t EM73000::py_set_io_P25(int64_t v_value) {
    f_P25 = v_value;
    return 0;
}

int64_t EM73000::py_set_io_P27(int64_t v_value) {
    f_P27 = v_value;
    return 0;
}

int64_t EM73000::py_set_io_P28(int64_t v_value) {
    f_P28 = v_value;
    f_timerA_counter = 0;
    return 0;
}

int64_t EM73000::py_set_io_P29(int64_t v_value) {
    f_P29 = v_value;
    f_timerB_counter = 0;
    return 0;
}

int64_t EM73000::py_set_io_P30(int64_t v_value) {
    snd_set_basic_freq_div((4 << ((v_value & 12) >> 2)));
    snd_set_mode((v_value & 3));
    f_P30 = v_value;
    return 0;
}

void EM73000::pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level) {
    if (((v_level >= 0))) {
        py_pin_set(v_port, v_mask, v_level);
    } else {
        py_pin_release(v_port, v_mask);
    }
}

void EM73000::py_pin_set(int64_t v_port, int64_t v_pin, int64_t v_level) {
    int64_t v_pin_mask = 0, v_prev_P0 = 0, v_prev_P8 = 0;
    v_pin_mask = (1 << v_pin);
    if (((v_port == kPort_P0))) {
        v_prev_P0 = f_P0;
        f_P0 = (((~(v_pin_mask)) & f_P0) | (v_level << v_pin));
        if (((((f_P0_wakeup_mask & v_prev_P0) & v_pin_mask)) && ((~((f_P0 & v_pin_mask)))))) {
            f_P16 &= (~(4));
        }
    } else {
        if (((v_port == kPort_P7))) {
            f_P7 = (((~(v_pin_mask)) & f_P7) | (v_level << v_pin));
        } else {
            if (((v_port == kPort_P8))) {
                v_prev_P8 = f_P8;
                f_P8 = (((~(v_pin_mask)) & f_P8) | (v_level << v_pin));
                if (((((f_P8_wakeup_mask & v_prev_P8) & v_pin_mask)) && ((~((f_P8 & v_pin_mask)))))) {
                    f_P16 &= (~(4));
                }
                if ((((v_prev_P8 & v_pin_mask)) && ((~((f_P8 & v_pin_mask)))))) {
                    f_IL |= ((((v_pin == 0)) << 0) | (((v_pin == 2)) << 5));
                }
            }
        }
    }
}

void EM73000::py_pin_release(int64_t v_port, int64_t v_pin) {
    int64_t v_pin_mask = 0, v_prev_P0 = 0, v_prev_P8 = 0;
    v_pin_mask = (1 << v_pin);
    if (((v_port == kPort_P0))) {
        v_prev_P0 = f_P0;
        f_P0 = ((f_P0 & (~(v_pin_mask))) | (f_P0_pullup_mask & v_pin_mask));
        if (((((f_P0_wakeup_mask & v_prev_P0) & v_pin_mask)) && ((~((f_P0 & v_pin_mask)))))) {
            f_P16 &= (~(4));
        }
    } else {
        if (((v_port == kPort_P7))) {
            f_P7 = ((f_P7 & (~(v_pin_mask))) | (f_P7_pullup_mask & v_pin_mask));
        } else {
            if (((v_port == kPort_P8))) {
                v_prev_P8 = f_P8;
                f_P8 = ((f_P8 & (~(v_pin_mask))) | (f_P8_pullup_mask & v_pin_mask));
                if (((((f_P8_wakeup_mask & v_prev_P8) & v_pin_mask)) && ((~((f_P8 & v_pin_mask)))))) {
                    f_P16 &= (~(4));
                }
            }
        }
    }
}

void EM73000::py_process_timer(int64_t v_exec_cycles) {
    if ((((f_P28 & 12) == 8))) {
        f_timerA_counter -= v_exec_cycles;
        if (((f_timerA_counter <= 0))) {
            f_timerA_counter += k_TIMER_DIV[(f_P28 & 3)];
            f_TIMERA = ((f_TIMERA + 1) & 4095);
            if (((f_TIMERA == 0))) {
                f_IL |= (1 << 3);
            }
        }
    }
    if ((((f_P29 & 12) == 8))) {
        f_timerB_counter -= v_exec_cycles;
        if (((f_timerB_counter <= 0))) {
            f_timerB_counter += k_TIMER_DIV[(f_P29 & 3)];
            f_TIMERB = ((f_TIMERB + 1) & 4095);
            if (((f_TIMERB == 0))) {
                f_IL |= (1 << 2);
            }
        }
    }
    if (((k_TIME_BASE_DIV[f_P25] > 0))) {
        f_time_base_counter -= v_exec_cycles;
        if (((f_time_base_counter <= 0))) {
            f_time_base_counter += k_TIME_BASE_DIV[f_P25];
            f_IL |= (1 << 1);
        }
    }
}

void EM73000::py_interrupt() {
    int64_t v_id = 0, v_pc = 0, v_sp = 0;
    for (v_id = 0; v_id < 6; v_id++) {
        if (((f_IL & (32 >> v_id)) & k_MASK2IL[f_IMASK])) {
            v_pc = f_PC;
            v_sp = (192 + (f_SP << 2));
            f_RAM[v_sp] = (((((f_CF & 1) << 3) | ((f_ZF & 1) << 2)) | ((f_SF & 1) << 1)) | ((v_pc >> 12) & 1));
            f_RAM[(v_sp + 1)] = ((v_pc >> 8) & 15);
            f_RAM[(v_sp + 2)] = ((v_pc >> 4) & 15);
            f_RAM[(v_sp + 3)] = (v_pc & 15);
            f_SP = ((f_SP - 1) & 15);
            f_PC = k_INT_ENTRYS[(5 - v_id)];
            f_SF = 1;
            f_EI = 0;
            f_IL &= (~((32 >> v_id)));
            return;
        }
    }
}

int64_t EM73000::pyclock() {
    int64_t v_exec_cycles = 0, v_opcode = 0;
    if (f_EI) {
        py_interrupt();
    }
    if ((!((f_P16 & 4)))) {
        v_opcode = rom_get_byte(f_PC);
        v_exec_cycles = (this->*T_execute[v_opcode].fn)(v_opcode);
        f_instr_counter += 1;
        py_process_timer(v_exec_cycles);
        return v_exec_cycles;
    }
    return k_WARMUP_TIME[(f_P16 & 3)];
    return 0;
}

void EM73000::py_write_ram(int64_t v_i, int64_t v_value) {
    v_i &= 255;
    if (((v_i < 244))) {
        f_RAM[(f_ram_bank + v_i)] = v_value;
    } else {
        if (((v_i == 244))) {
            f_TIMERA = ((f_TIMERA & 4080) | v_value);
        } else {
            if (((v_i == 245))) {
                f_TIMERA = ((f_TIMERA & 3855) | (v_value << 4));
            } else {
                if (((v_i == 246))) {
                    f_TIMERA = ((f_TIMERA & 255) | (v_value << 8));
                } else {
                    if (((v_i == 248))) {
                        f_TIMERB = ((f_TIMERB & 4080) | v_value);
                    } else {
                        if (((v_i == 249))) {
                            f_TIMERB = ((f_TIMERB & 3855) | (v_value << 4));
                        } else {
                            if (((v_i == 250))) {
                                f_TIMERB = ((f_TIMERB & 255) | (v_value << 8));
                            } else {
                                if (((v_i == 252))) {
                                    f_DP = ((f_DP & 4080) | v_value);
                                } else {
                                    if (((v_i == 253))) {
                                        f_DP = ((f_DP & 3855) | (v_value << 4));
                                    } else {
                                        if (((v_i == 254))) {
                                            f_DP = ((f_DP & 255) | (v_value << 8));
                                        } else {
                                            if (((v_i == 255))) {
                                                f_SP = v_value;
                                            } else {
                                                f_RAM[(f_ram_bank + v_i)] = v_value;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int64_t EM73000::py_read_ram(int64_t v_i) {
    v_i &= 255;
    if (((v_i < 244))) {
        return f_RAM[(f_ram_bank + v_i)];
    } else {
        if (((v_i == 244))) {
            return (f_TIMERA & 15);
        } else {
            if (((v_i == 245))) {
                return ((f_TIMERA >> 4) & 15);
            } else {
                if (((v_i == 246))) {
                    return ((f_TIMERA >> 8) & 15);
                } else {
                    if (((v_i == 248))) {
                        return (f_TIMERB & 15);
                    } else {
                        if (((v_i == 249))) {
                            return ((f_TIMERB >> 4) & 15);
                        } else {
                            if (((v_i == 250))) {
                                return ((f_TIMERB >> 8) & 15);
                            } else {
                                if (((v_i == 252))) {
                                    return (f_DP & 15);
                                } else {
                                    if (((v_i == 253))) {
                                        return ((f_DP >> 4) & 15);
                                    } else {
                                        if (((v_i == 254))) {
                                            return ((f_DP >> 8) & 15);
                                        } else {
                                            if (((v_i == 255))) {
                                                return f_SP;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return f_RAM[(f_ram_bank + v_i)];
    return 0;
}

int64_t EM73000::py_sbr_a(int64_t v_opcode) {
    f_PC = ((f_PC + 1) & 8191);
    if (f_SF) {
        f_PC = ((f_PC & 8128) | (v_opcode & 63));
    }
    f_SF = 1;
    return 8;
    return 0;
}

int64_t EM73000::py_lcall_a(int64_t v_opcode) {
    int64_t v_pc = 0, v_sp = 0;
    v_pc = (f_PC + 2);
    v_sp = (192 + (f_SP << 2));
    f_RAM[v_sp] = (v_pc >> 12);
    f_RAM[(v_sp + 1)] = ((v_pc >> 8) & 15);
    f_RAM[(v_sp + 2)] = ((v_pc >> 4) & 15);
    f_RAM[(v_sp + 3)] = (v_pc & 15);
    f_PC = (((v_opcode & 7) << 8) | rom_get_byte(((f_PC + 1) & 8191)));
    f_SP = ((f_SP - 1) & 15);
    return 16;
    return 0;
}

int64_t EM73000::py_std_k_y(int64_t v_opcode) {
    int64_t v_k = 0, v_ky = 0, v_y = 0;
    v_ky = rom_get_byte(((f_PC + 1) & 8191));
    v_k = ((v_ky >> 4) & 15);
    v_y = (v_ky & 15);
    f_RAM[v_y] = v_k;
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_add_k_y(int64_t v_opcode) {
    int64_t v_k = 0, v_ky = 0, v_y = 0;
    v_ky = rom_get_byte(((f_PC + 1) & 8191));
    v_k = ((v_ky >> 4) & 15);
    v_y = (v_ky & 15);
    f_RAM[v_y] = ((f_RAM[v_y] + v_k) & 15);
    f_ZF = ((f_RAM[v_y] == 0));
    f_SF = ((f_RAM[v_y] >= v_k));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_out_k_p(int64_t v_opcode) {
    int64_t v_k = 0, v_kp = 0, v_p = 0;
    v_kp = rom_get_byte(((f_PC + 1) & 8191));
    v_k = ((v_kp >> 4) & 15);
    v_p = (v_kp & 15);
    (this->*find_io_tbl(v_p)->set)(v_k);
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_cmp_k_y(int64_t v_opcode) {
    int64_t v_k = 0, v_ky = 0, v_y = 0;
    v_ky = rom_get_byte(((f_PC + 1) & 8191));
    v_k = ((v_ky >> 4) & 15);
    v_y = (v_ky & 15);
    f_CF = ((v_k >= f_RAM[v_y]));
    f_ZF = ((v_k == f_RAM[v_y]));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_exhl_x(int64_t v_opcode) {
    int64_t v_hl_tmp = 0, v_x = 0;
    v_x = (rom_get_byte(((f_PC + 1) & 8191)) & 255);
    v_hl_tmp = f_HL;
    f_HL = ((py_read_ram((v_x + 1)) << 4) | py_read_ram(v_x));
    py_write_ram(v_x, (v_hl_tmp & 15));
    py_write_ram((v_x + 1), ((v_hl_tmp >> 4) & 15));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_rti(int64_t v_opcode) {
    int64_t v_sp = 0;
    f_SP = ((f_SP + 1) & 15);
    v_sp = (192 + (f_SP << 2));
    f_PC = (((((f_RAM[v_sp] & 1) << 12) | (f_RAM[(v_sp + 1)] << 8)) | (f_RAM[(v_sp + 2)] << 4)) | f_RAM[(v_sp + 3)]);
    f_CF = (((f_RAM[v_sp] & 8) > 0));
    f_ZF = (((f_RAM[v_sp] & 4) > 0));
    f_SF = (((f_RAM[v_sp] & 2) > 0));
    f_EI = 1;
    return 16;
    return 0;
}

int64_t EM73000::py_ldhl_x(int64_t v_opcode) {
    int64_t v_x = 0;
    v_x = (rom_get_byte(((f_PC + 1) & 8191)) & 255);
    f_HL = ((py_read_ram((v_x + 1)) << 4) | py_read_ram(v_x));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_ret(int64_t v_opcode) {
    int64_t v_sp = 0;
    f_SP = ((f_SP + 1) & 15);
    v_sp = (192 + (f_SP << 2));
    f_PC = (((((f_RAM[v_sp] & 1) << 12) | (f_RAM[(v_sp + 1)] << 8)) | (f_RAM[(v_sp + 2)] << 4)) | f_RAM[(v_sp + 3)]);
    return 16;
    return 0;
}

int64_t EM73000::py_rlca(int64_t v_opcode) {
    int64_t v_cf = 0;
    v_cf = (((f_ACC & 8) > 0));
    f_ACC = (((f_ACC << 1) & 14) | (f_CF & 1));
    f_CF = v_cf;
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - v_cf);
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_rrca(int64_t v_opcode) {
    int64_t v_cf = 0;
    v_cf = (((f_ACC & 1) > 0));
    f_ACC = (((f_ACC >> 1) & 7) | ((f_CF << 3) & 8));
    f_CF = v_cf;
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - v_cf);
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_ttcfs(int64_t v_opcode) {
    f_SF = f_CF;
    f_CF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_tfcfc(int64_t v_opcode) {
    f_SF = (1 - f_CF);
    f_CF = 0;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_slbr1_a(int64_t v_opcode) {
    f_PC = ((f_PC + 3) & 8191);
    if (f_SF) {
        f_PC = (4096 | (rom_get_word(((f_PC - 2) & 8191)) & 4095));
    }
    f_SF = 1;
    return 24;
    return 0;
}

int64_t EM73000::py_slbr0_a(int64_t v_opcode) {
    f_PC = ((f_PC + 3) & 8191);
    if (f_SF) {
        f_PC = (rom_get_word(((f_PC - 2) & 8191)) & 4095);
    }
    f_SF = 1;
    return 24;
    return 0;
}

int64_t EM73000::py_nop(int64_t v_opcode) {
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_exam(int64_t v_opcode) {
    int64_t v_acc_tmp = 0;
    v_acc_tmp = f_ACC;
    f_ACC = py_read_ram(f_HL);
    py_write_ram(f_HL, v_acc_tmp);
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_stam(int64_t v_opcode) {
    py_write_ram(f_HL, f_ACC);
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_ldam(int64_t v_opcode) {
    f_ACC = py_read_ram(f_HL);
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_tzs(int64_t v_opcode) {
    f_SF = f_ZF;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_deca(int64_t v_opcode) {
    f_ACC = ((f_ACC - 1) & 15);
    f_ZF = ((f_ACC == 0));
    f_SF = ((f_ACC != 15));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_decm(int64_t v_opcode) {
    int64_t v_value = 0;
    v_value = ((py_read_ram(f_HL) - 1) & 15);
    py_write_ram(f_HL, v_value);
    f_ZF = ((v_value == 0));
    f_SF = ((v_value != 15));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_inca(int64_t v_opcode) {
    f_ACC = ((f_ACC + 1) & 15);
    f_ZF = ((f_ACC == 0));
    f_SF = ((f_ACC != 0));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_incm(int64_t v_opcode) {
    int64_t v_value = 0;
    v_value = ((py_read_ram(f_HL) + 1) & 15);
    py_write_ram(f_HL, v_value);
    f_ZF = ((v_value == 0));
    f_SF = ((v_value != 0));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_clpl(int64_t v_opcode) {
    int64_t v_bit = 0, v_new_value = 0, v_port = 0;
    v_bit = (f_HL & 3);
    v_port = (((f_HL & 12) >> 2) + 4);
    v_new_value = ((this->*find_io_tbl(v_port)->get)() & (~((1 << v_bit))));
    (this->*find_io_tbl(v_port)->set)(v_new_value);
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_tfpl(int64_t v_opcode) {
    int64_t v_bit = 0, v_port = 0;
    v_bit = (f_HL & 3);
    v_port = (((f_HL & 12) >> 2) + 4);
    f_SF = ((((this->*find_io_tbl(v_port)->get)() & (1 << v_bit)) == 0));
    f_PC = ((f_PC + 1) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_sepl(int64_t v_opcode) {
    int64_t v_bit = 0, v_new_value = 0, v_port = 0;
    v_bit = (f_HL & 3);
    v_port = (((f_HL & 12) >> 2) + 4);
    v_new_value = ((this->*find_io_tbl(v_port)->get)() | (1 << v_bit));
    (this->*find_io_tbl(v_port)->set)(v_new_value);
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_cil(int64_t v_opcode) {
    int64_t v_id = 0;
    v_id = ((rom_get_byte(((f_PC + 1) & 8191)) >> 6) & 3);
    return (this->*T_execute_cil[v_id].fn)(v_opcode);
    return 0;
}

int64_t EM73000::py_cil_r(int64_t v_opcode) {
    int64_t v_r = 0;
    v_r = (rom_get_byte(((f_PC + 1) & 8191)) & 63);
    f_IL &= v_r;
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_dicil_r(int64_t v_opcode) {
    int64_t v_r = 0;
    v_r = (rom_get_byte(((f_PC + 1) & 8191)) & 63);
    f_IL &= v_r;
    f_EI = 0;
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_eicil_r(int64_t v_opcode) {
    int64_t v_r = 0;
    v_r = (rom_get_byte(((f_PC + 1) & 8191)) & 63);
    f_IL &= v_r;
    f_EI = 1;
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_exal(int64_t v_opcode) {
    int64_t v_acc_tmp = 0;
    v_acc_tmp = f_ACC;
    f_ACC = (f_HL & 15);
    f_HL = ((f_HL & 240) | v_acc_tmp);
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_ldax(int64_t v_opcode) {
    f_ACC = (rom_get_byte((4096 | f_DP)) & 15);
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_exah(int64_t v_opcode) {
    int64_t v_acc_tmp = 0;
    v_acc_tmp = f_ACC;
    f_ACC = ((f_HL >> 4) & 15);
    f_HL = ((f_HL & 15) | (v_acc_tmp << 4));
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_ldaxi(int64_t v_opcode) {
    f_ACC = ((rom_get_byte((4096 | f_DP)) >> 4) & 15);
    f_DP = ((f_DP + 1) & 4095);
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_exa_x(int64_t v_opcode) {
    int64_t v_acc_tmp = 0, v_x = 0;
    v_x = rom_get_byte(((f_PC + 1) & 8191));
    v_acc_tmp = f_ACC;
    f_ACC = py_read_ram(v_x);
    py_write_ram(v_x, v_acc_tmp);
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_sta_x(int64_t v_opcode) {
    py_write_ram(rom_get_byte(((f_PC + 1) & 8191)), f_ACC);
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_lda_x(int64_t v_opcode) {
    f_ACC = py_read_ram(rom_get_byte(((f_PC + 1) & 8191)));
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_cmpa_x(int64_t v_opcode) {
    int64_t v_value = 0;
    v_value = py_read_ram(rom_get_byte(((f_PC + 1) & 8191)));
    f_CF = ((v_value >= f_ACC));
    f_ZF = ((v_value == f_ACC));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_bit_y_b(int64_t v_opcode) {
    int64_t v_id = 0;
    v_id = ((rom_get_byte(((f_PC + 1) & 8191)) >> 6) & 3);
    return (this->*T_execute_bit_y_b[v_id].fn)(v_opcode);
    return 0;
}

int64_t EM73000::py_tf_y_b(int64_t v_opcode) {
    int64_t v_b = 0, v_by = 0, v_y = 0;
    v_by = rom_get_byte(((f_PC + 1) & 8191));
    v_b = ((v_by >> 4) & 3);
    v_y = (v_by & 15);
    f_SF = (((f_RAM[v_y] & (1 << v_b)) == 0));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_set_y_b(int64_t v_opcode) {
    int64_t v_b = 0, v_by = 0, v_y = 0;
    v_by = rom_get_byte(((f_PC + 1) & 8191));
    v_b = ((v_by >> 4) & 3);
    v_y = (v_by & 15);
    f_RAM[v_y] = (f_RAM[v_y] | (1 << v_b));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_tt_y_b(int64_t v_opcode) {
    int64_t v_b = 0, v_by = 0, v_y = 0;
    v_by = rom_get_byte(((f_PC + 1) & 8191));
    v_b = ((v_by >> 4) & 3);
    v_y = (v_by & 15);
    f_SF = (((f_RAM[v_y] & (1 << v_b)) > 0));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_clr_y_b(int64_t v_opcode) {
    int64_t v_b = 0, v_by = 0, v_y = 0;
    v_by = rom_get_byte(((f_PC + 1) & 8191));
    v_b = ((v_by >> 4) & 3);
    v_y = (v_by & 15);
    f_RAM[v_y] = (f_RAM[v_y] & (~((1 << v_b))));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_bit_p_b(int64_t v_opcode) {
    int64_t v_id = 0;
    v_id = ((rom_get_byte(((f_PC + 1) & 8191)) >> 6) & 3);
    return (this->*T_execute_bit_p_b[v_id].fn)(v_opcode);
    return 0;
}

int64_t EM73000::py_tfp_p_b(int64_t v_opcode) {
    int64_t v_b = 0, v_bp = 0, v_p = 0;
    v_bp = rom_get_byte(((f_PC + 1) & 8191));
    v_b = ((v_bp >> 4) & 3);
    v_p = (v_bp & 15);
    f_SF = ((((this->*find_io_tbl(v_p)->get)() & (1 << v_b)) == 0));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_sep_p_b(int64_t v_opcode) {
    int64_t v_b = 0, v_bp = 0, v_p = 0;
    v_bp = rom_get_byte(((f_PC + 1) & 8191));
    v_b = ((v_bp >> 4) & 3);
    v_p = (v_bp & 15);
    (this->*find_io_tbl(v_p)->set)(((this->*find_io_tbl(v_p)->get)() | (1 << v_b)));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_ttp_p_b(int64_t v_opcode) {
    int64_t v_b = 0, v_bp = 0, v_p = 0;
    v_bp = rom_get_byte(((f_PC + 1) & 8191));
    v_b = ((v_bp >> 4) & 3);
    v_p = (v_bp & 15);
    f_SF = ((((this->*find_io_tbl(v_p)->get)() & (1 << v_b)) > 0));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_clp_p_b(int64_t v_opcode) {
    int64_t v_b = 0, v_bp = 0, v_p = 0;
    v_bp = rom_get_byte(((f_PC + 1) & 8191));
    v_b = ((v_bp >> 4) & 3);
    v_p = (v_bp & 15);
    (this->*find_io_tbl(v_p)->set)(((this->*find_io_tbl(v_p)->get)() & (~((1 << v_b)))));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_math_k(int64_t v_opcode) {
    int64_t v_id = 0;
    v_id = ((rom_get_byte(((f_PC + 1) & 8191)) >> 4) & 15);
    return (this->*T_execute_math_k[v_id].fn)(v_opcode);
    return 0;
}

int64_t EM73000::py_addl_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_HL = ((f_HL & 240) | ((f_HL + v_k) & 15));
    f_ZF = (((f_HL & 15) == 0));
    f_SF = (((f_HL & 15) >= v_k));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_cmpl_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_ZF = ((v_k == (f_HL & 15)));
    f_SF = ((v_k >= (f_HL & 15)));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_ora_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_ACC |= v_k;
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_adda_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_ACC = ((f_ACC + v_k) & 15);
    f_ZF = ((f_ACC == 0));
    f_SF = ((f_ACC >= v_k));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_anda_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_ACC &= v_k;
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_suba_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_ACC = ((v_k - f_ACC) & 15);
    f_ZF = ((f_ACC == 0));
    f_SF = ((v_k >= f_ACC));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_addh_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_HL = ((f_HL + (v_k << 4)) & 255);
    f_ZF = (((f_HL >> 4) == 0));
    f_SF = (((f_HL >> 4) >= v_k));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_cmph_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_ZF = (((f_HL >> 4) == v_k));
    f_SF = ((v_k >= (f_HL >> 4)));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_orm_k(int64_t v_opcode) {
    int64_t v_k = 0, v_value = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    v_value = (py_read_ram(f_HL) | v_k);
    py_write_ram(f_HL, v_value);
    f_ZF = ((v_value == 0));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_addm_k(int64_t v_opcode) {
    int64_t v_k = 0, v_value = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    v_value = ((py_read_ram(f_HL) + v_k) & 15);
    py_write_ram(f_HL, v_value);
    f_ZF = ((v_value == 0));
    f_SF = ((v_value >= v_k));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_andm_k(int64_t v_opcode) {
    int64_t v_k = 0, v_value = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    v_value = (py_read_ram(f_HL) & v_k);
    py_write_ram(f_HL, v_value);
    f_ZF = ((v_value == 0));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_subm_k(int64_t v_opcode) {
    int64_t v_k = 0, v_value = 0;
    v_k = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    v_value = ((v_k - py_read_ram(f_HL)) & 15);
    py_write_ram(f_HL, v_value);
    f_ZF = ((v_value == 0));
    f_SF = ((v_k >= v_value));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_io_p(int64_t v_opcode) {
    int64_t v_id = 0;
    v_id = ((rom_get_byte(((f_PC + 1) & 8191)) >> 6) & 3);
    return (this->*T_execute_io_p[v_id].fn)(v_opcode);
    return 0;
}

int64_t EM73000::py_ina_p(int64_t v_opcode) {
    int64_t v_p = 0;
    v_p = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    f_ACC = (this->*find_io_tbl(v_p)->get)();
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_inm_p(int64_t v_opcode) {
    int64_t v_p = 0;
    v_p = (rom_get_byte(((f_PC + 1) & 8191)) & 15);
    py_write_ram(f_HL, (this->*find_io_tbl(v_p)->get)());
    f_SF = ((py_read_ram(f_HL) != 0));
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_outa_p(int64_t v_opcode) {
    int64_t v_p = 0;
    v_p = (rom_get_byte(((f_PC + 1) & 8191)) & 31);
    (this->*find_io_tbl(v_p)->set)(f_ACC);
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_outm_p(int64_t v_opcode) {
    int64_t v_p = 0;
    v_p = (rom_get_byte((f_PC + 1)) & 31);
    (this->*find_io_tbl(v_p)->set)(py_read_ram(f_HL));
    f_SF = 1;
    f_PC = ((f_PC + 2) & 8191);
    return 16;
    return 0;
}

int64_t EM73000::py_adcam(int64_t v_opcode) {
    int64_t v_value = 0;
    v_value = py_read_ram(f_HL);
    f_ACC = (((f_ACC + v_value) + f_CF) & 15);
    f_CF = ((f_ACC < (v_value + f_CF)));
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - f_CF);
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_addam(int64_t v_opcode) {
    int64_t v_value = 0;
    v_value = py_read_ram(f_HL);
    f_ACC = ((f_ACC + v_value) & 15);
    f_ZF = ((f_ACC == 0));
    f_SF = ((f_ACC >= v_value));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_sbcam(int64_t v_opcode) {
    int64_t v_value = 0;
    v_value = py_read_ram(f_HL);
    f_ACC = (((v_value - f_ACC) - ((f_CF == 0))) & 15);
    f_CF = ((v_value >= (f_ACC + ((f_CF == 0)))));
    f_ZF = ((f_ACC == 0));
    f_SF = f_CF;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_cmpam(int64_t v_opcode) {
    int64_t v_value = 0;
    v_value = py_read_ram(f_HL);
    f_CF = ((v_value >= f_ACC));
    f_ZF = ((v_value == f_ACC));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_tla(int64_t v_opcode) {
    f_ACC = (f_HL & 15);
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_exae(int64_t v_opcode) {
    int64_t v_acc_tmp = 0;
    v_acc_tmp = f_ACC;
    f_ACC = f_IMASK;
    f_IMASK = v_acc_tmp;
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_tha(int64_t v_opcode) {
    f_ACC = (f_HL >> 4);
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_oram(int64_t v_opcode) {
    f_ACC |= py_read_ram(f_HL);
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_xoram(int64_t v_opcode) {
    f_ACC ^= py_read_ram(f_HL);
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_andam(int64_t v_opcode) {
    f_ACC &= py_read_ram(f_HL);
    f_ZF = ((f_ACC == 0));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_decl(int64_t v_opcode) {
    f_HL = ((f_HL & 240) | ((f_HL - 1) & 15));
    f_ZF = (((f_HL & 15) == 0));
    f_SF = (((f_HL & 15) != 15));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_stamd(int64_t v_opcode) {
    py_write_ram(f_HL, f_ACC);
    f_HL = ((f_HL & 240) | ((f_HL - 1) & 15));
    f_ZF = (((f_HL & 15) == 0));
    f_SF = (((f_HL & 15) != 15));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_incl(int64_t v_opcode) {
    f_HL = ((f_HL & 240) | ((f_HL + 1) & 15));
    f_ZF = (((f_HL & 15) == 0));
    f_SF = (((f_HL & 15) != 0));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_stami(int64_t v_opcode) {
    py_write_ram(f_HL, f_ACC);
    f_HL = ((f_HL & 240) | ((f_HL + 1) & 15));
    f_ZF = (((f_HL & 15) == 0));
    f_SF = (((f_HL & 15) != 0));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_ldl_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (v_opcode & 15);
    f_HL = ((f_HL & 240) | v_k);
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_ldh_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (v_opcode & 15);
    f_HL = ((f_HL & 15) | (v_k << 4));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_stdmi_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (v_opcode & 15);
    py_write_ram(f_HL, v_k);
    f_HL = ((f_HL & 240) | ((f_HL + 1) & 15));
    f_ZF = (((f_HL & 15) == 0));
    f_SF = (((f_HL & 15) != 0));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_cmpia_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (v_opcode & 15);
    f_CF = ((v_k >= f_ACC));
    f_ZF = ((v_k == f_ACC));
    f_SF = (1 - f_ZF);
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_lbr_a(int64_t v_opcode) {
    int64_t v_a = 0;
    f_PC = ((f_PC + 2) & 8191);
    if (f_SF) {
        v_a = (((v_opcode << 8) | rom_get_byte(((f_PC - 1) & 8191))) & 4095);
        f_PC = ((f_PC & 4096) | v_a);
    }
    f_SF = 1;
    return 16;
    return 0;
}

int64_t EM73000::py_ldia_k(int64_t v_opcode) {
    int64_t v_k = 0;
    v_k = (v_opcode & 15);
    f_ACC = v_k;
    f_ZF = ((f_ACC == 0));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_scall_a(int64_t v_opcode) {
    int64_t v_n = 0, v_pc = 0, v_sp = 0;
    v_pc = (f_PC + 1);
    v_sp = (192 + (f_SP << 2));
    f_RAM[v_sp] = (v_pc >> 12);
    f_RAM[(v_sp + 1)] = ((v_pc >> 8) & 15);
    f_RAM[(v_sp + 2)] = ((v_pc >> 4) & 15);
    f_RAM[(v_sp + 3)] = (v_pc & 15);
    f_SP = ((f_SP - 1) & 15);
    v_n = (v_opcode & 15);
    f_PC = (((v_n * 8) + 6) + (128 * ((v_n == 0))));
    return 16;
    return 0;
}

int64_t EM73000::py_clm_b(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (v_opcode & 3);
    py_write_ram(f_HL, (py_read_ram(f_HL) & (~((1 << v_b)))));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_sem_b(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (v_opcode & 3);
    py_write_ram(f_HL, (py_read_ram(f_HL) | (1 << v_b)));
    f_SF = 1;
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_tfa_b(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (v_opcode & 3);
    f_SF = (((f_ACC & (1 << v_b)) == 0));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_tfm_b(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (v_opcode & 3);
    f_SF = (((py_read_ram(f_HL) & (1 << v_b)) == 0));
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

int64_t EM73000::py_dummy(int64_t v_opcode) {
    f_PC = ((f_PC + 1) & 8191);
    return 8;
    return 0;
}

const EM73000::IoEntry* EM73000::find_io_tbl(int64_t addr) {
    static const IoEntry e[] = {
        {&EM73000::py_get_io_P0, &EM73000::py_set_io_dummy},
        {&EM73000::py_get_io_P4, &EM73000::py_set_io_P4},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P5},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P6},
        {&EM73000::py_get_io_P7, &EM73000::py_set_io_dummy},
        {&EM73000::py_get_io_P8, &EM73000::py_set_io_P8},
        {&EM73000::py_get_io_P9, &EM73000::py_set_io_P9},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P16},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P23},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P24},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P25},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P27},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P28},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P29},
        {&EM73000::py_get_io_dummy, &EM73000::py_set_io_P30},
    };
    switch (addr) {
        case 0: return &e[0];
        case 4: return &e[1];
        case 5: return &e[2];
        case 6: return &e[3];
        case 7: return &e[4];
        case 8: return &e[5];
        case 9: return &e[6];
        case 16: return &e[7];
        case 23: return &e[8];
        case 24: return &e[9];
        case 25: return &e[10];
        case 27: return &e[11];
        case 28: return &e[12];
        case 29: return &e[13];
        case 30: return &e[14];
        default: return nullptr;
    }
}

const EM73000::OpEntry EM73000::T_execute[256] = {
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_sbr_a, 0},
    {&EM73000::py_lcall_a, 0},
    {&EM73000::py_lcall_a, 0},
    {&EM73000::py_lcall_a, 0},
    {&EM73000::py_lcall_a, 0},
    {&EM73000::py_lcall_a, 0},
    {&EM73000::py_lcall_a, 0},
    {&EM73000::py_lcall_a, 0},
    {&EM73000::py_lcall_a, 0},
    {&EM73000::py_std_k_y, 0},
    {&EM73000::py_add_k_y, 0},
    {&EM73000::py_out_k_p, 0},
    {&EM73000::py_cmp_k_y, 0},
    {&EM73000::py_exhl_x, 0},
    {&EM73000::py_rti, 0},
    {&EM73000::py_ldhl_x, 0},
    {&EM73000::py_ret, 0},
    {&EM73000::py_rlca, 0},
    {&EM73000::py_rrca, 0},
    {&EM73000::py_ttcfs, 0},
    {&EM73000::py_tfcfc, 0},
    {&EM73000::py_dummy, 0},
    {&EM73000::py_slbr1_a, 0},
    {&EM73000::py_nop, 0},
    {&EM73000::py_slbr0_a, 0},
    {&EM73000::py_exam, 0},
    {&EM73000::py_stam, 0},
    {&EM73000::py_ldam, 0},
    {&EM73000::py_tzs, 0},
    {&EM73000::py_deca, 0},
    {&EM73000::py_decm, 0},
    {&EM73000::py_inca, 0},
    {&EM73000::py_incm, 0},
    {&EM73000::py_clpl, 0},
    {&EM73000::py_tfpl, 0},
    {&EM73000::py_sepl, 0},
    {&EM73000::py_cil, 0},
    {&EM73000::py_exal, 0},
    {&EM73000::py_ldax, 0},
    {&EM73000::py_exah, 0},
    {&EM73000::py_ldaxi, 0},
    {&EM73000::py_exa_x, 0},
    {&EM73000::py_sta_x, 0},
    {&EM73000::py_lda_x, 0},
    {&EM73000::py_cmpa_x, 0},
    {&EM73000::py_bit_y_b, 0},
    {&EM73000::py_bit_p_b, 0},
    {&EM73000::py_math_k, 0},
    {&EM73000::py_io_p, 0},
    {&EM73000::py_adcam, 0},
    {&EM73000::py_addam, 0},
    {&EM73000::py_sbcam, 0},
    {&EM73000::py_cmpam, 0},
    {&EM73000::py_tla, 0},
    {&EM73000::py_exae, 0},
    {&EM73000::py_tha, 0},
    {&EM73000::py_dummy, 0},
    {&EM73000::py_oram, 0},
    {&EM73000::py_xoram, 0},
    {&EM73000::py_dummy, 0},
    {&EM73000::py_andam, 0},
    {&EM73000::py_decl, 0},
    {&EM73000::py_stamd, 0},
    {&EM73000::py_incl, 0},
    {&EM73000::py_stami, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldl_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_ldh_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_stdmi_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_cmpia_k, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_lbr_a, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_ldia_k, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_scall_a, 0},
    {&EM73000::py_clm_b, 0},
    {&EM73000::py_clm_b, 0},
    {&EM73000::py_clm_b, 0},
    {&EM73000::py_clm_b, 0},
    {&EM73000::py_sem_b, 0},
    {&EM73000::py_sem_b, 0},
    {&EM73000::py_sem_b, 0},
    {&EM73000::py_sem_b, 0},
    {&EM73000::py_tfa_b, 0},
    {&EM73000::py_tfa_b, 0},
    {&EM73000::py_tfa_b, 0},
    {&EM73000::py_tfa_b, 0},
    {&EM73000::py_tfm_b, 0},
    {&EM73000::py_tfm_b, 0},
    {&EM73000::py_tfm_b, 0},
    {&EM73000::py_tfm_b, 0},
};

const EM73000::OpEntry EM73000::T_execute_cil[4] = {
    {&EM73000::py_dummy, 0},
    {&EM73000::py_eicil_r, 0},
    {&EM73000::py_dicil_r, 0},
    {&EM73000::py_cil_r, 0},
};

const EM73000::OpEntry EM73000::T_execute_bit_y_b[4] = {
    {&EM73000::py_tf_y_b, 0},
    {&EM73000::py_set_y_b, 0},
    {&EM73000::py_tt_y_b, 0},
    {&EM73000::py_clr_y_b, 0},
};

const EM73000::OpEntry EM73000::T_execute_bit_p_b[4] = {
    {&EM73000::py_tfp_p_b, 0},
    {&EM73000::py_sep_p_b, 0},
    {&EM73000::py_ttp_p_b, 0},
    {&EM73000::py_clp_p_b, 0},
};

const EM73000::OpEntry EM73000::T_execute_math_k[16] = {
    {&EM73000::py_dummy, 0},
    {&EM73000::py_addl_k, 0},
    {&EM73000::py_dummy, 0},
    {&EM73000::py_cmpl_k, 0},
    {&EM73000::py_ora_k, 0},
    {&EM73000::py_adda_k, 0},
    {&EM73000::py_anda_k, 0},
    {&EM73000::py_suba_k, 0},
    {&EM73000::py_dummy, 0},
    {&EM73000::py_addh_k, 0},
    {&EM73000::py_dummy, 0},
    {&EM73000::py_cmph_k, 0},
    {&EM73000::py_orm_k, 0},
    {&EM73000::py_addm_k, 0},
    {&EM73000::py_andm_k, 0},
    {&EM73000::py_subm_k, 0},
};

const EM73000::OpEntry EM73000::T_execute_io_p[4] = {
    {&EM73000::py_outa_p, 0},
    {&EM73000::py_ina_p, 0},
    {&EM73000::py_outm_p, 0},
    {&EM73000::py_inm_p, 0},
};
