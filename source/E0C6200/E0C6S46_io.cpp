// E0C6S46/E0C6S48 on-chip peripherals - port of MAME's e0c6s46.cpp
// (license: BSD-3-Clause, copyright-holders: hap). See E0C6S46.h.
//
// MAME schedules these with emu_timers; here each one is a countdown in OSC1
// ticks (the buzzer's in half ticks, since its pulse widths are counted at
// 2*OSC1 for the higher frequencies) decremented by E0C6S46::tick_osc1().

#include "E0C6S46.h"

#include <cstring>

//-------------------------------------------------
//  K input ports
//-------------------------------------------------

void E0C6S46::set_k_line(int line, bool level) {
    // only support 8 K input lines at the moment
    if (line < 0 || line > 7) return;

    const uint8_t port = line >> 2 & 1;
    const uint8_t bit = 1 << (line & 3);

    const uint8_t prev = s_.port_k[port];
    s_.port_k[port] = (s_.port_k[port] & ~bit) | (level ? bit : 0);

    // set IRQ flag on falling/rising edge of input
    const uint8_t dfk = (port == 0) ? s_.dfk0 : 0xf;
    const uint8_t edge = ~(prev ^ dfk) & (s_.port_k[port] ^ dfk) & bit;
    if (s_.irqmask[IRQREG_INPUT0 + port] & edge) {
        s_.irqflag[IRQREG_INPUT0 + port] |= 1;
        s_.possible_irq = 1;
    }

    // clock programmable timer on falling edge of K03
    if ((prev & ~s_.port_k[port] & bit) && s_.prgtimer_on && (s_.prgtimer_select & 7) < 2) clock_prgtimer();
}

//-------------------------------------------------
//  R output ports / P I/O ports
//-------------------------------------------------

void E0C6S46::write_r(uint8_t port, uint8_t data) {
    data &= 0xf;
    s_.port_r[port] = data;

    // R0-R3 aren't wired to anything on these boards. R43: buzzer on (low):
    // direct output or 1-shot output. The speaker hangs off R43's buzzer
    // output, which is just bz_pulse (see get_active_sound()).
    if (port == 4 && (data & 8) != s_.bz_43_on) {
        s_.bz_43_on = data & 8;
        reset_bz_envelope();
        reset_buzzer();
    }
}

void E0C6S46::write_p(uint8_t port, uint8_t data) {
    // Only Digital Monster's link port is wired up in MAME, and it ignores
    // writes - nothing to drive.
    s_.port_p[port] = data & 0xf;
}

uint8_t E0C6S46::read_p(uint8_t port) {
    // return written value if port direction is set to output
    if (s_.p_dir >> port & 1) return s_.port_p[port];
    return p_input_[port];
}

//-------------------------------------------------
//  timers
//-------------------------------------------------

void E0C6S46::core_256_cb() {
    // clock-timer, stopwatch timer, and some features of the buzzer all run
    // from the same internal 256hz timer (64 ticks high+low at default clock of 32768hz)
    s_.src_256_pulse ^= 1;

    // clock stopwatch on falling edge of pulse+on
    s_.swl_cur_pulse = s_.src_256_pulse | (s_.stopwatch_on ^ 1);
    if (s_.swl_cur_pulse == 0) clock_stopwatch();

    // clock 1-shot buzzer on rising edge if it's on
    if (s_.bz_1shot_on != 0 && s_.src_256_pulse == 1) clock_bz_1shot();

    // clock buzzer envelope on rising edge if it's on
    if ((s_.bz_envelope & 1) && s_.src_256_pulse == 1) clock_bz_envelope();

    // clock-timer is always running, advance it on falling edge
    // (handle clock_clktimer last in case of watchdog reset)
    if (s_.src_256_pulse == 0) clock_clktimer();
}

// clock-timer

void E0C6S46::clock_watchdog() {
    // initial reset after 3 to 4 seconds
    if (++s_.watchdog_count == 4) {
        s_.watchdog_count = 0;
        device_reset();
    }
}

void E0C6S46::clock_clktimer() {
    s_.clktimer_count++;

    // IRQ on falling edge of 32, 8, 2, 1hz
    uint8_t flag = 0;
    if ((s_.clktimer_count & 0x07) == 0) flag |= 1;
    if ((s_.clktimer_count & 0x1f) == 0) flag |= 2;
    if ((s_.clktimer_count & 0x7f) == 0) flag |= 4;
    if (s_.clktimer_count == 0) flag |= 8;

    s_.irqflag[IRQREG_CLKTIMER] |= flag;
    if (s_.irqflag[IRQREG_CLKTIMER] & s_.irqmask[IRQREG_CLKTIMER]) s_.possible_irq = 1;

    // 1hz falling edge also clocks the watchdog timer
    if (s_.clktimer_count == 0) clock_watchdog();
}

// stopwatch timer

void E0C6S46::clock_stopwatch() {
    s_.swl_slice++;

    // 1 slice is 3 ticks(256hz) on even and 2 ticks on uneven counts,
    // but from count 1 to 2 it's 3 ticks, 6 out of 100 times, to make
    // exactly 26/256hz * 6 + 25/256hz * 4 = 1 second
    int swl_next = 3 - (s_.swl_count & 1);
    if (s_.swl_count == 1 && !(s_.swh_count >> 1 & 1)) swl_next = 3;

    if (s_.swl_slice == swl_next) {
        s_.swl_slice = 0;

        // bcd counter, IRQ on falling edge of 10 and 1hz
        s_.swl_count = (s_.swl_count + 1) % 10;
        if (s_.swl_count == 0) {
            s_.irqflag[IRQREG_STOPWATCH] |= 1;
            s_.swh_count = (s_.swh_count + 1) % 10;
            if (s_.swh_count == 0) s_.irqflag[IRQREG_STOPWATCH] |= 2;

            if (s_.irqflag[IRQREG_STOPWATCH] & s_.irqmask[IRQREG_STOPWATCH]) s_.possible_irq = 1;
        }
    }
}

// programmable timer

void E0C6S46::clock_prgtimer() {
    // IRQ and reload when it reaches zero
    if (--s_.prgtimer_count == 0) {
        s_.irqflag[IRQREG_PRGTIMER] |= 1;
        if (s_.irqflag[IRQREG_PRGTIMER] & s_.irqmask[IRQREG_PRGTIMER]) s_.possible_irq = 1;

        // note: a reload of 0 indicates a 256-counter
        s_.prgtimer_count = s_.prgtimer_reload;
    }
}

bool E0C6S46::prgtimer_reset_prescaler() {
    // only 2 to 7 are clock dividers
    const uint8_t sel = s_.prgtimer_select & 7;
    s_.prgtimer_active = sel >= 2;
    if (sel >= 2) s_.prgtimer_remaining = 2 << (sel ^ 7);
    return sel >= 2;
}

void E0C6S46::prgtimer_cb() {
    // check if it's clocked by osc1, schedule next timeout
    if (!prgtimer_reset_prescaler()) return;

    s_.prgtimer_src_pulse ^= 1;
    s_.prgtimer_cur_pulse = s_.prgtimer_src_pulse | (s_.prgtimer_on ^ 1);

    // clock prgtimer on falling edge of pulse+on
    if (s_.prgtimer_cur_pulse == 0) clock_prgtimer();
}

// buzzer

void E0C6S46::schedule_buzzer() {
    // only schedule next buzzer timeout if it's on
    if (s_.bz_43_on && !s_.bz_1shot_running) return;

    // pulse width differs per frequency selection
    const int mul = (s_.bz_freq & 4) ? 1 : 2;
    int high = (s_.bz_freq & 2) ? 12 : 8;
    int low = 16 + (s_.bz_freq << 2 & 0xc);

    // pulse width envelope if it's on
    if (s_.bz_envelope & 1) high -= s_.bz_duty_ratio;
    low -= high;

    // from_ticks(n, mul * osc1), counted here in 1/(2*osc1) units
    s_.bz_active = 1;
    s_.bz_remaining = (s_.bz_pulse ? high : low) * (mul == 1 ? 2 : 1);
}

void E0C6S46::buzzer_cb() {
    const int32_t carry = s_.bz_remaining;  // <= 0: how far past the edge this tick landed
    s_.bz_active = 0;

    // invert pulse wave if buzzer is on
    if (s_.bz_43_on && !s_.bz_1shot_running) s_.bz_pulse = 0;
    else s_.bz_pulse ^= 1;

    schedule_buzzer();
    if (s_.bz_active) s_.bz_remaining += carry;  // keep the phase across ticks
}

void E0C6S46::reset_buzzer() {
    // don't reset if the timer is running
    if (!s_.bz_active) schedule_buzzer();
}

void E0C6S46::clock_bz_1shot() {
    s_.bz_1shot_running = 1;

    // reload counter the 1st time
    if (s_.bz_1shot_count == 0) {
        s_.bz_1shot_count = (s_.bz_freq & 8) ? 16 : 8;
        reset_buzzer();
    }
    // stop ringing when counter reaches 0
    else if (--s_.bz_1shot_count == 0) {
        s_.bz_1shot_on = 0;
        s_.bz_1shot_running = 0;
    }
}

void E0C6S46::clock_bz_envelope() {
    if (--s_.bz_envelope_count == 0) {
        s_.bz_envelope_count = (s_.bz_envelope & 2) ? 32 : 16;

        // maximum duty addition is 7
        if (s_.bz_duty_ratio < 7) s_.bz_duty_ratio++;
    }
}

void E0C6S46::reset_bz_envelope() {
    s_.bz_envelope_count = (s_.bz_envelope & 2) ? 32 : 16;
    s_.bz_duty_ratio = 0;
}

//-------------------------------------------------
//  LCD driver
//-------------------------------------------------

void E0C6S46::lcd_driver_cb() {
    // this gets called 32 times per second (osc1/1024)
    for (int bank = 0; bank < 2; bank++) {
        // determine operating mode
        bool lcd_on = false;
        uint8_t pixel = 0;
        if ((s_.lcd_control & 8) || (bank == 1 && (s_.lcd_control & 2))) pixel = 0;
        else if (s_.lcd_control & 4) pixel = 1;
        else lcd_on = true;

        for (int offset = 0; offset < vram_size_; offset++) {
            for (int c = 0; c < 4; c++) {
                if (lcd_on) pixel = s_.vram[bank][offset] >> c & 1;

                // 16 COM(common) pins, 40/51 SEG(segment) pins
                const int seg = offset / 2;
                const int com = bank * 8 + (offset & 1) * 4 + c;
                s_.lcd[com][seg] = pixel;
            }
        }
    }
    segments_state_are_update = true;
}

//-------------------------------------------------
//  internal I/O
//-------------------------------------------------

uint8_t E0C6S46::io_r(uint8_t offset) {
    switch (offset) {
        // IRQ flags, masks
        case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: {
            // IRQ flags are reset(acked) when read
            const uint8_t flag = s_.irqflag[offset];
            s_.irqflag[offset] = 0;
            return flag;
        }
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
            return s_.irqmask[offset & 7];

        // K input ports
        case 0x40: case 0x42:
            return s_.port_k[offset >> 1 & 1];
        case 0x41:
            return s_.dfk0;

        // R output ports
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54:
            return s_.port_r[offset & 7];
        case 0x7b:
            return s_.r_dir;

        // P I/O ports
        case 0x60: case 0x61: case 0x62: case 0x63:
            return read_p(offset & 3);
        case 0x7d:
            return s_.p_dir;
        case 0x7e:
            return s_.p_pullup;

        // clock-timer (lo, hi)
        case 0x20: case 0x21:
            return s_.clktimer_count >> (4 * (offset & 1)) & 0xf;

        // stopwatch timer
        case 0x22:
            return (uint8_t)s_.swl_count;
        case 0x23:
            return (uint8_t)s_.swh_count;
        case 0x77:
            return s_.stopwatch_on;

        // programmable timer
        case 0x24: case 0x25:
            return s_.prgtimer_count >> (4 * (offset & 1)) & 0xf;
        case 0x26: case 0x27:
            return s_.prgtimer_reload >> (4 * (offset & 1)) & 0xf;
        case 0x78:
            return s_.prgtimer_on;
        case 0x79:
            return s_.prgtimer_select;

        // buzzer
        case 0x74:
            return s_.bz_freq;
        case 0x75:
            // d3: 1-shot buzzer is on
            return s_.bz_1shot_on | s_.bz_envelope;

        // OSC circuit
        case 0x70:
            return s_.osc;

        // LCD driver
        case 0x71:
            return s_.lcd_control;
        case 0x72:
            return s_.lcd_contrast;

        // SVD circuit (supply voltage detection)
        case 0x73:
            // d3: criteria voltage* is 0: <=, 1: > source voltage (Vdd-Vss)
            // *0,1,2,3: -2.2V, -2.5V, -3.1V, -4.2V, 1 when off
            return s_.svd | (((s_.svd & 4) && s_.svd != 7) ? 0 : 8);

        default:
            return 0;
    }
}

void E0C6S46::io_w(uint8_t offset, uint8_t data) {
    switch (offset) {
        // IRQ masks
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: {
            static const uint8_t maskmask[6] = {0xf, 3, 1, 1, 0xf, 0xf};
            const uint8_t prev = s_.irqmask[offset & 7];
            s_.irqmask[offset & 7] = data & maskmask[offset & 7];
            const uint8_t rise = ~prev & s_.irqmask[offset & 7];

            if (rise) {
                s_.possible_irq = 1;

                if (offset >= 0x14) {
                    const uint8_t port = offset & 1;
                    const uint8_t dfk = (port == 0) ? s_.dfk0 : 0xf;

                    // IK flag gets set if input is active at rising edge of mask bit
                    if (rise & (s_.port_k[port] ^ dfk)) s_.irqflag[offset & 7] |= 1;
                }
            }
            break;
        }

        // K input ports
        case 0x41:
            // d0-d3: K0x IRQ on 0: rising edge, 1: falling edge
            s_.dfk0 = data;
            break;

        // R output ports
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54:
            write_r(offset & 7, data);
            break;
        case 0x7b:
            // d0-d3: Rx* direction 0: high-impedance, 1: output
            if (data != s_.r_dir) {
                s_.r_dir = data;
                for (int i = 0; i < 5; i++) write_r(i, s_.port_r[i]);
            }
            break;

        // P I/O ports
        case 0x60: case 0x61: case 0x62: case 0x63:
            write_p(offset & 3, data);
            break;
        case 0x7d:
            // d0-d3: Px* direction 0: input, 1: output
            if (data != s_.p_dir) {
                s_.p_dir = data;
                for (int i = 0; i < 4; i++) write_p(i, s_.port_p[i]);
            }
            break;
        case 0x7e:
            // d0-d3: Px* pull up resistor on/off
            s_.p_pullup = data;
            break;

        // OSC circuit
        case 0x70:
            // d0,d1: CPU operating voltage
            // d2: OSC3 on (high freq)
            // d3: clock source OSC1 or OSC3
            s_.osc = data;
            // (MAME's osc_change timer fires with zero delay; boards without
            // OSC3 have osc3 == osc1, so this is a no-op there too.)
            s_.cpu_clock = (s_.osc & 8) ? s_.osc3 : kOsc1;
            if (s_.cpu_clock == kOsc1) s_.cycle_accu = 0;
            break;

        // LCD driver
        case 0x71:
            // d0: heavy load protection
            // d1: duty 1/16 or 1/8
            // d2,d3: all pixels on,off
            s_.lcd_control = data;
            break;
        case 0x72:
            // contrast adjustment (0=light, 15=dark) - not hooked up (MAME doesn't either)
            s_.lcd_contrast = data;
            break;

        // SVD circuit (supply voltage detection)
        case 0x73:
            // d0,d1: criteria voltage
            // d2: on
            s_.svd = data & 7;
            break;

        // clock-timer
        case 0x76:
            // d0: reset watchdog
            // d1: reset clktimer
            if (data & 1) s_.watchdog_count = 0;
            if (data & 2) s_.clktimer_count = 0;
            break;

        // stopwatch timer
        case 0x77:
            // d0: run/stop counter
            // d1: reset stopwatch
            s_.stopwatch_on = data & 1;
            if (data & 2) {
                s_.swh_count = 0;
                s_.swl_count = 0;
                s_.swl_slice = 0;
            }
            if (s_.stopwatch_on && s_.swl_cur_pulse && !s_.src_256_pulse) {
                // clock stopwatch on falling edge of pulse+on
                s_.swl_cur_pulse = 0;
                clock_stopwatch();
            }
            break;

        // programmable timer
        case 0x26:
            s_.prgtimer_reload = (s_.prgtimer_reload & 0xf0) | data;
            break;
        case 0x27:
            s_.prgtimer_reload = (s_.prgtimer_reload & 0x0f) | data << 4;
            break;

        case 0x78:
            // d0: run/stop counter
            // d1: reset timer
            s_.prgtimer_on = data & 1;
            if (data & 2) s_.prgtimer_count = s_.prgtimer_reload;
            if (s_.prgtimer_on && (s_.prgtimer_select & 7) >= 2 && s_.prgtimer_cur_pulse && !s_.prgtimer_src_pulse) {
                // if input clock is osc1, clock timer on falling edge of pulse+on
                s_.prgtimer_cur_pulse = 0;
                clock_prgtimer();
            }
            break;

        case 0x79:
            // d0-d2: input clock select: 2-7 = osc1 divider 256hz-8192hz,
            // 0/1 = K03 input (0 enables noise rejector, no need to emulate that)
            // d3: output input clock to output port R33
            if ((data & 7) != (s_.prgtimer_select & 7)) {
                s_.prgtimer_src_pulse = 0;
                s_.prgtimer_cur_pulse = s_.prgtimer_on ^ 1;
                s_.prgtimer_select = data;
                prgtimer_reset_prescaler();
            }
            s_.prgtimer_select = data;
            break;

        // buzzer
        case 0x74:
            // d0-d2: frequency (8 steps, 4096hz to ~1170hz)
            // d3: 1-shot buzzer duration 31.25ms or 62.5ms
            s_.bz_freq = data;
            break;
        case 0x75:
            // d0: envelope on/off
            // d1: envelope cycle selection
            // d2: reset envelope
            // d3: trigger one-shot buzzer
            if ((~s_.bz_envelope & data & 1) || (data & 4)) reset_bz_envelope();
            s_.bz_envelope = data & 3;
            s_.bz_1shot_on |= data & 8;
            break;

        default:
            // read-only or unknown registers
            break;
    }
}
