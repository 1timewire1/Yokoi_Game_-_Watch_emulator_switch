#ifndef TAMAGO_MACHINE_H
#define TAMAGO_MACHINE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "tamago_cpu6502.h"

typedef struct {
    TamagoCpu cpu;
    const uint8_t *bios;
    size_t bios_size;
    uint8_t wram[0x600];
    uint8_t dram[0x200];
    uint8_t regs[0x100];
    uint8_t keys;
    uint8_t figure; /* 0 = none, 1..3 = figure CD1/CD2 bits on port A bits 5-6 */
    uint32_t last_cycles;
    uint8_t bank;
    uint16_t pending_irq;
    uint16_t irq_vectors[16];
    /* 24C-style serial EEPROM: 4 KiB, matching upstream eeprom(12). */
    uint8_t eeprom[0x1000];
    uint8_t eeprom_state;
    uint8_t eeprom_bits;
    uint8_t eeprom_shift;
    uint8_t eeprom_addr_bytes;
    uint16_t eeprom_addr;
    uint8_t eeprom_last_clk;
    uint8_t eeprom_last_data;
    uint8_t eeprom_data;
    uint8_t eeprom_clk;
    uint8_t eeprom_power;
    uint8_t eeprom_out;
    uint64_t timer_cycles;
    uint64_t tbh_cycles;
    bool bios_valid;
    bool eeprom_dirty; /* set whenever the firmware writes the EEPROM; the frontend clears it after saving */
} TamagoMachine;

void tamago_machine_init(TamagoMachine *m, const uint8_t *bios, size_t bios_size);
void tamago_machine_reset(TamagoMachine *m);
int tamago_machine_step(TamagoMachine *m);
#define TAMAGO_CLOCK_HZ 4000000u
/* Runs whole instructions until at least cycle_budget cycles elapsed; returns -1 on invalid opcode. */
int tamago_machine_run(TamagoMachine *m, uint32_t cycle_budget);
void tamago_machine_frame(TamagoMachine *m);
/* Fast-forward: one half second of the game's own time compressed into `cycles` CPU cycles. Fires what a real half
   second contains that matters for the game's clock and care logic - a frame interrupt (IRQ10 + NMI) and the 2 Hz
   time base (IRQ13, normally every 2,000,000 cycles) - and runs the CPU for `cycles` only, instead of 2,000,000. The
   game sees hours go by in seconds of host time. Sound produced meanwhile is dropped. */
void tamago_machine_tick(TamagoMachine *m, uint32_t cycles);
/* ---- Gotchi figures (Tama-Go only) ----------------------------------------------------
   contacts: bit0 = CD1, bit1 = CD2 (what the firmware sees on port A bits 5-6).
   rom/len: raw dump of the figure's SPI flash (NULL/0 = contacts only, like a Lite figure).
   The buffer must stay valid until the next call. tamago_machine_set_figure(m, 0, NULL, 0) removes it. */
void tamago_machine_set_figure(TamagoMachine *m, uint8_t contacts, const uint8_t *rom, size_t len);
bool tamago_machine_is_friends(void); /* of the primary console; Tamagotchi Friends has no figures */
bool tamago_machine_is_friends_of(const TamagoMachine *m);
/* Several consoles can live in one process (max 2). Sound and the debug trace follow the PRIMARY one, which is the last
   console initialised unless this selects another. */
void tamago_machine_set_primary(const TamagoMachine *m);
/* ---- Infrared link between two consoles ------------------------------------------------------------------------
   The firmware sends with the LED on port B bit 3 and listens on port A bit 7 (1 = idle, 0 = carrier). Connected
   consoles hear each other's LED. Because the pulses are only 27 cycles apart the two consoles must run in lock step:
   use tamago_link_run() instead of tamago_machine_run() (call tamago_machine_frame() on both first, as usual).
   Returns the first non-zero result of either console's step (-2 if the second one failed). */
void tamago_link_connect(TamagoMachine *a, TamagoMachine *b);
void tamago_link_disconnect(TamagoMachine *m);
/* Call before connecting: makes sure both consoles carry the firmware's link timing thresholds (zero page $7C..$7F), which a
   state saved by an older build lacks; see machine.c. Returns false if they could not be obtained (not a Tama-Go ROM). */
bool tamago_link_prepare(TamagoMachine *a, TamagoMachine *b);
/* Frees the per-console block of a console that is no longer used. */
void tamago_machine_release(const TamagoMachine *m);
bool tamago_link_connected(const TamagoMachine *m);
int tamago_link_run(TamagoMachine *a, TamagoMachine *b, uint32_t cycle_budget);
/* ---- Sound ---------------------------------------------------------------------------
   The firmware makes sound in software: TM0 (IRQ0) fires every (0x10000 - value of $3033:$3032)
   timer ticks and its handler flips the DAC register $3060 between +v and -v (a square wave);
   the time base TBH (IRQ12) clocks the note sequencer. Upstream never raised either interrupt,
   so the firmware never started a note. Pitch = timer 0 clock / (2 * period); the clock is TAMAGO_T0_HZ (measured), which makes the
   firmware's note table equal-tempered. */
#define TAMAGO_T0_HZ 10666667u /* clock of timer 0: 4 MHz x 8/3, measured on a recording of T7Emu running on the Tamagotchi Paradise (T7Emu's own constant is 8 MHz; the device plays 4/3 higher) */
#define TAMAGO_TBH_HZ 128u /* tempo reference: 128 = 100 % of the rate the registers ask for (512 Hz with $FC00) */
#define TAMAGO_AUDIO_RATE 48000u
void tamago_audio_irqs_enable(bool on); /* default on; the upstream diff test turns it off */
/* Renders the DAC activity of the frame just run into stereo s16 (n_samples frames).
   frame_cycles = cycles that frame spans (TAMAGO_CLOCK_HZ / 60). */
void tamago_audio_render(int16_t *stereo_out, size_t n_samples, uint32_t frame_cycles);

/* Sound tuning (defaults: the values above, measured against T7Emu's model and a recording of the real device). They can
   be changed at run time:
   - tbh_hz: tempo reference, 128 = 100 % of the IRQ12 rate the timer 1 registers ask for (default TAMAGO_TBH_HZ).
   - tm0_clock_hz: clock of timer TM0 that paces the DAC steps (default TAMAGO_T0_HZ, 10.667 MHz): pitch.
   - timbre: 0 = the DAC as it is, 1 = the clear tone T7Emu generates from pitch and envelope (low-passed like the recording). */
void tamago_audio_tune(uint32_t tbh_hz, uint32_t tm0_clock_hz);
void tamago_audio_timbre(int timbre);

/* Optional debug trace of audio/timer related register writes and key changes (not part of save states). */
typedef struct { uint32_t frame, cycle; uint16_t addr, pc; uint8_t value, type; } TamagoTraceEntry; /* type 0 = write, 1 = keys, 2 = read (extended only) */
void tamago_trace_enable(bool on);
/* Extended mode (call before tamago_trace_enable): every $30xx register incl. port B / EEPROM, writes AND reads, only
   when the value changed (link / infrared research). */
void tamago_trace_extended(bool on);
bool tamago_trace_is_extended(void);
bool tamago_trace_enabled(void);
void tamago_trace_keys(uint8_t keys);
size_t tamago_trace_count(void);
const TamagoTraceEntry *tamago_trace_data(void);
void tamago_trace_clear(void);

/* Full save state (CPU, RAM, registers, EEPROM, timers). Pointers are not stored; a BIOS checksum is. */
size_t tamago_machine_state_size(void);
void tamago_machine_serialize(const TamagoMachine *m, uint8_t *out);
bool tamago_machine_deserialize(TamagoMachine *m, const uint8_t *in, size_t size);
void tamago_machine_load_eeprom(TamagoMachine *m, const uint8_t *data, size_t size);
uint8_t tamago_machine_read(void *ctx, uint16_t addr);
void tamago_machine_write(void *ctx, uint16_t addr, uint8_t value);
#ifdef __cplusplus
}
#endif
#endif
