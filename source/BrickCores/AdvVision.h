#pragma once

#include <memory>

#include "BrickCore.h"
#include "Cop411.h"

class mcs48_cpu;

// Entex Adventure Vision (1982): INS8048 at 11 MHz (1 KB internal ROM, the
// BIOS), 4 KB cartridge, 1 KB external RAM, a COP411 for sound (1-bit
// speaker), 40 red LEDs and a spinning mirror that make a 150x40 picture
// (drawn here 480 columns wide, as MAME). Written for Yokoi after MAME's
// src/mame/entex/advision.cpp (license:BSD-3-Clause, Dan Boris, hap); the
// CPU is MAME's mcs48 core (mcs48.cpp/h), the sound CPU Yokoi's Cop411.
//
// Container (".brk", CONVERT_ROM/build_advision_pack.py): core "I8048",
// clock 733333 (11 MHz / 15). Blobs: "rom" the 8048 ROM (1 KB), "snd" the
// COP411 ROM (512 bytes), "cart" the cartridge. Input: port "JOY", mask =
// MAME's JOY bit, level 1 = pressed. Display: lit(x, y), 0 <= x < kW, y < 40.
class AdvVision : public BrickCore {
public:
    static constexpr int kW = 480, kH = 40;

    AdvVision();
    ~AdvVision();

    bool get_segments_state(uint8_t, uint8_t, uint8_t) override { return false; }
    const uint8_t* picture() const { return &frame_[0][0]; }   // kH rows of kW, 0/1

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int) const override { return 0; }
    int port_id(const std::string& name) const override { return name == "JOY" ? 0 : -1; }
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;

private:
    uint8_t program_r(uint16_t a) const;
    uint8_t ext_ram_r(uint8_t offset);
    void av_control_w(uint8_t data);
    uint8_t controller_r() const;
    void led_update();

    std::unique_ptr<mcs48_cpu> cpu_;
    Cop411 snd_;
    std::vector<uint8_t> bios_, cart_;
    uint8_t ext_ram_[0x400] = {};
    uint16_t rambank_ = 0;
    bool ea_ = false;
    uint8_t joy_ = 0xff;
    uint8_t sound_cmd_ = 0;
    bool snd_reset_ = true;

    bool video_strobe_ = false, video_enable_ = false;
    uint8_t video_bank_ = 0;
    uint8_t led_output_[5] = {}, led_latch_[5] = {};
    uint8_t display_[kH][kW] = {};   // being drawn
    uint8_t frame_[kH][kW] = {};     // last finished picture

    // time, in ns
    int64_t t_frame_ = 0;            // since this frame (mirror half turn) began
    int64_t t_led_next_ = 0;         // next led column
    int64_t t_led_off_ = -1;         // led_off timer, -1 = not armed
    int hpos_ = 0;
    int64_t snd_acc_ = 0;            // COP411 time owed, in ns
};
