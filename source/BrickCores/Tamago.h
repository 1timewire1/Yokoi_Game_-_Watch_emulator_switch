#pragma once

#include <vector>

#include "BrickCore.h"
#include "tamago_machine.h"

// Tamagotchi TamaTown Tama-Go (2010) and Tamagotchi Friends (2013): a 6502
// at 4 MHz (General Plus SoC), a 48x31 LCD in four shades plus ten status
// icons, a 4 KB serial EEPROM (the pet) and, on the Tama-Go, the Gotchi
// figures (an SPI flash in the figure's base, read when one is inserted).
// The machine itself is the user's Switch port of nullableVoidPtr/tamago
// (tamago_machine.c / tamago_cpu6502.c, C); this class wraps it for Yokoi.
//
// Container (".brk", CONVERT_ROM/build_tamago_pack.py): core "TAMAGO",
// clock 4000000. Blobs: "rom" the console ROM (640 KB Tama-Go, 768 KB
// Friends), "fig01".."fig12" the figures (Tama-Go only). Input: port
// "KEYS", mask 1 = A (left), 2 = B (middle), 4 = C (right), level 1 =
// pressed. Display: ink(), kPicW x kPicH, 0 = the bare LCD, 255 = the
// darkest shade - the icon rows and the dot matrix laid out as on the
// device, for switch_screen.cpp to colour.
class Tamago : public BrickCore {
public:
    static constexpr int kDot = 6;                          // one LCD dot, in picture pixels
    static constexpr int kLcdW = 48, kLcdH = 31;
    static constexpr int kIcon = 36, kIconRow = 44;
    static constexpr int kPicW = kLcdW * kDot;              // 288
    static constexpr int kPicH = kIconRow * 2 + kLcdH * kDot;  // 274

    Tamago();
    ~Tamago();

    bool get_segments_state(uint8_t, uint8_t, uint8_t) override { return false; }
    bool has_analog_sound() override { return true; }
    // Whichever console Yokoi plays (the linked pair: the one being played,
    // core/link_session.h's sound player) becomes the machine's primary,
    // the only one whose sound tamago_machine.c renders.
    float get_sound_level() override { heard_ = true; return sound_level_; }
    const uint8_t* ink();                                   // rebuilt when the LCD changed

    // The Gotchi figure on the Tama-Go (Settings > Tama-Go Figure): 0 =
    // none, 1..12 = the container's "figNN". Ignored on the Friends.
    int figure_count() const { return figure_count_; }
    // contacts: bit 0 = CD1, bit 1 = CD2 touching (the figure's base pins).
    void set_figure(int n, uint8_t contacts = 3);
    int figure() const { return figure_; }
    bool is_friends() const { return friends_; }

    // Pet Time catches the pet up in compressed half seconds, as the port's
    // "Avanzar tiempo" (tamago_machine_tick): a frame interrupt and the 2 Hz
    // time base, then only kTickCycles of CPU, instead of 2,000,000.
    void fast_forward(uint64_t ticks) override;
    // The calendar: month and day are plain bytes in work RAM.
    void set_date(int month, int day) override;

    // Infrared link with a second console of the same kind on this Switch
    // (core/link_session.cpp): the port's tamago_link_connect(). The two
    // pulses are 27 cycles apart, far shorter than Yokoi's 1/32768 s tick,
    // so this console then runs both in lock step, instruction by
    // instruction, and the other one's own execute() only marks time.
    bool link_pair(BrickCore* other) override;
    void link_unpair() override;

protected:
    void reset() override;
    int execute() override;
    uint8_t display_byte(int) const override { return 0; }
    int port_id(const std::string& name) const override { return name == "KEYS" ? 0 : -1; }
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t addr, uint8_t v) override;
    int peek_ram(uint32_t addr) const override;

private:
    void end_frame();
    int step_one();                        // one instruction, its frame and sound bookkeeping

    TamagoMachine m_ = {};
    std::vector<uint8_t> bios_;
    std::vector<const std::vector<uint8_t>*> figs_;
    int figure_count_ = 0, figure_ = 0;
    uint8_t contacts_ = 3;
    bool friends_ = false;
    uint8_t keys_ = 0x0F;                  // active low, bits 0-2
    uint32_t frame_cycle_ = 0;             // CPU cycles into this 1/60 s
    static constexpr uint32_t kFrame = TAMAGO_CLOCK_HZ / 60u;
    static constexpr int kSamples = 800;   // one frame at 48 kHz
    int16_t pcm_[kSamples * 2] = {};       // last frame's sound, played back during this one
    float sound_level_ = 0.0f;
    uint8_t ink_[kPicH][kPicW] = {};
    uint8_t ink_dram_[0x200] = {};         // the DRAM ink_ was drawn from
    bool ink_valid_ = false;
    static Tamago* s_heard_;               // the console whose sound is rendered
    bool heard_ = false;                   // Yokoi sampled this one's sound this frame
    Tamago* peer_ = nullptr;               // linked console (the one that runs both)
    bool follower_ = false;                // run by peer_, not by its own execute()
    uint64_t clk_ = 0;                     // CPU cycles run, the lock step's time base
    int ask_frames_ = 0;                   // frames the game has been asking for the time
    uint64_t ff_ticks_ = 0;                // Pet Time ticks owed, below a half second
    static constexpr uint32_t kTickCycles = 20000;   // the port's ffcycles default
};
