#ifndef TAMAGO_CPU6502_H
#define TAMAGO_CPU6502_H
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t (*tamago_read_fn)(void *ctx, uint16_t addr);
typedef void (*tamago_write_fn)(void *ctx, uint16_t addr, uint8_t value);
typedef struct {
    uint8_t a, x, y, s, c, z, i, d, v, n;
    uint16_t pc;
    int32_t cycles;
    bool halted;
    void *bus_ctx;
    tamago_read_fn read;
    tamago_write_fn write;
} TamagoCpu;

void tamago_cpu_init(TamagoCpu *cpu, void *ctx, tamago_read_fn read_fn, tamago_write_fn write_fn);
void tamago_cpu_reset(TamagoCpu *cpu);
int tamago_cpu_step(TamagoCpu *cpu); /* returns 0 on success, -1 on invalid opcode */
/* Enter hardware interrupts. IRQ is ignored while I is set; NMI is edge-triggered by caller. */
bool tamago_cpu_irq(TamagoCpu *cpu);
void tamago_cpu_nmi(TamagoCpu *cpu);
uint8_t tamago_cpu_status(const TamagoCpu *cpu);
void tamago_cpu_set_status(TamagoCpu *cpu, uint8_t status);
#endif
