#include "AdvVision.h"

#include <cstring>

#include "mcs48.h"

namespace {
constexpr int64_t kNsPerCycle = 1364;        // 15 / 11 MHz = 1.3636 us, in ns
constexpr int64_t kFrame = 71428571;         // 1/14 s: one half turn of the mirror
constexpr int64_t kSync = 100000;            // mirror sync pulse, 100 us
constexpr int64_t kLedStep = 20000;          // one led column, 20 us (MAME default: 480 wide)
constexpr int64_t kLedOff = 39000;           // leds go dark 39 us after bank 0
constexpr int64_t kSndCycle = 20000;         // COP411: 200 kHz / 4 = 50 kHz, 20 us
}

AdvVision::AdvVision() : BrickCore("I8048"), cpu_(new mcs48_cpu()) {
    mcs48_cpu& c = *cpu_;
    c.rd_program = [this](uint16_t a) { return program_r(a); };
    c.rd_ext = [this](uint8_t a) { return ext_ram_r(a); };
    c.wr_ext = [this](uint8_t a, uint8_t v) { ext_ram_[(rambank_ << 8) | a] = v; };
    c.rd_port = [this](int p) -> uint8_t { return p == 1 ? controller_r() : 0xff; };
    c.wr_port = [this](int p, uint8_t v) {
        if (p == 1) {            // P10,P11: RAM bank, P12: EA
            rambank_ = v & 3;
            ea_ = (v >> 2) & 1;
        } else if (p == 2) {
            av_control_w(v);
        }
    };
    c.rd_test = [this](int t) { return t == 1 ? (t_frame_ < kSync ? 0 : 1) : 0; };  // T1: mirror sync
    snd_.read_l = [this]() -> uint8_t { return sound_cmd_; };
    snd_.write_g = [this](uint8_t d) { set_speaker(d & 1); };
}

AdvVision::~AdvVision() = default;

uint8_t AdvVision::program_r(uint16_t a) const {
    if (!ea_ && a < 0x400) return a < bios_.size() ? bios_[a] : 0;
    a &= 0xfff;
    return a < cart_.size() ? cart_[a] : 0xff;
}

uint8_t AdvVision::ext_ram_r(uint8_t offset) {
    const uint8_t data = ext_ram_[(rambank_ << 8) | offset];
    if (video_bank_ > 0 && video_bank_ < 6) {
        led_latch_[5 - video_bank_] = data ^ 0xff;     // to the led latch
    } else if (video_bank_ == 6) {
        const bool rst = !(data & 1);                   // sound cpu reset line
        if (rst && !snd_reset_) set_speaker(false);
        if (!rst && snd_reset_) snd_.reset();
        snd_reset_ = rst;
    }
    return data;
}

void AdvVision::av_control_w(uint8_t data) {
    video_bank_ = data >> 5 & 7;                        // P25-P27: led latch select
    if (video_bank_ == 0 && t_led_off_ < 0) t_led_off_ = kLedOff;
    if (!video_strobe_ && (data & 0x10)) {              // P24 rising edge: latches -> leds
        memcpy(led_output_, led_latch_, sizeof(led_output_));
        t_led_off_ = -1;
        video_enable_ = true;
    }
    video_strobe_ = data & 0x10;
    sound_cmd_ = data >> 4;
}

uint8_t AdvVision::controller_r() const {
    uint8_t data = joy_;
    // some of the buttons share the same bitmask as joystick directions
    if (~data & 0x01) data &= 0xcf;
    if (~data & 0x02) data &= 0xaf;
    if (~data & 0x04) data &= 0x6f;
    return data | 0x07;
}

void AdvVision::led_update() {
    if (hpos_ >= kW) return;
    for (int y = 0; y < 8; y++)
        for (int b = 0; b < 5; b++)
            display_[b * 8 + y][hpos_] = video_enable_ ? (led_output_[b] >> (y ^ 7)) & 1 : 0;
    hpos_++;
}

void AdvVision::reset() {
    const std::vector<uint8_t>* b = rom_info().blob("rom");
    bios_ = b ? *b : std::vector<uint8_t>();
    const std::vector<uint8_t>* c = rom_info().blob("cart");
    cart_ = c ? *c : std::vector<uint8_t>();
    const std::vector<uint8_t>* s = rom_info().blob("snd");
    snd_.load(s ? *s : std::vector<uint8_t>(0x200, 0));
    memset(ext_ram_, 0, sizeof(ext_ram_));
    rambank_ = 0;
    ea_ = false;
    sound_cmd_ = 0;
    snd_reset_ = true;
    video_strobe_ = video_enable_ = false;
    video_bank_ = 0;
    memset(led_output_, 0, sizeof(led_output_));
    memset(led_latch_, 0, sizeof(led_latch_));
    memset(display_, 0, sizeof(display_));
    memset(frame_, 0, sizeof(frame_));
    t_frame_ = 0;
    t_led_next_ = 0;
    t_led_off_ = -1;
    hpos_ = 0;
    snd_acc_ = 0;
    set_speaker(false);
    cpu_->reset();
    snd_.reset();
}

int AdvVision::execute() {
    const int cycles = cpu_->run(1);
    const int64_t dt = (int64_t)cycles * kNsPerCycle;
    // led_off timer
    if (t_led_off_ >= 0) {
        t_led_off_ -= dt;
        if (t_led_off_ < 0) { video_enable_ = false; t_led_off_ = -1; }
    }
    // led columns, every kLedStep from the start of the frame
    t_frame_ += dt;
    while (t_led_next_ <= t_frame_ && hpos_ < kW) {
        led_update();
        t_led_next_ += kLedStep;
    }
    // a new half turn of the mirror: keep the finished picture, start over
    if (t_frame_ >= kFrame) {
        memcpy(frame_, display_, sizeof(frame_));
        memset(display_, 0, sizeof(display_));
        t_frame_ -= kFrame;
        hpos_ = 0;
        t_led_next_ = 0;
    }
    // sound cpu
    if (!snd_reset_) {
        snd_acc_ += dt;
        while (snd_acc_ >= kSndCycle) { snd_.step(); snd_acc_ -= kSndCycle; }
    }
    return cycles > 0 ? cycles : 1;
}

void AdvVision::port_input(int port, uint8_t mask, int level) {
    if (port != 0) return;
    if (level == 1) joy_ &= (uint8_t)~mask;   // active low
    else joy_ |= mask;
}

void AdvVision::serialize(BrickStateIO& io) {
    mcs48_cpu& c = *cpu_;
    io.pod(c.m_prevpc); io.pod(c.m_pc); io.pod(c.m_a); io.pod(c.m_psw); io.pod(c.m_f1); io.pod(c.m_a11);
    io.pod(c.m_p1); io.pod(c.m_p2); io.pod(c.m_timer); io.pod(c.m_prescaler); io.pod(c.m_t1_history);
    io.pod(c.m_sts); io.pod(c.m_dbbi); io.pod(c.m_dbbo); io.pod(c.m_irq_state); io.pod(c.m_irq_polled);
    io.pod(c.m_irq_in_progress); io.pod(c.m_timer_overflow); io.pod(c.m_timer_flag); io.pod(c.m_tirq_enabled);
    io.pod(c.m_xirq_enabled); io.pod(c.m_timecount_enabled); io.pod(c.m_flags_enabled); io.pod(c.m_dma_enabled);
    io.bytes(c.m_ram, sizeof(c.m_ram));
    snd_.serialize(io);
    io.bytes(ext_ram_, sizeof(ext_ram_));
    io.pod(rambank_); io.pod(ea_); io.pod(sound_cmd_); io.pod(snd_reset_);
    io.pod(video_strobe_); io.pod(video_enable_); io.pod(video_bank_);
    io.bytes(led_output_, sizeof(led_output_)); io.bytes(led_latch_, sizeof(led_latch_));
    io.bytes(display_, sizeof(display_)); io.bytes(frame_, sizeof(frame_));
    io.pod(t_frame_); io.pod(t_led_next_); io.pod(t_led_off_); io.pod(hpos_); io.pod(snd_acc_);
    if (!io.writing()) cpu_->update_regptr();
}
