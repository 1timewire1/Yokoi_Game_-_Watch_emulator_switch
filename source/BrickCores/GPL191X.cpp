#include "GPL191X.h"

#include <cmath>
#include <cstring>

namespace {

constexpr int64_t kSubClock = 32768;

constexpr int64_t kVaddrNmi = 0x7FFA;
constexpr int64_t kVaddrReset = 0x7FFC;
constexpr int64_t kVaddrIrq = 0x7FFE;

constexpr int64_t kTimerCtrlEnable = 0x80;
constexpr int64_t kTimerCtrlTm0CounterMode = 0x10;
constexpr int64_t kTimerCtrlTm0Timer1 = 0x20;
constexpr int64_t kTimerCtrlTm1_32k = 0x40;
constexpr int64_t kCnt0SrcBMask = 0x0C;
constexpr int64_t kCnt0SrcBVdd = 0x04;
constexpr int64_t kCnt0SrcBTimeBaseH = 0x08;
constexpr int64_t kCnt0SrcBClk128 = 0x0C;
constexpr int64_t kCnt0SrcAMask = 0x03;
constexpr int64_t kCnt0SrcAVdd = 0x01;
constexpr int64_t kCnt0SrcARosc = 0x02;
constexpr int64_t kCnt0SrcAClk32 = 0x03;

constexpr int64_t kWakeupExt = 0x01;
constexpr int64_t kWakeupTimeBaseL = 0x02;
constexpr int64_t kWakeupTimer0 = 0x04;
constexpr int64_t kWakeupTimeBaseH = 0x08;

constexpr int64_t kIntExt = 0x01;
constexpr int64_t kIntClk2k = 0x02;
constexpr int64_t kIntClk128 = 0x04;
constexpr int64_t kIntTimeBaseL = 0x08;
constexpr int64_t kIntTimeBaseH = 0x10;
constexpr int64_t kIntTimer1 = 0x20;
constexpr int64_t kIntTimer0 = 0x40;

constexpr int64_t kNmiCtrlTimer1 = 0x01;

constexpr int64_t kTimeBaseLMask = 0x80;
constexpr int64_t kTimeBaseLShift = 7;
constexpr int64_t kTimeBaseHMask = 0x03;
constexpr int64_t kTimeBaseHTbl[4] = {0x0F, 0x07, 0x03, 0x01};
constexpr int64_t kTimeBaseLTbl[2] = {0x1F, 0x3F};

constexpr int64_t kAudioEnable = 0x80;
constexpr int64_t kAudioToneMode = 0x40;

constexpr int64_t kCpuDivDefault = 0x02;
constexpr int64_t kLcdCtrlOn = 0x02;

constexpr int64_t kCpuRamOffset = 0x40;
constexpr int64_t kDpramOffset = 0x300;
constexpr int64_t kDpramSize = 0x80;

}  // namespace

// --- setup ---------------------------------------------------------------------

void GPL191X::reset() {
    if (mem_.empty()) {
        // The ROM array doubles as RAM (0x40..0x37F are writable), exactly as
        // BrickEmuPy's self._mem - so it's copied once, not on every reset.
        mem_ = rom();
        if (mem_.empty()) mem_.assign(1, 0);
        mem_size_ = mem_.size();
        sub_clock_div_ = (double)clock_hz() / (double)kSubClock;
        static const char* kNames[3] = {"AB", "CD", "EF"};
        for (int i = 0; i < 3; i++) pullup_[i] = options().num(std::string("port_pullup.") + kNames[i], 0);
    }
    TM0_counter_ = TM0_div_ = TM1_counter_ = TM1_div_ = 0;
    T64HZ_counter_ = 0;
    T64HZ_ = 0;
    T128HZ_counter_ = T2K_counter_ = 0;
    TM0_ = TM0_PRELOAD_ = TM0_LOAD_ = 0;
    TM1_ = TM1_PRELOAD_ = TM1_LOAD_ = 0;

    PC_ = A_ = X_ = Y_ = SP_ = 0;
    set_ps(0x04);
    CPU_ENBL_ = 1;
    BANK_ = 0;
    WAKEUP_CTRL_ = WAKEUPREQ_ = ENTER_SLEEP_ = TIME_BASE_ = TIMER_CTRL_ = CLK_CPU_CTRL_ = 0;
    CPU_DIV_ = kCpuDivDefault;
    CLK32K_ENABLE_ = 0x80;
    for (int i = 0; i < 3; i++) PDIR_[i] = PLATCH_[i] = 0;
    INT_CTRL_ = INT_CLEAR_ = IREQ_ = NMI_CTRL_ = 0;
    AUDIO_CTRL_[0] = AUDIO_DATA_[0] = AUDIO_CTRL_[1] = AUDIO_DATA_[1] = 0;
    LCD_CTRL_ = 0;
    PC_ = ((int64_t)mem_[(kVaddrReset + 1) % mem_size_] << 8) | mem_[kVaddrReset % mem_size_];
}

uint8_t GPL191X::display_byte(int index) const {
    if (index < 0 || index >= kDpramSize) return 0;
    if (CLK32K_ENABLE_ && (LCD_CTRL_ & kLcdCtrlOn)) {
        const size_t at = (size_t)(kDpramOffset + index);
        return at < mem_size_ ? mem_[at] : 0;
    }
    return 0;
}

int GPL191X::port_id(const std::string& name) const {
    if (name == "AB") return kAB;
    if (name == "CD") return kCD;
    if (name == "EF") return kEF;
    return -1;
}

int64_t GPL191X::port_read(int port) const {
    return ((~PDIR_[port] & (port_in_[port][1] | (pullup_[port] & ~port_in_[port][0]))) |
            (PDIR_[port] & PLATCH_[port])) & 0xFF;
}

void GPL191X::emit_port(int port) {
    port_output(port, (uint8_t)port_read(port));
}

void GPL191X::port_input(int port, uint8_t mask, int level) {
    if (port < kAB || port > kEF) return;
    const int64_t prev = port_read(port);
    port_in_[port][0] &= ~(int64_t)mask;
    port_in_[port][1] &= ~(int64_t)mask;
    if (level >= 0) port_in_[port][level ? 1 : 0] |= mask;
    if (port == kCD) {
        if ((prev & 0x02) > (port_read(port) & 0x02)) IREQ_ |= kIntExt;
        update_timers_div();
    }
    if (port == kEF) {
        if (port_read(port) & mask) WAKEUPREQ_ |= kWakeupExt & WAKEUP_CTRL_;
    }
}

// --- CPU helpers -------------------------------------------------------------------

int64_t GPL191X::get_ps() const {
    return (NF_ << 7) | (VF_ << 6) | (BF_ << 4) | (DF_ << 3) | (IF_ << 2) | (ZF_ << 1) | CF_;
}

void GPL191X::set_ps(int64_t ps) {
    NF_ = ps >> 7;
    VF_ = (ps & 0x40) > 0;
    BF_ = (ps & 0x10) > 0;
    DF_ = (ps & 0x08) > 0;
    IF_ = (ps & 0x04) > 0;
    ZF_ = (ps & 0x02) > 0;
    CF_ = ps & 0x1;
}

void GPL191X::irq() {
    wr(SP_ | kStack, PC_ >> 8);
    SP_ = (SP_ - 1) & 0xFF;
    wr(SP_ | kStack, PC_ & 0xFF);
    SP_ = (SP_ - 1) & 0xFF;
    wr(SP_ | kStack, get_ps());
    SP_ = (SP_ - 1) & 0xFF;
    IF_ = 1;
    const int64_t addr = kVaddrIrq | (BANK_ * (PC_ & 0x8000));
    PC_ = ((int64_t)mem_[(addr + 1) % mem_size_] << 8) | mem_[addr % mem_size_];
}

void GPL191X::nmi() {
    if (!CPU_ENBL_) return;
    wr(SP_ | kStack, PC_ >> 8);
    SP_ = (SP_ - 1) & 0xFF;
    wr(SP_ | kStack, PC_ & 0xFF);
    SP_ = (SP_ - 1) & 0xFF;
    wr(SP_ | kStack, get_ps());
    SP_ = (SP_ - 1) & 0xFF;
    const int64_t addr = kVaddrNmi | (BANK_ * (PC_ & 0x8000));
    PC_ = ((int64_t)mem_[(addr + 1) % mem_size_] << 8) | mem_[addr % mem_size_];
}

int64_t GPL191X::rd(int64_t addr) {
    if (addr >= kCpuRamOffset) return mem_[(size_t)(((addr & 0x7FFF) | (BANK_ * (addr & 0x8000))) % (int64_t)mem_size_)];
    return sfr_get(addr);
}

void GPL191X::wr(int64_t addr, int64_t value) {
    if (addr >= kCpuRamOffset && addr < kDpramSize + kDpramOffset) {
        mem_[(size_t)(addr % (int64_t)mem_size_)] = (uint8_t)value;
    } else {
        sfr_set(addr, value);
    }
}

// --- SFRs ------------------------------------------------------------------------------

int64_t GPL191X::sfr_get(int64_t addr) {
    switch (addr) {
        case 0x02: return port_read(kAB);
        case 0x03: return port_read(kCD);
        case 0x04: return port_read(kEF);
        case 0x07: return BANK_;
        case 0x08: return WAKEUPREQ_;
        case 0x0A: return T64HZ_ & 0xFF;
        case 0x0D: return IREQ_;
        case 0x12: return TM1_ & 0xFF;
        case 0x13: return TM1_ >> 8;
        case 0x15: return TM0_ & 0xFF;
        case 0x16: return TM0_ >> 8;
        case 0x1A: return AUDIO_CTRL_[0];
        case 0x1B: return AUDIO_CTRL_[1];
        default: return 0;  // write-only / dummy / unmapped
    }
}

bool GPL191X::sfr_set(int64_t addr, int64_t value) {
    switch (addr) {
        case 0x00: PDIR_[kCD] = value; emit_port(kCD); update_timers_div(); break;
        case 0x01: PDIR_[kAB] = value; emit_port(kAB); break;
        case 0x02: PLATCH_[kAB] = value; emit_port(kAB); break;
        case 0x03: PLATCH_[kCD] = value; emit_port(kCD); update_timers_div(); break;
        case 0x04: PLATCH_[kEF] = value; emit_port(kEF); break;
        case 0x06: PDIR_[kEF] = value & 0xFE; emit_port(kEF); break;
        case 0x07: BANK_ = value & 0x07; break;
        case 0x08: WAKEUP_CTRL_ = value; WAKEUPREQ_ &= value; break;
        case 0x09: ENTER_SLEEP_ = value; CPU_ENBL_ = 0; break;
        case 0x0A: TIME_BASE_ = value; update_timers_div(); break;
        case 0x0B: TIMER_CTRL_ = value; update_timers_div(); break;
        case 0x0C: CLK32K_ENABLE_ = value; break;
        case 0x0D: INT_CTRL_ = value; IREQ_ = 0; break;
        case 0x10:
            AUDIO_DATA_[0] = value;
            snd_set_data(0, AUDIO_CTRL_[0], value);
            update_audio_div();
            break;
        case 0x11:
            AUDIO_DATA_[1] = value;
            snd_set_data(1, AUDIO_CTRL_[1], value);
            update_audio_div();
            break;
        case 0x12: TM1_PRELOAD_ = (TM1_PRELOAD_ & 0xFF00) | value; break;
        case 0x13: TM1_PRELOAD_ = (TM1_PRELOAD_ & 0xFF) | (value << 8); TM1_ = TM1_PRELOAD_; break;
        case 0x14: TM1_LOAD_ = value; break;
        case 0x15: TM0_PRELOAD_ = (TM0_PRELOAD_ & 0xFF00) | value; break;
        case 0x16: TM0_PRELOAD_ = (TM0_PRELOAD_ & 0xFF) | (value << 8); TM0_ = TM0_PRELOAD_; break;
        case 0x17: TM0_LOAD_ = value; break;
        case 0x18: INT_CLEAR_ = value; IREQ_ &= ~value; break;
        case 0x19: NMI_CTRL_ = value; break;
        case 0x1A: AUDIO_CTRL_[0] = value; update_audio_div(); break;
        case 0x1B: AUDIO_CTRL_[1] = value; update_audio_div(); break;
        case 0x1F: CLK_CPU_CTRL_ = value; CPU_DIV_ = (int64_t)2 << (value & 0x3); break;
        case 0x20: LCD_CTRL_ = value; break;
        default: return false;
    }
    return true;
}

// --- timers ----------------------------------------------------------------------------

void GPL191X::update_audio_div() {
    for (int ch = 0; ch < 2; ch++) {
        if ((TIMER_CTRL_ & kTimerCtrlEnable) && (AUDIO_CTRL_[ch] & kAudioEnable) && (AUDIO_CTRL_[ch] & kAudioToneMode)) {
            const double div = ch == 0 ? TM0_div_ * (double)(0x10000 - TM0_PRELOAD_) * 2
                                       : TM1_div_ * (double)(0x10000 - TM1_PRELOAD_) * 2;
            snd_tone(ch, div);
        } else {
            snd_stop(ch);
        }
    }
}

void GPL191X::update_timers_div() {
    TM1_div_ = (TIMER_CTRL_ & kTimerCtrlTm1_32k) ? sub_clock_div_ : 1;

    TM0_div_ = 0;
    if (TIMER_CTRL_ & kTimerCtrlTm0CounterMode) {
        const int64_t srcB = TIMER_CTRL_ & kCnt0SrcBMask;
        const int64_t srcA = TIMER_CTRL_ & kCnt0SrcAMask;
        const int64_t tbh = kTimeBaseHTbl[TIME_BASE_ & kTimeBaseHMask];
        if (srcA == kCnt0SrcAClk32) {
            if (srcB == kCnt0SrcBVdd) TM0_div_ = sub_clock_div_;
            else if (srcB == kCnt0SrcBTimeBaseH) TM0_div_ = sub_clock_div_ * (double)(kSubClock / 64) * (double)tbh;
            else if (srcB == kCnt0SrcBClk128) TM0_div_ = sub_clock_div_ * (double)(kSubClock / 128);
            else if (port_read(kCD) & 0x02) TM0_div_ = sub_clock_div_;
        } else if (srcA == kCnt0SrcARosc) {
            if (srcB == kCnt0SrcBVdd) TM0_div_ = 1;
            else if (srcB == kCnt0SrcBTimeBaseH) TM0_div_ = (double)((kSubClock / 64) * tbh);
            else if (srcB == kCnt0SrcBClk128) TM0_div_ = (double)(kSubClock / 128);
            else if (port_read(kCD) & 0x02) TM0_div_ = 1;
        } else if (srcA == kCnt0SrcAVdd) {
            if (srcB == kCnt0SrcBTimeBaseH) TM0_div_ = (double)((kSubClock / 64) * tbh);
            else if (srcB == kCnt0SrcBClk128) TM0_div_ = (double)(kSubClock / 128);
        } else if (port_read(kCD) & 0x01) {
            if (srcB == kCnt0SrcBTimeBaseH) TM0_div_ = (double)((kSubClock / 64) * tbh);
            else if (srcB == kCnt0SrcBClk128) TM0_div_ = (double)(kSubClock / 128);
            else if (srcB == kCnt0SrcBVdd) {
                TM0_ += 1;
                if (TM0_ > 0xFFFF) {
                    TM0_ = TM0_PRELOAD_;
                    IREQ_ |= kIntTimer0;
                    WAKEUPREQ_ |= kWakeupTimer0 & WAKEUP_CTRL_;
                }
            }
        }
    } else {
        TM0_div_ = (TIMER_CTRL_ & kTimerCtrlTm0Timer1) ? TM1_div_ * (double)(0x10000 - TM1_PRELOAD_) : 1;
    }
    update_audio_div();
}

void GPL191X::timers_clock(double exec_cycles) {
    if (TIMER_CTRL_ & kTimerCtrlEnable) {
        if (TM0_div_ != 0) {
            TM0_counter_ -= exec_cycles;
            while (TM0_counter_ <= 0) {
                TM0_counter_ += TM0_div_;
                TM0_ += 1;
                if (TM0_ > 0xFFFF) {
                    TM0_ = TM0_PRELOAD_;
                    IREQ_ |= kIntTimer0;
                    WAKEUPREQ_ |= kWakeupTimer0 & WAKEUP_CTRL_;
                }
            }
        }
        TM1_counter_ -= exec_cycles;
        while (TM1_counter_ <= 0) {
            TM1_counter_ += TM1_div_;
            TM1_ += 1;
            if (TM1_ > 0xFFFF) {
                TM1_ = TM1_PRELOAD_;
                if (!(NMI_CTRL_ & kNmiCtrlTimer1)) nmi();
                else IREQ_ |= kIntTimer1;
            }
        }
    }

    T64HZ_counter_ -= exec_cycles;
    while (T64HZ_counter_ <= 0) {
        T64HZ_counter_ += sub_clock_div_ * (double)(kSubClock / 64);
        T64HZ_ += 1;
        if (!(T64HZ_ & kTimeBaseHTbl[TIME_BASE_ & kTimeBaseHMask])) {
            IREQ_ |= kIntTimeBaseH;
            WAKEUPREQ_ |= kWakeupTimeBaseH & WAKEUP_CTRL_;
        }
        if (!(T64HZ_ & kTimeBaseLTbl[(TIME_BASE_ & kTimeBaseLMask) >> kTimeBaseLShift])) {
            IREQ_ |= kIntTimeBaseL;
            WAKEUPREQ_ |= kWakeupTimeBaseL & WAKEUP_CTRL_;
        }
    }

    T128HZ_counter_ -= exec_cycles;
    while (T128HZ_counter_ <= 0) {
        T128HZ_counter_ += sub_clock_div_ * (double)(kSubClock / 128);
        IREQ_ |= kIntClk128;
    }

    T2K_counter_ -= exec_cycles;
    while (T2K_counter_ <= 0) {
        T2K_counter_ += sub_clock_div_ * (double)(kSubClock / 2048);
        IREQ_ |= kIntClk2k;
    }
}

// --- sound (GPL191Xsound.py) ------------------------------------------------------------

void GPL191X::snd_set_data(int ch, int64_t ctrl, int64_t data) {
    if (ctrl & kAudioToneMode) {
        envelope_[ch] = (double)(data & 0xFF) / 255.0;
        if (data == 0) ch_freq_[ch] = 0;
    } else {
        // Direct (DAC) level: BrickEmuPy starts a flat "tone" here, but
        // update_audio_div() - always called right after - stops it at the
        // same instant, so nothing is ever heard.
        ch_freq_[ch] = 0;
    }
    snd_apply();
}

void GPL191X::snd_tone(int ch, double div) {
    if (envelope_[ch] != 0) {
        ch_freq_[ch] = div != 0 ? (double)clock_hz() / div : 0;
        snd_apply();
    }
}

void GPL191X::snd_stop(int ch) {
    ch_freq_[ch] = 0;
    snd_apply();
}

void GPL191X::snd_apply() {
    // One speaker: channel 0's tone wins, else channel 1's.
    set_tone(ch_freq_[0] > 0 ? ch_freq_[0] : ch_freq_[1]);
}

// --- execution ---------------------------------------------------------------------------

double GPL191X::clock_step() {
    double exec_cycles;
    if (CPU_ENBL_) {
        const int64_t pc = PC_;
        PC_ = (pc + 1) & 0xFFFF;
        const uint8_t opcode = mem_[(size_t)(((pc & 0x7FFF) | (BANK_ * (pc & 0x8000))) % (int64_t)mem_size_)];
        exec_cycles = (double)((int64_t)(this->*kOps[opcode])() * CPU_DIV_);
        timers_clock(exec_cycles);
        if (!IF_ && (IREQ_ & INT_CTRL_) && opcode != 0x58) irq();
    } else {
        exec_cycles = sub_clock_div_;
        if (WAKEUP_CTRL_ & WAKEUPREQ_) {
            CPU_ENBL_ = 1;
            CPU_DIV_ = kCpuDivDefault;
        }
        if (CLK32K_ENABLE_) timers_clock(exec_cycles);
    }
    return exec_cycles;
}

int64_t GPL191X::execute_scaled() {
    // Asleep, one step is one 32768 Hz tick = clock/32768 cycles - exactly
    // clock_hz() once scaled, so there's no rounding either way.
    return (int64_t)std::llround(clock_step() * (double)kTickHz);
}

void GPL191X::serialize(BrickStateIO& io) {
    // RAM + display RAM live in the ROM array (see reset()).
    std::vector<uint8_t> ram(mem_.begin() + std::min<size_t>(kCpuRamOffset, mem_size_),
                             mem_.begin() + std::min<size_t>(kDpramOffset + kDpramSize, mem_size_));
    io.vec(ram);
    if (!io.writing() && io.ok()) std::copy(ram.begin(), ram.end(), mem_.begin() + std::min<size_t>(kCpuRamOffset, mem_size_));
    int64_t* regs[] = {&A_, &X_, &Y_, &SP_, &PC_, &NF_, &VF_, &DF_, &BF_, &IF_, &ZF_, &CF_, &BANK_, &CPU_ENBL_, &CPU_DIV_,
                       &WAKEUP_CTRL_, &WAKEUPREQ_, &ENTER_SLEEP_, &TIME_BASE_, &TIMER_CTRL_, &CLK_CPU_CTRL_, &CLK32K_ENABLE_,
                       &INT_CTRL_, &INT_CLEAR_, &IREQ_, &NMI_CTRL_, &LCD_CTRL_, &T64HZ_,
                       &TM0_, &TM0_PRELOAD_, &TM0_LOAD_, &TM1_, &TM1_PRELOAD_, &TM1_LOAD_};
    for (int64_t* r : regs) io.pod(*r);
    io.bytes(PDIR_, sizeof(PDIR_));
    io.bytes(PLATCH_, sizeof(PLATCH_));
    io.bytes(AUDIO_CTRL_, sizeof(AUDIO_CTRL_));
    io.bytes(AUDIO_DATA_, sizeof(AUDIO_DATA_));
    io.bytes(port_in_, sizeof(port_in_));
    double* ds[] = {&TM0_counter_, &TM0_div_, &TM1_counter_, &TM1_div_, &T64HZ_counter_, &T128HZ_counter_, &T2K_counter_};
    for (double* d : ds) io.pod(*d);
    io.bytes(envelope_, sizeof(envelope_));
    io.bytes(ch_freq_, sizeof(ch_freq_));
}

// --- opcode handlers (generated) -----------------------------------------------------------

#include "GPL191X_ops.inc"
