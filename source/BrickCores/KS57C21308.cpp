#include "KS57C21308.h"

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

const int64_t k_MAIN_CLOCK_DIV[] = {64, 64, 8, 4};
const int64_t k_BASIC_TIMER_DIV[] = {4096, 4096, 4096, 512, 512, 128, 128, 32};
const int64_t k_TIMER_T0_DIV[] = {0, 0, 0, 0, 1024, 64, 16, 1};
const int64_t k_WATCH_TIMER_DIV[] = {16384, 128};

}  // namespace

int64_t KS57C21308::rom_get_byte(int64_t a) const {
    const int64_t n = rom_size();
    return rom()[(size_t)(((a % n) + n) % n)];
}

void KS57C21308::reset() {
    if (!inited_) {
        inited_ = true;
        init_();
    }
    py_reset();
}

void KS57C21308::init_() {
    f_instr_counter = 0;
    f_basic_timer_counter = 0;
    f_T0_timer_counter = 0;
    f_watch_timer_counter = 0;
    f_sub_clock_div = ((double)((double)clock_hz()) / (32768));
    f_cpu_clock_div = k_MAIN_CLOCK_DIV[0];
    f_debug_page = 0;
}

int64_t KS57C21308::execute_scaled() {
    const int64_t scaled = std::llround((double)pyclock() * kTickHz);
    return scaled > 0 ? scaled : 1;
}

uint8_t KS57C21308::display_byte(int i) const {
    if (i < 0 || i >= 256) return 0;
    return (uint8_t)f_RAM[5120 + i];
}

int KS57C21308::port_id(const std::string& name) const {
    if (name == "CY") return kPort_CY;
    if (name == "EA") return kPort_EA;
    if (name == "GRAM") return kPort_GRAM;
    if (name == "HL") return kPort_HL;
    if (name == "IME") return kPort_IME;
    if (name == "IORAM") return kPort_IORAM;
    if (name == "MBE") return kPort_MBE;
    if (name == "MBS") return kPort_MBS;
    if (name == "MEMORY") return kPort_MEMORY;
    if (name == "PAGE") return kPort_PAGE;
    if (name == "PASR") return kPort_PASR;
    if (name == "PC") return kPort_PC;
    if (name == "PORT0") return kPort_PORT0;
    if (name == "PORT1") return kPort_PORT1;
    if (name == "PORT6") return kPort_PORT6;
    if (name == "RAM") return kPort_RAM;
    if (name == "RBE") return kPort_RBE;
    if (name == "RBS") return kPort_RBS;
    if (name == "SP") return kPort_SP;
    if (name == "WX") return kPort_WX;
    if (name == "YZ") return kPort_YZ;
    return -1;
}

void KS57C21308::port_input(int port, uint8_t mask, int level) {
    pypin_handler(port, mask, level);
}

void KS57C21308::serialize(BrickStateIO& io) {
    io.pod(f_instr_counter);
    io.pod(f_basic_timer_counter);
    io.pod(f_T0_timer_counter);
    io.pod(f_watch_timer_counter);
    io.pod(f_sub_clock_div);
    io.pod(f_cpu_clock_div);
    io.pod(f_debug_page);
    io.pod(f_PC);
    io.pod(f_CY);
    io.pod(f_SP);
    io.pod(f_EMB);
    io.pod(f_SMB);
    io.pod(f_ERB);
    io.pod(f_SRB);
    io.pod(f_BCNT);
    io.pod(f_BMOD);
    io.pod(f_IST);
    io.pod(f_SK);
    io.pod(f_SCMOD);
    io.pod(f_PASR);
    io.pod(f_WMODl);
    io.pod(f_WMODh);
    io.pod(f_TMOD0l);
    io.pod(f_TMOD0h);
    io.pod(f_TOE0);
    io.pod(f_LMOD0);
    io.pod(f_LMOD1);
    io.pod(f_TCNT0);
    io.pod(f_TREF0);
    io.pod(f_BSC0);
    io.pod(f_BSC1);
    io.pod(f_BSC2);
    io.pod(f_BSC3);
    io.pod(f_IME);
    io.pod(f_IPR);
    io.pod(f_PCON_MODE);
    io.pod(f_PCON_CLOCK);
    io.pod(f_IM0);
    io.pod(f_IM1);
    io.pod(f_IM2);
    io.pod(f_INT4B);
    io.pod(f_INTW);
    io.pod(f_INTT0);
    io.pod(f_INTP0);
    io.pod(f_INT01);
    io.pod(f_INT2);
    ser_vec(io, f_PORT0);
    ser_vec(io, f_PORT1);
    ser_vec(io, f_PORT2);
    ser_vec(io, f_PORT4);
    ser_vec(io, f_PORT5);
    ser_vec(io, f_PORT6);
    io.pod(f_PORT2_OUT_LATCH);
    io.pod(f_PORT4_OUT_LATCH);
    io.pod(f_PORT5_OUT_LATCH);
    io.pod(f_PORT6_OUT_LATCH);
    io.pod(f_PORT4_TC0_OUT);
    io.pod(f_PUMOD0);
    io.pod(f_POGB);
    io.pod(f_PM2);
    io.pod(f_PM4);
    io.pod(f_PM5);
    io.pod(f_PM6);
    ser_vec(io, f_RAM);
    io.pod(f_TMOD2H);
    io.pod(f_PMG2);
    io.pod(f_rom);
}


void KS57C21308::py_init_registers() {
    f_PC = 0;
    f_CY = 0;
    f_SP = 0;
    f_EMB = 0;
    f_SMB = 0;
    f_ERB = 0;
    f_SRB = 0;
    f_BCNT = 0;
    f_BMOD = 0;
    f_IST = 0;
    f_SK = 0;
    f_SCMOD = 0;
    f_PASR = 0;
    f_WMODl = 0;
    f_WMODh = 0;
    f_TMOD0l = 0;
    f_TMOD0h = 0;
    f_TOE0 = 0;
    f_LMOD0 = 0;
    f_LMOD1 = 0;
    f_TCNT0 = 0;
    f_TREF0 = 255;
    f_BSC0 = 0;
    f_BSC1 = 0;
    f_BSC2 = 0;
    f_BSC3 = 0;
    f_IME = 0;
    f_IPR = 0;
    f_PCON_MODE = 0;
    f_PCON_CLOCK = 0;
    f_SCMOD = 0;
    f_IM0 = 0;
    f_IM1 = 0;
    f_IM2 = 0;
    f_INT4B = 0;
    f_INTW = 0;
    f_INTT0 = 0;
    f_INTP0 = 0;
    f_INT01 = 0;
    f_INT2 = 0;
    f_PORT0 = {0, 0};
    f_PORT1 = {0, 0};
    f_PORT2 = {0, 0};
    f_PORT4 = {0, 0};
    f_PORT5 = {0, 0};
    f_PORT6 = {0, 0};
    f_PORT2_OUT_LATCH = 0;
    f_PORT4_OUT_LATCH = 0;
    f_PORT5_OUT_LATCH = 0;
    f_PORT6_OUT_LATCH = 0;
    f_PORT4_TC0_OUT = 0;
    f_PUMOD0 = 0;
    f_POGB = 0;
    f_PM2 = 0;
    f_PM4 = 0;
    f_PM5 = 0;
    f_PM6 = 0;
    f_RAM.assign((size_t)(7936), 0);
}

void KS57C21308::py_reset() {
    snd_toggle(0, 0);
    py_init_registers();
    py_go_vector(0);
}

void KS57C21308::pyreset() {
    py_reset();
}

int64_t KS57C21308::py_get_io_dummy() {
    return 0;
    return 0;
}

int64_t KS57C21308::py_set_io_dummy(int64_t v_value) {
    return 0;
}

int64_t KS57C21308::py_set_io_bsc0(int64_t v_value) {
    f_BSC0 = (v_value & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_bsc1(int64_t v_value) {
    f_BSC1 = (v_value & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_bsc2(int64_t v_value) {
    f_BSC2 = (v_value & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_bsc3(int64_t v_value) {
    f_BSC3 = (v_value & 15);
    return 0;
}

int64_t KS57C21308::py_get_io_bsc0() {
    return f_BSC0;
    return 0;
}

int64_t KS57C21308::py_get_io_bsc1() {
    return f_BSC1;
    return 0;
}

int64_t KS57C21308::py_get_io_bsc2() {
    return f_BSC2;
    return 0;
}

int64_t KS57C21308::py_get_io_bsc3() {
    return f_BSC3;
    return 0;
}

int64_t KS57C21308::py_get_io_spl() {
    return (f_SP & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_spl(int64_t v_value) {
    f_SP = ((f_SP & 240) | (v_value & 14));
    return 0;
}

int64_t KS57C21308::py_get_io_sph() {
    return (f_SP >> 4);
    return 0;
}

int64_t KS57C21308::py_set_io_sph(int64_t v_value) {
    f_SP = ((f_SP & 15) | ((v_value & 15) << 4));
    return 0;
}

int64_t KS57C21308::py_set_io_bmod(int64_t v_value) {
    f_BMOD = (v_value & 7);
    if ((v_value & 8)) {
        f_INT4B &= (~(1));
        f_BCNT = 0;
    }
    return 0;
}

int64_t KS57C21308::py_get_io_bcntl() {
    return (f_BCNT & 15);
    return 0;
}

int64_t KS57C21308::py_get_io_bcnth() {
    return (f_BCNT >> 4);
    return 0;
}

int64_t KS57C21308::py_get_io_tmod2hl() {
    return (f_TMOD2H & 15);
    return 0;
}

void KS57C21308::py_set_io_tmod2hl(int64_t v_value) {
    f_TMOD2H = ((f_TMOD2H & 240) | (v_value & 15));
}

int64_t KS57C21308::py_get_io_tmod2hh() {
    return (f_TMOD2H >> 4);
    return 0;
}

void KS57C21308::py_set_io_tmod2hh(int64_t v_value) {
    f_TMOD2H = ((f_TMOD2H & 15) | ((v_value & 15) << 4));
}

int64_t KS57C21308::py_set_io_wmodl(int64_t v_value) {
    f_WMODl = (v_value & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_wmodh(int64_t v_value) {
    if ((v_value & 4)) {
        f_INTW |= 1;
        if ((!((f_WMODh & 4)))) {
            f_watch_timer_counter = 0;
        }
    }
    f_WMODh = (v_value & 11);
    return 0;
}

int64_t KS57C21308::py_get_io_pasrl() {
    return (f_PASR & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_pasrl(int64_t v_value) {
    f_PASR = ((f_PASR & 240) | (v_value & 15));
    return 0;
}

int64_t KS57C21308::py_get_io_pasrh() {
    return ((f_PASR >> 4) & 1);
    return 0;
}

int64_t KS57C21308::py_set_io_pasrh(int64_t v_value) {
    f_PASR = (((v_value & 1) << 4) | (f_PASR & 15));
    return 0;
}

int64_t KS57C21308::py_get_io_tcnt0l() {
    return (f_TCNT0 & 15);
    return 0;
}

int64_t KS57C21308::py_get_io_tcnt0h() {
    return (f_TCNT0 >> 4);
    return 0;
}

int64_t KS57C21308::py_set_io_tmod0h(int64_t v_value) {
    f_TMOD0h = (v_value & 7);
    return 0;
}

int64_t KS57C21308::py_get_io_toe0() {
    return f_TOE0;
    return 0;
}

int64_t KS57C21308::py_set_io_toe0(int64_t v_value) {
    f_TOE0 = (v_value & 4);
    return 0;
}

int64_t KS57C21308::py_set_io_lmod0(int64_t v_value) {
    f_LMOD0 = v_value;
    return 0;
}

int64_t KS57C21308::py_set_io_lmod1(int64_t v_value) {
    f_LMOD1 = v_value;
    return 0;
}

int64_t KS57C21308::py_set_io_tmod0l(int64_t v_value) {
    if ((v_value & 8)) {
        f_INTT0 &= (~(1));
        f_TCNT0 = 0;
    }
    f_TMOD0l = (v_value & (4 | 8));
    return 0;
}

int64_t KS57C21308::py_set_io_tref0l(int64_t v_value) {
    f_TREF0 = ((f_TREF0 & 240) | (v_value & 15));
    return 0;
}

int64_t KS57C21308::py_set_io_tref0h(int64_t v_value) {
    f_TREF0 = ((f_TREF0 & 15) | ((v_value << 4) & 240));
    return 0;
}

int64_t KS57C21308::py_get_io_pswl() {
    return (((f_IST << 2) | (f_EMB << 1)) | f_ERB);
    return 0;
}

int64_t KS57C21308::py_set_io_pswl(int64_t v_value) {
    f_IST = ((v_value >> 2) & 3);
    f_EMB = ((v_value >> 1) & 1);
    f_ERB = (v_value & 1);
    return 0;
}

int64_t KS57C21308::py_get_io_pswh() {
    return ((f_CY << 3) | f_SK);
    return 0;
}

int64_t KS57C21308::py_set_io_pswh(int64_t v_value) {
    f_CY = ((v_value >> 3) & 1);
    f_SK = (v_value & 7);
    return 0;
}

int64_t KS57C21308::py_get_io_ipr() {
    return ((f_IME << 3) | f_IPR);
    return 0;
}

int64_t KS57C21308::py_set_io_ipr(int64_t v_value) {
    f_IME = ((v_value >> 3) & 1);
    f_IPR = (v_value & 7);
    return 0;
}

int64_t KS57C21308::py_get_io_pcon() {
    return ((f_PCON_MODE << 2) | f_PCON_CLOCK);
    return 0;
}

int64_t KS57C21308::py_set_io_pcon(int64_t v_value) {
    f_PCON_MODE = ((v_value >> 2) & 3);
    f_PCON_CLOCK = (v_value & 3);
    pyupdate_cpu_clock_div();
    return 0;
}

int64_t KS57C21308::py_get_io_im0() {
    return f_IM0;
    return 0;
}

int64_t KS57C21308::py_set_io_im0(int64_t v_value) {
    f_IM0 = (v_value & 15);
    return 0;
}

int64_t KS57C21308::py_get_io_im1() {
    return f_IM1;
    return 0;
}

int64_t KS57C21308::py_set_io_im1(int64_t v_value) {
    f_IM1 = (v_value & 1);
    return 0;
}

int64_t KS57C21308::py_get_io_im2() {
    return f_IM2;
    return 0;
}

int64_t KS57C21308::py_set_io_im2(int64_t v_value) {
    f_IM2 = (v_value & 3);
    return 0;
}

int64_t KS57C21308::py_set_io_scmod(int64_t v_value) {
    f_SCMOD = (v_value & 9);
    pyupdate_cpu_clock_div();
    return 0;
}

int64_t KS57C21308::py_get_io_int4b() {
    return f_INT4B;
    return 0;
}

int64_t KS57C21308::py_set_io_int4b(int64_t v_value) {
    f_INT4B = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_intw() {
    return f_INTW;
    return 0;
}

int64_t KS57C21308::py_set_io_intw(int64_t v_value) {
    f_INTW = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_intt0() {
    return f_INTT0;
    return 0;
}

int64_t KS57C21308::py_set_io_intt0(int64_t v_value) {
    f_INTT0 = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_intp0() {
    return f_INTP0;
    return 0;
}

int64_t KS57C21308::py_set_io_intp0(int64_t v_value) {
    f_INTP0 = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_int01() {
    return f_INT01;
    return 0;
}

int64_t KS57C21308::py_set_io_int01(int64_t v_value) {
    f_INT01 = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_int2() {
    return f_INT2;
    return 0;
}

int64_t KS57C21308::py_set_io_int2(int64_t v_value) {
    f_INT2 = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_pumodl() {
    return (f_PUMOD0 & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_clmod(int64_t v_value) {
    return 0;
}

int64_t KS57C21308::py_set_io_pumodl(int64_t v_value) {
    f_PUMOD0 = ((f_PUMOD0 & 240) | (v_value & 15));
    return 0;
}

int64_t KS57C21308::py_get_io_pumodh() {
    return ((f_PUMOD0 >> 4) & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_pumodh(int64_t v_value) {
    f_PUMOD0 = ((f_PUMOD0 & 15) | ((v_value & 15) << 4));
    return 0;
}

int64_t KS57C21308::py_set_io_pmg1l(int64_t v_value) {
    f_PM2 = ((v_value & 12) >> 2);
    return 0;
}

int64_t KS57C21308::py_set_io_pmg1h(int64_t v_value) {
    f_PM6 = (v_value & 15);
    return 0;
}

int64_t KS57C21308::py_set_io_pmg2(int64_t v_value) {
    f_PMG2 = (v_value & 15);
    if ((v_value & 1)) {
        f_PM4 = 15;
    }
    if ((v_value & 2)) {
        f_PM5 = 15;
    }
    return 0;
}

int64_t KS57C21308::py_get_io_port0() {
    return ((~(f_PORT0[0])) & (f_PORT0[1] | ((((f_LMOD1 & 1) > 0)) * 15)));
    return 0;
}

int64_t KS57C21308::py_get_io_port1() {
    return ((~(f_PORT1[0])) & (f_PORT1[1] | ((((f_PUMOD0 & 1) > 0)) * 15)));
    return 0;
}

int64_t KS57C21308::py_get_io_port2() {
    int64_t v_ext = 0;
    v_ext = ((~(f_PORT2[0])) & (f_PORT2[1] | ((((f_PUMOD0 & 2) > 0)) * 15)));
    return ((v_ext & (~(f_PM2))) | (f_PORT2_OUT_LATCH & f_PM2));
    return 0;
}

int64_t KS57C21308::py_set_io_port2(int64_t v_value) {
    f_PORT2_OUT_LATCH = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_port4() {
    int64_t v_ext = 0;
    v_ext = ((~(f_PORT4[0])) & (f_PORT4[1] | ((((f_PUMOD0 & 4) > 0)) * 15)));
    return ((v_ext & (~(f_PM4))) | ((f_PORT4_OUT_LATCH | f_PORT4_TC0_OUT) & f_PM4));
    return 0;
}

int64_t KS57C21308::py_set_io_port4(int64_t v_value) {
    f_PORT4_OUT_LATCH = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_port5() {
    int64_t v_ext = 0;
    v_ext = ((~(f_PORT5[0])) & (f_PORT5[1] | ((((f_PUMOD0 & 8) > 0)) * 15)));
    return ((v_ext & (~(f_PM5))) | (f_PORT5_OUT_LATCH & f_PM5));
    return 0;
}

int64_t KS57C21308::py_set_io_port5(int64_t v_value) {
    f_PORT5_OUT_LATCH = v_value;
    return 0;
}

int64_t KS57C21308::py_get_io_port6() {
    int64_t v_ext = 0;
    v_ext = ((~(f_PORT6[0])) & (f_PORT6[1] | ((((f_PUMOD0 & 8) > 0)) * 15)));
    return ((v_ext & (~(f_PM6))) | (f_PORT6_OUT_LATCH & f_PM6));
    return 0;
}

int64_t KS57C21308::py_set_io_port6(int64_t v_value) {
    f_PORT6_OUT_LATCH = v_value;
    return 0;
}

void KS57C21308::pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level) {
    int64_t v_prev_port = 0;
    if (((v_port == kPort_PORT0))) {
        v_prev_port = py_get_io_port0();
        f_PORT0[0] &= (~(v_mask));
        f_PORT0[1] &= (~(v_mask));
        if (((v_level >= 0))) {
            f_PORT0[v_level] |= v_mask;
        }
        if (((((v_prev_port != py_get_io_port0()))) && (((v_level == 0))))) {
            f_INTP0 |= 1;
        }
    }
    if (((v_port == kPort_PORT1))) {
        v_prev_port = py_get_io_port1();
        f_PORT1[0] &= (~(v_mask));
        f_PORT1[1] &= (~(v_mask));
        if (((v_level >= 0))) {
            f_PORT1[v_level] |= v_mask;
        }
        if (((v_prev_port != py_get_io_port1()))) {
            if ((((v_mask & 2)) && (((f_IM1 != 3))) && ((((((f_IM1 & 1) != v_level))) || ((f_IM1 & 2)))))) {
                f_INT01 |= 4;
            }
            if ((((v_mask & 1)) && (((f_IM0 != 3))) && ((((((f_IM0 & 1) != v_level))) || ((f_IM0 & 2)))))) {
                f_INT01 |= 1;
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
            if (((((((v_level == 0))) && (((v_prev_port != py_get_io_port6()))))) && ((((f_PM6 & v_mask) == 0))) && (((((f_IM2 == 3))) || (((((f_IM2 == 2))) && ((((v_mask & 4)) || ((v_mask & 8)))))))))) {
                f_INT2 |= 1;
            }
        }
    }
}

void KS57C21308::pyupdate_cpu_clock_div() {
    if ((f_SCMOD & 1)) {
        f_cpu_clock_div = (f_sub_clock_div * 4);
    } else {
        f_cpu_clock_div = k_MAIN_CLOCK_DIV[f_PCON_CLOCK];
    }
}

void KS57C21308::py_process_timers(int64_t v_exec_cycles) {
    if ((!((f_SCMOD & 8)))) {
        f_basic_timer_counter -= v_exec_cycles;
        while (((f_basic_timer_counter <= 0))) {
            f_basic_timer_counter += k_BASIC_TIMER_DIV[f_BMOD];
            f_BCNT = ((f_BCNT + 1) & 255);
            if (((f_BCNT == 0))) {
                f_INT4B |= 1;
            }
        }
        if (((((f_TMOD0h >= 4))) && ((f_TMOD0l & 4)))) {
            f_T0_timer_counter -= v_exec_cycles;
            while (((f_T0_timer_counter <= 0))) {
                f_T0_timer_counter += k_TIMER_T0_DIV[f_TMOD0h];
                f_TCNT0 += 1;
                if (((f_TCNT0 > f_TREF0))) {
                    f_INTT0 |= 1;
                    f_TCNT0 = 0;
                    f_PORT4_TC0_OUT = ((((f_TOE0 > 0)) * 15) & (f_PORT4_TC0_OUT ^ 2));
                    snd_toggle(f_PORT4_TC0_OUT, 0);
                }
            }
        }
    }
    if ((f_WMODl & 4)) {
        f_watch_timer_counter -= v_exec_cycles;
        while (((f_watch_timer_counter <= 0))) {
            if ((f_WMODl & 1)) {
                f_watch_timer_counter += (k_WATCH_TIMER_DIV[(f_WMODl & 2)] * f_sub_clock_div);
            } else {
                f_watch_timer_counter += (k_WATCH_TIMER_DIV[(f_WMODl & 2)] * 128);
            }
            f_INTW |= 1;
        }
    }
}

void KS57C21308::py_go_vector(int64_t v_addr) {
    int64_t v_vector = 0;
    v_vector = rom_get_word(v_addr);
    f_EMB = ((v_vector >> 15) & 1);
    f_ERB = ((v_vector >> 14) & 1);
    f_PC = (v_vector & rom_get_mask());
}

void KS57C21308::py_interrupt(int64_t v_IRQn) {
    int64_t v_vector_addr = 0;
    v_vector_addr = (v_IRQn << 1);
    py_stack_push(((f_CY << 3) | f_SK));
    py_stack_push((((f_IST << 2) | (f_EMB << 1)) | f_ERB));
    py_stack_push(((f_PC >> 4) & 15));
    py_stack_push((f_PC & 15));
    py_stack_push((((f_EMB << 3) | (f_ERB << 2)) | ((f_PC >> 12) & 1)));
    py_stack_push(((f_PC >> 8) & 15));
    py_go_vector(v_vector_addr);
    f_IST += 1;
    if (((v_IRQn == 1))) {
        if ((((f_INT4B & 2) == 0))) {
            f_INT4B &= (~(4));
        } else {
            if ((((f_INT4B & 8) == 0))) {
                f_INT4B &= (~(1));
            }
        }
    } else {
        if (((v_IRQn == 5))) {
            f_INTT0 &= (~(1));
        } else {
            if (((v_IRQn == 4))) {
                f_INTP0 &= (~(1));
            } else {
                if (((v_IRQn == 2))) {
                    f_INT01 &= (~(1));
                } else {
                    if (((v_IRQn == 3))) {
                        f_INT01 &= (~(4));
                    }
                }
            }
        }
    }
}

int64_t KS57C21308::py_get_interrupt_vector() {
    int64_t v_IRQn = 0;
    v_IRQn = 0;
    if ((((f_INTT0 & 2)) && ((f_INTT0 & 1)))) {
        if (((((5 == f_IPR))) && (((f_IST <= 1))))) {
            return 5;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 5;
            }
        }
    }
    if ((((f_INTP0 & 2)) && ((f_INTP0 & 1)))) {
        if (((((4 == f_IPR))) && (((f_IST <= 1))))) {
            return 4;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 4;
            }
        }
    }
    if ((((f_INT01 & 8)) && ((f_INT01 & 4)))) {
        if (((((3 == f_IPR))) && (((f_IST <= 1))))) {
            return 3;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 3;
            }
        }
    }
    if ((((f_INT01 & 2)) && ((f_INT01 & 1)))) {
        if (((((2 == f_IPR))) && (((f_IST <= 1))))) {
            return 2;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 2;
            }
        }
    }
    if ((((f_INT4B & 8)) && ((f_INT4B & 4)))) {
        if (((((1 == f_IPR))) && (((f_IST <= 1))))) {
            return 1;
        } else {
            if (((f_IST == 0))) {
                v_IRQn = 1;
            }
        }
    }
    if ((((f_INT4B & 2)) && ((f_INT4B & 1)))) {
        if (((((1 == f_IPR))) && (((f_IST <= 1))))) {
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

double KS57C21308::pyclock() {
    int64_t v_IRQn = 0, v_byte = 0, v_bytes_count = 0, v_opcode = 0;
    double v_exec_cycles = 0;
    v_exec_cycles = f_cpu_clock_div;
    if (((f_PCON_MODE == 0))) {
        v_byte = rom_get_byte(f_PC);
        v_bytes_count = T_execute[v_byte].size;
        v_opcode = rom_get_bytes(f_PC, v_bytes_count);
        f_PC += v_bytes_count;
        v_exec_cycles *= (this->*T_execute[v_byte].fn)(v_opcode);
        f_instr_counter += 1;
    } else {
        if ((((((((((f_INTW & 1)) && ((f_INTW & 2))) | (((f_INT2 & 1)) && ((f_INT2 & 2)))) | (((f_INT01 & 1)) && ((f_INT01 & 2)) && ((f_IM0 & 4)))) | (((f_INT01 & 4)) && ((f_INT01 & 8)))) | (((f_INTT0 & 1)) && ((f_INTT0 & 2)))) | (((f_INT4B & 4)) && ((f_INT4B & 8)))) | (((f_INT4B & 1)) && ((f_INT4B & 2))))) {
            f_PCON_MODE = 0;
        }
    }
    if (f_IME) {
        v_IRQn = py_get_interrupt_vector();
        if (((v_IRQn > 0))) {
            py_interrupt(v_IRQn);
        }
    }
    py_process_timers(v_exec_cycles);
    return v_exec_cycles;
    return 0;
}

int64_t KS57C21308::py_get_mem(int64_t v_addr) {
    int64_t v_mb = 0;
    v_mb = 15;
    if (((((f_EMB != 0))) || (((v_addr < 128))))) {
        v_mb = (f_EMB * f_SMB);
    }
    if (((v_mb == 0))) {
        return f_RAM[v_addr];
    } else {
        if (((v_mb == 1))) {
            return f_RAM[(((f_PASR + 1) << 8) + v_addr)];
        } else {
            const IoEntry* v_io = find_io_tbl((3840 + v_addr));
            if (((v_io != nullptr))) {
                return (this->*v_io->get)();
            }
            return 0;
        }
    }
    return 0;
}

int64_t KS57C21308::py_get_mem_byte(int64_t v_addr) {
    int64_t v_mb = 0, v_result = 0;
    v_mb = 15;
    if (((((f_EMB != 0))) || (((v_addr < 128))))) {
        v_mb = (f_EMB * f_SMB);
    }
    if (((v_mb == 0))) {
        v_addr = (v_addr & 254);
        return ((f_RAM[(v_addr + 1)] << 4) | f_RAM[v_addr]);
    } else {
        if (((v_mb == 1))) {
            v_addr = (((f_PASR + 1) << 8) + v_addr);
            if (((f_PASR == 19))) {
                return f_RAM[v_addr];
            }
            return ((f_RAM[(v_addr | 1)] << 4) | f_RAM[(v_addr & 4094)]);
        } else {
            v_addr = (3840 + (v_addr & 254));
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
    }
    return 0;
}

void KS57C21308::py_set_mem(int64_t v_addr, int64_t v_value) {
    int64_t v_mb = 0;
    v_mb = 15;
    if (((((f_EMB != 0))) || (((v_addr < 128))))) {
        v_mb = (f_EMB * f_SMB);
    }
    if (((v_mb == 0))) {
        f_RAM[v_addr] = v_value;
    } else {
        if (((v_mb == 1))) {
            f_RAM[(((f_PASR + 1) << 8) + v_addr)] = v_value;
        } else {
            const IoEntry* v_io = find_io_tbl((3840 + v_addr));
            if (((v_io != nullptr))) {
                (this->*v_io->set)(v_value);
            }
        }
    }
}

void KS57C21308::py_set_mem_byte(int64_t v_addr, int64_t v_value) {
    int64_t v_mb = 0;
    v_mb = 15;
    if (((((f_EMB != 0))) || (((v_addr < 128))))) {
        v_mb = (f_EMB * f_SMB);
    }
    if (((v_mb == 0))) {
        v_addr &= 254;
        f_RAM[v_addr] = (v_value & 15);
        f_RAM[(v_addr + 1)] = ((v_value >> 4) & 15);
    } else {
        if (((v_mb == 1))) {
            v_addr = (((f_PASR + 1) << 8) + v_addr);
            if (((f_PASR == 19))) {
                f_RAM[v_addr] = (v_value & 31);
            } else {
                f_RAM[(v_addr & 4094)] = (v_value & 15);
                f_RAM[(v_addr | 1)] = ((v_value >> 4) & 15);
            }
        } else {
            v_addr = (3840 + (v_addr & 254));
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
}

int64_t KS57C21308::py_get_ahl() {
    int64_t v_mb = 0;
    v_mb = (f_EMB * f_SMB);
    if (((v_mb == 0))) {
        return f_RAM[py_get_rp(2)];
    } else {
        if (((v_mb == 1))) {
            return f_RAM[(((f_PASR + 1) << 8) + py_get_rp(2))];
        } else {
            const IoEntry* v_io = find_io_tbl((3840 + py_get_rp(2)));
            if (((v_io != nullptr))) {
                return (this->*v_io->get)();
            }
            return 0;
        }
    }
    return 0;
}

void KS57C21308::py_set_ahl(int64_t v_value) {
    int64_t v_mb = 0;
    v_mb = (f_EMB * f_SMB);
    if (((v_mb == 0))) {
        f_RAM[py_get_rp(2)] = v_value;
    } else {
        if (((v_mb == 1))) {
            f_RAM[(((f_PASR + 1) << 8) + py_get_rp(2))] = v_value;
        } else {
            const IoEntry* v_io = find_io_tbl((3840 + py_get_rp(2)));
            if (((v_io != nullptr))) {
                (this->*v_io->set)(v_value);
            }
        }
    }
}

int64_t KS57C21308::py_get_ahl_byte() {
    int64_t v_addr = 0, v_mb = 0, v_result = 0;
    v_mb = (f_EMB * f_SMB);
    if (((v_mb == 0))) {
        v_addr = (py_get_rp(2) & 254);
        return ((f_RAM[(v_addr + 1)] << 4) | f_RAM[v_addr]);
    } else {
        if (((v_mb == 1))) {
            v_addr = (((f_PASR + 1) << 8) + py_get_rp(2));
            if (((f_PASR == 19))) {
                return f_RAM[v_addr];
            }
            return ((f_RAM[(v_addr | 1)] << 4) | f_RAM[(v_addr & 4094)]);
        } else {
            v_addr = (3840 + (py_get_rp(2) & 254));
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
    }
    return 0;
}

void KS57C21308::py_set_ahl_byte(int64_t v_value) {
    int64_t v_addr = 0, v_mb = 0;
    v_mb = (f_EMB * f_SMB);
    if (((v_mb == 0))) {
        v_addr = (py_get_rp(2) & 254);
        f_RAM[v_addr] = (v_value & 15);
        f_RAM[(v_addr + 1)] = (v_value >> 4);
    } else {
        if (((v_mb == 1))) {
            v_addr = (((f_PASR + 1) << 8) + py_get_rp(2));
            if (((f_PASR == 19))) {
                f_RAM[v_addr] = (v_value & 31);
            } else {
                f_RAM[(v_addr & 4094)] = (v_value & 15);
                f_RAM[(v_addr | 1)] = ((v_value >> 4) & 15);
            }
        } else {
            v_addr = (3840 + (py_get_rp(2) & 254));
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
}

int64_t KS57C21308::py_get_hmem(int64_t v_opcode) {
    int64_t v_addr = 0, v_mb = 0;
    v_addr = ((py_get_reg(3) << 4) | (v_opcode & 15));
    v_mb = (f_EMB * f_SMB);
    if (((v_mb == 0))) {
        return f_RAM[v_addr];
    } else {
        if (((v_mb == 1))) {
            return f_RAM[(((f_PASR + 1) << 8) + v_addr)];
        } else {
            const IoEntry* v_io = find_io_tbl((3840 + v_addr));
            if (((v_io != nullptr))) {
                return (this->*v_io->get)();
            }
            return 0;
        }
    }
    return 0;
}

void KS57C21308::py_set_hmem(int64_t v_opcode, int64_t v_value) {
    int64_t v_addr = 0, v_mb = 0;
    v_addr = ((py_get_reg(3) << 4) | (v_opcode & 15));
    v_mb = (f_EMB * f_SMB);
    if (((v_mb == 0))) {
        f_RAM[v_addr] = v_value;
    } else {
        if (((v_mb == 1))) {
            f_RAM[(((f_PASR + 1) << 8) + v_addr)] = v_value;
        } else {
            const IoEntry* v_io = find_io_tbl((3840 + v_addr));
            if (((v_io != nullptr))) {
                (this->*v_io->set)(v_value);
            }
        }
    }
}

int64_t KS57C21308::py_get_memb_al(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((4032 + ((v_opcode & 15) << 2)) | (py_get_reg(2) >> 2));
    const IoEntry* v_io = find_io_tbl(v_addr);
    if (((v_io != nullptr))) {
        return (this->*v_io->get)();
    }
    return 0;
    return 0;
}

int64_t KS57C21308::py_set_memb_al(int64_t v_opcode, int64_t v_value) {
    int64_t v_addr = 0;
    v_addr = ((4032 + ((v_opcode & 15) << 2)) | (py_get_reg(2) >> 2));
    const IoEntry* v_io = find_io_tbl(v_addr);
    if (((v_io != nullptr))) {
        (this->*v_io->set)(v_value);
    }
    return 0;
}

int64_t KS57C21308::py_get_mema(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = (4016 + (v_opcode & 79));
    const IoEntry* v_io = find_io_tbl(v_addr);
    if (((v_io != nullptr))) {
        return (this->*v_io->get)();
    }
    return 0;
    return 0;
}

void KS57C21308::py_set_mema(int64_t v_opcode, int64_t v_value) {
    int64_t v_addr = 0;
    v_addr = (4016 + (v_opcode & 79));
    const IoEntry* v_io = find_io_tbl(v_addr);
    if (((v_io != nullptr))) {
        (this->*v_io->set)(v_value);
    }
}

int64_t KS57C21308::py_get_reg(int64_t v_reg) {
    return f_RAM[(((f_ERB * f_SRB) * 8) + v_reg)];
    return 0;
}

void KS57C21308::py_set_reg(int64_t v_reg, int64_t v_value) {
    f_RAM[(((f_ERB * f_SRB) * 8) + v_reg)] = v_value;
}

int64_t KS57C21308::py_get_rp(int64_t v_rp) {
    int64_t v_rp_offset = 0;
    v_rp_offset = (((f_ERB * f_SRB) * 8) + (v_rp & 6));
    return ((f_RAM[(v_rp_offset + 1)] << 4) | f_RAM[v_rp_offset]);
    return 0;
}

void KS57C21308::py_set_rp(int64_t v_rp, int64_t v_value) {
    int64_t v_rp_offset = 0;
    v_rp_offset = (((f_ERB * f_SRB) * 8) + (v_rp & 6));
    f_RAM[v_rp_offset] = (v_value & 15);
    f_RAM[(v_rp_offset + 1)] = ((v_value >> 4) & 15);
}

void KS57C21308::py_stack_push(int64_t v_value) {
    int64_t v_sp = 0;
    f_SP = ((f_SP - 1) & 255);
    v_sp = f_SP;
    f_RAM[v_sp] = v_value;
}

void KS57C21308::py_stack_push_byte(int64_t v_value) {
    int64_t v_sph = 0, v_spl = 0;
    f_SP = ((f_SP - 1) & 255);
    v_sph = f_SP;
    f_RAM[v_sph] = ((v_value >> 4) & 15);
    f_SP = ((f_SP - 1) & 255);
    v_spl = f_SP;
    f_RAM[v_spl] = (v_value & 15);
}

int64_t KS57C21308::py_stack_pop() {
    int64_t v_sp = 0;
    v_sp = f_SP;
    f_SP = ((f_SP + 1) & 255);
    return f_RAM[v_sp];
    return 0;
}

int64_t KS57C21308::py_stack_pop_byte() {
    int64_t v_sph = 0, v_spl = 0;
    v_spl = f_SP;
    v_sph = ((f_SP + 1) & 255);
    f_SP = ((f_SP + 2) & 255);
    return ((f_RAM[v_sph] << 4) | f_RAM[v_spl]);
    return 0;
}

int64_t KS57C21308::py_skip_next() {
    int64_t v_byte_count = 0, v_opcode = 0;
    v_opcode = rom_get_byte(f_PC);
    v_byte_count = T_execute[v_opcode].size;
    f_PC += v_byte_count;
    return ((((v_byte_count < 3))) ? 1 : 2);
    return 0;
}

int64_t KS57C21308::py_execute_11011011(int64_t v_opcode) {
    return (this->*T_execute_11011011_tbl[((v_opcode >> 14) & 1)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11011101(int64_t v_opcode) {
    return (this->*T_execute_11011101_tbl[(v_opcode & 127)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11011001(int64_t v_opcode) {
    return (this->*T_execute_11011001_tbl[((v_opcode >> 3) & 1)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11111100(int64_t v_opcode) {
    return (this->*T_execute_11111100_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11111110(int64_t v_opcode) {
    return (this->*T_execute_11111110_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11111111(int64_t v_opcode) {
    return (this->*T_execute_11111111_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11111101(int64_t v_opcode) {
    return (this->*T_execute_11111101_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11011100(int64_t v_opcode) {
    return (this->*T_execute_11011100_tbl[(v_opcode & 255)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11110101(int64_t v_opcode) {
    return (this->*T_execute_11110101_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11110110(int64_t v_opcode) {
    return (this->*T_execute_11110110_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11110111(int64_t v_opcode) {
    return (this->*T_execute_11110111_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11110100(int64_t v_opcode) {
    return (this->*T_execute_11110100_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11111000(int64_t v_opcode) {
    return (this->*T_execute_11111000_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_execute_11111001(int64_t v_opcode) {
    return (this->*T_execute_11111001_tbl[((v_opcode >> 6) & 3)].fn)(v_opcode);
    return 0;
}

int64_t KS57C21308::py_jr_raddr(int64_t v_opcode) {
    f_PC += (v_opcode & 15);
    return 2;
    return 0;
}

int64_t KS57C21308::py_jr_mraddr(int64_t v_opcode) {
    f_PC -= (16 - (v_opcode & 15));
    return 2;
    return 0;
}

int64_t KS57C21308::py_ref(int64_t v_opcode) {
    int64_t v_byte = 0, v_bytes_count = 0, v_execute_time = 0, v_taddr = 0;
    v_taddr = ((v_opcode & 240) | ((v_opcode << 1) & 14));
    v_byte = rom_get_byte(v_taddr);
    if ((((v_byte & 192) == 0))) {
        f_PC = (rom_get_word(v_taddr) & rom_get_mask());
        return 3;
    }
    if ((((v_byte & 192) == 64))) {
        py_stack_push(0);
        py_stack_push(((f_EMB << 1) | f_ERB));
        py_stack_push(((f_PC >> 4) & 15));
        py_stack_push((f_PC & 15));
        py_stack_push(((f_PC >> 12) & 3));
        py_stack_push(((f_PC >> 8) & 15));
        f_PC = (rom_get_word(v_taddr) & rom_get_mask());
        return 3;
    }
    v_bytes_count = T_execute[v_byte].size;
    v_execute_time = 1;
    if (((v_bytes_count == 1))) {
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

int64_t KS57C21308::py_calls_addr11(int64_t v_opcode) {
    py_stack_push(0);
    py_stack_push(((f_EMB << 1) | f_ERB));
    py_stack_push(((f_PC >> 4) & 15));
    py_stack_push((f_PC & 15));
    py_stack_push(((f_PC >> 12) & 3));
    py_stack_push(((f_PC >> 8) & 15));
    f_PC = (v_opcode & 2047);
    return 3;
    return 0;
}

int64_t KS57C21308::py_pop_rp(int64_t v_opcode) {
    py_set_rp((v_opcode & 6), py_stack_pop_byte());
    return 1;
    return 0;
}

int64_t KS57C21308::py_push_rp(int64_t v_opcode) {
    py_stack_push_byte(py_get_rp((v_opcode & 6)));
    return 1;
    return 0;
}

int64_t KS57C21308::py_jps_addr12(int64_t v_opcode) {
    f_PC = (((f_PC & 61440) | (v_opcode & 4095)) & rom_get_mask());
    return 2;
    return 0;
}

int64_t KS57C21308::py_ads_a_im(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = (py_get_reg(0) + (v_opcode & 15));
    py_set_reg(0, (v_new_A & 15));
    if (((v_new_A > 15))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS57C21308::py_ld_a_im(int64_t v_opcode) {
    int64_t v_cycle_count = 0;
    py_set_reg(0, (v_opcode & 15));
    v_cycle_count = 1;
    v_opcode = rom_get_byte(f_PC);
    while ((((((v_opcode >> 4) == 11))) || (((v_opcode == 129))))) {
        v_cycle_count += 1;
        f_PC += T_execute[v_opcode].size;
        v_opcode = rom_get_byte(f_PC);
    }
    return v_cycle_count;
    return 0;
}

int64_t KS57C21308::py_cpse_a_ahl(int64_t v_opcode) {
    if (((py_get_reg(0) == py_get_ahl()))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS57C21308::py_incs_da(int64_t v_opcode) {
    int64_t v_d = 0, v_new_da = 0;
    v_d = (v_opcode & 255);
    v_new_da = ((py_get_mem(v_d) + 1) & 15);
    py_set_mem(v_d, v_new_da);
    if (((v_new_da == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_bitr_da_bit(int64_t v_opcode) {
    int64_t v_a = 0, v_new_da = 0;
    v_a = (v_opcode & 255);
    v_new_da = (py_get_mem(v_a) & (~((1 << ((v_opcode >> 12) & 3)))));
    py_set_mem(v_a, v_new_da);
    return 2;
    return 0;
}

int64_t KS57C21308::py_bits_da_bit(int64_t v_opcode) {
    int64_t v_d = 0, v_new_da = 0;
    v_d = (v_opcode & 255);
    v_new_da = (py_get_mem(v_d) | (1 << ((v_opcode >> 12) & 3)));
    py_set_mem(v_d, v_new_da);
    return 2;
    return 0;
}

int64_t KS57C21308::py_btsf_da_bit(int64_t v_opcode) {
    if ((!((py_get_mem((v_opcode & 255)) & (1 << ((v_opcode >> 12) & 3)))))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_btst_da_bit(int64_t v_opcode) {
    if ((py_get_mem((v_opcode & 255)) & (1 << ((v_opcode >> 12) & 3)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_ea_imm(int64_t v_opcode) {
    int64_t v_cycle_count = 0;
    py_set_rp(0, (v_opcode & 255));
    v_cycle_count = 2;
    v_opcode = rom_get_byte(f_PC);
    while ((((((v_opcode >> 4) == 11))) || (((v_opcode == 129))))) {
        v_cycle_count += 1;
        f_PC += T_execute[v_opcode].size;
        v_opcode = rom_get_byte(f_PC);
    }
    return v_cycle_count;
    return 0;
}

int64_t KS57C21308::py_incs_rrb(int64_t v_opcode) {
    int64_t v_new_rp = 0;
    v_new_rp = ((py_get_rp((v_opcode & 6)) + 1) & 255);
    py_set_rp((v_opcode & 6), v_new_rp);
    if (((v_new_rp == 0))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS57C21308::py_ld_hl_imm(int64_t v_opcode) {
    int64_t v_cycle_count = 0;
    py_set_rp(2, (v_opcode & 255));
    v_cycle_count = 2;
    v_opcode = rom_get_byte(f_PC);
    while (((v_opcode == 131))) {
        v_cycle_count += 1;
        f_PC += T_execute[v_opcode].size;
        v_opcode = rom_get_byte(f_PC);
    }
    return v_cycle_count;
    return 0;
}

int64_t KS57C21308::py_ld_wx_imm(int64_t v_opcode) {
    py_set_rp(4, (v_opcode & 255));
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_yz_imm(int64_t v_opcode) {
    py_set_rp(6, (v_opcode & 255));
    return 2;
    return 0;
}

int64_t KS57C21308::py_and_a_ahl(int64_t v_opcode) {
    py_set_reg(0, (py_get_reg(0) & py_get_ahl()));
    return 1;
    return 0;
}

int64_t KS57C21308::py_ld_da_ea(int64_t v_opcode) {
    int64_t v_d = 0;
    v_d = (v_opcode & 254);
    py_set_mem_byte((v_opcode & 255), py_get_rp(0));
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_da_a(int64_t v_opcode) {
    int64_t v_d = 0;
    v_d = (v_opcode & 255);
    py_set_mem(v_d, py_get_reg(0));
    return 2;
    return 0;
}

int64_t KS57C21308::py_rrc_a(int64_t v_opcode) {
    int64_t v_new_CY = 0;
    v_new_CY = (py_get_reg(0) & 1);
    py_set_reg(0, ((py_get_reg(0) >> 1) | (f_CY << 3)));
    f_CY = v_new_CY;
    return 1;
    return 0;
}

int64_t KS57C21308::py_or_a_ahl(int64_t v_opcode) {
    py_set_reg(0, (py_get_reg(0) | py_get_ahl()));
    return 1;
    return 0;
}

int64_t KS57C21308::py_ld_ea_da(int64_t v_opcode) {
    int64_t v_d = 0;
    v_d = (v_opcode & 254);
    py_set_rp(0, py_get_mem_byte(v_d));
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_a_da(int64_t v_opcode) {
    int64_t v_d = 0;
    v_d = (v_opcode & 255);
    py_set_reg(0, py_get_mem(v_d));
    return 2;
    return 0;
}

int64_t KS57C21308::py_sbs_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = (py_get_reg(0) - py_get_ahl());
    py_set_reg(0, (v_new_A & 15));
    if (((v_new_A < 0))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS57C21308::py_adc_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = ((py_get_reg(0) + py_get_ahl()) + f_CY);
    py_set_reg(0, (v_new_A & 15));
    f_CY = ((v_new_A > 15));
    v_opcode = rom_get_byte(f_PC);
    if ((((v_opcode >> 4) == 10))) {
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

int64_t KS57C21308::py_jp_addr14(int64_t v_opcode) {
    f_PC = (v_opcode & rom_get_mask());
    return 1;
    return 0;
}

int64_t KS57C21308::py_call_addr14(int64_t v_opcode) {
    py_stack_push(0);
    py_stack_push(((f_EMB << 1) | f_ERB));
    py_stack_push(((f_PC >> 4) & 15));
    py_stack_push((f_PC & 15));
    py_stack_push(((f_PC >> 12) & 3));
    py_stack_push(((f_PC >> 8) & 15));
    f_PC = (v_opcode & rom_get_mask());
    return 4;
    return 0;
}

int64_t KS57C21308::py_xor_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = (py_get_reg(0) ^ py_get_ahl());
    py_set_reg(0, v_new_A);
    return 1;
    return 0;
}

int64_t KS57C21308::py_xch_ea_da(int64_t v_opcode) {
    int64_t v_buf_ea = 0, v_d = 0;
    v_d = (v_opcode & 255);
    v_buf_ea = py_get_rp(0);
    py_set_rp(0, py_get_mem_byte(v_d));
    py_set_mem_byte(v_d, v_buf_ea);
    return 2;
    return 0;
}

int64_t KS57C21308::py_xch_a_da(int64_t v_opcode) {
    int64_t v_a = 0, v_buf_A = 0;
    v_a = (v_opcode & 255);
    v_buf_A = py_get_reg(0);
    py_set_reg(0, py_get_mem(v_a));
    py_set_mem(v_a, v_buf_A);
    return 2;
    return 0;
}

int64_t KS57C21308::py_sbc_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = ((py_get_reg(0) - py_get_ahl()) - f_CY);
    py_set_reg(0, (v_new_A & 15));
    f_CY = ((v_new_A < 0));
    v_opcode = rom_get_byte(f_PC);
    if ((((v_opcode >> 4) == 10))) {
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

int64_t KS57C21308::py_ads_ea_imm(int64_t v_opcode) {
    int64_t v_new_EA = 0;
    v_new_EA = (py_get_rp(0) + (v_opcode & 255));
    py_set_rp(0, (v_new_EA & 255));
    if (((v_new_EA > 255))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_incs_r(int64_t v_opcode) {
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

int64_t KS57C21308::py_decs_r(int64_t v_opcode) {
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

int64_t KS57C21308::py_ldc_ea_aea(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((f_PC & 65280) | py_get_rp(0));
    py_set_rp(0, rom_get_byte(v_addr));
    return 3;
    return 0;
}

int64_t KS57C21308::py_ads_a_ahl(int64_t v_opcode) {
    int64_t v_new_A = 0;
    v_new_A = (py_get_reg(0) + py_get_ahl());
    py_set_reg(0, (v_new_A & 15));
    if (((v_new_A > 15))) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS57C21308::py_ldc_ea_awx(int64_t v_opcode) {
    int64_t v_addr = 0;
    v_addr = ((f_PC & 65280) | py_get_rp(4));
    py_set_rp(0, rom_get_byte(v_addr));
    return 3;
    return 0;
}

int64_t KS57C21308::py_ccf(int64_t v_opcode) {
    f_CY = (!(f_CY));
    return 1;
    return 0;
}

int64_t KS57C21308::py_btst_cy(int64_t v_opcode) {
    if (f_CY) {
        return (1 + py_skip_next());
    }
    return 1;
    return 0;
}

int64_t KS57C21308::py_xch_a_ra(int64_t v_opcode) {
    int64_t v_buf_A = 0, v_reg = 0;
    v_reg = (v_opcode & 7);
    v_buf_A = py_get_reg(0);
    py_set_reg(0, py_get_reg(v_reg));
    py_set_reg(v_reg, v_buf_A);
    return 1;
    return 0;
}

int64_t KS57C21308::py_sret(int64_t v_opcode) {
    int64_t v_stack_value = 0;
    f_PC = (py_stack_pop() << 8);
    f_PC |= ((py_stack_pop() << 12) & 4096);
    f_PC |= py_stack_pop();
    f_PC |= (py_stack_pop() << 4);
    v_stack_value = py_stack_pop();
    f_EMB = ((v_stack_value >> 1) & 1);
    f_ERB = (v_stack_value & 1);
    py_stack_pop();
    f_PC &= rom_get_mask();
    return (3 + py_skip_next());
    return 0;
}

int64_t KS57C21308::py_ld_a_ahl(int64_t v_opcode) {
    py_set_reg(0, py_get_ahl());
    return 1;
    return 0;
}

int64_t KS57C21308::py_ldi_a_ahl(int64_t v_opcode) {
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

int64_t KS57C21308::py_ldd_a_ahl(int64_t v_opcode) {
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

int64_t KS57C21308::py_ld_a_awx(int64_t v_opcode) {
    py_set_reg(0, f_RAM[py_get_rp(4)]);
    return 1;
    return 0;
}

int64_t KS57C21308::py_ld_a_awl(int64_t v_opcode) {
    int64_t v_wl = 0;
    v_wl = ((py_get_reg(5) << 4) | py_get_reg(2));
    py_set_reg(0, f_RAM[v_wl]);
    return 1;
    return 0;
}

int64_t KS57C21308::py_bitr_cy(int64_t v_opcode) {
    f_CY = 0;
    return 1;
    return 0;
}

int64_t KS57C21308::py_bits_cy(int64_t v_opcode) {
    f_CY = 1;
    return 1;
    return 0;
}

int64_t KS57C21308::py_ld_ahl_a(int64_t v_opcode) {
    py_set_ahl(py_get_reg(0));
    return 1;
    return 0;
}

int64_t KS57C21308::py_xch_a_ahl(int64_t v_opcode) {
    int64_t v_buf_A = 0;
    v_buf_A = py_get_reg(0);
    py_set_reg(0, py_get_ahl());
    py_set_ahl(v_buf_A);
    return 1;
    return 0;
}

int64_t KS57C21308::py_xch_a_ahli(int64_t v_opcode) {
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

int64_t KS57C21308::py_xch_a_ahld(int64_t v_opcode) {
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

int64_t KS57C21308::py_xch_a_awx(int64_t v_opcode) {
    int64_t v_buf_A = 0;
    v_buf_A = py_get_reg(0);
    py_set_reg(0, f_RAM[py_get_rp(4)]);
    f_RAM[py_get_rp(4)] = v_buf_A;
    return 1;
    return 0;
}

int64_t KS57C21308::py_xch_a_awl(int64_t v_opcode) {
    int64_t v_buf_A = 0, v_wl = 0;
    v_buf_A = py_get_reg(0);
    v_wl = ((py_get_reg(5) << 4) | py_get_reg(2));
    py_set_reg(0, f_RAM[v_wl]);
    f_RAM[v_wl] = v_buf_A;
    return 1;
    return 0;
}

int64_t KS57C21308::py_ret(int64_t v_opcode) {
    int64_t v_stack_value = 0;
    f_PC = (py_stack_pop() << 8);
    f_PC |= ((py_stack_pop() << 12) & 4096);
    f_PC |= py_stack_pop();
    f_PC |= (py_stack_pop() << 4);
    v_stack_value = py_stack_pop();
    f_EMB = ((v_stack_value >> 1) & 1);
    f_ERB = (v_stack_value & 1);
    py_stack_pop();
    f_PC &= rom_get_mask();
    return 3;
    return 0;
}

int64_t KS57C21308::py_iret(int64_t v_opcode) {
    int64_t v_stack_value = 0;
    f_PC = (py_stack_pop() << 8);
    v_stack_value = py_stack_pop();
    f_PC |= ((v_stack_value << 12) & 4096);
    f_EMB = (v_stack_value >> 3);
    f_ERB = ((v_stack_value >> 2) & 1);
    f_PC |= py_stack_pop();
    f_PC |= (py_stack_pop() << 4);
    v_stack_value = py_stack_pop();
    f_IST = (v_stack_value >> 2);
    f_EMB = ((v_stack_value >> 1) & 1);
    f_ERB = (v_stack_value & 1);
    v_stack_value = py_stack_pop();
    f_CY = ((v_stack_value >> 3) & 1);
    f_SK = (v_stack_value & 7);
    f_PC &= rom_get_mask();
    return 3;
    return 0;
}

int64_t KS57C21308::py_jr_aea(int64_t v_opcode) {
    f_PC = ((((f_PC & 65280) | (py_get_reg(1) << 4)) | py_get_reg(0)) & rom_get_mask());
    return 3;
    return 0;
}

int64_t KS57C21308::py_incs_ahl(int64_t v_opcode) {
    int64_t v_new_ahl = 0;
    v_new_ahl = ((py_get_ahl() + 1) & 15);
    py_set_ahl(v_new_ahl);
    if (((v_new_ahl == 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_jr_awx(int64_t v_opcode) {
    f_PC = ((((f_PC & 65280) | (py_get_reg(5) << 4)) | py_get_reg(4)) & rom_get_mask());
    return 3;
    return 0;
}

int64_t KS57C21308::py_pop_sb(int64_t v_opcode) {
    f_SRB = py_stack_pop();
    f_SMB = py_stack_pop();
    return 2;
    return 0;
}

int64_t KS57C21308::py_push_sb(int64_t v_opcode) {
    py_stack_push(f_SMB);
    py_stack_push(f_SRB);
    return 2;
    return 0;
}

int64_t KS57C21308::py_cpse_a_r(int64_t v_opcode) {
    if (((py_get_reg(0) == py_get_reg((v_opcode & 7))))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_smb_n(int64_t v_opcode) {
    f_SMB = (v_opcode & 15);
    return 2;
    return 0;
}

int64_t KS57C21308::py_srb_n(int64_t v_opcode) {
    f_SRB = (v_opcode & 3);
    return 2;
    return 0;
}

int64_t KS57C21308::py_and_a_im(int64_t v_opcode) {
    py_set_reg(0, (py_get_reg(0) & v_opcode));
    return 2;
    return 0;
}

int64_t KS57C21308::py_or_a_im(int64_t v_opcode) {
    py_set_reg(0, ((py_get_reg(0) | v_opcode) & 15));
    return 2;
    return 0;
}

int64_t KS57C21308::py_xor_a_im(int64_t v_opcode) {
    py_set_reg(0, ((py_get_reg(0) ^ v_opcode) & 15));
    return 2;
    return 0;
}

int64_t KS57C21308::py_cpse_ahl_im(int64_t v_opcode) {
    if (((py_get_ahl() == (v_opcode & 15)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_ra_a(int64_t v_opcode) {
    py_set_reg((v_opcode & 7), py_get_reg(0));
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_a_r(int64_t v_opcode) {
    py_set_reg(0, py_get_reg((v_opcode & 7)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_cpse_r_im(int64_t v_opcode) {
    if (((py_get_reg((v_opcode & 7)) == ((v_opcode >> 4) & 15)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_ra_im(int64_t v_opcode) {
    py_set_reg((v_opcode & 7), ((v_opcode >> 4) & 15));
    return 2;
    return 0;
}

int64_t KS57C21308::py_ldb_ahda_bit_cy(int64_t v_opcode) {
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

int64_t KS57C21308::py_ldb_memb_al_cy(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    if (f_CY) {
        py_set_memb_al(v_opcode, (py_get_memb_al(v_opcode) | (1 << v_b)));
    } else {
        py_set_memb_al(v_opcode, (py_get_memb_al(v_opcode) & (~((1 << v_b)))));
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_ldb_mema_bit_cy(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if (f_CY) {
        py_set_mema(v_opcode, (py_get_mema(v_opcode) | (1 << v_b)));
    } else {
        py_set_mema(v_opcode, (py_get_mema(v_opcode) & (~((1 << v_b)))));
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_bitr_ahda_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    py_set_hmem(v_opcode, (py_get_hmem(v_opcode) & (~((1 << v_b)))));
    return 2;
    return 0;
}

int64_t KS57C21308::py_bitr_memb_al(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    py_set_memb_al(v_opcode, (py_get_memb_al(v_opcode) & (~((1 << v_b)))));
    return 2;
    return 0;
}

int64_t KS57C21308::py_bitr_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    py_set_mema(v_opcode, (py_get_mema(v_opcode) & (~((1 << v_b)))));
    return 2;
    return 0;
}

int64_t KS57C21308::py_bits_ahda_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    py_set_hmem(v_opcode, (py_get_hmem(v_opcode) | (1 << v_b)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_bits_memb_al(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    py_set_memb_al(v_opcode, (py_get_memb_al(v_opcode) | (1 << v_b)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_bits_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    py_set_mema(v_opcode, (py_get_mema(v_opcode) | (1 << v_b)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_btstz_ahda_bit(int64_t v_opcode) {
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

int64_t KS57C21308::py_btstz_memb_al(int64_t v_opcode) {
    int64_t v_b = 0, v_mem = 0;
    v_b = (py_get_reg(2) & 3);
    v_mem = py_set_memb_al(v_opcode, 0);
    if ((v_mem & (1 << v_b))) {
        py_set_memb_al(v_opcode, (v_mem & (~((1 << v_b)))));
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_btstz_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0, v_mem = 0;
    v_b = ((v_opcode >> 4) & 3);
    v_mem = py_get_mema(v_opcode);
    if ((v_mem & (1 << v_b))) {
        py_set_mema(v_opcode, (v_mem & (~((1 << v_b)))));
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_ahl_ea(int64_t v_opcode) {
    py_set_ahl_byte(py_get_rp(0));
    return 2;
    return 0;
}

int64_t KS57C21308::py_xch_ea_ahl(int64_t v_opcode) {
    int64_t v_xa = 0;
    v_xa = py_get_rp(0);
    py_set_rp(0, py_get_ahl_byte());
    py_set_ahl_byte(v_xa);
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_ea_ahl(int64_t v_opcode) {
    py_set_rp(0, py_get_ahl_byte());
    return 2;
    return 0;
}

int64_t KS57C21308::py_cpse_ea_ahl(int64_t v_opcode) {
    if (((py_get_rp(0) == py_get_ahl_byte()))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_xch_ea_rrb(int64_t v_opcode) {
    int64_t v_xa = 0;
    v_xa = py_get_rp(0);
    py_set_rp(0, py_get_rp((v_opcode & 6)));
    py_set_rp((v_opcode & 6), v_xa);
    return 2;
    return 0;
}

int64_t KS57C21308::py_cpse_ea_rr(int64_t v_opcode) {
    if (((py_get_rp(0) == py_get_rp((v_opcode & 6))))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_rrb_ea(int64_t v_opcode) {
    py_set_rp((v_opcode & 6), py_get_rp(0));
    return 2;
    return 0;
}

int64_t KS57C21308::py_ld_ea_rrb(int64_t v_opcode) {
    py_set_rp(0, py_get_rp((v_opcode & 6)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_decs_rr(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp((v_opcode & 6)) - 1) & 255);
    py_set_rp((v_opcode & 6), v_new_value);
    if (((v_new_value == 255))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_and_rrb_ea(int64_t v_opcode) {
    py_set_rp((v_opcode & 6), (py_get_rp((v_opcode & 6)) & py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_and_ea_rr(int64_t v_opcode) {
    py_set_rp(0, (py_get_rp((v_opcode & 6)) & py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_or_rrb_ea(int64_t v_opcode) {
    py_set_rp((v_opcode & 6), (py_get_rp((v_opcode & 6)) | py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_or_ea_rr(int64_t v_opcode) {
    py_set_rp(0, (py_get_rp((v_opcode & 6)) | py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_xor_rrb_ea(int64_t v_opcode) {
    py_set_rp((v_opcode & 6), (py_get_rp((v_opcode & 6)) ^ py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_xor_ea_rr(int64_t v_opcode) {
    py_set_rp(0, (py_get_rp((v_opcode & 6)) ^ py_get_rp(0)));
    return 2;
    return 0;
}

int64_t KS57C21308::py_ads_rrb_ea(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = (py_get_rp((v_opcode & 6)) + py_get_rp(0));
    py_set_rp((v_opcode & 6), (v_new_value & 255));
    if (((v_new_value > 255))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_ads_ea_rr(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = (py_get_rp((v_opcode & 6)) + py_get_rp(0));
    py_set_rp(0, (v_new_value & 255));
    if (((v_new_value > 255))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_adc_rrb_ea(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp((v_opcode & 6)) + py_get_rp(0)) + f_CY);
    py_set_rp((v_opcode & 6), (v_new_value & 255));
    f_CY = ((v_new_value > 255));
    return 2;
    return 0;
}

int64_t KS57C21308::py_adc_ea_rr(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp((v_opcode & 6)) + py_get_rp(0)) + f_CY);
    py_set_rp(0, (v_new_value & 255));
    f_CY = ((v_new_value > 255));
    return 2;
    return 0;
}

int64_t KS57C21308::py_sbs_rrb_ea(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = (py_get_rp((v_opcode & 6)) - py_get_rp(0));
    py_set_rp((v_opcode & 6), (v_new_value & 255));
    if (((v_new_value < 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_sbs_ea_rr(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = (py_get_rp(0) - py_get_rp((v_opcode & 6)));
    py_set_rp(0, (v_new_value & 255));
    if (((v_new_value < 0))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_sbc_rrb_ea(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp((v_opcode & 6)) - py_get_rp(0)) - f_CY);
    py_set_rp((v_opcode & 6), (v_new_value & 255));
    f_CY = ((v_new_value < 0));
    return 2;
    return 0;
}

int64_t KS57C21308::py_sbc_ea_rr(int64_t v_opcode) {
    int64_t v_new_value = 0;
    v_new_value = ((py_get_rp(0) - py_get_rp((v_opcode & 6))) - f_CY);
    py_set_rp(0, (v_new_value & 255));
    f_CY = ((v_new_value < 0));
    return 2;
    return 0;
}

int64_t KS57C21308::py_band_cy_ahda_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY &= ((py_get_hmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_band_cy_memb_al(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    f_CY &= ((py_get_memb_al(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_band_cy_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY &= ((py_get_mema(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_bor_cy_ahda_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY |= ((py_get_hmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_bor_cy_memb_al(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    f_CY |= ((py_get_memb_al(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_bor_cy_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY |= ((py_get_mema(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_bxor_cy_ahda_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY ^= ((py_get_hmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_bxor_cy_memb_al(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    f_CY ^= ((py_get_memb_al(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_bxor_cy_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY ^= ((py_get_mema(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_ldb_cy_ahda_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY = ((py_get_hmem(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_ldb_cy_memb_al(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    f_CY = ((py_get_memb_al(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_ldb_cy_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    f_CY = ((py_get_mema(v_opcode) >> v_b) & 1);
    return 2;
    return 0;
}

int64_t KS57C21308::py_btsf_ahda_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if ((!(((py_get_hmem(v_opcode) >> v_b) & 1)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_btsf_memb_al(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    if ((!(((py_get_memb_al(v_opcode) >> v_b) & 1)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_btsf_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if ((!(((py_get_mema(v_opcode) >> v_b) & 1)))) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_btst_ahda_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if (((py_get_hmem(v_opcode) >> v_b) & 1)) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_btst_memb_al(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = (py_get_reg(2) & 3);
    if (((py_get_memb_al(v_opcode) >> v_b) & 1)) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_btst_mema_bit(int64_t v_opcode) {
    int64_t v_b = 0;
    v_b = ((v_opcode >> 4) & 3);
    if (((py_get_mema(v_opcode) >> v_b) & 1)) {
        return (2 + py_skip_next());
    }
    return 2;
    return 0;
}

int64_t KS57C21308::py_dummy(int64_t v_opcode) {
    (void)0;
    return 0;
    return 0;
}

const KS57C21308::IoEntry* KS57C21308::find_io_tbl(int64_t addr) {
    static const IoEntry e[] = {
        {&KS57C21308::py_get_io_spl, &KS57C21308::py_set_io_spl},
        {&KS57C21308::py_get_io_sph, &KS57C21308::py_set_io_sph},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_bmod},
        {&KS57C21308::py_get_io_bcntl, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_bcnth, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_wmodl},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_wmodh},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_lmod0},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_lmod1},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_tmod0l},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_tmod0h},
        {&KS57C21308::py_get_io_toe0, &KS57C21308::py_set_io_toe0},
        {&KS57C21308::py_get_io_tcnt0l, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_tcnt0h, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_tref0l},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_tref0h},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_pasrl},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_pasrh},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_pswl, &KS57C21308::py_set_io_pswl},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_pswh},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_ipr},
        {&KS57C21308::py_get_io_pcon, &KS57C21308::py_set_io_pcon},
        {&KS57C21308::py_get_io_im0, &KS57C21308::py_set_io_im0},
        {&KS57C21308::py_get_io_im1, &KS57C21308::py_set_io_im1},
        {&KS57C21308::py_get_io_im2, &KS57C21308::py_set_io_im2},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_scmod},
        {&KS57C21308::py_get_io_int4b, &KS57C21308::py_set_io_int4b},
        {&KS57C21308::py_get_io_intw, &KS57C21308::py_set_io_intw},
        {&KS57C21308::py_get_io_intt0, &KS57C21308::py_set_io_intt0},
        {&KS57C21308::py_get_io_intp0, &KS57C21308::py_set_io_intp0},
        {&KS57C21308::py_get_io_int01, &KS57C21308::py_set_io_int01},
        {&KS57C21308::py_get_io_int2, &KS57C21308::py_set_io_int2},
        {&KS57C21308::py_get_io_bsc0, &KS57C21308::py_set_io_bsc0},
        {&KS57C21308::py_get_io_bsc1, &KS57C21308::py_set_io_bsc1},
        {&KS57C21308::py_get_io_bsc2, &KS57C21308::py_set_io_bsc2},
        {&KS57C21308::py_get_io_bsc3, &KS57C21308::py_set_io_bsc3},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_clmod},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_pumodl},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_pumodh},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_pmg1l},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_pmg1h},
        {&KS57C21308::py_get_io_dummy, &KS57C21308::py_set_io_pmg2},
        {&KS57C21308::py_get_io_port0, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_port1, &KS57C21308::py_set_io_dummy},
        {&KS57C21308::py_get_io_port2, &KS57C21308::py_set_io_port2},
        {&KS57C21308::py_get_io_port4, &KS57C21308::py_set_io_port4},
        {&KS57C21308::py_get_io_port5, &KS57C21308::py_set_io_port5},
        {&KS57C21308::py_get_io_port6, &KS57C21308::py_set_io_port6},
    };
    switch (addr) {
        case 3968: return &e[0];
        case 3969: return &e[1];
        case 3973: return &e[2];
        case 3974: return &e[3];
        case 3975: return &e[4];
        case 3976: return &e[5];
        case 3977: return &e[6];
        case 3980: return &e[7];
        case 3981: return &e[8];
        case 3984: return &e[9];
        case 3985: return &e[10];
        case 3986: return &e[11];
        case 3988: return &e[12];
        case 3989: return &e[13];
        case 3990: return &e[14];
        case 3991: return &e[15];
        case 4000: return &e[16];
        case 4001: return &e[17];
        case 4002: return &e[18];
        case 4003: return &e[19];
        case 4004: return &e[20];
        case 4005: return &e[21];
        case 4016: return &e[22];
        case 4017: return &e[23];
        case 4018: return &e[24];
        case 4019: return &e[25];
        case 4020: return &e[26];
        case 4021: return &e[27];
        case 4022: return &e[28];
        case 4023: return &e[29];
        case 4024: return &e[30];
        case 4026: return &e[31];
        case 4028: return &e[32];
        case 4029: return &e[33];
        case 4030: return &e[34];
        case 4031: return &e[35];
        case 4032: return &e[36];
        case 4033: return &e[37];
        case 4034: return &e[38];
        case 4035: return &e[39];
        case 4048: return &e[40];
        case 4058: return &e[41];
        case 4059: return &e[42];
        case 4060: return &e[43];
        case 4061: return &e[44];
        case 4072: return &e[45];
        case 4073: return &e[46];
        case 4074: return &e[47];
        case 4080: return &e[48];
        case 4081: return &e[49];
        case 4082: return &e[50];
        case 4084: return &e[51];
        case 4085: return &e[52];
        case 4086: return &e[53];
        default: return nullptr;
    }
}

const KS57C21308::OpEntry KS57C21308::T_execute[256] = {
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_mraddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_jr_raddr, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_pop_rp, 1},
    {&KS57C21308::py_push_rp, 1},
    {&KS57C21308::py_pop_rp, 1},
    {&KS57C21308::py_push_rp, 1},
    {&KS57C21308::py_pop_rp, 1},
    {&KS57C21308::py_push_rp, 1},
    {&KS57C21308::py_pop_rp, 1},
    {&KS57C21308::py_push_rp, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_cpse_a_ahl, 1},
    {&KS57C21308::py_and_a_ahl, 1},
    {&KS57C21308::py_or_a_ahl, 1},
    {&KS57C21308::py_xor_a_ahl, 1},
    {&KS57C21308::py_sbc_a_ahl, 1},
    {&KS57C21308::py_sbs_a_ahl, 1},
    {&KS57C21308::py_adc_a_ahl, 1},
    {&KS57C21308::py_ads_a_ahl, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_decs_r, 1},
    {&KS57C21308::py_decs_r, 1},
    {&KS57C21308::py_decs_r, 1},
    {&KS57C21308::py_decs_r, 1},
    {&KS57C21308::py_decs_r, 1},
    {&KS57C21308::py_decs_r, 1},
    {&KS57C21308::py_decs_r, 1},
    {&KS57C21308::py_decs_r, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_incs_r, 1},
    {&KS57C21308::py_incs_r, 1},
    {&KS57C21308::py_incs_r, 1},
    {&KS57C21308::py_incs_r, 1},
    {&KS57C21308::py_incs_r, 1},
    {&KS57C21308::py_incs_r, 1},
    {&KS57C21308::py_incs_r, 1},
    {&KS57C21308::py_incs_r, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_xch_a_ra, 1},
    {&KS57C21308::py_xch_a_ra, 1},
    {&KS57C21308::py_xch_a_ra, 1},
    {&KS57C21308::py_xch_a_ra, 1},
    {&KS57C21308::py_xch_a_ra, 1},
    {&KS57C21308::py_xch_a_ra, 1},
    {&KS57C21308::py_xch_a_ra, 1},
    {&KS57C21308::py_xch_a_ra, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_ref, 1},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_xch_a_da, 2},
    {&KS57C21308::py_xch_a_ahli, 1},
    {&KS57C21308::py_xch_a_ahld, 1},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_xch_a_ahl, 1},
    {&KS57C21308::py_xch_a_awx, 1},
    {&KS57C21308::py_xch_a_awl, 1},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_ld_ea_imm, 2},
    {&KS57C21308::py_incs_rrb, 1},
    {&KS57C21308::py_ld_hl_imm, 2},
    {&KS57C21308::py_incs_rrb, 1},
    {&KS57C21308::py_ld_wx_imm, 2},
    {&KS57C21308::py_incs_rrb, 1},
    {&KS57C21308::py_ld_yz_imm, 2},
    {&KS57C21308::py_rrc_a, 1},
    {&KS57C21308::py_ld_da_a, 2},
    {&KS57C21308::py_ldi_a_ahl, 1},
    {&KS57C21308::py_ldd_a_ahl, 1},
    {&KS57C21308::py_ld_a_da, 2},
    {&KS57C21308::py_ld_a_ahl, 1},
    {&KS57C21308::py_ld_a_awx, 1},
    {&KS57C21308::py_ld_a_awl, 1},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_jps_addr12, 2},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ads_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_ld_a_im, 1},
    {&KS57C21308::py_bitr_da_bit, 2},
    {&KS57C21308::py_bits_da_bit, 2},
    {&KS57C21308::py_btsf_da_bit, 2},
    {&KS57C21308::py_btst_da_bit, 2},
    {&KS57C21308::py_ld_ahl_a, 1},
    {&KS57C21308::py_ret, 1},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_ldc_ea_aea, 1},
    {&KS57C21308::py_ads_ea_imm, 2},
    {&KS57C21308::py_incs_da, 2},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_ldc_ea_awx, 1},
    {&KS57C21308::py_ld_da_ea, 2},
    {&KS57C21308::py_ld_ea_da, 2},
    {&KS57C21308::py_xch_ea_da, 2},
    {&KS57C21308::py_bitr_da_bit, 2},
    {&KS57C21308::py_bits_da_bit, 2},
    {&KS57C21308::py_btsf_da_bit, 2},
    {&KS57C21308::py_btst_da_bit, 2},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_iret, 1},
    {&KS57C21308::py_ccf, 1},
    {&KS57C21308::py_btst_cy, 1},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_execute_11011001, 2},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_execute_11011011, 3},
    {&KS57C21308::py_execute_11011100, 2},
    {&KS57C21308::py_execute_11011101, 2},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_bitr_da_bit, 2},
    {&KS57C21308::py_bits_da_bit, 2},
    {&KS57C21308::py_btsf_da_bit, 2},
    {&KS57C21308::py_btst_da_bit, 2},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_sret, 1},
    {&KS57C21308::py_bitr_cy, 1},
    {&KS57C21308::py_bits_cy, 1},
    {&KS57C21308::py_calls_addr11, 2},
    {&KS57C21308::py_calls_addr11, 2},
    {&KS57C21308::py_calls_addr11, 2},
    {&KS57C21308::py_calls_addr11, 2},
    {&KS57C21308::py_calls_addr11, 2},
    {&KS57C21308::py_calls_addr11, 2},
    {&KS57C21308::py_calls_addr11, 2},
    {&KS57C21308::py_calls_addr11, 2},
    {&KS57C21308::py_bitr_da_bit, 2},
    {&KS57C21308::py_bits_da_bit, 2},
    {&KS57C21308::py_btsf_da_bit, 2},
    {&KS57C21308::py_btst_da_bit, 2},
    {&KS57C21308::py_execute_11110100, 2},
    {&KS57C21308::py_execute_11110101, 2},
    {&KS57C21308::py_execute_11110110, 2},
    {&KS57C21308::py_execute_11110111, 2},
    {&KS57C21308::py_execute_11111000, 2},
    {&KS57C21308::py_execute_11111001, 2},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_dummy, 1},
    {&KS57C21308::py_execute_11111100, 2},
    {&KS57C21308::py_execute_11111101, 2},
    {&KS57C21308::py_execute_11111110, 2},
    {&KS57C21308::py_execute_11111111, 2},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11011001_tbl[2] = {
    {&KS57C21308::py_cpse_r_im, 0},
    {&KS57C21308::py_ld_ra_im, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11011011_tbl[2] = {
    {&KS57C21308::py_jp_addr14, 0},
    {&KS57C21308::py_call_addr14, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11011100_tbl[256] = {
    {&KS57C21308::py_ld_ahl_ea, 0},
    {&KS57C21308::py_xch_ea_ahl, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_ld_ea_ahl, 0},
    {&KS57C21308::py_cpse_ea_ahl, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_and_rrb_ea, 0},
    {&KS57C21308::py_and_rrb_ea, 0},
    {&KS57C21308::py_and_rrb_ea, 0},
    {&KS57C21308::py_and_rrb_ea, 0},
    {&KS57C21308::py_and_rrb_ea, 0},
    {&KS57C21308::py_and_rrb_ea, 0},
    {&KS57C21308::py_and_rrb_ea, 0},
    {&KS57C21308::py_and_rrb_ea, 0},
    {&KS57C21308::py_and_ea_rr, 0},
    {&KS57C21308::py_and_ea_rr, 0},
    {&KS57C21308::py_and_ea_rr, 0},
    {&KS57C21308::py_and_ea_rr, 0},
    {&KS57C21308::py_and_ea_rr, 0},
    {&KS57C21308::py_and_ea_rr, 0},
    {&KS57C21308::py_and_ea_rr, 0},
    {&KS57C21308::py_and_ea_rr, 0},
    {&KS57C21308::py_or_rrb_ea, 0},
    {&KS57C21308::py_or_rrb_ea, 0},
    {&KS57C21308::py_or_rrb_ea, 0},
    {&KS57C21308::py_or_rrb_ea, 0},
    {&KS57C21308::py_or_rrb_ea, 0},
    {&KS57C21308::py_or_rrb_ea, 0},
    {&KS57C21308::py_or_rrb_ea, 0},
    {&KS57C21308::py_or_rrb_ea, 0},
    {&KS57C21308::py_or_ea_rr, 0},
    {&KS57C21308::py_or_ea_rr, 0},
    {&KS57C21308::py_or_ea_rr, 0},
    {&KS57C21308::py_or_ea_rr, 0},
    {&KS57C21308::py_or_ea_rr, 0},
    {&KS57C21308::py_or_ea_rr, 0},
    {&KS57C21308::py_or_ea_rr, 0},
    {&KS57C21308::py_or_ea_rr, 0},
    {&KS57C21308::py_xor_rrb_ea, 0},
    {&KS57C21308::py_xor_rrb_ea, 0},
    {&KS57C21308::py_xor_rrb_ea, 0},
    {&KS57C21308::py_xor_rrb_ea, 0},
    {&KS57C21308::py_xor_rrb_ea, 0},
    {&KS57C21308::py_xor_rrb_ea, 0},
    {&KS57C21308::py_xor_rrb_ea, 0},
    {&KS57C21308::py_xor_rrb_ea, 0},
    {&KS57C21308::py_xor_ea_rr, 0},
    {&KS57C21308::py_xor_ea_rr, 0},
    {&KS57C21308::py_xor_ea_rr, 0},
    {&KS57C21308::py_xor_ea_rr, 0},
    {&KS57C21308::py_xor_ea_rr, 0},
    {&KS57C21308::py_xor_ea_rr, 0},
    {&KS57C21308::py_xor_ea_rr, 0},
    {&KS57C21308::py_xor_ea_rr, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_ads_rrb_ea, 0},
    {&KS57C21308::py_ads_rrb_ea, 0},
    {&KS57C21308::py_ads_rrb_ea, 0},
    {&KS57C21308::py_ads_rrb_ea, 0},
    {&KS57C21308::py_ads_rrb_ea, 0},
    {&KS57C21308::py_ads_rrb_ea, 0},
    {&KS57C21308::py_ads_rrb_ea, 0},
    {&KS57C21308::py_ads_rrb_ea, 0},
    {&KS57C21308::py_ads_ea_rr, 0},
    {&KS57C21308::py_ads_ea_rr, 0},
    {&KS57C21308::py_ads_ea_rr, 0},
    {&KS57C21308::py_ads_ea_rr, 0},
    {&KS57C21308::py_ads_ea_rr, 0},
    {&KS57C21308::py_ads_ea_rr, 0},
    {&KS57C21308::py_ads_ea_rr, 0},
    {&KS57C21308::py_ads_ea_rr, 0},
    {&KS57C21308::py_adc_rrb_ea, 0},
    {&KS57C21308::py_adc_rrb_ea, 0},
    {&KS57C21308::py_adc_rrb_ea, 0},
    {&KS57C21308::py_adc_rrb_ea, 0},
    {&KS57C21308::py_adc_rrb_ea, 0},
    {&KS57C21308::py_adc_rrb_ea, 0},
    {&KS57C21308::py_adc_rrb_ea, 0},
    {&KS57C21308::py_adc_rrb_ea, 0},
    {&KS57C21308::py_adc_ea_rr, 0},
    {&KS57C21308::py_adc_ea_rr, 0},
    {&KS57C21308::py_adc_ea_rr, 0},
    {&KS57C21308::py_adc_ea_rr, 0},
    {&KS57C21308::py_adc_ea_rr, 0},
    {&KS57C21308::py_adc_ea_rr, 0},
    {&KS57C21308::py_adc_ea_rr, 0},
    {&KS57C21308::py_adc_ea_rr, 0},
    {&KS57C21308::py_sbs_rrb_ea, 0},
    {&KS57C21308::py_sbs_rrb_ea, 0},
    {&KS57C21308::py_sbs_rrb_ea, 0},
    {&KS57C21308::py_sbs_rrb_ea, 0},
    {&KS57C21308::py_sbs_rrb_ea, 0},
    {&KS57C21308::py_sbs_rrb_ea, 0},
    {&KS57C21308::py_sbs_rrb_ea, 0},
    {&KS57C21308::py_sbs_rrb_ea, 0},
    {&KS57C21308::py_sbs_ea_rr, 0},
    {&KS57C21308::py_sbs_ea_rr, 0},
    {&KS57C21308::py_sbs_ea_rr, 0},
    {&KS57C21308::py_sbs_ea_rr, 0},
    {&KS57C21308::py_sbs_ea_rr, 0},
    {&KS57C21308::py_sbs_ea_rr, 0},
    {&KS57C21308::py_sbs_ea_rr, 0},
    {&KS57C21308::py_sbs_ea_rr, 0},
    {&KS57C21308::py_sbc_rrb_ea, 0},
    {&KS57C21308::py_sbc_rrb_ea, 0},
    {&KS57C21308::py_sbc_rrb_ea, 0},
    {&KS57C21308::py_sbc_rrb_ea, 0},
    {&KS57C21308::py_sbc_rrb_ea, 0},
    {&KS57C21308::py_sbc_rrb_ea, 0},
    {&KS57C21308::py_sbc_rrb_ea, 0},
    {&KS57C21308::py_sbc_rrb_ea, 0},
    {&KS57C21308::py_sbc_ea_rr, 0},
    {&KS57C21308::py_sbc_ea_rr, 0},
    {&KS57C21308::py_sbc_ea_rr, 0},
    {&KS57C21308::py_sbc_ea_rr, 0},
    {&KS57C21308::py_sbc_ea_rr, 0},
    {&KS57C21308::py_sbc_ea_rr, 0},
    {&KS57C21308::py_sbc_ea_rr, 0},
    {&KS57C21308::py_sbc_ea_rr, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_decs_rr, 0},
    {&KS57C21308::py_decs_rr, 0},
    {&KS57C21308::py_decs_rr, 0},
    {&KS57C21308::py_decs_rr, 0},
    {&KS57C21308::py_decs_rr, 0},
    {&KS57C21308::py_decs_rr, 0},
    {&KS57C21308::py_decs_rr, 0},
    {&KS57C21308::py_decs_rr, 0},
    {&KS57C21308::py_xch_ea_rrb, 0},
    {&KS57C21308::py_xch_ea_rrb, 0},
    {&KS57C21308::py_xch_ea_rrb, 0},
    {&KS57C21308::py_xch_ea_rrb, 0},
    {&KS57C21308::py_xch_ea_rrb, 0},
    {&KS57C21308::py_xch_ea_rrb, 0},
    {&KS57C21308::py_xch_ea_rrb, 0},
    {&KS57C21308::py_xch_ea_rrb, 0},
    {&KS57C21308::py_cpse_ea_rr, 0},
    {&KS57C21308::py_cpse_ea_rr, 0},
    {&KS57C21308::py_cpse_ea_rr, 0},
    {&KS57C21308::py_cpse_ea_rr, 0},
    {&KS57C21308::py_cpse_ea_rr, 0},
    {&KS57C21308::py_cpse_ea_rr, 0},
    {&KS57C21308::py_cpse_ea_rr, 0},
    {&KS57C21308::py_cpse_ea_rr, 0},
    {&KS57C21308::py_ld_rrb_ea, 0},
    {&KS57C21308::py_ld_rrb_ea, 0},
    {&KS57C21308::py_ld_rrb_ea, 0},
    {&KS57C21308::py_ld_rrb_ea, 0},
    {&KS57C21308::py_ld_rrb_ea, 0},
    {&KS57C21308::py_ld_rrb_ea, 0},
    {&KS57C21308::py_ld_rrb_ea, 0},
    {&KS57C21308::py_ld_rrb_ea, 0},
    {&KS57C21308::py_ld_ea_rrb, 0},
    {&KS57C21308::py_ld_ea_rrb, 0},
    {&KS57C21308::py_ld_ea_rrb, 0},
    {&KS57C21308::py_ld_ea_rrb, 0},
    {&KS57C21308::py_ld_ea_rrb, 0},
    {&KS57C21308::py_ld_ea_rrb, 0},
    {&KS57C21308::py_ld_ea_rrb, 0},
    {&KS57C21308::py_ld_ea_rrb, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11011101_tbl[256] = {
    {&KS57C21308::py_ld_ra_a, 0},
    {&KS57C21308::py_ld_ra_a, 0},
    {&KS57C21308::py_ld_ra_a, 0},
    {&KS57C21308::py_ld_ra_a, 0},
    {&KS57C21308::py_ld_ra_a, 0},
    {&KS57C21308::py_ld_ra_a, 0},
    {&KS57C21308::py_ld_ra_a, 0},
    {&KS57C21308::py_ld_ra_a, 0},
    {&KS57C21308::py_ld_a_r, 0},
    {&KS57C21308::py_ld_a_r, 0},
    {&KS57C21308::py_ld_a_r, 0},
    {&KS57C21308::py_ld_a_r, 0},
    {&KS57C21308::py_ld_a_r, 0},
    {&KS57C21308::py_ld_a_r, 0},
    {&KS57C21308::py_ld_a_r, 0},
    {&KS57C21308::py_ld_a_r, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_and_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_or_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_xor_a_im, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_smb_n, 0},
    {&KS57C21308::py_srb_n, 0},
    {&KS57C21308::py_srb_n, 0},
    {&KS57C21308::py_srb_n, 0},
    {&KS57C21308::py_srb_n, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_jr_aea, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_incs_ahl, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_jr_awx, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_pop_sb, 0},
    {&KS57C21308::py_push_sb, 0},
    {&KS57C21308::py_cpse_a_r, 0},
    {&KS57C21308::py_cpse_a_r, 0},
    {&KS57C21308::py_cpse_a_r, 0},
    {&KS57C21308::py_cpse_a_r, 0},
    {&KS57C21308::py_cpse_a_r, 0},
    {&KS57C21308::py_cpse_a_r, 0},
    {&KS57C21308::py_cpse_a_r, 0},
    {&KS57C21308::py_cpse_a_r, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_cpse_ahl_im, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
    {&KS57C21308::py_dummy, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11110101_tbl[4] = {
    {&KS57C21308::py_band_cy_ahda_bit, 0},
    {&KS57C21308::py_band_cy_memb_al, 0},
    {&KS57C21308::py_band_cy_mema_bit, 0},
    {&KS57C21308::py_band_cy_mema_bit, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11110110_tbl[4] = {
    {&KS57C21308::py_bor_cy_ahda_bit, 0},
    {&KS57C21308::py_bor_cy_memb_al, 0},
    {&KS57C21308::py_bor_cy_mema_bit, 0},
    {&KS57C21308::py_bor_cy_mema_bit, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11110111_tbl[4] = {
    {&KS57C21308::py_bxor_cy_ahda_bit, 0},
    {&KS57C21308::py_bxor_cy_memb_al, 0},
    {&KS57C21308::py_bxor_cy_mema_bit, 0},
    {&KS57C21308::py_bxor_cy_mema_bit, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11110100_tbl[4] = {
    {&KS57C21308::py_ldb_cy_ahda_bit, 0},
    {&KS57C21308::py_ldb_cy_memb_al, 0},
    {&KS57C21308::py_ldb_cy_mema_bit, 0},
    {&KS57C21308::py_ldb_cy_mema_bit, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11111000_tbl[4] = {
    {&KS57C21308::py_btsf_ahda_bit, 0},
    {&KS57C21308::py_btsf_memb_al, 0},
    {&KS57C21308::py_btsf_mema_bit, 0},
    {&KS57C21308::py_btsf_mema_bit, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11111001_tbl[4] = {
    {&KS57C21308::py_btst_ahda_bit, 0},
    {&KS57C21308::py_btst_memb_al, 0},
    {&KS57C21308::py_btst_mema_bit, 0},
    {&KS57C21308::py_btst_mema_bit, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11111100_tbl[4] = {
    {&KS57C21308::py_ldb_ahda_bit_cy, 0},
    {&KS57C21308::py_ldb_memb_al_cy, 0},
    {&KS57C21308::py_ldb_mema_bit_cy, 0},
    {&KS57C21308::py_ldb_mema_bit_cy, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11111110_tbl[4] = {
    {&KS57C21308::py_bitr_ahda_bit, 0},
    {&KS57C21308::py_bitr_memb_al, 0},
    {&KS57C21308::py_bitr_mema_bit, 0},
    {&KS57C21308::py_bitr_mema_bit, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11111111_tbl[4] = {
    {&KS57C21308::py_bits_ahda_bit, 0},
    {&KS57C21308::py_bits_memb_al, 0},
    {&KS57C21308::py_bits_mema_bit, 0},
    {&KS57C21308::py_bits_mema_bit, 0},
};

const KS57C21308::OpEntry KS57C21308::T_execute_11111101_tbl[4] = {
    {&KS57C21308::py_btstz_ahda_bit, 0},
    {&KS57C21308::py_btstz_memb_al, 0},
    {&KS57C21308::py_btstz_mema_bit, 0},
    {&KS57C21308::py_btstz_mema_bit, 0},
};
