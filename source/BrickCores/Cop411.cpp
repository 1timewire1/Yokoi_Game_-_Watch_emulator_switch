#include "Cop411.h"

#include <cstring>

// MAME src/devices/cpu/cop400/cop400.cpp + cop400op.hxx (Curt Coder,
// BSD-3-Clause): COP410_OPCODE_MAP / _23_MAP / _33_MAP and execute_run().

namespace {
inline bool instlen2(uint8_t op) { return op == 0x23 || op == 0x33 || op == 0x60 || op == 0x61 || op == 0x68 || op == 0x69; }
}

void Cop411::load(const std::vector<uint8_t>& rom) {
    rom_.assign(0x200, 0);
    for (size_t i = 0; i < rom.size() && i < 0x200; i++) rom_[i] = rom[i];
}

void Cop411::reset() {
    // device_start
    memset(ram_, 0, sizeof(ram_));
    pc_ = sa_ = sb_ = 0;
    a_ = b_ = c_ = g_ = q_ = en_ = sio_ = si_ = skl_ = sk_ = 0;
    skip_ = skip_lbi_ = halt_ = second_byte_ = 0;
    opcode_ = 0x44;
    // device_reset
    out_d(0);
    write_g_(0);
    skl_ = 1;
    out_sk(0);
}

void Cop411::write_en(uint8_t v) {
    if (en_ != v) {
        en_ = v;
        sk_update();
    }
}

void Cop411::serial_tick() {
    if (en_ & 1) {
        si_ = (uint8_t)(((si_ << 1) & 0x0e) | 0);  // SI not connected
        if ((si_ & 0x0f) == 0x0c) sio_ = (sio_ - 1) & 0x0f;
    } else {
        sio_ = (uint8_t)((sio_ << 1) & 0x0f);
    }
}

void Cop411::lbi(uint8_t operand) {
    skip_lbi_++;
    if (skip_lbi_ > 1) return;
    skip_lbi_++;
    if (operand & 0x80) b_ = operand & 0x7f;
    else b_ = (uint8_t)((operand & 0x30) | (((operand & 0x0f) + 1) & 0x0f));
}

void Cop411::step() {
    if (halt_) return;
    if (!(en_ & 1) && skl_) {
        out_sk(0);
        out_sk(1);
    }
    const uint8_t operand = rom(pc_++);
    if (!second_byte_) opcode_ = operand;
    if (!second_byte_ && instlen2(opcode_)) {
        second_byte_ = 1;
    } else if (skip_) {
        skip_ = 0;
        second_byte_ = 0;
    } else if (!second_byte_ && (opcode_ == 0xff || opcode_ == 0xbf)) {  // JID / LQID
        if (opcode_ == 0xbf) push(pc_);
        pc_ = (uint16_t)((pc_ & 0x700) | (a_ << 4) | ram_r(b_));
        second_byte_ = 1;
    } else {
        exec(operand);
        if (skip_lbi_ > 0) skip_lbi_--;
        second_byte_ = 0;
    }
    serial_tick();
}

void Cop411::exec33(uint8_t op) {
    switch (op) {
        case 0x01: if (!(in_g() & 1)) skip(); return;          // SKGBZ0
        case 0x11: if (!(in_g() & 2)) skip(); return;          // SKGBZ1
        case 0x03: if (!(in_g() & 4)) skip(); return;          // SKGBZ2
        case 0x13: if (!(in_g() & 8)) skip(); return;          // SKGBZ3
        case 0x21: if (in_g() == 0) skip(); return;            // SKGZ
        case 0x2a: a_ = in_g(); return;                        // ING
        case 0x2e: {                                           // INL
            const uint8_t l = read_l ? read_l() : 0;
            ram_w(b_, l >> 4);
            a_ = l & 0xf;
            return;
        }
        case 0x38: halt_ = 1; return;                          // HALT
        case 0x3a: write_g_(ram_r(b_)); return;                // OMG
        case 0x3c: q_ = (uint8_t)((a_ << 4) | ram_r(b_)); return;  // CAMQ
        case 0x3e: out_d(b_ & 0x0f); return;                   // OBD
        default:
            if ((op & 0xf0) == 0x60) write_en(op & 0x0f);      // LEI
            return;                                            // (else illegal)
    }
}

void Cop411::exec(uint8_t operand) {
    const uint8_t op = opcode_;
    if (op >= 0x80) {
        if (op == 0xbf) {                                      // LQID (2nd byte)
            q_ = operand;
            pop();
        } else if (op == 0xff) {                               // JID (2nd byte)
            pc_ = (uint16_t)((pc_ & 0x700) | operand);
        } else {                                               // JP / JSRP
            const uint8_t page = (uint8_t)(pc_ >> 6);
            if (page == 2 || page == 3) {
                pc_ = (uint16_t)((pc_ & 0x780) | (op & 0x7f));
            } else if ((op & 0xc0) == 0xc0) {
                pc_ = (uint16_t)((pc_ & 0x7c0) | (op & 0x3f));
            } else {
                push(pc_);
                pc_ = (uint16_t)(0x80 | (op & 0x3f));
            }
        }
        return;
    }
    if ((op & 0xf0) == 0x70) {                                 // STII
        ram_w(b_, op & 0x0f);
        b_ = (uint8_t)((b_ & 0x70) + (((b_ & 0x0f) + 1) & 0x0f));
        return;
    }
    if ((op & 0xf0) == 0x50) {
        if (op == 0x50) { b_ = (uint8_t)((b_ & 0x70) | a_); return; }  // CAB
        a_ = (uint8_t)(a_ + (op & 0x0f));                       // AISC
        if (a_ > 0x0f) { skip(); a_ &= 0xf; }
        return;
    }
    if (op < 0x40 && (op & 0x08)) { lbi(operand); return; }   // LBI
    if (op < 0x40) {
        const uint8_t r = op & 0x30;
        switch (op & 0x07) {
            case 0x04: {                                       // XIS
                const uint8_t t = ram_r(b_);
                ram_w(b_, a_);
                a_ = t;
                const uint8_t bd = ((b_ & 0x0f) + 1) & 0x0f;
                b_ = (uint8_t)(((b_ & 0x70) | bd) ^ r);
                if (bd == 0) skip();
                return;
            }
            case 0x05: a_ = ram_r(b_); b_ ^= r; return;        // LD
            case 0x06: {                                       // X
                const uint8_t t = ram_r(b_);
                ram_w(b_, a_);
                a_ = t;
                b_ ^= r;
                return;
            }
            case 0x07: {                                       // XDS
                const uint8_t t = ram_r(b_);
                ram_w(b_, a_);
                a_ = t;
                const uint8_t bd = ((b_ & 0x0f) - 1) & 0x0f;
                b_ = (uint8_t)(((b_ & 0x70) | bd) ^ r);
                if (bd == 0x0f) skip();
                return;
            }
            default: break;
        }
        switch (op) {
            case 0x00: a_ = 0; return;                                         // CLRA
            case 0x01: if (!(ram_r(b_) & 1)) skip(); return;                   // SKMBZ0
            case 0x11: if (!(ram_r(b_) & 2)) skip(); return;                   // SKMBZ1
            case 0x03: if (!(ram_r(b_) & 4)) skip(); return;                   // SKMBZ2
            case 0x13: if (!(ram_r(b_) & 8)) skip(); return;                   // SKMBZ3
            case 0x02: a_ ^= ram_r(b_); return;                                // XOR
            case 0x20: if (c_) skip(); return;                                 // SKC
            case 0x21: if (a_ == ram_r(b_)) skip(); return;                    // SKE
            case 0x22: c_ = 1; return;                                         // SC
            case 0x23:                                                         // XAD 3,15
                if (operand == 0xbf) {
                    const uint8_t rd = operand & 0x7f;
                    const uint8_t t = a_;
                    a_ = ram_r(rd);
                    ram_w(rd, t);
                }
                return;
            case 0x30:                                                         // ASC
                a_ = (uint8_t)(a_ + c_ + ram_r(b_));
                if (a_ > 0xf) { c_ = 1; skip(); a_ &= 0xf; } else { c_ = 0; }
                return;
            case 0x31: a_ = (a_ + ram_r(b_)) & 0x0f; return;                   // ADD
            case 0x32: c_ = 0; return;                                         // RC
            case 0x33: exec33(operand); return;
            default: return;                                                   // illegal
        }
    }
    switch (op) {
        case 0x40: a_ ^= 0xf; return;                                          // COMP
        case 0x42: ram_w(b_, ram_r(b_) & 0xB); return;                         // RMB2
        case 0x43: ram_w(b_, ram_r(b_) & 0x7); return;                         // RMB3
        case 0x44: return;                                                     // NOP
        case 0x45: ram_w(b_, ram_r(b_) & 0xD); return;                         // RMB1
        case 0x46: ram_w(b_, ram_r(b_) | 0x4); return;                         // SMB2
        case 0x47: ram_w(b_, ram_r(b_) | 0x2); return;                         // SMB1
        case 0x48: pop(); return;                                              // RET
        case 0x49: pop(); skip(); return;                                      // RETSK
        case 0x4b: ram_w(b_, ram_r(b_) | 0x8); return;                         // SMB3
        case 0x4c: ram_w(b_, ram_r(b_) & 0xE); return;                         // RMB0
        case 0x4d: ram_w(b_, ram_r(b_) | 0x1); return;                         // SMB0
        case 0x4e: a_ = b_ & 0xf; return;                                      // CBA
        case 0x4f: {                                                           // XAS
            const uint8_t t = sio_;
            sio_ = a_;
            a_ = t;
            if (skl_ != c_) {
                skl_ = c_;
                sk_update();
            }
            return;
        }
        case 0x60: case 0x61: pc_ = (uint16_t)(((op & 0x07) << 8) | operand); return;               // JMP
        case 0x68: case 0x69: push(pc_); pc_ = (uint16_t)(((op & 0x07) << 8) | operand); return;    // JSR
        default: return;                                                       // illegal
    }
}

void Cop411::serialize(BrickStateIO& io) {
    io.bytes(ram_, sizeof(ram_));
    io.pod(pc_);
    io.pod(sa_);
    io.pod(sb_);
    io.pod(a_);
    io.pod(b_);
    io.pod(c_);
    io.pod(g_);
    io.pod(q_);
    io.pod(en_);
    io.pod(sio_);
    io.pod(si_);
    io.pod(skl_);
    io.pod(sk_);
    io.pod(skip_);
    io.pod(skip_lbi_);
    io.pod(halt_);
    io.pod(opcode_);
    io.pod(second_byte_);
}
