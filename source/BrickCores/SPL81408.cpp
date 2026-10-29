#include "SPL81408.h"

#include <cstring>

namespace {

constexpr uint32_t kVectorReset = 0x7FFC;
constexpr uint32_t kVectorIrq = 0x7FFE;

constexpr uint32_t kLcdOffset = 0x20, kLcdSize = 0x1D;
constexpr uint32_t kRamOffset = 0x60, kRamSize = 0xA0;
constexpr uint32_t kSfrSize = 0x20;

// interrupt request / enable bits (IO 0x0D)
constexpr uint8_t kIntExt = 0x01, kIntClk2k = 0x02, kIntClk128 = 0x04, kIntTimeBaseL = 0x08,
                  kIntTimeBaseH = 0x10, kIntTimer0 = 0x40;
// wake-up sources (IO 0x08)
constexpr uint8_t kWakePortEF = 0x01, kWakeTimeBaseL = 0x02, kWakeTimer0 = 0x04, kWakeTimeBaseH = 0x08;

constexpr uint8_t kTimerEnable = 0x80, kTimerCounterMode = 0x10;
constexpr uint8_t kLcdCtrlState = 0x0C, kLcdCtrlAllOff = 0x08;
constexpr uint8_t kAudioEnable = 0x80, kAudioTone = 0x40;

}  // namespace

void SPL81408::reset() {
    if (!options_loaded_) {
        static const char* kNames[3] = {"CD", "EF", "AB"};
        for (int i = 0; i < 3; i++) pullup_[i] = (uint8_t)options().num(std::string("port_pullup.") + kNames[i], 0);
        options_loaded_ = true;
    }
    t2khz_counter_ = 0;
    t2khz_ = 0;
    tm0_ = 0;
    tm0_reload_ = 0;
    sp_ = a_ = x_ = 0;
    set_ps(0x04);
    cpu_enbl_ = 1;
    ram_.assign(kRamSize, 0);
    lcd_.assign(kLcdSize, 0);
    rom_bank_ = wakeup_ctrl_ = wakeup_req_ = time_base_ = timer_ctrl_ = 0;
    cpu_ctrl_ = 0;
    cpu_div_ = 2;
    clk32768_disable_ = 0;
    memset(pdir_, 0, sizeof(pdir_));
    memset(platch_, 0, sizeof(platch_));
    int_ctrl_ = ireq_ = sys_ctrl_ = lcd_ctrl_ = audio_ctrl_ = audio_data_ = 0;
    // (Port inputs and the sound channel's state survive a reset, as in
    // SPL81408.reset().)
    pc_ = rom_word(kVectorReset);
}

uint8_t SPL81408::display_byte(int index) const {
    if (index < 0 || index >= (int)kLcdSize) return 0;
    if (!cpu_enbl_ && clk32768_disable_) return 0;
    if ((lcd_ctrl_ & kLcdCtrlState) == 0) return lcd_[index];
    if ((lcd_ctrl_ & kLcdCtrlAllOff) == 0) return 0xFF;
    return 0;
}

int SPL81408::port_id(const std::string& name) const {
    if (name == "CD") return kCD;
    if (name == "EF") return kEF;
    if (name == "AB") return kAB;
    return -1;
}

uint8_t SPL81408::port_read(int port) const {
    return (uint8_t)((pdir_[port] & platch_[port]) |
                     (~pdir_[port] & (~input_low_[port] & (input_high_[port] | pullup_[port]))));
}

void SPL81408::wake(uint8_t source) {
    if (!cpu_enbl_) {
        wakeup_req_ |= source;
        cpu_enbl_ |= wakeup_ctrl_ & source;
    }
}

void SPL81408::port_input(int port, uint8_t mask, int level) {
    if (port < kCD || port > kAB) return;
    const uint8_t prev = port_read(port);
    input_low_[port] &= ~mask;
    input_high_[port] &= ~mask;
    if (level == 0) input_low_[port] |= mask;
    else if (level > 0) input_high_[port] |= mask;
    const uint8_t now = port_read(port);
    if (port == kCD && (mask & 0x2) && (prev & 0x2) > (now & 0x2)) ireq_ |= kIntExt;
    if (port == kEF && prev != now) {
        ireq_ |= kIntExt;
        wake(kWakePortEF);
    }
}

// --- memory and I/O --------------------------------------------------------------

uint8_t SPL81408::io_read(uint8_t reg) {
    switch (reg) {
        case 0x00: return pdir_[kCD];
        case 0x04: return port_read(kCD);
        case 0x05: return port_read(kEF);
        case 0x06: return pdir_[kEF];
        case 0x07: return rom_bank_;
        case 0x08: return wakeup_req_;
        case 0x09: return sys_ctrl_;
        case 0x0A: return (uint8_t)t2khz_;  // (BrickEmuPy returns the unbounded count)
        case 0x0B: return timer_ctrl_;
        case 0x0C: return clk32768_disable_;
        case 0x0D: return ireq_;
        case 0x0E: return cpu_ctrl_;
        case 0x10: return (uint8_t)((tm0_ / kTickHz) & 0xFF);
        case 0x11: return (uint8_t)((tm0_ / kTickHz) >> 8);
        case 0x18: return lcd_ctrl_;
        case 0x1B: return audio_ctrl_;
        default: return 0;  // 0x01, 0x03, 0x0F, 0x12, 0x19, 0x1A, 0x1E, 0x1F, unmapped
    }
}

void SPL81408::io_write(uint8_t reg, uint8_t v) {
    switch (reg) {
        case 0x00: pdir_[kCD] = v & 0x0F; break;
        case 0x01: audio_data_ = v; sound_set_data(audio_ctrl_, v); break;
        case 0x04: platch_[kCD] = v; break;
        case 0x05: platch_[kEF] = v; break;
        case 0x06: pdir_[kEF] = v; break;
        case 0x07: rom_bank_ = v; break;
        case 0x08: wakeup_ctrl_ = v; wakeup_req_ = 0; break;
        case 0x09: cpu_enbl_ = 0; sys_ctrl_ = v; break;  // sleep
        case 0x0A: time_base_ = v; break;
        case 0x0B: timer_ctrl_ = v; break;
        case 0x0C: clk32768_disable_ = v; break;
        case 0x0D: int_ctrl_ = v; ireq_ = 0; break;
        case 0x0E: cpu_ctrl_ = v; cpu_div_ = 2u << (v < 30 ? v : 30); break;
        case 0x10: tm0_reload_ = (uint16_t)((tm0_reload_ & 0xFF00) | v); break;
        case 0x11: tm0_reload_ = (uint16_t)((tm0_reload_ & 0x00FF) | (v << 8)); break;
        case 0x12: tm0_ = (int64_t)tm0_reload_ * kTickHz; break;
        case 0x18: lcd_ctrl_ = v; break;
        case 0x1A: ireq_ = 0; break;
        case 0x1B: audio_ctrl_ = v; break;
        default: break;
    }
}

uint8_t SPL81408::read_mem(uint32_t addr) {
    if (addr >= kLcdOffset && addr < kLcdOffset + kLcdSize) return lcd_[addr - kLcdOffset];
    if (addr >= kRamOffset && addr < kRamOffset + kRamSize) return ram_[addr - kRamOffset];
    if (addr < kSfrSize) return io_read((uint8_t)addr);
    return rom_byte(rom_bank_ ? addr : (addr & 0x7FFF));
}

void SPL81408::write_mem(uint32_t addr, uint8_t value) {
    if (addr >= kLcdOffset && addr < kLcdOffset + kLcdSize) lcd_[addr - kLcdOffset] = value;
    else if (addr >= kRamOffset && addr < kRamOffset + kRamSize) ram_[addr - kRamOffset] = value;
    else if (addr < kSfrSize) io_write((uint8_t)addr, value);
}

// --- sound -----------------------------------------------------------------------

void SPL81408::sound_set_data(uint8_t ctrl, uint8_t data) {
    if (!(ctrl & kAudioEnable)) {
        set_speaker(false);
    } else if (ctrl & kAudioTone) {
        envelope_ = data;  // square-wave amplitude, toggled by timer 0
    } else {
        set_speaker(data > 128);  // DAC level (data / 128 - 1), as a 1-bit speaker
    }
}

void SPL81408::sound_toggle() {
    toggle_state_ = (int8_t)-toggle_state_;
    set_speaker(toggle_state_ > 0 && envelope_ > 0);
}

// --- timers ------------------------------------------------------------------------

void SPL81408::timers_clock(int64_t e) {
    if (!(timer_ctrl_ & kTimerCounterMode) && (timer_ctrl_ & kTimerEnable)) {
        tm0_ += e;
        while (tm0_ > (int64_t)0xFFFF * kTickHz) {
            tm0_ -= (int64_t)(0x10000 - tm0_reload_) * kTickHz;
            ireq_ |= kIntTimer0;
            wake(kWakeTimer0);
            if (audio_ctrl_ == 0xC0) sound_toggle();
        }
    }

    // 2048 Hz time base: one tick per 16 periods of the 32768 Hz sub clock.
    t2khz_counter_ -= e;
    while (t2khz_counter_ <= 0) {
        t2khz_counter_ += (int64_t)clock_hz() * 16;
        t2khz_++;
        ireq_ |= kIntClk2k;
        if (t2khz_ % (2048 / 128) == 0) {
            ireq_ |= kIntClk128;
            if (t2khz_ % (2048u >> ((time_base_ & 0x03) + 2)) == 0) {
                ireq_ |= kIntTimeBaseH;
                wake(kWakeTimeBaseH);
                if (t2khz_ % (2048u >> ((time_base_ & 0x80) == 0)) == 0) {
                    ireq_ |= kIntTimeBaseL;
                    wake(kWakeTimeBaseL);
                }
            }
        }
    }
}

// --- CPU -----------------------------------------------------------------------------

uint8_t SPL81408::ps() const {
    return (uint8_t)((nf_ << 7) | (vf_ << 6) | (bf_ << 4) | (df_ << 3) | (if_ << 2) | (zf_ << 1) | cf_);
}

void SPL81408::set_ps(uint8_t p) {
    nf_ = p >> 7;
    vf_ = (p & 0x40) != 0;
    bf_ = (p & 0x10) != 0;
    df_ = (p & 0x08) != 0;
    if_ = (p & 0x04) != 0;
    zf_ = (p & 0x02) != 0;
    cf_ = p & 0x01;
}

// The stack is in page 0: SP addresses memory directly, post-decrement.
void SPL81408::push(uint8_t v) {
    write_mem(sp_, v);
    sp_--;
}

uint8_t SPL81408::pull() {
    sp_++;
    return read_mem(sp_);
}

void SPL81408::irq() {
    if (if_) return;
    push((uint8_t)(pc_ >> 8));
    push((uint8_t)pc_);
    push(ps());
    if_ = 1;
    pc_ = rom_word(kVectorIrq);
}

void SPL81408::adc(uint8_t op) {
    const int a = a_;
    int r = a + op + cf_;
    if (df_ && (a & 0x0F) + (op & 0x0F) + cf_ > 9) r += 6;
    vf_ = ((~(a ^ op) & (a ^ r)) >> 7) & 1;
    nf_ = (r >> 7) & 1;
    if (df_ && r > 0x99) r += 0x60;
    zf_ = (r & 0xFF) == 0;
    cf_ = r > 255;
    a_ = (uint8_t)r;
}

void SPL81408::sbc(uint8_t op) {
    const int a = a_;
    const int borrow = !cf_;
    int r = a - op - borrow;
    if (df_) {
        if ((a & 0x0F) - (op & 0x0F) - borrow < 0) r -= 6;
        if (r < 0) r -= 0x60;
    }
    vf_ = (((a ^ op) & (a ^ r)) >> 7) & 1;
    nf_ = (r >> 7) & 1;
    zf_ = (r & 0xFF) == 0;
    cf_ = r >= 0;
    a_ = (uint8_t)r;
}

void SPL81408::compare(int reg, uint8_t op) {
    const int t = reg - op;
    nf_ = (t >> 7) & 1;
    zf_ = t == 0;
    cf_ = t >= 0;
}

int SPL81408::branch(bool taken) {
    if (!taken) {
        pc_ = (pc_ + 1) & 0xFFFF;
        return 2;
    }
    const uint8_t off = rom_byte(pc_);
    pc_ = (pc_ + 1) & 0xFFFF;
    const uint32_t prev = pc_;
    pc_ = (pc_ + off - ((off & 0x80) << 1)) & 0xFFFF;
    return 3 + ((pc_ ^ prev) > 255);
}

int64_t SPL81408::execute_scaled() {
    if (!cpu_enbl_) {
        // Asleep: one 32768 Hz sub-clock period, timers only if it runs.
        if (!clk32768_disable_) timers_clock(clock_hz());
        return clock_hz();
    }

    const uint8_t op = rom_byte(pc_);
    pc_ = (pc_ + 1) & 0xFFFF;
    auto imm = [&]() {
        const uint8_t v = rom_byte(pc_);
        pc_ = (pc_ + 1) & 0xFFFF;
        return v;
    };
    auto abs = [&]() {
        const uint16_t v = rom_word(pc_);
        pc_ = (pc_ + 2) & 0xFFFF;
        return v;
    };
    int c = 2;

    switch (op) {
        case 0x00: bf_ = 1; pc_ = (pc_ + 1) & 0xFFFF; irq(); c = 7; break;               // BRK
        case 0x02: set_ps(pull()); pc_ = pull(); pc_ |= pull() << 8; c = 6; break;        // RTI
        case 0x05: a_ |= read_mem(imm()); nz(a_); c = 3; break;                          // ORA zp
        case 0x07: a_ ^= read_mem(imm()); nz(a_); c = 3; break;                          // EOR zp
        case 0x08: c = branch(!nf_); break;                                              // BPL
        case 0x0A: c = branch(!vf_); break;                                              // BVC
        case 0x0F: a_ ^= read_mem((imm() + x_) & 0xFF); nz(a_); c = 4; break;            // EOR zp,X
        case 0x10: {                                                                      // JSR abs
            const uint32_t ret = (pc_ + 1) & 0xFFFF;
            push((uint8_t)(ret >> 8));
            push((uint8_t)ret);
            pc_ = rom_word(pc_);
            c = 6;
            break;
        }
        case 0x11: {                                                                      // BIT zp
            const uint8_t m = read_mem(imm());
            nf_ = m >> 7;
            vf_ = (m & 0x40) != 0;
            zf_ = (a_ & m) == 0;
            c = 3;
            break;
        }
        case 0x12: pc_ = pull(); pc_ |= pull() << 8; pc_ += 1; c = 6; break;              // RTS (unmasked, as the .py)
        case 0x15: a_ &= read_mem(imm()); nz(a_); c = 3; break;                          // AND zp
        case 0x17: adc(read_mem(imm())); c = 3; break;                                   // ADC zp
        case 0x18: c = branch(nf_); break;                                               // BMI
        case 0x1A: c = branch(vf_); break;                                               // BVS
        case 0x24: {                                                                      // STA (zp,X)
            const uint8_t zp = imm();
            const uint32_t addr = read_mem((zp + x_) & 0xFF) | (read_mem((zp + x_ + 1) & 0xFF) << 8);
            write_mem(addr, a_);
            c = 6;
            break;
        }
        case 0x25: write_mem(imm(), a_); c = 3; break;                                   // STA zp
        case 0x27: compare(a_, read_mem(imm())); c = 3; break;                           // CMP zp
        case 0x28: c = branch(!cf_); break;                                              // BCC
        case 0x2A: c = branch(!zf_); break;                                              // BNE
        case 0x2D: write_mem((imm() + x_) & 0xFF, a_); c = 4; break;                     // STA zp,X
        case 0x2F: compare(a_, read_mem((imm() + x_) & 0xFF)); c = 4; break;             // CMP zp,X
        case 0x32: compare(x_, imm()); c = 2; break;                                     // CPX #
        case 0x33: compare(x_, read_mem(imm())); c = 3; break;                           // CPX zp
        case 0x34: {                                                                      // LDA (zp,X)
            const uint32_t zp = imm() + x_;
            a_ = read_mem(read_mem(zp & 0xFF) | (read_mem((zp + 1) & 0xFF) << 8));
            nz(a_);
            c = 6;
            break;
        }
        case 0x35: a_ = read_mem(imm()); nz(a_); c = 3; break;                           // LDA zp
        case 0x37: sbc(read_mem(imm())); c = 3; break;                                   // SBC zp
        case 0x38: c = branch(cf_); break;                                               // BCS
        case 0x3A: c = branch(zf_); break;                                               // BEQ
        case 0x3D: a_ = read_mem((imm() + x_) & 0xFF); nz(a_); c = 4; break;             // LDA zp,X
        case 0x40: push(ps()); c = 3; break;                                             // PHP
        case 0x42: push(a_); c = 3; break;                                               // PHA
        case 0x43: pc_ = rom_word(pc_); c = 3; break;                                    // JMP abs
        case 0x44: a_ |= imm(); nz(a_); c = 2; break;                                    // ORA #
        case 0x46: a_ ^= imm(); nz(a_); c = 2; break;                                    // EOR #
        case 0x48: cf_ = 0; break;                                                       // CLC
        case 0x4A: if_ = 0; break;                                                       // CLI
        case 0x50: set_ps(pull()); c = 4; break;                                         // PLP
        case 0x51: {                                                                      // BIT abs
            const uint8_t m = read_mem(abs());
            nf_ = m >> 7;
            vf_ = (m & 0x40) != 0;
            zf_ = (a_ & m) == 0;
            c = 4;
            break;
        }
        case 0x52: a_ = pull(); nz(a_); c = 4; break;                                    // PLA
        case 0x53: {                                                                      // JMP (abs)
            const uint32_t addr = rom_word(pc_);
            pc_ = read_mem(addr) | (read_mem((addr + 1) & 0xFFFF) << 8);
            c = 6;
            break;
        }
        case 0x54: a_ &= imm(); nz(a_); c = 2; break;                                    // AND #
        case 0x56: adc(imm()); c = 2; break;                                             // ADC #
        case 0x58: cf_ = 1; break;                                                       // SEC
        case 0x5A: if_ = 1; break;                                                       // SEI
        case 0x66: compare(a_, imm()); c = 2; break;                                     // CMP #
        case 0x72: x_++; nz(x_); break;                                                  // INX
        case 0x74: a_ = imm(); nz(a_); c = 2; break;                                     // LDA #
        case 0x75: a_ = read_mem(abs()); nz(a_); c = 4; break;                           // LDA abs
        case 0x76: sbc(imm()); c = 2; break;                                             // SBC #
        case 0x78: vf_ = 0; break;                                                       // CLV
        case 0x7A: df_ = 1; break;                                                       // SED
        case 0x7D: {                                                                      // LDA abs,X
            const uint32_t base = abs();
            const uint32_t addr = (base + x_) & 0xFFFF;
            a_ = read_mem(addr);
            nz(a_);
            c = 4 + ((base ^ addr) > 255);
            break;
        }
        case 0x91: {                                                                      // ROL zp
            const uint8_t zp = imm();
            const int r = (read_mem(zp) << 1) | cf_;
            write_mem(zp, (uint8_t)r);
            nf_ = (r & 0x80) != 0;
            zf_ = (r & 0xFF) == 0;
            cf_ = r > 0xFF;
            c = 5;
            break;
        }
        case 0x93: {                                                                      // ROR zp
            const uint8_t zp = imm();
            const uint8_t prev = read_mem(zp);
            write_mem(zp, (uint8_t)((prev >> 1) | (cf_ << 7)));
            nf_ = cf_;
            zf_ = ((prev & 0xFE) | cf_) == 0;
            cf_ = prev & 1;
            c = 5;
            break;
        }
        case 0xA1: write_mem(imm(), x_); c = 3; break;                                   // STX zp
        case 0xA3: { const uint8_t zp = imm(); const uint8_t r = read_mem(zp) - 1; write_mem(zp, r); nz(r); c = 5; break; }  // DEC zp
        case 0xAB: { const uint8_t zp = (imm() + x_) & 0xFF; const uint8_t r = read_mem(zp) - 1; write_mem(zp, r); nz(r); c = 6; break; }  // DEC zp,X
        case 0xB0: x_ = imm(); nz(x_); c = 2; break;                                     // LDX #
        case 0xB1: x_ = read_mem(imm()); nz(x_); c = 3; break;                           // LDX zp
        case 0xB3: { const uint8_t zp = imm(); const uint8_t r = read_mem(zp) + 1; write_mem(zp, r); nz(r); c = 5; break; }  // INC zp
        case 0xD0: {                                                                      // ROL A
            const int r = (a_ << 1) | cf_;
            a_ = (uint8_t)r;
            nf_ = (r & 0x80) != 0;
            zf_ = (r & 0xFF) == 0;
            cf_ = r > 0xFF;
            break;
        }
        case 0xD2: {                                                                      // ROR A
            const uint8_t prev = a_;
            a_ = (uint8_t)((prev >> 1) | (cf_ << 7));
            nf_ = cf_;
            zf_ = ((prev & 0xFE) | cf_) == 0;
            cf_ = prev & 1;
            break;
        }
        case 0xE0: a_ = x_; nz(a_); break;                                               // TXA
        case 0xE1: write_mem(abs(), x_); c = 4; break;                                   // STX abs
        case 0xE2: x_--; nz(x_); break;                                                  // DEX
        case 0xE8: sp_ = x_; break;                                                      // TXS
        case 0xF0: x_ = a_; nz(x_); break;                                               // TAX
        case 0xF1: x_ = read_mem(abs()); nz(x_); c = 4; break;                           // LDX abs
        case 0xF2: break;                                                                // NOP
        case 0xF8: x_ = sp_; nz(x_); break;                                              // TSX
        default: break;  // unimplemented in BrickEmuPy (it stops there) - 2-cycle no-op here
    }

    const int64_t e = (int64_t)c * cpu_div_ * kTickHz;
    timers_clock(e);
    if (!if_ && (ireq_ & int_ctrl_)) irq();
    return e;
}

void SPL81408::serialize(BrickStateIO& io) {
    io.pod(pc_);
    io.pod(sp_);
    io.pod(a_);
    io.pod(x_);
    uint8_t flags[7] = {nf_, vf_, bf_, df_, if_, zf_, cf_};
    io.bytes(flags, sizeof(flags));
    nf_ = flags[0]; vf_ = flags[1]; bf_ = flags[2]; df_ = flags[3]; if_ = flags[4]; zf_ = flags[5]; cf_ = flags[6];
    io.pod(cpu_div_);
    io.pod(cpu_enbl_);
    io.vec(ram_);
    io.vec(lcd_);
    uint8_t regs[15] = {rom_bank_, wakeup_ctrl_, wakeup_req_, time_base_, timer_ctrl_, cpu_ctrl_, clk32768_disable_,
                        int_ctrl_, ireq_, sys_ctrl_, lcd_ctrl_, audio_ctrl_, audio_data_, envelope_, (uint8_t)toggle_state_};
    io.bytes(regs, sizeof(regs));
    rom_bank_ = regs[0]; wakeup_ctrl_ = regs[1]; wakeup_req_ = regs[2]; time_base_ = regs[3]; timer_ctrl_ = regs[4];
    cpu_ctrl_ = regs[5]; clk32768_disable_ = regs[6]; int_ctrl_ = regs[7]; ireq_ = regs[8]; sys_ctrl_ = regs[9];
    lcd_ctrl_ = regs[10]; audio_ctrl_ = regs[11]; audio_data_ = regs[12]; envelope_ = regs[13];
    toggle_state_ = (int8_t)regs[14];
    io.bytes(pdir_, sizeof(pdir_));
    io.bytes(platch_, sizeof(platch_));
    io.bytes(input_low_, sizeof(input_low_));
    io.bytes(input_high_, sizeof(input_high_));
    io.pod(t2khz_counter_);
    io.pod(t2khz_);
    io.pod(tm0_);
    io.pod(tm0_reload_);
}
