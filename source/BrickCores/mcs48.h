#pragma once
#include <cstdint>
#include <functional>

// Intel 8048 (MCS-48) - see mcs48.cpp. One machine cycle = 15 clocks.
class mcs48_cpu {
public:
    typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t offs_t;
    std::function<u8(u16)> rd_program;           // fetch, with EA applied by the machine
    std::function<u8(u8)> rd_ext;                 // MOVX A,@Rr
    std::function<void(u8, u8)> wr_ext;           // MOVX @Rr,A
    std::function<u8(int)> rd_port;               // ports 1, 2
    std::function<void(int, u8)> wr_port;
    std::function<int(int)> rd_test;              // T0, T1
    std::function<u8()> rd_bus;
    std::function<void(u8)> wr_bus;

    void reset();
    int run(int cycles);   // returns cycles used
    void set_irq(bool s) { m_irq_state = s; }

    // state (public for save states)
    u16 m_prevpc = 0, m_pc = 0;
    u8 m_a = 0, m_psw = 0;
    bool m_f1 = false;
    u16 m_a11 = 0;
    u8 m_p1 = 0, m_p2 = 0, m_timer = 0, m_prescaler = 0, m_t1_history = 0, m_sts = 0, m_dbbi = 0, m_dbbo = 0;
    bool m_irq_state = false, m_irq_polled = false, m_irq_in_progress = false, m_timer_overflow = false, m_timer_flag = false;
    bool m_tirq_enabled = false, m_xirq_enabled = false;
    u8 m_timecount_enabled = 0;
    bool m_flags_enabled = false, m_dma_enabled = false;
    u8 m_ram[64] = {};
    int m_icount = 0;
    void update_regptr();

private:
    enum expander_op { EXPANDER_OP_READ = 0, EXPANDER_OP_WRITE = 1, EXPANDER_OP_OR = 2, EXPANDER_OP_AND = 3 };
    typedef void (mcs48_cpu::*ophandler)();
    static const ophandler s_opcodes[256];
    u8* m_dataptr = m_ram;
    u8* m_regptr = m_ram;

    u8 program_r(offs_t a) { return rd_program((u16)(a & 0xfff)); }
    u8 ram_r(offs_t a) { return m_ram[a & 63]; }
    void ram_w(offs_t a, u8 v) { m_ram[a & 63] = v; }
    u8 ext_r(offs_t a) { return rd_ext ? rd_ext((u8)a) : 0xff; }
    void ext_w(offs_t a, u8 v) { if (wr_ext) wr_ext((u8)a, v); }
    u8 port_r(offs_t a) { return rd_port ? rd_port((int)a) : 0xff; }
    void port_w(offs_t a, u8 v) { if (wr_port) wr_port((int)a, v); }
    int test_r(offs_t a) { return rd_test ? rd_test((int)a) : 0; }
    u8 bus_r() { return rd_bus ? rd_bus() : 0xff; }
    void bus_w(u8 v) { if (wr_bus) wr_bus(v); }
    void prog_w(int) {}
    u8 opcode_fetch();
    u8 argument_fetch();
    void push_pc_psw();
    void pull_pc_psw();
    void pull_pc();
    void execute_add(u8 dat);
    void execute_addc(u8 dat);
    void execute_jmp(u16 address);
    void execute_call(u16 address);
    void execute_jcc(bool result);
    u8 p2_mask() { return 0xff; }
    void expander_operation(expander_op operation, u8 port);
    void check_irqs();
    void burn_cycles(int count);
    void adc_a_n();
    void adc_a_r0();
    void adc_a_r1();
    void adc_a_r2();
    void adc_a_r3();
    void adc_a_r4();
    void adc_a_r5();
    void adc_a_r6();
    void adc_a_r7();
    void adc_a_xr0();
    void adc_a_xr1();
    void add_a_n();
    void add_a_r0();
    void add_a_r1();
    void add_a_r2();
    void add_a_r3();
    void add_a_r4();
    void add_a_r5();
    void add_a_r6();
    void add_a_r7();
    void add_a_xr0();
    void add_a_xr1();
    void anl_a_n();
    void anl_a_r0();
    void anl_a_r1();
    void anl_a_r2();
    void anl_a_r3();
    void anl_a_r4();
    void anl_a_r5();
    void anl_a_r6();
    void anl_a_r7();
    void anl_a_xr0();
    void anl_a_xr1();
    void anl_bus_n();
    void anl_p1_n();
    void anl_p2_n();
    void anld_p4_a();
    void anld_p5_a();
    void anld_p6_a();
    void anld_p7_a();
    void call_0();
    void call_1();
    void call_2();
    void call_3();
    void call_4();
    void call_5();
    void call_6();
    void call_7();
    void clr_a();
    void clr_c();
    void clr_f0();
    void clr_f1();
    void cpl_a();
    void cpl_c();
    void cpl_f0();
    void cpl_f1();
    void da_a();
    void dec_a();
    void dec_r0();
    void dec_r1();
    void dec_r2();
    void dec_r3();
    void dec_r4();
    void dec_r5();
    void dec_r6();
    void dec_r7();
    void dis_i();
    void dis_tcnti();
    void djnz_r0();
    void djnz_r1();
    void djnz_r2();
    void djnz_r3();
    void djnz_r4();
    void djnz_r5();
    void djnz_r6();
    void djnz_r7();
    void en_dma();
    void en_flags();
    void en_i();
    void en_tcnti();
    void ent0_clk();
    void illegal();
    void in_a_dbb();
    void in_a_p0();
    void in_a_p1();
    void in_a_p2();
    void inc_a();
    void inc_r0();
    void inc_r1();
    void inc_r2();
    void inc_r3();
    void inc_r4();
    void inc_r5();
    void inc_r6();
    void inc_r7();
    void inc_xr0();
    void inc_xr1();
    void ins_a_bus();
    void jb_0();
    void jb_1();
    void jb_2();
    void jb_3();
    void jb_4();
    void jb_5();
    void jb_6();
    void jb_7();
    void jc();
    void jf0();
    void jf1();
    void jmp_0();
    void jmp_1();
    void jmp_2();
    void jmp_3();
    void jmp_4();
    void jmp_5();
    void jmp_6();
    void jmp_7();
    void jmpp_xa();
    void jnc();
    void jni();
    void jnibf();
    void jnt_0();
    void jnt_1();
    void jnz();
    void jobf();
    void jt_0();
    void jt_1();
    void jtf();
    void jz();
    void mov_a_n();
    void mov_a_psw();
    void mov_a_r0();
    void mov_a_r1();
    void mov_a_r2();
    void mov_a_r3();
    void mov_a_r4();
    void mov_a_r5();
    void mov_a_r6();
    void mov_a_r7();
    void mov_a_t();
    void mov_a_xr0();
    void mov_a_xr1();
    void mov_psw_a();
    void mov_r0_a();
    void mov_r0_n();
    void mov_r1_a();
    void mov_r1_n();
    void mov_r2_a();
    void mov_r2_n();
    void mov_r3_a();
    void mov_r3_n();
    void mov_r4_a();
    void mov_r4_n();
    void mov_r5_a();
    void mov_r5_n();
    void mov_r6_a();
    void mov_r6_n();
    void mov_r7_a();
    void mov_r7_n();
    void mov_sts_a();
    void mov_t_a();
    void mov_xr0_a();
    void mov_xr0_n();
    void mov_xr1_a();
    void mov_xr1_n();
    void movd_a_p4();
    void movd_a_p5();
    void movd_a_p6();
    void movd_a_p7();
    void movd_p4_a();
    void movd_p5_a();
    void movd_p6_a();
    void movd_p7_a();
    void movp3_a_xa();
    void movp_a_xa();
    void movx_a_xr0();
    void movx_a_xr1();
    void movx_xr0_a();
    void movx_xr1_a();
    void nop();
    void orl_a_n();
    void orl_a_r0();
    void orl_a_r1();
    void orl_a_r2();
    void orl_a_r3();
    void orl_a_r4();
    void orl_a_r5();
    void orl_a_r6();
    void orl_a_r7();
    void orl_a_xr0();
    void orl_a_xr1();
    void orl_bus_n();
    void orl_p1_n();
    void orl_p2_n();
    void orld_p4_a();
    void orld_p5_a();
    void orld_p6_a();
    void orld_p7_a();
    void out_dbb_a();
    void outl_bus_a();
    void outl_p0_a();
    void outl_p1_a();
    void outl_p2_a();
    void ret();
    void retr();
    void rl_a();
    void rlc_a();
    void rr_a();
    void rrc_a();
    void sel_mb0();
    void sel_mb1();
    void sel_rb0();
    void sel_rb1();
    void stop_tcnt();
    void strt_cnt();
    void strt_t();
    void swap_a();
    void xch_a_r0();
    void xch_a_r1();
    void xch_a_r2();
    void xch_a_r3();
    void xch_a_r4();
    void xch_a_r5();
    void xch_a_r6();
    void xch_a_r7();
    void xch_a_xr0();
    void xch_a_xr1();
    void xchd_a_xr0();
    void xchd_a_xr1();
    void xrl_a_n();
    void xrl_a_r0();
    void xrl_a_r1();
    void xrl_a_r2();
    void xrl_a_r3();
    void xrl_a_r4();
    void xrl_a_r5();
    void xrl_a_r6();
    void xrl_a_r7();
    void xrl_a_xr0();
    void xrl_a_xr1();
};
