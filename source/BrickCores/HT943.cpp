#include "HT943.h"

#include <cstring>

namespace {

constexpr uint32_t kTimerIntLocation = 4;
constexpr uint32_t kExternalIntLocation = 8;

// Sound ROM: 12 channels of 32 notes, then 4 of 64 (HT4BITsound.py).
constexpr int kChannelSize = 32;
constexpr int kSingleChannels = 12;
constexpr int kSromSize = kChannelSize * 20;

// Note/speed codes are the chip's LFSR states; this maps them to dividers.
constexpr uint8_t kLfsr2Div[128] = {
    0, 2, 123, 3, 124, 75, 117, 4, 125, 101, 111, 76, 118, 42, 69, 5, 126, 66, 63, 102, 112, 86, 36, 77, 119,
    21, 95, 43, 70, 25, 105, 6, 127, 115, 99, 67, 64, 34, 19, 103, 113, 17, 15, 87, 37, 55, 89, 78, 120, 39,
    60, 22, 96, 52, 57, 44, 71, 91, 30, 26, 106, 47, 80, 7, 1, 122, 74, 116, 100, 110, 41, 68, 65, 62, 85, 35,
    20, 94, 24, 104, 114, 98, 33, 18, 16, 14, 54, 88, 38, 59, 51, 56, 90, 29, 46, 79, 121, 73, 109, 40, 61, 84,
    93, 23, 97, 32, 13, 53, 58, 50, 28, 45, 72, 108, 83, 92, 31, 12, 49, 27, 107, 82, 11, 48, 81, 10, 9, 8,
};

uint8_t lfsr2div(int code) { return (code >= 0 && code < 128) ? kLfsr2Div[code] : 0; }

}  // namespace

void HT943::load_options() {
    const BrickRom::Options& o = options();
    timer_div_ = (int)o.num("timer_clock_div", 16);
    if (timer_div_ < 1) timer_div_ = 1;
    static const char* kNames[3] = {"PP", "PM", "PS"};
    for (int i = 0; i < 3; i++) {
        pullup_[i] = (uint8_t)(o.num(std::string("port_pullup.") + kNames[i], 0xF) & 0xF);
        wakeup_[i] = (uint8_t)(o.num(std::string("port_wakeup.") + kNames[i], 0) & 0xF);
    }
    sound_freq_div_ = (int)o.num("sound_freq_div", 64);
    if (sound_freq_div_ < 1) sound_freq_div_ = 1;
    const std::vector<long> speed = o.list("sound_speed_div");
    const std::vector<long> effect = o.list("sound_effect");
    for (int i = 0; i < 16; i++) {
        sound_speed_div_[i] = i < (int)speed.size() ? (uint8_t)speed[i] : 0;
        sound_effect_[i] = i < (int)effect.size() ? (uint8_t)effect[i] : 0;
    }
    srom_.assign(kSromSize, 0);
    if (const std::vector<uint8_t>* s = rom_info().blob("sound_rom"))
        memcpy(srom_.data(), s->data(), s->size() < srom_.size() ? s->size() : srom_.size());
    options_loaded_ = true;
}

void HT943::reset() {
    if (!options_loaded_) load_options();
    acc_ = 0;
    memset(wr_, 0, sizeof(wr_));
    pc_ = 0;
    stack_ = 0;
    ei_ = cf_ = tf_ = ef_ = halt_ = 0;
    timer_on_ = 0;
    tc_ = 0;
    timer_counter_ = 0;
    // (The sound player's channel/note/counter survive a reset, as in
    // HT4BIT._reset - only "off" and one-shot mode are reset.)
    sound_off();
    snd_repeat_ = 0;
    ram_.assign(256, 0xF);
    pa_ = 0;
    for (int i = 0; i < 3; i++) pins_[i] = pullup_[i];
}

uint8_t HT943::display_byte(int index) const {
    if (halt_ || reset_held() || index < 0 || index >= (int)ram_.size()) return 0;
    return ram_[index];
}

int HT943::port_id(const std::string& name) const {
    if (name == "PP") return kPP;
    if (name == "PM") return kPM;
    if (name == "PS") return kPS;
    if (name == "PA") return kPA;
    return -1;
}

void HT943::port_input(int port, uint8_t pin, int level) {
    if (port < kPP || port > kPS || pin > 3) return;
    const uint8_t bit = (uint8_t)(1u << pin);
    if (level >= 0) {
        pins_[port] = (uint8_t)((pins_[port] & ~bit) | ((level ? 1 : 0) << pin));
        // A wake-up pin pulled low ends HALT through the external interrupt.
        if (halt_ && (wakeup_[port] & bit) && !level) {
            ef_ = 1;
            halt_ = 0;
        }
    } else {
        pins_[port] = (uint8_t)((pins_[port] & ~bit) | (pullup_[port] & bit));
    }
}

void HT943::interrupt(uint32_t location) {
    stack_ = ((uint32_t)cf_ << 12) | (pc_ & 0xFFF);
    pc_ = (pc_ & 0xF000) | location;
}

// --- sound -------------------------------------------------------------------

void HT943::sound_off() {
    snd_on_ = 0;
    set_tone(0);
}

void HT943::sound_channel(uint8_t channel) {
    snd_on_ = 1;
    snd_note_ = 0;
    snd_channel_ = channel & 0xF;
}

double HT943::sound_freq() const {
    // (HT4BITsound._get_freq's ">" here, not ">=", is equivalent: channel
    // 12 is the first double-length one, so its extra offset is 0.)
    int offset = snd_channel_ * kChannelSize;
    if (snd_channel_ > kSingleChannels) offset += (snd_channel_ - kSingleChannels) * kChannelSize;
    const int at = offset + snd_note_;
    const uint8_t note = at < (int)srom_.size() ? srom_[at] : 0;
    const uint8_t div = lfsr2div(note);
    if (note == 0 || div == 0) return 0;
    return (double)clock_hz() / sound_freq_div_ / div * 2;
}

void HT943::sound_clock(int cycles) {
    if (!snd_on_) return;
    snd_counter_ -= cycles;
    if (snd_counter_ > 0) return;
    const int size = kChannelSize * ((snd_channel_ >= kSingleChannels) + 1);
    if (snd_note_ >= size) {
        snd_note_ = 0;
        if (!snd_repeat_) sound_off();
    } else {
        snd_counter_ += lfsr2div(sound_speed_div_[snd_channel_]) * sound_freq_div_ * 16;
        set_tone(sound_freq(), sound_effect_[snd_channel_] & 1);
        snd_note_++;
    }
}

// --- CPU -----------------------------------------------------------------------

int HT943::execute() {
    if (halt_) return 8;

    if (ei_ && stack_ == 0) {
        if (ef_) {
            ef_ = 0;
            interrupt(kExternalIntLocation);
        } else if (tf_) {
            tf_ = 0;
            interrupt(kTimerIntLocation);
        }
    }

    const uint8_t op = rom_byte(pc_);
    const uint8_t imm = rom_byte(pc_ + 1);
    int cycles = 4;

    // Conditional jumps: 11-bit target within the current 2K page.
    auto branch = [&](bool taken) {
        pc_ += 2;
        if (taken) pc_ = (pc_ & 0xF800) | ((uint32_t)(op & 0x7) << 8) | imm;
        cycles = 8;
    };
    auto add = [&](int v) {
        int r = acc_ + v;
        cf_ = r > 15;
        acc_ = r & 0xF;
    };

    switch (op) {
        case 0x00: cf_ = acc_ & 1; acc_ = (uint8_t)((cf_ << 3) | (acc_ >> 1)); pc_++; break;          // RR A
        case 0x01: cf_ = acc_ >> 3; acc_ = (uint8_t)(cf_ | ((acc_ << 1) & 0xF)); pc_++; break;        // RL A
        case 0x02: { uint8_t c = acc_ & 1; acc_ = (uint8_t)((cf_ << 3) | (acc_ >> 1)); cf_ = c; pc_++; break; }   // RRC A
        case 0x03: { uint8_t c = acc_ >> 3; acc_ = (uint8_t)(cf_ | ((acc_ << 1) & 0xF)); cf_ = c; pc_++; break; } // RLC A
        case 0x04: acc_ = ram(0); pc_++; break;
        case 0x05: ram(0) = acc_; pc_++; break;
        case 0x06: acc_ = ram(2); pc_++; break;
        case 0x07: ram(2) = acc_; pc_++; break;
        case 0x08: add(ram(0) + cf_); pc_++; break;                 // ADC A,[R1R0]
        case 0x09: add(ram(0)); pc_++; break;                       // ADD A,[R1R0]
        case 0x0A: add((ram(0) ^ 0xF) + cf_); pc_++; break;         // SBC A,[R1R0]
        case 0x0B: add((ram(0) ^ 0xF) + 1); pc_++; break;           // SUB A,[R1R0]
        case 0x0C: ram(0) = (ram(0) + 1) & 0xF; pc_++; break;
        case 0x0D: ram(0) = (ram(0) - 1) & 0xF; pc_++; break;
        case 0x0E: ram(2) = (ram(2) + 1) & 0xF; pc_++; break;
        case 0x0F: ram(2) = (ram(2) - 1) & 0xF; pc_++; break;
        case 0x10: case 0x12: case 0x14: case 0x16: case 0x18:      // INC Rn
            wr_[(op >> 1) & 7] = (wr_[(op >> 1) & 7] + 1) & 0xF; pc_++; break;
        case 0x11: case 0x13: case 0x15: case 0x17: case 0x19:      // DEC Rn
            wr_[(op >> 1) & 7] = (wr_[(op >> 1) & 7] - 1) & 0xF; pc_++; break;
        case 0x1A: acc_ &= ram(0); pc_++; break;
        case 0x1B: acc_ ^= ram(0); pc_++; break;
        case 0x1C: acc_ |= ram(0); pc_++; break;
        case 0x1D: ram(0) &= acc_; pc_++; break;
        case 0x1E: ram(0) ^= acc_; pc_++; break;
        case 0x1F: ram(0) |= acc_; pc_++; break;
        case 0x20: case 0x22: case 0x24: case 0x26: case 0x28:      // MOV Rn,A
            wr_[(op >> 1) & 7] = acc_; pc_++; break;
        case 0x21: case 0x23: case 0x25: case 0x27: case 0x29:      // MOV A,Rn
            acc_ = wr_[(op >> 1) & 7]; pc_++; break;
        case 0x2A: cf_ = 0; pc_++; break;
        case 0x2B: cf_ = 1; pc_++; break;
        case 0x2C: ei_ = 1; pc_++; break;
        case 0x2D: ei_ = 0; pc_++; break;
        case 0x2E: pc_ = (pc_ & 0xF000) | (stack_ & 0xFFF); stack_ = 0; break;                      // RET
        case 0x2F: pc_ = (pc_ & 0xF000) | (stack_ & 0xFFF); cf_ = (stack_ >> 12) & 1; stack_ = 0; break; // RETI
        case 0x30: pa_ = acc_; port_output(kPA, pa_); pc_++; break;  // OUT PA,A (HT943)
        case 0x31: acc_ = (acc_ + 1) & 0xF; pc_++; break;
        case 0x32: acc_ = pins_[kPM]; pc_++; break;                 // IN A,PM (HT943)
        case 0x33: acc_ = pins_[kPS]; pc_++; break;                 // IN A,PS (HT943)
        case 0x34: acc_ = pins_[kPP]; pc_++; break;                 // IN A,PP (HT943)
        case 0x36:                                                  // DAA
            if (acc_ > 9 || cf_) {
                acc_ = (acc_ + 6) & 0xF;
                cf_ = 1;
            }
            pc_++;
            break;
        case 0x37:                                                  // HALT (2 bytes)
            pc_ += 2;
            halt_ = 1;
            ef_ = 0;
            sound_off();
            cycles = 8;  // (the timer still counts this one)
            break;
        case 0x38: timer_on_ = 1; pc_++; break;
        case 0x39: timer_on_ = 0; pc_++; break;
        case 0x3A: acc_ = tc_ & 0xF; pc_++; break;
        case 0x3B: acc_ = (tc_ >> 4) & 0xF; pc_++; break;
        case 0x3C: tc_ = (uint8_t)((tc_ & 0xF0) | acc_); pc_++; break;
        case 0x3D: tc_ = (uint8_t)((tc_ & 0x0F) | (acc_ << 4)); pc_++; break;
        case 0x3F: acc_ = (acc_ - 1) & 0xF; pc_++; break;
        case 0x40: add(imm & 0xF); pc_ += 2; cycles = 8; break;                 // ADD A,x
        case 0x41: add((imm ^ 0xF) + 1); pc_ += 2; cycles = 8; break;            // SUB A,x (unmasked, as HT4BIT.py)
        case 0x42: acc_ &= imm & 0xF; pc_ += 2; cycles = 8; break;
        case 0x43: acc_ ^= imm & 0xF; pc_ += 2; cycles = 8; break;
        case 0x44: acc_ |= imm & 0xF; pc_ += 2; cycles = 8; break;
        case 0x45: sound_channel(imm & 0xF); pc_ += 2; cycles = 8; break;       // SOUND n
        case 0x46: wr_[4] = imm & 0xF; pc_ += 2; cycles = 8; break;
        case 0x47: tc_ = imm; pc_ += 2; cycles = 8; break;                      // TIMER xx
        case 0x48: snd_repeat_ = 0; pc_++; break;                               // SOUND ONE
        case 0x49: snd_repeat_ = 1; pc_++; break;                               // SOUND LOOP
        case 0x4A: sound_off(); pc_++; break;
        case 0x4B: sound_channel(acc_); pc_++; break;                           // SOUND A
        case 0x4C: case 0x4D: {                                                 // READ(F) R4A
            pc_++;
            const uint32_t page = (op == 0x4C) ? (pc_ & 0xFF00) : ((pc_ & 0xF000) | 0xF00);
            const uint8_t b = rom_byte(page | ((uint32_t)acc_ << 4) | ram(0));
            acc_ = b & 0xF;
            wr_[4] = (b >> 4) & 0xF;
            cycles = 8;
            break;
        }
        case 0x4E: case 0x4F: {                                                 // READ(F) MR0A
            pc_++;
            const uint32_t page = (op == 0x4E) ? (pc_ & 0xFF00) : ((pc_ & 0xF000) | 0xF00);
            const uint8_t b = rom_byte(page | ((uint32_t)acc_ << 4) | wr_[4]);
            acc_ = b & 0xF;
            ram(0) = (b >> 4) & 0xF;
            cycles = 8;
            break;
        }
        default:
            if (op >= 0x50 && op <= 0x5F) {                 // MOV R1R0,xx
                wr_[0] = op & 0xF;
                wr_[1] = imm & 0xF;
                pc_ += 2;
                cycles = 8;
            } else if (op >= 0x60 && op <= 0x6F) {          // MOV R3R2,xx
                wr_[2] = op & 0xF;
                wr_[3] = imm & 0xF;
                pc_ += 2;
                cycles = 8;
            } else if (op >= 0x70 && op <= 0x7F) {          // MOV A,x
                acc_ = op & 0xF;
                pc_++;
            } else if (op <= 0x9F && op >= 0x80) {          // JAn address
                branch(acc_ & (1 << ((op >> 3) & 3)));
            } else if (op <= 0xA7 && op >= 0xA0) {
                branch(wr_[0] != 0);
            } else if (op <= 0xAF && op >= 0xA8) {
                branch(wr_[1] != 0);
            } else if (op <= 0xB7 && op >= 0xB0) {
                branch(acc_ == 0);
            } else if (op <= 0xBF && op >= 0xB8) {
                branch(acc_ != 0);
            } else if (op <= 0xC7 && op >= 0xC0) {
                branch(cf_ != 0);
            } else if (op <= 0xCF && op >= 0xC8) {
                branch(cf_ == 0);
            } else if (op <= 0xD7 && op >= 0xD0) {          // JTMR - clears TF when taken
                const bool taken = tf_ != 0;
                branch(taken);
                if (taken) tf_ = 0;
            } else if (op <= 0xDF && op >= 0xD8) {
                branch(wr_[4] != 0);
            } else if (op >= 0xE0 && op <= 0xEF) {          // JMP
                pc_ = (pc_ & 0xF000) | ((uint32_t)(op & 0xF) << 8) | imm;
                cycles = 8;
            } else if (op >= 0xF0) {                        // CALL
                stack_ = (pc_ + 2) & 0xFFF;
                pc_ = (pc_ & 0xF000) | ((uint32_t)(op & 0xF) << 8) | imm;
                cycles = 8;
            } else {
                pc_++;                                      // 0x35, 0x3E: NOP
            }
            break;
    }

    sound_clock(cycles);

    timer_counter_ -= cycles;
    while (timer_counter_ <= 0) {
        timer_counter_ += timer_div_;
        if (timer_on_) {
            tc_ = (uint8_t)(tc_ + 1);
            if (tc_ == 0) tf_ = 1;
        }
    }
    return cycles;
}

void HT943::serialize(BrickStateIO& io) {
    io.pod(acc_);
    io.bytes(wr_, sizeof(wr_));
    io.pod(pc_);
    io.pod(stack_);
    io.pod(ei_);
    io.pod(cf_);
    io.pod(tf_);
    io.pod(ef_);
    io.pod(halt_);
    io.pod(timer_on_);
    io.pod(tc_);
    io.pod(timer_counter_);
    io.vec(ram_);
    io.pod(pa_);
    io.bytes(pins_, sizeof(pins_));
    io.pod(snd_counter_);
    io.pod(snd_note_);
    io.pod(snd_channel_);
    io.pod(snd_repeat_);
    io.pod(snd_on_);
}
