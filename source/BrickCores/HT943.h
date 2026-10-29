#pragma once

#include "BrickCore.h"

// Holtek HT943 (HT4BIT family, 4-bit, 8-bit opcodes) with its sound ROM
// player - a port of BrickEmuPy's cores/HT4BIT.py + HT943.py +
// HT4BITsound.py (github.com/azya52/BrickEmuPy, CC0). Brick games and
// keychains: Brick Game E-23 Plus Mark II / E-88 / GA-888, Keychain Pin
// Ball, Pikorin 55, Space Intruder, Bandai's Mame Game Galaxian and
// Tamagotchi, Compile's Puyolin.
//
// The display is the RAM itself: get_VRAM() is the 256 nibbles, blank while
// halted (powered off) or held in reset. Inputs are the PP/PM/PS pins
// ("mask" = pin number), each with a mask-option pull-up and halt wake-up.
class HT943 : public BrickCore {
public:
    HT943() : BrickCore("HT943") {}

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int index) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;

private:
    enum Port { kPP = 0, kPM = 1, kPS = 2, kPA = 3 };

    uint8_t rom_byte(uint32_t addr) const { return rom()[addr % rom().size()]; }
    uint8_t& ram(int rp) { return ram_[(wr_[rp + 1] << 4) | wr_[rp]]; }
    void interrupt(uint32_t location);
    void load_options();

    // sound (HT4BITsound.py)
    void sound_clock(int cycles);
    void sound_off();
    void sound_channel(uint8_t channel);
    double sound_freq() const;

    // mask options
    bool options_loaded_ = false;
    int timer_div_ = 16;
    uint8_t pullup_[3] = {0xF, 0xF, 0xF};
    uint8_t wakeup_[3] = {0, 0, 0};
    int sound_freq_div_ = 64;
    uint8_t sound_speed_div_[16] = {};
    uint8_t sound_effect_[16] = {};
    std::vector<uint8_t> srom_;

    // CPU
    uint8_t acc_ = 0;
    uint8_t wr_[5] = {};
    uint32_t pc_ = 0;
    uint32_t stack_ = 0;  // one level: CF << 12 | return address, 0 = empty
    uint8_t ei_ = 0, cf_ = 0, tf_ = 0, ef_ = 0, halt_ = 0;
    uint8_t timer_on_ = 0;
    uint8_t tc_ = 0;
    int32_t timer_counter_ = 0;
    std::vector<uint8_t> ram_ = std::vector<uint8_t>(256, 0xF);
    uint8_t pa_ = 0;
    uint8_t pins_[3] = {0xF, 0xF, 0xF};  // PP, PM, PS

    // sound player
    int32_t snd_counter_ = 0;
    int32_t snd_note_ = 0;
    uint8_t snd_channel_ = 0;
    uint8_t snd_repeat_ = 0;
    uint8_t snd_on_ = 0;
};
