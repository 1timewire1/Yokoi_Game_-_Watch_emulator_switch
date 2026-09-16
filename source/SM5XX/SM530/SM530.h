#pragma once
#include <cstdint>
#include <string>
#include "SM5XX/SM5XX.h"


// Sharp SM530 (Nelsonic Game Watch wristwatches, later Konami units).
// Ported from MAME's src/devices/cpu/sm510/sm530.{h,cpp} and sm530op.cpp
// (BSD-3-Clause, copyright-holders "hap"), cross-checked against a Lua port
// of the same source in reference/segment-main/sm510/sm530.lua. Reuses this
// engine's shared SM5XX opcode handlers (g_op_*) wherever MAME's own
// sm530_device leaves the sm511_device base method un-overridden; every
// opcode MAME *does* override on SM530 gets its own op_* here instead of a
// shared g_op_*, same convention as SM511_2.
//
// Real hardware differences from SM511 that matter here:
//  - 2Kx8 ROM (11-bit prg width, 32 pages x 64 words - not the 63+1 MAME-
//    dump-padding quirk SM511_2_ROM_MAME_WORD works around, so this uses
//    its own local word count instead of the shared ROM_WORD).
//  - Only 1 subroutine-return stack level (SM511 has 2), no r_buffer/
//    s_buffer chain.
//  - RAM address is composed directly from BM (2 bits, LB reaches only 4
//    values) and BL (4 bits), with SABM/SABL each contributing one extra
//    bit for exactly one following instruction (SM511 only has SBM/BM).
//  - LCD is 2 rows ("h", commons) x 12 groups ("o") x 4 segments ("y"),
//    read straight out of two 12-cell RAM banks, gated by a display-enable
//    latch (SDS/RDS) - not the "bs" special-pin scheme SM511/2 has. Segment
//    tags in these games' SVGs are titled "o.y.h" (see FINDINGS.md), so
//    get_segments_state(col,line,word) here means (o,y,h), not (o,y,word)
//    as elsewhere - segment_on is indexed [o][y] and packs h into 2 bits.
//  - A 4-flag "gamma" (10s/1s/0.5s/0.1s), each tested and cleared
//    independently by TG x - not a single gamma_flag_second bool. That
//    shared bool is still kept in sync (OR of all 4 flags) purely so the
//    inherited, non-virtual condition_to_wake_up() keeps working.
//  - Dedicated 1s and 1/100s hardware counters (count_1s, count_10ms);
//    DTA reads count_10ms here, not "1 second passed".
//  - K is one 8-bit input read as two nibbles: KTA (low 4 bits), KETA
//    (high 4 bits) - not multiplexed across 8 groups. Fed through
//    k_input_sp_not_multiplex/k_input[0] via input_set(..., not_multiplex).
//  - New instructions with no SM511 equivalent: TRS (indirect call through
//    a page-14 vector table), TG, KETA, ATS, ATF, SDS, RDS, INIS.
//  - Melody controller is SM511's, but with step mask 0xff (not 0x7f) -
//    same TONE table, double the cycles-per-note.

constexpr uint8_t SM530_ROM_COL = 2;
constexpr uint8_t SM530_ROM_LINE = 16; // 2*16 = 32 pages (5-bit page, matches PRGMASK 0x7FF)
constexpr uint8_t SM530_ROM_WORD = 64; // local word count - NOT the shared ROM_WORD (that's the SM511-dump-padding constant)

constexpr uint8_t SM530_RAM_COL = 8;  // addr = col*16+line, same convention as SM511_2's ram[8][16]
constexpr uint8_t SM530_RAM_LINE = 16;

constexpr uint8_t SM530_LCD_GROUPS = 12; // "o": 0-11
constexpr uint8_t SM530_LCD_SEGS = 4;    // "y": 0-3 segments per group
constexpr uint8_t SM530_LCD_ROWS = 2;    // "h": 0-1 commons, packed as 2 bits per segment_on cell

constexpr uint16_t SM530_MELODY_ROM_SIZE = 256;
constexpr uint32_t SM530_TIME_CHANGE_NOTE = 125000; // in u_second = 125ms (double SM511's 62.5ms: step mask 0xff vs 0x7f)


class SM530 : public SM5XX
{
public:
    SM530() : SM5XX("SM530\0") {}

public:
    void init() override;
    void load_rom(const uint8_t* file_hex, size_t size_hex) override;
    void load_rom_melody(const uint8_t* file_hex, size_t size_hex) override;

    bool screen_is_on() override; // dead code for the real rendering path, same as every other core - kept for parity
    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override; // (o, y, h) - see header comment
    bool get_active_sound() override;

    bool save_state(FILE* file) override;
    bool load_state(FILE* file) override;
    uint8_t get_cpu_type_id() override { return 4; } // CPU_TYPE_SM530 (0=?,1=SM5A?,2=SM511_2,3=SM510 by existing convention; see yokoi_cpu_utils.cpp)

private:
    /// Variables / registers ///
    uint8_t ram[SM530_RAM_COL][SM530_RAM_LINE];
    uint8_t rom[SM530_ROM_COL][SM530_ROM_LINE][SM530_ROM_WORD];

    uint8_t segment_on[SM530_LCD_GROUPS][SM530_LCD_SEGS]; // h packed as bit0/bit1 per cell

    uint8_t rom_melody[SM530_MELODY_ROM_SIZE];
    uint8_t rom_melody_address;

    bool ds; // display-enable latch (SDS/RDS) - gates lcd output, unlike SM511's dead-code bp_lcd_blackplate
    uint8_t bp; // ATBP target - not consumed anywhere real, kept for parity with SM511_2's bp_lcd_blackplate/op_atbp

    // gamma: bit0=10s, bit1=1s, bit2=0.5s, bit3=0.1s - each tested+cleared independently by TG x
    uint8_t gamma_sm530;
    uint16_t subdiv;
    uint8_t count_1s;
    uint8_t count_10ms;

    // SABM (col override, mirrors SM511_2's alternativ_col_ram) / SABL (line override) - each valid
    // for exactly the one instruction right after it, per op_argument()'s "valid for 1 step" note.
    uint8_t alternativ_col_ram;
    uint8_t alternativ_line_ram;

    uint8_t f_output; // ATF target - not consumed by rendering, exposed via debug_f_output() only
    uint8_t s_output; // ATS target - same deal, no known consumer for these games' S pin

    // LAX on SM530 only latches ACC if its high nibble differs from the PREVIOUS
    // opcode's high nibble (real hardware quirk - see op_lax()'s comment). Updated
    // at the end of every execute_curr_opcode() call, so this is deliberately kept
    // out of the shared g_op_lax()/SM5A-SM511 lookahead-skip scheme, which checks
    // the wrong opcode range (0x20) for SM530's own LAX encoding (0x10-0x1F) and
    // would otherwise silently eat any real 0x20-0x2F instruction right after a LAX.
    uint8_t prev_opcode;

    // Melody (duplicated from SM511_2 rather than inherited - see header comment on sibling cores)
    bool me_melody_activate;
    bool mes_melody_finish;
    uint16_t melody_cycle_count;
    uint16_t nb_cycle_need_for_change_note;
    struct { uint8_t note; bool double_time; bool octave; } curr_note_melody;
    uint8_t curr_phase;
    uint8_t cycle_in_curr_phase;

/// ##### FUNCTION ################################################# ///
private:
    void update_segment() override;
    void update_sound() override;

    bool no_pc_increase(uint8_t opcode) override;
    bool is_on_double_octet(uint8_t opcode) override;

    void execute_curr_opcode() override;

    bool condition_to_update_segment() override;

    void wake_up() override;

    uint8_t read_rom_value() override;
    uint8_t read_ram_value() override;
    void write_ram_value(uint8_t value) override;
    void set_ram_value(uint8_t col, uint8_t line, uint8_t value) override;

private:
    uint8_t read_rom_melody_value();
    void load_new_note(uint8_t* update_melody_adress = nullptr);

    // -- from SM530_instruction.cpp ------------------------------ //
    // Opcodes MAME overrides on SM530 (everything else reuses SM5XX's
    // shared g_op_* handlers directly from execute_curr_opcode()).

    void op_lax(); // per-core, not shared g_op_lax() - see prev_opcode's doc comment
    void op_lb();
    void op_sbm();  // SABM: one-shot column override, mirrors SM511_2's op_sbm/alternativ_col_ram
    void op_sabl(); // SABL: one-shot line override - new vs SM511, needs its own field
    void op_incb();
    void op_decb(); // per-core on this engine even where MAME shares it (see SM511_2 precedent)

    void op_t();  // "T" (TR): jump within the current page
    void op_tl();
    void op_trs();
    void op_atpl();
    void op_rtn0();
    void op_rtn1();

    void op_dta();
    void op_adx();

    void op_tg();

    void op_kta();
    void op_keta();
    void op_ats();
    void op_atf();
    void op_atbp();
    void op_sds();
    void op_rds();

    void op_idiv();
    void op_inis();

    void op_cend();

    void op_tmel();
    void op_rme();
    void op_sme();
    void op_pre();

public : // debug
    uint8_t debug_get_elem_rom(int col, int line, int word) { return rom[col][line][word]; }
    int debug_rom_adress_size_col() override { return SM530_ROM_COL; }
    int debug_rom_adress_size_line() override { return SM530_ROM_LINE; }

    uint8_t debug_get_elem_ram(int col, int line) override;
    int debug_ram_adress_size_col() override { return SM530_RAM_COL; }
    int debug_ram_adress_size_line() override { return SM530_RAM_LINE; }

    uint8_t debug_f_output() { return f_output; }
    uint8_t debug_gamma_sm530() { return gamma_sm530; }
    bool debug_is_sleep() { return is_sleep; }
    bool debug_ds() { return ds; }
};
