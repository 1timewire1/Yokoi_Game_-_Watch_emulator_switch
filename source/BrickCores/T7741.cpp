#include "T7741.h"

#include <cmath>
#include <cstring>

namespace {

constexpr int kPortInp = 0, kPortOutp = 1, kPortIop = 2;
constexpr double kPrescalerSize = 32768;
constexpr double kSubClock6770 = 32768;
constexpr double kDiv1 = 16, kDiv2 = 20, kDiv3 = 24, kDiv4 = 32;

}  // namespace

int T7741::port_id(const std::string& name) const {
    if (name == "INP") return kPortInp;
    if (name == "OUTP") return kPortOutp;
    if (name == "IOP") return kPortIop;
    return -1;
}

void T7741::port_input(int port, uint8_t mask, int level) {
    int& reg = port == kPortInp ? inp_ : iop_;
    if (port != kPortInp && port != kPortIop) return;
    reg &= ~mask;
    if (level > 0) reg |= mask;
}

void T7741::set_out(int port, int value) {
    (port == kPortOutp ? outp_ : iop_) = value;
    port_output(port, (uint8_t)value);
}

void T7741::buzzer(int level) {
    bz_ = level;
    set_speaker(bz_ != sound_gnd_);
    nsf_ = 0;
}

void T7741::load_options() {
    options_loaded_ = true;
    const double clock = clock_hz();
    sound_gnd_ = (int)options().num("sound_gnd", 0);
    if (t6770s_) {
        sub_clock_div_ = kSubClock6770 / clock;
        com_div_ = 768;
    } else {
        sub_clock_div_ = (double)options().num("sub_clock", 32768) / clock;
        com_div_ = (double)options().num("com_div", 768);
        const std::vector<long> p = options().list("prescaler_div");
        if (p.size() == 3) {
            px_div_ = kPrescalerSize / p[0];
            py_div_ = kPrescalerSize / p[1];
            pz_div_ = kPrescalerSize / p[2];
        }
    }
    frame_div_ = com_div_ * kComCount;
}

void T7741::reset() {
    if (!options_loaded_) load_options();
    pc_ = 0xF00;
    a_ = b_ = h_ = l_ = 0;
    cf_ = nsf_ = 0;
    pzf_ = pyf_ = pxf_ = 0;
    inp_ = 0;
    bz_ = 0;
    if (t6770s_) {
        outp_ = iop_ = 0;
    } else {
        set_out(kPortOutp, outp_);
        set_out(kPortIop, iop_);
    }
    gram_offset_ = 0;
    scan_ = 1;
    pchtmp_ = -1;
    std::memset(ram_, 0, sizeof(ram_));
    std::memset(gram_, 0, sizeof(gram_));
    clc_src_ = 0;
}

uint32_t T7741::word_at(uint32_t pc) const {
    const std::vector<uint8_t>& r = rom();
    const size_t n = r.size();
    const size_t a = (size_t)pc << 1;
    return ((uint32_t)r[a % n] << 8) | r[(a + 1) % n];
}

int64_t T7741::execute_scaled() {
    const uint32_t opcode = word_at(pc_);
    pc_ = (pc_ & 0xF00) | ((pc_ + 1) & 0xFF);
    double exec = op(opcode & 0x3FF);

    if (t6770s_) {
        if (clc_src_) {
            counter_ += exec;
            exec /= sub_clock_div_;
        } else {
            counter_ += exec * sub_clock_div_;
        }
        const double counts = exec * sub_clock_div_;
        if (std::fmod(counter_, 512) < counts) {
            pzf_ = 1;
            if (std::fmod(counter_, 4096) < counts) {
                pyf_ = 1;
                if (std::fmod(counter_, 32768) < counts) pxf_ = 1;
            }
        }
        cycle_counter_ += exec;
    } else {
        if (clc_src_) exec /= sub_clock_div_;
        const double counts = exec * sub_clock_div_;
        counter_ += counts;
        cycle_counter_ += counts;
        if (std::fmod(counter_, pz_div_) < counts) {
            pzf_ = 1;
            if (std::fmod(counter_, py_div_) < counts) {
                pyf_ = 1;
                if (std::fmod(counter_, px_div_) < counts) pxf_ = 1;
            }
        }
    }
    return std::llround(exec * kTickHz);
}

double T7741::op(uint32_t opcode) {
    return t6770s_ ? op_t6770s(opcode) : op_t7741(opcode);
}

// Opcodes that mean different things on the two chips.
double T7741::op_t7741(uint32_t o) {
    switch (o) {
        case 0x001: l_ = b_; nsf_ = 0; return kDiv1;
        case 0x005: nsf_ = 0; return kDiv1;
        case 0x006:
            pc_ = ((uint32_t)ram_[0x70] << 8) | (ram_[0x71] << 4) | ram_[0x72];
            l_ = 0; nsf_ = 0;
            return kDiv4;
        case 0x00B: a_ = ram_[0x10 | l_]; nsf_ = 0; return kDiv1;
        case 0x00F: a_ = mhl(); nsf_ = 0; return kDiv1;
        case 0x016: return nop2();
        case 0x017: nsf_ = 0; return kDiv1;
        case 0x018:
            if (scan_ == 0) {
                for (int& g : gram_) g = 0xF;
            }
            gram_offset_ = 0;
            nsf_ = 0;
            return frame_div_ - std::fmod(cycle_counter_, frame_div_);
        case 0x01B: buzzer(1); return kDiv1;
        case 0x020: h_ = a_ & 7; nsf_ = 0; return kDiv1;
        case 0x021: l_ = a_; nsf_ = 0; return kDiv1;
        case 0x025: nsf_ = 0; return kDiv1;
        case 0x026:
            pc_ = ((uint32_t)ram_[0x73] << 8) | (ram_[0x74] << 4) | ram_[0x75];
            l_ = 0; nsf_ = 0;
            return kDiv4;
        case 0x034: nsf_ = 0; return kDiv1;
        case 0x036: return nop2();
        case 0x03B: buzzer(0); return kDiv1;
        default: break;
    }
    if (o >= 0x040 && o < 0x050) {
        const int v = a_ + (int)(o & 0xF) + cf_;
        a_ = v & 0xF;
        cf_ = nsf_ = v > 15;
        return kDiv1;
    }
    if (o >= 0x050 && o < 0x060) {
        const int v = (int)(o & 0xF) - a_ - cf_;
        a_ = v & 0xF;
        cf_ = nsf_ = v < 0;
        return kDiv1;
    }
    if (o >= 0x070 && o < 0x080) {
        mhl() = o & 0xF;
        const int nb = b_ + 1;
        b_ = nb & 0xF;
        h_ = nb & 7;
        nsf_ = nb > 15;
        return kDiv1;
    }
    if (o >= 0x0E0 && o < 0x0F0) {
        a_ = o & 0xF;
        set_out(kPortOutp, a_);
        cf_ = nsf_ = 0;
        return kDiv1;
    }
    if (o >= 0x0F0 && o < 0x100) {
        a_ = o & 0xF;
        set_out(kPortIop, a_);
        cf_ = nsf_ = 0;
        return kDiv1;
    }
    if (o >= 0x100 && o < 0x180) {
        const int bb = (o >> 4) & 3;
        const bool set = o < 0x140;
        if (o & 8) {
            int& m = mhl();
            if (bb == 1) m = set ? (m | b_) : (m & ~b_);
            a_ = m;
        } else {
            int& m = ram_[o & 7];
            m = set ? (m | (1 << bb)) : (m & ~(1 << bb));
            a_ = m;
        }
        cf_ = nsf_ = 0;
        return kDiv1;
    }
    if (o >= 0x180 && o < 0x1C0) {
        const int bb = (o >> 4) & 3;
        if (o & 8) {
            nsf_ = bb == 1 ? (mhl() >= (mhl() | b_)) : 1;
        } else {
            nsf_ = (ram_[o & 7] >> bb) & 1;
        }
        return kDiv1;
    }
    if (o >= 0x1C0 && o < 0x1D0) {
        int count = (l_ - (int)(o & 0xF)) + 2;
        if (count <= 0) count = 1;
        for (int i = 0; i < count; i++) {
            gram_[gram_offset_] = mhl();
            gram_offset_ = (gram_offset_ + 1) % kGramSize;
            l_ = (l_ - 1) & 0xF;
        }
        a_ = (a_ + 1) & 0xF;
        h_ = a_ & 7;
        l_ = 0xF;
        cf_ = nsf_ = a_ == 0;
        return kDiv4 + kDiv1 * (count - 1);
    }
    if (o >= 0x1F0 && o < 0x200) {
        const double delay = kDiv1 + 4.0 * (15 - l_);
        l_ = o & 0xF;
        nsf_ = 0;
        return delay;
    }
    if (o >= 0x200 && o < 0x300 && (o & 0x10)) {
        const bool second = o & 0x20;
        const int base = second ? 0x73 : 0x70;
        ram_[base] = (pc_ >> 8) & 0xF;
        ram_[base + 1] = (pc_ >> 4) & 0xF;
        ram_[base + 2] = pc_ & 0xF;
        pc_ = ((o & 0xC0) << 4) | (second ? 0x3F0 : 0x1F0) | (o & 0xF);
        l_ = 0;
        nsf_ = 0;
        return kDiv4;
    }
    return op_shared(o);
}

double T7741::op_t6770s(uint32_t o) {
    switch (o) {
        case 0x001: a_ = ram_[0x10 | l_]; nsf_ = cf_ = 0; return kDiv1;
        case 0x005: l_ = b_; nsf_ = 0; return kDiv1;
        case 0x006: pc_ = pca_; l_ = 0; nsf_ = 0; return kDiv4;
        case 0x00B: nsf_ = 0; return kDiv1;
        case 0x00F: nsf_ = 0; return kDiv1;
        case 0x016: buzzer(1); return kDiv4;
        case 0x017: {
            const double delay = kDiv1 + 8.0 * b_;
            b_ = 0xF;
            nsf_ = 0;
            return delay;
        }
        case 0x018:
            gram_offset_ = 0;
            nsf_ = 0;
            return frame_div_ - std::fmod(cycle_counter_, frame_div_);
        case 0x01B: nsf_ = 0; return kDiv1;
        case 0x020: b_ = a_; h_ = a_ & 7; nsf_ = 0; return kDiv1;
        case 0x021: a_ = mhl(); cf_ = nsf_ = 0; return kDiv1;
        case 0x025: l_ = a_; nsf_ = 0; return kDiv1;
        case 0x026: pc_ = pcd_; l_ = 0; nsf_ = 0; return kDiv4;
        case 0x034: {
            mhl() = a_;
            ram_[(h_ << 4) | ((l_ + 1) & 0xF)] = a_;
            l_ = (l_ + 2) & 0xF;
            nsf_ = 0;
            return kDiv1;
        }
        case 0x036: buzzer(0); return kDiv4;
        case 0x03B: nsf_ = 0; return kDiv1;
        default: break;
    }
    if (o >= 0x040 && o < 0x060) {
        mhl() = o & 0xF;
        h_ = b_ = 4;
        const int nl = l_ + (o < 0x050 ? 1 : -1);
        l_ = nl & 0xF;
        nsf_ = o < 0x050 ? nl > 15 : nl < 0;
        return kDiv1;
    }
    if (o >= 0x070 && o < 0x080) {
        mhl() = o & 0xF;
        const int nb = b_ + 1;
        b_ = nb & 0xF;
        h_ = nb & 7;
        nsf_ = nb > 15;
        a_ = cf_ = 0;
        return kDiv1;
    }
    if (o >= 0x0E0 && o < 0x100) {
        a_ = o & 0xF;
        (o < 0x0F0 ? outp_ : iop_) = a_;
        cf_ = nsf_ = 0;
        return kDiv1;
    }
    if (o >= 0x100 && o < 0x180) {
        const int bit = 1 << ((o >> 4) & 3);
        int& m = (o & 8) ? mhl() : ram_[o & 7];
        m = o < 0x140 ? (m | bit) : (m & ~bit);
        a_ = m;
        cf_ = nsf_ = 0;
        return kDiv1;
    }
    if (o >= 0x180 && o < 0x1C0) {
        const int bb = (o >> 4) & 3;
        nsf_ = (((o & 8) ? mhl() : ram_[o & 7]) >> bb) & 1;
        return kDiv1;
    }
    if (o >= 0x1C0 && o < 0x1D0) {
        int count = (l_ - (int)(o & 0xF)) + 2;
        if (count <= 0) count = 1;
        for (int i = 0; i < count; i++) {
            gram_[gram_offset_] = mhl();
            gram_offset_ = (gram_offset_ + 1) % kGramSize;
            l_ = (l_ - 1) & 0xF;
        }
        nsf_ = 0;
        return kDiv4 + kDiv1 * (count - 1);
    }
    if (o >= 0x1F0 && o < 0x200) {
        const int v = a_ + (int)(o & 0xF) + cf_;
        a_ = v & 0xF;
        cf_ = nsf_ = v > 15;
        return kDiv1;
    }
    if (o >= 0x200 && o < 0x300 && (o & 0x10)) {
        const bool second = o & 0x20;
        (second ? pcd_ : pca_) = pc_;
        pc_ = ((o & 0xC0) << 4) | (second ? 0x3F0 : 0x1F0) | (o & 0xF);
        l_ = 0;
        nsf_ = 0;
        return kDiv4;
    }
    return op_shared(o);
}

// Everything both chips agree on.
double T7741::op_shared(uint32_t o) {
    if (o >= 0x300) {  // bs imm
        if (!nsf_) {
            pc_ = pchtmp_ >= 0 ? (((uint32_t)pchtmp_ << 8) | (o & 0xFF)) : ((pc_ & 0xF00) | (o & 0xFF));
        }
        pchtmp_ = -1;
        nsf_ = 0;
        return kDiv1;
    }
    const int imm = o & 0xF;
    switch (o & 0x3F0) {
        case 0x060: {
            mhl() = imm;
            const int nl = l_ + 1;
            l_ = nl & 0xF;
            nsf_ = nl > 15;
            return kDiv1;
        }
        case 0x080: case 0x090: {
            const int count = 16 - b_;
            const bool up = (o & 0x3F0) == 0x080;
            for (int n = 0; n < count; n++) {
                ram_[(h_ << 4) | ((up ? l_ - imm : l_ + imm) & 0xF)] = mhl();
                l_ = (l_ + (up ? 1 : -1)) & 0xF;
            }
            b_ = (imm - 1) & 0xF;
            cf_ = 0;
            nsf_ = imm == 0;
            return kDiv3 * (count - 1) + kDiv4;
        }
        case 0x0A0: {
            const int v = ram_[0x10 | imm] + 1;
            a_ = ram_[0x10 | imm] = v & 0xF;
            nsf_ = cf_ = v > 15;
            return kDiv1;
        }
        case 0x0B0: {
            const int v = ram_[0x10 | imm] - 1;
            a_ = ram_[0x10 | imm] = v & 0xF;
            nsf_ = cf_ = v < 0;
            return kDiv1;
        }
        case 0x0C0: case 0x0D0: {
            const bool add = (o & 0x3F0) == 0x0C0;
            int count = 16 - imm - l_ + 1;
            if (count <= 0) count = 1;
            for (int n = 0; n < count; n++) {
                int& m = mhl();
                const int other = ram_[((b_ & 7) << 4) | l_];
                int res;
                if (add) {
                    res = m + other + cf_;
                    if (res > 9) { res += 6; cf_ = 1; } else { cf_ = 0; }
                } else {
                    res = m - other - cf_;
                    if (res < 0 || res > 9) { res += 10; cf_ = 1; } else { cf_ = 0; }
                }
                a_ = m = res & 0xF;
                l_ = (l_ + 1) & 0xF;
            }
            nsf_ = 0;
            return kDiv4 + kDiv2 * (count - 1);
        }
        case 0x1D0:
            if (!nsf_) pchtmp_ = imm;
            return kDiv1;
        case 0x1E0: {
            const int v = mhl() - imm;
            a_ = v & 0xF;
            cf_ = v < 0;
            nsf_ = v != 0;
            return kDiv4;
        }
        case 0x200: l_ = imm; nsf_ = 0; return kDiv1;
        case 0x220: a_ = ram_[imm]; cf_ = nsf_ = 0; return kDiv1;
        case 0x240: b_ = imm; nsf_ = 0; return kDiv1;
        case 0x260: ram_[imm] = a_; nsf_ = 0; return kDiv1;
        case 0x280: h_ = imm & 7; nsf_ = 0; return kDiv1;
        case 0x2A0: a_ = ram_[0x10 | imm]; cf_ = nsf_ = 0; return kDiv1;
        case 0x2C0: a_ = imm; cf_ = nsf_ = 0; return kDiv1;
        case 0x2E0: ram_[0x10 | imm] = a_; nsf_ = 0; return kDiv1;
        default: break;
    }
    switch (o) {
        case 0x000: case 0x014: case 0x01F: case 0x02B: case 0x02F: case 0x03F:
            nsf_ = 0;
            return kDiv1;
        case 0x002: {
            const int v = a_ + mhl();
            a_ = v & 0xF;
            nsf_ = cf_ = v > 15;
            return kDiv1;
        }
        case 0x003: {
            const int v = l_ + 1;
            l_ = v & 0xF;
            nsf_ = v > 15;
            return kDiv1;
        }
        case 0x004: scan_ = 0; nsf_ = 0; return kDiv1;
        case 0x007: nsf_ = pxf_; pxf_ = 0; return kDiv1;
        case 0x008: nsf_ = pyf_; pyf_ = 0; return kDiv1;
        case 0x009: ram_[0x10 | l_] = a_; nsf_ = 0; return kDiv1;
        case 0x00A: {
            int& m = mhl();
            const int cf = m & 1;
            a_ = m = (cf_ << 3) | (m >> 1);
            cf_ = cf;
            nsf_ = 0;
            return kDiv1;
        }
        case 0x00C: {
            int& lo = mhl();
            l_ = (l_ + 1) & 0xF;
            int& hi = mhl();
            int mh = hi;
            lo += 1;
            if (lo > 9) {
                lo = (lo + 6) & 0xF;
                mh += 1;
                hi = mh & 0xF;
            }
            a_ = hi;
            nsf_ = cf_ = mh > 15;
            return kDiv4;
        }
        case 0x00D: a_ = inp_; nsf_ = cf_ = 0; return kDiv1;
        case 0x00E: {
            int& m = mhl();
            const int cf = m >> 3;
            a_ = m = t6770s_ ? (cf_ | (m << 1)) : ((cf_ | (m << 1)) & 0xF);
            cf_ = cf;
            nsf_ = 0;
            return kDiv1;
        }
        case 0x010: case 0x030: {
            const int v = b_ + (o == 0x010 ? 1 : -1);
            b_ = v & 0xF;
            h_ = v & 7;
            nsf_ = o == 0x010 ? v > 15 : v < 0;
            return kDiv1;
        }
        case 0x011: a_ = b_; nsf_ = cf_ = 0; return kDiv1;
        case 0x012: {
            const int v = a_ + b_ + cf_;
            a_ = v & 0xF;
            nsf_ = cf_ = v > 15;
            return kDiv1;
        }
        case 0x013: {
            const int v = b_ + 1;
            b_ = v & 0xF;
            nsf_ = v > 15;
            return kDiv1;
        }
        case 0x015: clc_src_ = 1; nsf_ = 0; return kDiv1;
        case 0x019: b_ = a_; nsf_ = 0; return kDiv1;
        case 0x01A: case 0x03A: {
            const int count = b_ + 1;
            for (int n = 0; n < count; n++) {
                l_ = (l_ + (o == 0x01A ? -1 : 1)) & 0xF;
                mhl() = 0;
            }
            a_ = 0;
            b_ = 15;
            nsf_ = cf_ = 0;
            return kDiv1 + kDiv1 * count;
        }
        case 0x01C: case 0x01E: case 0x02E: case 0x03C: case 0x03E:
            return nop2();
        case 0x01D: a_ = iop_; cf_ = nsf_ = 0; return kDiv1;
        case 0x022: {
            const int v = mhl() - a_ - cf_;
            a_ = v & 0xF;
            cf_ = nsf_ = v < 0;
            return kDiv1;
        }
        case 0x023: {
            const int v = l_ - 1;
            l_ = v & 0xF;
            nsf_ = v < 0;
            return kDiv1;
        }
        case 0x024: scan_ = 1; nsf_ = 0; return kDiv1;
        case 0x027: case 0x037: nsf_ = 0; return kDiv1;
        case 0x028: nsf_ = pzf_; pzf_ = 0; return kDiv1;
        case 0x029: mhl() = a_; nsf_ = 0; return kDiv1;
        case 0x02A: {
            const uint32_t addr = (pc_ & 0xFE0) | ((cf_ << 4) | a_);
            double cycles = op(word_at(pc_) & 0x3FF);
            pc_ = (pc_ & 0xF00) | ((pc_ + 1) & 0xFF);
            cycles += op(word_at(addr) & 0x3FF);
            return cycles + kDiv4;
        }
        case 0x02C: {
            int& lo = mhl();
            l_ = (l_ + 1) & 0xF;
            int& hi = mhl();
            int mh = hi;
            lo = (lo - 1) & 0xF;
            if (lo > 9) {
                lo = (lo + 10) & 0xF;
                mh -= 1;
                hi = mh & 0xF;
            }
            a_ = hi;
            nsf_ = cf_ = mh < 0;
            return kDiv4;
        }
        case 0x02D: {
            int& m = mhl();
            const int v = m + 1;
            a_ = m = v & 0xF;
            nsf_ = cf_ = v > 15;
            return kDiv1;
        }
        case 0x031: a_ = l_; cf_ = nsf_ = 0; return kDiv1;
        case 0x032: {
            const int v = b_ - a_ - cf_;
            a_ = v & 0xF;
            cf_ = nsf_ = v < 0;
            return kDiv1;
        }
        case 0x033: {
            const int v = b_ - 1;
            b_ = v & 0xF;
            nsf_ = v < 0;
            return kDiv1;
        }
        case 0x035: clc_src_ = 0; nsf_ = 0; return kDiv1;
        case 0x038:
            gram_offset_ = (gram_offset_ + kSegCount / 4) % kGramSize;
            nsf_ = 0;
            return com_div_ - std::fmod(cycle_counter_, com_div_);
        case 0x039: b_ = l_; nsf_ = 0; return kDiv1;
        case 0x03D: {
            int& m = mhl();
            const int v = m - 1;
            a_ = m = v & 0xF;
            nsf_ = cf_ = v < 0;
            return kDiv1;
        }
        default: break;
    }
    nsf_ = 0;
    return kDiv1;
}

void T7741::serialize(BrickStateIO& io) {
    io.pod(pc_); io.pod(pca_); io.pod(pcd_);
    io.pod(a_); io.pod(b_); io.pod(h_); io.pod(l_); io.pod(cf_); io.pod(nsf_);
    io.pod(pzf_); io.pod(pyf_); io.pod(pxf_);
    io.pod(inp_); io.pod(outp_); io.pod(iop_); io.pod(bz_);
    io.pod(gram_offset_); io.pod(scan_); io.pod(pchtmp_); io.pod(clc_src_);
    io.bytes(ram_, sizeof(ram_));
    io.bytes(gram_, sizeof(gram_));
    io.pod(counter_); io.pod(cycle_counter_);
    if (!io.writing() && !t6770s_) {
        // The matrix follows the output ports, which a load doesn't "write".
        port_output(kPortOutp, (uint8_t)outp_);
        port_output(kPortIop, (uint8_t)iop_);
    }
}
