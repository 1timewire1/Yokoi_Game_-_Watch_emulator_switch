// Sharp SM8500 CPU core (the CPU inside the Game.com's SM8521), from MAME
// src/devices/cpu/sm8500 (license:BSD-3-Clause, copyright-holders:Wilbert Pol),
// taken out of MAME's device framework: the machine supplies read/write and
// the DMA/timer hooks. sm85ops.h is MAME's opcode file, unchanged.
#pragma once
#include <cstdint>
#include <functional>

class sm8500_cpu {
public:
    static constexpr int ILL_INT = 0, DMA_INT = 1, TIM0_INT = 2, EXT_INT = 3, UART_INT = 4, LCDC_INT = 5,
                         TIM1_INT = 6, CK_INT = 7, PIO_INT = 8, WDT_INT = 9, NMI_INT = 10;

    std::function<uint8_t(uint16_t)> read;
    std::function<void(uint16_t, uint8_t)> write;
    std::function<void(int)> dma_cb;    // called with the cycles of a halted step
    std::function<void(int)> timer_cb;  // called with the cycles of every step

    void reset();
    int run(int cycles);                // returns the cycles used
    void set_input_line(int line, bool assert);
    uint16_t pc() const { return m_PC; }

    // state (public for save states)
    uint16_t m_PC = 0;
    uint8_t m_IE0 = 0, m_IE1 = 0, m_IR0 = 0, m_IR1 = 0, m_SYS = 0, m_CKC = 0, m_clock_changed = 0;
    uint16_t m_SP = 0;
    uint8_t m_PS0 = 0, m_PS1 = 0;
    uint16_t m_IFLAGS = 0;
    uint8_t m_CheckInterrupts = 0;
    int32_t m_halted = 0;
    int32_t m_icount = 0;
    uint16_t m_oldpc = 0;
    uint8_t m_register_ram[0x108] = {};

private:
    static constexpr uint8_t FLAG_C = 0x80, FLAG_Z = 0x40, FLAG_S = 0x20, FLAG_V = 0x10,
                             FLAG_D = 0x08, FLAG_H = 0x04, FLAG_B = 0x02, FLAG_I = 0x01;
    struct prog_t {
        sm8500_cpu* c;
        uint8_t read_byte(uint32_t a) { return c->read((uint16_t)a); }
        void write_byte(uint32_t a, uint8_t v) { c->write((uint16_t)a, v); }
    } m_program_s{this};
    prog_t* m_program = &m_program_s;

    void get_sp();
    uint8_t mem_readbyte(uint32_t offset);
    void mem_writebyte(uint32_t offset, uint8_t data);
    uint16_t mem_readword(uint32_t a) { return (uint16_t)((mem_readbyte(a) << 8) | mem_readbyte(a + 1)); }
    void mem_writeword(uint32_t a, uint16_t v) { mem_writebyte(a, (uint8_t)(v >> 8)); mem_writebyte(a + 1, (uint8_t)v); }
    void take_interrupt(uint16_t vector);
    void process_interrupts();
};
