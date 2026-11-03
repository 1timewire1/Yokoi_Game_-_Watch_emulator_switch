#include "LC5732.h"

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

int64_t LC5732::rom_get_byte(int64_t a) const {
    const int64_t n = rom_size();
    return rom()[(size_t)(((a % n) + n) % n)];
}

void LC5732::reset() {
    if (!inited_) {
        inited_ = true;
        init_();
    }
    py_reset();
}

void LC5732::init_() {
    f_cycle_counter = 0;
    f_instr_counter = 0;
    f_500ms_div = ((double)((double)clock_hz()) / (2));
    f_100ms_div = ((double)((double)clock_hz()) / (10));
    f_32ms_div = ((double)((double)clock_hz()) / (((double)(1000) / (32))));
}

int64_t LC5732::execute_scaled() {
    const int64_t scaled = std::llround((double)pyclock() * kTickHz);
    return scaled > 0 ? scaled : 1;
}

uint8_t LC5732::display_byte(int i) const {
    return (i >= 0 && i < 16) ? (uint8_t)f_LCDRAM[i] : 0;
}

int LC5732::port_id(const std::string& name) const {
    if (name == "AC") return kPort_AC;
    if (name == "ALM") return kPort_ALM;
    if (name == "CF") return kPort_CF;
    if (name == "CSTF") return kPort_CSTF;
    if (name == "DP") return kPort_DP;
    if (name == "EDP") return kPort_EDP;
    if (name == "HALT") return kPort_HALT;
    if (name == "HEF") return kPort_HEF;
    if (name == "LCDRAM") return kPort_LCDRAM;
    if (name == "LGT") return kPort_LGT;
    if (name == "MEMORY") return kPort_MEMORY;
    if (name == "PAGE") return kPort_PAGE;
    if (name == "PC") return kPort_PC;
    if (name == "PDF") return kPort_PDF;
    if (name == "PM") return kPort_PM;
    if (name == "PS") return kPort_PS;
    if (name == "RAM") return kPort_RAM;
    if (name == "SP") return kPort_SP;
    if (name == "STACK") return kPort_STACK;
    if (name == "STS") return kPort_STS;
    if (name == "TREG") return kPort_TREG;
    return -1;
}

void LC5732::port_input(int port, uint8_t mask, int level) {
    pypin_handler(port, mask, level);
}

void LC5732::serialize(BrickStateIO& io) {
    io.pod(f_cycle_counter);
    io.pod(f_instr_counter);
    io.pod(f_500ms_div);
    io.pod(f_100ms_div);
    io.pod(f_32ms_div);
    io.pod(f_AC);
    io.pod(f_DP);
    io.pod(f_EDP);
    io.pod(f_SP);
    io.pod(f_PC);
    io.pod(f_CF);
    io.pod(f_STACK);
    io.pod(f_TREG);
    io.pod(f_PAGE);
    io.pod(f_HALT);
    io.pod(f_ALM);
    io.pod(f_CSTF);
    io.pod(f_HEF);
    io.pod(f_STS);
    io.pod(f_PS);
    io.pod(f_PM);
    io.pod(f_LIGHT);
    io.pod(f_PDF);
    ser_vec(io, f_RAM);
    ser_vec(io, f_LCDRAM);
    io.pod(f_BACKUP);
    io.pod(f_self);
    io.pod(f_LGT);
    io.pod(f_rom);
}

void LC5732::snd_set_alm(int64_t alm) {
    // LC5732sound.py. The table is sound_freq_tbl.<octave>=<Hz>,<Hz>,...
    if (snd_tbl_.empty()) {
        for (int o = 0; o < 3; o++) {
            std::vector<double> row;
            const std::string key = "sound_freq_tbl." + std::to_string(o);
            if (options().has(key)) {
                const char* p = options().str(key).c_str();
                while (*p) {
                    char* end = nullptr;
                    row.push_back(std::strtod(p, &end));
                    p = *end == ',' ? end + 1 : end;
                    if (end == p) break;
                }
            }
            snd_tbl_.push_back(row);
        }
    }
    const int scal = alm & 0xF, octave = (alm >> 4) & 3;
    if (scal != 0xF && octave != 3 && scal < (int)snd_tbl_[octave].size()) set_tone(snd_tbl_[octave][scal]);
    else set_tone(0);
}

void LC5732::py_reset() {
    f_AC = 0;
    f_DP = 0;
    f_EDP = 0;
    f_SP = 0;
    f_PC = 0;
    f_CF = 0;
    f_STACK = 0;
    f_TREG = 0;
    f_PAGE = 0;
    f_HALT = 0;
    f_ALM = 255;
    f_CSTF = 0;
    f_HEF = 1;
    f_STS = 0;
    f_PS = 0;
    f_PM = 0;
    f_LIGHT = 0;
    f_PDF = 0;
    f_RAM.assign((size_t)(48), 0);
    f_LCDRAM.assign((size_t)(16), 255);
    snd_set_alm(f_ALM);
}

void LC5732::pyreset() {
    py_reset();
}

void LC5732::pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level) {
    if (((v_port == kPort_PS))) {
        f_PS &= (~(v_mask));
        if (((v_level > 0))) {
            f_PS |= v_mask;
            f_STS |= 4;
            if ((f_HEF & 4)) {
                f_HALT = 0;
            }
        }
    } else {
        if (((v_port == kPort_PM))) {
            f_PM &= (~(v_mask));
            if (((v_level > 0))) {
                f_PM |= v_mask;
                f_STS |= 8;
                if ((f_HEF & 8)) {
                    f_HALT = 0;
                }
            }
        } else {
            if (((v_port == kPort_RES))) {
                if (((v_level == 1))) {
                    py_reset();
                    f_HALT = 1;
                } else {
                    f_HALT = 0;
                }
            }
        }
    }
}

void LC5732::py_timers_clock(int64_t v_exec_cycles) {
    if (((py_mod(f_cycle_counter, f_32ms_div) <= v_exec_cycles))) {
        f_STS |= 2;
        if ((f_HEF & 2)) {
            f_HALT = 0;
        }
    }
    if (f_CSTF) {
        if (((py_mod(f_cycle_counter, f_100ms_div) <= v_exec_cycles))) {
            f_STS |= 16;
            if ((f_HEF & 16)) {
                f_HALT = 0;
            }
        }
    }
    if (((py_mod(f_cycle_counter, f_500ms_div) <= v_exec_cycles))) {
        f_STS |= 1;
        if ((f_HEF & 1)) {
            f_HALT = 0;
        }
    }
}

int64_t LC5732::pyclock() {
    int64_t v_exec_cycles = 0, v_opcode = 0;
    v_exec_cycles = 4;
    if (((f_PS == 15))) {
        py_reset();
        f_PS = 15;
    }
    if ((!(f_HALT))) {
        v_opcode = rom_get_byte(f_PC);
        f_PC += 1;
        v_exec_cycles = (this->*T_execute[v_opcode].fn)(v_opcode);
        f_instr_counter += 1;
    }
    f_cycle_counter += v_exec_cycles;
    py_timers_clock(v_exec_cycles);
    return v_exec_cycles;
    return 0;
}

int64_t LC5732::py_get_ram() {
    if (((f_DP < 48))) {
        return f_RAM[f_DP];
    }
    return 0;
    return 0;
}

void LC5732::py_set_ram(int64_t v_value) {
    if (((f_DP < 48))) {
        f_RAM[f_DP] = v_value;
    }
}

int64_t LC5732::py_halt(int64_t v_opcode) {
    f_HALT = 1;
    return 4;
    return 0;
}

int64_t LC5732::py_taat(int64_t v_opcode) {
    int64_t v_addr = 0, v_data = 0;
    v_addr = ((1792 | (f_AC << 4)) | py_get_ram());
    v_data = rom_get_byte(v_addr);
    f_AC = (v_data >> 4);
    f_TREG = (v_data & 15);
    return 8;
    return 0;
}

int64_t LC5732::py_twrt(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((1792 | (f_AC << 4)) | py_get_ram());
    f_LCDRAM[f_SP] = rom_get_byte(v_addr);
    return 8;
    return 0;
}

int64_t LC5732::py_tmel(int64_t v_opcode) {
    int64_t v_addr = 0, v_data = 0;
    v_addr = ((1792 | (f_AC << 4)) | py_get_ram());
    v_data = rom_get_byte(v_addr);
    f_ALM = v_data;
    snd_set_alm(v_data);
    return 8;
    return 0;
}

int64_t LC5732::py_csp(int64_t v_opcode) {
    f_CSTF = 0;
    return 4;
    return 0;
}

int64_t LC5732::py_cst(int64_t v_opcode) {
    f_CSTF = 1;
    return 4;
    return 0;
}

int64_t LC5732::py_rc5(int64_t v_opcode) {
    f_HEF &= (~(1));
    return 4;
    return 0;
}

int64_t LC5732::py_sc5(int64_t v_opcode) {
    f_HEF |= 1;
    return 4;
    return 0;
}

int64_t LC5732::py_jmp_x(int64_t v_opcode) {
    f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    return 8;
    return 0;
}

int64_t LC5732::py_jmp_p(int64_t v_opcode) {
    f_PC = (((f_PAGE << 8) | (f_AC << 4)) | py_get_ram());
    return 4;
    return 0;
}

int64_t LC5732::py_page(int64_t v_opcode) {
    f_PAGE = (py_get_ram() & 7);
    return 4;
    return 0;
}

int64_t LC5732::py_mtr(int64_t v_opcode) {
    py_set_ram(f_TREG);
    return 4;
    return 0;
}

int64_t LC5732::py_rts(int64_t v_opcode) {
    f_PC = f_STACK;
    f_STACK = 0;
    return 4;
    return 0;
}

int64_t LC5732::py_mpcl(int64_t v_opcode) {
    py_set_ram(((f_PC - 1) & 15));
    return 4;
    return 0;
}

int64_t LC5732::py_mpcm(int64_t v_opcode) {
    py_set_ram((((f_PC - 1) >> 4) & 15));
    return 4;
    return 0;
}

int64_t LC5732::py_mpch(int64_t v_opcode) {
    py_set_ram((((f_PC - 1) >> 8) & 15));
    return 4;
    return 0;
}

int64_t LC5732::py_in(int64_t v_opcode) {
    return 4;
    return 0;
}

int64_t LC5732::py_asr0(int64_t v_opcode) {
    f_AC = (f_AC >> 1);
    return 4;
    return 0;
}

int64_t LC5732::py_asr1(int64_t v_opcode) {
    f_AC = ((f_AC >> 1) | 8);
    return 4;
    return 0;
}

int64_t LC5732::py_asl0(int64_t v_opcode) {
    f_AC = ((f_AC << 1) & 15);
    return 4;
    return 0;
}

int64_t LC5732::py_asl1(int64_t v_opcode) {
    f_AC = (((f_AC << 1) & 15) | 1);
    return 4;
    return 0;
}

int64_t LC5732::py_sdpl(int64_t v_opcode) {
    f_DP = ((f_DP & 240) | f_AC);
    return 4;
    return 0;
}

int64_t LC5732::py_sdph(int64_t v_opcode) {
    f_DP = ((f_DP & 15) | (f_AC << 4));
    return 4;
    return 0;
}

int64_t LC5732::py_edpl(int64_t v_opcode) {
    int64_t v_tmp = 0;
    v_tmp = f_DP;
    f_DP = ((f_DP & 240) | (f_EDP & 15));
    f_EDP = ((f_EDP & 240) | (v_tmp & 15));
    return 4;
    return 0;
}

int64_t LC5732::py_edph(int64_t v_opcode) {
    int64_t v_tmp = 0;
    v_tmp = f_DP;
    f_DP = ((f_DP & 15) | (f_EDP & 240));
    f_EDP = ((f_EDP & 15) | (v_tmp & 240));
    return 4;
    return 0;
}

int64_t LC5732::py_mvi_x(int64_t v_opcode) {
    py_set_ram((v_opcode & 15));
    return 4;
    return 0;
}

int64_t LC5732::py_ldi_x(int64_t v_opcode) {
    f_AC = (v_opcode & 15);
    return 4;
    return 0;
}

int64_t LC5732::py_baz_x(int64_t v_opcode) {
    if (((f_AC == 0))) {
        f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    } else {
        f_PC += 1;
        return 4;
    }
    return 8;
    return 0;
}

int64_t LC5732::py_bab0_x(int64_t v_opcode) {
    if ((f_AC & 1)) {
        f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    } else {
        f_PC += 1;
        return 4;
    }
    return 8;
    return 0;
}

int64_t LC5732::py_banz_x(int64_t v_opcode) {
    if (f_AC) {
        f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    } else {
        f_PC += 1;
        return 4;
    }
    return 8;
    return 0;
}

int64_t LC5732::py_bab1_x(int64_t v_opcode) {
    if ((f_AC & 2)) {
        f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    } else {
        f_PC += 1;
        return 4;
    }
    return 8;
    return 0;
}

int64_t LC5732::py_bcnh_x(int64_t v_opcode) {
    if (((f_CF == 0))) {
        f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    } else {
        f_PC += 1;
        return 4;
    }
    return 8;
    return 0;
}

int64_t LC5732::py_bab2_x(int64_t v_opcode) {
    if ((f_AC & 4)) {
        f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    } else {
        f_PC += 1;
        return 4;
    }
    return 8;
    return 0;
}

int64_t LC5732::py_bch_x(int64_t v_opcode) {
    if (f_CF) {
        f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    } else {
        f_PC += 1;
        return 4;
    }
    return 8;
    return 0;
}

int64_t LC5732::py_bab3_x(int64_t v_opcode) {
    if ((f_AC & 8)) {
        f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    } else {
        f_PC += 1;
        return 4;
    }
    return 8;
    return 0;
}

int64_t LC5732::py_adc(int64_t v_opcode) {
    int64_t v_result = 0;
    v_result = ((f_AC + py_get_ram()) + f_CF);
    f_AC = (v_result & 15);
    f_CF = ((v_result > 15));
    return 4;
    return 0;
}

int64_t LC5732::py_sbc(int64_t v_opcode) {
    int64_t v_result = 0;
    v_result = ((f_AC + (py_get_ram() ^ 15)) + f_CF);
    f_AC = (v_result & 15);
    f_CF = ((v_result > 15));
    return 4;
    return 0;
}

int64_t LC5732::py_add(int64_t v_opcode) {
    int64_t v_result = 0;
    v_result = (f_AC + py_get_ram());
    f_AC = (v_result & 15);
    f_CF = ((v_result > 15));
    return 4;
    return 0;
}

int64_t LC5732::py_sub(int64_t v_opcode) {
    int64_t v_result = 0;
    v_result = ((f_AC + (py_get_ram() ^ 15)) + 1);
    f_AC = (v_result & 15);
    f_CF = ((v_result > 15));
    return 4;
    return 0;
}

int64_t LC5732::py_adn(int64_t v_opcode) {
    f_AC = ((f_AC + py_get_ram()) & 15);
    return 4;
    return 0;
}

int64_t LC5732::py_and(int64_t v_opcode) {
    f_AC = (f_AC & py_get_ram());
    return 4;
    return 0;
}

int64_t LC5732::py_eor(int64_t v_opcode) {
    f_AC = (f_AC ^ py_get_ram());
    return 4;
    return 0;
}

int64_t LC5732::py_or(int64_t v_opcode) {
    f_AC = (f_AC | py_get_ram());
    return 4;
    return 0;
}

int64_t LC5732::py_adc_m(int64_t v_opcode) {
    int64_t v_result = 0;
    v_result = ((f_AC + py_get_ram()) + f_CF);
    f_AC = (v_result & 15);
    py_set_ram(f_AC);
    f_CF = ((v_result > 15));
    return 4;
    return 0;
}

int64_t LC5732::py_sbc_m(int64_t v_opcode) {
    int64_t v_result = 0;
    v_result = ((f_AC + (py_get_ram() ^ 15)) + f_CF);
    f_AC = (v_result & 15);
    py_set_ram(f_AC);
    f_CF = ((v_result > 15));
    return 4;
    return 0;
}

int64_t LC5732::py_add_m(int64_t v_opcode) {
    int64_t v_result = 0;
    v_result = (f_AC + py_get_ram());
    f_AC = (v_result & 15);
    py_set_ram(f_AC);
    f_CF = ((v_result > 15));
    return 4;
    return 0;
}

int64_t LC5732::py_sub_m(int64_t v_opcode) {
    int64_t v_result = 0;
    v_result = ((f_AC + (py_get_ram() ^ 15)) + 1);
    f_AC = (v_result & 15);
    py_set_ram(f_AC);
    f_CF = ((v_result > 15));
    return 4;
    return 0;
}

int64_t LC5732::py_adn_m(int64_t v_opcode) {
    f_AC = ((f_AC + py_get_ram()) & 15);
    py_set_ram(f_AC);
    return 4;
    return 0;
}

int64_t LC5732::py_and_m(int64_t v_opcode) {
    f_AC = (f_AC & py_get_ram());
    py_set_ram(f_AC);
    return 4;
    return 0;
}

int64_t LC5732::py_eor_m(int64_t v_opcode) {
    f_AC = (f_AC ^ py_get_ram());
    py_set_ram(f_AC);
    return 4;
    return 0;
}

int64_t LC5732::py_or_m(int64_t v_opcode) {
    f_AC = (f_AC | py_get_ram());
    py_set_ram(f_AC);
    return 4;
    return 0;
}

int64_t LC5732::py_adci(int64_t v_opcode) {
    int64_t v_result = 0, v_x = 0;
    v_x = (rom_get_byte(f_PC) & 15);
    v_result = ((f_AC + v_x) + f_CF);
    f_AC = (v_result & 15);
    f_CF = ((v_result > 15));
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_sbci(int64_t v_opcode) {
    int64_t v_result = 0, v_x = 0;
    v_x = (rom_get_byte(f_PC) & 15);
    v_result = ((f_AC + (v_x ^ 15)) + f_CF);
    f_AC = (v_result & 15);
    f_CF = ((v_result > 15));
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_addi(int64_t v_opcode) {
    int64_t v_result = 0, v_x = 0;
    v_x = (rom_get_byte(f_PC) & 15);
    v_result = (f_AC + v_x);
    f_AC = (v_result & 15);
    f_CF = ((v_result > 15));
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_subi(int64_t v_opcode) {
    int64_t v_result = 0, v_x = 0;
    v_x = (rom_get_byte(f_PC) & 15);
    v_result = ((f_AC + (v_x ^ 15)) + 1);
    f_AC = (v_result & 15);
    f_CF = ((v_result > 15));
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_adni(int64_t v_opcode) {
    int64_t v_x = 0;
    v_x = (rom_get_byte(f_PC) & 15);
    f_AC = ((f_AC + v_x) & 15);
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_andi(int64_t v_opcode) {
    int64_t v_x = 0;
    v_x = (rom_get_byte(f_PC) & 15);
    f_AC = (f_AC & v_x);
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_eori(int64_t v_opcode) {
    int64_t v_x = 0;
    v_x = (rom_get_byte(f_PC) & 15);
    f_AC = (f_AC ^ v_x);
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_ori(int64_t v_opcode) {
    int64_t v_x = 0;
    v_x = (rom_get_byte(f_PC) & 15);
    f_AC = (f_AC | v_x);
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_inc(int64_t v_opcode) {
    f_AC = ((py_get_ram() + 1) & 15);
    py_set_ram(f_AC);
    return 4;
    return 0;
}

int64_t LC5732::py_dec(int64_t v_opcode) {
    f_AC = ((py_get_ram() - 1) & 15);
    py_set_ram(f_AC);
    return 4;
    return 0;
}

int64_t LC5732::py_idpl(int64_t v_opcode) {
    f_DP = ((f_DP & 240) | ((f_DP + 1) & 15));
    return 4;
    return 0;
}

int64_t LC5732::py_ddpl(int64_t v_opcode) {
    f_DP = ((f_DP & 240) | ((f_DP - 1) & 15));
    return 4;
    return 0;
}

int64_t LC5732::py_idph(int64_t v_opcode) {
    f_DP = ((f_DP + 16) & 255);
    return 4;
    return 0;
}

int64_t LC5732::py_ddph(int64_t v_opcode) {
    f_DP = ((f_DP - 16) & 255);
    return 4;
    return 0;
}

int64_t LC5732::py_isp(int64_t v_opcode) {
    f_SP = ((f_SP + 1) & 15);
    return 4;
    return 0;
}

int64_t LC5732::py_dsp(int64_t v_opcode) {
    f_SP = ((f_SP - 1) & 15);
    return 4;
    return 0;
}

int64_t LC5732::py_jsr_x(int64_t v_opcode) {
    f_STACK = (f_PC + 1);
    f_PC = (((v_opcode & 7) << 8) | rom_get_byte(f_PC));
    return 8;
    return 0;
}

int64_t LC5732::py_ipm(int64_t v_opcode) {
    f_AC = f_PM;
    return 4;
    return 0;
}

int64_t LC5732::py_lda(int64_t v_opcode) {
    f_AC = py_get_ram();
    return 4;
    return 0;
}

int64_t LC5732::py_lsp(int64_t v_opcode) {
    f_AC = f_SP;
    return 4;
    return 0;
}

int64_t LC5732::py_lhlt(int64_t v_opcode) {
    f_AC = (f_STS >> 1);
    f_STS &= (~((((2 | 4) | 8) | 16)));
    return 4;
    return 0;
}

int64_t LC5732::py_l500(int64_t v_opcode) {
    f_AC = (f_STS & 1);
    f_STS &= (~(1));
    return 4;
    return 0;
}

int64_t LC5732::py_sta(int64_t v_opcode) {
    py_set_ram(f_AC);
    return 4;
    return 0;
}

int64_t LC5732::py_ssp(int64_t v_opcode) {
    f_SP = f_AC;
    return 4;
    return 0;
}

int64_t LC5732::py_ips(int64_t v_opcode) {
    f_AC = f_PS;
    return 4;
    return 0;
}

int64_t LC5732::py_mdpl_x(int64_t v_opcode) {
    f_DP = ((f_DP & 240) | (v_opcode & 15));
    return 4;
    return 0;
}

int64_t LC5732::py_mdph_x(int64_t v_opcode) {
    f_DP = ((f_DP & 15) | ((v_opcode & 15) << 4));
    return 4;
    return 0;
}

int64_t LC5732::py_sic_x(int64_t v_opcode) {
    f_HEF = ((f_HEF & 1) | ((v_opcode & 15) << 1));
    return 4;
    return 0;
}

int64_t LC5732::py_msp_x(int64_t v_opcode) {
    f_SP = (v_opcode & 15);
    return 4;
    return 0;
}

int64_t LC5732::py_rcf(int64_t v_opcode) {
    f_CF = 0;
    return 4;
    return 0;
}

int64_t LC5732::py_scf(int64_t v_opcode) {
    f_CF = 1;
    return 4;
    return 0;
}

int64_t LC5732::py_rlgt(int64_t v_opcode) {
    f_LIGHT = 0;
    return 4;
    return 0;
}

int64_t LC5732::py_slgt(int64_t v_opcode) {
    f_LIGHT = 1;
    return 4;
    return 0;
}

int64_t LC5732::py_spdr_x(int64_t v_opcode) {
    f_PDF = (v_opcode & 3);
    return 4;
    return 0;
}

int64_t LC5732::py_rbak(int64_t v_opcode) {
    f_BACKUP = 0;
    return 4;
    return 0;
}

int64_t LC5732::py_sbak(int64_t v_opcode) {
    f_BACKUP = 1;
    return 4;
    return 0;
}

int64_t LC5732::py_sas_x(int64_t v_opcode) {
    f_ALM = rom_get_byte(f_PC);
    snd_set_alm(f_ALM);
    f_PC += 1;
    return 8;
    return 0;
}

int64_t LC5732::py_csec(int64_t v_opcode) {
    f_STS &= (~(((1 | 2) | 16)));
    return 4;
    return 0;
}

int64_t LC5732::py_out(int64_t v_opcode) {
    f_LCDRAM[f_SP] = ((f_AC << 4) | py_get_ram());
    return 4;
    return 0;
}

int64_t LC5732::py_ldpl(int64_t v_opcode) {
    f_AC = (f_DP & 15);
    return 4;
    return 0;
}

int64_t LC5732::py_ldph(int64_t v_opcode) {
    f_AC = (f_DP >> 4);
    return 4;
    return 0;
}

int64_t LC5732::py_nop(int64_t v_opcode) {
    return 4;
    return 0;
}


const LC5732::OpEntry LC5732::T_execute[256] = {
    {&LC5732::py_halt, 0},
    {&LC5732::py_taat, 0},
    {&LC5732::py_twrt, 0},
    {&LC5732::py_tmel, 0},
    {&LC5732::py_csp, 0},
    {&LC5732::py_cst, 0},
    {&LC5732::py_rc5, 0},
    {&LC5732::py_sc5, 0},
    {&LC5732::py_jmp_x, 0},
    {&LC5732::py_jmp_x, 0},
    {&LC5732::py_jmp_x, 0},
    {&LC5732::py_jmp_x, 0},
    {&LC5732::py_jmp_x, 0},
    {&LC5732::py_jmp_x, 0},
    {&LC5732::py_jmp_x, 0},
    {&LC5732::py_jmp_x, 0},
    {&LC5732::py_jmp_p, 0},
    {&LC5732::py_page, 0},
    {&LC5732::py_mtr, 0},
    {&LC5732::py_rts, 0},
    {&LC5732::py_mpcl, 0},
    {&LC5732::py_mpcm, 0},
    {&LC5732::py_mpch, 0},
    {&LC5732::py_in, 0},
    {&LC5732::py_asr0, 0},
    {&LC5732::py_asr1, 0},
    {&LC5732::py_asl0, 0},
    {&LC5732::py_asl1, 0},
    {&LC5732::py_sdpl, 0},
    {&LC5732::py_sdph, 0},
    {&LC5732::py_edpl, 0},
    {&LC5732::py_edph, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_mvi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_ldi_x, 0},
    {&LC5732::py_baz_x, 0},
    {&LC5732::py_baz_x, 0},
    {&LC5732::py_baz_x, 0},
    {&LC5732::py_baz_x, 0},
    {&LC5732::py_baz_x, 0},
    {&LC5732::py_baz_x, 0},
    {&LC5732::py_baz_x, 0},
    {&LC5732::py_baz_x, 0},
    {&LC5732::py_bab0_x, 0},
    {&LC5732::py_bab0_x, 0},
    {&LC5732::py_bab0_x, 0},
    {&LC5732::py_bab0_x, 0},
    {&LC5732::py_bab0_x, 0},
    {&LC5732::py_bab0_x, 0},
    {&LC5732::py_bab0_x, 0},
    {&LC5732::py_bab0_x, 0},
    {&LC5732::py_banz_x, 0},
    {&LC5732::py_banz_x, 0},
    {&LC5732::py_banz_x, 0},
    {&LC5732::py_banz_x, 0},
    {&LC5732::py_banz_x, 0},
    {&LC5732::py_banz_x, 0},
    {&LC5732::py_banz_x, 0},
    {&LC5732::py_banz_x, 0},
    {&LC5732::py_bab1_x, 0},
    {&LC5732::py_bab1_x, 0},
    {&LC5732::py_bab1_x, 0},
    {&LC5732::py_bab1_x, 0},
    {&LC5732::py_bab1_x, 0},
    {&LC5732::py_bab1_x, 0},
    {&LC5732::py_bab1_x, 0},
    {&LC5732::py_bab1_x, 0},
    {&LC5732::py_bcnh_x, 0},
    {&LC5732::py_bcnh_x, 0},
    {&LC5732::py_bcnh_x, 0},
    {&LC5732::py_bcnh_x, 0},
    {&LC5732::py_bcnh_x, 0},
    {&LC5732::py_bcnh_x, 0},
    {&LC5732::py_bcnh_x, 0},
    {&LC5732::py_bcnh_x, 0},
    {&LC5732::py_bab2_x, 0},
    {&LC5732::py_bab2_x, 0},
    {&LC5732::py_bab2_x, 0},
    {&LC5732::py_bab2_x, 0},
    {&LC5732::py_bab2_x, 0},
    {&LC5732::py_bab2_x, 0},
    {&LC5732::py_bab2_x, 0},
    {&LC5732::py_bab2_x, 0},
    {&LC5732::py_bch_x, 0},
    {&LC5732::py_bch_x, 0},
    {&LC5732::py_bch_x, 0},
    {&LC5732::py_bch_x, 0},
    {&LC5732::py_bch_x, 0},
    {&LC5732::py_bch_x, 0},
    {&LC5732::py_bch_x, 0},
    {&LC5732::py_bch_x, 0},
    {&LC5732::py_bab3_x, 0},
    {&LC5732::py_bab3_x, 0},
    {&LC5732::py_bab3_x, 0},
    {&LC5732::py_bab3_x, 0},
    {&LC5732::py_bab3_x, 0},
    {&LC5732::py_bab3_x, 0},
    {&LC5732::py_bab3_x, 0},
    {&LC5732::py_bab3_x, 0},
    {&LC5732::py_adc, 0},
    {&LC5732::py_sbc, 0},
    {&LC5732::py_add, 0},
    {&LC5732::py_sub, 0},
    {&LC5732::py_adn, 0},
    {&LC5732::py_and, 0},
    {&LC5732::py_eor, 0},
    {&LC5732::py_or, 0},
    {&LC5732::py_adc_m, 0},
    {&LC5732::py_sbc_m, 0},
    {&LC5732::py_add_m, 0},
    {&LC5732::py_sub_m, 0},
    {&LC5732::py_adn_m, 0},
    {&LC5732::py_and_m, 0},
    {&LC5732::py_eor_m, 0},
    {&LC5732::py_or_m, 0},
    {&LC5732::py_adci, 0},
    {&LC5732::py_sbci, 0},
    {&LC5732::py_addi, 0},
    {&LC5732::py_subi, 0},
    {&LC5732::py_adni, 0},
    {&LC5732::py_andi, 0},
    {&LC5732::py_eori, 0},
    {&LC5732::py_ori, 0},
    {&LC5732::py_inc, 0},
    {&LC5732::py_dec, 0},
    {&LC5732::py_idpl, 0},
    {&LC5732::py_ddpl, 0},
    {&LC5732::py_idph, 0},
    {&LC5732::py_ddph, 0},
    {&LC5732::py_isp, 0},
    {&LC5732::py_dsp, 0},
    {&LC5732::py_jsr_x, 0},
    {&LC5732::py_jsr_x, 0},
    {&LC5732::py_jsr_x, 0},
    {&LC5732::py_jsr_x, 0},
    {&LC5732::py_jsr_x, 0},
    {&LC5732::py_jsr_x, 0},
    {&LC5732::py_jsr_x, 0},
    {&LC5732::py_jsr_x, 0},
    {&LC5732::py_ipm, 0},
    {&LC5732::py_lda, 0},
    {&LC5732::py_lsp, 0},
    {&LC5732::py_lhlt, 0},
    {&LC5732::py_l500, 0},
    {&LC5732::py_sta, 0},
    {&LC5732::py_ssp, 0},
    {&LC5732::py_ips, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdpl_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_mdph_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_sic_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_msp_x, 0},
    {&LC5732::py_rcf, 0},
    {&LC5732::py_scf, 0},
    {&LC5732::py_rlgt, 0},
    {&LC5732::py_slgt, 0},
    {&LC5732::py_spdr_x, 0},
    {&LC5732::py_spdr_x, 0},
    {&LC5732::py_spdr_x, 0},
    {&LC5732::py_spdr_x, 0},
    {&LC5732::py_rbak, 0},
    {&LC5732::py_sbak, 0},
    {&LC5732::py_sas_x, 0},
    {&LC5732::py_csec, 0},
    {&LC5732::py_out, 0},
    {&LC5732::py_ldpl, 0},
    {&LC5732::py_ldph, 0},
    {&LC5732::py_nop, 0},
};
