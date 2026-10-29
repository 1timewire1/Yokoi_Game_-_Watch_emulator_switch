// E0C6200 CPU core - port of MAME's e0c6200.cpp / e0c6200op.cpp
// (license: BSD-3-Clause, copyright-holders: hap). See E0C6S46.h.
//
// Legend (from the original):
//   X/Y   = XH.XL / YH.YL 8-bit,  IX/IY = XP.XH.XL / YP.YH.YL 12-bit
//   MX/MY = data memory at IX/IY, Mn = data memory at 0-F (4-bit immediate)
//   r/q   = 2-bit param directing to A/B/MX/MY, i = 4-bit immediate
//   e     = 8-bit immediate, s = 8-bit immediate branch destination

#include "E0C6S46.h"

#include <cstring>

E0C6S46::E0C6S46(Model model)
    : SM5XX(model == Model::S48 ? "E0C6S48" : "E0C6S46"),
      model_(model),
      rom_words_(model == Model::S48 ? 0x2000 : 0x1800),
      ram_size_(model == Model::S48 ? 0x300 : 0x280),
      vram_size_(model == Model::S48 ? 0x66 : 0x50) {
    frequency = kOsc1;
    time_per_cycle_us = 1'000'000.0 / kOsc1;
}

void E0C6S46::init() {
    // SM5XX bookkeeping step() relies on (it never touches anything else).
    cycle_curr_opcode = 0;
    cpu_frequency_divider = 1;
    is_sleep = false;
    f_clock_divider = 0;
    gamma_flag_second = false;
    time_addresses = nullptr;
    sound_divide_frequency = 1;
    segments_state_are_update = true;

    power_on();
}

void E0C6S46::load_rom(const uint8_t* file_hex, size_t size_hex) {
    // 16-bit big-endian words, 12 bits used (MAME: ENDIANNESS_BIG, 16-bit bus).
    std::memset(rom_, 0, sizeof(rom_));
    const size_t words = size_hex / 2;
    for (size_t i = 0; i < words && i < (size_t)rom_words_; i++) {
        rom_[i] = (uint16_t)((file_hex[i * 2] << 8) | file_hex[i * 2 + 1]);
    }
}

void E0C6S46::configure_for_ref(const std::string& ref) {
    // OSC3 is an RC oscillator whose speed is set by a resistor on each
    // board. MAME's hh_e0c6200.cpp uses a 1 MHz placeholder (and no OSC3 at
    // all on tama/tamag2); these are BrickEmuPy's per-device values (its
    // .brick "clock"), by azya, who also contributed to the MAME driver.
    // Only Alien Fever runs on OSC3 all the time, so it's the one whose
    // speed visibly depends on this; the rest switch to it in short bursts.
    // Boards not listed have no OSC3 (== OSC1, switching to it does nothing).
    // (Pack refs are the MAME set names, uppercased by convert_switch.py.)
    struct Osc3 {
        const char* ref;
        uint32_t hz;
    };
    static const Osc3 kOsc3Boards[] = {
        {"TAMA", 1'600'000},      {"TAMAG2", 1'600'000},    {"TAMAANG", 1'060'000},
        {"TAMAMOT", 1'060'000},   {"DIGIMON", 1'060'000},   {"DIGIMONV2", 1'060'000},
        {"DIGIMONV3", 1'060'000}, {"ALIENFEV", 1'400'000},  {"STACKCH", 2'000'000},
        // BrickEmuPy-only sets (not in MAME)
        {"TAMAJ", 1'600'000},     {"TAMAUMI", 1'060'000},   {"TAMAMORI", 1'060'000},
        {"TAMAGENJIN", 1'060'000}, {"TAMAYASA", 1'060'000}, {"DIGIMONES", 1'060'000},
        {"DIGIMONV4", 1'060'000}, {"DIGIMONJA", 1'060'000},
        // Unidentified S48 Tamagotchi (BrickEmuPy maintainer's dump, no
        // .brick upstream) - assumed the same board as the other S48 ones.
        {"TAMAOTCH", 1'060'000},
    };
    s_.osc3 = kOsc1;
    for (const Osc3& b : kOsc3Boards) {
        if (ref == b.ref) s_.osc3 = b.hz;
    }
    // Digital Monster's P20 is the (unemulated) link port, read high when idle.
    std::memset(p_input_, 0, sizeof(p_input_));
    if (ref.rfind("DIGIMON", 0) == 0) p_input_[2] = 0xf;
}

bool E0C6S46::get_segments_state(uint8_t col, uint8_t line, uint8_t word) {
    if (col >= 16 || line >= 51 || word != 0) return false;
    return s_.lcd[col][line] != 0;
}

void E0C6S46::set_reset_line(bool asserted) {
    if (asserted && !s_.reset_held) device_reset();
    s_.reset_held = asserted ? 1 : 0;
}

//-------------------------------------------------
//  power on / reset
//-------------------------------------------------

void E0C6S46::power_on() {
    // device_start(): zerofill (RAM contents are don't-care on real hardware;
    // MAME leaves them zeroed too).
    const uint32_t osc3 = s_.osc3 ? s_.osc3 : kOsc1;
    std::memset(&s_, 0, sizeof(s_));
    s_.osc3 = osc3;
    s_.cpu_clock = kOsc1;
    std::memset(s_.port_k, 0xf, sizeof(s_.port_k));
    s_.dfk0 = 0xf;
    s_.ticks_to_256 = 64;   // core_256 timer: from_ticks(64, osc1)
    s_.ticks_to_lcd = 1024; // LCD driver: from_ticks(1024, osc1)

    device_reset();
}

void E0C6S46::device_reset() {
    // e0c6200_cpu_device::device_reset()
    s_.halt = s_.sleep = 0;
    s_.op = 0xfff;  // nop
    s_.pc = 0x100;
    s_.f &= 3;      // decimal flag is 0 on 6200A, undefined on 6200

    // e0c6s46_device::device_reset()
    std::memset(s_.irqflag, 0, sizeof(s_.irqflag));
    std::memset(s_.irqmask, 0, sizeof(s_.irqmask));

    io_w(0x41, 0xf);
    io_w(0x54, 0xf);
    io_w(0x70, 0x0);
    io_w(0x71, 0x8);
    io_w(0x73, s_.svd & 3);

    io_w(0x74, 0x0);
    io_w(0x75, 0x4);
    io_w(0x76, 0x3);
    io_w(0x77, 0x2);
    io_w(0x78, 0x2);
    io_w(0x79, 0x0);
    io_w(0x7b, 0x0);
    io_w(0x7d, 0x0);
    io_w(0x7e, 0x0);

    for (int i = 0; i < 5; i++) write_r(i, s_.port_r[i]);
    for (int i = 0; i < 4; i++) write_p(i, s_.port_p[i]);
}

//-------------------------------------------------
//  execute loop
//-------------------------------------------------

void E0C6S46::tick_osc1() {
    // Peripherals (MAME's emu_timers, all clocked from OSC1).
    if (--s_.ticks_to_256 <= 0) {
        s_.ticks_to_256 = 64;
        core_256_cb();
    }
    if (--s_.ticks_to_lcd <= 0) {
        s_.ticks_to_lcd = 1024;
        lcd_driver_cb();
    }
    if (s_.prgtimer_active && --s_.prgtimer_remaining <= 0) {
        prgtimer_cb();
    }
    if (s_.bz_active) {
        s_.bz_remaining -= 2;
        while (s_.bz_active && s_.bz_remaining <= 0) buzzer_cb();
    }

    if (s_.reset_held) return;

    // CPU: as many cycles as the current instruction clock fits in 1/32768 s.
    int cycles = 1;
    if (s_.cpu_clock != kOsc1) {
        s_.cycle_accu += s_.cpu_clock;
        cycles = (int)(s_.cycle_accu / kOsc1);
        s_.cycle_accu %= kOsc1;
    }
    run_cycles(cycles);
}

void E0C6S46::run_cycles(int cycles) {
    // e0c6200_cpu_device::execute_run()
    s_.icount += cycles;
    while (s_.icount > 0) {
        // check/handle interrupt, but not right after EI or in the middle of a longjump
        if (s_.possible_irq && (s_.op & 0xfe0) != 0xe40 && (s_.op & 0xff8) != 0xf48) {
            s_.possible_irq = 0;
            if ((s_.f & I_FLAG) && check_interrupt()) {
                do_interrupt();
                if (s_.icount <= 0) break;
            }
        }

        // core cpu not running (peripherals still work)
        if (s_.halt || s_.sleep) {
            s_.icount = 0;
            break;
        }

        // remember previous state, prepare pset-longjump
        s_.prev_op = s_.op;
        s_.prev_pc = s_.pc;
        s_.jpc = ((s_.prev_op & 0xfe0) == 0xe40) ? s_.npc : (s_.prev_pc & 0x1f00);

        // fetch next opcode
        s_.op = read_rom_word(s_.pc) & 0xfff;
        s_.pc = (s_.pc & 0x1000) | ((s_.pc + 1) & 0x0fff);

        // minimal opcode time is 5 clock cycles, opcodes take 5, 7, or 12 clock cycles
        s_.icount -= 5;
        execute_one();
    }
}

uint16_t E0C6S46::read_rom_word(uint16_t pc) const {
    return (pc < rom_words_) ? rom_[pc] : 0;
}

void E0C6S46::do_interrupt() {
    // interrupt handling takes 13 cycles, plus 1 extra if cpu was halted
    s_.icount -= 13;
    if (s_.halt) s_.icount--;

    s_.halt = s_.sleep = 0;
    push_pc();
    s_.f &= ~I_FLAG;

    // page 1 of the current bank
    s_.pc = (s_.pc & 0x1000) | 0x100 | s_.irq_vector;
}

bool E0C6S46::check_interrupt() {
    // priority order is not the same as register order
    static const int priorder[6] = {
        IRQREG_CLKTIMER, IRQREG_STOPWATCH, IRQREG_INPUT0, IRQREG_INPUT1, IRQREG_SERIAL, IRQREG_PRGTIMER,
    };

    // check interrupts from high to low priority
    for (int pri = 5; pri >= 0; pri--) {
        s_.irq_vector = (uint8_t)(2 * pri + 2);
        const int reg = priorder[pri];
        switch (reg) {
            // IK0/IK1 only have 1 flag bit
            case IRQREG_INPUT0:
            case IRQREG_INPUT1:
                if (s_.irqflag[reg]) return true;
                break;
            // other: mask vs flag
            default:
                if (s_.irqflag[reg] & s_.irqmask[reg]) return true;
                break;
        }
    }
    return false;
}

//-------------------------------------------------
//  data memory
//-------------------------------------------------

uint8_t E0C6S46::data_read(uint16_t addr) {
    addr &= 0xfff;
    if (addr < ram_size_) return s_.ram[addr];
    if (addr >= 0xe00 && addr < 0xe00 + vram_size_) return s_.vram[0][addr - 0xe00];
    if (addr >= 0xe80 && addr < 0xe80 + vram_size_) return s_.vram[1][addr - 0xe80];
    if (addr >= 0xf00 && addr < 0xf80) return io_r((uint8_t)(addr - 0xf00));
    return 0;
}

void E0C6S46::data_write(uint16_t addr, uint8_t data) {
    addr &= 0xfff;
    data &= 0xf;
    if (addr < ram_size_) s_.ram[addr] = data;
    else if (addr >= 0xe00 && addr < 0xe00 + vram_size_) s_.vram[0][addr - 0xe00] = data;
    else if (addr >= 0xe80 && addr < 0xe80 + vram_size_) s_.vram[1][addr - 0xe80] = data;
    else if (addr >= 0xf00 && addr < 0xf80) io_w((uint8_t)(addr - 0xf00), data);
}

uint8_t E0C6S46::read_mx() { return data_read(s_.xp << 8 | s_.xh << 4 | s_.xl) & 0xf; }
uint8_t E0C6S46::read_my() { return data_read(s_.yp << 8 | s_.yh << 4 | s_.yl) & 0xf; }
uint8_t E0C6S46::read_mn() { return data_read(s_.op & 0xf) & 0xf; }
void E0C6S46::write_mx(uint8_t data) { data_write(s_.xp << 8 | s_.xh << 4 | s_.xl, data); }
void E0C6S46::write_my(uint8_t data) { data_write(s_.yp << 8 | s_.yh << 4 | s_.yl, data); }
void E0C6S46::write_mn(uint8_t data) { data_write(s_.op & 0xf, data); }

// r/q operand: 0 = A, 1 = B, 2 = MX, 3 = MY
uint8_t E0C6S46::read_rq(int rq) {
    switch (rq & 3) {
        case 0: return s_.a;
        case 1: return s_.b;
        case 2: return read_mx();
        default: return read_my();
    }
}

void E0C6S46::write_rq(int rq, uint8_t data) {
    switch (rq & 3) {
        case 0: s_.a = data & 0xf; break;
        case 1: s_.b = data & 0xf; break;
        case 2: write_mx(data); break;
        default: write_my(data); break;
    }
}

// common stack ops

void E0C6S46::push(uint8_t data) { data_write(--s_.sp, data); }
uint8_t E0C6S46::pop() { return data_read(s_.sp++) & 0xf; }

void E0C6S46::push_pc() {
    // the highest bit(bank bit) is not pushed onto the stack
    push(s_.pc >> 8 & 0xf);
    push(s_.pc >> 4 & 0xf);
    push(s_.pc & 0xf);
}

void E0C6S46::pop_pc() {
    // the highest bit(bank bit) is unchanged
    const uint16_t bank = s_.pc & 0x1000;
    uint16_t pc = pop();
    pc |= pop() << 4;
    pc |= pop() << 8;
    s_.pc = pc | bank;
}

// misc internal helpers

void E0C6S46::set_cf(uint8_t data) { s_.f = (s_.f & ~C_FLAG) | ((data & 0x10) ? C_FLAG : 0); }
void E0C6S46::set_zf(uint8_t data) { s_.f = (s_.f & ~Z_FLAG) | ((data & 0xf) ? 0 : Z_FLAG); }

void E0C6S46::inc_x() {
    s_.xl++;
    s_.xh = (s_.xh + (s_.xl >> 4 & 1)) & 0xf;
    s_.xl &= 0xf;
}

void E0C6S46::inc_y() {
    s_.yl++;
    s_.yh = (s_.yh + (s_.yl >> 4 & 1)) & 0xf;
    s_.yl &= 0xf;
}

void E0C6S46::do_branch(int condition) {
    if (condition) s_.pc = s_.jpc | (s_.op & 0xff);
}

// arithmetic / logical (all 7 clock cycles except RRC; 5 already deducted)

uint8_t E0C6S46::op_inc(uint8_t x) {
    s_.icount -= 2;
    x++;
    set_cf(x); set_zf(x);
    return x & 0xf;
}

uint8_t E0C6S46::op_dec(uint8_t x) {
    s_.icount -= 2;
    x--;
    set_cf(x); set_zf(x);
    return x & 0xf;
}

uint8_t E0C6S46::op_add(uint8_t x, uint8_t y, int decimal) {
    s_.icount -= 2;
    x += y;
    set_cf(x);
    // decimal correction
    if ((s_.f & decimal) && x >= 10) {
        x -= 10;
        s_.f |= C_FLAG;
    }
    set_zf(x);
    return x & 0xf;
}

uint8_t E0C6S46::op_adc(uint8_t x, uint8_t y, int decimal) { return op_add(x, y + (s_.f & 1), decimal); }

uint8_t E0C6S46::op_sub(uint8_t x, uint8_t y, int decimal) {
    s_.icount -= 2;
    x -= y;
    set_cf(x);
    // decimal correction (carry remains same)
    if ((s_.f & decimal) && (s_.f & C_FLAG)) x -= 6;
    set_zf(x);
    return x & 0xf;
}

uint8_t E0C6S46::op_sbc(uint8_t x, uint8_t y, int decimal) { return op_sub(x, y + (s_.f & 1), decimal); }

uint8_t E0C6S46::op_and(uint8_t x, uint8_t y) {
    s_.icount -= 2;
    x &= y;
    set_zf(x);
    return x;
}

uint8_t E0C6S46::op_or(uint8_t x, uint8_t y) {
    s_.icount -= 2;
    x |= y;
    set_zf(x);
    return x;
}

uint8_t E0C6S46::op_xor(uint8_t x, uint8_t y) {
    s_.icount -= 2;
    x ^= y;
    set_zf(x);
    return x;
}

uint8_t E0C6S46::op_rlc(uint8_t x) {
    s_.icount -= 2;
    x = (x << 1) | (s_.f & 1);
    set_cf(x); set_zf(x);
    return x & 0xf;
}

uint8_t E0C6S46::op_rrc(uint8_t x) {
    // note: RRC only takes 5 clock cycles
    const int c = x & 1;
    x = (x >> 1) | (s_.f << 3 & 8);
    s_.f = (s_.f & ~C_FLAG) | c;
    set_zf(x);
    return x & 0xf;
}

//-------------------------------------------------
//  execute one
//-------------------------------------------------

void E0C6S46::execute_one() {
    const uint16_t op = s_.op;

    switch (op & 0xf00) {
        case 0x000: do_branch(); return;                      // JP s
        case 0x200: do_branch(s_.f & C_FLAG); return;         // JP C,s
        case 0x300: do_branch(~s_.f & C_FLAG); return;        // JP NC,s
        case 0x600: do_branch(s_.f & Z_FLAG); return;         // JP Z,s
        case 0x700: do_branch(~s_.f & Z_FLAG); return;        // JP NZ,s

        case 0x400:  // CALL s (on current bank)
            s_.icount -= 2;
            push_pc();
            s_.pc = (s_.pc & 0x1000) | (s_.jpc & 0x0f00) | (op & 0xff);
            return;

        case 0x500:  // CALZ s (zero page, on current bank)
            s_.icount -= 2;
            push_pc();
            s_.pc = (s_.pc & 0x1000) | (op & 0xff);
            return;

        case 0x100:  // RETD e: return from subroutine, then LBPX MX,e
            s_.icount -= 7;
            pop_pc();
            [[fallthrough]];
        case 0x900:  // LBPX MX,e
            write_mx(op & 0xf); inc_x();
            write_mx(op >> 4 & 0xf); inc_x();
            return;

        case 0xb00:  // LD X,e
            s_.xh = op >> 4 & 0xf;
            s_.xl = op & 0xf;
            return;

        case 0x800:  // LD Y,e
            s_.yh = op >> 4 & 0xf;
            s_.yl = op & 0xf;
            return;

        default:
            break;
    }

    const int r = (op >> 2) & 3;  // r,q forms: rrqq in the low nibble
    const int q = op & 3;

    switch (op & 0xff0) {
        // LD r,q / LDPX r,q / LDPY r,q
        case 0xec0: write_rq(r, read_rq(q)); return;
        case 0xee0: write_rq(r, read_rq(q)); inc_x(); return;
        case 0xef0: write_rq(r, read_rq(q)); inc_y(); return;

        // ADD/ADC/SUB/SBC r,q (flags: C, Z; decimal mode applies)
        case 0xa80: { uint8_t x = read_rq(r), y = read_rq(q); write_rq(r, op_add(x, y, D_FLAG)); return; }
        case 0xa90: { uint8_t x = read_rq(r), y = read_rq(q); write_rq(r, op_adc(x, y, D_FLAG)); return; }
        case 0xaa0: { uint8_t x = read_rq(r), y = read_rq(q); write_rq(r, op_sub(x, y, D_FLAG)); return; }
        case 0xab0: { uint8_t x = read_rq(r), y = read_rq(q); write_rq(r, op_sbc(x, y, D_FLAG)); return; }

        // AND/OR/XOR r,q (flags: Z)
        case 0xac0: { uint8_t x = read_rq(r), y = read_rq(q); write_rq(r, op_and(x, y)); return; }
        case 0xad0: { uint8_t x = read_rq(r), y = read_rq(q); write_rq(r, op_or(x, y)); return; }
        case 0xae0: { uint8_t x = read_rq(r), y = read_rq(q); write_rq(r, op_xor(x, y)); return; }

        // RLC r(,r): only r == q is a valid encoding
        case 0xaf0:
            switch (op & 0xf) {
                case 0x0: s_.a = op_rlc(s_.a); return;
                case 0x5: s_.b = op_rlc(s_.b); return;
                case 0xa: read_mx(); write_mx(op_rlc(read_mx())); return;
                case 0xf: read_my(); write_my(op_rlc(read_my())); return;
                default: return;  // illegal
            }

        // CP r,q (flags: C, Z, no D flag) / FAN r,q (flags: Z)
        case 0xf00: { uint8_t x = read_rq(r), y = read_rq(q); op_sub(x, y); return; }
        case 0xf10: { uint8_t x = read_rq(r), y = read_rq(q); op_and(x, y); return; }

        // ACPX/ACPY MX/MY,r and SCPX/SCPY MX/MY,r (flags: C, Z)
        case 0xf20:
            if (op & 8) {
                if (op & 4) { write_my(op_adc(read_my(), read_rq(q), D_FLAG)); inc_y(); }
                else { write_mx(op_adc(read_mx(), read_rq(q), D_FLAG)); inc_x(); }
            }
            return;  // 0xf20-0xf27 illegal
        case 0xf30:
            if (op & 8) {
                if (op & 4) { write_my(op_sbc(read_my(), read_rq(q), D_FLAG)); inc_y(); }
                else { write_mx(op_sbc(read_mx(), read_rq(q), D_FLAG)); inc_x(); }
            }
            return;  // 0xf30-0xf37 illegal

        // LD XP/XH/XL,r (0xe80-0xe8b), RRC r (0xe8c-0xe8f)
        case 0xe80:
            switch (r) {
                case 0: s_.xp = read_rq(q); return;
                case 1: s_.xh = read_rq(q); return;
                case 2: s_.xl = read_rq(q); return;
                default: write_rq(q, op_rrc(read_rq(q))); return;
            }

        // LD YP/YH/YL,r
        case 0xe90:
            switch (r) {
                case 0: s_.yp = read_rq(q); return;
                case 1: s_.yh = read_rq(q); return;
                case 2: s_.yl = read_rq(q); return;
                default: return;  // illegal
            }

        // LD r,XP/XH/XL and LD r,YP/YH/YL
        case 0xea0:
            switch (r) {
                case 0: write_rq(q, s_.xp); return;
                case 1: write_rq(q, s_.xh); return;
                case 2: write_rq(q, s_.xl); return;
                default: return;  // illegal
            }
        case 0xeb0:
            switch (r) {
                case 0: write_rq(q, s_.yp); return;
                case 1: write_rq(q, s_.yh); return;
                case 2: write_rq(q, s_.yl); return;
                default: return;  // illegal
            }

        // PUSH r/Xphl/Yphl/F, DEC SP
        case 0xfc0:
            switch (op & 0xf) {
                case 0x0: push(s_.a); return;
                case 0x1: push(s_.b); return;
                case 0x2: push(read_mx()); return;
                case 0x3: push(read_my()); return;
                case 0x4: push(s_.xp); return;
                case 0x5: push(s_.xh); return;
                case 0x6: push(s_.xl); return;
                case 0x7: push(s_.yp); return;
                case 0x8: push(s_.yh); return;
                case 0x9: push(s_.yl); return;
                case 0xa: push(s_.f); return;
                case 0xb: s_.sp--; return;  // DEC SP
                default: return;  // illegal
            }

        // POP r/Xphl/Yphl/F, INC SP, RETS, RET
        case 0xfd0:
            switch (op & 0xf) {
                case 0x0: s_.a = pop(); return;
                case 0x1: s_.b = pop(); return;
                case 0x2: write_mx(pop()); return;
                case 0x3: write_my(pop()); return;
                case 0x4: s_.xp = pop(); return;
                case 0x5: s_.xh = pop(); return;
                case 0x6: s_.xl = pop(); return;
                case 0x7: s_.yp = pop(); return;
                case 0x8: s_.yh = pop(); return;
                case 0x9: s_.yl = pop(); return;
                case 0xa: s_.f = pop(); s_.possible_irq = 1; return;
                case 0xb: s_.sp++; return;  // INC SP
                case 0xe:  // RETS: return from subroutine, then skip next instruction
                    s_.icount -= 7;
                    pop_pc();
                    s_.pc = (s_.pc & 0x1000) | ((s_.pc + 1) & 0x0fff);
                    return;
                case 0xf:  // RET
                    s_.icount -= 2;
                    pop_pc();
                    return;
                default: return;  // illegal
            }

        // LD SPL,r / LD r,SPH / JPBA
        case 0xfe0:
            switch (op & 0xf) {
                case 0x0: case 0x1: case 0x2: case 0x3:
                    s_.sp = (s_.sp & 0xf0) | read_rq(q); return;
                case 0x4: case 0x5: case 0x6: case 0x7:
                    write_rq(q, s_.sp >> 4 & 0xf); return;
                case 0x8:  // JPBA: jump indirect using registers A and B
                    s_.pc = s_.jpc | s_.b << 4 | s_.a;
                    return;
                default: return;  // illegal
            }

        // LD SPH,r / LD r,SPL / HALT / SLP / NOP5 / NOP7
        case 0xff0:
            switch (op & 0xf) {
                case 0x0: case 0x1: case 0x2: case 0x3:
                    s_.sp = (s_.sp & 0x0f) | read_rq(q) << 4; return;
                case 0x4: case 0x5: case 0x6: case 0x7:
                    write_rq(q, s_.sp & 0xf); return;
                case 0x8: s_.halt = 1; return;  // HALT
                case 0x9: return;               // SLP: E0C6S46 has no support for it (illegal)
                case 0xb: return;               // NOP5
                case 0xf: s_.icount -= 2; return;  // NOP7
                default: return;  // illegal
            }

        // LD r,i / LDPX MX,i / LDPY MY,i
        case 0xe00: s_.a = op & 0xf; return;
        case 0xe10: s_.b = op & 0xf; return;
        case 0xe20: write_mx(op & 0xf); return;
        case 0xe30: write_my(op & 0xf); return;
        case 0xe60: write_mx(op & 0xf); inc_x(); return;
        case 0xe70: write_my(op & 0xf); inc_y(); return;

        // PSET p: page set, used to set page/bank before a jump instruction
        case 0xe40:
        case 0xe50:
            s_.npc = op << 8 & 0x1f00;
            return;

        // LD A/B,Mn, LD Mn,A/B, INC/DEC Mn (flags: C, Z)
        case 0xfa0: s_.a = read_mn(); return;
        case 0xfb0: s_.b = read_mn(); return;
        case 0xf80: write_mn(s_.a); return;
        case 0xf90: write_mn(s_.b); return;
        case 0xf60: write_mn(op_inc(read_mn())); return;
        case 0xf70: write_mn(op_dec(read_mn())); return;

        // ADD/ADC r,i (flags: C, Z)
        case 0xc00: case 0xc10: case 0xc20: case 0xc30: {
            const int rr = op >> 4 & 3;
            write_rq(rr, op_add(read_rq(rr), op & 0xf, D_FLAG));
            return;
        }
        case 0xc40: case 0xc50: case 0xc60: case 0xc70: {
            const int rr = op >> 4 & 3;
            write_rq(rr, op_adc(read_rq(rr), op & 0xf, D_FLAG));
            return;
        }

        // ADC XH/XL/YH/YL,i (flags: C, Z, no D flag)
        case 0xa00: s_.xh = op_adc(s_.xh, op & 0xf); return;
        case 0xa10: s_.xl = op_adc(s_.xl, op & 0xf); return;
        case 0xa20: s_.yh = op_adc(s_.yh, op & 0xf); return;
        case 0xa30: s_.yl = op_adc(s_.yl, op & 0xf); return;

        // CP XH/XL/YH/YL,i (flags: C, Z, no D flag)
        case 0xa40: op_sub(s_.xh, op & 0xf); return;
        case 0xa50: op_sub(s_.xl, op & 0xf); return;
        case 0xa60: op_sub(s_.yh, op & 0xf); return;
        case 0xa70: op_sub(s_.yl, op & 0xf); return;

        // SBC r,i (flags: C, Z)
        case 0xd40: case 0xd50: case 0xd60: case 0xd70: {
            const int rr = op >> 4 & 3;
            write_rq(rr, op_sbc(read_rq(rr), op & 0xf, D_FLAG));
            return;
        }

        // CP r,i (flags: C, Z, no D flag)
        case 0xdc0: case 0xdd0: case 0xde0: case 0xdf0:
            op_sub(read_rq(op >> 4 & 3), op & 0xf);
            return;

        // AND r,i / FAN r,i / OR r,i / XOR r,i
        case 0xc80: case 0xc90: case 0xca0: case 0xcb0: {
            const int rr = op >> 4 & 3;
            write_rq(rr, op_and(read_rq(rr), op & 0xf));
            return;
        }
        case 0xd80: case 0xd90: case 0xda0: case 0xdb0:
            op_and(read_rq(op >> 4 & 3), op & 0xf);
            return;
        case 0xcc0: case 0xcd0: case 0xce0: case 0xcf0: {
            const int rr = op >> 4 & 3;
            write_rq(rr, op_or(read_rq(rr), op & 0xf));
            return;
        }
        case 0xd00: case 0xd10: case 0xd20: case 0xd30: {
            const int rr = op >> 4 & 3;
            write_rq(rr, op_xor(read_rq(rr), op & 0xf));
            return;
        }

        // SET F,i (includes SCF, SZF, SDF, EI)
        case 0xf40:
            s_.icount -= 2;
            s_.f |= (op & 0xf);
            s_.possible_irq = 1;
            return;

        // RST F,i (includes RCF, RZF, RDF, DI)
        case 0xf50:
            s_.icount -= 2;
            s_.f &= (op & 0xf);
            return;

        default:
            return;  // illegal opcode
    }
}
