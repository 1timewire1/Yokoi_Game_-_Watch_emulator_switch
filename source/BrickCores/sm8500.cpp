// Sharp SM8500 - see sm8500.h. Code by Wilbert Pol (MAME, license:BSD-3-Clause).
#include "sm8500.h"
#include <cstdio>

namespace {
template <typename T> constexpr int BIT(T x, int n) { return (x >> n) & 1; }
inline void logerror(const char*, ...) {}
constexpr uint8_t sm8500_b2w[8] = { 0, 8, 2, 10, 4, 12, 6, 14 };
}

void sm8500_cpu::get_sp()
{
	m_SP = m_program->read_byte(0x1d);
	if (m_SYS & 0x40) m_SP |= ( m_program->read_byte(0x1c) << 8 );
}


uint8_t sm8500_cpu::mem_readbyte( uint32_t offset )
{
	offset &= 0xffff;
	if ( offset < 0x10)
	{
		return m_register_ram[offset + (m_PS0 & 0xF8)];
	}

	return m_program->read_byte( offset );
}


void sm8500_cpu::mem_writebyte( uint32_t offset, uint8_t data )
{
	uint8_t i;
	offset &= 0xffff;
	if (offset < 0x10)
	{
		m_register_ram[offset + (m_PS0 & 0xF8)] = data;
	}

	m_program->write_byte( offset, data );

	switch (offset)
	{
		case 0x10: m_IE0 = data; break;
		case 0x11: m_IE1 = data; break;
		case 0x12: m_IR0 = data; break;
		case 0x13: m_IR1 = data; break;
		case 0x19: m_SYS = data; break;
		case 0x1a: m_CKC = data; break;
		case 0x1c:
		case 0x1d: get_sp(); break;
		case 0x1e: m_PS0 = data;
				for (i = 0; i < 16; i++)    // refresh register contents in debugger
				{
					m_program->write_byte(i, mem_readbyte(i));
				}
				break;
		case 0x1f: m_PS1 = data; break;
	}
}


void sm8500_cpu::reset()
{
	for (auto & elem : m_register_ram)
	{
		elem = 0;
	}

	m_PC = 0x1020;
	m_clock_changed = 0;
	m_CheckInterrupts = 0;
	m_halted = 0;
	m_IFLAGS = 0;
	mem_writeword(0x10, 0);                 // IE0, IE1
	mem_writeword(0x12, 0);                 // IR0, IR1
	mem_writeword(0x14, 0xffff);            // P0, P1
	mem_writeword(0x16, 0xff00);            // P2, P3
	mem_writebyte(0x19, 0);                 // SYS
	mem_writebyte(0x1a, 0);                 // CKC
	mem_writebyte(0x1f, 0);                 // PS1
	mem_writebyte(0x2b, 0xff);              // URTT
	mem_writebyte(0x2d, 0x42);              // URTS
	mem_writebyte(0x5f, 0x38);              // WDTC
}


#define PUSH_BYTE(X)    m_SP--; \
			if ( ( m_SYS & 0x40 ) == 0 ) m_SP &= 0xFF; \
			mem_writebyte( m_SP, X );


void sm8500_cpu::take_interrupt(uint16_t vector)
{
	/* Get regs from ram */
	get_sp();
	m_SYS = m_program->read_byte(0x19);
	m_PS1 = m_program->read_byte(0x1f);
	/* Push PC */
	PUSH_BYTE( m_PC & 0xFF );
	PUSH_BYTE( m_PC >> 8 );
	/* Push PS1 */
	PUSH_BYTE( m_PS1 );
	/* Clear I flag */
	m_PS1 &= ~ 0x01;
	/* save regs to ram */
	m_program->write_byte(0x1f, m_PS1);
	m_program->write_byte(0x1d, m_SP&0xFF);
	if (m_SYS&0x40) m_program->write_byte(0x1c, m_SP>>8);
	/* Change PC to address stored at "vector" */
	m_PC = mem_readword( vector );
}


void sm8500_cpu::process_interrupts()
{
	if ( m_CheckInterrupts )
	{
		int irqline = 0;
		while( irqline < 11 )
		{
			if ( m_IFLAGS & ( 1 << irqline ) )
			{
				m_halted = 0;
				m_IE0 = m_program->read_byte(0x10);
				m_IE1 = m_program->read_byte(0x11);
				m_IR0 = m_program->read_byte(0x12);
				m_IR1 = m_program->read_byte(0x13);
				m_PS0 = m_program->read_byte(0x1e);
				m_PS1 = m_program->read_byte(0x1f);
				switch( irqline )
				{
				case WDT_INT:
					take_interrupt( 0x101C );
					break;
				case ILL_INT:
				case NMI_INT:
					take_interrupt( 0x101E );
					break;
				case DMA_INT:
					m_IR0 |= 0x80;
					if ( BIT( m_IE0, 7) && BIT( m_PS1, 0) )
					{
						take_interrupt( 0x1000 );
					}
					break;
				case TIM0_INT:
					m_IR0 |= 0x40;
					if ( BIT( m_IE0, 6) && BIT( m_PS1, 0) )
					{
						take_interrupt( 0x1002 );
					}
					break;
				case EXT_INT:
					m_IR0 |= 0x10;
					if ( BIT( m_IE0, 4) && ( ( m_PS0 & 0x07 ) < 7 ) && BIT( m_PS1, 0) )
					{
						take_interrupt( 0x1006 );
					}
					break;
				case UART_INT:
					m_IR0 |= 0x08;
					if ( BIT( m_IE0, 3) && ( ( m_PS0 & 0x07 ) < 6 ) && BIT( m_PS1, 0) )
					{
						take_interrupt( 0x1008 );
					}
					break;
				case LCDC_INT:
					m_IR0 |= 0x01;
					if ( BIT( m_IE0, 0) && ( ( m_PS0 & 0x07 ) < 5 ) && BIT( m_PS1, 0) )
					{
						take_interrupt( 0x100E );
					}
					break;
				case TIM1_INT:
					m_IR1 |= 0x40;
					if ( BIT( m_IE1, 6) && ( ( m_PS0 & 0x07 ) < 4 ) && BIT( m_PS1, 0) )
					{
						take_interrupt( 0x1012 );
					}
					break;
				case CK_INT:
					m_IR1 |= 0x10;
					if ( BIT( m_IE1, 4) && ( ( m_PS0 & 0x07 ) < 3 ) && BIT( m_PS1, 0) )
					{
						take_interrupt( 0x1016 );
					}
					break;
				case PIO_INT:
					m_IR1 |= 0x04;
					if ( BIT( m_IE1, 2) && ( ( m_PS0 & 0x07 ) < 2 ) && BIT( m_PS1, 0) )
					{
						take_interrupt( 0x101A );
					}
					break;
				}
				m_IFLAGS &= ~ ( 1 << irqline );
				m_program->write_byte(0x12, m_IR0);
				m_program->write_byte(0x13, m_IR1);
			}
			irqline++;
		}
	}
}


int sm8500_cpu::run(int cycles)
{
	m_icount = cycles;
	int used = 0;
	do
	{
		int     mycycles = 0;
		uint8_t   r1,r2;
		uint16_t  s1,s2;
		uint32_t  d1,d2;
		uint32_t  res;

		process_interrupts();
		if ( !m_halted ) {
			m_oldpc = m_PC;
			uint8_t op = mem_readbyte( m_PC++ );
			m_SYS = m_program->read_byte(0x19);
			m_PS0 = m_program->read_byte(0x1e);
			m_PS1 = m_program->read_byte(0x1f);
			get_sp();
			switch( op )
			{
#include "sm85ops.h"
			}
			if (m_SYS&0x40) m_program->write_byte(0x1c,m_SP>>8);
			m_program->write_byte(0x1d,m_SP&0xFF);
			mem_writebyte(0x1e,m_PS0); // need to update debugger
			m_program->write_byte(0x1f,m_PS1);
		} else {
			mycycles = 4;
			if (dma_cb) dma_cb( mycycles );
		}
		if (timer_cb) timer_cb( mycycles );
		m_icount -= mycycles;
		used += mycycles;
	} while ( m_icount > 0 );
	return used;
}


void sm8500_cpu::set_input_line( int inptnum, bool assert_ )
{
	m_IR0 = m_program->read_byte(0x12);
	m_IR1 = m_program->read_byte(0x13);
	if ( assert_ )
	{
		m_IFLAGS |= ( 0x01 << inptnum );
		m_CheckInterrupts = 1;
		switch( inptnum )
		{
			case DMA_INT:   m_IR0 |= 0x80; break;
			case TIM0_INT:  m_IR0 |= 0x40; break;
			case EXT_INT:   m_IR0 |= 0x10; break;
			case UART_INT:  m_IR0 |= 0x08; break;
			case LCDC_INT:  m_IR0 |= 0x01; break;
			case TIM1_INT:  m_IR1 |= 0x40; break;
			case CK_INT:    m_IR1 |= 0x10; break;
			case PIO_INT:   m_IR1 |= 0x04; break;
		}
	}
	else
	{
		m_IFLAGS &= ~( 0x01 << inptnum );
		switch( inptnum )
		{
			case DMA_INT:   m_IR0 &= ~0x80; break;
			case TIM0_INT:  m_IR0 &= ~0x40; break;
			case EXT_INT:   m_IR0 &= ~0x10; break;
			case UART_INT:  m_IR0 &= ~0x08; break;
			case LCDC_INT:  m_IR0 &= ~0x01; break;
			case TIM1_INT:  m_IR1 &= ~0x40; break;
			case CK_INT:    m_IR1 &= ~0x10; break;
			case PIO_INT:   m_IR1 &= ~0x04; break;
		}
		if ( 0 == m_IFLAGS )
		{
			m_CheckInterrupts = 0;
		}
	}
	m_program->write_byte(0x12, m_IR0);
	m_program->write_byte(0x13, m_IR1);
}
