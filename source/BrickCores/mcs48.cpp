// Intel MCS-48 (8048) CPU core, from MAME src/devices/cpu/mcs48 (license:
// BSD-3-Clause, Dan Boris, Mirko Buffoni, Aaron Giles, Chris Kirmse...),
// taken out of MAME's device framework: the machine supplies the program
// read, external data (MOVX), ports, test pins. Opcode handlers unchanged.
#include "mcs48.h"

#define TIMER_ENABLED   0x01
#define COUNTER_ENABLED 0x02
#define C_FLAG          0x80
#define A_FLAG          0x40
#define F_FLAG          0x20
#define B_FLAG          0x10
#define STS_IBF         0x02
#define STS_OBF         0x01
#define R0 m_regptr[0]
#define R1 m_regptr[1]
#define R2 m_regptr[2]
#define R3 m_regptr[3]
#define R4 m_regptr[4]
#define R5 m_regptr[5]
#define R6 m_regptr[6]
#define R7 m_regptr[7]
namespace { inline void logerror(const char*, ...) {} }
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t offs_t;

u8 mcs48_cpu::opcode_fetch()
{
	u16 address = m_pc;
	m_pc = ((m_pc + 1) & 0x7ff) | (m_pc & 0x800);
	return program_r(address);
}


/*-------------------------------------------------
    argument_fetch - fetch an opcode argument
    byte
-------------------------------------------------*/

u8 mcs48_cpu::argument_fetch()
{
	u16 address = m_pc;
	m_pc = ((m_pc + 1) & 0x7ff) | (m_pc & 0x800);
	return program_r(address);
}


/*-------------------------------------------------
    update_regptr - update the regptr member to
    point to the appropriate register bank
-------------------------------------------------*/

void mcs48_cpu::update_regptr()
{
	m_regptr = &m_dataptr[(m_psw & B_FLAG) ? 24 : 0];
}


/*-------------------------------------------------
    update_ea - apply EA pin (External Access input,
    EA=1: external ROM, EA=0: internal ROM)
-------------------------------------------------*/



/*-------------------------------------------------
    push_pc_psw - push the PC and PSW values onto
    the stack
-------------------------------------------------*/

void mcs48_cpu::push_pc_psw()
{
	u8 sp = m_psw & 0x07;
	ram_w(8 + 2*sp, m_pc);
	ram_w(9 + 2*sp, ((m_pc >> 8) & 0x0f) | (m_psw & 0xf0));
	m_psw = (m_psw & 0xf0) | ((sp + 1) & 0x07);
}


/*-------------------------------------------------
    pull_pc_psw - pull the PC and PSW values from
    the stack
-------------------------------------------------*/

void mcs48_cpu::pull_pc_psw()
{
	u8 sp = (m_psw - 1) & 0x07;
	m_pc = ram_r(8 + 2*sp);
	m_pc |= ram_r(9 + 2*sp) << 8;
	m_psw = ((m_pc >> 8) & 0xf0) | sp;
	m_pc &= (m_irq_in_progress) ? 0x7ff : 0xfff;
	update_regptr();
}


/*-------------------------------------------------
    pull_pc - pull the PC value from the stack,
    leaving the upper part of PSW intact
-------------------------------------------------*/

void mcs48_cpu::pull_pc()
{
	u8 sp = (m_psw - 1) & 0x07;
	m_pc = ram_r(8 + 2*sp);
	m_pc |= ram_r(9 + 2*sp) << 8;
	m_pc &= (m_irq_in_progress) ? 0x7ff : 0xfff;
	m_psw = (m_psw & 0xf0) | sp;
}


/*-------------------------------------------------
    execute_add - perform the logic of an ADD
    instruction
-------------------------------------------------*/

void mcs48_cpu::execute_add(u8 dat)
{
	u16 temp = m_a + dat;
	u16 temp4 = (m_a & 0x0f) + (dat & 0x0f);

	m_psw &= ~(C_FLAG | A_FLAG);
	m_psw |= (temp4 << 2) & A_FLAG;
	m_psw |= (temp >> 1) & C_FLAG;
	m_a = temp;
}


/*-------------------------------------------------
    execute_addc - perform the logic of an ADDC
    instruction
-------------------------------------------------*/

void mcs48_cpu::execute_addc(u8 dat)
{
	u8 carryin = (m_psw & C_FLAG) >> 7;
	u16 temp = m_a + dat + carryin;
	u16 temp4 = (m_a & 0x0f) + (dat & 0x0f) + carryin;

	m_psw &= ~(C_FLAG | A_FLAG);
	m_psw |= (temp4 << 2) & A_FLAG;
	m_psw |= (temp >> 1) & C_FLAG;
	m_a = temp;
}


/*-------------------------------------------------
    execute_jmp - perform the logic of a JMP
    instruction
-------------------------------------------------*/

void mcs48_cpu::execute_jmp(u16 address)
{
	u16 a11 = (m_irq_in_progress) ? 0 : m_a11;
	m_pc = address | a11;
}


/*-------------------------------------------------
    execute_call - perform the logic of a CALL
    instruction
-------------------------------------------------*/

void mcs48_cpu::execute_call(u16 address)
{
	push_pc_psw();
	execute_jmp(address);
}


/*-------------------------------------------------
    execute_jcc - perform the logic of a
    conditional jump instruction
-------------------------------------------------*/

void mcs48_cpu::execute_jcc(bool result)
{
	u16 pch = m_pc & 0xf00;
	u8 offset = argument_fetch();
	if (result)
		m_pc = pch | offset;
}


/*-------------------------------------------------
    p2_mask - return the mask of bits that the
    code can directly affect
-------------------------------------------------*/



/*-------------------------------------------------
    expander_operation - perform an operation via
    the 8243 expander chip
-------------------------------------------------*/

void mcs48_cpu::expander_operation(expander_op operation, u8 port)
{
	// put opcode on low 4 bits of P2 (overwriting latch)
	port_w(2, m_p2 = (m_p2 & 0xf0) | (u8(operation) << 2) | (port & 3));

	// generate high-to-low transition on PROG line
	prog_w(0);

	// transfer data on low 4 bits of P2
	if (operation != EXPANDER_OP_READ)
		port_w(2, m_p2 = (m_p2 & 0xf0) | (m_a & 0x0f));
	else
	{
		// place P20-P23 in input mode
		port_w(2, m_p2 |= 0x0f);

		// input data to lower 4 bits of A (upper 4 bits are cleared)
		m_a = port_r(2) & 0x0f;
	}

	// generate low-to-high transition on PROG line
	prog_w(1);
}



/*******************************************************************************
    OPCODE HANDLERS
*******************************************************************************/


#define OPHANDLER(_name) void mcs48_cpu::_name()
OPHANDLER( illegal )
{
	burn_cycles(1);
	logerror("Illegal opcode = %02x @ %04X\n", program_r(m_prevpc), m_prevpc);
}

OPHANDLER( add_a_r0 )       { burn_cycles(1); execute_add(R0); }
OPHANDLER( add_a_r1 )       { burn_cycles(1); execute_add(R1); }
OPHANDLER( add_a_r2 )       { burn_cycles(1); execute_add(R2); }
OPHANDLER( add_a_r3 )       { burn_cycles(1); execute_add(R3); }
OPHANDLER( add_a_r4 )       { burn_cycles(1); execute_add(R4); }
OPHANDLER( add_a_r5 )       { burn_cycles(1); execute_add(R5); }
OPHANDLER( add_a_r6 )       { burn_cycles(1); execute_add(R6); }
OPHANDLER( add_a_r7 )       { burn_cycles(1); execute_add(R7); }
OPHANDLER( add_a_xr0 )      { burn_cycles(1); execute_add(ram_r(R0)); }
OPHANDLER( add_a_xr1 )      { burn_cycles(1); execute_add(ram_r(R1)); }
OPHANDLER( add_a_n )        { burn_cycles(2); execute_add(argument_fetch()); }

OPHANDLER( adc_a_r0 )       { burn_cycles(1); execute_addc(R0); }
OPHANDLER( adc_a_r1 )       { burn_cycles(1); execute_addc(R1); }
OPHANDLER( adc_a_r2 )       { burn_cycles(1); execute_addc(R2); }
OPHANDLER( adc_a_r3 )       { burn_cycles(1); execute_addc(R3); }
OPHANDLER( adc_a_r4 )       { burn_cycles(1); execute_addc(R4); }
OPHANDLER( adc_a_r5 )       { burn_cycles(1); execute_addc(R5); }
OPHANDLER( adc_a_r6 )       { burn_cycles(1); execute_addc(R6); }
OPHANDLER( adc_a_r7 )       { burn_cycles(1); execute_addc(R7); }
OPHANDLER( adc_a_xr0 )      { burn_cycles(1); execute_addc(ram_r(R0)); }
OPHANDLER( adc_a_xr1 )      { burn_cycles(1); execute_addc(ram_r(R1)); }
OPHANDLER( adc_a_n )        { burn_cycles(2); execute_addc(argument_fetch()); }

OPHANDLER( anl_a_r0 )       { burn_cycles(1); m_a &= R0; }
OPHANDLER( anl_a_r1 )       { burn_cycles(1); m_a &= R1; }
OPHANDLER( anl_a_r2 )       { burn_cycles(1); m_a &= R2; }
OPHANDLER( anl_a_r3 )       { burn_cycles(1); m_a &= R3; }
OPHANDLER( anl_a_r4 )       { burn_cycles(1); m_a &= R4; }
OPHANDLER( anl_a_r5 )       { burn_cycles(1); m_a &= R5; }
OPHANDLER( anl_a_r6 )       { burn_cycles(1); m_a &= R6; }
OPHANDLER( anl_a_r7 )       { burn_cycles(1); m_a &= R7; }
OPHANDLER( anl_a_xr0 )      { burn_cycles(1); m_a &= ram_r(R0); }
OPHANDLER( anl_a_xr1 )      { burn_cycles(1); m_a &= ram_r(R1); }
OPHANDLER( anl_a_n )        { burn_cycles(2); m_a &= argument_fetch(); }

OPHANDLER( anl_bus_n )      { burn_cycles(2); bus_w(bus_r() & argument_fetch()); }
OPHANDLER( anl_p1_n )       { burn_cycles(2); port_w(1, m_p1 &= argument_fetch()); }
OPHANDLER( anl_p2_n )       { burn_cycles(2); port_w(2, m_p2 &= argument_fetch() | ~p2_mask()); }
OPHANDLER( anld_p4_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_AND, 4); }
OPHANDLER( anld_p5_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_AND, 5); }
OPHANDLER( anld_p6_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_AND, 6); }
OPHANDLER( anld_p7_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_AND, 7); }

OPHANDLER( call_0 )         { burn_cycles(2); execute_call(argument_fetch() | 0x000); }
OPHANDLER( call_1 )         { burn_cycles(2); execute_call(argument_fetch() | 0x100); }
OPHANDLER( call_2 )         { burn_cycles(2); execute_call(argument_fetch() | 0x200); }
OPHANDLER( call_3 )         { burn_cycles(2); execute_call(argument_fetch() | 0x300); }
OPHANDLER( call_4 )         { burn_cycles(2); execute_call(argument_fetch() | 0x400); }
OPHANDLER( call_5 )         { burn_cycles(2); execute_call(argument_fetch() | 0x500); }
OPHANDLER( call_6 )         { burn_cycles(2); execute_call(argument_fetch() | 0x600); }
OPHANDLER( call_7 )         { burn_cycles(2); execute_call(argument_fetch() | 0x700); }

OPHANDLER( clr_a )          { burn_cycles(1); m_a = 0; }
OPHANDLER( clr_c )          { burn_cycles(1); m_psw &= ~C_FLAG; }
OPHANDLER( clr_f0 )         { burn_cycles(1); m_psw &= ~F_FLAG; }
OPHANDLER( clr_f1 )         { burn_cycles(1); m_f1 = false; }

OPHANDLER( cpl_a )          { burn_cycles(1); m_a ^= 0xff; }
OPHANDLER( cpl_c )          { burn_cycles(1); m_psw ^= C_FLAG; }
OPHANDLER( cpl_f0 )         { burn_cycles(1); m_psw ^= F_FLAG; }
OPHANDLER( cpl_f1 )         { burn_cycles(1); m_f1 = !m_f1; }

OPHANDLER( da_a )
{
	burn_cycles(1);

	if ((m_a & 0x0f) > 0x09 || (m_psw & A_FLAG))
	{
		if (m_a > 0xf9)
			m_psw |= C_FLAG;
		m_a += 0x06;
	}
	if ((m_a & 0xf0) > 0x90 || (m_psw & C_FLAG))
	{
		m_a += 0x60;
		m_psw |= C_FLAG;
	}
}

OPHANDLER( dec_a )          { burn_cycles(1); m_a--; }
OPHANDLER( dec_r0 )         { burn_cycles(1); R0--; }
OPHANDLER( dec_r1 )         { burn_cycles(1); R1--; }
OPHANDLER( dec_r2 )         { burn_cycles(1); R2--; }
OPHANDLER( dec_r3 )         { burn_cycles(1); R3--; }
OPHANDLER( dec_r4 )         { burn_cycles(1); R4--; }
OPHANDLER( dec_r5 )         { burn_cycles(1); R5--; }
OPHANDLER( dec_r6 )         { burn_cycles(1); R6--; }
OPHANDLER( dec_r7 )         { burn_cycles(1); R7--; }

OPHANDLER( dis_i )          { burn_cycles(1); m_xirq_enabled = false; }
OPHANDLER( dis_tcnti )      { burn_cycles(1); m_tirq_enabled = false; m_timer_overflow = false; }

OPHANDLER( djnz_r0 )        { burn_cycles(2); execute_jcc(--R0 != 0); }
OPHANDLER( djnz_r1 )        { burn_cycles(2); execute_jcc(--R1 != 0); }
OPHANDLER( djnz_r2 )        { burn_cycles(2); execute_jcc(--R2 != 0); }
OPHANDLER( djnz_r3 )        { burn_cycles(2); execute_jcc(--R3 != 0); }
OPHANDLER( djnz_r4 )        { burn_cycles(2); execute_jcc(--R4 != 0); }
OPHANDLER( djnz_r5 )        { burn_cycles(2); execute_jcc(--R5 != 0); }
OPHANDLER( djnz_r6 )        { burn_cycles(2); execute_jcc(--R6 != 0); }
OPHANDLER( djnz_r7 )        { burn_cycles(2); execute_jcc(--R7 != 0); }

OPHANDLER( en_i )           { burn_cycles(1); m_xirq_enabled = true; }
OPHANDLER( en_tcnti )       { burn_cycles(1); m_tirq_enabled = true; }
OPHANDLER( en_dma )         { burn_cycles(1); m_dma_enabled = true; port_w(2, m_p2); }
OPHANDLER( en_flags )       { burn_cycles(1); m_flags_enabled = true; port_w(2, m_p2); }
OPHANDLER( ent0_clk )      { burn_cycles(1); }

OPHANDLER( in_a_p0 )        { burn_cycles(2); m_a = bus_r() & m_dbbo; }
OPHANDLER( in_a_p1 )        { burn_cycles(2); m_a = port_r(1) & m_p1; }
OPHANDLER( in_a_p2 )        { burn_cycles(2); m_a = port_r(2) & m_p2; }
OPHANDLER( ins_a_bus )      { burn_cycles(2); m_a = bus_r(); }
OPHANDLER( in_a_dbb )
{
	burn_cycles(1);

	// acknowledge the IBF IRQ and clear the bit in STS
	m_sts &= ~STS_IBF;

	// if P2 flags are enabled, update the state of P2
	if (m_flags_enabled)
		port_w(2, m_p2);
	m_a = m_dbbi;
}

OPHANDLER( inc_a )          { burn_cycles(1); m_a++; }
OPHANDLER( inc_r0 )         { burn_cycles(1); R0++; }
OPHANDLER( inc_r1 )         { burn_cycles(1); R1++; }
OPHANDLER( inc_r2 )         { burn_cycles(1); R2++; }
OPHANDLER( inc_r3 )         { burn_cycles(1); R3++; }
OPHANDLER( inc_r4 )         { burn_cycles(1); R4++; }
OPHANDLER( inc_r5 )         { burn_cycles(1); R5++; }
OPHANDLER( inc_r6 )         { burn_cycles(1); R6++; }
OPHANDLER( inc_r7 )         { burn_cycles(1); R7++; }
OPHANDLER( inc_xr0 )        { burn_cycles(1); ram_w(R0, ram_r(R0) + 1); }
OPHANDLER( inc_xr1 )        { burn_cycles(1); ram_w(R1, ram_r(R1) + 1); }

OPHANDLER( jb_0 )           { burn_cycles(2); execute_jcc((m_a & 0x01) != 0); }
OPHANDLER( jb_1 )           { burn_cycles(2); execute_jcc((m_a & 0x02) != 0); }
OPHANDLER( jb_2 )           { burn_cycles(2); execute_jcc((m_a & 0x04) != 0); }
OPHANDLER( jb_3 )           { burn_cycles(2); execute_jcc((m_a & 0x08) != 0); }
OPHANDLER( jb_4 )           { burn_cycles(2); execute_jcc((m_a & 0x10) != 0); }
OPHANDLER( jb_5 )           { burn_cycles(2); execute_jcc((m_a & 0x20) != 0); }
OPHANDLER( jb_6 )           { burn_cycles(2); execute_jcc((m_a & 0x40) != 0); }
OPHANDLER( jb_7 )           { burn_cycles(2); execute_jcc((m_a & 0x80) != 0); }
OPHANDLER( jc )             { burn_cycles(2); execute_jcc((m_psw & C_FLAG) != 0); }
OPHANDLER( jf0 )            { burn_cycles(2); execute_jcc((m_psw & F_FLAG) != 0); }
OPHANDLER( jf1 )            { burn_cycles(2); execute_jcc(m_f1); }
OPHANDLER( jnc )            { burn_cycles(2); execute_jcc((m_psw & C_FLAG) == 0); }
OPHANDLER( jni )            { burn_cycles(2); m_irq_polled = (m_irq_state == 0); execute_jcc(m_irq_state != 0); }
OPHANDLER( jnibf )          { burn_cycles(2); m_irq_polled = (m_sts & STS_IBF) != 0; execute_jcc((m_sts & STS_IBF) == 0); }
OPHANDLER( jnt_0 )          { burn_cycles(2); execute_jcc(test_r(0) == 0); }
OPHANDLER( jnt_1 )          { burn_cycles(2); execute_jcc(test_r(1) == 0); }
OPHANDLER( jnz )            { burn_cycles(2); execute_jcc(m_a != 0); }
OPHANDLER( jobf )           { burn_cycles(2); execute_jcc((m_sts & STS_OBF) != 0); }
OPHANDLER( jtf )            { burn_cycles(2); execute_jcc(m_timer_flag); m_timer_flag = false; }
OPHANDLER( jt_0 )           { burn_cycles(2); execute_jcc(test_r(0) != 0); }
OPHANDLER( jt_1 )           { burn_cycles(2); execute_jcc(test_r(1) != 0); }
OPHANDLER( jz )             { burn_cycles(2); execute_jcc(m_a == 0); }

OPHANDLER( jmp_0 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x000); }
OPHANDLER( jmp_1 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x100); }
OPHANDLER( jmp_2 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x200); }
OPHANDLER( jmp_3 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x300); }
OPHANDLER( jmp_4 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x400); }
OPHANDLER( jmp_5 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x500); }
OPHANDLER( jmp_6 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x600); }
OPHANDLER( jmp_7 )          { burn_cycles(2); execute_jmp(argument_fetch() | 0x700); }
OPHANDLER( jmpp_xa )        { burn_cycles(2); m_pc &= 0xf00; m_pc |= program_r(m_pc | m_a); }

OPHANDLER( mov_a_n )        { burn_cycles(2); m_a = argument_fetch(); }
OPHANDLER( mov_a_psw )      { burn_cycles(1); m_a = m_psw | 0x08; }
OPHANDLER( mov_a_r0 )       { burn_cycles(1); m_a = R0; }
OPHANDLER( mov_a_r1 )       { burn_cycles(1); m_a = R1; }
OPHANDLER( mov_a_r2 )       { burn_cycles(1); m_a = R2; }
OPHANDLER( mov_a_r3 )       { burn_cycles(1); m_a = R3; }
OPHANDLER( mov_a_r4 )       { burn_cycles(1); m_a = R4; }
OPHANDLER( mov_a_r5 )       { burn_cycles(1); m_a = R5; }
OPHANDLER( mov_a_r6 )       { burn_cycles(1); m_a = R6; }
OPHANDLER( mov_a_r7 )       { burn_cycles(1); m_a = R7; }
OPHANDLER( mov_a_xr0 )      { burn_cycles(1); m_a = ram_r(R0); }
OPHANDLER( mov_a_xr1 )      { burn_cycles(1); m_a = ram_r(R1); }
OPHANDLER( mov_a_t )        { burn_cycles(1); m_a = m_timer; }

OPHANDLER( mov_psw_a )      { burn_cycles(1); m_psw = m_a & ~0x08; update_regptr(); }
OPHANDLER( mov_sts_a )      { burn_cycles(1); m_sts = (m_sts & 0x0f) | (m_a & 0xf0); }
OPHANDLER( mov_r0_a )       { burn_cycles(1); R0 = m_a; }
OPHANDLER( mov_r1_a )       { burn_cycles(1); R1 = m_a; }
OPHANDLER( mov_r2_a )       { burn_cycles(1); R2 = m_a; }
OPHANDLER( mov_r3_a )       { burn_cycles(1); R3 = m_a; }
OPHANDLER( mov_r4_a )       { burn_cycles(1); R4 = m_a; }
OPHANDLER( mov_r5_a )       { burn_cycles(1); R5 = m_a; }
OPHANDLER( mov_r6_a )       { burn_cycles(1); R6 = m_a; }
OPHANDLER( mov_r7_a )       { burn_cycles(1); R7 = m_a; }
OPHANDLER( mov_r0_n )       { burn_cycles(2); R0 = argument_fetch(); }
OPHANDLER( mov_r1_n )       { burn_cycles(2); R1 = argument_fetch(); }
OPHANDLER( mov_r2_n )       { burn_cycles(2); R2 = argument_fetch(); }
OPHANDLER( mov_r3_n )       { burn_cycles(2); R3 = argument_fetch(); }
OPHANDLER( mov_r4_n )       { burn_cycles(2); R4 = argument_fetch(); }
OPHANDLER( mov_r5_n )       { burn_cycles(2); R5 = argument_fetch(); }
OPHANDLER( mov_r6_n )       { burn_cycles(2); R6 = argument_fetch(); }
OPHANDLER( mov_r7_n )       { burn_cycles(2); R7 = argument_fetch(); }
OPHANDLER( mov_t_a )        { burn_cycles(1); m_timer = m_a; }
OPHANDLER( mov_xr0_a )      { burn_cycles(1); ram_w(R0, m_a); }
OPHANDLER( mov_xr1_a )      { burn_cycles(1); ram_w(R1, m_a); }
OPHANDLER( mov_xr0_n )      { burn_cycles(2); ram_w(R0, argument_fetch()); }
OPHANDLER( mov_xr1_n )      { burn_cycles(2); ram_w(R1, argument_fetch()); }

OPHANDLER( movd_a_p4 )      { burn_cycles(2); expander_operation(EXPANDER_OP_READ, 4); }
OPHANDLER( movd_a_p5 )      { burn_cycles(2); expander_operation(EXPANDER_OP_READ, 5); }
OPHANDLER( movd_a_p6 )      { burn_cycles(2); expander_operation(EXPANDER_OP_READ, 6); }
OPHANDLER( movd_a_p7 )      { burn_cycles(2); expander_operation(EXPANDER_OP_READ, 7); }
OPHANDLER( movd_p4_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_WRITE, 4); }
OPHANDLER( movd_p5_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_WRITE, 5); }
OPHANDLER( movd_p6_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_WRITE, 6); }
OPHANDLER( movd_p7_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_WRITE, 7); }

OPHANDLER( movp_a_xa )      { burn_cycles(2); m_a = program_r((m_pc & 0xf00) | m_a); }
OPHANDLER( movp3_a_xa )     { burn_cycles(2); m_a = program_r(0x300 | m_a); }

OPHANDLER( movx_a_xr0 )     { burn_cycles(2); m_a = ext_r(R0); }
OPHANDLER( movx_a_xr1 )     { burn_cycles(2); m_a = ext_r(R1); }
OPHANDLER( movx_xr0_a )     { burn_cycles(2); ext_w(R0, m_a); }
OPHANDLER( movx_xr1_a )     { burn_cycles(2); ext_w(R1, m_a); }

OPHANDLER( nop )            { burn_cycles(1); }

OPHANDLER( orl_a_r0 )       { burn_cycles(1); m_a |= R0; }
OPHANDLER( orl_a_r1 )       { burn_cycles(1); m_a |= R1; }
OPHANDLER( orl_a_r2 )       { burn_cycles(1); m_a |= R2; }
OPHANDLER( orl_a_r3 )       { burn_cycles(1); m_a |= R3; }
OPHANDLER( orl_a_r4 )       { burn_cycles(1); m_a |= R4; }
OPHANDLER( orl_a_r5 )       { burn_cycles(1); m_a |= R5; }
OPHANDLER( orl_a_r6 )       { burn_cycles(1); m_a |= R6; }
OPHANDLER( orl_a_r7 )       { burn_cycles(1); m_a |= R7; }
OPHANDLER( orl_a_xr0 )      { burn_cycles(1); m_a |= ram_r(R0); }
OPHANDLER( orl_a_xr1 )      { burn_cycles(1); m_a |= ram_r(R1); }
OPHANDLER( orl_a_n )        { burn_cycles(2); m_a |= argument_fetch(); }

OPHANDLER( orl_bus_n )      { burn_cycles(2); bus_w(bus_r() | argument_fetch()); }
OPHANDLER( orl_p1_n )       { burn_cycles(2); port_w(1, m_p1 |= argument_fetch()); }
OPHANDLER( orl_p2_n )       { burn_cycles(2); port_w(2, m_p2 |= argument_fetch() & p2_mask()); }
OPHANDLER( orld_p4_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_OR, 4); }
OPHANDLER( orld_p5_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_OR, 5); }
OPHANDLER( orld_p6_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_OR, 6); }
OPHANDLER( orld_p7_a )      { burn_cycles(2); expander_operation(EXPANDER_OP_OR, 7); }

OPHANDLER( outl_bus_a )     { burn_cycles(2); bus_w(m_a); }
OPHANDLER( outl_p0_a )      { burn_cycles(2); bus_w(m_dbbo = m_a); }
OPHANDLER( outl_p1_a )      { burn_cycles(2); port_w(1, m_p1 = m_a); }
OPHANDLER( outl_p2_a )      { burn_cycles(2); u8 mask = p2_mask(); port_w(2, m_p2 = (m_p2 & ~mask) | (m_a & mask)); }
OPHANDLER( out_dbb_a )
{
	burn_cycles(1);

	// copy to the DBBO and update the bit in STS
	m_dbbo = m_a;
	m_sts |= STS_OBF;

	// if P2 flags are enabled, update the state of P2
	if (m_flags_enabled)
		port_w(2, m_p2);
}

OPHANDLER( ret )            { burn_cycles(2); pull_pc(); }
OPHANDLER( retr )
{
	burn_cycles(2);

	// implicitly clear the IRQ in progress flip flop
	m_irq_in_progress = false;
	pull_pc_psw();
}

OPHANDLER( rl_a )           { burn_cycles(1); m_a = (m_a << 1) | (m_a >> 7); }
OPHANDLER( rlc_a )          { burn_cycles(1); u8 newc = m_a & C_FLAG; m_a = (m_a << 1) | (m_psw >> 7); m_psw = (m_psw & ~C_FLAG) | newc; }

OPHANDLER( rr_a )           { burn_cycles(1); m_a = (m_a >> 1) | (m_a << 7); }
OPHANDLER( rrc_a )          { burn_cycles(1); u8 newc = (m_a << 7) & C_FLAG; m_a = (m_a >> 1) | (m_psw & C_FLAG); m_psw = (m_psw & ~C_FLAG) | newc; }

OPHANDLER( sel_mb0 )        { burn_cycles(1); m_a11 = 0x000; }
OPHANDLER( sel_mb1 )        { burn_cycles(1); m_a11 = 0x800; }

OPHANDLER( sel_rb0 )        { burn_cycles(1); m_psw &= ~B_FLAG; update_regptr(); }
OPHANDLER( sel_rb1 )        { burn_cycles(1); m_psw |=  B_FLAG; update_regptr(); }

OPHANDLER( stop_tcnt )      { burn_cycles(1); m_timecount_enabled = 0; }
OPHANDLER( strt_t )         { burn_cycles(1); m_timecount_enabled = TIMER_ENABLED; m_prescaler = 0; }
OPHANDLER( strt_cnt )
{
	burn_cycles(1);
	if (!(m_timecount_enabled & COUNTER_ENABLED))
		m_t1_history = test_r(1);

	m_timecount_enabled = COUNTER_ENABLED;
}

OPHANDLER( swap_a )         { burn_cycles(1); m_a = (m_a << 4) | (m_a >> 4); }

OPHANDLER( xch_a_r0 )       { burn_cycles(1); u8 tmp = m_a; m_a = R0; R0 = tmp; }
OPHANDLER( xch_a_r1 )       { burn_cycles(1); u8 tmp = m_a; m_a = R1; R1 = tmp; }
OPHANDLER( xch_a_r2 )       { burn_cycles(1); u8 tmp = m_a; m_a = R2; R2 = tmp; }
OPHANDLER( xch_a_r3 )       { burn_cycles(1); u8 tmp = m_a; m_a = R3; R3 = tmp; }
OPHANDLER( xch_a_r4 )       { burn_cycles(1); u8 tmp = m_a; m_a = R4; R4 = tmp; }
OPHANDLER( xch_a_r5 )       { burn_cycles(1); u8 tmp = m_a; m_a = R5; R5 = tmp; }
OPHANDLER( xch_a_r6 )       { burn_cycles(1); u8 tmp = m_a; m_a = R6; R6 = tmp; }
OPHANDLER( xch_a_r7 )       { burn_cycles(1); u8 tmp = m_a; m_a = R7; R7 = tmp; }
OPHANDLER( xch_a_xr0 )      { burn_cycles(1); u8 tmp = m_a; m_a = ram_r(R0); ram_w(R0, tmp); }
OPHANDLER( xch_a_xr1 )      { burn_cycles(1); u8 tmp = m_a; m_a = ram_r(R1); ram_w(R1, tmp); }

OPHANDLER( xchd_a_xr0 )     { burn_cycles(1); u8 oldram = ram_r(R0); ram_w(R0, (oldram & 0xf0) | (m_a & 0x0f)); m_a = (m_a & 0xf0) | (oldram & 0x0f); }
OPHANDLER( xchd_a_xr1 )     { burn_cycles(1); u8 oldram = ram_r(R1); ram_w(R1, (oldram & 0xf0) | (m_a & 0x0f)); m_a = (m_a & 0xf0) | (oldram & 0x0f); }

OPHANDLER( xrl_a_r0 )       { burn_cycles(1); m_a ^= R0; }
OPHANDLER( xrl_a_r1 )       { burn_cycles(1); m_a ^= R1; }
OPHANDLER( xrl_a_r2 )       { burn_cycles(1); m_a ^= R2; }
OPHANDLER( xrl_a_r3 )       { burn_cycles(1); m_a ^= R3; }
OPHANDLER( xrl_a_r4 )       { burn_cycles(1); m_a ^= R4; }
OPHANDLER( xrl_a_r5 )       { burn_cycles(1); m_a ^= R5; }
OPHANDLER( xrl_a_r6 )       { burn_cycles(1); m_a ^= R6; }
OPHANDLER( xrl_a_r7 )       { burn_cycles(1); m_a ^= R7; }
OPHANDLER( xrl_a_xr0 )      { burn_cycles(1); m_a ^= ram_r(R0); }
OPHANDLER( xrl_a_xr1 )      { burn_cycles(1); m_a ^= ram_r(R1); }
OPHANDLER( xrl_a_n )        { burn_cycles(2); m_a ^= argument_fetch(); }




#define OP(_a) &mcs48_cpu::_a
const mcs48_cpu::ophandler mcs48_cpu::s_opcodes[256] =
{
	OP(nop),        OP(illegal),    OP(outl_bus_a),OP(add_a_n),   OP(jmp_0),     OP(en_i),       OP(illegal),   OP(dec_a),      // 00
	OP(ins_a_bus),  OP(in_a_p1),    OP(in_a_p2),   OP(illegal),   OP(movd_a_p4), OP(movd_a_p5),  OP(movd_a_p6), OP(movd_a_p7),
	OP(inc_xr0),    OP(inc_xr1),    OP(jb_0),      OP(adc_a_n),   OP(call_0),    OP(dis_i),      OP(jtf),       OP(inc_a),      // 10
	OP(inc_r0),     OP(inc_r1),     OP(inc_r2),    OP(inc_r3),    OP(inc_r4),    OP(inc_r5),     OP(inc_r6),    OP(inc_r7),
	OP(xch_a_xr0),  OP(xch_a_xr1),  OP(illegal),   OP(mov_a_n),   OP(jmp_1),     OP(en_tcnti),   OP(jnt_0),     OP(clr_a),      // 20
	OP(xch_a_r0),   OP(xch_a_r1),   OP(xch_a_r2),  OP(xch_a_r3),  OP(xch_a_r4),  OP(xch_a_r5),   OP(xch_a_r6),  OP(xch_a_r7),
	OP(xchd_a_xr0), OP(xchd_a_xr1), OP(jb_1),      OP(illegal),   OP(call_1),    OP(dis_tcnti),  OP(jt_0),      OP(cpl_a),      // 30
	OP(illegal),    OP(outl_p1_a),  OP(outl_p2_a), OP(illegal),   OP(movd_p4_a), OP(movd_p5_a),  OP(movd_p6_a), OP(movd_p7_a),
	OP(orl_a_xr0),  OP(orl_a_xr1),  OP(mov_a_t),   OP(orl_a_n),   OP(jmp_2),     OP(strt_cnt),   OP(jnt_1),     OP(swap_a),     // 40
	OP(orl_a_r0),   OP(orl_a_r1),   OP(orl_a_r2),  OP(orl_a_r3),  OP(orl_a_r4),  OP(orl_a_r5),   OP(orl_a_r6),  OP(orl_a_r7),
	OP(anl_a_xr0),  OP(anl_a_xr1),  OP(jb_2),      OP(anl_a_n),   OP(call_2),    OP(strt_t),     OP(jt_1),      OP(da_a),       // 50
	OP(anl_a_r0),   OP(anl_a_r1),   OP(anl_a_r2),  OP(anl_a_r3),  OP(anl_a_r4),  OP(anl_a_r5),   OP(anl_a_r6),  OP(anl_a_r7),
	OP(add_a_xr0),  OP(add_a_xr1),  OP(mov_t_a),   OP(illegal),   OP(jmp_3),     OP(stop_tcnt),  OP(illegal),   OP(rrc_a),      // 60
	OP(add_a_r0),   OP(add_a_r1),   OP(add_a_r2),  OP(add_a_r3),  OP(add_a_r4),  OP(add_a_r5),   OP(add_a_r6),  OP(add_a_r7),
	OP(adc_a_xr0),  OP(adc_a_xr1),  OP(jb_3),      OP(illegal),   OP(call_3),    OP(ent0_clk),   OP(jf1),       OP(rr_a),       // 70
	OP(adc_a_r0),   OP(adc_a_r1),   OP(adc_a_r2),  OP(adc_a_r3),  OP(adc_a_r4),  OP(adc_a_r5),   OP(adc_a_r6),  OP(adc_a_r7),
	OP(movx_a_xr0), OP(movx_a_xr1), OP(illegal),   OP(ret),       OP(jmp_4),     OP(clr_f0),     OP(jni),       OP(illegal),    // 80
	OP(orl_bus_n),  OP(orl_p1_n),   OP(orl_p2_n),  OP(illegal),   OP(orld_p4_a), OP(orld_p5_a),  OP(orld_p6_a), OP(orld_p7_a),
	OP(movx_xr0_a), OP(movx_xr1_a), OP(jb_4),      OP(retr),      OP(call_4),    OP(cpl_f0),     OP(jnz),       OP(clr_c),      // 90
	OP(anl_bus_n),  OP(anl_p1_n),   OP(anl_p2_n),  OP(illegal),   OP(anld_p4_a), OP(anld_p5_a),  OP(anld_p6_a), OP(anld_p7_a),
	OP(mov_xr0_a),  OP(mov_xr1_a),  OP(illegal),   OP(movp_a_xa), OP(jmp_5),     OP(clr_f1),     OP(illegal),   OP(cpl_c),      // A0
	OP(mov_r0_a),   OP(mov_r1_a),   OP(mov_r2_a),  OP(mov_r3_a),  OP(mov_r4_a),  OP(mov_r5_a),   OP(mov_r6_a),  OP(mov_r7_a),
	OP(mov_xr0_n),  OP(mov_xr1_n),  OP(jb_5),      OP(jmpp_xa),   OP(call_5),    OP(cpl_f1),     OP(jf0),       OP(illegal),    // B0
	OP(mov_r0_n),   OP(mov_r1_n),   OP(mov_r2_n),  OP(mov_r3_n),  OP(mov_r4_n),  OP(mov_r5_n),   OP(mov_r6_n),  OP(mov_r7_n),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(illegal),   OP(jmp_6),     OP(sel_rb0),    OP(jz),        OP(mov_a_psw),  // C0
	OP(dec_r0),     OP(dec_r1),     OP(dec_r2),    OP(dec_r3),    OP(dec_r4),    OP(dec_r5),     OP(dec_r6),    OP(dec_r7),
	OP(xrl_a_xr0),  OP(xrl_a_xr1),  OP(jb_6),      OP(xrl_a_n),   OP(call_6),    OP(sel_rb1),    OP(illegal),   OP(mov_psw_a),  // D0
	OP(xrl_a_r0),   OP(xrl_a_r1),   OP(xrl_a_r2),  OP(xrl_a_r3),  OP(xrl_a_r4),  OP(xrl_a_r5),   OP(xrl_a_r6),  OP(xrl_a_r7),
	OP(illegal),    OP(illegal),    OP(illegal),   OP(movp3_a_xa),OP(jmp_7),     OP(sel_mb0),    OP(jnc),       OP(rl_a),       // E0
	OP(djnz_r0),    OP(djnz_r1),    OP(djnz_r2),   OP(djnz_r3),   OP(djnz_r4),   OP(djnz_r5),    OP(djnz_r6),   OP(djnz_r7),
	OP(mov_a_xr0),  OP(mov_a_xr1),  OP(jb_7),      OP(illegal),   OP(call_7),    OP(sel_mb1),    OP(jc),        OP(rlc_a),      // F0
	OP(mov_a_r0),   OP(mov_a_r1),   OP(mov_a_r2),  OP(mov_a_r3),  OP(mov_a_r4),  OP(mov_a_r5),   OP(mov_a_r6),  OP(mov_a_r7)
};
void mcs48_cpu::check_irqs()
{
	// if something is in progress, we do nothing
	if (m_irq_in_progress)
		return;

	// external interrupts take priority
	else if ((m_irq_state || (m_sts & STS_IBF) != 0) && m_xirq_enabled)
	{
		// indicate we took the external IRQ
		

		burn_cycles(2);
		m_irq_in_progress = true;

		// force JNI to be taken (hack)
		if (m_irq_polled)
		{
			m_pc = ((m_prevpc + 1) & 0x7ff) | (m_prevpc & 0x800);
			execute_jcc(true);
		}

		// transfer to location 0x03
		execute_call(0x03);
	}

	// timer overflow interrupts follow
	else if (m_timer_overflow && m_tirq_enabled)
	{
		

		burn_cycles(2);
		m_irq_in_progress = true;

		// transfer to location 0x07
		execute_call(0x07);

		// timer overflow flip-flop is reset once taken
		m_timer_overflow = false;
	}
}


/*-------------------------------------------------
    burn_cycles - burn cycles, processing timers
    and counters
-------------------------------------------------*/

void mcs48_cpu::burn_cycles(int count)
{
	if (m_timecount_enabled)
	{
		bool timerover = false;

		// if the timer is enabled, accumulate prescaler cycles
		if (m_timecount_enabled & TIMER_ENABLED)
		{
			u8 oldtimer = m_timer;
			m_prescaler += count;
			m_timer += m_prescaler >> 5;
			m_prescaler &= 0x1f;
			timerover = m_timer < oldtimer;
		}

		// if the counter is enabled, poll the T1 test input once for each cycle
		else if (m_timecount_enabled & COUNTER_ENABLED)
		{
			for ( ; count > 0; count--, m_icount--)
			{
				m_t1_history = (m_t1_history << 1) | (test_r(1) & 1);
				if ((m_t1_history & 3) == 2)
				{
					if (++m_timer == 0)
						timerover = true;
				}
			}
		}

		// if either source caused a timer overflow, set the flags
		if (timerover)
		{
			m_timer_flag = true;

			// according to the docs, if an overflow occurs with interrupts disabled, the overflow is not stored
			if (m_tirq_enabled)
				m_timer_overflow = true;
		}
	}

	// (note: if timer counter is enabled, count was already reduced to 0)
	m_icount -= count;
}


/*-------------------------------------------------
    mcs48_execute - execute until we run out
    of cycles
-------------------------------------------------*/

int mcs48_cpu::run(int cycles)
{
	m_icount = cycles;
	const int start = cycles;
	update_regptr();

	// iterate over remaining cycles, guaranteeing at least one instruction
	do
	{
		// check interrupts
		check_irqs();
		m_irq_polled = false;

		m_prevpc = m_pc;
		

		// fetch and process opcode
		unsigned opcode = opcode_fetch();
		(this->*s_opcodes[opcode])();

	} while (m_icount > 0);
	return start - m_icount;
}




void mcs48_cpu::reset() {
	m_pc = 0;
	m_psw = m_psw & (C_FLAG | A_FLAG);
	update_regptr();
	m_f1 = false; m_a11 = 0; m_tirq_enabled = false; m_xirq_enabled = false;
	m_timecount_enabled = 0; m_timer_flag = false; m_sts = 0; m_flags_enabled = false; m_dma_enabled = false;
	m_irq_in_progress = false; m_timer_overflow = false; m_irq_polled = false;
	m_dbbo = 0xff;
	bus_w(0xff);
	m_p1 = 0xff; m_p2 = 0xff;
	port_w(1, m_p1); port_w(2, m_p2);
}
