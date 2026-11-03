#include "SPLB32.h"

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

const int64_t k_SFR_TIMEBASE_H_TBL[] = {7, 1, 3, 0};
const int64_t k_SFR_TIMEBASE_L_TBL[] = {511, 127, 255, 63};

}  // namespace

int64_t SPLB32::rom_get_byte(int64_t a) const {
    const int64_t n = rom_size();
    return rom()[(size_t)(((a % n) + n) % n)];
}

void SPLB32::reset() {
    if (!inited_) {
        inited_ = true;
        init_();
    }
    pyreset();
}

void SPLB32::init_() {
    f_instr_counter = 0;
    f_pullup_ext[kPort_PA] = options().num("port_pullup.PA", 0);
    f_pullup_ext[kPort_PB] = options().num("port_pullup.PB", 0);
    f_pullup_ext[kPort_PC] = options().num("port_pullup.PC", 0);
    f_pullup_ext[kPort_PD] = options().num("port_pullup.PD", 0);
    f_port_input[kPort_PA] = {0, 0};
    f_port_input[kPort_PB] = {4, 0};
    f_port_input[kPort_PC] = {0, 0};
    f_port_input[kPort_PD] = {0, 0};
    f_clock = (double)clock_hz();
    f_sub_clock_div = ((double)((double)clock_hz()) / (32768));
}

int64_t SPLB32::execute_scaled() {
    const int64_t scaled = std::llround((double)pyclock() * kTickHz);
    return scaled > 0 ? scaled : 1;
}

uint8_t SPLB32::display_byte(int i) const {
    if (i < 0 || i >= 512 || !f_CLK32768_ENABLE) return 0;
    if ((f_LCD_CTRL1 & 13) == 1) return (uint8_t)f_DPRAM[i];
    return (f_LCD_CTRL1 & 4) ? 0xFF : 0;
}

int SPLB32::port_id(const std::string& name) const {
    if (name == "A") return kPort_A;
    if (name == "BF") return kPort_BF;
    if (name == "CF") return kPort_CF;
    if (name == "DF") return kPort_DF;
    if (name == "IF") return kPort_IF;
    if (name == "LCDRAM") return kPort_LCDRAM;
    if (name == "NF") return kPort_NF;
    if (name == "PA") return kPort_PA;
    if (name == "PB") return kPort_PB;
    if (name == "PC") return kPort_PC;
    if (name == "PC16") return kPort_PC16;
    if (name == "PD") return kPort_PD;
    if (name == "RAM") return kPort_RAM;
    if (name == "SFR") return kPort_SFR;
    if (name == "SP") return kPort_SP;
    if (name == "VF") return kPort_VF;
    if (name == "X") return kPort_X;
    if (name == "Y") return kPort_Y;
    if (name == "ZF") return kPort_ZF;
    return -1;
}

void SPLB32::port_input(int port, uint8_t mask, int level) {
    pypin_handler(port, mask, level);
}

void SPLB32::serialize(BrickStateIO& io) {
    io.pod(f_instr_counter);
    for (auto& kv : f_pullup_ext) io.pod(kv.second);
    for (auto& kv : f_port_input) ser_vec(io, kv.second);
    io.pod(f_clock);
    io.pod(f_sub_clock_div);
    io.pod(f_TM0_counter);
    io.pod(f_TM1_counter);
    io.pod(f_T1KHZ_counter);
    io.pod(f_T1KHZ);
    io.pod(f_TM0);
    io.pod(f_TM0_RELOAD);
    io.pod(f_TM1);
    io.pod(f_TM1_RELOAD);
    io.pod(f_PC);
    io.pod(f_A);
    io.pod(f_X);
    io.pod(f_Y);
    io.pod(f_SP);
    io.pod(f_NF);
    io.pod(f_VF);
    io.pod(f_BF);
    io.pod(f_DF);
    io.pod(f_IF);
    io.pod(f_ZF);
    io.pod(f_CF);
    io.pod(f_CPU_ENBL);
    ser_vec(io, f_RAM);
    ser_vec(io, f_DPRAM);
    io.pod(f_ROM_BANK);
    io.pod(f_WAKEUP_CTRL);
    io.pod(f_WAKEUPREQ);
    io.pod(f_TIME_BASE);
    io.pod(f_TIMER_CTRL);
    io.pod(f_CPU_DIV);
    io.pod(f_CLK32768_ENABLE);
    for (auto& kv : f_PDIR) io.pod(kv.second);
    for (auto& kv : f_ATTR) io.pod(kv.second);
    for (auto& kv : f_PLATCH) io.pod(kv.second);
    io.pod(f_IO_CTRL);
    io.pod(f_INT_CTRL);
    io.pod(f_IREQ);
    io.pod(f_NMI_CTRL);
    io.pod(f_NMIREQ);
    io.pod(f_AUDIO_CH0_CTRL);
    io.pod(f_AUDIO_CH0_DATA);
    io.pod(f_AUDIO_CH1_CTRL);
    io.pod(f_AUDIO_CH1_DATA);
    io.pod(f_BYTE_MIRROR);
    io.pod(f_NIBBLE_SWAP);
    io.pod(f_LCD_CTRL1);
    io.pod(f_addr_reset);
    io.pod(f_addr_irq);
    io.pod(f_addr_nmi);
    io.bytes(snd_envelope_, sizeof(snd_envelope_));
    io.bytes(snd_state_, sizeof(snd_state_));
    ser_vec(io, ee_mem_);
    io.pod(ee_state_);
    io.pod(ee_bits_);
    io.pod(ee_addr_);
    io.pod(ee_scl_);
    io.pod(ee_sda_);
    io.pod(ee_ack_);
    io.pod(ee_rx_);
    io.pod(ee_tx_);
}

void SPLB32::eeprom_port(int64_t port, int64_t value) {
    enum { Idle, AddrByte, WordAddr, WriteData, ReadData };
    if (port != kPort_PD) return;
    const int64_t kSda = 2, kScl = 1, kHwAddr = 160, kHwMask = 0xF8, kSize = 1024, kPage = 256, kWritePage = 16;
    const int64_t prev_scl = ee_scl_, prev_sda = ee_sda_;
    ee_scl_ = value & kScl;
    ee_sda_ = value & kSda;
    if (!(ee_scl_ && (prev_scl != ee_scl_ || prev_sda != ee_sda_))) return;
    if (prev_sda > ee_sda_) {           // START
        ee_state_ = AddrByte;
        ee_bits_ = 0;
        ee_ack_ = 0;
        pypin_handler(kPort_PD, kSda, -1);
    } else if (prev_sda < ee_sda_) {    // STOP
        ee_state_ = Idle;
        ee_ack_ = 0;
    } else if (ee_ack_) {               // SCL rise
        pypin_handler(kPort_PD, kSda, 0);
        ee_ack_ = 0;
    } else if (ee_state_ == ReadData) {
        if (ee_bits_ < 8) {
            pypin_handler(kPort_PD, kSda, ((ee_tx_ >> (7 - ee_bits_)) & 1) ? -1 : 0);
            ee_bits_++;
        } else {
            ee_addr_ = (ee_addr_ + 1) % kSize;
            ee_tx_ = ee_mem_[(size_t)ee_addr_];
            ee_bits_ = 0;
        }
    } else {
        pypin_handler(kPort_PD, kSda, -1);
        ee_rx_ = ((ee_rx_ << 1) | (ee_sda_ > 0)) & 0xFF;
        if (++ee_bits_ == 8) {
            const int64_t rx = ee_rx_;
            ee_bits_ = 0;
            if (ee_state_ == AddrByte) {
                if ((rx & kHwMask) == kHwAddr) {
                    ee_ack_ = 1;
                    ee_addr_ = ((rx & ~kHwMask) >> 1) * kPage + (ee_addr_ % kPage);
                    if (rx & 1) {
                        ee_state_ = ReadData;
                        ee_tx_ = ee_mem_[(size_t)ee_addr_];
                    } else {
                        ee_state_ = WordAddr;
                    }
                } else {
                    ee_state_ = Idle;
                }
            } else if (ee_state_ == WordAddr) {
                ee_ack_ = 1;
                ee_addr_ = ee_addr_ - (ee_addr_ % kPage) + rx;
                ee_state_ = WriteData;
            } else if (ee_state_ == WriteData) {
                ee_ack_ = 1;
                ee_mem_[(size_t)ee_addr_] = (uint8_t)rx;
                ee_addr_ = ee_addr_ - (ee_addr_ % kWritePage) + ((ee_addr_ + 1) % kWritePage);
            }
        }
    }
}

int64_t SPLB32::py_get_rom_addr(int64_t v_addr) {
    if (((v_addr >= 49152))) {
        return (v_addr & 32767);
    }
    return ((v_addr & 32767) | f_ROM_BANK);
    return 0;
}

void SPLB32::pyreset() {
    f_TM0_counter = 0;
    f_TM1_counter = 0;
    f_T1KHZ_counter = 0;
    f_T1KHZ = 0;
    f_TM0 = 0;
    f_TM0_RELOAD = 0;
    f_TM1 = 0;
    f_TM1_RELOAD = 0;
    f_PC = 0;
    f_A = 0;
    f_X = 0;
    f_Y = 0;
    f_SP = 0;
    py_set_ps(4);
    f_CPU_ENBL = 1;
    f_RAM.assign((size_t)(1216), 0);
    f_DPRAM.assign((size_t)(512), 0);
    f_ROM_BANK = 0;
    f_WAKEUP_CTRL = 0;
    f_WAKEUPREQ = 0;
    f_TIME_BASE = 0;
    f_TIMER_CTRL = 0;
    f_CPU_DIV = 2;
    f_CLK32768_ENABLE = 128;
    f_PDIR[kPort_PD] = 0;
    f_PDIR[kPort_PC] = 0;
    f_PDIR[kPort_PB] = 0;
    f_PDIR[kPort_PA] = 0;
    f_ATTR[kPort_PD] = 0;
    f_ATTR[kPort_PC] = 0;
    f_ATTR[kPort_PB] = 0;
    f_ATTR[kPort_PA] = 0;
    f_PLATCH[kPort_PD] = 0;
    f_PLATCH[kPort_PC] = 0;
    f_PLATCH[kPort_PB] = 0;
    f_PLATCH[kPort_PA] = 0;
    f_IO_CTRL = 0;
    f_INT_CTRL = 0;
    f_IREQ = 0;
    f_NMI_CTRL = 0;
    f_NMIREQ = 0;
    f_AUDIO_CH0_CTRL = 0;
    f_AUDIO_CH0_DATA = 0;
    f_AUDIO_CH1_CTRL = 0;
    f_AUDIO_CH1_DATA = 0;
    f_BYTE_MIRROR = 0;
    f_NIBBLE_SWAP = 0;
    f_LCD_CTRL1 = 0;
    f_addr_reset = rom_get_word_LSB(32764);
    f_addr_irq = rom_get_word_LSB(32766);
    f_addr_nmi = rom_get_word_LSB(32762);
    f_PC = f_addr_reset;
}

int64_t SPLB32::py_port_read(int64_t v_port) {
    return ((((~(f_PDIR[v_port])) & (f_port_input[v_port][1] | (f_pullup_ext[v_port] & (~(f_port_input[v_port][0]))))) | ((f_PDIR[v_port] & f_PLATCH[v_port]) & (f_pullup_ext[v_port] | (~(f_ATTR[v_port]))))) & 255);
    return 0;
}

void SPLB32::pypin_handler(int64_t v_port, int64_t v_mask, int64_t v_level) {
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
        if (((((v_port == kPort_PC))) && ((v_mask & 2)))) {
            if ((((v_prev_port & 16) > (py_port_read(v_port) & 16)))) {
                f_IREQ |= 1;
            }
        }
        if (((v_port == kPort_PA))) {
            if ((py_port_read(v_port) & v_mask)) {
                f_WAKEUPREQ |= 1;
            }
        }
    }
}

void SPLB32::py_IRQ() {
    py_write_mem((f_SP | 256), (f_PC >> 8));
    f_SP = ((f_SP - 1) & 255);
    py_write_mem((f_SP | 256), (f_PC & 255));
    f_SP = ((f_SP - 1) & 255);
    py_write_mem((f_SP | 256), py_get_ps());
    f_SP = ((f_SP - 1) & 255);
    f_IF = 1;
    f_PC = f_addr_irq;
}

void SPLB32::py_NMI() {
    if ((f_NMI_CTRL & 128)) {
        if (f_CPU_ENBL) {
            py_write_mem((f_SP | 256), (f_PC >> 8));
            f_SP = ((f_SP - 1) & 255);
            py_write_mem((f_SP | 256), (f_PC & 255));
            f_SP = ((f_SP - 1) & 255);
            py_write_mem((f_SP | 256), py_get_ps());
            f_SP = ((f_SP - 1) & 255);
            f_PC = f_addr_nmi;
        }
    }
}

void SPLB32::py_timers_clock(double v_exec_cycles) {
    int64_t v_time_base_h = 0, v_time_base_l = 0;
    if ((f_TIMER_CTRL & 96)) {
        f_TM0_counter -= v_exec_cycles;
        while (((f_TM0_counter <= 0))) {
            if ((((f_TIMER_CTRL & 96) == 32))) {
                f_TM0_counter = 1;
                f_TM0 += v_exec_cycles;
            } else {
                if ((((f_TIMER_CTRL & 96) == 64))) {
                    f_TM0_counter += f_sub_clock_div;
                    f_TM0 += 1;
                } else {
                }
            }
            while (((f_TM0 > 65535))) {
                f_TM0 -= (65536 - f_TM0_RELOAD);
                f_IREQ |= 16;
                f_WAKEUPREQ |= 4;
                if ((((f_AUDIO_CH0_CTRL & 8)) && ((f_AUDIO_CH0_CTRL & 64)))) {
                    snd_toggle(0);
                }
                if ((((f_AUDIO_CH1_CTRL & 8)) && ((f_AUDIO_CH1_CTRL & 64)))) {
                    snd_toggle(1);
                }
            }
        }
    }
    if ((f_TIMER_CTRL & 3)) {
        f_TM1_counter -= v_exec_cycles;
        while (((f_TM1_counter <= 0))) {
            if ((((f_TIMER_CTRL & 3) == 1))) {
                f_TM1_counter = 1;
                f_TM1 += v_exec_cycles;
            } else {
                if ((((f_TIMER_CTRL & 3) == 2))) {
                    f_TM1_counter += f_sub_clock_div;
                    f_TM1 += 1;
                } else {
                }
            }
            while (((f_TM1 > 65535))) {
                f_TM1 -= (65536 - f_TM1_RELOAD);
                f_IREQ |= 32;
                f_WAKEUPREQ |= 16;
                f_NMIREQ |= 2;
                if ((f_NMI_CTRL & 2)) {
                    py_NMI();
                }
                if ((((f_AUDIO_CH0_CTRL & 16)) && ((f_AUDIO_CH0_CTRL & 64)))) {
                    snd_toggle(0);
                }
                if ((((f_AUDIO_CH1_CTRL & 16)) && ((f_AUDIO_CH1_CTRL & 64)))) {
                    snd_toggle(1);
                }
            }
        }
    }
    f_T1KHZ_counter -= v_exec_cycles;
    while (((f_T1KHZ_counter <= 0))) {
        f_T1KHZ_counter += (f_sub_clock_div * py_div(32768, 1024));
        f_T1KHZ += 1;
        v_time_base_h = (f_TIME_BASE & 3);
        if ((!((f_T1KHZ & k_SFR_TIMEBASE_H_TBL[v_time_base_h])))) {
            f_IREQ |= 8;
            f_WAKEUPREQ |= 8;
        }
        v_time_base_l = ((f_TIME_BASE & 12) >> 2);
        if ((!((f_T1KHZ & k_SFR_TIMEBASE_L_TBL[v_time_base_l])))) {
            f_IREQ |= 4;
            f_WAKEUPREQ |= 2;
        }
    }
}

double SPLB32::pyclock() {
    int64_t v_opcode = 0;
    double v_exec_cycles = 0;
    if (f_CPU_ENBL) {
        v_opcode = py_read_mem(f_PC);
        f_PC = ((f_PC + 1) & 65535);
        v_exec_cycles = ((this->*T_execute[v_opcode].fn)(0) << f_CPU_DIV);
        f_instr_counter += 1;
        py_timers_clock(v_exec_cycles);
        if ((((!(f_IF))) && ((f_IREQ & f_INT_CTRL)))) {
            py_IRQ();
        }
    } else {
        v_exec_cycles = f_sub_clock_div;
        if ((f_WAKEUP_CTRL & f_WAKEUPREQ)) {
            f_CPU_ENBL = 1;
            f_CPU_DIV = 2;
        }
        if (f_CLK32768_ENABLE) {
            py_timers_clock(v_exec_cycles);
        }
    }
    return v_exec_cycles;
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portA_attr(int64_t v_value) {
    f_ATTR[kPort_PA] = v_value;
    eeprom_port(kPort_PA, py_port_read(kPort_PA));
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portA_dir(int64_t v_value) {
    f_PDIR[kPort_PA] = v_value;
    eeprom_port(kPort_PA, py_port_read(kPort_PA));
    return 0;
}

int64_t SPLB32::py_get_sfr_io_portA_data() {
    return py_port_read(kPort_PA);
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portA_data(int64_t v_value) {
    f_PLATCH[kPort_PA] = v_value;
    eeprom_port(kPort_PA, py_port_read(kPort_PA));
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portB_attr(int64_t v_value) {
    f_ATTR[kPort_PB] = (((((v_value & 1) * 3) | ((v_value & 2) * 6)) | ((v_value & 4) * 12)) | ((v_value & 8) * 24));
    eeprom_port(kPort_PB, py_port_read(kPort_PB));
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portB_dir(int64_t v_value) {
    f_PDIR[kPort_PB] = (((((v_value & 1) * 3) | ((v_value & 2) * 6)) | ((v_value & 4) * 12)) | ((v_value & 8) * 24));
    eeprom_port(kPort_PB, py_port_read(kPort_PB));
    return 0;
}

int64_t SPLB32::py_get_sfr_io_portB_data() {
    return py_port_read(kPort_PB);
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portB_data(int64_t v_value) {
    f_PLATCH[kPort_PB] = v_value;
    eeprom_port(kPort_PB, py_port_read(kPort_PB));
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portC_attr(int64_t v_value) {
    f_ATTR[kPort_PC] = v_value;
    eeprom_port(kPort_PC, py_port_read(kPort_PC));
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portC_dir(int64_t v_value) {
    f_PDIR[kPort_PC] = v_value;
    eeprom_port(kPort_PC, py_port_read(kPort_PC));
    return 0;
}

int64_t SPLB32::py_get_sfr_io_portC_data() {
    return py_port_read(kPort_PC);
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portC_data(int64_t v_value) {
    f_PLATCH[kPort_PC] = v_value;
    eeprom_port(kPort_PC, py_port_read(kPort_PC));
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portD_dir(int64_t v_value) {
    f_PDIR[kPort_PD] = (v_value & 63);
    eeprom_port(kPort_PD, py_port_read(kPort_PD));
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portD_attr(int64_t v_value) {
    f_ATTR[kPort_PD] = (v_value & 63);
    eeprom_port(kPort_PD, py_port_read(kPort_PD));
    return 0;
}

int64_t SPLB32::py_get_sfr_io_portD_data() {
    return py_port_read(kPort_PD);
    return 0;
}

int64_t SPLB32::py_set_sfr_io_portD_data(int64_t v_value) {
    f_PLATCH[kPort_PD] = (v_value & 63);
    eeprom_port(kPort_PD, py_port_read(kPort_PD));
    return 0;
}

int64_t SPLB32::py_get_sfr_audio_ch0_ctrl() {
    return f_AUDIO_CH0_CTRL;
    return 0;
}

int64_t SPLB32::py_set_sfr_audio_ch0_ctrl(int64_t v_value) {
    f_AUDIO_CH0_CTRL = v_value;
    return 0;
}

int64_t SPLB32::py_get_sfr_audio_ch0_data() {
    return 0;
    return 0;
}

int64_t SPLB32::py_set_sfr_audio_ch0_data(int64_t v_value) {
    f_AUDIO_CH0_DATA = v_value;
    snd_set_data(0, f_AUDIO_CH0_CTRL, v_value);
    return 0;
}

int64_t SPLB32::py_get_sfr_audio_ch1_ctrl() {
    return f_AUDIO_CH1_CTRL;
    return 0;
}

int64_t SPLB32::py_set_sfr_audio_ch1_ctrl(int64_t v_value) {
    f_AUDIO_CH1_CTRL = v_value;
    return 0;
}

int64_t SPLB32::py_get_sfr_audio_ch1_data() {
    return f_AUDIO_CH1_DATA;
    return 0;
}

int64_t SPLB32::py_set_sfr_audio_ch1_data(int64_t v_value) {
    f_AUDIO_CH1_DATA = v_value;
    snd_set_data(1, f_AUDIO_CH1_CTRL, v_value);
    return 0;
}

int64_t SPLB32::py_get_sfr_bank_sel() {
    return (f_ROM_BANK >> 15);
    return 0;
}

int64_t SPLB32::py_set_sfr_bank_sel(int64_t v_value) {
    f_ROM_BANK = ((v_value & 15) << 15);
    return 0;
}

int64_t SPLB32::py_get_sfr_wakeup_ctrl() {
    int64_t v_value = 0;
    v_value = f_WAKEUPREQ;
    return v_value;
    return 0;
}

int64_t SPLB32::py_set_sfr_wakeup_ctrl(int64_t v_value) {
    f_WAKEUP_CTRL = v_value;
    f_WAKEUPREQ &= v_value;
    return 0;
}

int64_t SPLB32::py_get_sfr_timer_timebase_sel() {
    return f_T1KHZ;
    return 0;
}

int64_t SPLB32::py_set_sfr_timer_timebase_sel(int64_t v_value) {
    f_TIME_BASE = v_value;
    return 0;
}

int64_t SPLB32::py_get_sfr_timer_ctrl() {
    return f_TIMER_CTRL;
    return 0;
}

int64_t SPLB32::py_set_sfr_timer_ctrl(int64_t v_value) {
    f_TIMER_CTRL = v_value;
    return 0;
}

int64_t SPLB32::py_set_sfr_clk_32768_en(int64_t v_value) {
    f_CLK32768_ENABLE = (v_value & 128);
    return 0;
}

int64_t SPLB32::py_get_sfr_nmi_ctrl() {
    int64_t v_buf = 0;
    v_buf = (f_NMIREQ | (f_NMI_CTRL & 128));
    return v_buf;
    return 0;
}

int64_t SPLB32::py_set_sfr_nmi_ctrl(int64_t v_value) {
    f_NMI_CTRL = v_value;
    f_NMIREQ &= v_value;
    return 0;
}

int64_t SPLB32::py_get_sfr_int_ctrl() {
    return f_IREQ;
    return 0;
}

int64_t SPLB32::py_set_sfr_int_ctrl(int64_t v_value) {
    f_INT_CTRL = v_value;
    f_IREQ = 0;
    return 0;
}

int64_t SPLB32::py_get_sfr_clk_cpu_ctrl() {
    return f_CPU_DIV;
    return 0;
}

int64_t SPLB32::py_set_sfr_clk_cpu_ctrl(int64_t v_value) {
    if ((((v_value & 7) == 7))) {
        f_CPU_ENBL = 0;
    } else {
        f_CPU_DIV = (v_value & 7);
    }
    return 0;
}

int64_t SPLB32::py_set_sfr_clk_int_clear(int64_t v_value) {
    f_IREQ &= (~(v_value));
    return 0;
}

int64_t SPLB32::py_get_sfr_timer_TM1L() {
    return (((int64_t)(f_TM1)) & 255);
    return 0;
}

int64_t SPLB32::py_set_sfr_timer_TM1L(int64_t v_value) {
    f_TM1_RELOAD = ((f_TM1_RELOAD & 65280) | v_value);
    return 0;
}

int64_t SPLB32::py_get_sfr_timer_TM1H() {
    return (((int64_t)(f_TM1)) >> 8);
    return 0;
}

int64_t SPLB32::py_set_sfr_timer_TM1H(int64_t v_value) {
    f_TM1_RELOAD = ((f_TM1_RELOAD & 255) | (v_value << 8));
    f_TM1 = f_TM1_RELOAD;
    return 0;
}

int64_t SPLB32::py_get_sfr_timer_TM0L() {
    return (((int64_t)(f_TM0)) & 255);
    return 0;
}

int64_t SPLB32::py_set_sfr_timer_TM0L(int64_t v_value) {
    f_TM0_RELOAD = ((f_TM0_RELOAD & 65280) | v_value);
    return 0;
}

int64_t SPLB32::py_get_sfr_timer_TM0H() {
    return (((int64_t)(f_TM0)) >> 8);
    return 0;
}

int64_t SPLB32::py_set_sfr_timer_TM0H(int64_t v_value) {
    f_TM0_RELOAD = ((f_TM0_RELOAD & 255) | (v_value << 8));
    f_TM0 = f_TM0_RELOAD;
    return 0;
}

int64_t SPLB32::py_get_sfr_lcd_ctrl1() {
    return f_LCD_CTRL1;
    return 0;
}

int64_t SPLB32::py_set_sfr_lcd_ctrl1(int64_t v_value) {
    f_LCD_CTRL1 = v_value;
    return 0;
}

int64_t SPLB32::py_get_sfr_specialfunc_config() {
    return 0;
    return 0;
}

int64_t SPLB32::py_set_sfr_specialfunc_config(int64_t v_value) {
    return 0;
}

int64_t SPLB32::py_get_sfr_aux_byte_mirror() {
    return f_BYTE_MIRROR;
    return 0;
}

int64_t SPLB32::py_set_sfr_aux_byte_mirror(int64_t v_value) {
    int64_t v_b = 0;
    v_b = (((v_value & 240) >> 4) | ((v_value & 15) << 4));
    v_b = (((v_b & 204) >> 2) | ((v_b & 51) << 2));
    f_BYTE_MIRROR = (((v_b & 170) >> 1) | ((v_b & 85) << 1));
    return 0;
}

int64_t SPLB32::py_get_sfr_aux_nibble_swap() {
    return f_NIBBLE_SWAP;
    return 0;
}

int64_t SPLB32::py_set_sfr_aux_nibble_swap(int64_t v_value) {
    f_NIBBLE_SWAP = (((v_value & 15) << 4) | ((v_value & 240) >> 4));
    return 0;
}

int64_t SPLB32::py_get_sfr_dummy() {
    return 0;
    return 0;
}

void SPLB32::py_set_sfr_dummy(int64_t v_value) {
}

void SPLB32::py_write_mem(int64_t v_addr, int64_t v_value) {
    if (((((v_addr >= 64))) && (((v_addr < (1216 + 64)))))) {
        f_RAM[(v_addr - 64)] = v_value;
    } else {
        if (((((v_addr >= 15872))) && (((v_addr < (512 + 15872)))))) {
            f_DPRAM[(v_addr - 15872)] = v_value;
        } else {
            const IoEntry* v_io = find_sfr_tbl(v_addr);
            if (((v_io != nullptr))) {
                (this->*v_io->set)(v_value);
            }
        }
    }
}

int64_t SPLB32::py_read_mem(int64_t v_addr) {
    if (((v_addr >= 49152))) {
        return rom_get_byte((v_addr & 32767));
    } else {
        if (((v_addr >= 16384))) {
            return rom_get_byte(((v_addr & 32767) | f_ROM_BANK));
        } else {
            if (((v_addr >= 15872))) {
                return f_DPRAM[(v_addr - 15872)];
            } else {
                if (((((v_addr >= 64))) && (((v_addr < (1216 + 64)))))) {
                    return f_RAM[(v_addr - 64)];
                } else {
                    const IoEntry* v_io = find_sfr_tbl(v_addr);
                    if (((v_io != nullptr))) {
                        return (this->*v_io->get)();
                    }
                    return 0;
                }
            }
        }
    }
    return 0;
}

int64_t SPLB32::py_get_ps() {
    return (((((((f_NF << 7) | (f_VF << 6)) | (f_BF << 4)) | (f_DF << 3)) | (f_IF << 2)) | (f_ZF << 1)) | f_CF);
    return 0;
}

void SPLB32::py_set_ps(int64_t v_ps) {
    f_NF = (v_ps >> 7);
    f_VF = (((v_ps & 64) > 0));
    f_BF = (((v_ps & 16) > 0));
    f_DF = (((v_ps & 8) > 0));
    f_IF = (((v_ps & 4) > 0));
    f_ZF = (((v_ps & 2) > 0));
    f_CF = (v_ps & 1);
}

void SPLB32::py_adc(int64_t v_operand) {
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

void SPLB32::py_sbc(int64_t v_operand) {
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

int64_t SPLB32::py_brk(int64_t) {
    f_BF = 1;
    py_IRQ();
    return 7;
    return 0;
}

int64_t SPLB32::py_ora_ind_x(int64_t) {
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

int64_t SPLB32::py_ora_zp(int64_t) {
    f_A |= py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t SPLB32::py_asl_zp(int64_t) {
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

int64_t SPLB32::py_php(int64_t) {
    py_write_mem((f_SP | 256), py_get_ps());
    f_SP = ((f_SP - 1) & 255);
    return 3;
    return 0;
}

int64_t SPLB32::py_ora_imm(int64_t) {
    f_A |= py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t SPLB32::py_asl_a(int64_t) {
    int64_t v_new_value = 0;
    v_new_value = (f_A << 1);
    f_A = (v_new_value & 255);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = (v_new_value >> 8);
    return 2;
    return 0;
}

int64_t SPLB32::py_ora_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t SPLB32::py_asl_abs(int64_t) {
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

int64_t SPLB32::py_bpl(int64_t) {
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

int64_t SPLB32::py_ora_ind_y(int64_t) {
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

int64_t SPLB32::py_ora_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_A |= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t SPLB32::py_asl_zp_x(int64_t) {
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

int64_t SPLB32::py_ora_abs_y(int64_t) {
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

int64_t SPLB32::py_and_ind_x(int64_t) {
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

int64_t SPLB32::py_ora_abs_x(int64_t) {
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

int64_t SPLB32::py_asl_abs_x(int64_t) {
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

int64_t SPLB32::py_clc(int64_t) {
    f_CF = 0;
    return 2;
    return 0;
}

int64_t SPLB32::py_jsr_abs(int64_t) {
    int64_t v_pc = 0;
    v_pc = ((f_PC + 1) & 65535);
    py_write_mem((f_SP | 256), (v_pc >> 8));
    f_SP = ((f_SP - 1) & 255);
    py_write_mem((f_SP | 256), (v_pc & 255));
    f_SP = ((f_SP - 1) & 255);
    f_PC = (py_read_mem(f_PC) | (py_read_mem(v_pc) << 8));
    return 6;
    return 0;
}

int64_t SPLB32::py_and_abs_y(int64_t) {
    int64_t v_addr = 0;
    v_addr = (((py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8)) + f_Y) & 65535);
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t SPLB32::py_and_abs_x(int64_t) {
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

int64_t SPLB32::py_rol_abs_x(int64_t) {
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

int64_t SPLB32::py_bit_zp(int64_t) {
    int64_t v_m = 0;
    v_m = py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (v_m >> 7);
    f_VF = (((v_m & 64) > 0));
    f_ZF = (!((f_A & v_m)));
    return 3;
    return 0;
}

int64_t SPLB32::py_and_zp(int64_t) {
    f_A &= py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t SPLB32::py_rol_zp(int64_t) {
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

int64_t SPLB32::py_plp(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    py_set_ps(py_read_mem((f_SP | 256)));
    return 4;
    return 0;
}

int64_t SPLB32::py_and_imm(int64_t) {
    f_A &= py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t SPLB32::py_rol_a(int64_t) {
    int64_t v_new_value = 0;
    v_new_value = ((f_A << 1) | f_CF);
    f_A = (v_new_value & 255);
    f_NF = (((v_new_value & 128) > 0));
    f_ZF = (!((v_new_value & 255)));
    f_CF = ((v_new_value > 255));
    return 2;
    return 0;
}

int64_t SPLB32::py_bit_abs(int64_t) {
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

int64_t SPLB32::py_and_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t SPLB32::py_rol_abs(int64_t) {
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

int64_t SPLB32::py_bmi(int64_t) {
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

int64_t SPLB32::py_and_ind_y(int64_t) {
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

int64_t SPLB32::py_and_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_A &= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 6;
    return 0;
}

int64_t SPLB32::py_rol_zp_x(int64_t) {
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

int64_t SPLB32::py_sec(int64_t) {
    f_CF = 1;
    return 2;
    return 0;
}

int64_t SPLB32::py_rti(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    py_set_ps(py_read_mem((f_SP | 256)));
    f_SP = ((f_SP + 1) & 255);
    f_PC = py_read_mem((f_SP | 256));
    f_SP = ((f_SP + 1) & 255);
    f_PC |= (py_read_mem((f_SP | 256)) << 8);
    return 6;
    return 0;
}

int64_t SPLB32::py_eor_ind_x(int64_t) {
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

int64_t SPLB32::py_eor_zp(int64_t) {
    f_A ^= py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t SPLB32::py_lsr_zp(int64_t) {
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

int64_t SPLB32::py_pha(int64_t) {
    py_write_mem((f_SP | 256), f_A);
    f_SP = ((f_SP - 1) & 255);
    return 3;
    return 0;
}

int64_t SPLB32::py_eor_imm(int64_t) {
    f_A ^= (py_read_mem(f_PC) & 255);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t SPLB32::py_lsr_a(int64_t) {
    int64_t v_prev_value = 0;
    v_prev_value = f_A;
    f_A = (v_prev_value >> 1);
    f_NF = 0;
    f_ZF = (!((v_prev_value & 254)));
    f_CF = (v_prev_value & 1);
    return 2;
    return 0;
}

int64_t SPLB32::py_jmp_abs(int64_t) {
    f_PC = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    return 3;
    return 0;
}

int64_t SPLB32::py_eor_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t SPLB32::py_lsr_abs(int64_t) {
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

int64_t SPLB32::py_bvc(int64_t) {
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

int64_t SPLB32::py_eor_ind_y(int64_t) {
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

int64_t SPLB32::py_eor_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_A ^= py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t SPLB32::py_lsr_zp_x(int64_t) {
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

int64_t SPLB32::py_cli(int64_t) {
    f_IF = 0;
    return 2;
    return 0;
}

int64_t SPLB32::py_eor_abs_y(int64_t) {
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

int64_t SPLB32::py_eor_abs_x(int64_t) {
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

int64_t SPLB32::py_lsr_abs_x(int64_t) {
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

int64_t SPLB32::py_rts(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    f_PC = py_read_mem((f_SP | 256));
    f_SP = ((f_SP + 1) & 255);
    f_PC |= (py_read_mem((f_SP | 256)) << 8);
    f_PC += 1;
    return 6;
    return 0;
}

int64_t SPLB32::py_adc_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t SPLB32::py_adc_zp(int64_t) {
    py_adc(py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t SPLB32::py_ror_zp(int64_t) {
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

int64_t SPLB32::py_pla(int64_t) {
    f_SP = ((f_SP + 1) & 255);
    f_A = py_read_mem((f_SP | 256));
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t SPLB32::py_adc_imm(int64_t) {
    py_adc(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t SPLB32::py_ror_a(int64_t) {
    int64_t v_prev_value = 0;
    v_prev_value = f_A;
    f_A = ((v_prev_value >> 1) | (f_CF << 7));
    f_NF = f_CF;
    f_ZF = (!(((v_prev_value & 254) | f_CF)));
    f_CF = (v_prev_value & 1);
    return 2;
    return 0;
}

int64_t SPLB32::py_jmp_ind(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_PC = py_read_mem(v_addr);
    f_PC |= (py_read_mem(((v_addr + 1) & 65535)) << 8);
    return 6;
    return 0;
}

int64_t SPLB32::py_adc_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_ror_abs(int64_t) {
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

int64_t SPLB32::py_bvs(int64_t) {
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

int64_t SPLB32::py_adc_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return (5 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t SPLB32::py_adc_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 5;
    return 0;
}

int64_t SPLB32::py_ror_zp_x(int64_t) {
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

int64_t SPLB32::py_sei(int64_t) {
    f_IF = 1;
    return 2;
    return 0;
}

int64_t SPLB32::py_adc_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t SPLB32::py_adc_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    py_adc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t SPLB32::py_ror_abs_x(int64_t) {
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

int64_t SPLB32::py_sta_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t SPLB32::py_sty_zp(int64_t) {
    py_write_mem(py_read_mem(f_PC), f_Y);
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t SPLB32::py_sta_zp(int64_t) {
    py_write_mem(py_read_mem(f_PC), f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t SPLB32::py_stx_zp(int64_t) {
    py_write_mem(py_read_mem(f_PC), f_X);
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t SPLB32::py_dey(int64_t) {
    f_Y = ((f_Y - 1) & 255);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 2;
    return 0;
}

int64_t SPLB32::py_txa(int64_t) {
    f_A = f_X;
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t SPLB32::py_sty_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_write_mem(v_addr, f_Y);
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_sta_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_stx_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_write_mem(v_addr, f_X);
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_bcc(int64_t) {
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

int64_t SPLB32::py_sta_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t SPLB32::py_sty_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_write_mem(v_addr, f_Y);
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_sta_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_stx_zp_y(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_Y) & 255);
    py_write_mem(v_addr, f_X);
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_tya(int64_t) {
    f_A = f_Y;
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t SPLB32::py_sta_abs_y(int64_t) {
    int64_t v_addr = 0;
    v_addr = (((py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8)) + f_Y) & 65535);
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 2) & 65535);
    return 5;
    return 0;
}

int64_t SPLB32::py_txs(int64_t) {
    f_SP = f_X;
    return 2;
    return 0;
}

int64_t SPLB32::py_sta_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    py_write_mem(v_addr, f_A);
    f_PC = ((f_PC + 2) & 65535);
    return 5;
    return 0;
}

int64_t SPLB32::py_ldy_imm(int64_t) {
    f_Y = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 2;
    return 0;
}

int64_t SPLB32::py_lda_ind_x(int64_t) {
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

int64_t SPLB32::py_ldx_imm(int64_t) {
    f_X = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t SPLB32::py_ldy_zp(int64_t) {
    f_Y = py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 3;
    return 0;
}

int64_t SPLB32::py_lda_zp(int64_t) {
    f_A = py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 3;
    return 0;
}

int64_t SPLB32::py_ldx_zp(int64_t) {
    f_X = py_read_mem(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 3;
    return 0;
}

int64_t SPLB32::py_tay(int64_t) {
    f_Y = f_A;
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 2;
    return 0;
}

int64_t SPLB32::py_lda_imm(int64_t) {
    f_A = py_read_mem(f_PC);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 2;
    return 0;
}

int64_t SPLB32::py_tax(int64_t) {
    f_X = f_A;
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t SPLB32::py_ldy_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_Y = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 4;
    return 0;
}

int64_t SPLB32::py_lda_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t SPLB32::py_ldx_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    f_X = py_read_mem(v_addr);
    f_PC = ((f_PC + 2) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 4;
    return 0;
}

int64_t SPLB32::py_bcs(int64_t) {
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

int64_t SPLB32::py_lda_ind_y(int64_t) {
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

int64_t SPLB32::py_ldy_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_Y = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 4;
    return 0;
}

int64_t SPLB32::py_lda_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    f_A = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_A >> 7);
    f_ZF = (!(f_A));
    return 4;
    return 0;
}

int64_t SPLB32::py_ldx_zp_y(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_Y) & 255);
    f_X = py_read_mem(v_addr);
    f_PC = ((f_PC + 1) & 65535);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 4;
    return 0;
}

int64_t SPLB32::py_clv(int64_t) {
    f_VF = 0;
    return 2;
    return 0;
}

int64_t SPLB32::py_lda_abs_y(int64_t) {
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

int64_t SPLB32::py_tsx(int64_t) {
    f_X = f_SP;
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t SPLB32::py_ldy_abs_x(int64_t) {
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

int64_t SPLB32::py_lda_abs_x(int64_t) {
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

int64_t SPLB32::py_ldx_abs_y(int64_t) {
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

int64_t SPLB32::py_cpy_imm(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_Y - py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 2;
    return 0;
}

int64_t SPLB32::py_cmp_ind_x(int64_t) {
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

int64_t SPLB32::py_cpy_zp(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_Y - py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 3;
    return 0;
}

int64_t SPLB32::py_cmp_zp(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_A - py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 3;
    return 0;
}

int64_t SPLB32::py_dec_zp(int64_t) {
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

int64_t SPLB32::py_iny(int64_t) {
    f_Y = ((f_Y + 1) & 255);
    f_NF = (f_Y >> 7);
    f_ZF = (!(f_Y));
    return 2;
    return 0;
}

int64_t SPLB32::py_cmp_imm(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_A - py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 2;
    return 0;
}

int64_t SPLB32::py_cpy_abs(int64_t) {
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

int64_t SPLB32::py_cmp_abs(int64_t) {
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

int64_t SPLB32::py_dec_abs(int64_t) {
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

int64_t SPLB32::py_dex(int64_t) {
    f_X = ((f_X - 1) & 255);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t SPLB32::py_bne(int64_t) {
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

int64_t SPLB32::py_cmp_ind_y(int64_t) {
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

int64_t SPLB32::py_cmp_zp_x(int64_t) {
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

int64_t SPLB32::py_dec_zp_x(int64_t) {
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

int64_t SPLB32::py_cld(int64_t) {
    f_DF = 0;
    return 2;
    return 0;
}

int64_t SPLB32::py_cmp_abs_y(int64_t) {
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

int64_t SPLB32::py_cmp_abs_x(int64_t) {
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

int64_t SPLB32::py_dec_abs_x(int64_t) {
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

int64_t SPLB32::py_cpx_imm(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_X - py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 2;
    return 0;
}

int64_t SPLB32::py_sbc_ind_x(int64_t) {
    int64_t v_addr = 0, v_zp = 0;
    v_zp = (py_read_mem(f_PC) + f_X);
    v_addr = (py_read_mem((v_zp & 255)) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 6;
    return 0;
}

int64_t SPLB32::py_cpx_zp(int64_t) {
    int64_t v_test_value = 0;
    v_test_value = (f_X - py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    f_NF = ((v_test_value >> 7) & 1);
    f_ZF = (!(v_test_value));
    f_CF = ((v_test_value >= 0));
    return 3;
    return 0;
}

int64_t SPLB32::py_sbc_zp(int64_t) {
    py_sbc(py_read_mem(py_read_mem(f_PC)));
    f_PC = ((f_PC + 1) & 65535);
    return 3;
    return 0;
}

int64_t SPLB32::py_inc_zp(int64_t) {
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

int64_t SPLB32::py_inx(int64_t) {
    f_X = ((f_X + 1) & 255);
    f_NF = (f_X >> 7);
    f_ZF = (!(f_X));
    return 2;
    return 0;
}

int64_t SPLB32::py_sbc_imm(int64_t) {
    py_sbc(py_read_mem(f_PC));
    f_PC = ((f_PC + 1) & 65535);
    return 2;
    return 0;
}

int64_t SPLB32::py_cpx_abs(int64_t) {
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

int64_t SPLB32::py_sbc_abs(int64_t) {
    int64_t v_addr = 0;
    v_addr = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_inc_abs(int64_t) {
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

int64_t SPLB32::py_nop(int64_t) {
    return 2;
    return 0;
}

int64_t SPLB32::py_beq(int64_t) {
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

int64_t SPLB32::py_sbc_ind_y(int64_t) {
    int64_t v_addr = 0, v_base = 0, v_zp = 0;
    v_zp = py_read_mem(f_PC);
    v_base = (py_read_mem(v_zp) | (py_read_mem(((v_zp + 1) & 255)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t SPLB32::py_sbc_zp_x(int64_t) {
    int64_t v_addr = 0;
    v_addr = ((py_read_mem(f_PC) + f_X) & 255);
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 1) & 65535);
    return 4;
    return 0;
}

int64_t SPLB32::py_inc_zp_x(int64_t) {
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

int64_t SPLB32::py_sed(int64_t) {
    f_DF = 1;
    return 2;
    return 0;
}

int64_t SPLB32::py_sbc_abs_y(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_Y) & 65535);
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t SPLB32::py_sbc_abs_x(int64_t) {
    int64_t v_addr = 0, v_base = 0;
    v_base = (py_read_mem(f_PC) | (py_read_mem(((f_PC + 1) & 65535)) << 8));
    v_addr = ((v_base + f_X) & 65535);
    py_sbc(py_read_mem(v_addr));
    f_PC = ((f_PC + 2) & 65535);
    return (4 + (((v_base ^ v_addr) > 255)));
    return 0;
}

int64_t SPLB32::py_inc_abs_x(int64_t) {
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

int64_t SPLB32::py_dummy(int64_t) {
    (void)0;
    return 2;
    return 0;
}

const SPLB32::IoEntry* SPLB32::find_sfr_tbl(int64_t addr) {
    static const IoEntry e[] = {
        {&SPLB32::py_get_sfr_bank_sel, &SPLB32::py_set_sfr_bank_sel},
        {&SPLB32::py_get_sfr_clk_cpu_ctrl, &SPLB32::py_set_sfr_clk_cpu_ctrl},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_clk_32768_en},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_io_portA_attr},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_io_portA_dir},
        {&SPLB32::py_get_sfr_io_portA_data, &SPLB32::py_set_sfr_io_portA_data},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_io_portB_attr},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_io_portB_dir},
        {&SPLB32::py_get_sfr_io_portB_data, &SPLB32::py_set_sfr_io_portB_data},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_io_portC_attr},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_io_portC_dir},
        {&SPLB32::py_get_sfr_io_portC_data, &SPLB32::py_set_sfr_io_portC_data},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_io_portD_attr},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_io_portD_dir},
        {&SPLB32::py_get_sfr_io_portD_data, &SPLB32::py_set_sfr_io_portD_data},
        {&SPLB32::py_get_sfr_specialfunc_config, &SPLB32::py_set_sfr_specialfunc_config},
        {&SPLB32::py_get_sfr_lcd_ctrl1, &SPLB32::py_set_sfr_lcd_ctrl1},
        {&SPLB32::py_get_sfr_int_ctrl, &SPLB32::py_set_sfr_int_ctrl},
        {&SPLB32::py_get_sfr_dummy, &SPLB32::py_set_sfr_clk_int_clear},
        {&SPLB32::py_get_sfr_nmi_ctrl, &SPLB32::py_set_sfr_nmi_ctrl},
        {&SPLB32::py_get_sfr_wakeup_ctrl, &SPLB32::py_set_sfr_wakeup_ctrl},
        {&SPLB32::py_get_sfr_timer_timebase_sel, &SPLB32::py_set_sfr_timer_timebase_sel},
        {&SPLB32::py_get_sfr_timer_ctrl, &SPLB32::py_set_sfr_timer_ctrl},
        {&SPLB32::py_get_sfr_timer_TM0L, &SPLB32::py_set_sfr_timer_TM0L},
        {&SPLB32::py_get_sfr_timer_TM0H, &SPLB32::py_set_sfr_timer_TM0H},
        {&SPLB32::py_get_sfr_timer_TM1L, &SPLB32::py_set_sfr_timer_TM1L},
        {&SPLB32::py_get_sfr_timer_TM1H, &SPLB32::py_set_sfr_timer_TM1H},
        {&SPLB32::py_get_sfr_aux_byte_mirror, &SPLB32::py_set_sfr_aux_byte_mirror},
        {&SPLB32::py_get_sfr_aux_nibble_swap, &SPLB32::py_set_sfr_aux_nibble_swap},
        {&SPLB32::py_get_sfr_audio_ch0_ctrl, &SPLB32::py_set_sfr_audio_ch0_ctrl},
        {&SPLB32::py_get_sfr_audio_ch0_data, &SPLB32::py_set_sfr_audio_ch0_data},
        {&SPLB32::py_get_sfr_audio_ch1_ctrl, &SPLB32::py_set_sfr_audio_ch1_ctrl},
        {&SPLB32::py_get_sfr_audio_ch1_data, &SPLB32::py_set_sfr_audio_ch1_data},
    };
    switch (addr) {
        case 0: return &e[0];
        case 4: return &e[1];
        case 14: return &e[2];
        case 6: return &e[3];
        case 7: return &e[4];
        case 8: return &e[5];
        case 40: return &e[6];
        case 41: return &e[7];
        case 10: return &e[8];
        case 42: return &e[9];
        case 43: return &e[10];
        case 11: return &e[11];
        case 44: return &e[12];
        case 45: return &e[13];
        case 9: return &e[14];
        case 39: return &e[15];
        case 36: return &e[16];
        case 1: return &e[17];
        case 2: return &e[18];
        case 56: return &e[19];
        case 24: return &e[20];
        case 12: return &e[21];
        case 15: return &e[22];
        case 16: return &e[23];
        case 17: return &e[24];
        case 20: return &e[25];
        case 21: return &e[26];
        case 62: return &e[27];
        case 63: return &e[28];
        case 18: return &e[29];
        case 19: return &e[30];
        case 22: return &e[31];
        case 23: return &e[32];
        default: return nullptr;
    }
}

const SPLB32::OpEntry SPLB32::T_execute[256] = {
    {&SPLB32::py_brk, 0},
    {&SPLB32::py_ora_ind_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ora_zp, 0},
    {&SPLB32::py_asl_zp, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_php, 0},
    {&SPLB32::py_ora_imm, 0},
    {&SPLB32::py_asl_a, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ora_abs, 0},
    {&SPLB32::py_asl_abs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bpl, 0},
    {&SPLB32::py_ora_ind_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ora_zp_x, 0},
    {&SPLB32::py_asl_zp_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_clc, 0},
    {&SPLB32::py_ora_abs_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ora_abs_x, 0},
    {&SPLB32::py_asl_abs_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_jsr_abs, 0},
    {&SPLB32::py_and_ind_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bit_zp, 0},
    {&SPLB32::py_and_zp, 0},
    {&SPLB32::py_rol_zp, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_plp, 0},
    {&SPLB32::py_and_imm, 0},
    {&SPLB32::py_rol_a, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bit_abs, 0},
    {&SPLB32::py_and_abs, 0},
    {&SPLB32::py_rol_abs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bmi, 0},
    {&SPLB32::py_and_ind_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_and_zp_x, 0},
    {&SPLB32::py_rol_zp_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sec, 0},
    {&SPLB32::py_and_abs_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_and_abs_x, 0},
    {&SPLB32::py_rol_abs_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_rti, 0},
    {&SPLB32::py_eor_ind_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_eor_zp, 0},
    {&SPLB32::py_lsr_zp, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_pha, 0},
    {&SPLB32::py_eor_imm, 0},
    {&SPLB32::py_lsr_a, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_jmp_abs, 0},
    {&SPLB32::py_eor_abs, 0},
    {&SPLB32::py_lsr_abs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bvc, 0},
    {&SPLB32::py_eor_ind_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_eor_zp_x, 0},
    {&SPLB32::py_lsr_zp_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cli, 0},
    {&SPLB32::py_eor_abs_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_eor_abs_x, 0},
    {&SPLB32::py_lsr_abs_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_rts, 0},
    {&SPLB32::py_adc_ind_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_adc_zp, 0},
    {&SPLB32::py_ror_zp, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_pla, 0},
    {&SPLB32::py_adc_imm, 0},
    {&SPLB32::py_ror_a, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_jmp_ind, 0},
    {&SPLB32::py_adc_abs, 0},
    {&SPLB32::py_ror_abs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bvs, 0},
    {&SPLB32::py_adc_ind_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_adc_zp_x, 0},
    {&SPLB32::py_ror_zp_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sei, 0},
    {&SPLB32::py_adc_abs_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_adc_abs_x, 0},
    {&SPLB32::py_ror_abs_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sta_ind_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sty_zp, 0},
    {&SPLB32::py_sta_zp, 0},
    {&SPLB32::py_stx_zp, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dey, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_txa, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sty_abs, 0},
    {&SPLB32::py_sta_abs, 0},
    {&SPLB32::py_stx_abs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bcc, 0},
    {&SPLB32::py_sta_ind_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sty_zp_x, 0},
    {&SPLB32::py_sta_zp_x, 0},
    {&SPLB32::py_stx_zp_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_tya, 0},
    {&SPLB32::py_sta_abs_y, 0},
    {&SPLB32::py_txs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sta_abs_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ldy_imm, 0},
    {&SPLB32::py_lda_ind_x, 0},
    {&SPLB32::py_ldx_imm, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ldy_zp, 0},
    {&SPLB32::py_lda_zp, 0},
    {&SPLB32::py_ldx_zp, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_tay, 0},
    {&SPLB32::py_lda_imm, 0},
    {&SPLB32::py_tax, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ldy_abs, 0},
    {&SPLB32::py_lda_abs, 0},
    {&SPLB32::py_ldx_abs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bcs, 0},
    {&SPLB32::py_lda_ind_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ldy_zp_x, 0},
    {&SPLB32::py_lda_zp_x, 0},
    {&SPLB32::py_ldx_zp_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_clv, 0},
    {&SPLB32::py_lda_abs_y, 0},
    {&SPLB32::py_tsx, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_ldy_abs_x, 0},
    {&SPLB32::py_lda_abs_x, 0},
    {&SPLB32::py_ldx_abs_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cpy_imm, 0},
    {&SPLB32::py_cmp_ind_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cpy_zp, 0},
    {&SPLB32::py_cmp_zp, 0},
    {&SPLB32::py_dec_zp, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_iny, 0},
    {&SPLB32::py_cmp_imm, 0},
    {&SPLB32::py_dex, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cpy_abs, 0},
    {&SPLB32::py_cmp_abs, 0},
    {&SPLB32::py_dec_abs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_bne, 0},
    {&SPLB32::py_cmp_ind_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cmp_zp_x, 0},
    {&SPLB32::py_dec_zp_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cld, 0},
    {&SPLB32::py_cmp_abs_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cmp_abs_x, 0},
    {&SPLB32::py_dec_abs_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cpx_imm, 0},
    {&SPLB32::py_sbc_ind_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cpx_zp, 0},
    {&SPLB32::py_sbc_zp, 0},
    {&SPLB32::py_inc_zp, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_inx, 0},
    {&SPLB32::py_sbc_imm, 0},
    {&SPLB32::py_nop, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_cpx_abs, 0},
    {&SPLB32::py_sbc_abs, 0},
    {&SPLB32::py_inc_abs, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_beq, 0},
    {&SPLB32::py_sbc_ind_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sbc_zp_x, 0},
    {&SPLB32::py_inc_zp_x, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sed, 0},
    {&SPLB32::py_sbc_abs_y, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_dummy, 0},
    {&SPLB32::py_sbc_abs_x, 0},
    {&SPLB32::py_inc_abs_x, 0},
    {&SPLB32::py_dummy, 0},
};
