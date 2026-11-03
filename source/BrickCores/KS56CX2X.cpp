#include "KS56CX2X.h"

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

const int64_t k_MAIN_CLOCK_DIV[] = {64, 16, 8, 4};
const int64_t k_BASIC_TIMER_DIV[] = {4096, 4096, 512, 512, 128, 128, 32};
const int64_t k_TIMER_T0_DIV[] = {0, 0, 0, 0, 1024, 256, 64, 16};
const int64_t k_WATCH_TIMER_DIV[] = {16384, 128};

}  // namespace

int64_t KS56CX2X::rom_get_byte(int64_t a) const {
    const int64_t n = rom_size();
    return rom()[(size_t)(((a % n) + n) % n)];
}

void KS56CX2X::reset() {
    if (!inited_) {
        inited_ = true;
        init_();
    }
    py_reset();
}

void KS56CX2X::init_() {
    f_instr_counter = 0;
    f_basic_timer_counter = 0;
    f_T0_timer_counter = 0;
    f_watch_timer_counter = 0;
    f_timerWatch_counter = 0;
    f_sub_clock_div = ((double)((double)clock_hz()) / (32768));
    f_cpu_clock_div = k_MAIN_CLOCK_DIV[0];
}

int64_t KS56CX2X::execute_scaled() {
    const int64_t scaled = std::llround((double)pyclock() * kTickHz);
    return scaled > 0 ? scaled : 1;
}

uint8_t KS56CX2X::display_byte(int i) const {
    if (i < 0 || i >= 256) return 0;
    return (uint8_t)f_RAM[256 + i];
}

int KS56CX2X::port_id(const std::string& name) const {
    if (name == "BC") return kPort_BC;
    if (name == "BCE") return kPort_BCE;
    if (name == "CY") return kPort_CY;
    if (name == "DE") return kPort_DE;
    if (name == "DEE") return kPort_DEE;
    if (name == "HL") return kPort_HL;
    if (name == "HLE") return kPort_HLE;
    if (name == "IME") return kPort_IME;
    if (name == "IORAM") return kPort_IORAM;
    if (name == "MBE") return kPort_MBE;
    if (name == "MBS") return kPort_MBS;
    if (name == "MEMORY") return kPort_MEMORY;
    if (name == "PC") return kPort_PC;
    if (name == "PORT1") return kPort_PORT1;
    if (name == "PORT6") return kPort_PORT6;
    if (name == "RAM0") return kPort_RAM0;
    if (name == "RAM1") return kPort_RAM1;
    if (name == "RBE") return kPort_RBE;
    if (name == "RBS") return kPort_RBS;
    if (name == "SBS") return kPort_SBS;
    if (name == "SP") return kPort_SP;
    if (name == "XA") return kPort_XA;
    if (name == "XAE") return kPort_XAE;
    return -1;
}

void KS56CX2X::port_input(int port, uint8_t mask, int level) {
    pypin_handler(port, mask, level);
}

void KS56CX2X::serialize(BrickStateIO& io) {
    io.pod(f_instr_counter);
    io.pod(f_basic_timer_counter);
    io.pod(f_T0_timer_counter);
    io.pod(f_watch_timer_counter);
    io.pod(f_timerWatch_counter);
    io.pod(f_sub_clock_div);
    io.pod(f_cpu_clock_div);
    io.pod(f_PC);
    io.pod(f_CY);
    io.pod(f_SP);
    io.pod(f_MBE);
    io.pod(f_MBS);
    io.pod(f_RBE);
    io.pod(f_RBS);
    io.pod(f_SBS);
    io.pod(f_TMOD2H);
    io.pod(f_WDTM);
    io.pod(f_BT);
    io.pod(f_BTM);
    io.pod(f_IST);
    io.pod(f_SK);
    io.pod(f_SCC);
    io.pod(f_WMl);
    io.pod(f_WMh);
    io.pod(f_TM0l);
    io.pod(f_TM0h);
    io.pod(f_T0);
    io.pod(f_TMOD0);
    io.pod(f_IME);
    io.pod(f_IPS);
    io.pod(f_PCC_MODE);
    io.pod(f_PCC_CLOCK);
    io.pod(f_IM0);
    io.pod(f_IM1);
    io.pod(f_IM2);
    io.pod(f_INTA);
    io.pod(f_INTC);
    io.pod(f_INTE);
    io.pod(f_INTF);
    io.pod(f_INTG);
    io.pod(f_INTH);
    ser_vec(io, f_PORT0);
    ser_vec(io, f_PORT1);
    ser_vec(io, f_PORT2);
    ser_vec(io, f_PORT3);
    ser_vec(io, f_PORT5);
    ser_vec(io, f_PORT6);
    ser_vec(io, f_PORT8);
    ser_vec(io, f_PORT9);
    io.pod(f_PORT2_OUT_LATCH);
    io.pod(f_PORT3_OUT_LATCH);
    io.pod(f_PORT5_OUT_LATCH);
    io.pod(f_PORT6_OUT_LATCH);
    io.pod(f_PORT8_OUT_LATCH);
    io.pod(f_PORT9_OUT_LATCH);
    io.pod(f_POGA);
    io.pod(f_POGB);
    io.pod(f_PM3);
    io.pod(f_PM6);
    ser_vec(io, f_RAM);
    io.pod(f_watch_timer_enable);
    io.pod(f_rom);
}


void KS56CX2X::py_init_registers() {
    f_PC = 0;
    f_CY = 0;
    f_SP = 0;
    f_MBE = 0;
    f_MBS = 0;
    f_RBE = 0;
    f_RBS = 0;
    f_SBS = 0;
    f_TMOD2H = 255;
    f_WDTM = 0;
    f_BT = 0;
    f_BTM = 0;
    f_IST = 0;
    f_SK = 0;
    f_SCC = 0;
    f_WMl = 0;
    f_WMh = 0;
    f_TM0l = 0;
    f_TM0h = 0;
    f_T0 = 0;
    f_TMOD0 = 255;
    f_IME = 0;
    f_IPS = 0;
    f_PCC_MODE = 0;
    f_PCC_CLOCK = 0;
    f_IM0 = 0;
    f_IM1 = 0;
    f_IM2 = 0;
    f_INTA = 0;
    f_INTC = 0;
    f_INTE = 0;
    f_INTF = 0;
    f_INTG = 0;
    f_INTH = 0;
    f_PORT0 = {0, 0};
    f_PORT1 = {0, 0};
    f_PORT2 = {0, 0};
    f_PORT3 = {0, 0};
    f_PORT5 = {0, 0};
    f_PORT6 = {0, 0};
    f_PORT8 = {0, 0};
    f_PORT9 = {0, 0};
    f_PORT2_OUT_LATCH = 0;
    f_PORT3_OUT_LATCH = 0;
    f_PORT5_OUT_LATCH = 0;
    f_PORT6_OUT_LATCH = 0;
    f_PORT8_OUT_LATCH = 0;
    f_PORT9_OUT_LATCH = 0;
    f_POGA = 0;
    f_POGB = 0;
    f_PM3 = 0;
    f_PM6 = 0;
    f_RAM.assign((size_t)(4096), 0);
}

void KS56CX2X::py_reset() {
    py_init_registers();
    py_go_vector(0);
}

void KS56CX2X::pyreset() {
    py_reset();
}

int64_t KS56CX2X::py_get_io_dummy() {
    return 0;
    return 0;
}

int64_t KS56CX2X::py_set_io_dummy(int64_t v_value) {
    return 0;
}

int64_t KS56CX2X::py_get_io_spl() {
    return (f_SP & 15);
    return 0;
}

int64_t KS56CX2X::py_set_io_spl(int64_t v_value) {
    f_SP = ((f_SP & 240) | (v_value & 15));
    return 0;
}

int64_t KS56CX2X::py_get_io_sph() {
    return (f_SP >> 4);
    return 0;
}

int64_t KS56CX2X::py_set_io_sph(int64_t v_value) {
    f_SP = ((f_SP & 15) | ((v_value & 15) << 4));
    return 0;
}

int64_t KS56CX2X::py_get_io_rbs() {
    return f_RBS;
    return 0;
}

int64_t KS56CX2X::py_get_io_mbs() {
    return f_MBS;
    return 0;
}

int64_t KS56CX2X::py_get_io_sbs() {
    return f_SBS;
    return 0;
}

int64_t KS56CX2X::py_set_io_sbs(int64_t v_value) {
    f_SBS = (v_value & 15);
    return 0;
}

int64_t KS56CX2X::py_set_io_btm(int64_t v_value) {
    f_BTM = (v_value & 7);
    if ((v_value & 8)) {
        f_INTA &= (~(1));
        f_BT = 0;
    }
    return 0;
}

int64_t KS56CX2X::py_get_io_btl() {
    return (f_BT & 15);
    return 0;
}

int64_t KS56CX2X::py_get_io_bth() {
    return (f_BT >> 4);
    return 0;
}

int64_t KS56CX2X::py_get_io_tmod2hl() {
    return (f_TMOD2H & 15);
    return 0;
}

int64_t KS56CX2X::py_set_io_tmod2hl(int64_t v_value) {
    f_TMOD2H = ((f_TMOD2H & 240) | (v_value & 15));
    return 0;
}

int64_t KS56CX2X::py_get_io_tmod2hh() {
    return (f_TMOD2H >> 4);
    return 0;
}

int64_t KS56CX2X::py_set_io_tmod2hh(int64_t v_value) {
    f_TMOD2H = ((f_TMOD2H & 15) | ((v_value & 15) << 4));
    return 0;
}

int64_t KS56CX2X::py_get_io_wml() {
    return f_WMl;
    return 0;
}

int64_t KS56CX2X::py_set_io_wml(int64_t v_value) {
    f_WMl = (v_value & 15);
    f_watch_timer_enable = (v_value & 4);
    return 0;
}

int64_t KS56CX2X::py_get_io_wmh() {
    return f_WMh;
    return 0;
}

int64_t KS56CX2X::py_set_io_wmh(int64_t v_value) {
    f_WMh = (v_value & 11);
    return 0;
}

int64_t KS56CX2X::py_get_io_tm0l() {
    return f_TM0l;
    return 0;
}

int64_t KS56CX2X::py_set_io_tm0l(int64_t v_value) {
    f_TM0l = (v_value & 12);
    if ((v_value & 8)) {
        f_INTE &= (~(1));
        f_T0 = 0;
    }
    return 0;
}

int64_t KS56CX2X::py_get_io_tm0h() {
    return f_TM0h;
    return 0;
}

int64_t KS56CX2X::py_set_io_tm0h(int64_t v_value) {
    f_TM0h = (v_value & 7);
    return 0;
}

int64_t KS56CX2X::py_get_io_t0l() {
    return (f_T0 & 15);
    return 0;
}

int64_t KS56CX2X::py_get_io_t0h() {
    return (f_T0 >> 4);
    return 0;
}

int64_t KS56CX2X::py_get_io_tmod0l() {
    return (f_TMOD0 & 15);
    return 0;
}

int64_t KS56CX2X::py_set_io_tmod0l(int64_t v_value) {
    f_TMOD0 = ((f_TMOD0 & 240) | (v_value & 15));
    return 0;
}

int64_t KS56CX2X::py_get_io_tmod0h() {
    return (f_TMOD0 >> 4);
    return 0;
}

int64_t KS56CX2X::py_set_io_tmod0h(int64_t v_value) {
    f_TMOD0 = ((f_TMOD0 & 15) | ((v_value << 4) & 240));
    return 0;
}

int64_t KS56CX2X::py_set_io_wdtm(int64_t v_value) {
    f_WDTM = ((v_value >> 3) & 1);
    return 0;
}

int64_t KS56CX2X::py_get_io_pswl() {
    return (((f_IST << 2) | (f_MBE << 1)) | f_RBE);
    return 0;
}

int64_t KS56CX2X::py_set_io_pswl(int64_t v_value) {
    f_IST = ((v_value >> 2) & 3);
    f_MBE = ((v_value >> 1) & 1);
    f_RBE = (v_value & 1);
    return 0;
}

int64_t KS56CX2X::py_get_io_pswh() {
    return ((f_CY << 3) | f_SK);
    return 0;
}

int64_t KS56CX2X::py_set_io_pswh(int64_t v_value) {
    f_CY = ((v_value >> 3) & 1);
    f_SK = (v_value & 7);
    return 0;
}

int64_t KS56CX2X::py_get_io_ips() {
    return ((f_IME << 3) | f_IPS);
    return 0;
}

int64_t KS56CX2X::py_set_io_ips(int64_t v_value) {
    f_IME = ((v_value >> 3) & 1);
    f_IPS = (v_value & 7);
    return 0;
}

int64_t KS56CX2X::py_get_io_pcc() {
    return ((f_PCC_MODE << 2) | f_PCC_CLOCK);
    return 0;
}

int64_t KS56CX2X::py_set_io_pcc(int64_t v_value) {
    f_PCC_MODE = ((v_value >> 2) & 3);
    f_PCC_CLOCK = (v_value & 3);
    pyupdate_cpu_clock_div();
    return 0;
}

int64_t KS56CX2X::py_get_io_im0() {
    return f_IM0;
    return 0;
}

int64_t KS56CX2X::py_set_io_im0(int64_t v_value) {
    f_IM0 = (v_value & 15);
    return 0;
}

int64_t KS56CX2X::py_get_io_im1() {
    return f_IM1;
    return 0;
}

int64_t KS56CX2X::py_set_io_im1(int64_t v_value) {
    f_IM1 = (v_value & 1);
    return 0;
}

int64_t KS56CX2X::py_get_io_im2() {
    return f_IM2;
    return 0;
}

int64_t KS56CX2X::py_set_io_im2(int64_t v_value) {
    f_IM2 = (v_value & 3);
    return 0;
}

int64_t KS56CX2X::py_get_io_scc() {
    return f_SCC;
    return 0;
}

int64_t KS56CX2X::py_set_io_scc(int64_t v_value) {
    f_SCC = (v_value & 9);
    pyupdate_cpu_clock_div();
    return 0;
}

int64_t KS56CX2X::py_get_io_inta() {
    return f_INTA;
    return 0;
}

int64_t KS56CX2X::py_set_io_inta(int64_t v_value) {
    f_INTA = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_intc() {
    return f_INTC;
    return 0;
}

int64_t KS56CX2X::py_set_io_intc(int64_t v_value) {
    f_INTC = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_inte() {
    return f_INTE;
    return 0;
}

int64_t KS56CX2X::py_set_io_inte(int64_t v_value) {
    f_INTE = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_intf() {
    return f_INTF;
    return 0;
}

int64_t KS56CX2X::py_set_io_intf(int64_t v_value) {
    f_INTF = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_intg() {
    return f_INTG;
    return 0;
}

int64_t KS56CX2X::py_set_io_intg(int64_t v_value) {
    f_INTG = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_inth() {
    return f_INTH;
    return 0;
}

int64_t KS56CX2X::py_set_io_inth(int64_t v_value) {
    f_INTH = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_pogal() {
    return (f_POGA & 15);
    return 0;
}

int64_t KS56CX2X::py_set_io_pogal(int64_t v_value) {
    f_POGA = ((f_POGA & 240) | (v_value & 15));
    return 0;
}

int64_t KS56CX2X::py_get_io_pogah() {
    return ((f_POGA >> 4) & 15);
    return 0;
}

int64_t KS56CX2X::py_set_io_pogah(int64_t v_value) {
    f_POGA = ((f_POGA & 15) | ((v_value & 15) << 4));
    return 0;
}

int64_t KS56CX2X::py_get_io_pmgal() {
    return f_PM3;
    return 0;
}

int64_t KS56CX2X::py_set_io_pmgal(int64_t v_value) {
    f_PM3 = (v_value & 15);
    return 0;
}

int64_t KS56CX2X::py_get_io_pmgah() {
    return f_PM6;
    return 0;
}

int64_t KS56CX2X::py_set_io_pmgah(int64_t v_value) {
    f_PM6 = (v_value & 15);
    return 0;
}

int64_t KS56CX2X::py_get_io_port0() {
    return ((~(f_PORT0[0])) & (f_PORT0[1] | ((((f_POGA & 1) > 0)) * 15)));
    return 0;
}

int64_t KS56CX2X::py_get_io_port1() {
    return ((~(f_PORT1[0])) & (f_PORT1[1] | ((((f_POGA & 2) > 0)) * 15)));
    return 0;
}

int64_t KS56CX2X::py_get_io_port2() {
    return ((~(f_PORT2[0])) & (f_PORT2[1] | ((((f_POGA & 4) > 0)) * 15)));
    return 0;
}

int64_t KS56CX2X::py_set_io_port2(int64_t v_value) {
    f_PORT2_OUT_LATCH = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_port3() {
    int64_t v_ext = 0;
    v_ext = ((~(f_PORT3[0])) & (f_PORT3[1] | ((((f_POGA & 8) > 0)) * 15)));
    return ((v_ext & (~(f_PM3))) | (f_PORT3_OUT_LATCH & f_PM3));
    return 0;
}

int64_t KS56CX2X::py_set_io_port3(int64_t v_value) {
    f_PORT3_OUT_LATCH = v_value;
    snd_toggle((v_value & 8), 0);
    return 0;
}

int64_t KS56CX2X::py_get_io_port5() {
    return ((~(f_PORT5[0])) & f_PORT5[1]);
    return 0;
}

int64_t KS56CX2X::py_set_io_port5(int64_t v_value) {
    f_PORT5_OUT_LATCH = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_port6() {
    int64_t v_ext = 0;
    v_ext = ((~(f_PORT6[0])) & (f_PORT6[1] | ((((f_POGA & 50) > 0)) * 15)));
    return ((v_ext & (~(f_PM6))) | (f_PORT6_OUT_LATCH & f_PM6));
    return 0;
}

int64_t KS56CX2X::py_set_io_port6(int64_t v_value) {
    f_PORT6_OUT_LATCH = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_port8() {
    return ((~(f_PORT8[0])) & (f_PORT8[1] | ((((f_POGB & 1) > 0)) * 15)));
    return 0;
}

int64_t KS56CX2X::py_set_io_port8(int64_t v_value) {
    f_PORT8_OUT_LATCH = v_value;
    return 0;
}

int64_t KS56CX2X::py_get_io_port9() {
    return ((~(f_PORT9[0])) & (f_PORT9[1] | ((((f_POGB & 2) > 0)) * 15)));
    return 0;
}

int64_t KS56CX2X::py_set_io_port9(int64_t v_value) {
    f_PORT9_OUT_LATCH = v_value;
    return 0;
}

void KS56CX2X::pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level) {
    int64_t v_prev_port = 0;
    if (((v_port == kPort_PORT1))) {
        v_prev_port = py_get_io_port1();
        f_PORT1[0] &= (~(v_mask));
        f_PORT1[1] &= (~(v_mask));
        if (((v_level >= 0))) {
            f_PORT1[v_level] |= v_mask;
        }
        if (((v_prev_port != py_get_io_port1()))) {
            if ((((v_mask & 2)) && (((f_IM1 != ((py_get_io_port1() >> 1) & 1)))))) {
                f_INTG |= 4;
            }
            if ((((v_mask & 1)) && (((f_IM0 != 3))) && ((((((f_IM0 & 1) != v_level))) || ((f_IM0 & 2)))))) {
                f_INTG |= 1;
            }
        }
    } else {
        if (((v_port == kPort_PORT6))) {
            v_prev_port = py_get_io_port6();
            f_PORT6[0] &= (~(v_mask));
            f_PORT6[1] &= (~(v_mask));
            if (((v_level >= 0))) {
                f_PORT6[v_level] |= v_mask;
            }
            if (((((((v_level == 0))) && (((v_prev_port != py_get_io_port6()))))) && ((((f_PM6 & v_mask) == 0))) && (((((f_IM2 == 3))) || (((((f_IM2 == 2))) && ((v_mask & 12)))))))) {
                f_INTH |= 1;
            }
        }
    }
}

void KS56CX2X::pyupdate_cpu_clock_div() {
    if ((f_SCC & 1)) {
        f_cpu_clock_div = f_sub_clock_div;
    } else {
        f_cpu_clock_div = k_MAIN_CLOCK_DIV[f_PCC_CLOCK];
    }
}

void KS56CX2X::py_process_basic_timer(int64_t v_exec_cycles) {
    f_basic_timer_counter -= v_exec_cycles;
    while (((f_basic_timer_counter <= 0))) {
        f_basic_timer_counter += k_BASIC_TIMER_DIV[f_BTM];
        f_BT = ((f_BT + 1) & 255);
        if (((((f_BT == 0))) && (((f_WDTM == 0))))) {
            f_INTA |= 1;
        }
    }
}

void KS56CX2X::py_process_t0_timer(int64_t v_exec_cycles) {
    if (((((k_TIMER_T0_DIV[f_TM0h] > 0))) && ((f_TM0l & 4)))) {
        f_T0_timer_counter -= v_exec_cycles;
        while (((f_T0_timer_counter <= 0))) {
            f_T0_timer_counter += k_TIMER_T0_DIV[f_TM0h];
            f_T0 = ((f_T0 + 1) & 255);
            if (((f_T0 == f_TMOD0))) {
                f_INTE |= 1;
                f_T0 = 0;
            }
        }
    }
}

void KS56CX2X::py_process_watch_timer(int64_t v_exec_cycles) {
    if ((f_WMl & 4)) {
        f_watch_timer_counter -= v_exec_cycles;
        while (((f_watch_timer_counter <= 0))) {
            if ((f_WMl & 1)) {
                f_watch_timer_counter += (k_WATCH_TIMER_DIV[(f_WMl & 2)] * f_sub_clock_div);
            } else {
                f_watch_timer_counter += (k_WATCH_TIMER_DIV[(f_WMl & 2)] * 128);
            }
            f_INTC |= 1;
        }
    } else {
        f_watch_timer_counter = 0;
    }
}

void KS56CX2X::py_go_vector(int64_t v_addr) {
    int64_t v_vector = 0;
    v_vector = rom_get_word(v_addr);
    f_MBE = ((v_vector >> 15) & 1);
    f_RBE = ((v_vector >> 14) & 1);
    f_PC = (v_vector & rom_get_mask());
}

void KS56CX2X::py_interrupt(int64_t v_IRQn) {
    int64_t v_vector_addr = 0;
    v_vector_addr = (v_IRQn << 1);
    py_stack_push(((f_CY << 3) | f_SK));
    py_stack_push((((f_IST << 2) | (f_MBE << 1)) | f_RBE));
    py_stack_push(((f_PC >> 4) & 15));
    py_stack_push((f_PC & 15));
    py_stack_push((((f_MBE << 3) | (f_RBE << 2)) | ((f_PC >> 12) & 1)));
    py_stack_push(((f_PC >> 8) & 15));
    py_go_vector(v_vector_addr);
    f_IST += 1;
    if (((v_IRQn == 1))) {
        if ((((f_INTA & 2) == 0))) {
            f_INTA &= (~(4));
        } else {
            if ((((f_INTA & 8) == 0))) {
                f_INTA &= (~(1));
            }
        }
    } else {
        if (((v_IRQn == 5))) {
            f_INTE &= (~(1));
        } else {
            if (((v_IRQn == 2))) {
                f_INTG &= (~(1));
            } else {
                if (((v_IRQn == 3))) {
                    f_INTG &= (~(4));
                }
            }
        }
    }
}

int64_t KS56CX2X::py_get_interrupt_vector() {
    int64_t v_IRQn = 0;
    v_IRQn = 0;
    if ((((f_INTE & 2)) && ((f_INTE & 1)))) {
        if (((((5 == f_IPS))) && (((f_IST <= 1))))) {
            return 5;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 5;
            }
        }
    }
    if ((((f_INTG & 8)) && ((f_INTG & 4)))) {
        if (((((3 == f_IPS))) && (((f_IST <= 1))))) {
            return 3;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 3;
            }
        }
    }
    if ((((f_INTG & 2)) && ((f_INTG & 1)))) {
        if (((((2 == f_IPS))) && (((f_IST <= 1))))) {
            return 2;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 2;
            }
        }
    }
    if ((((f_INTA & 8)) && ((f_INTA & 4)))) {
        if (((((1 == f_IPS))) && (((f_IST <= 1))))) {
            return 1;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 1;
            }
        }
    }
    if ((((f_INTA & 2)) && ((f_INTA & 1)))) {
        if (((((1 == f_IPS))) && (((f_IST <= 1))))) {
            return 1;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 1;
            }
        }
    }
    return v_IRQn;
    return 0;
}

double KS56CX2X::pyclock() {
    int64_t v_IRQn = 0, v_byte = 0, v_bytes_count = 0, v_opcode = 0;
    double v_exec_cycles = 0;
    v_exec_cycles = f_cpu_clock_div;
    if (((f_PCC_MODE == 0))) {
        v_byte = rom_get_byte(f_PC);
        v_bytes_count = T_execute[v_byte].size;
        v_opcode = rom_get_bytes(f_PC, v_bytes_count);
        f_PC += v_bytes_count;
        v_exec_cycles *= (this->*T_execute[v_byte].fn)(v_opcode);
        f_instr_counter += 1;
    } else {
        if (((((((((f_INTC & 1)) && ((f_INTC & 2))) | (((f_INTH & 1)) && ((f_INTH & 2)))) | (((f_INTG & 1)) && ((f_INTG & 2)) && ((f_IM0 & 4)))) | (((f_INTG & 4)) && ((f_INTG & 8)))) | (((f_INTE & 2)) && ((f_INTE & 1)))) | (((f_INTA & 2)) && ((f_INTA & 1))))) {
            f_PCC_MODE = 0;
        }
    }
    if (((f_PCC_MODE != 2))) {
        py_process_watch_timer(v_exec_cycles);
        if ((!((f_SCC & 8)))) {
            py_process_basic_timer(v_exec_cycles);
            py_process_t0_timer(v_exec_cycles);
        }
    }
    if (f_IME) {
        v_IRQn = py_get_interrupt_vector();
        if (((v_IRQn > 0))) {
            py_interrupt(v_IRQn);
        }
    }
    return v_exec_cycles;
    return 0;
}

int64_t KS56CX2X::py_get_mem(int64_t v_addr) {
    int64_t v_mb = 0;
    v_mb = 15;
    if (((((f_MBE != 0))) || (((v_addr < 128))))) {
        v_mb = (f_MBE * f_MBS);
    }
    if (((v_mb != 15))) {
        return f_RAM[((v_mb << 8) + v_addr)];
    } else {
        const IoEntry* v_io = find_io_tbl(((v_mb << 8) + v_addr));
        if (((v_io != nullptr))) {
            return (this->*v_io->get)();
        }
        return 0;
    }
    return 0;
}

void KS56CX2X::py_set_mem(int64_t v_addr, int64_t v_value) {
    int64_t v_mb = 0;
    v_mb = 15;
    if (((((f_MBE != 0))) || (((v_addr < 128))))) {
        v_mb = (f_MBE * f_MBS);
    }
    if (((v_mb != 15))) {
        f_RAM[((v_mb << 8) + v_addr)] = v_value;
    } else {
        const IoEntry* v_io = find_io_tbl(((v_mb << 8) + v_addr));
        if (((v_io != nullptr))) {
            (this->*v_io->set)(v_value);
        }
    }
}

int64_t KS56CX2X::py_get_ahl() {
    int64_t v_mb = 0;
    v_mb = (f_MBE * f_MBS);
    if (((v_mb != 15))) {
        return f_RAM[((v_mb << 8) + py_get_rp(2))];
    } else {
        const IoEntry* v_io = find_io_tbl(((v_mb << 8) + py_get_rp(2)));
        if (((v_io != nullptr))) {
            return (this->*v_io->get)();
        }
        return 0;
    }
    return 0;
}

int64_t KS56CX2X::py_get_ahl_byte() {
    int64_t v_addr = 0, v_mb = 0, v_result = 0;
    v_mb = (f_MBE * f_MBS);
    v_addr = ((v_mb << 8) + py_get_rp(2));
    if (((v_mb != 15))) {
        return ((f_RAM[(v_addr + 1)] << 4) | f_RAM[v_addr]);
    } else {
        v_result = 0;
        const IoEntry* v_io = find_io_tbl((v_addr + 1));
        if (((v_io != nullptr))) {
            v_result = ((this->*v_io->get)() << 4);
        }
        v_io = find_io_tbl(v_addr);
        if (((v_io != nullptr))) {
            v_result |= (this->*v_io->get)();
        }
        return v_result;
    }
    return 0;
}

void KS56CX2X::py_set_ahl(int64_t v_value) {
    int64_t v_mb = 0;
    v_mb = (f_MBE * f_MBS);
    if (((v_mb != 15))) {
        f_RAM[((v_mb << 8) + py_get_rp(2))] = v_value;
    } else {
        const IoEntry* v_io = find_io_tbl(((v_mb << 8) + py_get_rp(2)));
        if (((v_io != nullptr))) {
            (this->*v_io->set)(v_value);
        }
    }
}

void KS56CX2X::py_set_ahl_byte(int64_t v_value) {
    int64_t v_addr = 0, v_mb = 0;
    v_mb = (f_MBE * f_MBS);
    v_addr = ((v_mb << 8) + py_get_rp(2));
    if (((v_mb != 15))) {
        f_RAM[v_addr] = (v_value & 15);
        f_RAM[(v_addr + 1)] = (v_value >> 4);
    } else {
        const IoEntry* v_io = find_io_tbl(v_addr);
        if (((v_io != nullptr))) {
            (this->*v_io->set)((v_value & 15));
        }
        v_io = find_io_tbl((v_addr + 1));
        if (((v_io != nullptr))) {
            (this->*v_io->set)((v_value >> 4));
        }
    }
}

int64_t KS56CX2X::py_get_hmem(int64_t v_opcode) {
    int64_t v_addr = 0, v_mb = 0;
    v_addr = ((py_get_reg(3) << 4) | (v_opcode & 15));
    v_mb = (f_MBE * f_MBS);
    if (((v_mb != 15))) {
        return f_RAM[((v_mb << 8) + v_addr)];
    } else {
        const IoEntry* v_io = find_io_tbl(((v_mb << 8) + v_addr));
        if (((v_io != nullptr))) {
            return (this->*v_io->get)();
        }
        return 0;
    }
    return 0;
}

void KS56CX2X::py_set_hmem(int64_t v_opcode, int64_t v_value) {
    int64_t v_addr = 0, v_mb = 0;
    v_addr = ((py_get_reg(3) << 4) | (v_opcode & 15));
    v_mb = (f_MBE * f_MBS);
    if (((v_mb != 15))) {
        f_RAM[((v_mb << 8) + v_addr)] = v_value;
    } else {
        const IoEntry* v_io = find_io_tbl(((v_mb << 8) + v_addr));
        if (((v_io != nullptr))) {
            (this->*v_io->set)(v_value);
        }
    }
}

int64_t KS56CX2X::py_get_pmeml(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((4032 | ((v_opcode & 15) << 2)) | (py_get_reg(2) >> 2));
    const IoEntry* v_io = find_io_tbl(v_addr);
    if (((v_io != nullptr))) {
        return (this->*v_io->get)();
    }
    return 0;
    return 0;
}

int64_t KS56CX2X::py_set_pmeml(int64_t v_opcode, int64_t v_value) {
    int64_t v_addr = 0;
    v_addr = ((4032 | ((v_opcode & 15) << 2)) | (py_get_reg(2) >> 2));
    const IoEntry* v_io = find_io_tbl(v_addr);
    if (((v_io != nullptr))) {
        (this->*v_io->set)(v_value);
    }
    return 0;
}

int64_t KS56CX2X::py_get_fmem(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = (4016 | (v_opcode & 79));
    const IoEntry* v_io = find_io_tbl(v_addr);
    if (((v_io != nullptr))) {
        return (this->*v_io->get)();
    }
    return 0;
    return 0;
}

void KS56CX2X::py_set_fmem(int64_t v_opcode, int64_t v_value) {
    int64_t v_addr = 0;
    v_addr = (4016 | (v_opcode & 79));
    const IoEntry* v_io = find_io_tbl(v_addr);
    if (((v_io != nullptr))) {
        (this->*v_io->set)(v_value);
    }
}

int64_t KS56CX2X::py_get_reg(int64_t v_reg) {
    return f_RAM[(((f_RBE * f_RBS) * 8) + v_reg)];
    return 0;
}

void KS56CX2X::py_set_reg(int64_t v_reg, int64_t v_value) {
    f_RAM[(((f_RBE * f_RBS) * 8) + v_reg)] = v_value;
}

int64_t KS56CX2X::py_get_rp(int64_t v_rp) {
    int64_t v_rp_offset = 0;
    v_rp_offset = ((((f_RBE * f_RBS) * 8) + (v_rp & 6)) ^ ((v_rp & 1) << 3));
    return ((f_RAM[(v_rp_offset + 1)] << 4) | f_RAM[v_rp_offset]);
    return 0;
}

void KS56CX2X::py_set_rp(int64_t v_rp, int64_t v_value) {
    int64_t v_rp_offset = 0;
    v_rp_offset = ((((f_RBE * f_RBS) * 8) + (v_rp & 6)) ^ ((v_rp & 1) << 3));
    f_RAM[v_rp_offset] = (v_value & 15);
    f_RAM[(v_rp_offset + 1)] = ((v_value >> 4) & 15);
}

void KS56CX2X::py_stack_push(int64_t v_value) {
    int64_t v_sp = 0;
    f_SP = ((f_SP - 1) & 255);
    v_sp = (((f_SBS & 1) << 8) + f_SP);
    f_RAM[v_sp] = v_value;
}

void KS56CX2X::py_stack_push_byte(int64_t v_value) {
    int64_t v_sph = 0, v_spl = 0;
    f_SP = ((f_SP - 1) & 255);
    v_sph = (((f_SBS & 1) << 8) + f_SP);
    f_RAM[v_sph] = ((v_value >> 4) & 15);
    f_SP = ((f_SP - 1) & 255);
    v_spl = (((f_SBS & 1) << 8) + f_SP);
    f_RAM[v_spl] = (v_value & 15);
}

int64_t KS56CX2X::py_stack_pop() {
    int64_t v_sp = 0;
    v_sp = (((f_SBS & 1) << 8) + f_SP);
    f_SP = ((f_SP + 1) & 255);
    return f_RAM[v_sp];
    return 0;
}

int64_t KS56CX2X::py_stack_pop_byte() {
    int64_t v_sph = 0, v_spl = 0;
    v_spl = (((f_SBS & 1) << 8) + f_SP);
    v_sph = (((f_SBS & 1) << 8) + ((f_SP + 1) & 255));
    f_SP = ((f_SP + 2) & 255);
    return ((f_RAM[v_sph] << 4) | f_RAM[v_spl]);
    return 0;
}

int64_t KS56CX2X::py_skip_next() {
    int64_t v_byte_count = 0, v_opcode = 0;
    v_opcode = rom_get_byte(f_PC);
    v_byte_count = T_execute[v_opcode].size;
    f_PC += v_byte_count;
    return ((((v_byte_count < 3))) ? 1 : 2);
    return 0;
}

int64_t KS56CX2X::py_execute_10101011(int64_t v_opcode) {
    return (this->*T_execute_10101011[((v_opcode >> 14) & 1)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10011001(int64_t v_opcode) {
    return (this->*T_execute_10011001[(v_opcode & 127)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10011010(int64_t v_opcode) {
    return (this->*T_execute_10011010[((v_opcode >> 3) & 1)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10011011(int64_t v_opcode) {
    return (this->*T_execute_10011011[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10011100(int64_t v_opcode) {
    return (this->*T_execute_10011100[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10011101(int64_t v_opcode) {
    return (this->*T_execute_10011101[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10011111(int64_t v_opcode) {
    return (this->*T_execute_10011111[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10101010(int64_t v_opcode) {
    return (this->*T_execute_10101010[(v_opcode & 255)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10101100(int64_t v_opcode) {
    return (this->*T_execute_10101100[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10101110(int64_t v_opcode) {
    return (this->*T_execute_10101110[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10111100(int64_t v_opcode) {
    return (this->*T_execute_10111100[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10111101(int64_t v_opcode) {
    return (this->*T_execute_10111101[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10111110(int64_t v_opcode) {
    return (this->*T_execute_10111110[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_execute_10111111(int64_t v_opcode) {
    return (this->*T_execute_10111111[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS56CX2X::py_br_raddr(int64_t v_opcode) {
    f_PC += v_opcode;
    return 8;
    return 0;
}

int64_t KS56CX2X::py_br_mraddr(int64_t v_opcode) {
    f_PC -= (16 - (v_opcode & 15));
    return 8;
    return 0;
}

int64_t KS56CX2X::py_geti_taddr(int64_t v_opcode) {
    int64_t v_byte = 0, v_bytes_count = 0, v_execute_time = 0, v_taddr = 0;
    v_taddr = (v_opcode << 1);
    v_byte = rom_get_byte(v_taddr);
    v_bytes_count = T_execute[v_byte].size;
    v_execute_time = 1;
    if (((v_bytes_count == 1))) {
        if ((((v_byte & 192) == 0))) {
            f_PC = (rom_get_word(v_taddr) & rom_get_mask());
            return 3;
        }
        v_opcode = v_byte;
        v_execute_time += (this->*T_execute[v_byte].fn)(v_opcode);
        v_byte = rom_get_byte((v_taddr + 1));
        v_execute_time += (this->*T_execute[v_byte].fn)(v_opcode);
    } else {
        v_opcode = rom_get_bytes(v_taddr, v_bytes_count);
        v_execute_time += (this->*T_execute[v_byte].fn)(v_opcode);
    }
    return v_execute_time;
    return 0;
}

int64_t KS56CX2X::py_callf_faddr(int64_t v_opcode) {
    py_stack_push(((f_PC >> 4) & 15));
    py_stack_push((f_PC & 15));
    py_stack_push((((f_MBE << 3) | (f_RBE << 2)) | ((f_PC >> 12) & 3)));
    py_stack_push(((f_PC >> 8) & 15));
    f_PC = (v_opcode & rom_get_mask());
    return 2;
    return 0;
}

int64_t KS56CX2X::py_pop_rp(int64_t v_opcode) {
    py_set_rp((v_opcode & 6), py_stack_pop_byte());
    return 1;
    return 0;
}

int64_t KS56CX2X::py_push_rp(int64_t v_opcode) {
    py_stack_push_byte(py_get_rp((v_opcode & 6)));
    return 1;
    return 0;
}

int64_t KS56CX2X::py_brcb_caddr(int64_t v_opcode) {
    f_PC = (((f_PC & 61440) | (v_opcode & 4095)) & rom_get_mask());
    return 2;
    return 0;
}

int64_t KS56CX2X::py_adds_a_n4(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = (py_get_reg(0) + (v_opcode & 15));
    py_set_reg(0, (v_new_A & 15));
    if (((v_new_A > 15))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_a_n4(int64_t v_opcode) {
    int64_t v_cycle_count = 0;
    py_set_reg(0, (v_opcode & 15));
    v_cycle_count = 1;
    v_opcode = rom_get_byte(f_PC);
    while ((((((v_opcode >> 4) == 7))) || (((v_opcode == 137))))) {
        v_cycle_count += 1;
        f_PC += T_execute[v_opcode].size;
        v_opcode = rom_get_byte(f_PC);
    }
    return v_cycle_count;
    return 0;
}

int64_t KS56CX2X::py_ske_a_ahl(int64_t v_opcode) {
    if (((py_get_reg(0) == py_get_ahl()))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_incs_mem(int64_t v_opcode) {
    int64_t v_d = 0, v_new_mem = 0;
    v_d = (v_opcode & 255);
    v_new_mem = ((py_get_mem(v_d) + 1) & 15);
    py_set_mem(v_d, v_new_mem);
    if (((v_new_mem == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_clr1_mem_bit(int64_t v_opcode) {
    int64_t v_d = 0, v_new_mem = 0;
    v_d = (v_opcode & 255);
    v_new_mem = (py_get_mem(v_d) & (~((1 << ((v_opcode >> 12) & 3)))));
    py_set_mem(v_d, v_new_mem);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_set1_mem_bit(int64_t v_opcode) {
    int64_t v_d = 0, v_new_mem = 0;
    v_d = (v_opcode & 255);
    v_new_mem = (py_get_mem(v_d) | (1 << ((v_opcode >> 12) & 3)));
    py_set_mem(v_d, v_new_mem);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_skf_mem_bit(int64_t v_opcode) {
    if ((!((py_get_mem((v_opcode & 255)) & (1 << ((v_opcode >> 12) & 3)))))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_skt_mem_bit(int64_t v_opcode) {
    if ((py_get_mem((v_opcode & 255)) & (1 << ((v_opcode >> 12) & 3)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_xa_n8(int64_t v_opcode) {
    int64_t v_cycle_count = 0;
    py_set_rp(0, (v_opcode & 255));
    v_cycle_count = 2;
    v_opcode = rom_get_byte(f_PC);
    while ((((((v_opcode >> 4) == 7))) || (((v_opcode == 137))))) {
        v_cycle_count += 1;
        f_PC += T_execute[v_opcode].size;
        v_opcode = rom_get_byte(f_PC);
    }
    return v_cycle_count;
    return 0;
}

int64_t KS56CX2X::py_incs_rp1(int64_t v_opcode) {
    int64_t v_new_rp = 0;
    v_new_rp = ((py_get_rp((v_opcode & 6)) + 1) & 255);
    py_set_rp((v_opcode & 6), v_new_rp);
    if (((v_new_rp == 0))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_hl_n8(int64_t v_opcode) {
    int64_t v_cycle_count = 0;
    py_set_rp(2, (v_opcode & 255));
    v_cycle_count = 2;
    v_opcode = rom_get_byte(f_PC);
    while (((v_opcode == 139))) {
        v_cycle_count += 1;
        f_PC += T_execute[v_opcode].size;
        v_opcode = rom_get_byte(f_PC);
    }
    return v_cycle_count;
    return 0;
}

int64_t KS56CX2X::py_mov_de_n8(int64_t v_opcode) {
    py_set_rp(4, (v_opcode & 255));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_bc_n8(int64_t v_opcode) {
    py_set_rp(6, (v_opcode & 255));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_and_a_ahl(int64_t v_opcode) {
    py_set_reg(0, (py_get_reg(0) & py_get_ahl()));
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_mem_xa(int64_t v_opcode) {
    int64_t v_d = 0;
    v_d = (v_opcode & 255);
    py_set_mem(v_d, py_get_reg(0));
    py_set_mem((v_d + 1), py_get_reg(1));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_mem_A(int64_t v_opcode) {
    int64_t v_d = 0;
    v_d = (v_opcode & 255);
    py_set_mem(v_d, py_get_reg(0));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_rorc_a(int64_t v_opcode) {
    int64_t v_new_CY = 0;
    v_new_CY = (py_get_reg(0) & 1);
    py_set_reg(0, ((py_get_reg(0) >> 1) | (f_CY << 3)));
    f_CY = v_new_CY;
    return 1;
    return 0;
}

int64_t KS56CX2X::py_or_a_ahl(int64_t v_opcode) {
    py_set_reg(0, (py_get_reg(0) | py_get_ahl()));
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_xa_mem(int64_t v_opcode) {
    int64_t v_d = 0;
    v_d = (v_opcode & 254);
    py_set_reg(0, py_get_mem(v_d));
    py_set_reg(1, py_get_mem((v_d + 1)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_a_mem(int64_t v_opcode) {
    int64_t v_d = 0;
    v_d = (v_opcode & 255);
    py_set_reg(0, py_get_mem(v_d));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_subs_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = (py_get_reg(0) - py_get_ahl());
    py_set_reg(0, (v_new_A & 15));
    if (((v_new_A < 0))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_addc_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = ((py_get_reg(0) + py_get_ahl()) + f_CY);
    py_set_reg(0, (v_new_A & 15));
    f_CY = ((v_new_A > 15));
    v_opcode = rom_get_byte(f_PC);
    if ((((v_opcode >> 4) == 6))) {
        if (f_CY) {
            return (1 + py_skip_next());
        }
        f_PC += 1;
        py_set_reg(0, ((py_get_reg(0) + (v_opcode & 15)) & 15));
        return 2;
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_br_addr(int64_t v_opcode) {
    f_PC = (v_opcode & rom_get_mask());
    return 1;
    return 0;
}

int64_t KS56CX2X::py_call_addr(int64_t v_opcode) {
    py_stack_push(((f_PC >> 4) & 15));
    py_stack_push((f_PC & 15));
    py_stack_push((((f_MBE << 3) | (f_RBE << 2)) | ((f_PC >> 12) & 3)));
    py_stack_push(((f_PC >> 8) & 15));
    f_PC = (v_opcode & rom_get_mask());
    return 3;
    return 0;
}

int64_t KS56CX2X::py_xor_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = (py_get_reg(0) ^ py_get_ahl());
    py_set_reg(0, v_new_A);
    return 1;
    return 0;
}

int64_t KS56CX2X::py_xch_xa_mem(int64_t v_opcode) {
    int64_t v_buf_A = 0, v_buf_X = 0, v_d = 0;
    v_d = (v_opcode & 254);
    v_buf_A = py_get_reg(0);
    v_buf_X = py_get_reg(1);
    py_set_reg(0, py_get_mem(v_d));
    py_set_reg(1, py_get_mem((v_d + 1)));
    py_set_mem(v_d, v_buf_A);
    py_set_mem((v_d + 1), v_buf_X);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xch_a_mem(int64_t v_opcode) {
    int64_t v_buf_A = 0, v_d = 0;
    v_d = (v_opcode & 255);
    v_buf_A = py_get_reg(0);
    py_set_reg(0, py_get_mem(v_d));
    py_set_mem(v_d, v_buf_A);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_subc_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = ((py_get_reg(0) - py_get_ahl()) - f_CY);
    py_set_reg(0, (v_new_A & 15));
    f_CY = ((v_new_A < 0));
    v_opcode = rom_get_byte(f_PC);
    if ((((v_opcode >> 4) == 6))) {
        if (f_CY) {
            f_PC += 1;
            py_set_reg(0, ((py_get_reg(0) + (v_opcode & 15)) & 15));
            return 2;
        }
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_adds_xa_n8(int64_t v_opcode) {
    int64_t v_new_XA = 0;
    v_new_XA = (py_get_rp(0) + (v_opcode & 255));
    py_set_rp(0, (v_new_XA & 255));
    if (((v_new_XA > 255))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_incs_reg(int64_t v_opcode) {
    int64_t v_new_value = 0, v_reg = 0;
    v_reg = (v_opcode & 7);
    v_new_value = ((py_get_reg(v_reg) + 1) & 15);
    py_set_reg(v_reg, v_new_value);
    if (((v_new_value == 0))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_decs_reg(int64_t v_opcode) {
    int64_t v_new_value = 0, v_reg = 0;
    v_reg = (v_opcode & 7);
    v_new_value = ((py_get_reg(v_reg) - 1) & 15);
    py_set_reg(v_reg, v_new_value);
    if (((v_new_value == 15))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_movt_xa_pcxa(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((f_PC & 65280) | py_get_rp(0));
    py_set_rp(0, rom_get_byte(v_addr));
    return 3;
    return 0;
}

int64_t KS56CX2X::py_movt_xa_bcxa(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((py_get_rp(6) << 8) | py_get_rp(0));
    py_set_rp(0, rom_get_byte(v_addr));
    return 3;
    return 0;
}

int64_t KS56CX2X::py_adds_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = (py_get_reg(0) + py_get_ahl());
    py_set_reg(0, (v_new_A & 15));
    if (((v_new_A > 15))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_movt_xa_pcde(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((f_PC & 65280) | py_get_rp(4));
    py_set_rp(0, rom_get_byte(v_addr));
    return 3;
    return 0;
}

int64_t KS56CX2X::py_movt_xa_bcde(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((py_get_rp(6) << 8) | py_get_rp(4));
    py_set_rp(0, rom_get_byte(v_addr));
    return 3;
    return 0;
}

int64_t KS56CX2X::py_not1_cy(int64_t v_opcode) {
    f_CY = (!(f_CY));
    return 1;
    return 0;
}

int64_t KS56CX2X::py_skt_cy(int64_t v_opcode) {
    if (f_CY) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_xch_a_reg1(int64_t v_opcode) {
    int64_t v_buf_A = 0, v_reg = 0;
    v_reg = (v_opcode & 7);
    v_buf_A = py_get_reg(0);
    py_set_reg(0, py_get_reg(v_reg));
    py_set_reg(v_reg, v_buf_A);
    return 1;
    return 0;
}

int64_t KS56CX2X::py_rets(int64_t v_opcode) {
    int64_t v_stack_value = 0;
    f_PC = (py_stack_pop() << 8);
    v_stack_value = py_stack_pop();
    f_PC |= ((v_stack_value << 12) & 4096);
    f_MBE = (v_stack_value >> 3);
    f_RBE = ((v_stack_value >> 2) & 1);
    f_PC |= py_stack_pop();
    f_PC |= (py_stack_pop() << 4);
    f_PC &= rom_get_mask();
    return (3 + py_skip_next());
    return 0;
}

int64_t KS56CX2X::py_mov_a_ahl(int64_t v_opcode) {
    py_set_reg(0, py_get_ahl());
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_a_ahli(int64_t v_opcode) {
    int64_t v_new_L = 0;
    py_set_reg(0, py_get_ahl());
    v_new_L = ((py_get_reg(2) + 1) & 15);
    py_set_reg(2, v_new_L);
    if (((v_new_L == 0))) {
        return (2 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_a_ahld(int64_t v_opcode) {
    int64_t v_new_L = 0;
    py_set_reg(0, py_get_ahl());
    v_new_L = ((py_get_reg(2) - 1) & 15);
    py_set_reg(2, v_new_L);
    if (((v_new_L == 15))) {
        return (2 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_a_ade(int64_t v_opcode) {
    py_set_reg(0, f_RAM[py_get_rp(4)]);
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_a_adl(int64_t v_opcode) {
    int64_t v_dl = 0;
    v_dl = ((py_get_reg(5) << 4) | py_get_reg(2));
    py_set_reg(0, f_RAM[v_dl]);
    return 1;
    return 0;
}

int64_t KS56CX2X::py_clr1_cy(int64_t v_opcode) {
    f_CY = 0;
    return 1;
    return 0;
}

int64_t KS56CX2X::py_set1_cy(int64_t v_opcode) {
    f_CY = 1;
    return 1;
    return 0;
}

int64_t KS56CX2X::py_mov_ahl_a(int64_t v_opcode) {
    py_set_ahl(py_get_reg(0));
    return 1;
    return 0;
}

int64_t KS56CX2X::py_xch_a_ahl(int64_t v_opcode) {
    int64_t v_buf_A = 0;
    v_buf_A = py_get_reg(0);
    py_set_reg(0, py_get_ahl());
    py_set_ahl(v_buf_A);
    return 1;
    return 0;
}

int64_t KS56CX2X::py_xch_a_ahli(int64_t v_opcode) {
    int64_t v_buf_A = 0, v_new_L = 0;
    v_buf_A = py_get_reg(0);
    py_set_reg(0, py_get_ahl());
    py_set_ahl(v_buf_A);
    v_new_L = ((py_get_reg(2) + 1) & 15);
    py_set_reg(2, v_new_L);
    if (((v_new_L == 0))) {
        return (2 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_xch_a_ahld(int64_t v_opcode) {
    int64_t v_buf_A = 0, v_new_L = 0;
    v_buf_A = py_get_reg(0);
    py_set_reg(0, py_get_ahl());
    py_set_ahl(v_buf_A);
    v_new_L = ((py_get_reg(2) - 1) & 15);
    py_set_reg(2, v_new_L);
    if (((v_new_L == 15))) {
        return (2 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS56CX2X::py_xch_a_ade(int64_t v_opcode) {
    int64_t v_buf_A = 0;
    v_buf_A = py_get_reg(0);
    py_set_reg(0, f_RAM[py_get_rp(4)]);
    f_RAM[py_get_rp(4)] = v_buf_A;
    return 1;
    return 0;
}

int64_t KS56CX2X::py_xch_a_adl(int64_t v_opcode) {
    int64_t v_buf_A = 0, v_dl = 0;
    v_buf_A = py_get_reg(0);
    v_dl = ((py_get_reg(5) << 4) | py_get_reg(2));
    py_set_reg(0, f_RAM[v_dl]);
    f_RAM[v_dl] = v_buf_A;
    return 1;
    return 0;
}

int64_t KS56CX2X::py_ret(int64_t v_opcode) {
    int64_t v_stack_value = 0;
    f_PC = (py_stack_pop() << 8);
    v_stack_value = py_stack_pop();
    f_PC |= ((v_stack_value << 12) & 4096);
    f_MBE = (v_stack_value >> 3);
    f_RBE = ((v_stack_value >> 2) & 1);
    f_PC |= py_stack_pop();
    f_PC |= (py_stack_pop() << 4);
    f_PC &= rom_get_mask();
    return 3;
    return 0;
}

int64_t KS56CX2X::py_reti(int64_t v_opcode) {
    int64_t v_stack_value = 0;
    f_PC = (py_stack_pop() << 8);
    v_stack_value = py_stack_pop();
    f_PC |= ((v_stack_value << 12) & 4096);
    f_MBE = (v_stack_value >> 3);
    f_RBE = ((v_stack_value >> 2) & 1);
    f_PC |= py_stack_pop();
    f_PC |= (py_stack_pop() << 4);
    v_stack_value = py_stack_pop();
    f_IST = (v_stack_value >> 2);
    f_MBE = ((v_stack_value >> 1) & 1);
    f_RBE = (v_stack_value & 1);
    v_stack_value = py_stack_pop();
    f_CY = ((v_stack_value >> 3) & 1);
    f_SK = (v_stack_value & 7);
    f_PC &= rom_get_mask();
    return 3;
    return 0;
}

int64_t KS56CX2X::py_br_pcxa(int64_t v_opcode) {
    f_PC = ((((f_PC & 65280) | (py_get_reg(1) << 4)) | py_get_reg(0)) & rom_get_mask());
    return 3;
    return 0;
}

int64_t KS56CX2X::py_br_bcxa(int64_t v_opcode) {
    f_PC = (((((py_get_reg(7) << 12) | (py_get_reg(6) << 8)) | (py_get_reg(1) << 4)) | py_get_reg(0)) & rom_get_mask());
    return 3;
    return 0;
}

int64_t KS56CX2X::py_incs_ahl(int64_t v_opcode) {
    int64_t v_new_ahl = 0;
    v_new_ahl = ((py_get_ahl() + 1) & 15);
    py_set_ahl(v_new_ahl);
    if (((v_new_ahl == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_br_pcde(int64_t v_opcode) {
    f_PC = ((((f_PC & 65280) | (py_get_reg(5) << 4)) | py_get_reg(4)) & rom_get_mask());
    return 3;
    return 0;
}

int64_t KS56CX2X::py_br_bcde(int64_t v_opcode) {
    f_PC = (((((py_get_reg(7) << 12) | (py_get_reg(6) << 8)) | (py_get_reg(5) << 4)) | py_get_reg(4)) & rom_get_mask());
    return 3;
    return 0;
}

int64_t KS56CX2X::py_pop_bs(int64_t v_opcode) {
    f_RBS = py_stack_pop();
    f_MBS = py_stack_pop();
    return 2;
    return 0;
}

int64_t KS56CX2X::py_push_bs(int64_t v_opcode) {
    py_stack_push(f_MBS);
    py_stack_push(f_RBS);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_ske_a_reg(int64_t v_opcode) {
    if (((py_get_reg(0) == py_get_reg((v_opcode & 7))))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_sel_mbn(int64_t v_opcode) {
    f_MBS = (v_opcode & 15);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_sel_rbn(int64_t v_opcode) {
    f_RBS = (v_opcode & 3);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_and_a_n4(int64_t v_opcode) {
    py_set_reg(0, (py_get_reg(0) & v_opcode));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_or_a_n4(int64_t v_opcode) {
    py_set_reg(0, ((py_get_reg(0) | v_opcode) & 15));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xor_a_n4(int64_t v_opcode) {
    py_set_reg(0, ((py_get_reg(0) ^ v_opcode) & 15));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_ske_ahl_n4(int64_t v_opcode) {
    if (((py_get_ahl() == (v_opcode & 15)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_reg1_A(int64_t v_opcode) {
    py_set_reg((v_opcode & 7), py_get_reg(0));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_a_reg(int64_t v_opcode) {
    py_set_reg(0, py_get_reg((v_opcode & 7)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_ske_reg_n4(int64_t v_opcode) {
    if (((py_get_reg((v_opcode & 7)) == ((v_opcode >> 4) & 15)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_reg1_n4(int64_t v_opcode) {
    py_set_reg((v_opcode & 7), ((v_opcode >> 4) & 15));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov1_hmembit_cy(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if (f_CY) {
        py_set_hmem(v_opcode, (py_get_hmem(v_opcode) | (1 << v_b)));
    } else {
        py_set_hmem(v_opcode, (py_get_hmem(v_opcode) & (~((1 << v_b)))));
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov1_pmeml_cy(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    if (f_CY) {
        py_set_pmeml(v_opcode, (py_get_pmeml(v_opcode) | (1 << v_b)));
    } else {
        py_set_pmeml(v_opcode, (py_get_pmeml(v_opcode) & (~((1 << v_b)))));
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov1_fmembit_cy(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if (f_CY) {
        py_set_fmem(v_opcode, (py_get_fmem(v_opcode) | (1 << v_b)));
    } else {
        py_set_fmem(v_opcode, (py_get_fmem(v_opcode) & (~((1 << v_b)))));
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_clr1_hmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    py_set_hmem(v_opcode, (py_get_hmem(v_opcode) & (~((1 << v_b)))));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_clr1_pmeml(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    py_set_pmeml(v_opcode, (py_get_pmeml(v_opcode) & (~((1 << v_b)))));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_clr1_fmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    py_set_fmem(v_opcode, (py_get_fmem(v_opcode) & (~((1 << v_b)))));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_set1_hmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    py_set_hmem(v_opcode, (py_get_hmem(v_opcode) | (1 << v_b)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_set1_pmeml(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    py_set_pmeml(v_opcode, (py_get_pmeml(v_opcode) | (1 << v_b)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_set1_fmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    py_set_fmem(v_opcode, (py_get_fmem(v_opcode) | (1 << v_b)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_sktclr_hmembit(int64_t v_opcode) {
    int64_t v_b = 0, v_mem = 0;
    v_b = ((v_opcode >> 4) & 3);
    v_mem = py_get_hmem(v_opcode);
    if ((v_mem & (1 << v_b))) {
        py_set_hmem(v_opcode, (v_mem & (~((1 << v_b)))));
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_sktclr_pmeml(int64_t v_opcode) {
    int64_t v_b = 0, v_mem = 0;
    v_b = (py_get_reg(2) & 3);
    v_mem = py_set_pmeml(v_opcode, 0);
    if ((v_mem & (1 << v_b))) {
        py_set_pmeml(v_opcode, (v_mem & (~((1 << v_b)))));
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_sktclr_fmembit(int64_t v_opcode) {
    int64_t v_b = 0, v_mem = 0;
    v_b = ((v_opcode >> 4) & 3);
    v_mem = py_get_fmem(v_opcode);
    if ((v_mem & (1 << v_b))) {
        py_set_fmem(v_opcode, (v_mem & (~((1 << v_b)))));
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_ahl_xa(int64_t v_opcode) {
    py_set_ahl_byte(py_get_rp(0));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xch_xa_ahl(int64_t v_opcode) {
    int64_t v_xa = 0;
    v_xa = py_get_rp(0);
    py_set_rp(0, py_get_ahl_byte());
    py_set_ahl_byte(v_xa);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_xa_ahl(int64_t v_opcode) {
    py_set_rp(0, py_get_ahl_byte());
    return 2;
    return 0;
}

int64_t KS56CX2X::py_ske_xa_ahl(int64_t v_opcode) {
    if (((py_get_rp(0) == py_get_ahl_byte()))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xch_xa_rpe(int64_t v_opcode) {
    int64_t v_xa = 0;
    v_xa = py_get_rp(0);
    py_set_rp(0, py_get_rp((v_opcode & 7)));
    py_set_rp((v_opcode & 7), v_xa);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_ske_xa_rpe(int64_t v_opcode) {
    if (((py_get_rp(0) == py_get_rp((v_opcode & 7))))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_rpe1_XA(int64_t v_opcode) {
    py_set_rp((v_opcode & 7), py_get_rp(0));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov_xa_rpe(int64_t v_opcode) {
    py_set_rp(0, py_get_rp((v_opcode & 7)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_decs_rpe(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp((v_opcode & 7)) - 1) & 255);
    py_set_rp((v_opcode & 7), v_new_value);
    if (((v_new_value == 255))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_and_rpe1_xa(int64_t v_opcode) {
    py_set_rp((v_opcode & 7), (py_get_rp((v_opcode & 7)) & py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_and_xa_rpe(int64_t v_opcode) {
    py_set_rp(0, (py_get_rp((v_opcode & 7)) & py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_or_rpe1_xa(int64_t v_opcode) {
    py_set_rp((v_opcode & 7), (py_get_rp((v_opcode & 7)) | py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_or_xa_rpe(int64_t v_opcode) {
    py_set_rp(0, (py_get_rp((v_opcode & 7)) | py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xor_rpe1_xa(int64_t v_opcode) {
    py_set_rp((v_opcode & 7), (py_get_rp((v_opcode & 7)) ^ py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xor_xa_rpe(int64_t v_opcode) {
    py_set_rp(0, (py_get_rp((v_opcode & 7)) ^ py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_adds_rpe1_xa(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = (py_get_rp((v_opcode & 7)) + py_get_rp(0));
    py_set_rp((v_opcode & 7), (v_new_value & 255));
    if (((v_new_value > 255))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_adds_xa_rpe(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = (py_get_rp((v_opcode & 7)) + py_get_rp(0));
    py_set_rp(0, (v_new_value & 255));
    if (((v_new_value > 255))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_addc_rpe1_xa(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp((v_opcode & 7)) + py_get_rp(0)) + f_CY);
    py_set_rp((v_opcode & 7), (v_new_value & 255));
    f_CY = ((v_new_value > 255));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_addc_xa_rpe(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp((v_opcode & 7)) + py_get_rp(0)) + f_CY);
    py_set_rp(0, (v_new_value & 255));
    f_CY = ((v_new_value > 255));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_subs_rpe1_xa(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = (py_get_rp((v_opcode & 7)) - py_get_rp(0));
    py_set_rp((v_opcode & 7), (v_new_value & 255));
    if (((v_new_value < 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_subs_xa_rpe(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = (py_get_rp(0) - py_get_rp((v_opcode & 7)));
    py_set_rp(0, (v_new_value & 255));
    if (((v_new_value < 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_subc_rpe1_xa(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp((v_opcode & 7)) - py_get_rp(0)) - f_CY);
    py_set_rp((v_opcode & 7), (v_new_value & 255));
    f_CY = ((v_new_value < 0));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_subc_xa_rpe(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp(0) - py_get_rp((v_opcode & 7))) - f_CY);
    py_set_rp(0, (v_new_value & 255));
    f_CY = ((v_new_value < 0));
    return 2;
    return 0;
}

int64_t KS56CX2X::py_and1_cy_hmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY &= ((py_get_hmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_and1_cy_pmeml(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    f_CY &= ((py_get_pmeml(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_and1_cy_fmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY &= ((py_get_fmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_or1_cy_hmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY |= ((py_get_hmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_or1_cy_pmeml(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    f_CY |= ((py_get_pmeml(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_or1_cy_fmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY |= ((py_get_fmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xor1_cy_hmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY ^= ((py_get_hmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xor1_cy_pmeml(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    f_CY ^= ((py_get_pmeml(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_xor1_cy_fmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY ^= ((py_get_fmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov1_cy_hmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY = ((py_get_hmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov1_cy_pmeml(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    f_CY = ((py_get_pmeml(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_mov1_cy_fmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY = ((py_get_fmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS56CX2X::py_skf_hmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if ((!(((py_get_hmem(v_opcode) >> v_b) & 1)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_skf_pmeml(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    if ((!(((py_get_pmeml(v_opcode) >> v_b) & 1)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_skf_fmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if ((!(((py_get_fmem(v_opcode) >> v_b) & 1)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_skt_hmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if (((py_get_hmem(v_opcode) >> v_b) & 1)) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_skt_pmeml(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    if (((py_get_pmeml(v_opcode) >> v_b) & 1)) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_skt_fmembit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if (((py_get_fmem(v_opcode) >> v_b) & 1)) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS56CX2X::py_dummy(int64_t v_opcode) {
    return 0;
    return 0;
}

const KS56CX2X::IoEntry* KS56CX2X::find_io_tbl(int64_t addr) {
    static const IoEntry e[] = {
        {&KS56CX2X::py_get_io_spl, &KS56CX2X::py_set_io_spl},
        {&KS56CX2X::py_get_io_sph, &KS56CX2X::py_set_io_sph},
        {&KS56CX2X::py_get_io_rbs, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_mbs, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_sbs, &KS56CX2X::py_set_io_sbs},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_btm},
        {&KS56CX2X::py_get_io_btl, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_bth, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_tmod2hl, &KS56CX2X::py_set_io_tmod2hl},
        {&KS56CX2X::py_get_io_tmod2hh, &KS56CX2X::py_set_io_tmod2hh},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_wdtm},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_wml, &KS56CX2X::py_set_io_wml},
        {&KS56CX2X::py_get_io_wmh, &KS56CX2X::py_set_io_wmh},
        {&KS56CX2X::py_get_io_tm0l, &KS56CX2X::py_set_io_tm0l},
        {&KS56CX2X::py_get_io_tm0h, &KS56CX2X::py_set_io_tm0h},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_t0l, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_t0h, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_tmod0l, &KS56CX2X::py_set_io_tmod0l},
        {&KS56CX2X::py_get_io_tmod0h, &KS56CX2X::py_set_io_tmod0h},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_pswl, &KS56CX2X::py_set_io_pswl},
        {&KS56CX2X::py_get_io_pswh, &KS56CX2X::py_set_io_pswh},
        {&KS56CX2X::py_get_io_ips, &KS56CX2X::py_set_io_ips},
        {&KS56CX2X::py_get_io_pcc, &KS56CX2X::py_set_io_pcc},
        {&KS56CX2X::py_get_io_im0, &KS56CX2X::py_set_io_im0},
        {&KS56CX2X::py_get_io_im1, &KS56CX2X::py_set_io_im1},
        {&KS56CX2X::py_get_io_im2, &KS56CX2X::py_set_io_im2},
        {&KS56CX2X::py_get_io_scc, &KS56CX2X::py_set_io_scc},
        {&KS56CX2X::py_get_io_inta, &KS56CX2X::py_set_io_inta},
        {&KS56CX2X::py_get_io_intc, &KS56CX2X::py_set_io_intc},
        {&KS56CX2X::py_get_io_inte, &KS56CX2X::py_set_io_inte},
        {&KS56CX2X::py_get_io_intf, &KS56CX2X::py_set_io_intf},
        {&KS56CX2X::py_get_io_intg, &KS56CX2X::py_set_io_intg},
        {&KS56CX2X::py_get_io_inth, &KS56CX2X::py_set_io_inth},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_pogal, &KS56CX2X::py_set_io_pogal},
        {&KS56CX2X::py_get_io_pogah, &KS56CX2X::py_set_io_pogah},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_pmgal, &KS56CX2X::py_set_io_pmgal},
        {&KS56CX2X::py_get_io_pmgah, &KS56CX2X::py_set_io_pmgah},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_dummy, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_port0, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_port1, &KS56CX2X::py_set_io_dummy},
        {&KS56CX2X::py_get_io_port2, &KS56CX2X::py_set_io_port2},
        {&KS56CX2X::py_get_io_port3, &KS56CX2X::py_set_io_port3},
        {&KS56CX2X::py_get_io_port5, &KS56CX2X::py_set_io_port5},
        {&KS56CX2X::py_get_io_port6, &KS56CX2X::py_set_io_port6},
        {&KS56CX2X::py_get_io_port8, &KS56CX2X::py_set_io_port8},
        {&KS56CX2X::py_get_io_port9, &KS56CX2X::py_set_io_port9},
    };
    switch (addr) {
        case 3968: return &e[0];
        case 3969: return &e[1];
        case 3970: return &e[2];
        case 3971: return &e[3];
        case 3972: return &e[4];
        case 3973: return &e[5];
        case 3974: return &e[6];
        case 3975: return &e[7];
        case 3976: return &e[8];
        case 3977: return &e[9];
        case 3979: return &e[10];
        case 3980: return &e[11];
        case 3981: return &e[12];
        case 3982: return &e[13];
        case 3983: return &e[14];
        case 3984: return &e[15];
        case 3985: return &e[16];
        case 3986: return &e[17];
        case 3987: return &e[18];
        case 3988: return &e[19];
        case 3989: return &e[20];
        case 3990: return &e[21];
        case 3991: return &e[22];
        case 3992: return &e[23];
        case 3993: return &e[24];
        case 4000: return &e[25];
        case 4001: return &e[26];
        case 4002: return &e[27];
        case 4004: return &e[28];
        case 4005: return &e[29];
        case 4006: return &e[30];
        case 4007: return &e[31];
        case 4008: return &e[32];
        case 4009: return &e[33];
        case 4010: return &e[34];
        case 4012: return &e[35];
        case 4013: return &e[36];
        case 4014: return &e[37];
        case 4015: return &e[38];
        case 4016: return &e[39];
        case 4017: return &e[40];
        case 4018: return &e[41];
        case 4019: return &e[42];
        case 4020: return &e[43];
        case 4021: return &e[44];
        case 4022: return &e[45];
        case 4023: return &e[46];
        case 4024: return &e[47];
        case 4026: return &e[48];
        case 4028: return &e[49];
        case 4029: return &e[50];
        case 4030: return &e[51];
        case 4031: return &e[52];
        case 4032: return &e[53];
        case 4033: return &e[54];
        case 4034: return &e[55];
        case 4035: return &e[56];
        case 4047: return &e[57];
        case 4048: return &e[58];
        case 4060: return &e[59];
        case 4061: return &e[60];
        case 4062: return &e[61];
        case 4063: return &e[62];
        case 4064: return &e[63];
        case 4065: return &e[64];
        case 4066: return &e[65];
        case 4067: return &e[66];
        case 4068: return &e[67];
        case 4069: return &e[68];
        case 4070: return &e[69];
        case 4071: return &e[70];
        case 4072: return &e[71];
        case 4073: return &e[72];
        case 4076: return &e[73];
        case 4077: return &e[74];
        case 4078: return &e[75];
        case 4079: return &e[76];
        case 4080: return &e[77];
        case 4081: return &e[78];
        case 4082: return &e[79];
        case 4083: return &e[80];
        case 4085: return &e[81];
        case 4086: return &e[82];
        case 4088: return &e[83];
        case 4089: return &e[84];
        default: return nullptr;
    }
}

const KS56CX2X::OpEntry KS56CX2X::T_execute[256] = {
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_br_raddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_geti_taddr, 1},
    {&KS56CX2X::py_callf_faddr, 2},
    {&KS56CX2X::py_callf_faddr, 2},
    {&KS56CX2X::py_callf_faddr, 2},
    {&KS56CX2X::py_callf_faddr, 2},
    {&KS56CX2X::py_callf_faddr, 2},
    {&KS56CX2X::py_callf_faddr, 2},
    {&KS56CX2X::py_callf_faddr, 2},
    {&KS56CX2X::py_callf_faddr, 2},
    {&KS56CX2X::py_pop_rp, 1},
    {&KS56CX2X::py_push_rp, 1},
    {&KS56CX2X::py_pop_rp, 1},
    {&KS56CX2X::py_push_rp, 1},
    {&KS56CX2X::py_pop_rp, 1},
    {&KS56CX2X::py_push_rp, 1},
    {&KS56CX2X::py_pop_rp, 1},
    {&KS56CX2X::py_push_rp, 1},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_brcb_caddr, 2},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_adds_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_mov_a_n4, 1},
    {&KS56CX2X::py_ske_a_ahl, 1},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_incs_mem, 2},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_clr1_mem_bit, 2},
    {&KS56CX2X::py_set1_mem_bit, 2},
    {&KS56CX2X::py_skf_mem_bit, 2},
    {&KS56CX2X::py_skt_mem_bit, 2},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_mov_xa_n8, 2},
    {&KS56CX2X::py_incs_rp1, 1},
    {&KS56CX2X::py_mov_hl_n8, 2},
    {&KS56CX2X::py_incs_rp1, 1},
    {&KS56CX2X::py_mov_de_n8, 2},
    {&KS56CX2X::py_incs_rp1, 1},
    {&KS56CX2X::py_mov_bc_n8, 2},
    {&KS56CX2X::py_and_a_ahl, 1},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_mov_mem_xa, 2},
    {&KS56CX2X::py_mov_mem_A, 2},
    {&KS56CX2X::py_clr1_mem_bit, 2},
    {&KS56CX2X::py_set1_mem_bit, 2},
    {&KS56CX2X::py_skf_mem_bit, 2},
    {&KS56CX2X::py_skt_mem_bit, 2},
    {&KS56CX2X::py_rorc_a, 1},
    {&KS56CX2X::py_execute_10011001, 2},
    {&KS56CX2X::py_execute_10011010, 2},
    {&KS56CX2X::py_execute_10011011, 2},
    {&KS56CX2X::py_execute_10011100, 2},
    {&KS56CX2X::py_execute_10011101, 2},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_execute_10011111, 2},
    {&KS56CX2X::py_or_a_ahl, 1},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_mov_xa_mem, 2},
    {&KS56CX2X::py_mov_a_mem, 2},
    {&KS56CX2X::py_clr1_mem_bit, 2},
    {&KS56CX2X::py_set1_mem_bit, 2},
    {&KS56CX2X::py_skf_mem_bit, 2},
    {&KS56CX2X::py_skt_mem_bit, 2},
    {&KS56CX2X::py_subs_a_ahl, 1},
    {&KS56CX2X::py_addc_a_ahl, 1},
    {&KS56CX2X::py_execute_10101010, 2},
    {&KS56CX2X::py_execute_10101011, 3},
    {&KS56CX2X::py_execute_10101100, 2},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_execute_10101110, 2},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_xor_a_ahl, 1},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_xch_xa_mem, 2},
    {&KS56CX2X::py_xch_a_mem, 2},
    {&KS56CX2X::py_clr1_mem_bit, 2},
    {&KS56CX2X::py_set1_mem_bit, 2},
    {&KS56CX2X::py_skf_mem_bit, 2},
    {&KS56CX2X::py_skt_mem_bit, 2},
    {&KS56CX2X::py_subc_a_ahl, 1},
    {&KS56CX2X::py_adds_xa_n8, 2},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_execute_10111100, 2},
    {&KS56CX2X::py_execute_10111101, 2},
    {&KS56CX2X::py_execute_10111110, 2},
    {&KS56CX2X::py_execute_10111111, 2},
    {&KS56CX2X::py_incs_reg, 1},
    {&KS56CX2X::py_incs_reg, 1},
    {&KS56CX2X::py_incs_reg, 1},
    {&KS56CX2X::py_incs_reg, 1},
    {&KS56CX2X::py_incs_reg, 1},
    {&KS56CX2X::py_incs_reg, 1},
    {&KS56CX2X::py_incs_reg, 1},
    {&KS56CX2X::py_incs_reg, 1},
    {&KS56CX2X::py_decs_reg, 1},
    {&KS56CX2X::py_decs_reg, 1},
    {&KS56CX2X::py_decs_reg, 1},
    {&KS56CX2X::py_decs_reg, 1},
    {&KS56CX2X::py_decs_reg, 1},
    {&KS56CX2X::py_decs_reg, 1},
    {&KS56CX2X::py_decs_reg, 1},
    {&KS56CX2X::py_decs_reg, 1},
    {&KS56CX2X::py_movt_xa_pcxa, 1},
    {&KS56CX2X::py_movt_xa_bcxa, 1},
    {&KS56CX2X::py_adds_a_ahl, 1},
    {&KS56CX2X::py_dummy, 1},
    {&KS56CX2X::py_movt_xa_pcde, 1},
    {&KS56CX2X::py_movt_xa_bcde, 1},
    {&KS56CX2X::py_not1_cy, 1},
    {&KS56CX2X::py_skt_cy, 1},
    {&KS56CX2X::py_xch_a_reg1, 1},
    {&KS56CX2X::py_xch_a_reg1, 1},
    {&KS56CX2X::py_xch_a_reg1, 1},
    {&KS56CX2X::py_xch_a_reg1, 1},
    {&KS56CX2X::py_xch_a_reg1, 1},
    {&KS56CX2X::py_xch_a_reg1, 1},
    {&KS56CX2X::py_xch_a_reg1, 1},
    {&KS56CX2X::py_xch_a_reg1, 1},
    {&KS56CX2X::py_rets, 1},
    {&KS56CX2X::py_mov_a_ahl, 1},
    {&KS56CX2X::py_mov_a_ahli, 1},
    {&KS56CX2X::py_mov_a_ahld, 1},
    {&KS56CX2X::py_mov_a_ade, 1},
    {&KS56CX2X::py_mov_a_adl, 1},
    {&KS56CX2X::py_clr1_cy, 1},
    {&KS56CX2X::py_set1_cy, 1},
    {&KS56CX2X::py_mov_ahl_a, 1},
    {&KS56CX2X::py_xch_a_ahl, 1},
    {&KS56CX2X::py_xch_a_ahli, 1},
    {&KS56CX2X::py_xch_a_ahld, 1},
    {&KS56CX2X::py_xch_a_ade, 1},
    {&KS56CX2X::py_xch_a_adl, 1},
    {&KS56CX2X::py_ret, 1},
    {&KS56CX2X::py_reti, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
    {&KS56CX2X::py_br_mraddr, 1},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10011001[128] = {
    {&KS56CX2X::py_br_pcxa, 0},
    {&KS56CX2X::py_br_bcxa, 0},
    {&KS56CX2X::py_incs_ahl, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_br_pcde, 0},
    {&KS56CX2X::py_br_bcde, 0},
    {&KS56CX2X::py_pop_bs, 0},
    {&KS56CX2X::py_push_bs, 0},
    {&KS56CX2X::py_ske_a_reg, 0},
    {&KS56CX2X::py_ske_a_reg, 0},
    {&KS56CX2X::py_ske_a_reg, 0},
    {&KS56CX2X::py_ske_a_reg, 0},
    {&KS56CX2X::py_ske_a_reg, 0},
    {&KS56CX2X::py_ske_a_reg, 0},
    {&KS56CX2X::py_ske_a_reg, 0},
    {&KS56CX2X::py_ske_a_reg, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_mbn, 0},
    {&KS56CX2X::py_sel_rbn, 0},
    {&KS56CX2X::py_sel_rbn, 0},
    {&KS56CX2X::py_sel_rbn, 0},
    {&KS56CX2X::py_sel_rbn, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_and_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_or_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_xor_a_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_ske_ahl_n4, 0},
    {&KS56CX2X::py_mov_reg1_A, 0},
    {&KS56CX2X::py_mov_reg1_A, 0},
    {&KS56CX2X::py_mov_reg1_A, 0},
    {&KS56CX2X::py_mov_reg1_A, 0},
    {&KS56CX2X::py_mov_reg1_A, 0},
    {&KS56CX2X::py_mov_reg1_A, 0},
    {&KS56CX2X::py_mov_reg1_A, 0},
    {&KS56CX2X::py_mov_reg1_A, 0},
    {&KS56CX2X::py_mov_a_reg, 0},
    {&KS56CX2X::py_mov_a_reg, 0},
    {&KS56CX2X::py_mov_a_reg, 0},
    {&KS56CX2X::py_mov_a_reg, 0},
    {&KS56CX2X::py_mov_a_reg, 0},
    {&KS56CX2X::py_mov_a_reg, 0},
    {&KS56CX2X::py_mov_a_reg, 0},
    {&KS56CX2X::py_mov_a_reg, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10101011[2] = {
    {&KS56CX2X::py_br_addr, 0},
    {&KS56CX2X::py_call_addr, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10011010[2] = {
    {&KS56CX2X::py_ske_reg_n4, 0},
    {&KS56CX2X::py_mov_reg1_n4, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10011011[4] = {
    {&KS56CX2X::py_mov1_hmembit_cy, 0},
    {&KS56CX2X::py_mov1_pmeml_cy, 0},
    {&KS56CX2X::py_mov1_fmembit_cy, 0},
    {&KS56CX2X::py_mov1_fmembit_cy, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10011100[4] = {
    {&KS56CX2X::py_clr1_hmembit, 0},
    {&KS56CX2X::py_clr1_pmeml, 0},
    {&KS56CX2X::py_clr1_fmembit, 0},
    {&KS56CX2X::py_clr1_fmembit, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10011101[4] = {
    {&KS56CX2X::py_set1_hmembit, 0},
    {&KS56CX2X::py_set1_pmeml, 0},
    {&KS56CX2X::py_set1_fmembit, 0},
    {&KS56CX2X::py_set1_fmembit, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10011111[4] = {
    {&KS56CX2X::py_sktclr_hmembit, 0},
    {&KS56CX2X::py_sktclr_pmeml, 0},
    {&KS56CX2X::py_sktclr_fmembit, 0},
    {&KS56CX2X::py_sktclr_fmembit, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10101010[256] = {
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_mov_ahl_xa, 0},
    {&KS56CX2X::py_xch_xa_ahl, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_mov_xa_ahl, 0},
    {&KS56CX2X::py_ske_xa_ahl, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_xch_xa_rpe, 0},
    {&KS56CX2X::py_xch_xa_rpe, 0},
    {&KS56CX2X::py_xch_xa_rpe, 0},
    {&KS56CX2X::py_xch_xa_rpe, 0},
    {&KS56CX2X::py_xch_xa_rpe, 0},
    {&KS56CX2X::py_xch_xa_rpe, 0},
    {&KS56CX2X::py_xch_xa_rpe, 0},
    {&KS56CX2X::py_xch_xa_rpe, 0},
    {&KS56CX2X::py_ske_xa_rpe, 0},
    {&KS56CX2X::py_ske_xa_rpe, 0},
    {&KS56CX2X::py_ske_xa_rpe, 0},
    {&KS56CX2X::py_ske_xa_rpe, 0},
    {&KS56CX2X::py_ske_xa_rpe, 0},
    {&KS56CX2X::py_ske_xa_rpe, 0},
    {&KS56CX2X::py_ske_xa_rpe, 0},
    {&KS56CX2X::py_ske_xa_rpe, 0},
    {&KS56CX2X::py_mov_rpe1_XA, 0},
    {&KS56CX2X::py_mov_rpe1_XA, 0},
    {&KS56CX2X::py_mov_rpe1_XA, 0},
    {&KS56CX2X::py_mov_rpe1_XA, 0},
    {&KS56CX2X::py_mov_rpe1_XA, 0},
    {&KS56CX2X::py_mov_rpe1_XA, 0},
    {&KS56CX2X::py_mov_rpe1_XA, 0},
    {&KS56CX2X::py_mov_rpe1_XA, 0},
    {&KS56CX2X::py_mov_xa_rpe, 0},
    {&KS56CX2X::py_mov_xa_rpe, 0},
    {&KS56CX2X::py_mov_xa_rpe, 0},
    {&KS56CX2X::py_mov_xa_rpe, 0},
    {&KS56CX2X::py_mov_xa_rpe, 0},
    {&KS56CX2X::py_mov_xa_rpe, 0},
    {&KS56CX2X::py_mov_xa_rpe, 0},
    {&KS56CX2X::py_mov_xa_rpe, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_decs_rpe, 0},
    {&KS56CX2X::py_decs_rpe, 0},
    {&KS56CX2X::py_decs_rpe, 0},
    {&KS56CX2X::py_decs_rpe, 0},
    {&KS56CX2X::py_decs_rpe, 0},
    {&KS56CX2X::py_decs_rpe, 0},
    {&KS56CX2X::py_decs_rpe, 0},
    {&KS56CX2X::py_decs_rpe, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_dummy, 0},
    {&KS56CX2X::py_and_rpe1_xa, 0},
    {&KS56CX2X::py_and_rpe1_xa, 0},
    {&KS56CX2X::py_and_rpe1_xa, 0},
    {&KS56CX2X::py_and_rpe1_xa, 0},
    {&KS56CX2X::py_and_rpe1_xa, 0},
    {&KS56CX2X::py_and_rpe1_xa, 0},
    {&KS56CX2X::py_and_rpe1_xa, 0},
    {&KS56CX2X::py_and_rpe1_xa, 0},
    {&KS56CX2X::py_and_xa_rpe, 0},
    {&KS56CX2X::py_and_xa_rpe, 0},
    {&KS56CX2X::py_and_xa_rpe, 0},
    {&KS56CX2X::py_and_xa_rpe, 0},
    {&KS56CX2X::py_and_xa_rpe, 0},
    {&KS56CX2X::py_and_xa_rpe, 0},
    {&KS56CX2X::py_and_xa_rpe, 0},
    {&KS56CX2X::py_and_xa_rpe, 0},
    {&KS56CX2X::py_or_rpe1_xa, 0},
    {&KS56CX2X::py_or_rpe1_xa, 0},
    {&KS56CX2X::py_or_rpe1_xa, 0},
    {&KS56CX2X::py_or_rpe1_xa, 0},
    {&KS56CX2X::py_or_rpe1_xa, 0},
    {&KS56CX2X::py_or_rpe1_xa, 0},
    {&KS56CX2X::py_or_rpe1_xa, 0},
    {&KS56CX2X::py_or_rpe1_xa, 0},
    {&KS56CX2X::py_or_xa_rpe, 0},
    {&KS56CX2X::py_or_xa_rpe, 0},
    {&KS56CX2X::py_or_xa_rpe, 0},
    {&KS56CX2X::py_or_xa_rpe, 0},
    {&KS56CX2X::py_or_xa_rpe, 0},
    {&KS56CX2X::py_or_xa_rpe, 0},
    {&KS56CX2X::py_or_xa_rpe, 0},
    {&KS56CX2X::py_or_xa_rpe, 0},
    {&KS56CX2X::py_xor_rpe1_xa, 0},
    {&KS56CX2X::py_xor_rpe1_xa, 0},
    {&KS56CX2X::py_xor_rpe1_xa, 0},
    {&KS56CX2X::py_xor_rpe1_xa, 0},
    {&KS56CX2X::py_xor_rpe1_xa, 0},
    {&KS56CX2X::py_xor_rpe1_xa, 0},
    {&KS56CX2X::py_xor_rpe1_xa, 0},
    {&KS56CX2X::py_xor_rpe1_xa, 0},
    {&KS56CX2X::py_xor_xa_rpe, 0},
    {&KS56CX2X::py_xor_xa_rpe, 0},
    {&KS56CX2X::py_xor_xa_rpe, 0},
    {&KS56CX2X::py_xor_xa_rpe, 0},
    {&KS56CX2X::py_xor_xa_rpe, 0},
    {&KS56CX2X::py_xor_xa_rpe, 0},
    {&KS56CX2X::py_xor_xa_rpe, 0},
    {&KS56CX2X::py_xor_xa_rpe, 0},
    {&KS56CX2X::py_adds_rpe1_xa, 0},
    {&KS56CX2X::py_adds_rpe1_xa, 0},
    {&KS56CX2X::py_adds_rpe1_xa, 0},
    {&KS56CX2X::py_adds_rpe1_xa, 0},
    {&KS56CX2X::py_adds_rpe1_xa, 0},
    {&KS56CX2X::py_adds_rpe1_xa, 0},
    {&KS56CX2X::py_adds_rpe1_xa, 0},
    {&KS56CX2X::py_adds_rpe1_xa, 0},
    {&KS56CX2X::py_adds_xa_rpe, 0},
    {&KS56CX2X::py_adds_xa_rpe, 0},
    {&KS56CX2X::py_adds_xa_rpe, 0},
    {&KS56CX2X::py_adds_xa_rpe, 0},
    {&KS56CX2X::py_adds_xa_rpe, 0},
    {&KS56CX2X::py_adds_xa_rpe, 0},
    {&KS56CX2X::py_adds_xa_rpe, 0},
    {&KS56CX2X::py_adds_xa_rpe, 0},
    {&KS56CX2X::py_addc_rpe1_xa, 0},
    {&KS56CX2X::py_addc_rpe1_xa, 0},
    {&KS56CX2X::py_addc_rpe1_xa, 0},
    {&KS56CX2X::py_addc_rpe1_xa, 0},
    {&KS56CX2X::py_addc_rpe1_xa, 0},
    {&KS56CX2X::py_addc_rpe1_xa, 0},
    {&KS56CX2X::py_addc_rpe1_xa, 0},
    {&KS56CX2X::py_addc_rpe1_xa, 0},
    {&KS56CX2X::py_addc_xa_rpe, 0},
    {&KS56CX2X::py_addc_xa_rpe, 0},
    {&KS56CX2X::py_addc_xa_rpe, 0},
    {&KS56CX2X::py_addc_xa_rpe, 0},
    {&KS56CX2X::py_addc_xa_rpe, 0},
    {&KS56CX2X::py_addc_xa_rpe, 0},
    {&KS56CX2X::py_addc_xa_rpe, 0},
    {&KS56CX2X::py_addc_xa_rpe, 0},
    {&KS56CX2X::py_subs_rpe1_xa, 0},
    {&KS56CX2X::py_subs_rpe1_xa, 0},
    {&KS56CX2X::py_subs_rpe1_xa, 0},
    {&KS56CX2X::py_subs_rpe1_xa, 0},
    {&KS56CX2X::py_subs_rpe1_xa, 0},
    {&KS56CX2X::py_subs_rpe1_xa, 0},
    {&KS56CX2X::py_subs_rpe1_xa, 0},
    {&KS56CX2X::py_subs_rpe1_xa, 0},
    {&KS56CX2X::py_subs_xa_rpe, 0},
    {&KS56CX2X::py_subs_xa_rpe, 0},
    {&KS56CX2X::py_subs_xa_rpe, 0},
    {&KS56CX2X::py_subs_xa_rpe, 0},
    {&KS56CX2X::py_subs_xa_rpe, 0},
    {&KS56CX2X::py_subs_xa_rpe, 0},
    {&KS56CX2X::py_subs_xa_rpe, 0},
    {&KS56CX2X::py_subs_xa_rpe, 0},
    {&KS56CX2X::py_subc_rpe1_xa, 0},
    {&KS56CX2X::py_subc_rpe1_xa, 0},
    {&KS56CX2X::py_subc_rpe1_xa, 0},
    {&KS56CX2X::py_subc_rpe1_xa, 0},
    {&KS56CX2X::py_subc_rpe1_xa, 0},
    {&KS56CX2X::py_subc_rpe1_xa, 0},
    {&KS56CX2X::py_subc_rpe1_xa, 0},
    {&KS56CX2X::py_subc_rpe1_xa, 0},
    {&KS56CX2X::py_subc_xa_rpe, 0},
    {&KS56CX2X::py_subc_xa_rpe, 0},
    {&KS56CX2X::py_subc_xa_rpe, 0},
    {&KS56CX2X::py_subc_xa_rpe, 0},
    {&KS56CX2X::py_subc_xa_rpe, 0},
    {&KS56CX2X::py_subc_xa_rpe, 0},
    {&KS56CX2X::py_subc_xa_rpe, 0},
    {&KS56CX2X::py_subc_xa_rpe, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10101100[4] = {
    {&KS56CX2X::py_and1_cy_hmembit, 0},
    {&KS56CX2X::py_and1_cy_pmeml, 0},
    {&KS56CX2X::py_and1_cy_fmembit, 0},
    {&KS56CX2X::py_and1_cy_fmembit, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10101110[4] = {
    {&KS56CX2X::py_or1_cy_hmembit, 0},
    {&KS56CX2X::py_or1_cy_pmeml, 0},
    {&KS56CX2X::py_or1_cy_fmembit, 0},
    {&KS56CX2X::py_or1_cy_fmembit, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10111100[4] = {
    {&KS56CX2X::py_xor1_cy_hmembit, 0},
    {&KS56CX2X::py_xor1_cy_pmeml, 0},
    {&KS56CX2X::py_xor1_cy_fmembit, 0},
    {&KS56CX2X::py_xor1_cy_fmembit, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10111101[4] = {
    {&KS56CX2X::py_mov1_cy_hmembit, 0},
    {&KS56CX2X::py_mov1_cy_pmeml, 0},
    {&KS56CX2X::py_mov1_cy_fmembit, 0},
    {&KS56CX2X::py_mov1_cy_fmembit, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10111110[4] = {
    {&KS56CX2X::py_skf_hmembit, 0},
    {&KS56CX2X::py_skf_pmeml, 0},
    {&KS56CX2X::py_skf_fmembit, 0},
    {&KS56CX2X::py_skf_fmembit, 0},
};

const KS56CX2X::OpEntry KS56CX2X::T_execute_10111111[4] = {
    {&KS56CX2X::py_skt_hmembit, 0},
    {&KS56CX2X::py_skt_pmeml, 0},
    {&KS56CX2X::py_skt_fmembit, 0},
    {&KS56CX2X::py_skt_fmembit, 0},
};
