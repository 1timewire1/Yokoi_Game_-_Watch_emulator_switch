#include "HTG12N0.h"

namespace {

constexpr double kSubClock = 32768;

}  // namespace

void HTG12N0::reset() {
    HT943::reset();
    ram_.assign(256, 0);
    vram_.assign(128, 0);
    rtc_div_ = (double)clock_hz() / kSubClock * (double)options().num("rtc_clock_div", 16384);
    rtc_counter_ = 0;
    pb_ = port_c_ = 0;
}

uint8_t& HTG12N0::ram(int rp) {
    const int index = (wr_[rp + 1] << 4) | wr_[rp];
    if (index >= 128) return vram_[index - 128];
    return ram_[(port_c_ & kRamBank1) ? index + 128 : index];
}

uint8_t HTG12N0::display_byte(int index) const {
    if (!((port_c_ & kLcdOn) || reset_held()) || index < 0 || index >= (int)vram_.size()) return 0;
    return vram_[index];
}

void HTG12N0::port_input(int port, uint8_t pin, int level) {
    if ((port != kPM && port != kPS) || pin > 3) return;
    const uint8_t bit = (uint8_t)(1u << pin);
    if (level >= 0) {
        pins_[port] = (uint8_t)((pins_[port] & ~bit) | ((level ? 1 : 0) << pin));
        if (wakeup_[port] & bit) halt_ = 0;
    } else {
        pins_[port] = (uint8_t)((pins_[port] & ~bit) | (pullup_[port] & bit));
    }
}

void HTG12N0::io_op(uint8_t op) {
    switch (op) {
        case 0x30: pa_ = acc_; break;                           // OUT PA,A
        case 0x32: acc_ = pins_[kPM]; break;                    // IN A,PM
        case 0x33: acc_ = pins_[kPS]; break;                    // IN A,PS
        case 0x34: port_c_ = acc_; break;                       // OUT PC,A
        default:                                                // 0x35: OUT PB,A
            pb_ = acc_;
            pc_ = ((uint32_t)(pb_ & 3) << 12) | (pc_ & 0xFFF);
            break;
    }
    pc_++;
}

int HTG12N0::execute() {
    const int cycles = HT943::execute();
    rtc_counter_ -= cycles;
    while (rtc_counter_ <= 0) {
        rtc_counter_ += rtc_div_;
        ef_ = 1;
        halt_ = 0;
    }
    return cycles;
}

void HTG12N0::serialize(BrickStateIO& io) {
    HT943::serialize(io);
    io.vec(vram_);
    io.pod(rtc_counter_);
    io.pod(pb_);
    io.pod(port_c_);
}
