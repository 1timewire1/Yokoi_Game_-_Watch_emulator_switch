#include "tamago_machine.h"
#include <string.h>

#define BIOS_BANK_SIZE 0x8000u
#define BIOS_FIXED_BASE 0xC000u

/* Tamagotchi Friends: el archivo trae 16 KiB extra al principio. La ROM estatica (ventana fija
   C000-FFFF) empieza en 0x4000, el banco 0 tambien en 0x4000 y el banco n>=1 en (n+1)*32 KiB;
   23 bancos. Tama-Go: ROM estatica en 0, banco n en n*32 KiB, 20 bancos. Esto se guarda fuera de
   TamagoMachine para no cambiar el formato de los save states. */
/* (the per-console state lives in struct Aux below, one per TamagoMachine) */

enum { EEPROM_DISABLED = 0, EEPROM_COMMAND = 1, EEPROM_ADDRESS = 2,
       EEPROM_READ = 3, EEPROM_WRITE = 4 };

static void eeprom_update(TamagoMachine *m, uint8_t power, uint8_t clk, uint8_t data) {
    int clk_d = (int)clk - (int)m->eeprom_last_clk;
    int data_d = (int)data - (int)m->eeprom_last_data;
    m->eeprom_power = power; m->eeprom_last_clk = clk; m->eeprom_last_data = data;
    if (!power) { m->eeprom_state = EEPROM_DISABLED; m->eeprom_out = 1; return; }
    if (!clk_d && !data_d) return;
    if (clk && data_d) {
        if (data_d > 0) { /* STOP */ m->eeprom_state = EEPROM_DISABLED; m->eeprom_out = 0; }
        else { /* START */ m->eeprom_state = EEPROM_COMMAND; m->eeprom_out = 0; m->eeprom_bits = 0; m->eeprom_shift = 0; }
    }
    if (m->eeprom_state == EEPROM_DISABLED) return;
    if (clk_d > 0) {
        m->eeprom_shift = (uint8_t)((m->eeprom_shift << 1) | (data ? 1 : 0));
    } else if (clk_d < 0) {
        if (m->eeprom_bits < 8) {
            if (m->eeprom_state == EEPROM_READ) {
                uint8_t b = m->eeprom[m->eeprom_addr & 0x0FFF];
                m->eeprom_out = (uint8_t)((b >> (7 - m->eeprom_bits)) & 1);
            } else m->eeprom_out = 1;
        } else {
            m->eeprom_out = 0; /* ACK unless command is invalid */
            switch (m->eeprom_state) {
            case EEPROM_COMMAND:
                if ((m->eeprom_shift & 0xF1) == 0xA0) {
                    m->eeprom_state = EEPROM_ADDRESS; m->eeprom_addr_bytes = 0; m->eeprom_addr = 0;
                } else if ((m->eeprom_shift & 0xF1) == 0xA1) {
                    m->eeprom_state = EEPROM_READ;
                } else m->eeprom_out = 1;
                break;
            case EEPROM_ADDRESS:
                m->eeprom_addr = (uint16_t)(((m->eeprom_addr << 8) | m->eeprom_shift) & 0x0FFF);
                if (++m->eeprom_addr_bytes >= 2) m->eeprom_state = EEPROM_WRITE;
                break;
            case EEPROM_WRITE:
                m->eeprom[m->eeprom_addr & 0x0FFF] = m->eeprom_shift; m->eeprom_dirty = true;
                m->eeprom_addr = (uint16_t)((m->eeprom_addr + 1) & 0x0FFF);
                break;
            case EEPROM_READ:
                m->eeprom_addr = (uint16_t)((m->eeprom_addr + 1) & 0x0FFF);
                break;
            default: break;
            }
            m->eeprom_shift = 0;
        }
        m->eeprom_bits = (uint8_t)((m->eeprom_bits + 1) % 9);
    }
}

/* Everything below is state that must exist once PER console but is kept outside TamagoMachine so existing save states
   stay loadable (the machine is saved with a plain memcpy; its size must not change). Each TamagoMachine finds its Aux
   through aux(m); up to TAMAGO_MAX_MACHINES consoles can run in one process (two for a link session). */
typedef struct { uint32_t cycle; int16_t level; } AudioEvent;
#define AUDIO_EVENTS_MAX 8192u
/* T7Emu-style clear tone (timbre 1): once per millisecond of game time the pitch (timer 0 reload) and loudness (envelope
   $93) the firmware is playing are read, and a band-limited square wave is generated from them. */
typedef struct { uint32_t cycle, hz; uint8_t lv; } SynState;
#define SYN_MAX 64u
#define TAMAGO_MAX_MACHINES 3 /* two consoles for a link session + a scratch one for tamago_link_prepare */
typedef struct {
    const TamagoMachine *owner;
    /* ROM layout */
    size_t static_off; unsigned banks; bool friends;
    /* timers */
    uint32_t tm0_acc, tbh2_acc, frame_cycle;
    /* sound */
    AudioEvent audio_ev[AUDIO_EVENTS_MAX];
    size_t audio_n;
    int audio_level;                 /* last DAC level (carried over between frames) */
    float hp_x1, hp_y1;              /* DC-blocking filter state */
    /* "Real" timbre filter state: two low-pass biquads in series (4th order Butterworth, direct form I) */
    float lp1_x1, lp1_x2, lp1_y1, lp1_y2, lp2_x1, lp2_x2, lp2_y1, lp2_y2;
    SynState syn[SYN_MAX];
    size_t syn_n;
    uint32_t syn_acc, syn_hz_cur, syn_ohz;
    uint8_t syn_lv_cur;
    float syn_ph, syn_amp;
    /* Gotchi figure = SPI flash slave */
    const uint8_t *fig_rom; size_t fig_len;
    uint8_t spi_cs; int spi_pos; uint8_t spi_addr_b[3]; uint32_t spi_addr; bool spi_is_read;
    uint8_t spi_rxfifo[256]; uint8_t spi_rxw, spi_rxr;
    const uint8_t *spi_rom; size_t spi_len;
    /* Infrared link: the LED is port B bit 3 ($3016), the receiver is port A bit 7 ($3012; 1 = idle, 0 = carrier). A linked
       console's LED pulses pull this console's receiver line low for IR_HOLD of this console's cycles after each one. */
    TamagoMachine *peer;
    uint64_t clk;            /* cycles this console has run (time base of the link) */
    uint64_t rx_edge;        /* value of clk at the last LED pulse received from the peer */
    uint64_t t0_start;       /* clk when the timer 0 counter was last (re)loaded or (re)started */
    uint32_t t0_base;        /* its value at that moment: the counter ($3032/$3033 read back) counts up from the reload value */
    bool rx_seen;
    /* Tamagotchi Friends link: its IR LED is bit 0 of $303C while timer 0 runs (the carrier), and its sampling / modulation clock is
       timer 1 on source 1, whose underflow is IRQ 10 (see tamago_machine_step). */
    bool led_on;
    uint64_t t1_acc;
} Aux;
#define LINK_T1_HZ (TAMAGO_T0_HZ / 4) /* clock of timer 1 on source 1 (a guess; only the 2:1 ratio of the receive and send rates matters) */
#define IR_HOLD 60u /* cycles the demodulated line stays low after a carrier pulse (the pulses are 27 cycles apart) */
static Aux g_aux[TAMAGO_MAX_MACHINES];
static unsigned g_aux_next;
static const TamagoMachine *g_primary; /* the console whose sound / debug trace are produced: the last one initialised */
static Aux *g_aux_last; /* the console looked up last: with one console running this is nearly always the answer */
static Aux *aux_slow(const TamagoMachine *m) {
    for (unsigned i = 0; i < TAMAGO_MAX_MACHINES; ++i) if (g_aux[i].owner == m) return g_aux_last = &g_aux[i];
    Aux *a = &g_aux[g_aux_next]; /* not registered (should not happen): recycle a slot */
    g_aux_next = (g_aux_next + 1) % TAMAGO_MAX_MACHINES;
    memset(a, 0, sizeof *a); a->owner = m; a->banks = 20; a->spi_cs = 1;
    return g_aux_last = a;
}
static inline Aux *aux(const TamagoMachine *m) {
    Aux *l = g_aux_last;
    return (l && l->owner == m) ? l : aux_slow(m);
}
static size_t bank_off(const Aux *a, unsigned b) {
    size_t o = (size_t)b * BIOS_BANK_SIZE;
    return a->friends ? (b ? o + BIOS_BANK_SIZE : 0x4000u) : o;
}
static bool g_audio_irqs = true;
static uint32_t g_tbh_hz = TAMAGO_TBH_HZ, g_tm0_clock = TAMAGO_T0_HZ; /* see tamago_audio_tune() */
static int g_timbre;
static void audio_filters_reset(Aux *a) {
    a->hp_x1 = a->hp_y1 = 0;
    a->lp1_x1 = a->lp1_x2 = a->lp1_y1 = a->lp1_y2 = a->lp2_x1 = a->lp2_x2 = a->lp2_y1 = a->lp2_y2 = 0;
    a->syn_n = 0; a->syn_acc = 0; a->syn_hz_cur = 0; a->syn_lv_cur = 0; a->syn_ph = 0; a->syn_amp = 0; a->syn_ohz = 0;
}
void tamago_audio_irqs_enable(bool on) { g_audio_irqs = on; }
void tamago_audio_tune(uint32_t tbh_hz, uint32_t tm0_clock_hz) {
    g_tbh_hz = tbh_hz >= 8 && tbh_hz <= 4096 ? tbh_hz : TAMAGO_TBH_HZ;
    g_tm0_clock = tm0_clock_hz >= 100000 && tm0_clock_hz <= 64000000 ? tm0_clock_hz : TAMAGO_T0_HZ;
}
void tamago_audio_timbre(int timbre) { g_timbre = timbre == 1 ? 1 : 0; }

static void audio_dac_write(const TamagoMachine *m) {
    if (m != g_primary) return; /* only the primary console is heard */
    Aux *a = aux(m);
    int level = (int8_t)m->regs[0x60] + (int8_t)m->regs[0x62];
    if (a->audio_n < AUDIO_EVENTS_MAX) a->audio_ev[a->audio_n++] = (AudioEvent){ a->frame_cycle, (int16_t)level };
}

/* The DAC is a staircase that changes at CPU-cycle resolution (4 MHz); the output has 48,000 samples a second. Taking the
   level at one instant per sample (what the first version did) folds everything above 24 kHz back into the audible
   range and makes every edge land on a sample boundary: a rough, jittery tone. Each output sample is instead the
   AVERAGE of the staircase over the time it spans, so an edge inside a sample gives it a fractional level and the
   pitch and timing of the note come out exact. */
static inline float syn_blep(float t, float dt) {
    if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}
static void render_synth(Aux *a, int16_t *out, size_t n, uint32_t frame_cycles) {
    size_t si = 0;
    const float kAmp = 0.0102f; /* T7Emu's 0.03 per sample at 16 kHz, rescaled to 48 kHz (about 0.7 ms) */
    for (size_t i = 0; i < n; ++i) {
        const double t = ((double)i + 0.5) * (double)frame_cycles / (double)n;
        while (si < a->syn_n && (double)a->syn[si].cycle <= t) { a->syn_hz_cur = a->syn[si].hz; a->syn_lv_cur = a->syn[si].lv; ++si; }
        const uint32_t hz = a->syn_hz_cur;
        const float want = hz ? (float)a->syn_lv_cur * 256.0f : 0.0f;
        if (hz) { if (a->syn_ohz == 0 && a->syn_amp < 1.0f) a->syn_ph = 0.0f; a->syn_ohz = hz; }
        a->syn_amp += (want - a->syn_amp) * kAmp;
        float y = 0.0f;
        if (a->syn_ohz && a->syn_amp > 0.5f) {
            float dt = (float)a->syn_ohz / (float)TAMAGO_AUDIO_RATE; if (dt > 0.45f) dt = 0.45f;
            float t2 = a->syn_ph + 0.5f; if (t2 >= 1.0f) t2 -= 1.0f;
            y = ((a->syn_ph < 0.5f ? 1.0f : -1.0f) + syn_blep(a->syn_ph, dt) - syn_blep(t2, dt)) * a->syn_amp;
            a->syn_ph += dt; if (a->syn_ph >= 1.0f) a->syn_ph -= 1.0f;
        } else if (!hz) a->syn_ohz = 0;
        /* what the speaker does to it: low pass (see the measured spectrum of the reference recording) */
        float tt = 0.001175280f * y + 0.002350559f * a->lp1_x1 + 0.001175280f * a->lp1_x2 + 1.328044222f * a->lp1_y1 - 0.453725385f * a->lp1_y2;
        a->lp1_x2 = a->lp1_x1; a->lp1_x1 = y; a->lp1_y2 = a->lp1_y1; a->lp1_y1 = tt;
        float o = tt + 2.0f * a->lp2_x1 + a->lp2_x2 + 1.581005271f * a->lp2_y1 - 0.730625727f * a->lp2_y2;
        a->lp2_x2 = a->lp2_x1; a->lp2_x1 = tt; a->lp2_y2 = a->lp2_y1; a->lp2_y1 = o;
        o *= 0.8f;
        if (o > 32000.0f) o = 32000.0f; else if (o < -32000.0f) o = -32000.0f;
        out[2 * i] = out[2 * i + 1] = (int16_t)o;
    }
    while (si < a->syn_n) { a->syn_hz_cur = a->syn[si].hz; a->syn_lv_cur = a->syn[si].lv; ++si; }
    a->syn_n = 0; a->audio_n = 0;
}

void tamago_audio_render(int16_t *out, size_t n, uint32_t frame_cycles) {
    if (!g_primary) { memset(out, 0, n * 2 * sizeof(int16_t)); return; }
    Aux *a = aux(g_primary);
    if (g_timbre == 1) { render_synth(a, out, n, frame_cycles); return; }
    size_t ei = 0;
    double t0 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        const double t1 = (double)(i + 1) * (double)frame_cycles / (double)n;
        double acc = 0.0, cur = t0;
        while (ei < a->audio_n && (double)a->audio_ev[ei].cycle < t1) {
            const double et = (double)a->audio_ev[ei].cycle > cur ? (double)a->audio_ev[ei].cycle : cur;
            acc += (double)a->audio_level * (et - cur);
            cur = et;
            a->audio_level = a->audio_ev[ei++].level;
        }
        acc += (double)a->audio_level * (t1 - cur);
        float x = (float)(acc / (t1 - t0)) * 96.0f;
        float y = x - a->hp_x1 + 0.995f * a->hp_y1; /* removes the DC offset the DAC can be left at */
        a->hp_x1 = x; a->hp_y1 = y;
        if (y > 32000.0f) y = 32000.0f; else if (y < -32000.0f) y = -32000.0f;
        out[2 * i] = out[2 * i + 1] = (int16_t)y;
        t0 = t1;
    }
    while (ei < a->audio_n) a->audio_level = a->audio_ev[ei++].level;
    a->audio_n = 0;
}

static void fire_irq(TamagoMachine *m, unsigned i) {
    uint16_t mask = (uint16_t)(((uint16_t)m->regs[0x70] << 8) | m->regs[0x71]);
    if (i < 16 && (mask & (uint16_t)(0x8000u >> i))) {
        m->pending_irq |= (uint16_t)(0x8000u >> i);
        m->regs[0x73] = (uint8_t)(m->pending_irq >> 8);
        m->regs[0x74] = (uint8_t)m->pending_irq;
    }
}

/* ---- Gotchi figure = SPI flash slave (protocol rebuilt from the console ROM, as in T7Emu) ----
   CS = port B data ($3016) bit 4, active low. Command via $30B3: 0x03 = flash read + 3 address bytes
   (hi, mid, lo). The first 4 bytes of a read transaction (command + address) come back as 0x00; the
   data is flash[addr], flash[addr+1], ... from byte 4 on. Reads of $30B6 take the byte received while
   sending. No ROM (contacts only / Lite figure): reads return 0xFF, so the 'AA 55' signature check fails.
   Kept outside TamagoMachine so the save state format does not change. */
#ifdef TAMAGO_SPI_STATS /* host tests only */
unsigned long g_spi_stat_cs, g_spi_stat_tx, g_spi_stat_reads, g_spi_stat_sig;
#endif

static void spi_reset(Aux *a) {
    a->spi_cs = 1; a->spi_pos = 0; a->spi_addr = 0; a->spi_is_read = false; a->spi_rxw = a->spi_rxr = 0;
    a->spi_rom = NULL; a->spi_len = 0;
}

static void spi_cs_set(Aux *a, uint8_t level) { /* level: bit 4 of the value written to $3016 */
    uint8_t cs = level ? 1 : 0;
    if (cs == a->spi_cs) return;
    a->spi_cs = cs;
    if (cs == 0) { /* falling edge: new transaction */
        a->spi_pos = 0; a->spi_rxw = a->spi_rxr = 0; a->spi_is_read = false; a->spi_addr = 0;
        a->spi_len = a->fig_len;
        a->spi_rom = a->spi_len ? a->fig_rom : NULL;
#ifdef TAMAGO_SPI_STATS
        ++g_spi_stat_cs;
#endif
    }
}

static void spi_tx(Aux *a, uint8_t v) { /* write to $30B3 = one transfer */
    if (a->spi_cs) return;
    int p = a->spi_pos;
    uint8_t rx;
    if (p == 0) a->spi_is_read = (v == 0x03);
    if (p >= 1 && p <= 3) a->spi_addr_b[p - 1] = v;
    if (p == 3 && a->spi_is_read)
        a->spi_addr = ((uint32_t)a->spi_addr_b[0] << 16) | ((uint32_t)a->spi_addr_b[1] << 8) | a->spi_addr_b[2];
    if (a->spi_is_read) {
        if (p < 4) rx = 0x00;
        else if (a->spi_len) { size_t ad = (size_t)a->spi_addr + (size_t)(p - 4); rx = (ad < a->spi_len) ? a->spi_rom[ad] : 0x00; }
        else rx = 0xFF;
#ifdef TAMAGO_SPI_STATS
        if (p >= 4) { ++g_spi_stat_reads; if (a->spi_addr == 1 && p == 5 && a->spi_len >= 3 && a->spi_rom[1] == 0xAA && a->spi_rom[2] == 0x55) ++g_spi_stat_sig; }
#endif
    } else rx = v; /* loopback / echo */
    a->spi_rxfifo[a->spi_rxw++] = rx;
    ++a->spi_pos;
#ifdef TAMAGO_SPI_STATS
    ++g_spi_stat_tx;
#endif
}

static uint8_t spi_rx(Aux *a) { /* read of $30B6 */
    if (a->spi_rxr != a->spi_rxw) return a->spi_rxfifo[a->spi_rxr++];
    return 0xFF;
}

void tamago_machine_set_figure(TamagoMachine *m, uint8_t contacts, const uint8_t *rom, size_t len) {
    if (!m) return;
    Aux *a = aux(m);
    m->figure = (uint8_t)(contacts & 3);
    a->fig_rom = rom; a->fig_len = rom ? len : 0;
    spi_reset(a);
}

void tamago_machine_set_primary(const TamagoMachine *m) { g_primary = m; }
void tamago_link_connect(TamagoMachine *a, TamagoMachine *b) {
    if (!a || !b || a == b) return;
    Aux *x = aux(a), *y = aux(b);
    x->peer = b; y->peer = a; x->rx_seen = y->rx_seen = false;
}
/* ---- Link timing thresholds ---------------------------------------------------------------------------------
   The firmware calibrates its infrared pulse timing ONCE, when it powers on (routine at $7580, about 3.5 s in), and keeps the
   result in zero page $7C..$7F. A state saved by an older build of this emulator (before the timer 0 counter and the
   decimal-mode fixes) carries zeros there and the calibration is not repeated when it is loaded, so a link with it silently
   fails (both consoles wait in STAND BY). tamago_link_prepare fills the four bytes from a console that has them or, if none
   does, by booting a scratch console just far enough for the firmware to calibrate. */
static bool link_th_ok(const TamagoMachine *m) { return (m->wram[0x7C] | m->wram[0x7D] | m->wram[0x7E] | m->wram[0x7F]) != 0; }
void tamago_machine_release(const TamagoMachine *m) {
    for (unsigned i = 0; i < TAMAGO_MAX_MACHINES; ++i) if (g_aux[i].owner == m) g_aux[i].owner = NULL;
    if (g_aux_last && g_aux_last->owner == NULL) g_aux_last = NULL;
    if (g_primary == m) g_primary = NULL;
}
bool tamago_link_prepare(TamagoMachine *a, TamagoMachine *b) {
    if (aux(a)->friends) return true; /* the Friends link has no calibration: it is timed by timer 1 interrupts */
    uint8_t th[4];
    if (link_th_ok(a)) memcpy(th, &a->wram[0x7C], 4);
    else if (link_th_ok(b)) memcpy(th, &b->wram[0x7C], 4);
    else {
        static TamagoMachine scratch;
        const TamagoMachine *prev = g_primary;
        tamago_machine_init(&scratch, a->bios, a->bios_size);
        bool got = false;
        for (int i = 0; i < 600 && !got; ++i) {
            scratch.keys = 0x0F;
            tamago_machine_frame(&scratch);
            if (tamago_machine_run(&scratch, TAMAGO_CLOCK_HZ / 60) != 0) break;
            got = link_th_ok(&scratch);
        }
        if (got) memcpy(th, &scratch.wram[0x7C], 4);
        tamago_machine_release(&scratch);
        g_primary = prev;
        if (!got) return false;
    }
    if (!link_th_ok(a)) memcpy(&a->wram[0x7C], th, 4);
    if (!link_th_ok(b)) memcpy(&b->wram[0x7C], th, 4);
    return true;
}

void tamago_link_disconnect(TamagoMachine *m) {
    if (!m) return;
    Aux *x = aux(m);
    if (x->peer) { Aux *y = aux(x->peer); y->peer = NULL; y->rx_seen = false; }
    x->peer = NULL; x->rx_seen = false;
}
bool tamago_link_connected(const TamagoMachine *m) { return m && aux(m)->peer != NULL; }
int tamago_link_run(TamagoMachine *a, TamagoMachine *b, uint32_t cycle_budget) {
    uint64_t da = 0, db = 0;
    while (da < cycle_budget || db < cycle_budget) { /* always step the console that is behind, so the pulses line up */
        TamagoMachine *m = (da <= db && da < cycle_budget) || db >= cycle_budget ? a : b;
        uint64_t *d = m == a ? &da : &db;
        int r = tamago_machine_step(m);
        if (r != 0) return m == a ? r : -2;
        *d += m->last_cycles;
    }
    return 0;
}
bool tamago_machine_is_friends_of(const TamagoMachine *m) { return m && aux(m)->friends; }
bool tamago_machine_is_friends(void) { return tamago_machine_is_friends_of(g_primary); }

static uint8_t rom_read(const TamagoMachine *m, uint16_t addr) {
    const Aux *a = aux(m);
    size_t off;
    if (addr >= 0x4000 && addr < 0xC000) {
        off = bank_off(a, m->bank % a->banks) + (size_t)(addr - 0x4000);
    } else {
        off = a->static_off + (size_t)(addr - BIOS_FIXED_BASE);
    }
    return (m->bios && off < m->bios_size) ? m->bios[off] : 0xFF;
}

static uint8_t irq_vector_read(const TamagoMachine *m, bool high) {
    unsigned idx = 15; /* upstream: no pending IRQ selects vector 15 (e.g. for BRK) */
    uint16_t pending = m->pending_irq;
    if (pending) {
        for (unsigned i = 0; i < 16; ++i) {
            if (pending & (uint16_t)(0x8000u >> i)) { idx = i; break; }
        }
    }
    uint16_t v = m->irq_vectors[idx];
    return high ? (uint8_t)(v >> 8) : (uint8_t)v;
}

#ifdef TRACE_ALL
unsigned long g_reg_reads[256], g_reg_writes[256]; /* host experiments only */
#endif
/* Timer 0 counter, computed on demand (only the link firmware reads it): runs at TAMAGO_T0_HZ (10.667 MHz) while the CPU
   runs at 4 MHz, counts up from the reload value and reloads on overflow. */
static uint32_t t0_now(const TamagoMachine *m, const Aux *a) {
    if ((m->regs[0x31] >> 4) != 2) return a->t0_base;
    const uint32_t reload = ((uint32_t)m->regs[0x33] << 8) | m->regs[0x32];
    uint64_t v = a->t0_base + (uint64_t)((double)(a->clk - a->t0_start) * (double)TAMAGO_T0_HZ / (double)TAMAGO_CLOCK_HZ);
    if (v >= 0x10000u) v = reload + (v - 0x10000u) % (0x10000u - reload);
    return (uint32_t)v;
}
static uint8_t read_raw(void *ctx, uint16_t addr) {
    TamagoMachine *m = (TamagoMachine *)ctx;
#ifdef TRACE_ALL
    if (addr >= 0x3000 && addr < 0x4000) g_reg_reads[addr & 0xFF]++;
#endif
    if (addr < 0x1000) return m->wram[addr % sizeof(m->wram)];
    if (addr < 0x3000) return m->dram[(addr - 0x1000) % sizeof(m->dram)];
    if (addr < 0x4000) {
        uint8_t r = (uint8_t)addr;
        if (r == 0x12) {
            uint8_t mask = m->regs[0x11], data = m->regs[0x12];
            /* Upstream: keys in bits 0-3; inserted figure (CD1/CD2) in bits 5-6 while SPI power is off. */
            uint8_t spi_power = (uint8_t)(mask & data & 0x10);
            const Aux *a = aux(m);
            const uint8_t rx = (a->peer && ((a->rx_seen && a->clk - a->rx_edge <= IR_HOLD) || aux(a->peer)->led_on)) ? 0 : 1; /* IR receiver: idle high, low while the peer's carrier is on */
            uint8_t input = (uint8_t)((m->keys & 0x0F) | ((spi_power ? 0 : m->figure) << 5) | (rx << 7));
            return (uint8_t)((mask & data) | ((uint8_t)~mask & input));
        }
        if (r == 0x16) {
            uint8_t mask = m->regs[0x15];
            return (uint8_t)((mask & m->regs[0x16]) | ((uint8_t)~mask & (m->eeprom_out ? 1 : 0)));
        }
        if (r == 0xB6) return spi_rx(aux(m)); /* figure ROM data */
        if (r == 0x32) return (uint8_t)t0_now(m, aux(m)); /* timer 0 counter: the link firmware calibrates its pulse timing with it */
        if (r == 0x33) return (uint8_t)(t0_now(m, aux(m)) >> 8);
        if (r == 0xB7) return 0xFF; /* upstream: P_SPI_Status always reads 0xFF */
        return m->regs[r];
    }
    if (addr == 0xFFFE) return irq_vector_read(m, false);
    if (addr == 0xFFFF) return irq_vector_read(m, true);
    return rom_read(m, addr);
}

#define TRACE_MAX 32768u
static TamagoTraceEntry g_trace[TRACE_MAX];
static size_t g_trace_n;
static bool g_trace_on;
static uint32_t g_trace_frame, g_trace_cycle;
static uint8_t g_trace_last_keys = 0x0F;
/* Extended trace (link / infrared research): every $30xx register, reads as well as writes. To keep the file readable a
   write or read is only logged when the value differs from the last one logged for that register, and each register
   gets at most TRACE_BUDGET entries per frame. */
#define TRACE_BUDGET 128u
static bool g_trace_ext;
static int16_t g_trace_lastw[256], g_trace_lastr[256];
static uint16_t g_trace_budget[256];
static void trace_reset_last(void) {
    for (int i = 0; i < 256; ++i) { g_trace_lastw[i] = -1; g_trace_lastr[i] = -1; g_trace_budget[i] = 0; }
}

static void trace_add(uint16_t addr, uint8_t value, uint8_t type, uint16_t pc) {
    if (g_trace_n >= TRACE_MAX) return; /* the frontend flushes regularly; drop when full */
    g_trace[g_trace_n++] = (TamagoTraceEntry){ g_trace_frame, g_trace_cycle, addr, pc, value, type };
}
void tamago_trace_enable(bool on) { g_trace_on = on; g_trace_last_keys = 0x0F; trace_reset_last(); }
void tamago_trace_extended(bool on) { g_trace_ext = on; trace_reset_last(); }
bool tamago_trace_is_extended(void) { return g_trace_ext; }
bool tamago_trace_enabled(void) { return g_trace_on; }
void tamago_trace_keys(uint8_t keys) {
    if (g_trace_on && keys != g_trace_last_keys) trace_add(0, keys, 1, 0);
    g_trace_last_keys = keys;
}
size_t tamago_trace_count(void) { return g_trace_n; }
const TamagoTraceEntry *tamago_trace_data(void) { return g_trace; }
void tamago_trace_clear(void) { g_trace_n = 0; }
/* Every $30xx register except the very noisy ones: bank select ($00), watchdog ($04),
   EEPROM bit-banging ($14-$16) and the pending-IRQ housekeeping ($73-$75). */
static bool trace_wanted(uint16_t r) {
    if (g_trace_ext) return !(r == 0x00 || r == 0x04 || (r >= 0x73 && r <= 0x75)); /* EEPROM / port B included */
#ifdef TRACE_ALL
    (void)r; return true; /* host experiments: log everything */
#else
    return !(r == 0x00 || r == 0x04 || (r >= 0x14 && r <= 0x16) || (r >= 0x73 && r <= 0x75));
#endif
}

uint8_t tamago_machine_read(void *ctx, uint16_t addr) {
    const uint8_t v = read_raw(ctx, addr);
    if (g_trace_on && g_trace_ext && (const TamagoMachine *)ctx == g_primary && addr >= 0x3000 && addr < 0x4000 && trace_wanted(addr & 0xFF)) {
        const uint8_t tr = (uint8_t)addr;
        if (g_trace_lastr[tr] != v && g_trace_budget[tr] < TRACE_BUDGET) {
            g_trace_lastr[tr] = v; ++g_trace_budget[tr];
            trace_add(addr, v, 2, ((const TamagoMachine *)ctx)->cpu.pc);
        }
    }
    return v;
}
void tamago_machine_write(void *ctx, uint16_t addr, uint8_t value) {
    TamagoMachine *m = (TamagoMachine *)ctx;
    Aux *a = aux(m);
#ifdef TRACE_ALL
    if (addr >= 0x3000 && addr < 0x4000) g_reg_writes[addr & 0xFF]++;
#endif
    if (g_trace_on && m == g_primary && addr >= 0x3000 && addr < 0x4000 && trace_wanted(addr & 0xFF)) {
        const uint8_t tr = (uint8_t)addr;
        if (!g_trace_ext) trace_add(addr, value, 0, m->cpu.pc);
        else if (g_trace_lastw[tr] != value && g_trace_budget[tr] < TRACE_BUDGET) {
            g_trace_lastw[tr] = value; ++g_trace_budget[tr];
            trace_add(addr, value, 0, m->cpu.pc);
        }
    }
    if (addr < 0x1000) { m->wram[addr % sizeof(m->wram)] = value; return; }
    if (addr < 0x3000) { m->dram[(addr - 0x1000) % sizeof(m->dram)] = value; return; }
    if (addr < 0x4000) {
        uint8_t r = (uint8_t)addr;
        if (r == 0x00) { m->bank = (uint8_t)(value % a->banks); m->regs[r] = value; return; }
        if (r == 0x73 || r == 0x74) { m->pending_irq &= (uint16_t)~((uint16_t)value << (r == 0x73 ? 8 : 0)); m->regs[r] &= (uint8_t)~value; return; }
        const uint8_t old_value = m->regs[r];
        if (r == 0x31) { a->t0_base = t0_now(m, a); a->t0_start = a->clk; } /* latch the counter before its clock changes */
        m->regs[r] = value;
        if (r == 0x16 && a->peer && (value & ~old_value & 0x08) && (m->regs[0x15] & 0x08)) { /* IR LED switched on */
            Aux *pa = aux(a->peer);
            pa->rx_edge = pa->clk; pa->rx_seen = true;
        }
        if (r == 0x3C || r == 0x31) a->led_on = (m->regs[0x3C] & 1) && (m->regs[0x31] >> 4) != 0; /* Friends: carrier on */
        if (r == 0x32 || r == 0x33) { a->t0_base = ((uint32_t)m->regs[0x33] << 8) | m->regs[0x32]; a->t0_start = a->clk; } /* reload */
        if (r == 0x60 || r == 0x62) audio_dac_write(m); /* DAC output */
        if (r == 0xB3) spi_tx(a, value);              /* figure SPI: command / address / dummy bytes */
        if (r == 0x16) spi_cs_set(a, value & 0x10);  /* figure SPI chip select (active low) */
        if (r == 0x16 || r == 0x15) {
            uint8_t mask = m->regs[0x15];
            uint8_t data = (uint8_t)(~mask | m->regs[0x16]);
            m->eeprom_power = (data >> 2) & 1;
            m->eeprom_clk = (data >> 1) & 1;
            m->eeprom_data = data & 1;
            eeprom_update(m, (uint8_t)((data >> 2) & 1),
                          (uint8_t)((data >> 1) & 1), (uint8_t)(data & 1));
        }
    }
}

static Aux *aux_register(const TamagoMachine *m) {
    Aux *a = NULL;
    for (unsigned i = 0; i < TAMAGO_MAX_MACHINES && !a; ++i) if (g_aux[i].owner == m) a = &g_aux[i];
    for (unsigned i = 0; i < TAMAGO_MAX_MACHINES && !a; ++i) if (!g_aux[i].owner) a = &g_aux[i];
    if (!a) { a = &g_aux[g_aux_next]; g_aux_next = (g_aux_next + 1) % TAMAGO_MAX_MACHINES; } /* all taken: recycle */
    memset(a, 0, sizeof *a);
    a->owner = m; a->banks = 20; a->spi_cs = 1;
    return a;
}

void tamago_machine_init(TamagoMachine *m, const uint8_t *bios, size_t bios_size) {
    memset(m, 0, sizeof(*m));
    Aux *a = aux_register(m);
    g_primary = m;
    m->bios = bios;
    m->bios_size = bios_size;
    /* Friends: vector de reset a 0x7FFC (en Tama-Go esta a 0x3FFC, aqui vacio). */
    a->static_off = 0; a->banks = 20; a->friends = false;
    a->fig_rom = NULL; a->fig_len = 0; spi_reset(a);
    if (bios && bios_size >= 0x8000u && !(bios[0x3FFC] | bios[0x3FFD]) && (bios[0x7FFC] | bios[0x7FFD])) {
        a->friends = true; a->static_off = 0x4000u; a->banks = 23;
    }
    m->bios_valid = bios && bios_size >= (size_t)(a->friends ? (a->banks + 1u) : a->banks) * BIOS_BANK_SIZE;
    m->keys = 0x0F;
    a->tm0_acc = a->tbh2_acc = a->frame_cycle = 0; a->audio_n = 0; a->audio_level = 0; audio_filters_reset(a);
    memset(m->eeprom, 0, sizeof(m->eeprom));
    m->eeprom_out = 1;
    m->eeprom_last_clk = 0;
    m->eeprom_last_data = 0;
    if (bios && bios_size >= 0x4000) {
        for (unsigned i = 0; i < 16; ++i) {
            size_t off = a->static_off + 0x3FC0u + (size_t)i * 2u;
            if (off + 1 < bios_size)
                m->irq_vectors[i] = (uint16_t)(bios[off] | ((uint16_t)bios[off + 1] << 8));
        }
    }
    tamago_cpu_init(&m->cpu, m, tamago_machine_read, tamago_machine_write);
    tamago_machine_reset(m);
}

void tamago_machine_reset(TamagoMachine *m) {
    m->bank = 0;
    m->pending_irq = 0;
    tamago_cpu_reset(&m->cpu);
}

/* What tone is the firmware playing right now (T7Emu t7_snd_tone_hz): the square wave only goes out while $92 bit 0 is set and
   ($B3 bit 0 or none of the $94 bits in $97), pitch = half the timer 0 rate, loudness = |$93|. */
static void synth_sample(const TamagoMachine *m) {
    if (m != g_primary) return;
    Aux *a = aux(m);
    uint32_t hz = 0; uint8_t lv = 0;
    if (a->syn_n < SYN_MAX && (m->regs[0x31] >> 4) == 2 && (m->regs[0x70] & 0x80) && (m->regs[0x65] & 0x02) && (m->wram[0x92] & 0x01) &&
        ((m->wram[0xB3] & 0x01) || !(m->wram[0x97] & 0x94))) {
        int a = (int8_t)m->wram[0x93]; if (a == -128) a = 0; if (a < 0) a = -a;
        const uint32_t cnt = 0x10000u - (uint32_t)(((uint32_t)m->regs[0x33] << 8) | m->regs[0x32]);
        if (a && cnt >= 16) { lv = (uint8_t)a; hz = (g_tm0_clock / 2u + cnt / 2u) / cnt; }
        /* The buzzer only plays notes: timer 0 also paces other things (the link / infrared carrier, sample playback), which
           would come out as a screech or a buzz. Anything outside the range of the note table is silence. */
        if (hz < 60u || hz > 8000u) { hz = 0; lv = 0; }
    }
    if (a->syn_n < SYN_MAX) a->syn[a->syn_n++] = (SynState){ a->frame_cycle, hz, lv };
}

int tamago_machine_step(TamagoMachine *m) {
    if (!m || !m->bios_valid) return -1;
    Aux *a = aux(m);
    /* The CPU counts cycles downward (negative elapsed). Reset it every step so the
       int32 counter can never overflow (it wrapped after ~80 minutes before). */
    m->cpu.cycles = 0;
    /* The upstream core services an enabled pending IRQ before fetching the next opcode. */
    if (!m->cpu.i && m->pending_irq) (void)tamago_cpu_irq(&m->cpu);
    int result = tamago_cpu_step(&m->cpu);
    if (result != 0) return result;
    uint64_t elapsed = (uint64_t)(-(int64_t)m->cpu.cycles);
    m->last_cycles = (uint32_t)elapsed;
    a->clk += elapsed;
    if (m == g_primary) g_trace_cycle += (uint32_t)elapsed;
    a->frame_cycle += (uint32_t)elapsed;
    if (g_audio_irqs) {
        /* TBH time base: clocks the note sequencer (handler at IRQ12). */
        /* Timer 1 as T7Emu has it: source nibble ($3031 low) 2, clock 32768 Hz (16 times that with $3030 bit 0), period
           0x10000 - reload ($3034/$3035): the firmware's $FC00 gives 512 Hz. g_tbh_hz / 128 scales it (tempo menu). */
        a->tbh2_acc += (uint32_t)elapsed;
        if ((m->regs[0x31] & 0x0F) == 2) {
            uint32_t n1 = 0x10000u - (uint32_t)(((uint32_t)m->regs[0x35] << 8) | m->regs[0x34]);
            if (n1 < 16) n1 = 16;
            const uint64_t t1hz = (m->regs[0x30] & 1) ? 524288u : 32768u;
            const uint64_t period = (uint64_t)TAMAGO_CLOCK_HZ * n1 * 128u / (t1hz * g_tbh_hz);
            while (period && a->tbh2_acc >= period) { a->tbh2_acc -= (uint32_t)period; fire_irq(m, 12); if (m->regs[0x76] & 0x02) tamago_cpu_nmi(&m->cpu); /* NMI source 6 follows timer 1 */ }
        } else a->tbh2_acc = 0;
        a->syn_acc += (uint32_t)elapsed;
        while (a->syn_acc >= TAMAGO_CLOCK_HZ / 1000u) { a->syn_acc -= TAMAGO_CLOCK_HZ / 1000u; synth_sample(m); }
        /* TM0 (IRQ0) only runs while its interrupt is enabled; period = 0x10000 - reload value. */
        if (m->regs[0x70] & 0x80) {
            uint32_t period = 0x10000u - (uint32_t)(((uint32_t)m->regs[0x33] << 8) | m->regs[0x32]);
            if (g_tm0_clock != TAMAGO_CLOCK_HZ) period = (uint32_t)((uint64_t)period * TAMAGO_CLOCK_HZ / g_tm0_clock); /* tuned timer clock */
            if (period < 64) period = 64; /* guards against interrupt storms on bogus reload values */
            a->tm0_acc += (uint32_t)elapsed;
            while (a->tm0_acc >= period) { a->tm0_acc -= period; fire_irq(m, 0); }
        } else a->tm0_acc = 0;
    }
    if (a->friends && (m->regs[0x31] & 0x0F) == 1) { /* Friends link: timer 1 underflow = IRQ 10 at the programmed rate */
        a->t1_acc += (uint32_t)elapsed;
        uint32_t n1 = 0x10000u - (uint32_t)(((uint32_t)m->regs[0x35] << 8) | m->regs[0x34]);
        if (n1 < 16) n1 = 16;
        const uint64_t period = (uint64_t)n1 * TAMAGO_CLOCK_HZ / LINK_T1_HZ;
        while (a->t1_acc >= period) { a->t1_acc -= period; fire_irq(m, 10); }
    } else a->t1_acc = 0;
    m->timer_cycles += elapsed;
    m->tbh_cycles += elapsed;
    while (m->tbh_cycles >= 2000000u) {
        fire_irq(m, 13);
        m->tbh_cycles -= 2000000u;
    }
    return 0;
}

int tamago_machine_run(TamagoMachine *m, uint32_t cycle_budget) {
    uint64_t done = 0;
    while (done < cycle_budget) {
        int r = tamago_machine_step(m);
        if (r != 0) return r;
        done += m->last_cycles;
    }
    return 0;
}

void tamago_machine_frame(TamagoMachine *m) {
    if (!m) return;
    Aux *a = aux(m);
    if (m == g_primary) { ++g_trace_frame; g_trace_cycle = 0; if (g_trace_ext) memset(g_trace_budget, 0, sizeof g_trace_budget); }
    a->frame_cycle = 0;
    /* Upstream currently models one frame interrupt per display frame. */
    if (!(a->friends && (m->regs[0x31] & 0x0F) == 1)) fire_irq(m, 10); /* (not while the Friends link runs timer 1 itself) */
    /* The upstream implementation enters NMI immediately when source 6 is enabled. */
    if ((m->regs[0x76] & (uint8_t)(0x80u >> 6)) && (m->regs[0x31] & 0x0F) != 2) tamago_cpu_nmi(&m->cpu);
    /* Friends: su tick viene de la IRQ 3, cada 122a (contador en WRAM $55): se pone justo debajo
       y se lanza una vez por fotograma, igual que T7Emu. */
    if (a->friends) {
        if (m->wram[0x55] < 0x79) m->wram[0x55] = 0x79;
        fire_irq(m, 3);
    }
}

void tamago_machine_tick(TamagoMachine *m, uint32_t cycles) {
    if (!m) return;
    Aux *a = aux(m);
    tamago_machine_frame(m);
    fire_irq(m, 13); /* the 2 Hz time base that drives the game's clock */
    if (!m->cpu.halted) (void)tamago_machine_run(m, cycles);
    a->audio_n = 0; a->syn_n = 0; /* whatever sound the game tried to make is dropped */
}

#define STATE_MAGIC 0x4F474D54u /* "TMGO" */
#define STATE_VERSION 2u /* v2 appends the TM0/TBH timer phases; v1 states (without them) still load */
#define STATE_EXTRA (2 * sizeof(uint32_t))
typedef struct { uint32_t magic, version, struct_size, bios_hash; } StateHeader;

static uint32_t bios_hash(const TamagoMachine *m) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; m->bios && i < m->bios_size; i += 61) { h ^= m->bios[i]; h *= 16777619u; }
    return h ^ (uint32_t)m->bios_size;
}

size_t tamago_machine_state_size(void) { return sizeof(StateHeader) + sizeof(TamagoMachine) + STATE_EXTRA; }

void tamago_machine_serialize(const TamagoMachine *m, uint8_t *out) {
    StateHeader h = { STATE_MAGIC, STATE_VERSION, (uint32_t)sizeof(TamagoMachine), bios_hash(m) };
    const Aux *a = aux(m);
    uint32_t extra[2] = { a->tm0_acc, a->tbh2_acc };
    TamagoMachine copy = *m;
    copy.bios = NULL; copy.bios_size = 0;
    copy.cpu.bus_ctx = NULL; copy.cpu.read = NULL; copy.cpu.write = NULL;
    memcpy(out, &h, sizeof h);
    memcpy(out + sizeof h, &copy, sizeof copy);
    memcpy(out + sizeof h + sizeof copy, extra, sizeof extra);
}

bool tamago_machine_deserialize(TamagoMachine *m, const uint8_t *in, size_t size) {
    StateHeader h;
    const size_t base = sizeof(StateHeader) + sizeof(TamagoMachine);
    if (size < sizeof h) return false;
    memcpy(&h, in, sizeof h);
    bool v2 = size == base + STATE_EXTRA && h.version == STATE_VERSION;
    bool v1 = size == base && h.version == 1u;
    if (h.magic != STATE_MAGIC || !(v1 || v2) ||
        h.struct_size != sizeof(TamagoMachine) || h.bios_hash != bios_hash(m)) return false;
    Aux *a = aux(m);
    TamagoMachine tmp;
    memcpy(&tmp, in + sizeof h, sizeof tmp);
    tmp.bios = m->bios; tmp.bios_size = m->bios_size;
    tmp.cpu.bus_ctx = m->cpu.bus_ctx; tmp.cpu.read = m->cpu.read; tmp.cpu.write = m->cpu.write;
    tmp.bios_valid = m->bios_valid; tmp.eeprom_dirty = false;
    *m = tmp;
    if (v2) { uint32_t extra[2]; memcpy(extra, in + base, sizeof extra); a->tm0_acc = extra[0]; a->tbh2_acc = extra[1]; }
    else a->tm0_acc = a->tbh2_acc = 0;
    a->audio_n = 0; audio_filters_reset(a); a->audio_level = 0;
    return true;
}

void tamago_machine_load_eeprom(TamagoMachine *m, const uint8_t *data, size_t size) {
    if (!m || !data) return;
    if (size > sizeof(m->eeprom)) size = sizeof(m->eeprom);
    memcpy(m->eeprom, data, size);
}
