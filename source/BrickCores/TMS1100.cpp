#include "TMS1100.h"

#include <cstdlib>
#include <cstring>

// See TMS1100.h. Operand fields in TMS1000-family opcodes are stored with
// their bits reversed (the chip's PLA reads them LSB-first).

namespace {

inline uint8_t rev2(uint8_t v) { return (uint8_t)(((v & 1) << 1) | ((v >> 1) & 1)); }
inline uint8_t rev3(uint8_t v) { return (uint8_t)(((v & 1) << 2) | (v & 2) | ((v >> 2) & 1)); }
inline uint8_t rev4(uint8_t v) { return (uint8_t)((rev2(v & 3) << 2) | rev2((v >> 2) & 3)); }

}  // namespace

// --- options -------------------------------------------------------------

TMS1100::Pin TMS1100::parse_pin(const std::string& s) {
    Pin p;
    if (s.size() < 2) return p;
    const char kind = (char)(s[0] | 0x20);
    const int bit = atoi(s.c_str() + 1);
    if (kind == 'r' && bit >= 0 && bit <= 10) {
        p.port = 0;
        p.bit = (int8_t)bit;
    } else if (kind == 'o' && bit >= 0 && bit <= 7) {
        p.port = 1;
        p.bit = (int8_t)bit;
    }
    return p;
}

void TMS1100::load_options() {
    const std::vector<uint8_t>& r = rom();
    for (size_t i = 0; i < prg_.size(); i++) prg_[i] = i < r.size() ? r[i] : 0;
    memset(opla_, 0, sizeof(opla_));
    if (const std::vector<uint8_t>* pla = rom_info().blob("opla")) {
        for (size_t i = 0; i < 32 && i < pla->size(); i++) opla_[i] = (*pla)[i];
    } else {
        for (int i = 0; i < 32; i++) opla_[i] = (uint8_t)i;  // no PLA: A and SL straight through
    }
    const BrickRom::Options& o = options();
    auto pin = [&](const char* key) { return o.has(key) ? parse_pin(o.str(key)) : Pin(); };
    lcd_lp_ = pin("mv.lcd_lp");
    lcd_dc_ = pin("mv.lcd_dc");
    lcd_data_ = pin("mv.lcd_data");
    speaker_pin_ = pin("mv.speaker");
    for (int n = 0; n < kKeys; n++) {
        const std::string key = "mv.key." + std::to_string(n);
        keys_[n] = Key();
        if (!o.has(key)) continue;
        const std::string& v = o.str(key);
        const size_t comma = v.find(',');
        if (comma == std::string::npos) continue;
        keys_[n].strobe = parse_pin(v.substr(0, comma));
        keys_[n].k = (uint8_t)(strtol(v.c_str() + comma + 1, nullptr, 0) & 0xf);
    }
    paddle_ = Paddle();
    if (o.has("mv.paddle")) {
        const std::string& v = o.str("mv.paddle");
        const size_t comma = v.find(',');
        const std::vector<long> rest = comma == std::string::npos ? std::vector<long>() : o.list("mv.paddle");
        // list() parses every field as a number; the first one is the pin
        if (comma != std::string::npos && rest.size() == 4) {
            paddle_.strobe = parse_pin(v.substr(0, comma));
            paddle_.k = (uint8_t)(rest[1] & 0xf);
            const double per_us = clock_hz() / 1e6;
            paddle_.min_cycles = (uint32_t)(rest[2] * per_us);
            paddle_.max_cycles = (uint32_t)(rest[3] * per_us);
        }
    }
    butmask_ = (uint16_t)(o.num("mv.butmask", 0) & 0xfff);
    frame_cycles_len_ = clock_hz() / 60 ? clock_hz() / 60 : 1;
    options_loaded_ = true;
}

bool TMS1100::pin_level(const Pin& p) const {
    if (p.port == 0) return (r_ >> p.bit) & 1;
    if (p.port == 1) return (o_ >> p.bit) & 1;
    return false;
}

bool TMS1100::poke_ram(uint32_t addr, uint8_t value) {
    if (addr >= sizeof(ram_)) return false;
    ram_[addr] = value & 0xf;
    return true;
}

int TMS1100::peek_ram(uint32_t addr) const { return addr >= sizeof(ram_) ? -1 : ram_[addr]; }

// --- CPU -----------------------------------------------------------------

void TMS1100::reset() {
    if (!options_loaded_) load_options();
    memset(ram_, 0, sizeof(ram_));
    a_ = x_ = y_ = 0;
    status_ = 1;
    sl_ = 0;
    pc_ = sr_ = 0;
    pa_ = pb_ = 0xf;
    ca_ = cb_ = cs_ = cl_ = 0;
    now_ = 0;
    lcd_lp_level_ = lcd_dc_level_ = lcd_addr_ = 0;
    memset(lcd_latch_, 0, sizeof(lcd_latch_));
    lcd_row_ = lcd_col_ = 0;
    paddle_since_ = 0;
    paddle_strobe_ = 0;
    last_flush_ = 0;
    frame_cycles_ = 0;
    memset(acc_, 0, sizeof(acc_));
    memset(bri_, 0, sizeof(bri_));
    memset(shown_, 0, sizeof(shown_));
    r_ = 0;
    o_ = 0;
    write_r(0);
    write_o(opla_[0]);
}

// The program counter is a 6-bit shift register, not a binary counter: it
// shifts left and feeds back bit5 XNOR bit4, patched so that it runs
// through all 64 values (0x1f -> 0x3f -> 0x3e).
uint8_t TMS1100::next_pc(uint8_t pc) {
    uint8_t fb = (uint8_t)(((pc >> 5) ^ (pc >> 4) ^ 1) & 1);
    if (pc == 0x1f) fb = 1;
    else if (pc == 0x3f) fb = 0;
    return (uint8_t)(((pc << 1) | fb) & 0x3f);
}

uint8_t TMS1100::read_k() {
    uint8_t k = 0;
    for (int n = 0; n < kKeys; n++) {
        if (((buttons_ >> n) & 1) && keys_[n].strobe.port >= 0 && pin_level(keys_[n].strobe)) k |= keys_[n].k;
    }
    if (has_paddle() && paddle_strobe_) {
        // min > max is allowed: the knob turns the other way
        const double span = (double)paddle_.max_cycles - (double)paddle_.min_cycles;
        const uint64_t need = (uint64_t)(paddle_.min_cycles + span * paddle_pos_);
        if (now_ - paddle_since_ >= need) k |= paddle_.k;
    }
    return k & 0xf;
}

void TMS1100::write_r(uint16_t r) {
    r_ = r & 0x7ff;
    outputs_changed();
}

void TMS1100::write_o(uint8_t o) {
    o_ = o;
    outputs_changed();
}

int TMS1100::execute() {
    const uint8_t op = prg_[((ca_ & 1) << 10) | ((pa_ & 0xf) << 6) | (pc_ & 0x3f)];
    pc_ = next_pc(pc_);
    run_op(op);
    now_++;
    if (++frame_cycles_ >= frame_cycles_len_) end_frame();
    return 1;
}

void TMS1100::run_op(uint8_t op) {
    const uint8_t prev_status = status_;
    uint8_t st = 1;  // status after this instruction: 1 unless it tests something
    uint8_t& m = ram();

    if (op >= 0x80) {
        // BR / CALL - taken when the previous instruction left status 1
        if (prev_status) {
            if (op < 0xc0) {
                if (!cl_) pa_ = pb_;  // inside a subroutine a branch stays in its page
                ca_ = cb_;
            } else {
                const uint8_t old_pa = pa_;
                if (!cl_) {
                    sr_ = pc_;
                    cs_ = ca_;
                    cl_ = 1;
                }
                pa_ = pb_;
                pb_ = old_pa;
                ca_ = cb_;
            }
            pc_ = op & 0x3f;
        }
        status_ = 1;
        return;
    }

    switch (op >> 4) {
        case 0x1:  // LDP
            pb_ = rev4(op & 0xf);
            break;
        case 0x4:  // TCY
            y_ = rev4(op & 0xf);
            break;
        case 0x5:  // YNEC
            st = y_ != rev4(op & 0xf);
            break;
        case 0x6:  // TCMIY
            m = rev4(op & 0xf);
            y_ = (y_ + 1) & 0xf;
            break;
        case 0x7:
            if (op == 0x7f) {  // CLA
                a_ = 0;
            } else {  // AC1AAC: A += constant + 1
                const unsigned sum = a_ + rev4(op & 0xf) + 1u;
                st = sum > 15;
                a_ = sum & 0xf;
            }
            break;
        case 0x2:
            if (op >= 0x28) {  // LDX
                x_ = rev3(op & 7);
                break;
            }
            switch (op) {
                case 0x20: y_ = a_; break;                                       // TAY
                case 0x21: a_ = m; break;                                        // TMA
                case 0x22: y_ = m; break;                                        // TMY
                case 0x23: a_ = y_; break;                                       // TYA
                case 0x24: m = a_; st = y_ != 0; y_ = (y_ - 1) & 0xf; break;     // TAMDYN
                case 0x25: m = a_; st = y_ == 15; y_ = (y_ + 1) & 0xf; break;    // TAMIYC
                case 0x26: m = a_; a_ = 0; break;                                // TAMZA
                case 0x27: m = a_; break;                                        // TAM
            }
            break;
        case 0x3:
            if (op < 0x3c) {
                const uint8_t bit = (uint8_t)(1u << rev2(op & 3));
                if (op < 0x34) m |= bit;                // SBIT
                else if (op < 0x38) m &= (uint8_t)~bit; // RBIT
                else st = (m & bit) != 0;               // TBIT1
                break;
            }
            switch (op) {
                case 0x3c: st = m >= a_; a_ = (m - a_) & 0xf; break;             // SAMAN
                case 0x3d: st = a_ == 0; a_ = (uint8_t)(-a_) & 0xf; break;       // CPAIZ
                case 0x3e: st = m == 15; a_ = (m + 1) & 0xf; break;              // IMAC
                case 0x3f: st = m != 0; break;                                   // MNEZ
            }
            break;
        case 0x0:
            switch (op) {
                case 0x00: st = m != a_; break;                                  // MNEA
                case 0x01: st = a_ <= m; break;                                  // ALEM
                case 0x02: st = y_ != a_; sl_ = st; break;                       // YNEA
                case 0x03: { const uint8_t t = m; m = a_; a_ = t; } break;       // XMA
                case 0x04: st = y_ != 0; y_ = (y_ - 1) & 0xf; break;             // DYN
                case 0x05: st = y_ == 15; y_ = (y_ + 1) & 0xf; break;            // IYC
                case 0x06: { const unsigned s = a_ + m; st = s > 15; a_ = s & 0xf; } break;  // AMAAC
                case 0x07: st = m != 0; a_ = (m - 1) & 0xf; break;               // DMAN
                case 0x08: a_ = read_k(); break;                                 // TKA
                case 0x09: x_ ^= 4; break;                                       // COMX
                case 0x0a: write_o(opla_[((sl_ & 1) << 4) | a_]); break;         // TDO
                case 0x0b: cb_ ^= 1; break;                                      // COMC
                case 0x0c: if (y_ <= 10) write_r(r_ & ~(1u << y_)); break;       // RSTR
                case 0x0d: if (y_ <= 10) write_r(r_ | (1u << y_)); break;        // SETR
                case 0x0e: st = read_k() != 0; break;                            // KNEZ
                case 0x0f:                                                       // RETN
                    if (cl_) {
                        cl_ = 0;
                        pc_ = sr_;
                        ca_ = cs_;
                    }
                    pa_ = pb_;
                    break;
            }
            break;
    }
    m &= 0xf;
    status_ = st;
}

// --- board ---------------------------------------------------------------

void TMS1100::outputs_changed() {
    set_speaker(pin_level(speaker_pin_));
    if (has_paddle()) {
        const uint8_t s = pin_level(paddle_.strobe);
        if (s && !paddle_strobe_) paddle_since_ = now_;  // starts charging
        paddle_strobe_ = s;
    }
    lcd_update();
}

// HLCD 0488: latch pulse high resets its 3-bit nibble address; while the
// data clock is low the data lines are written to the addressed nibble,
// and its rising edge (latch pulse low) moves to the next one. Latch pulse
// and data clock both high copy the 8 nibbles out: the first 4 the 16 row
// lines, the last 4 the 16 column lines - a dot shows where both are on.
void TMS1100::lcd_update() {
    if (lcd_lp_.port < 0 || lcd_dc_.port < 0 || lcd_data_.port < 0) return;
    const uint8_t lp = pin_level(lcd_lp_), dc = pin_level(lcd_dc_);
    const uint8_t src = lcd_data_.port == 0 ? (uint8_t)(r_ >> lcd_data_.bit) : (uint8_t)(o_ >> lcd_data_.bit);
    const uint8_t data = src & 0xf;
    if (lp) lcd_addr_ = 0;
    if (!dc) lcd_latch_[lcd_addr_] = data;
    if (dc && !lcd_dc_level_ && !lp) lcd_addr_ = (lcd_addr_ + 1) & 7;
    if (lp && dc) {
        // Line 0 is the top row / leftmost column (Pinball: its paddle at the
        // bottom, the ball rising from it, the digits under BALLS / SCORE).
        // The type 1 output PLA reverses the data lines (prepare_microvision.py).
        const uint16_t row = (uint16_t)(lcd_latch_[0] | lcd_latch_[1] << 4 | lcd_latch_[2] << 8 | lcd_latch_[3] << 12);
        const uint16_t col = (uint16_t)(lcd_latch_[4] | lcd_latch_[5] << 4 | lcd_latch_[6] << 8 | lcd_latch_[7] << 12);
        if (row != lcd_row_ || col != lcd_col_) {
            flush_display();
            lcd_row_ = row;
            lcd_col_ = col;
        }
    }
    lcd_lp_level_ = lp;
    lcd_dc_level_ = dc;
}

void TMS1100::flush_display() {
    const uint32_t dt = (uint32_t)(now_ - last_flush_);
    last_flush_ = now_;
    if (!dt || !lcd_row_ || !lcd_col_) return;
    for (int r = 0; r < kDots; r++) {
        if (!((lcd_row_ >> r) & 1)) continue;
        for (int c = 0; c < kDots; c++) {
            if ((lcd_col_ >> c) & 1) acc_[r][c] += dt;
        }
    }
}

void TMS1100::end_frame() {
    flush_display();
    if (knob_ == 1) set_paddle(paddle_pos_ - 1.0f / kKnobFrames);
    else if (knob_ == 2) set_paddle(paddle_pos_ + 1.0f / kKnobFrames);
    const float frame = (float)(frame_cycles_ ? frame_cycles_ : 1);
    frame_cycles_ = 0;
    for (int r = 0; r < kDots; r++) {
        uint8_t* out = &shown_[r * 2];
        out[0] = out[1] = 0;
        for (int c = 0; c < kDots; c++) {
            // an LCD fades slowly: half of the last frame's level carries over
            float bri = bri_[r][c] * 0.5f + (acc_[r][c] / frame) * 0.5f;
            if (bri > 0.25f) bri = 0.25f;
            bri_[r][c] = bri;
            acc_[r][c] = 0;
            if (bri > 0.005f) out[c >> 3] |= (uint8_t)(1 << (c & 7));
        }
    }
}

uint8_t TMS1100::display_byte(int index) const {
    if (index < 0 || index >= kDots * 2) return 0;
    return shown_[index];
}

void TMS1100::port_input(int port, uint8_t mask, int level) {
    if (port == 1 && mask < 2) {
        if (level > 0) knob_ |= (uint8_t)(1u << mask);
        else knob_ &= (uint8_t)~(1u << mask);
        return;
    }
    if (port != 0 || mask >= kKeys) return;
    if (level > 0 && !((butmask_ >> mask) & 1)) buttons_ |= (uint16_t)(1u << mask);
    else buttons_ &= (uint16_t)~(1u << mask);
}

void TMS1100::serialize(BrickStateIO& io) {
    io.bytes(ram_, sizeof(ram_));
    io.pod(a_);
    io.pod(x_);
    io.pod(y_);
    io.pod(status_);
    io.pod(sl_);
    io.pod(pc_);
    io.pod(sr_);
    io.pod(pa_);
    io.pod(pb_);
    io.pod(ca_);
    io.pod(cb_);
    io.pod(cs_);
    io.pod(cl_);
    io.pod(r_);
    io.pod(o_);
    io.pod(now_);
    io.pod(lcd_lp_level_);
    io.pod(lcd_dc_level_);
    io.pod(lcd_addr_);
    io.bytes(lcd_latch_, sizeof(lcd_latch_));
    io.pod(lcd_row_);
    io.pod(lcd_col_);
    io.pod(paddle_since_);
    io.pod(paddle_strobe_);
    io.pod(paddle_pos_);
    io.pod(last_flush_);
    io.pod(frame_cycles_);
    io.bytes(acc_, sizeof(acc_));
    io.bytes(bri_, sizeof(bri_));
    io.bytes(shown_, sizeof(shown_));
    if (!io.writing()) set_speaker(pin_level(speaker_pin_));
}
