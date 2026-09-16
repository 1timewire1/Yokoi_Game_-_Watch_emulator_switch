#include "SM5XX/SM530/SM530.h"
#include <cstring>


namespace {
    // SM530's PC is a flat 11-bit value (5-bit page + 6-bit word) in MAME/real
    // hardware; this engine represents it as {col,line,word} like every other
    // core, so page = col*16+line (col 0-1, line 0-15, matching PRGMASK 0x7FF
    // = 32 pages). These two helpers are the single point doing that
    // conversion, used everywhere SM530 needs to treat the PC as one number
    // (do_branch, TRS's vector fetch, ATPL) - see SM530.h's header comment.
    inline uint16_t pc_to_flat(const ProgramCounter& pc) {
        return (uint16_t)(((pc.col << 4) | pc.line) << 6) | (pc.word & 0x3F);
    }
    inline ProgramCounter flat_to_pc(uint16_t flat) {
        flat &= 0x7FF;
        ProgramCounter pc;
        pc.word = flat & 0x3F;
        uint8_t page = (flat >> 6) & 0x1F;
        pc.col = page >> 4;
        pc.line = page & 0xF;
        return pc;
    }
}


void SM530::init()
{
    program_counter = flat_to_pc(((uint16_t)0xF << 6)); // reset_vector: do_branch(0xf, 0)
    s_buffer_program_counter = {0, 0, 0}; // 1-level stack, doubles as the RTN/RTNS save slot

    for(int col = 0; col < SM530_RAM_COL; ++col){
        for(int line = 0; line < SM530_RAM_LINE; ++line){ ram[col][line] = 0; } }
    ram_address = {0, 0};

    for(int i = 0; i < 8; i++){ k_input[i] = 0x00; }
    k_input_sp_not_multiplex = 0x00; // this is the real K register on SM530 - see op_kta/op_keta

    beta_input = true;
    alpha_input = true;

    carry = 0;
    accumulator = 0;

    f_clock_divider = 0x0000;
    gamma_flag_second = false; // kept in sync with gamma_sm530 purely for the inherited condition_to_wake_up()

    me_melody_activate = true;
    mes_melody_finish = false;
    nb_cycle_need_for_change_note = (uint16_t)(SM530_TIME_CHANGE_NOTE * FREQUENCY_CPU / 1'000'000);
    uint8_t init_add = 0x00;
    load_new_note(&init_add);

    bp_lcd_blackplate = true; // dead code for rendering, kept only so screen_is_on() has something to return
    bp = 0;
    ds = true; // "assume LCD is on" - matches sm530_device::device_reset()

    gamma_sm530 = 0;
    subdiv = 0;
    count_1s = 0;
    count_10ms = 0;

    alternativ_col_ram = 0x00;
    alternativ_line_ram = 0x00;

    f_output = 0;
    s_output = 0;

    prev_opcode = 0;

    is_sleep = false;

    segments_state_are_update = false;
    cycle_curr_opcode = 0;

    for(int o = 0; o < SM530_LCD_GROUPS; o++){
        for(int y = 0; y < SM530_LCD_SEGS; y++){ segment_on[o][y] = 0x00; }
    }
    flag_time_update_screen = 0x00;

    cpu_frequency_divider = 1; // SM530 doesn't have SM511's CLKLO/CLKHI opcodes - always full rate

    stop_cpu = false;
}


bool SM530::no_pc_increase(uint8_t opcode){
    // NOTE: TRS (0xC0-0xFF) is deliberately NOT here, unlike T. op_trs() saves the
    // current program_counter as the subroutine's return address - it needs that to
    // already be the POST-advance (next-instruction) value, same as any call
    // instruction elsewhere in this engine (e.g. SM511_2's op_tm()/op_tml(), which
    // call get_parameter_of_opcode() - advancing PC - before saving). Suppressing
    // the advance here made every TRS push its own address as the return address,
    // so RTN0 jumped straight back into the same TRS forever (infinite call loop,
    // caught via a standalone trace harness showing PC stuck cycling through 9
    // addresses with zero RAM writes across 2,000,000+ steps).
    return
           ((opcode & 0xF0) == 0x80) || ((opcode & 0xF0) == 0x90) // T (0x80-0xB0, matches op_t's dispatch range)
        || ((opcode & 0xF0) == 0xA0) || ((opcode & 0xF0) == 0xB0)
        || (opcode == 0x6A)           // ATPL - same reasoning as SM511's own ATPL: op_atpl() reads
                                       // program_counter.word assuming it's still the pre-fetch value
        || (opcode == 0x68) || (opcode == 0x69) // RTN0/RTN1 - overwritten by the pop anyway
    ;
}

bool SM530::is_on_double_octet(uint8_t opcode){
    // LBL, PRE, TL - matches sm530_device::op_argument() exactly
    return (opcode == 0x6B) // LBL
        || (opcode == 0x78) // PRE
        || ((opcode & 0xF8) == 0x60) // TL (0x60-0x67)
    ;
}


/////////////////////////// RAM / ROM MANIPULATION //////////////////////////////////////////////////////

void SM530::load_rom(const uint8_t* file_hex, size_t){
    // Raw 2048-byte dump, 32 pages x 64 words, no MAME-dump padding quirk to work around.
    for(int col = 0; col < SM530_ROM_COL; col++){
        for(int line = 0; line < SM530_ROM_LINE; line++){
            for(int word = 0; word < SM530_ROM_WORD; word++){
                rom[col][line][word] = file_hex[(SM530_ROM_LINE * col + line) * SM530_ROM_WORD + word];
            }
        }
    }
}

void SM530::load_rom_melody(const uint8_t* file_hex, size_t){
    for(int word = 0; word < SM530_MELODY_ROM_SIZE; word++){ rom_melody[word] = file_hex[word]; }
}

uint8_t SM530::read_rom_value(){
    return rom[program_counter.col][program_counter.line][program_counter.word];
}
uint8_t SM530::read_rom_melody_value(){
    return rom_melody[rom_melody_address];
}

uint8_t SM530::read_ram_value(){
    uint8_t col = ram_address.col;
    if(alternativ_col_ram != 0x00){ col = alternativ_col_ram; } // SABM, valid for 1 step
    uint8_t line = ram_address.line;
    if(alternativ_line_ram != 0x00){ line = alternativ_line_ram; } // SABL, valid for 1 step

    col = min(col, (uint8_t)(SM530_RAM_COL-1));
    line = min(line, (uint8_t)(SM530_RAM_LINE-1));
    return ram[col][line];
}

void SM530::write_ram_value(uint8_t value){
    uint8_t col = ram_address.col;
    if(alternativ_col_ram != 0x00){ col = alternativ_col_ram; }
    uint8_t line = ram_address.line;
    if(alternativ_line_ram != 0x00){ line = alternativ_line_ram; }

    col = min(col, (uint8_t)(SM530_RAM_COL-1));
    line = min(line, (uint8_t)(SM530_RAM_LINE-1));
    ram[col][line] = value & 0x0F;
}

void SM530::set_ram_value(uint8_t col, uint8_t line, uint8_t value) {
    if (col >= SM530_RAM_COL || line >= SM530_RAM_LINE)
        return;
    ram[col][line] = value;
}

/////////////////////////// Wake up //////////////////////////////////////////////////////

void SM530::wake_up(){
    is_sleep = false;
    program_counter = flat_to_pc(0); // wakeup_vector: do_branch(0, 0)
    gamma_flag_second = false; // see this field's note in init() - only meant as a one-shot wake trigger here
}


/////////////////////////// Segment / screen //////////////////////////////////////////////////////

bool SM530::condition_to_update_segment(){
    // SM530 has no SM511-style periodic-divider display refresh; its LCD is a
    // direct read of RAM gated only by the ds latch (see sm530_device::lcd_update(),
    // which has no divider/timing gate at all - it's driven straight from
    // clock_div()'s per-cycle call chain, i.e. effectively "always current").
    // Refreshing every cycle would be wasteful for this engine's polling model,
    // so this mirrors SM511_2's own "simplify version for perf" compromise
    // instead: refresh whenever the low word bits change, which in practice is
    // every single step anyway since SM530 has no low-power CLKLO mode.
    return true;
}

bool SM530::screen_is_on() { return ds; } // matches SM511_2's screen_is_on() shape; dead code for real rendering either way

void SM530::update_segment(){
    // LCD RAM: 0x40-0x4B (12 cells, h=0/row A) and 0x50-0x5B (12 cells, h=1/row B),
    // each cell mapped through ram[col][line] via col=addr>>4,line=addr&0xF (0x40=(4,0)..0x4B=(4,11),
    // 0x50=(5,0)..0x5B=(5,11)). Segment tags in these games' SVGs are titled "o.y.h" - see
    // get_segments_state()'s comment for why the (col,line,word) parameter order there is (o,y,h).
    for(int o = 0; o < SM530_LCD_GROUPS; o++){
        uint8_t row_a = ds ? ram[4][o] : 0;
        uint8_t row_b = ds ? ram[5][o] : 0;
        for(int y = 0; y < SM530_LCD_SEGS; y++){
            uint8_t h0 = (row_a >> y) & 0x01;
            uint8_t h1 = (row_b >> y) & 0x01;
            segment_on[o][y] = h0 | (h1 << 1);
        }
    }
    segments_state_are_update = true;
}

bool SM530::get_segments_state(uint8_t col, uint8_t line, uint8_t word) {
    // (col,line,word) here means (o,y,h) - see update_segment()'s comment.
    if(col >= SM530_LCD_GROUPS || line >= SM530_LCD_SEGS || word >= SM530_LCD_ROWS){ return false; }
    return (segment_on[col][line] >> word) & 0x01;
}


/////////////////////////// Melody system //////////////////////////////////////////////////////
// Duplicated from SM511_2 rather than inherited (this engine keeps concrete CPU classes as
// independent siblings of SM5XX, not a MAME-style inheritance chain) - same TONE table and
// stop-byte convention, just a doubled per-note cycle count for SM530's 0xff step mask
// (128 cycles/step on SM511 vs 256 on SM530, same 16/32-step-per-note count either way).

void SM530::load_new_note(uint8_t* update_melody_adress){
    melody_cycle_count = 0;
    curr_phase = 0;
    cycle_in_curr_phase = 0;

    if(update_melody_adress == nullptr){ rom_melody_address += 1; }
    else { rom_melody_address = *update_melody_adress; }

    uint8_t data = read_rom_melody_value();
    curr_note_melody.note = (data & 0x0F);
    curr_note_melody.double_time = ((data & 0x20) == 0x20);
    curr_note_melody.octave = ((data & 0x10) == 0x10);
}

void SM530::update_sound(){
    // Gamma flags + 1s/1/100s counters: mirrors sm530_device::div_timer_cb() exactly, just
    // reading f_clock_divider (already incremented by the shared, non-virtual
    // SM5XX::step_clock_divider() that runs immediately before this in execute_cycle() -
    // same 15-bit/0x7FFF wrap MAME's m_div uses) instead of keeping a duplicate divider.
    // update_sound() is virtual and called every cycle at the same rate step_clock_divider()
    // itself runs, which is the only reason this lives here despite the name.
    if(f_clock_divider == 0x0000){
        gamma_sm530 |= 0x02; // 1s
        count_1s = (count_1s + 1) % 10;
        if(count_1s == 0){ gamma_sm530 |= 0x01; } // 10s
    }
    if((f_clock_divider & 0x3FFF) == 0){ gamma_sm530 |= 0x04; } // 0.5s
    if((f_clock_divider & 0xFF) < 250){
        subdiv = (subdiv + 1) % 32000;
        if((subdiv % 320) == 0){
            count_10ms = (count_10ms + 1) % 10;
            if(count_10ms == 0){ gamma_sm530 |= 0x08; } // 0.1s
        }
    }
    if(gamma_sm530 != 0){ gamma_flag_second = true; } // see this field's note in wake_up()/init()

    // See SM511_2::update_sound()'s comment: real hardware keeps stepping the melody ROM
    // and detecting the stop byte regardless of ME state - only the R-pin output is gated
    // by it (get_active_sound() below), not this stepping/stop-detection logic.
    static const uint8_t note_frequency_control[4*12]{
            7,  8,  8,  8
            , 8,  8,  8,  8
            , 8,  9,  9,  9
            , 9,  9,  9, 10
            , 9, 10, 10, 10
            ,10, 11, 10, 11
            ,11, 11, 11, 11
            ,11, 12, 12, 12
            ,12, 13, 12, 13
            ,13, 13, 13, 14
            ,14, 14, 14, 14
            ,14, 15, 15, 15
        };

    if( (curr_note_melody.note) == 0x00) {
        curr_phase = 0x00;
    }
    else if( (curr_note_melody.note) == 0x01) {
        curr_phase = 0x00;
        mes_melody_finish = true;
    }
    else if((curr_note_melody.note) <= 13)
    {
        int ind_note_freq = (curr_note_melody.note-2)*4 + curr_phase;
        if( cycle_in_curr_phase >= (note_frequency_control[ind_note_freq] * (curr_note_melody.octave? 2: 1))){
            cycle_in_curr_phase = 0;
            curr_phase = (curr_phase+1) & 0x03;
        }
        cycle_in_curr_phase += 1;
    }

    melody_cycle_count += 1;
    if( ( (!curr_note_melody.double_time)
            && (melody_cycle_count >= nb_cycle_need_for_change_note))
        || (melody_cycle_count >= 2*nb_cycle_need_for_change_note))
    {
        load_new_note();
    }
}

bool SM530::get_active_sound(){
    return me_melody_activate && ((curr_phase&0x01) == 0x01);
}


/////////////////////////// Debug //////////////////////////////////////////////////////
uint8_t SM530::debug_get_elem_ram(int col, int line) {
    uint8_t col_ = min(col, SM530_RAM_COL-1);
    uint8_t line_ = min(line, SM530_RAM_LINE-1);
    return ram[col_][line_];
}
