#include "D750X.h"

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

int64_t D750X::rom_get_byte(int64_t a) const {
    const int64_t n = rom_size();
    return rom()[(size_t)(((a % n) + n) % n)];
}

void D750X::reset() {
    if (!inited_) {
        inited_ = true;
        init_();
    }
    py_reset();
}

void D750X::init_() {
    f_rom_mask = rom_get_mask();
    f_instr_counter = 0;
    f_timer_counter = 0;
    f_sound_port = options().num("sound.port", 0);
    { const std::vector<long> m = options().list("sound.mask"); f_sound_mask.assign(m.begin(), m.end()); }
    f_crystal_clock_div = ((double)((double)clock_hz()) / (32768));
    f_counter_div = {256, (64 * f_crystal_clock_div), f_crystal_clock_div, f_crystal_clock_div};
}

int64_t D750X::execute_scaled() {
    const int64_t scaled = std::llround((double)pyclock() * kTickHz);
    return scaled > 0 ? scaled : 1;
}

uint8_t D750X::display_byte(int i) const {
    return (i >= 0 && i < 24) ? (uint8_t)f_RAM[i] : 0;
}

int D750X::port_id(const std::string& name) const {
    if (name == "4") return kPort_4;
    if (name == "5") return kPort_5;
    if (name == "A") return kPort_A;
    if (name == "CY") return kPort_CY;
    if (name == "DE") return kPort_DE;
    if (name == "DL") return kPort_DL;
    if (name == "HL") return kPort_HL;
    if (name == "IE1") return kPort_IE1;
    if (name == "IET") return kPort_IET;
    if (name == "IME") return kPort_IME;
    if (name == "MEMORY") return kPort_MEMORY;
    if (name == "PC") return kPort_PC;
    if (name == "PORT") return kPort_PORT;
    if (name == "RAM") return kPort_RAM;
    if (name == "SIO") return kPort_SIO;
    if (name == "SIOCR") return kPort_SIOCR;
    if (name == "SP") return kPort_SP;
    if (name == "TCR") return kPort_TCR;
    if (name == "TMR") return kPort_TMR;
    return -1;
}

void D750X::port_input(int port, uint8_t mask, int level) {
    pypin_handler(port, mask, level);
}

void D750X::serialize(BrickStateIO& io) {
    io.pod(f_rom_mask);
    io.pod(f_instr_counter);
    io.pod(f_timer_counter);
    io.pod(f_sound_port);
    ser_vec(io, f_sound_mask);
    io.pod(f_crystal_clock_div);
    ser_vec(io, f_counter_div);
    io.pod(f_A);
    io.pod(f_HL);
    io.pod(f_DE);
    io.pod(f_SP);
    io.pod(f_PC);
    io.pod(f_CF);
    io.pod(f_SK);
    io.pod(f_HALT);
    io.pod(f_STOP);
    io.pod(f_IER);
    io.pod(f_RQF);
    io.pod(f_IME);
    ser_vec(io, f_P);
    io.pod(f_SIO);
    io.pod(f_SIOCR);
    io.pod(f_TCR);
    io.pod(f_TMR);
    ser_vec(io, f_RAM);
    io.pod(f_rom);
}


void D750X::py_reset() {
    f_A = 0;
    f_HL = 0;
    f_DE = 0;
    f_SP = 0;
    f_PC = 0;
    f_CF = 0;
    f_SK = 0;
    f_HALT = 0;
    f_STOP = 0;
    f_IER = 0;
    f_RQF = 0;
    f_IME = 0;
    f_P.assign((size_t)(16), 0);
    f_SIO = 0;
    f_SIOCR = 0;
    f_TCR = 0;
    f_TMR = 255;
    f_RAM.assign((size_t)(256), 0);
}

void D750X::pyreset() {
    py_reset();
}

void D750X::pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level) {
    v_port = ((int64_t)(v_port));
    f_P[v_port] &= (~(v_mask));
    if (((v_level > 0))) {
        f_P[v_port] |= v_mask;
    }
}

void D750X::py_interrupt(int64_t v_addr) {
    f_IME = 0;
    f_RAM[((f_SP - 1) & 255)] = ((f_PC >> 4) & 15);
    f_RAM[((f_SP - 2) & 255)] = (f_PC & 15);
    f_RAM[((f_SP - 3) & 255)] = ((f_SK << 2) | f_CF);
    f_SP = ((f_SP - 4) & 255);
    f_RAM[f_SP] = ((f_PC >> 8) & 15);
    f_PC = v_addr;
}

int64_t D750X::pyclock() {
    int64_t v_data = 0, v_exec_cycles = 0, v_opcode = 0;
    v_exec_cycles = 1;
    if ((!(((f_HALT) || (f_STOP))))) {
        {
            const auto a_ = rom_get_byte(f_PC);
            v_data = a_;
            v_opcode = a_;
        }
        f_PC = ((f_PC + 1) & f_rom_mask);
        if (((T_execute[v_data].size == 2))) {
            v_opcode = ((v_data << 8) | rom_get_byte(f_PC));
            f_PC = ((f_PC + 1) & f_rom_mask);
        }
        v_exec_cycles = (this->*T_execute[v_data].fn)(v_opcode);
        f_instr_counter += 1;
        if (f_IME) {
            if (((f_IER & f_RQF) & 1)) {
                f_RQF &= (~(1));
                py_interrupt(16);
            }
        }
    }
    if ((((!(f_STOP))) || (((f_P[12] > 0))))) {
        f_timer_counter -= v_exec_cycles;
        while (((f_timer_counter <= 0))) {
            f_timer_counter += f_counter_div[(f_P[12] & 3)];
            f_TCR = ((f_TCR + 1) & 255);
            if (((f_TCR == f_TMR))) {
                f_TCR = 0;
                f_RQF |= 1;
                {
                    const auto a_ = 0;
                    f_STOP = a_;
                    f_HALT = a_;
                }
            }
        }
    }
    return v_exec_cycles;
    return 0;
}

int64_t D750X::py_instruction_00111110(int64_t v_opcode) {
    return (this->*T_instruction_00111110_tbl[(v_opcode & 255)].fn)(v_opcode);
    return 0;
}

int64_t D750X::py_instruction_00111111(int64_t v_opcode) {
    return (this->*T_instruction_00111111_tbl[(v_opcode & 255)].fn)(v_opcode);
    return 0;
}

int64_t D750X::py_dummy(int64_t v_opcode) {
    (void)0;
    return 0;
}

int64_t D750X::py_skip_next() {
    int64_t v_byte_count = 0, v_opcode = 0;
    v_opcode = rom_get_byte(f_PC);
    v_byte_count = T_execute[v_opcode].size;
    f_PC = ((f_PC + v_byte_count) & f_rom_mask);
    f_SK = 3;
    return (v_byte_count * 2);
    return 0;
}

int64_t D750X::py_aisc_data(int64_t v_opcode) {
    int64_t v_prev = 0, v_res = 0;
    v_res = (f_A + v_opcode);
    f_A = (v_res & 15);
    if (((v_res > 15))) {
        v_prev = rom_get_byte((f_PC - 2));
        if (((((v_prev != 124))) || (f_CF))) {
            return (2 + py_skip_next());
        }
    }
    return 2;
    return 0;
}

int64_t D750X::py_lai_data(int64_t v_opcode) {
    int64_t v_cycles = 0;
    f_A = (v_opcode & 15);
    v_cycles = 2;
    while ((((rom_get_byte(f_PC) >> 4) == 1))) {
        f_PC = ((f_PC + 1) & f_rom_mask);
        v_cycles += 2;
    }
    return v_cycles;
    return 0;
}

int64_t D750X::py_jmp_addr(int64_t v_opcode) {
    f_PC = (v_opcode & 4095);
    return 4;
    return 0;
}

int64_t D750X::py_call_addr(int64_t v_opcode) {
    f_RAM[((f_SP - 1) & 255)] = ((f_PC >> 4) & 15);
    f_RAM[((f_SP - 2) & 255)] = (f_PC & 15);
    f_RAM[((f_SP - 3) & 255)] = ((f_SK << 2) | f_CF);
    f_SP = ((f_SP - 4) & 255);
    f_RAM[f_SP] = ((f_PC >> 8) & 15);
    f_PC = (v_opcode & 2047);
    return 4;
    return 0;
}

int64_t D750X::py_ladr_addr(int64_t v_opcode) {
    f_A = f_RAM[(v_opcode & 127)];
    return 4;
    return 0;
}

int64_t D750X::py_xadr(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = f_RAM[(v_opcode & 127)];
    f_RAM[(v_opcode & 127)] = f_A;
    f_A = v_new_A;
    return 4;
    return 0;
}

int64_t D750X::py_xhdr(int64_t v_opcode) {
    int64_t v_new_H = 0;
    v_new_H = f_RAM[(v_opcode & 127)];
    f_RAM[(v_opcode & 127)] = (f_HL >> 4);
    f_HL = ((v_new_H << 4) | (f_HL & 15));
    return 4;
    return 0;
}

int64_t D750X::py_xldr(int64_t v_opcode) {
    int64_t v_new_L = 0;
    v_new_L = f_RAM[(v_opcode & 127)];
    f_RAM[(v_opcode & 127)] = (f_HL & 15);
    f_HL = ((f_HL & 240) | v_new_L);
    return 4;
    return 0;
}

int64_t D750X::py_ddrs_addr(int64_t v_opcode) {
    int64_t v_res = 0;
    {
        const auto a_ = ((f_RAM[(v_opcode & 127)] - 1) & 15);
        v_res = a_;
        f_RAM[(v_opcode & 127)] = a_;
    }
    if (((v_res == 15))) {
        return (4 + py_skip_next());
    }
    return 4;
    return 0;
}

int64_t D750X::py_idrs_addr(int64_t v_opcode) {
    int64_t v_res = 0;
    {
        const auto a_ = ((f_RAM[(v_opcode & 127)] + 1) & 15);
        v_res = a_;
        f_RAM[(v_opcode & 127)] = a_;
    }
    if (((v_res == 0))) {
        return (4 + py_skip_next());
    }
    return 4;
    return 0;
}

int64_t D750X::py_lei_data(int64_t v_opcode) {
    f_DE = ((f_DE & 240) | (v_opcode & 15));
    return 4;
    return 0;
}

int64_t D750X::py_lli_data(int64_t v_opcode) {
    f_HL = ((f_HL & 240) | (v_opcode & 15));
    return 4;
    return 0;
}

int64_t D750X::py_ldi_data(int64_t v_opcode) {
    f_DE = (((v_opcode & 15) << 4) | (f_DE & 15));
    return 4;
    return 0;
}

int64_t D750X::py_lhi_data(int64_t v_opcode) {
    f_HL = (((v_opcode & 15) << 4) | (f_HL & 15));
    return 4;
    return 0;
}

int64_t D750X::py_skeei_data(int64_t v_opcode) {
    if ((((f_DE & 15) == (v_opcode & 15)))) {
        return (4 + py_skip_next());
    }
    return 4;
    return 0;
}

int64_t D750X::py_sklei_data(int64_t v_opcode) {
    if ((((f_HL & 15) == (v_opcode & 15)))) {
        return (4 + py_skip_next());
    }
    return 4;
    return 0;
}

int64_t D750X::py_skdei_data(int64_t v_opcode) {
    if ((((f_DE >> 4) == (v_opcode & 15)))) {
        return (4 + py_skip_next());
    }
    return 4;
    return 0;
}

int64_t D750X::py_skhei_data(int64_t v_opcode) {
    if ((((f_HL >> 4) == (v_opcode & 15)))) {
        return (4 + py_skip_next());
    }
    return 4;
    return 0;
}

int64_t D750X::py_tae(int64_t v_opcode) {
    f_DE = ((f_DE & 240) | f_A);
    return 4;
    return 0;
}

int64_t D750X::py_pshde(int64_t v_opcode) {
    f_RAM[((f_SP - 1) & 255)] = (f_DE >> 4);
    f_SP = ((f_SP - 2) & 255);
    f_RAM[f_SP] = (f_DE & 15);
    return 4;
    return 0;
}

int64_t D750X::py_popde(int64_t v_opcode) {
    f_DE = ((f_RAM[((f_SP + 1) & 255)] << 4) | f_RAM[f_SP]);
    f_SP = ((f_SP + 2) & 255);
    return 4;
    return 0;
}

int64_t D750X::py_tea(int64_t v_opcode) {
    f_A = (f_DE & 15);
    return 4;
    return 0;
}

int64_t D750X::py_tal(int64_t v_opcode) {
    f_HL = ((f_HL & 240) | f_A);
    return 4;
    return 0;
}

int64_t D750X::py_tla(int64_t v_opcode) {
    f_A = (f_HL & 15);
    return 4;
    return 0;
}

int64_t D750X::py_pshhl(int64_t v_opcode) {
    f_RAM[((f_SP - 1) & 255)] = (f_HL >> 4);
    f_SP = ((f_SP - 2) & 255);
    f_RAM[f_SP] = (f_HL & 15);
    return 4;
    return 0;
}

int64_t D750X::py_pophl(int64_t v_opcode) {
    f_HL = ((f_RAM[((f_SP + 1) & 255)] << 4) | f_RAM[f_SP]);
    f_SP = ((f_SP + 2) & 255);
    return 4;
    return 0;
}

int64_t D750X::py_tad(int64_t v_opcode) {
    f_DE = ((f_A << 4) | (f_DE & 15));
    return 4;
    return 0;
}

int64_t D750X::py_tda(int64_t v_opcode) {
    f_A = (f_DE >> 4);
    return 4;
    return 0;
}

int64_t D750X::py_tah(int64_t v_opcode) {
    f_HL = ((f_A << 4) | (f_HL & 15));
    return 4;
    return 0;
}

int64_t D750X::py_tha(int64_t v_opcode) {
    f_A = (f_HL >> 4);
    return 4;
    return 0;
}

int64_t D750X::py_jam_addr(int64_t v_opcode) {
    f_PC = ((((v_opcode & 7) << 8) | (f_A << 4)) | f_RAM[f_HL]);
    return 4;
    return 0;
}

int64_t D750X::py_tamsp(int64_t v_opcode) {
    f_SP = (((f_A << 4) | f_RAM[f_HL]) & 247);
    return 4;
    return 0;
}

int64_t D750X::py_tspam(int64_t v_opcode) {
    f_A = (f_SP >> 4);
    f_RAM[f_HL] = (f_SP & 14);
    return 4;
    return 0;
}

int64_t D750X::py_halt(int64_t v_opcode) {
    f_HALT = 1;
    return 4;
    return 0;
}

int64_t D750X::py_stop(int64_t v_opcode) {
    f_STOP = 1;
    return 4;
    return 0;
}

int64_t D750X::py_timer(int64_t v_opcode) {
    f_TCR = 0;
    f_RQF &= (~(1));
    return 4;
    return 0;
}

int64_t D750X::py_sio(int64_t v_opcode) {
    f_SIOCR = 0;
    f_RQF &= (~(2));
    return 4;
    return 0;
}

int64_t D750X::py_tsioam(int64_t v_opcode) {
    f_RAM[f_HL] = (f_SIO & 15);
    f_A = (f_SIO >> 4);
    return 4;
    return 0;
}

int64_t D750X::py_tcntam(int64_t v_opcode) {
    f_A = (f_TCR >> 4);
    f_RAM[f_HL] = (f_TCR & 15);
    return 4;
    return 0;
}

int64_t D750X::py_tamsio(int64_t v_opcode) {
    f_SIO = ((f_A << 4) | f_RAM[f_HL]);
    return 4;
    return 0;
}

int64_t D750X::py_tammod(int64_t v_opcode) {
    f_TMR = ((f_A << 4) | f_RAM[f_HL]);
    return 4;
    return 0;
}

int64_t D750X::py_ski_data(int64_t v_opcode) {
    if ((f_RQF & v_opcode)) {
        f_RQF &= (~(v_opcode));
        return (4 + py_skip_next());
    }
    return 4;
    return 0;
}

int64_t D750X::py_skaei_data(int64_t v_opcode) {
    if (((f_A == (v_opcode & 15)))) {
        return (4 + py_skip_next());
    }
    return 4;
    return 0;
}

int64_t D750X::py_ip54(int64_t v_opcode) {
    f_A = f_P[5];
    f_RAM[f_HL] = f_P[4];
    return 4;
    return 0;
}

int64_t D750X::py_op54(int64_t v_opcode) {
    f_P[5] = f_A;
    f_P[4] = f_RAM[f_HL];
    return 4;
    return 0;
}

int64_t D750X::py_anl(int64_t v_opcode) {
    f_A &= f_RAM[f_HL];
    return 4;
    return 0;
}

int64_t D750X::py_rar(int64_t v_opcode) {
    int64_t v_cf = 0;
    v_cf = (f_A & 1);
    f_A = ((f_CF << 3) | (f_A >> 1));
    f_CF = v_cf;
    return 4;
    return 0;
}

int64_t D750X::py_orl(int64_t v_opcode) {
    f_A |= f_RAM[f_HL];
    return 4;
    return 0;
}

int64_t D750X::py_ip_addr(int64_t v_opcode) {
    f_A = f_P[(v_opcode & 15)];
    return 4;
    return 0;
}

int64_t D750X::py_op_addr(int64_t v_opcode) {
    int64_t v_port = 0;
    v_port = (v_opcode & 15);
    f_P[v_port] = f_A;
    if (((v_port == f_sound_port))) {
        snd_toggle((((f_P[f_sound_port] & f_sound_mask[0]) > 0)), (((f_P[f_sound_port] & f_sound_mask[1]) > 0)));
    }
    return 4;
    return 0;
}

int64_t D750X::py_di_data(int64_t v_opcode) {
    if ((((v_opcode & 7) == 0))) {
        f_IME = 0;
    } else {
        f_IER &= (~(v_opcode));
    }
    return 4;
    return 0;
}

int64_t D750X::py_ei_data(int64_t v_opcode) {
    f_IER |= (v_opcode & 7);
    if ((((v_opcode & 7) == 0))) {
        f_IME = 1;
    }
    return 4;
    return 0;
}

int64_t D750X::py_lam_dl(int64_t v_opcode) {
    f_A = f_RAM[((f_DE & 240) | (f_HL & 15))];
    return 2;
    return 0;
}

int64_t D750X::py_lam_de(int64_t v_opcode) {
    f_A = f_RAM[f_DE];
    return 2;
    return 0;
}

int64_t D750X::py_rtpsw(int64_t v_opcode) {
    int64_t v_psw = 0;
    f_PC = (f_RAM[f_SP] << 8);
    v_psw = f_RAM[((f_SP + 1) & 255)];
    f_CF = (v_psw & 1);
    f_SK = (v_psw >> 2);
    f_PC |= f_RAM[((f_SP + 2) & 255)];
    f_PC |= (f_RAM[((f_SP + 3) & 255)] << 4);
    f_SP = ((f_SP + 4) & 255);
    return 4;
    return 0;
}

int64_t D750X::py_xam_dl(int64_t v_opcode) {
    int64_t v_a = 0, v_dl = 0;
    v_dl = ((f_DE & 240) | (f_HL & 15));
    v_a = f_RAM[v_dl];
    f_RAM[v_dl] = f_A;
    f_A = v_a;
    return 2;
    return 0;
}

int64_t D750X::py_xam_de(int64_t v_opcode) {
    int64_t v_a = 0;
    v_a = f_RAM[f_DE];
    f_RAM[f_DE] = f_A;
    f_A = v_a;
    return 2;
    return 0;
}

int64_t D750X::py_des(int64_t v_opcode) {
    f_DE = ((f_DE & 240) | ((f_DE - 1) & 15));
    if ((((f_DE & 15) == 15))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_ies(int64_t v_opcode) {
    f_DE = ((f_DE & 240) | ((f_DE + 1) & 15));
    if ((((f_DE & 15) == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_xad(int64_t v_opcode) {
    int64_t v_a = 0;
    v_a = (f_DE >> 4);
    f_DE = ((f_A << 4) | (f_DE & 15));
    f_A = v_a;
    return 2;
    return 0;
}

int64_t D750X::py_xae(int64_t v_opcode) {
    int64_t v_a = 0;
    v_a = (f_DE & 15);
    f_DE = ((f_DE & 240) | f_A);
    f_A = v_a;
    return 2;
    return 0;
}

int64_t D750X::py_anp_data(int64_t v_opcode) {
    int64_t v_port = 0;
    v_port = ((v_opcode >> 4) & 15);
    f_P[v_port] &= v_opcode;
    if (((v_port == f_sound_port))) {
        snd_toggle((((f_P[f_sound_port] & f_sound_mask[0]) > 0)), (((f_P[f_sound_port] & f_sound_mask[1]) > 0)));
    }
    return 4;
    return 0;
}

int64_t D750X::py_orp_data(int64_t v_opcode) {
    int64_t v_port = 0;
    v_port = ((v_opcode >> 4) & 15);
    f_P[v_port] |= (v_opcode & 15);
    if (((v_port == f_sound_port))) {
        snd_toggle((((f_P[f_sound_port] & f_sound_mask[0]) > 0)), (((f_P[f_sound_port] & f_sound_mask[1]) > 0)));
    }
    return 4;
    return 0;
}

int64_t D750X::py_lhli_data(int64_t v_opcode) {
    int64_t v_cycles = 0;
    f_HL = (v_opcode & 255);
    v_cycles = 4;
    v_opcode = rom_get_byte(f_PC);
    while (((((v_opcode == 78))) || ((((v_opcode >> 4) == 12))))) {
        v_cycles += py_skip_next();
        v_opcode = rom_get_byte(f_PC);
    }
    return v_cycles;
    return 0;
}

int64_t D750X::py_ldei_data(int64_t v_opcode) {
    f_DE = (v_opcode & 255);
    return 4;
    return 0;
}

int64_t D750X::py_lam_hld(int64_t v_opcode) {
    f_A = f_RAM[f_HL];
    f_HL = ((f_HL & 240) | ((f_HL - 1) & 15));
    if ((((f_HL & 15) == 15))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_lam_hli(int64_t v_opcode) {
    f_A = f_RAM[f_HL];
    f_HL = ((f_HL & 240) | ((f_HL + 1) & 15));
    if ((((f_HL & 15) == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_lam_hl(int64_t v_opcode) {
    f_A = f_RAM[f_HL];
    return 2;
    return 0;
}

int64_t D750X::py_rt(int64_t v_opcode) {
    f_PC = (f_RAM[f_SP] << 8);
    f_PC |= f_RAM[((f_SP + 2) & 255)];
    f_PC |= (f_RAM[((f_SP + 3) & 255)] << 4);
    f_SP = ((f_SP + 4) & 255);
    return 2;
    return 0;
}

int64_t D750X::py_xam_hld(int64_t v_opcode) {
    int64_t v_a = 0;
    v_a = f_RAM[f_HL];
    f_RAM[f_HL] = f_A;
    f_A = v_a;
    f_HL = ((f_HL & 240) | ((f_HL - 1) & 15));
    if ((((f_HL & 15) == 15))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_xam_hli(int64_t v_opcode) {
    int64_t v_a = 0;
    v_a = f_RAM[f_HL];
    f_RAM[f_HL] = f_A;
    f_A = v_a;
    f_HL = ((f_HL & 240) | ((f_HL + 1) & 15));
    if ((((f_HL & 15) == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_xam_hl(int64_t v_opcode) {
    int64_t v_a = 0;
    v_a = f_RAM[f_HL];
    f_RAM[f_HL] = f_A;
    f_A = v_a;
    return 2;
    return 0;
}

int64_t D750X::py_st(int64_t v_opcode) {
    f_RAM[f_HL] = f_A;
    return 2;
    return 0;
}

int64_t D750X::py_dls(int64_t v_opcode) {
    f_HL = ((f_HL & 240) | ((f_HL - 1) & 15));
    if ((((f_HL & 15) == 15))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_ils(int64_t v_opcode) {
    f_HL = ((f_HL & 240) | ((f_HL + 1) & 15));
    if ((((f_HL & 15) == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_skc(int64_t v_opcode) {
    if (f_CF) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_rts(int64_t v_opcode) {
    f_PC = (f_RAM[f_SP] << 8);
    f_PC |= f_RAM[((f_SP + 2) & 255)];
    f_PC |= (f_RAM[((f_SP + 3) & 255)] << 4);
    f_SP = ((f_SP + 4) & 255);
    return (2 + py_skip_next());
    return 0;
}

int64_t D750X::py_lamt(int64_t v_opcode) {
    int64_t v_data = 0;
    v_data = rom_get_byte((((f_PC & 1984) | (f_CF << 4)) | f_A));
    f_A = (v_data >> 4);
    f_RAM[f_HL] = (v_data & 15);
    return 4;
    return 0;
}

int64_t D750X::py_skaem(int64_t v_opcode) {
    if (((f_A == f_RAM[f_HL]))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_skmbf_data(int64_t v_opcode) {
    if ((((f_RAM[f_HL] & (1 << (v_opcode & 3))) == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_skmbt_data(int64_t v_opcode) {
    if ((f_RAM[f_HL] & (1 << (v_opcode & 3)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_rmb(int64_t v_opcode) {
    f_RAM[f_HL] &= (~((1 << (v_opcode & 3))));
    return 2;
    return 0;
}

int64_t D750X::py_smb(int64_t v_opcode) {
    f_RAM[f_HL] |= (1 << (v_opcode & 3));
    return 2;
    return 0;
}

int64_t D750X::py_ipl(int64_t v_opcode) {
    f_A = f_P[(f_HL & 15)];
    return 2;
    return 0;
}

int64_t D750X::py_ip1(int64_t v_opcode) {
    f_A = f_P[1];
    return 2;
    return 0;
}

int64_t D750X::py_opl(int64_t v_opcode) {
    int64_t v_port = 0;
    v_port = (f_HL & 15);
    f_P[v_port] = f_A;
    if (((v_port == f_sound_port))) {
        snd_toggle((((f_P[f_sound_port] & f_sound_mask[0]) > 0)), (((f_P[f_sound_port] & f_sound_mask[1]) > 0)));
    }
    return 2;
    return 0;
}

int64_t D750X::py_op3(int64_t v_opcode) {
    f_P[3] = f_A;
    return 2;
    return 0;
}

int64_t D750X::py_skabt_data(int64_t v_opcode) {
    if ((f_A & (1 << (v_opcode & 3)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_rc(int64_t v_opcode) {
    f_CF = 0;
    return 2;
    return 0;
}

int64_t D750X::py_sc(int64_t v_opcode) {
    f_CF = 1;
    return 2;
    return 0;
}

int64_t D750X::py_xah(int64_t v_opcode) {
    int64_t v_a = 0;
    v_a = (f_HL >> 4);
    f_HL = ((f_A << 4) | (f_HL & 15));
    f_A = v_a;
    return 2;
    return 0;
}

int64_t D750X::py_xal(int64_t v_opcode) {
    int64_t v_a = 0;
    v_a = (f_HL & 15);
    f_HL = ((f_HL & 240) | f_A);
    f_A = v_a;
    return 2;
    return 0;
}

int64_t D750X::py_acsc(int64_t v_opcode) {
    int64_t v_res = 0;
    v_res = ((f_A + f_RAM[f_HL]) + f_CF);
    f_A = (v_res & 15);
    if (((v_res > 15))) {
        f_CF = 1;
        return (2 + py_skip_next());
    }
    f_CF = 0;
    return 2;
    return 0;
}

int64_t D750X::py_asc(int64_t v_opcode) {
    int64_t v_res = 0;
    v_res = (f_A + f_RAM[f_HL]);
    f_A = (v_res & 15);
    if (((v_res > 15))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t D750X::py_exl(int64_t v_opcode) {
    f_A ^= f_RAM[f_HL];
    return 2;
    return 0;
}

int64_t D750X::py_cma(int64_t v_opcode) {
    f_A ^= 15;
    return 2;
    return 0;
}

int64_t D750X::py_jcp_addr(int64_t v_opcode) {
    f_PC = ((f_PC & 1984) | (v_opcode & 63));
    return 2;
    return 0;
}

int64_t D750X::py_lhlt_addr(int64_t v_opcode) {
    int64_t v_cycles = 0;
    f_HL = rom_get_byte((192 | (v_opcode & 15)));
    v_cycles = 4;
    v_opcode = rom_get_byte(f_PC);
    while (((((v_opcode == 78))) || ((((v_opcode >> 4) == 12))))) {
        v_cycles += py_skip_next();
        v_opcode = rom_get_byte(f_PC);
    }
    return v_cycles;
    return 0;
}

int64_t D750X::py_calt_addr(int64_t v_opcode) {
    int64_t v_tdata = 0;
    f_RAM[((f_SP - 1) & 255)] = ((f_PC >> 4) & 15);
    f_RAM[((f_SP - 2) & 255)] = (f_PC & 15);
    f_RAM[((f_SP - 3) & 255)] = ((f_SK << 2) | f_CF);
    f_SP = ((f_SP - 4) & 255);
    f_RAM[f_SP] = ((f_PC >> 8) & 15);
    v_tdata = rom_get_byte(v_opcode);
    f_PC = (((v_tdata << 2) & 896) | (v_tdata & 31));
    return 4;
    return 0;
}


const D750X::OpEntry D750X::T_execute[256] = {
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_aisc_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_lai_data, 1},
    {&D750X::py_jmp_addr, 2},
    {&D750X::py_jmp_addr, 2},
    {&D750X::py_jmp_addr, 2},
    {&D750X::py_jmp_addr, 2},
    {&D750X::py_jmp_addr, 2},
    {&D750X::py_jmp_addr, 2},
    {&D750X::py_jmp_addr, 2},
    {&D750X::py_jmp_addr, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_call_addr, 2},
    {&D750X::py_call_addr, 2},
    {&D750X::py_call_addr, 2},
    {&D750X::py_call_addr, 2},
    {&D750X::py_call_addr, 2},
    {&D750X::py_call_addr, 2},
    {&D750X::py_call_addr, 2},
    {&D750X::py_call_addr, 2},
    {&D750X::py_ladr_addr, 2},
    {&D750X::py_xadr, 2},
    {&D750X::py_xhdr, 2},
    {&D750X::py_xldr, 2},
    {&D750X::py_ddrs_addr, 2},
    {&D750X::py_idrs_addr, 2},
    {&D750X::py_instruction_00111110, 2},
    {&D750X::py_instruction_00111111, 2},
    {&D750X::py_lam_dl, 1},
    {&D750X::py_lam_de, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_rtpsw, 1},
    {&D750X::py_xam_dl, 1},
    {&D750X::py_xam_de, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_des, 1},
    {&D750X::py_ies, 1},
    {&D750X::py_xad, 1},
    {&D750X::py_xae, 1},
    {&D750X::py_anp_data, 2},
    {&D750X::py_orp_data, 2},
    {&D750X::py_lhli_data, 2},
    {&D750X::py_ldei_data, 2},
    {&D750X::py_lam_hld, 1},
    {&D750X::py_lam_hli, 1},
    {&D750X::py_lam_hl, 1},
    {&D750X::py_rt, 1},
    {&D750X::py_xam_hld, 1},
    {&D750X::py_xam_hli, 1},
    {&D750X::py_xam_hl, 1},
    {&D750X::py_st, 1},
    {&D750X::py_dls, 1},
    {&D750X::py_ils, 1},
    {&D750X::py_skc, 1},
    {&D750X::py_rts, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_lamt, 1},
    {&D750X::py_skaem, 1},
    {&D750X::py_skmbf_data, 1},
    {&D750X::py_skmbf_data, 1},
    {&D750X::py_skmbf_data, 1},
    {&D750X::py_skmbf_data, 1},
    {&D750X::py_skmbt_data, 1},
    {&D750X::py_skmbt_data, 1},
    {&D750X::py_skmbt_data, 1},
    {&D750X::py_skmbt_data, 1},
    {&D750X::py_rmb, 1},
    {&D750X::py_rmb, 1},
    {&D750X::py_rmb, 1},
    {&D750X::py_rmb, 1},
    {&D750X::py_smb, 1},
    {&D750X::py_smb, 1},
    {&D750X::py_smb, 1},
    {&D750X::py_smb, 1},
    {&D750X::py_ipl, 1},
    {&D750X::py_ip1, 1},
    {&D750X::py_opl, 1},
    {&D750X::py_op3, 1},
    {&D750X::py_skabt_data, 1},
    {&D750X::py_skabt_data, 1},
    {&D750X::py_skabt_data, 1},
    {&D750X::py_skabt_data, 1},
    {&D750X::py_rc, 1},
    {&D750X::py_sc, 1},
    {&D750X::py_xah, 1},
    {&D750X::py_xal, 1},
    {&D750X::py_acsc, 1},
    {&D750X::py_asc, 1},
    {&D750X::py_exl, 1},
    {&D750X::py_cma, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_jcp_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_lhlt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
    {&D750X::py_calt_addr, 1},
};

const D750X::OpEntry D750X::T_instruction_00111110_tbl[256] = {
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lei_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_lli_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_ldi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_lhi_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_skeei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_sklei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skdei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_skhei_data, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_tae, 2},
    {&D750X::py_tea, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_pshde, 2},
    {&D750X::py_popde, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_tal, 2},
    {&D750X::py_tla, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_pshhl, 2},
    {&D750X::py_pophl, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_tad, 2},
    {&D750X::py_tda, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_tah, 2},
    {&D750X::py_tha, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
};

const D750X::OpEntry D750X::T_instruction_00111111_tbl[256] = {
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_jam_addr, 2},
    {&D750X::py_jam_addr, 2},
    {&D750X::py_jam_addr, 2},
    {&D750X::py_jam_addr, 2},
    {&D750X::py_jam_addr, 2},
    {&D750X::py_jam_addr, 2},
    {&D750X::py_jam_addr, 2},
    {&D750X::py_jam_addr, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_tamsp, 2},
    {&D750X::py_timer, 2},
    {&D750X::py_sio, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_tspam, 2},
    {&D750X::py_halt, 2},
    {&D750X::py_stop, 2},
    {&D750X::py_ip54, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_tsioam, 2},
    {&D750X::py_tcntam, 2},
    {&D750X::py_op54, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_tamsio, 2},
    {&D750X::py_tammod, 2},
    {&D750X::py_ski_data, 2},
    {&D750X::py_ski_data, 2},
    {&D750X::py_ski_data, 2},
    {&D750X::py_ski_data, 2},
    {&D750X::py_ski_data, 2},
    {&D750X::py_ski_data, 2},
    {&D750X::py_ski_data, 2},
    {&D750X::py_ski_data, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_skaei_data, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_di_data, 2},
    {&D750X::py_di_data, 2},
    {&D750X::py_di_data, 2},
    {&D750X::py_di_data, 2},
    {&D750X::py_di_data, 2},
    {&D750X::py_di_data, 2},
    {&D750X::py_di_data, 2},
    {&D750X::py_di_data, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_ei_data, 2},
    {&D750X::py_ei_data, 2},
    {&D750X::py_ei_data, 2},
    {&D750X::py_ei_data, 2},
    {&D750X::py_ei_data, 2},
    {&D750X::py_ei_data, 2},
    {&D750X::py_ei_data, 2},
    {&D750X::py_ei_data, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_anl, 2},
    {&D750X::py_rar, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_orl, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_ip_addr, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_op_addr, 2},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
    {&D750X::py_dummy, 1},
};
