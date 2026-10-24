#pragma once

#include <cstdint>
#include <string>

#include "SM5XX/SM5XX.h"

// Tiger R-Zone cartridge I/O (MAME src/mame/handheld/rzone.cpp, by hap with
// thanks to Sean Riddle - BSD-3-Clause). The console has 14 buttons but the
// cartridge's SM510/SM512 only reads one input pin: they all go through a
// 74165 parallel-in/serial-out shift register, which the game loads and clocks
// by toggling two of its R/S output pins and samples one bit at a time on K1.
// Plugged into the CPU as an SM5XXPortDevice (see SM5XX.h).
//
// Two cartridge wirings exist:
//   Type1 (SM510: rztoshden, rzindy500)  R1 = speaker, R2 = SH/LD,
//                                        S1 = LED,     S2 = CLK
//   Type2 (SM512: rzbatfor)              S1 = SH/LD, S2 = CLK, S3 = speaker,
//                                        S4 = LED; R (melody) = speaker too
class RZoneIO : public SM5XXPortDevice {
public:
    enum class Type { Type1, Type2 };

    // Button bits, same order as MAME's IN.0 (all active-high).
    enum Button : uint16_t {
        PowerOn = 0x0001,
        Up = 0x0002,
        Right = 0x0004,
        Left = 0x0008,
        Down = 0x0010,
        Button1 = 0x0020,
        Button2 = 0x0040,
        Button3 = 0x0080,
        Button4 = 0x0100,
        Sound = 0x0200,  // volume down
        Select = 0x0400,
        PowerOff = 0x0800,
        Pause = 0x1000,  // labelled "Button 5" on the console
        Start = 0x2000,
    };

    explicit RZoneIO(Type type) : type_(type) {}

    // Which wiring a game uses, by its pack ref; returns false for non-R-Zone games.
    static bool type_for_ref(const std::string& ref, Type& out) {
        if (ref == "RZBATFOR") { out = Type::Type2; return true; }
        if (ref == "RZTOSHDEN" || ref == "RZINDY500") { out = Type::Type1; return true; }
        return false;
    }

    void set_buttons(uint16_t buttons) { buttons_ = buttons; }
    uint16_t buttons() const { return buttons_; }
    bool led() const { return led_; }

    void write_r(uint8_t r) override {
        r_ = r;
        if (type_ == Type::Type1) {
            sctrl_w((r >> 1) & 1);
        }
    }

    void write_s(uint8_t s) override {
        if (type_ == Type::Type1) {
            led_ = s & 1;
            sclock_w((s >> 1) & 1);
        } else {
            sctrl_w(s & 1);
            sclock_w((s >> 1) & 1);
            s3_ = (s >> 2) & 1;
            led_ = (s >> 3) & 1;
        }
    }

    // K1 = the 74165's serial output; K2-K4 unconnected.
    uint8_t read_k() override {
        sctrl_w(sctrl_);  // reload the parallel inputs if SH/LD is still low
        return mux_ & 1;
    }

    // Type1 drives the speaker straight from R1 (MAME's RMASK_DIRECT - no
    // buzzer clock modulation); Type2 mixes S3 with the SM512's melody output.
    bool sound(bool cpu_sound) override {
        return (type_ == Type::Type1) ? (r_ & 1) != 0 : (cpu_sound || s3_);
    }

private:
    // 74165 SH/LD: while low, the parallel inputs are (continuously) loaded.
    void sctrl_w(int state) {
        if (!state || !sctrl_) mux_ = buttons_;
        sctrl_ = state;
    }

    // 74165 CLK: shift on the rising edge while SH/LD is high.
    void sclock_w(int state) {
        if (sctrl_ && !sclock_ && state) mux_ >>= 1;
        sclock_ = state;
    }

    Type type_;
    uint16_t buttons_ = 0;
    uint16_t mux_ = 0;
    int sctrl_ = 0;
    int sclock_ = 0;
    uint8_t r_ = 0;
    int s3_ = 0;
    bool led_ = false;
};
