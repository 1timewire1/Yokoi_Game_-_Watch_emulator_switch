#include "STK55C324.h"

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



}  // namespace

int64_t STK55C324::rom_get_byte(int64_t a) const {
    const int64_t n = rom_size();
    return rom()[(size_t)(((a % n) + n) % n)];
}

void STK55C324::reset() {
    if (!inited_) {
        inited_ = true;
        init_();
    }
    pyreset();
}

void STK55C324::init_() {
    f_rom_offset = (65536 - rom_size());
    f_pullup[kPort_P1] = options().num("port_pullup.P1", 0);
    f_port_input[kPort_P1] = {0, 0};
    f_instr_counter = 0;
    f_64Hz_counter = 0;
    f_timer1_counter = 0;
    f_fixtime_counter = 0;
    f_sub_clock_div = ((double)((double)clock_hz()) / (32768));
}

int64_t STK55C324::execute_scaled() {
    const int64_t scaled = std::llround((double)pyclock() * kTickHz);
    return scaled > 0 ? scaled : 1;
}

uint8_t STK55C324::display_byte(int i) const {
    if (i < 0 || i >= 64) return 0;
    if ((f_CONTROL & 16) && (f_SLEEP_MODE & 1) == 0) return (uint8_t)f_LCDRAM[i];
    return 0;
}

int STK55C324::port_id(const std::string& name) const {
    if (name == "A") return kPort_A;
    if (name == "BF") return kPort_BF;
    if (name == "CF") return kPort_CF;
    if (name == "DF") return kPort_DF;
    if (name == "IF") return kPort_IF;
    if (name == "LCDRAM") return kPort_LCDRAM;
    if (name == "NF") return kPort_NF;
    if (name == "P1") return kPort_P1;
    if (name == "PC") return kPort_PC;
    if (name == "RAM") return kPort_RAM;
    if (name == "SFR") return kPort_SFR;
    if (name == "SP") return kPort_SP;
    if (name == "TF") return kPort_TF;
    if (name == "VF") return kPort_VF;
    if (name == "X") return kPort_X;
    if (name == "Y") return kPort_Y;
    if (name == "ZF") return kPort_ZF;
    return -1;
}

void STK55C324::port_input(int port, uint8_t mask, int level) {
    pypin_handler(port, mask, level);
}

void STK55C324::serialize(BrickStateIO& io) {
    io.pod(f_rom_offset);
    for (auto& kv : f_pullup) io.pod(kv.second);
    for (auto& kv : f_port_input) ser_vec(io, kv.second);
    io.pod(f_instr_counter);
    io.pod(f_64Hz_counter);
    io.pod(f_timer1_counter);
    io.pod(f_fixtime_counter);
    io.pod(f_sub_clock_div);
    io.pod(f_PC);
    io.pod(f_A);
    io.pod(f_X);
    io.pod(f_Y);
    io.pod(f_NF);
    io.pod(f_VF);
    io.pod(f_TF);
    io.pod(f_BF);
    io.pod(f_DF);
    io.pod(f_IF);
    io.pod(f_ZF);
    io.pod(f_CF);
    io.pod(f_SP);
    io.pod(f_CONTROL);
    io.pod(f_nSTANDBY);
    io.pod(f_SLEEP_MODE);
    ser_vec(io, f_RAM);
    ser_vec(io, f_LCDRAM);
    for (auto& kv : f_PDIR) io.pod(kv.second);
    for (auto& kv : f_PLATCH) io.pod(kv.second);
    io.pod(f_P1INT);
    io.pod(f_TIMER1);
    io.pod(f_TIMER1_LATCH);
    io.pod(f_TIMER_CLK_LCD_CONTRAST);
    io.pod(f_TIMER1_DIV);
    io.pod(f_WATCH_TIMER_CTRL);
    io.pod(f_IREQ);
    io.pod(f_SOUND_CH1_VOL);
    io.pod(f_SOUND_CH2_VOL);
    io.pod(f_SOUND_CLK);
    io.pod(f_SOUND_CH1_DATA);
    io.pod(f_SOUND_CH2_DATA);
    io.pod(f_SOUND_CTRL);
    io.pod(f_addr_reset);
    io.pod(f_addr_irq);
    io.pod(f_addr_nmi);
    io.bytes(snd_clock_, sizeof(snd_clock_));
    io.bytes(snd_data_, sizeof(snd_data_));
    io.bytes(snd_volume_, sizeof(snd_volume_));
    io.bytes(snd_enable_, sizeof(snd_enable_));
}


void STK55C324::pyreset() {
    f_PC = 0;
    f_A = 0;
    f_X = 0;
    f_Y = 0;
    py_set_ps(4);
    f_SP = 0;
    f_CONTROL = 0;
    f_nSTANDBY = 1;
    f_SLEEP_MODE = 0;
    f_RAM.assign((size_t)(160), 0);
    f_LCDRAM.assign((size_t)(64), 0);
    f_PDIR[kPort_P1] = 0;
    f_PLATCH[kPort_P1] = 0;
    f_P1INT = 0;
    {
        const auto a_ = 255;
        f_TIMER1 = a_;
        f_TIMER1_LATCH = a_;
    }
    f_TIMER_CLK_LCD_CONTRAST = 240;
    f_TIMER1_DIV = 2;
    f_WATCH_TIMER_CTRL = 0;
    f_IREQ = 0;
    f_SOUND_CH1_VOL = 0;
    f_SOUND_CH2_VOL = 0;
    f_SOUND_CLK = 0;
    f_SOUND_CH1_DATA = 0;
    f_SOUND_CH2_DATA = 0;
    f_SOUND_CTRL = 0;
    f_addr_reset = rom_get_word_LSB(65532);
    f_addr_irq = rom_get_word_LSB(65534);
    f_addr_nmi = rom_get_word_LSB(65530);
    f_PC = f_addr_reset;
}

int64_t STK55C324::py_port_read(int64_t v_port) {
    return ((f_PDIR[v_port] & f_PLATCH[v_port]) | ((~(f_PDIR[v_port])) & ((~(f_port_input[v_port][0])) & (f_port_input[v_port][1] | f_pullup[v_port]))));
    return 0;
}

void STK55C324::pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level) {
    int64_t v_prev_port = 0;
    if (((v_port == kPort_RES))) {
        if (((v_level == 0))) {
            pyreset();
        }
    } else {
        v_prev_port = py_port_read(v_port);
        f_port_input[v_port][0] &= (~(v_mask));
        f_port_input[v_port][1] &= (~(v_mask));
        if (((v_level >= 0))) {
            f_port_input[v_port][v_level] |= v_mask;
        }
        if (((v_port == kPort_P1))) {
            if ((((v_prev_port & f_P1INT) > (py_port_read(v_port) & f_P1INT)))) {
                f_IREQ |= 4;
                f_nSTANDBY = 1;
                f_SLEEP_MODE = 0;
            }
        }
    }
}

void STK55C324::py_irq() {
    py_write_mem(f_SP, (f_PC >> 8));
    f_SP = ((f_SP - 1) & 255);
    py_write_mem(f_SP, (f_PC & 255));
    f_SP = ((f_SP - 1) & 255);
    py_write_mem(f_SP, py_get_ps());
    f_SP = ((f_SP - 1) & 255);
    f_IF = 1;
    f_PC = f_addr_irq;
}

void STK55C324::py_nmi() {
    py_write_mem(f_SP, (f_PC >> 8));
    f_SP = ((f_SP - 1) & 255);
    py_write_mem(f_SP, (f_PC & 255));
    f_SP = ((f_SP - 1) & 255);
    py_write_mem(f_SP, py_get_ps());
    f_SP = ((f_SP - 1) & 255);
    f_PC = f_addr_nmi;
}

void STK55C324::py_timers_clock(double v_exec_cycles) {
    f_64Hz_counter -= v_exec_cycles;
    while (((f_64Hz_counter <= 0))) {
        f_64Hz_counter += (f_sub_clock_div * py_div(32768, 64));
        if ((f_CONTROL & 4)) {
            f_nSTANDBY = 1;
            f_SLEEP_MODE &= 1;
            py_nmi();
        }
    }
    if ((f_CONTROL & 64)) {
        f_fixtime_counter -= v_exec_cycles;
        while (((f_fixtime_counter <= 0))) {
            if ((f_WATCH_TIMER_CTRL & 2)) {
                f_fixtime_counter += (f_sub_clock_div * 32768);
            } else {
                f_fixtime_counter += (f_sub_clock_div * py_div(32768, 2));
            }
            f_IREQ |= 1;
            f_nSTANDBY = 1;
            f_SLEEP_MODE &= 1;
        }
    }
    if ((f_CONTROL & 8)) {
        f_timer1_counter -= v_exec_cycles;
        while (((f_timer1_counter <= 0))) {
            f_timer1_counter += f_TIMER1_DIV;
            f_TIMER1 -= 1;
            if (((f_TIMER1 == 0))) {
                f_TIMER1 = f_TIMER1_LATCH;
                if ((f_CONTROL & 2)) {
                    f_nSTANDBY = 1;
                    f_IREQ |= 2;
                }
            }
        }
    }
}

double STK55C324::pyclock() {
    int64_t v_i = 0, v_opcode = 0;
    double v_exec_cycles = 0;
    v_exec_cycles = 64;
    if ((((f_SLEEP_MODE & 1) == 0))) {
        if (((f_nSTANDBY) && ((((((f_SLEEP_MODE & 2) == 0))) || ((f_WATCH_TIMER_CTRL & 16)))))) {
            if ((((!(f_IF))) && (f_IREQ))) {
                for (int64_t v_i : {1, 2, 4}) {
                    if ((f_IREQ & v_i)) {
                        py_irq();
                        break;
                    }
                }
            }
            v_opcode = rom_get_byte(f_PC);
            f_PC = ((f_PC + 1) & 65535);
            v_exec_cycles = (this->*T_execute[v_opcode].fn)(0);
            f_instr_counter += 1;
            if ((f_WATCH_TIMER_CTRL & 16)) {
                v_exec_cycles *= f_sub_clock_div;
            }
        }
        py_timers_clock(v_exec_cycles);
    }
    return v_exec_cycles;
    return 0;
}

int64_t STK55C324::py_get_sfr_dummy() {
    return 0;
    return 0;
}

void STK55C324::py_set_sfr_dummy(int64_t v_value) {
}

int64_t STK55C324::py_set_sfr_standby_mode(int64_t v_value) {
    f_nSTANDBY = 0;
    return 0;
}

int64_t STK55C324::py_set_sfr_sleep_mode(int64_t v_value) {
    f_SLEEP_MODE = v_value;
    return 0;
}

int64_t STK55C324::py_set_sfr_watch_timer_ctrl(int64_t v_value) {
    f_WATCH_TIMER_CTRL = v_value;
    return 0;
}

int64_t STK55C324::py_get_sfr_irq_flag() {
    return f_IREQ;
    return 0;
}

int64_t STK55C324::py_set_sfr_irq_flag(int64_t v_value) {
    f_IREQ &= v_value;
    return 0;
}

int64_t STK55C324::py_get_sfr_port1_data() {
    return py_port_read(kPort_P1);
    return 0;
}

int64_t STK55C324::py_set_sfr_port1_data(int64_t v_value) {
    f_PLATCH[kPort_P1] = v_value;
    return 0;
}

int64_t STK55C324::py_set_sfr_port1_dir(int64_t v_value) {
    f_PDIR[kPort_P1] = v_value;
    return 0;
}

int64_t STK55C324::py_set_sfr_port1_int(int64_t v_value) {
    f_P1INT = v_value;
    return 0;
}

int64_t STK55C324::py_get_sfr_timer1_data() {
    return f_TIMER1;
    return 0;
}

int64_t STK55C324::py_set_sfr_timer1_data(int64_t v_value) {
    if (((v_value == 0))) {
        v_value = 1;
    }
    {
        const auto a_ = v_value;
        f_TIMER1 = a_;
        f_TIMER1_LATCH = a_;
    }
    return 0;
}

int64_t STK55C324::py_set_sfr_timer1_div_contrast(int64_t v_value) {
    f_TIMER1_DIV = (2 << (v_value & 3));
    f_TIMER_CLK_LCD_CONTRAST = v_value;
    return 0;
}

int64_t STK55C324::py_set_sfr_ctrl(int64_t v_value) {
    f_CONTROL = v_value;
    if ((v_value & 4)) {
        f_64Hz_counter = 0;
    }
    return 0;
}

int64_t STK55C324::py_set_sfr_volume_ch1(int64_t v_value) {
    f_SOUND_CH1_VOL = v_value;
    snd_set_volume(0, v_value);
    return 0;
}

int64_t STK55C324::py_set_sfr_volume_ch2(int64_t v_value) {
    f_SOUND_CH2_VOL = v_value;
    snd_set_volume(1, v_value);
    return 0;
}

int64_t STK55C324::py_set_sfr_sound_clock(int64_t v_value) {
    f_SOUND_CLK = v_value;
    snd_set_clock(v_value);
    return 0;
}

int64_t STK55C324::py_set_sfr_data_ch1(int64_t v_value) {
    f_SOUND_CH1_DATA = v_value;
    snd_set_data(0, v_value);
    return 0;
}

int64_t STK55C324::py_set_sfr_data_ch2(int64_t v_value) {
    f_SOUND_CH2_DATA = v_value;
    snd_set_data(1, v_value);
    return 0;
}

int64_t STK55C324::py_set_sfr_sound_ctrl(int64_t v_value) {
    f_SOUND_CTRL = v_value;
    snd_set_control(v_value);
    return 0;
}

void STK55C324::py_write_mem(int64_t v_addr, int64_t v_value) {
    int64_t v_addr_ovrlp = 0;
    v_addr_ovrlp = (v_addr & 65279);
    if (((v_addr_ovrlp < 64))) {
        f_LCDRAM[v_addr_ovrlp] = v_value;
    } else {
        if (((((v_addr_ovrlp >= 64))) && (((v_addr_ovrlp < (160 + 64)))))) {
            f_RAM[(v_addr_ovrlp - 64)] = v_value;
        } else {
            const IoEntry* v_io = find_sfr_tbl(v_addr);
            if (((v_io != nullptr))) {
                (this->*v_io->set)(v_value);
            }
        }
    }
}

int64_t STK55C324::py_read_mem(int64_t v_addr) {
    int64_t v_addr_ovrlp = 0;
    v_addr_ovrlp = (v_addr & 65279);
    if (((v_addr_ovrlp < 64))) {
        return f_LCDRAM[v_addr_ovrlp];
    } else {
        if (((((v_addr_ovrlp >= 64))) && (((v_addr_ovrlp < (160 + 64)))))) {
            return f_RAM[(v_addr_ovrlp - 64)];
        } else {
            if (((v_addr >= f_rom_offset))) {
                return rom_get_byte((v_addr - f_rom_offset));
            } else {
                const IoEntry* v_io = find_sfr_tbl(v_addr);
                if (((v_io != nullptr))) {
                    return (this->*v_io->get)();
                }
            }
        }
    }
    return 0;
    return 0;
}

int64_t STK55C324::py_get_ps() {
    return ((((((((f_NF << 7) | (f_VF << 6)) | (f_TF << 5)) | (f_BF << 4)) | (f_DF << 3)) | (f_IF << 2)) | (f_ZF << 1)) | f_CF);
    return 0;
}

void STK55C324::py_set_ps(int64_t v_ps) {
    f_NF = (v_ps >> 7);
    f_VF = (((v_ps & 64) > 0));
    f_TF = (((v_ps & 32) > 0));
    f_BF = (((v_ps & 16) > 0));
    f_DF = (((v_ps & 8) > 0));
    f_IF = (((v_ps & 4) > 0));
    f_ZF = (((v_ps & 2) > 0));
    f_CF = (v_ps & 1);
}

void STK55C324::py_adc(int64_t v_operand) {
    int64_t v_A = 0, v_new_value = 0;
    v_A = f_A;
    v_new_value = ((v_A + v_operand) + f_CF);
    if (((f_DF) && ((((((v_A & 15) + (v_operand & 15)) + f_CF) > 9))))) {
        v_new_value += 6;
    }
    f_VF = ((((~((v_A ^ v_operand))) & (v_A ^ v_new_value)) >> 7) & 1);
    f_NF = ((v_new_value >> 7) & 1);
    if (((f_DF) && (((v_new_value > 153))))) {
        v_new_value += 96;
    }
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    f_A = (v_new_value & 255);
}

void STK55C324::py_sbc(int64_t v_operand) {
    int64_t v_A = 0, v_new_value = 0;
    v_A = f_A;
    v_new_value = ((v_A - v_operand) - (!(f_CF)));
    if (f_DF) {
        if ((((((v_A & 15) - (v_operand & 15)) - (!(f_CF))) < 0))) {
            v_new_value -= 6;
        }
        if (((v_new_value < 0))) {
            v_new_value -= 96;
        }
    }
    f_VF = ((((v_A ^ v_operand) & (v_A ^ v_new_value)) >> 7) & 1);
    f_NF = ((v_new_value >> 7) & 1);
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value >= 0));
    f_A = (v_new_value & 255);
}

int64_t STK55C324::py_brk(int64_t) {
    f_BF = 1;
    py_irq();
    return 7;
    return 0;
}

int64_t STK55C324::py_ora_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 6;
    return 0;
}

int64_t STK55C324::py_tsb_zp(int64_t) {
    int64_t v_m = 0, v_res = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_m = py_read_mem(v_zp);
    v_res = (v_m | f_A);
    f_PC = ((f_PC + 1) & 65535);
    f_ZF = (!(v_res));
    py_write_mem(v_zp, (v_res & 255));
    return 5;
    return 0;
}

int64_t STK55C324::py_ora_zp(int64_t) {
    f_A |= py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t STK55C324::py_asl_zp(int64_t) {
    int64_t v_new_value = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_new_value = (py_read_mem(v_zp) << 1);
    py_write_mem(v_zp, (v_new_value & 255));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return 5;
    return 0;
}

int64_t STK55C324::py_php(int64_t) {
    py_write_mem(f_SP, py_get_ps());
    f_SP = ((f_SP - 1) & 255);
    return 3;
    return 0;
}

int64_t STK55C324::py_ora_imm(int64_t) {
    f_A |= py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t STK55C324::py_asl_a(int64_t) {
    int64_t v_new_value = 0;
    v_new_value = (f_A << 1);
    f_A = (v_new_value & 255);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = (v_new_value >> 8);
    return 2;
    return 0;
}

int64_t STK55C324::py_tsb_abs(int64_t) {
    int64_t v_addr = 0, v_m = 0, v_res = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_m = py_read_mem(v_addr);
    v_res = (v_m | f_A);
    f_PC = ((f_PC + 2) & 65535);
    f_ZF = (!(v_res));
    py_write_mem(v_addr, (v_res & 255));
    return 6;
    return 0;
}

int64_t STK55C324::py_ora_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_asl_abs(int64_t) {
    int64_t v_addr = 0, v_new_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_new_value = (py_read_mem(v_addr) << 1);
    py_write_mem(v_addr, (v_new_value & 255));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return 6;
    return 0;
}

int64_t STK55C324::py_bpl(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    if ((!(f_NF))) {
        v_rel = py_read_mem(f_PC);
        v_prev_PC = ((f_PC + 1) & 65535);
        f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
        return (3 + (((f_PC ^ v_prev_PC) > 255)));
    }
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_ora_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (5 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_ora_zp_ind(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_addr = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 5;
    return 0;
}

int64_t STK55C324::py_trb_zp(int64_t) {
    int64_t v_m = 0, v_res = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_m = py_read_mem(v_zp);
    v_res = (v_m & (f_A ^ 255));
    f_PC = ((f_PC + 1) & 65535);
    f_ZF = (!(v_res));
    py_write_mem(v_zp, v_res);
    return 5;
    return 0;
}

int64_t STK55C324::py_ora_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t STK55C324::py_asl_zp_x(int64_t) {
    int64_t v_addr = 0, v_new_value = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    v_new_value = (py_read_mem(v_addr) << 1);
    py_write_mem(v_addr, (v_new_value & 255));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return 6;
    return 0;
}

int64_t STK55C324::py_ora_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_inc_a(int64_t) {
    f_A = ((f_A + 1) & 255);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t STK55C324::py_and_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 6;
    return 0;
}

int64_t STK55C324::py_trb_abs(int64_t) {
    int64_t v_addr = 0, v_m = 0, v_res = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_m = py_read_mem(v_addr);
    v_res = (v_m & (f_A ^ 255));
    f_PC = ((f_PC + 2) & 65535);
    f_ZF = (!(v_res));
    py_write_mem(v_addr, v_res);
    return 6;
    return 0;
}

int64_t STK55C324::py_ora_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_asl_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_new_value = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    v_new_value = (py_read_mem(v_addr) << 1);
    py_write_mem(v_addr, (v_new_value & 255));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return (6 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_clc(int64_t) {
    f_CF = 0;
    return 2;
    return 0;
}

int64_t STK55C324::py_jsr_abs(int64_t) {
    int64_t v_pc = 0;
    v_pc = ((f_PC + 1) & 65535);
    py_write_mem(f_SP, (v_pc >> 8));
    f_SP = ((f_SP - 1) & 255);
    py_write_mem(f_SP, (v_pc & 255));
    f_SP = ((f_SP - 1) & 255);
    f_PC = (py_read_mem(f_PC) | (py_read_mem(v_pc) << 8));
    return 6;
    return 0;
}

int64_t STK55C324::py_and_abs_y(int64_t) {
    int64_t v_addr = 0;
    v_addr = (((py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8)) + f_Y) & 65535);
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_dec_a(int64_t) {
    f_A = ((f_A - 1) & 255);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t STK55C324::py_bit_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_m = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    v_m = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (v_m >> 7);
    f_VF = (((v_m & 64) > 0));
    f_ZF = (!((f_A & v_m)));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_and_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_rol_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_new_value = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    v_new_value = ((py_read_mem(v_addr) << 1) | f_CF);
    py_write_mem(v_addr, (v_new_value & 255));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return (6 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_bit_zp(int64_t) {
    int64_t v_m = 0;
    v_m = py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (v_m >> 7);
    f_VF = (((v_m & 64) > 0));
    f_ZF = (!((f_A & v_m)));
    return 3;
    return 0;
}

int64_t STK55C324::py_and_zp(int64_t) {
    f_A &= py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t STK55C324::py_rol_zp(int64_t) {
    int64_t v_new_value = 0, v_opcode = 0;
    v_opcode = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    v_new_value = ((py_read_mem(v_opcode) << 1) | f_CF);
    py_write_mem(v_opcode, (v_new_value & 255));
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return 5;
    return 0;
}

int64_t STK55C324::py_plp(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    py_set_ps(py_read_mem(f_SP));
    return 4;
    return 0;
}

int64_t STK55C324::py_and_imm(int64_t) {
    f_A &= py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t STK55C324::py_rol_a(int64_t) {
    int64_t v_new_value = 0;
    v_new_value = ((f_A << 1) | f_CF);
    f_A = (v_new_value & 255);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return 2;
    return 0;
}

int64_t STK55C324::py_bit_abs(int64_t) {
    int64_t v_addr = 0, v_m = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_m = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (v_m >> 7);
    f_VF = (((v_m & 64) > 0));
    f_ZF = (!((f_A & v_m)));
    return 4;
    return 0;
}

int64_t STK55C324::py_and_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_rol_abs(int64_t) {
    int64_t v_addr = 0, v_new_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_new_value = ((py_read_mem(v_addr) << 1) | f_CF);
    py_write_mem(v_addr, (v_new_value & 255));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return 5;
    return 0;
}

int64_t STK55C324::py_bmi(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    if (f_NF) {
        v_rel = py_read_mem(f_PC);
        v_prev_PC = ((f_PC + 1) & 65535);
        f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
        return (3 + (((f_PC ^ v_prev_PC) > 255)));
    }
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_and_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (5 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_and_zp_ind(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_addr = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 5;
    return 0;
}

int64_t STK55C324::py_bit_zp_x(int64_t) {
    int64_t v_addr = 0, v_m = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    v_m = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (v_m >> 7);
    f_VF = (((v_m & 64) > 0));
    f_ZF = (!((f_A & v_m)));
    return 4;
    return 0;
}

int64_t STK55C324::py_and_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 6;
    return 0;
}

int64_t STK55C324::py_rol_zp_x(int64_t) {
    int64_t v_addr = 0, v_new_value = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    v_new_value = ((py_read_mem(v_addr) << 1) | f_CF);
    py_write_mem(v_addr, (v_new_value & 255));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return 6;
    return 0;
}

int64_t STK55C324::py_sec(int64_t) {
    f_CF = 1;
    return 2;
    return 0;
}

int64_t STK55C324::py_rti(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    py_set_ps(py_read_mem(f_SP));
    f_SP = ((f_SP + 1) & 255);
    f_PC = py_read_mem(f_SP);
    f_SP = ((f_SP + 1) & 255);
    f_PC |= (py_read_mem(f_SP) << 8);
    return 6;
    return 0;
}

int64_t STK55C324::py_eor_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_eor_zp(int64_t) {
    f_A ^= py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t STK55C324::py_lsr_zp(int64_t) {
    int64_t v_addr = 0, v_prev_value = 0;
    v_addr = py_read_mem(f_PC);
    v_prev_value = py_read_mem(v_addr);
    py_write_mem(v_addr, (v_prev_value >> 1));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = 0;
    f_ZF = (!((v_prev_value & 254)));
    f_CF = (v_prev_value & 1);
    return 5;
    return 0;
}

int64_t STK55C324::py_pha(int64_t) {
    py_write_mem(f_SP, f_A);
    f_SP = ((f_SP - 1) & 255);
    return 3;
    return 0;
}

int64_t STK55C324::py_eor_imm(int64_t) {
    f_A ^= (py_read_mem(f_PC) & 255);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t STK55C324::py_lsr_a(int64_t) {
    int64_t v_prev_value = 0;
    v_prev_value = f_A;
    f_A = (v_prev_value >> 1);
    f_NF = 0;
    f_ZF = (!((v_prev_value & 254)));
    f_CF = (v_prev_value & 1);
    return 2;
    return 0;
}

int64_t STK55C324::py_jmp_abs(int64_t) {
    f_PC = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    return 3;
    return 0;
}

int64_t STK55C324::py_eor_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_lsr_abs(int64_t) {
    int64_t v_addr = 0, v_prev_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_prev_value = py_read_mem(v_addr);
    py_write_mem(v_addr, (v_prev_value >> 1));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = 0;
    f_ZF = (!((v_prev_value & 254)));
    f_CF = (v_prev_value & 1);
    return 6;
    return 0;
}

int64_t STK55C324::py_bvc(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    if ((!(f_VF))) {
        v_rel = py_read_mem(f_PC);
        v_prev_PC = ((f_PC + 1) & 65535);
        f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
        return (3 + (((f_PC ^ v_prev_PC) > 255)));
    }
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_eor_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (5 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_eor_zp_ind(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_addr = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 5;
    return 0;
}

int64_t STK55C324::py_eor_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_lsr_zp_x(int64_t) {
    int64_t v_addr = 0, v_prev_value = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    v_prev_value = py_read_mem(v_addr);
    py_write_mem(v_addr, (v_prev_value >> 1));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = 0;
    f_ZF = (!((v_prev_value & 254)));
    f_CF = (v_prev_value & 1);
    return 6;
    return 0;
}

int64_t STK55C324::py_cli(int64_t) {
    f_IF = 0;
    return 2;
    return 0;
}

int64_t STK55C324::py_eor_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_phy(int64_t) {
    py_write_mem(f_SP, f_Y);
    f_SP = ((f_SP - 1) & 255);
    return 3;
    return 0;
}

int64_t STK55C324::py_eor_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_lsr_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_prev_value = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    v_prev_value = py_read_mem(v_addr);
    py_write_mem(v_addr, (v_prev_value >> 1));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = 0;
    f_ZF = (!((v_prev_value & 254)));
    f_CF = (v_prev_value & 1);
    return (6 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_rts(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    f_PC = py_read_mem(f_SP);
    f_SP = ((f_SP + 1) & 255);
    f_PC |= (py_read_mem(f_SP) << 8);
    f_PC += 1;
    return 6;
    return 0;
}

int64_t STK55C324::py_adc_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t STK55C324::py_stz_zp(int64_t) {
    py_write_mem(py_read_mem(f_PC), 0);
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t STK55C324::py_adc_zp(int64_t) {
    py_adc(py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t STK55C324::py_ror_zp(int64_t) {
    int64_t v_opcode = 0, v_prev_value = 0;
    v_opcode = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    v_prev_value = py_read_mem(v_opcode);
    py_write_mem(v_opcode, ((v_prev_value >> 1) | (f_CF << 7)));
    f_NF = f_CF;
    f_ZF = (!(((v_prev_value & 254) | f_CF)));
    f_CF = (v_prev_value & 1);
    return 5;
    return 0;
}

int64_t STK55C324::py_pla(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    f_A = py_read_mem(f_SP);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_adc_imm(int64_t) {
    py_adc(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_ror_a(int64_t) {
    int64_t v_prev_value = 0;
    v_prev_value = f_A;
    f_A = ((v_prev_value >> 1) | (f_CF << 7));
    f_NF = f_CF;
    f_ZF = (!(((v_prev_value & 254) | f_CF)));
    f_CF = (v_prev_value & 1);
    return 2;
    return 0;
}

int64_t STK55C324::py_jmp_ind(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_PC = py_read_mem(v_addr);
    f_PC |= (py_read_mem(((v_addr + 1) & 65535)) << 8);
    return 6;
    return 0;
}

int64_t STK55C324::py_adc_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_ror_abs(int64_t) {
    int64_t v_addr = 0, v_prev_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_prev_value = py_read_mem(v_addr);
    py_write_mem(v_addr, ((v_prev_value >> 1) | (f_CF << 7)));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = f_CF;
    f_ZF = (!(((v_prev_value & 254) | f_CF)));
    f_CF = (v_prev_value & 1);
    return 6;
    return 0;
}

int64_t STK55C324::py_bvs(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    if (f_VF) {
        v_rel = py_read_mem(f_PC);
        v_prev_PC = ((f_PC + 1) & 65535);
        f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
        return (3 + (((f_PC ^ v_prev_PC) > 255)));
    }
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_adc_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return (5 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_adc_zp_ind(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_addr = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t STK55C324::py_stz_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_write_mem(v_addr, 0);
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_adc_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 5;
    return 0;
}

int64_t STK55C324::py_ror_zp_x(int64_t) {
    int64_t v_addr = 0, v_prev_value = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    v_prev_value = py_read_mem(v_addr);
    py_write_mem(v_addr, ((v_prev_value >> 1) | (f_CF << 7)));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = f_CF;
    f_ZF = (!(((v_prev_value & 254) | f_CF)));
    f_CF = (v_prev_value & 1);
    return 6;
    return 0;
}

int64_t STK55C324::py_sei(int64_t) {
    f_IF = 1;
    return 2;
    return 0;
}

int64_t STK55C324::py_adc_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_ply(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    f_Y = py_read_mem(f_SP);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 4;
    return 0;
}

int64_t STK55C324::py_jmp_abs_ind_x(int64_t) {
    int64_t v_addr = 0, v_ptr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_ptr = ((v_addr + f_X) & 65535);
    f_PC = py_read_mem(v_ptr);
    f_PC |= (py_read_mem(((v_ptr + 1) & 65535)) << 8);
    return 6;
    return 0;
}

int64_t STK55C324::py_adc_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_ror_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_prev_value = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    v_prev_value = py_read_mem(v_addr);
    py_write_mem(v_addr, ((v_prev_value >> 1) | (f_CF << 7)));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = f_CF;
    f_ZF = (!(((v_prev_value & 254) | f_CF)));
    f_CF = (v_prev_value & 1);
    return (6 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_bra(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    v_rel = py_read_mem(f_PC);
    v_prev_PC = ((f_PC + 1) & 65535);
    f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
    return (3 + (((f_PC ^ v_prev_PC) > 255)));
    return 0;
}

int64_t STK55C324::py_sta_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t STK55C324::py_sty_zp(int64_t) {
    py_write_mem(py_read_mem(f_PC), f_Y);
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t STK55C324::py_sta_zp(int64_t) {
    py_write_mem(py_read_mem(f_PC), f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t STK55C324::py_stx_zp(int64_t) {
    py_write_mem(py_read_mem(f_PC), f_X);
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t STK55C324::py_dey(int64_t) {
    f_Y = ((f_Y - 1) & 255);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 2;
    return 0;
}

int64_t STK55C324::py_bit_imm(int64_t) {
    int64_t v_m = 0;
    v_m = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (v_m >> 7);
    f_VF = (((v_m & 64) > 0));
    f_ZF = (!((f_A & v_m)));
    return 2;
    return 0;
}

int64_t STK55C324::py_txa(int64_t) {
    f_A = f_X;
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t STK55C324::py_sty_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_write_mem(v_addr, f_Y);
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_sta_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_stx_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_write_mem(v_addr, f_X);
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_bcc(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    if ((!(f_CF))) {
        v_rel = py_read_mem(f_PC);
        v_prev_PC = ((f_PC + 1) & 65535);
        f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
        return (3 + (((f_PC ^ v_prev_PC) > 255)));
    }
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_sta_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t STK55C324::py_sta_zp_ind(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_addr = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t STK55C324::py_sty_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_write_mem(v_addr, f_Y);
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_sta_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_stx_zp_y(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_Y) & 255);
    py_write_mem(v_addr, f_X);
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_tya(int64_t) {
    f_A = f_Y;
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t STK55C324::py_sta_abs_y(int64_t) {
    int64_t v_addr = 0;
    v_addr = (((py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8)) + f_Y) & 65535);
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 2) & 65535);
    return 5;
    return 0;
}

int64_t STK55C324::py_txs(int64_t) {
    f_SP = f_X;
    return 2;
    return 0;
}

int64_t STK55C324::py_stz_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_write_mem(v_addr, 0);
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_sta_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 2) & 65535);
    return 5;
    return 0;
}

int64_t STK55C324::py_stz_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    py_write_mem(v_addr, 0);
    f_PC = ((f_PC + 2) & 65535);
    return 5;
    return 0;
}

int64_t STK55C324::py_ldy_imm(int64_t) {
    f_Y = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 2;
    return 0;
}

int64_t STK55C324::py_lda_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 6;
    return 0;
}

int64_t STK55C324::py_ldx_imm(int64_t) {
    f_X = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t STK55C324::py_ldy_zp(int64_t) {
    f_Y = py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 3;
    return 0;
}

int64_t STK55C324::py_lda_zp(int64_t) {
    f_A = py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t STK55C324::py_ldx_zp(int64_t) {
    f_X = py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 3;
    return 0;
}

int64_t STK55C324::py_tay(int64_t) {
    f_Y = f_A;
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 2;
    return 0;
}

int64_t STK55C324::py_lda_imm(int64_t) {
    f_A = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t STK55C324::py_tax(int64_t) {
    f_X = f_A;
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t STK55C324::py_ldy_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_Y = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 4;
    return 0;
}

int64_t STK55C324::py_lda_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_ldx_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_X = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 4;
    return 0;
}

int64_t STK55C324::py_bcs(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    if (f_CF) {
        v_rel = py_read_mem(f_PC);
        v_prev_PC = ((f_PC + 1) & 65535);
        f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
        return (3 + (((f_PC ^ v_prev_PC) > 255)));
    }
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_lda_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (5 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_lda_zp_ind(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_addr = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 5;
    return 0;
}

int64_t STK55C324::py_ldy_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_Y = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 4;
    return 0;
}

int64_t STK55C324::py_lda_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t STK55C324::py_ldx_zp_y(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_Y) & 255);
    f_X = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 4;
    return 0;
}

int64_t STK55C324::py_clv(int64_t) {
    f_VF = 0;
    return 2;
    return 0;
}

int64_t STK55C324::py_lda_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_tsx(int64_t) {
    f_X = f_SP;
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t STK55C324::py_ldy_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    f_Y = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_lda_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_ldx_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    f_X = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_cpy_imm(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_Y - py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 2;
    return 0;
}

int64_t STK55C324::py_cmp_ind_x(int64_t) {
    int64_t v_addr = 0, v_test_value = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_test_value = (f_A - py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 6;
    return 0;
}

int64_t STK55C324::py_cpy_zp(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_Y - py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 3;
    return 0;
}

int64_t STK55C324::py_cmp_zp(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_A - py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 3;
    return 0;
}

int64_t STK55C324::py_dec_zp(int64_t) {
    int64_t v_new_value = 0, v_opcode = 0;
    v_opcode = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    v_new_value = ((py_read_mem(v_opcode) - 1) & 255);
    py_write_mem(v_opcode, v_new_value);
    f_NF = (v_new_value >> 7);
    f_ZF = (!(v_new_value));
    return 5;
    return 0;
}

int64_t STK55C324::py_iny(int64_t) {
    f_Y = ((f_Y + 1) & 255);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 2;
    return 0;
}

int64_t STK55C324::py_cmp_imm(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_A - py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 2;
    return 0;
}

int64_t STK55C324::py_cpy_abs(int64_t) {
    int64_t v_addr = 0, v_test_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_test_value = (f_Y - py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 3;
    return 0;
}

int64_t STK55C324::py_cmp_abs(int64_t) {
    int64_t v_addr = 0, v_test_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_test_value = (f_A - py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 4;
    return 0;
}

int64_t STK55C324::py_dec_abs(int64_t) {
    int64_t v_addr = 0, v_new_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_new_value = ((py_read_mem(v_addr) - 1) & 255);
    py_write_mem(v_addr, v_new_value);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (v_new_value >> 7);
    f_ZF = (!(v_new_value));
    return 5;
    return 0;
}

int64_t STK55C324::py_dex(int64_t) {
    f_X = ((f_X - 1) & 255);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t STK55C324::py_bne(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    if ((!(f_ZF))) {
        v_rel = py_read_mem(f_PC);
        v_prev_PC = ((f_PC + 1) & 65535);
        f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
        return (3 + (((f_PC ^ v_prev_PC) > 255)));
    }
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_cmp_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_test_value = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    v_test_value = (f_A - py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return (5 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_cmp_zp_ind(int64_t) {
    int64_t v_addr = 0, v_test_value = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_addr = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_test_value = (f_A - py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 5;
    return 0;
}

int64_t STK55C324::py_cmp_zp_x(int64_t) {
    int64_t v_addr = 0, v_test_value = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    v_test_value = (f_A - py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 4;
    return 0;
}

int64_t STK55C324::py_dec_zp_x(int64_t) {
    int64_t v_addr = 0, v_new_value = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    v_new_value = ((py_read_mem(v_addr) - 1) & 255);
    py_write_mem(v_addr, v_new_value);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (v_new_value >> 7);
    f_ZF = (!(v_new_value));
    return 6;
    return 0;
}

int64_t STK55C324::py_cld(int64_t) {
    f_DF = 0;
    return 2;
    return 0;
}

int64_t STK55C324::py_cmp_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_test_value = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    v_test_value = (f_A - py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_phx(int64_t) {
    py_write_mem(f_SP, f_X);
    f_SP = ((f_SP - 1) & 255);
    return 3;
    return 0;
}

int64_t STK55C324::py_cmp_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_test_value = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    v_test_value = (f_A - py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_dec_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_new_value = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    v_new_value = ((py_read_mem(v_addr) - 1) & 255);
    py_write_mem(v_addr, v_new_value);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (v_new_value >> 7);
    f_ZF = (!(v_new_value));
    return 7;
    return 0;
}

int64_t STK55C324::py_cpx_imm(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_X - py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 2;
    return 0;
}

int64_t STK55C324::py_sbc_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t STK55C324::py_cpx_zp(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_X - py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 3;
    return 0;
}

int64_t STK55C324::py_sbc_zp(int64_t) {
    py_sbc(py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t STK55C324::py_inc_zp(int64_t) {
    int64_t v_new_value = 0, v_opcode = 0;
    v_opcode = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    v_new_value = ((py_read_mem(v_opcode) + 1) & 255);
    py_write_mem(v_opcode, v_new_value);
    f_NF = (v_new_value >> 7);
    f_ZF = (!(v_new_value));
    return 5;
    return 0;
}

int64_t STK55C324::py_inx(int64_t) {
    f_X = ((f_X + 1) & 255);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t STK55C324::py_sbc_imm(int64_t) {
    py_sbc(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_cpx_abs(int64_t) {
    int64_t v_addr = 0, v_test_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_test_value = (f_X - py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 4;
    return 0;
}

int64_t STK55C324::py_sbc_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_inc_abs(int64_t) {
    int64_t v_addr = 0, v_new_value = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_new_value = ((py_read_mem(v_addr) + 1) & 255);
    py_write_mem(v_addr, v_new_value);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (v_new_value >> 7);
    f_ZF = (!(v_new_value));
    return 6;
    return 0;
}

int64_t STK55C324::py_nop(int64_t) {
    return 2;
    return 0;
}

int64_t STK55C324::py_beq(int64_t) {
    int64_t v_prev_PC = 0, v_rel = 0;
    if (f_ZF) {
        v_rel = py_read_mem(f_PC);
        v_prev_PC = ((f_PC + 1) & 65535);
        f_PC = (((v_prev_PC + v_rel) - ((v_rel & 128) << 1)) & 65535);
        return (3 + (((f_PC ^ v_prev_PC) > 255)));
    }
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t STK55C324::py_sbc_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_sbc_zp_ind(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_addr = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t STK55C324::py_sbc_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t STK55C324::py_inc_zp_x(int64_t) {
    int64_t v_addr = 0, v_new_value = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    v_new_value = ((py_read_mem(v_addr) + 1) & 255);
    py_write_mem(v_addr, v_new_value);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (v_new_value >> 7);
    f_ZF = (!(v_new_value));
    return 6;
    return 0;
}

int64_t STK55C324::py_sed(int64_t) {
    f_DF = 1;
    return 2;
    return 0;
}

int64_t STK55C324::py_sbc_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_plx(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    f_X = py_read_mem(f_SP);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 4;
    return 0;
}

int64_t STK55C324::py_sbc_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t STK55C324::py_inc_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_new_value = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    v_new_value = ((py_read_mem(v_addr) + 1) & 255);
    py_write_mem(v_addr, v_new_value);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (v_new_value >> 7);
    f_ZF = (!(v_new_value));
    return 7;
    return 0;
}

int64_t STK55C324::py_dummy(int64_t) {
    (void)0;
    return 2;
    return 0;
}

const STK55C324::IoEntry* STK55C324::find_sfr_tbl(int64_t addr) {
    static const IoEntry e[] = {
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_standby_mode},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_sleep_mode},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_watch_timer_ctrl},
        {&STK55C324::py_get_sfr_irq_flag, &STK55C324::py_set_sfr_irq_flag},
        {&STK55C324::py_get_sfr_port1_data, &STK55C324::py_set_sfr_port1_data},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_port1_dir},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_volume_ch1},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_volume_ch2},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_port1_int},
        {&STK55C324::py_get_sfr_timer1_data, &STK55C324::py_set_sfr_timer1_data},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_timer1_div_contrast},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_ctrl},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_sound_clock},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_data_ch1},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_data_ch2},
        {&STK55C324::py_get_sfr_dummy, &STK55C324::py_set_sfr_sound_ctrl},
    };
    switch (addr) {
        case 4096: return &e[0];
        case 4097: return &e[1];
        case 4098: return &e[2];
        case 4099: return &e[3];
        case 4100: return &e[4];
        case 4101: return &e[5];
        case 4104: return &e[6];
        case 4105: return &e[7];
        case 4108: return &e[8];
        case 4109: return &e[9];
        case 4110: return &e[10];
        case 4111: return &e[11];
        case 4112: return &e[12];
        case 4113: return &e[13];
        case 4114: return &e[14];
        case 4115: return &e[15];
        default: return nullptr;
    }
}

const STK55C324::OpEntry STK55C324::T_execute[256] = {
    {&STK55C324::py_brk, 0},
    {&STK55C324::py_ora_ind_x, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_tsb_zp, 0},
    {&STK55C324::py_ora_zp, 0},
    {&STK55C324::py_asl_zp, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_php, 0},
    {&STK55C324::py_ora_imm, 0},
    {&STK55C324::py_asl_a, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_tsb_abs, 0},
    {&STK55C324::py_ora_abs, 0},
    {&STK55C324::py_asl_abs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bpl, 0},
    {&STK55C324::py_ora_ind_y, 0},
    {&STK55C324::py_ora_zp_ind, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_trb_zp, 0},
    {&STK55C324::py_ora_zp_x, 0},
    {&STK55C324::py_asl_zp_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_clc, 0},
    {&STK55C324::py_ora_abs_y, 0},
    {&STK55C324::py_inc_a, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_trb_abs, 0},
    {&STK55C324::py_ora_abs_x, 0},
    {&STK55C324::py_asl_abs_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_jsr_abs, 0},
    {&STK55C324::py_and_ind_x, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_bit_zp, 0},
    {&STK55C324::py_and_zp, 0},
    {&STK55C324::py_rol_zp, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_plp, 0},
    {&STK55C324::py_and_imm, 0},
    {&STK55C324::py_rol_a, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bit_abs, 0},
    {&STK55C324::py_and_abs, 0},
    {&STK55C324::py_rol_abs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bmi, 0},
    {&STK55C324::py_and_ind_y, 0},
    {&STK55C324::py_and_zp_ind, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bit_zp_x, 0},
    {&STK55C324::py_and_zp_x, 0},
    {&STK55C324::py_rol_zp_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_sec, 0},
    {&STK55C324::py_and_abs_y, 0},
    {&STK55C324::py_dec_a, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bit_abs_x, 0},
    {&STK55C324::py_and_abs_x, 0},
    {&STK55C324::py_rol_abs_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_rti, 0},
    {&STK55C324::py_eor_ind_x, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_eor_zp, 0},
    {&STK55C324::py_lsr_zp, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_pha, 0},
    {&STK55C324::py_eor_imm, 0},
    {&STK55C324::py_lsr_a, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_jmp_abs, 0},
    {&STK55C324::py_eor_abs, 0},
    {&STK55C324::py_lsr_abs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bvc, 0},
    {&STK55C324::py_eor_ind_y, 0},
    {&STK55C324::py_eor_zp_ind, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_eor_zp_x, 0},
    {&STK55C324::py_lsr_zp_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_cli, 0},
    {&STK55C324::py_eor_abs_y, 0},
    {&STK55C324::py_phy, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_eor_abs_x, 0},
    {&STK55C324::py_lsr_abs_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_rts, 0},
    {&STK55C324::py_adc_ind_x, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_stz_zp, 0},
    {&STK55C324::py_adc_zp, 0},
    {&STK55C324::py_ror_zp, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_pla, 0},
    {&STK55C324::py_adc_imm, 0},
    {&STK55C324::py_ror_a, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_jmp_ind, 0},
    {&STK55C324::py_adc_abs, 0},
    {&STK55C324::py_ror_abs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bvs, 0},
    {&STK55C324::py_adc_ind_y, 0},
    {&STK55C324::py_adc_zp_ind, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_stz_zp_x, 0},
    {&STK55C324::py_adc_zp_x, 0},
    {&STK55C324::py_ror_zp_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_sei, 0},
    {&STK55C324::py_adc_abs_y, 0},
    {&STK55C324::py_ply, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_jmp_abs_ind_x, 0},
    {&STK55C324::py_adc_abs_x, 0},
    {&STK55C324::py_ror_abs_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bra, 0},
    {&STK55C324::py_sta_ind_x, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_sty_zp, 0},
    {&STK55C324::py_sta_zp, 0},
    {&STK55C324::py_stx_zp, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_dey, 0},
    {&STK55C324::py_bit_imm, 0},
    {&STK55C324::py_txa, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_sty_abs, 0},
    {&STK55C324::py_sta_abs, 0},
    {&STK55C324::py_stx_abs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bcc, 0},
    {&STK55C324::py_sta_ind_y, 0},
    {&STK55C324::py_sta_zp_ind, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_sty_zp_x, 0},
    {&STK55C324::py_sta_zp_x, 0},
    {&STK55C324::py_stx_zp_y, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_tya, 0},
    {&STK55C324::py_sta_abs_y, 0},
    {&STK55C324::py_txs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_stz_abs, 0},
    {&STK55C324::py_sta_abs_x, 0},
    {&STK55C324::py_stz_abs_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_ldy_imm, 0},
    {&STK55C324::py_lda_ind_x, 0},
    {&STK55C324::py_ldx_imm, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_ldy_zp, 0},
    {&STK55C324::py_lda_zp, 0},
    {&STK55C324::py_ldx_zp, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_tay, 0},
    {&STK55C324::py_lda_imm, 0},
    {&STK55C324::py_tax, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_ldy_abs, 0},
    {&STK55C324::py_lda_abs, 0},
    {&STK55C324::py_ldx_abs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bcs, 0},
    {&STK55C324::py_lda_ind_y, 0},
    {&STK55C324::py_lda_zp_ind, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_ldy_zp_x, 0},
    {&STK55C324::py_lda_zp_x, 0},
    {&STK55C324::py_ldx_zp_y, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_clv, 0},
    {&STK55C324::py_lda_abs_y, 0},
    {&STK55C324::py_tsx, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_ldy_abs_x, 0},
    {&STK55C324::py_lda_abs_x, 0},
    {&STK55C324::py_ldx_abs_y, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_cpy_imm, 0},
    {&STK55C324::py_cmp_ind_x, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_cpy_zp, 0},
    {&STK55C324::py_cmp_zp, 0},
    {&STK55C324::py_dec_zp, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_iny, 0},
    {&STK55C324::py_cmp_imm, 0},
    {&STK55C324::py_dex, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_cpy_abs, 0},
    {&STK55C324::py_cmp_abs, 0},
    {&STK55C324::py_dec_abs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_bne, 0},
    {&STK55C324::py_cmp_ind_y, 0},
    {&STK55C324::py_cmp_zp_ind, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_cmp_zp_x, 0},
    {&STK55C324::py_dec_zp_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_cld, 0},
    {&STK55C324::py_cmp_abs_y, 0},
    {&STK55C324::py_phx, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_cmp_abs_x, 0},
    {&STK55C324::py_dec_abs_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_cpx_imm, 0},
    {&STK55C324::py_sbc_ind_x, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_cpx_zp, 0},
    {&STK55C324::py_sbc_zp, 0},
    {&STK55C324::py_inc_zp, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_inx, 0},
    {&STK55C324::py_sbc_imm, 0},
    {&STK55C324::py_nop, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_cpx_abs, 0},
    {&STK55C324::py_sbc_abs, 0},
    {&STK55C324::py_inc_abs, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_beq, 0},
    {&STK55C324::py_sbc_ind_y, 0},
    {&STK55C324::py_sbc_zp_ind, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_sbc_zp_x, 0},
    {&STK55C324::py_inc_zp_x, 0},
    {&STK55C324::py_dummy, 0},
    {&STK55C324::py_sed, 0},
    {&STK55C324::py_sbc_abs_y, 0},
    {&STK55C324::py_plx, 0},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_dummy, 1},
    {&STK55C324::py_sbc_abs_x, 0},
    {&STK55C324::py_inc_abs_x, 0},
    {&STK55C324::py_dummy, 0},
};
