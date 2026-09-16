#include "SM5XX/SM530/SM530.h"

namespace {
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
    // do_branch(pu, pl): pu is the raw upper-address value as opcodes compute it
    // (may exceed 5 bits before the final &PRGMASK truncation - see op_trs/op_tl,
    // which is why this takes the full flat-address route rather than assuming
    // pu is already a clean 0-31 page number).
    inline ProgramCounter branch_to(uint8_t pu, uint8_t pl) {
        return flat_to_pc(((uint16_t)pu << 6) | (pl & 0x3F));
    }
}

// SM530-specific opcode dispatch. Ported from MAME's sm530_device::execute_one()
// (src/devices/cpu/sm510/sm530.cpp) - see SM530.h's header comment for the full
// list of what differs from SM511. Opcodes not listed here explicitly (COMA, ADD,
// ADDC, RC, SC, TABL, TAM, EXBL, TC, LAX, LDA, EXC, RM, SM, TM/TMI, LBL) reuse the
// shared g_op_* handlers directly - matching MAME's own choice to leave those
// un-overridden on sm530_device (inherited straight from sm510_base_device).
void SM530::execute_curr_opcode() {
    switch (curr_opcode & 0xf0)
    {
        case 0x00: op_adx(); break;
        case 0x10: op_lax(); break;
        case 0x30: op_lb(); break;

        case 0x80: case 0x90: case 0xa0: case 0xb0:
            op_t(); break; // "T" (TR): jump within the current page - see op_t()'s own definition below
        case 0xc0: case 0xd0: case 0xe0: case 0xf0:
            op_trs(); break;

        default:
            switch (curr_opcode & 0xfc)
            {
                case 0x20: g_op_lda(); break;
                case 0x24: g_op_exc(); break;
                case 0x28: g_op_exc(); op_incb(); break; // EXCI
                case 0x2c: g_op_exc(); op_decb(); break; // EXCD
                case 0x40: g_op_rm(); break;
                case 0x44: g_op_sm(); break;
                case 0x48: g_op_tmi(); break; // TM
                case 0x60: case 0x64: op_tl(); break;
                case 0x6c: op_tg(); break;

                default:
                    switch (curr_opcode)
                    {
                        case 0x4c: op_incb(); break;
                        case 0x4d: op_decb(); break;
                        case 0x4e: op_rds(); break;
                        case 0x4f: op_sds(); break;

                        case 0x50: op_kta(); break;
                        case 0x51: op_keta(); break;
                        case 0x52: op_dta(); break;
                        case 0x53: g_op_coma(); break;
                        case 0x54: g_op_add(); break;
                        case 0x55: g_op_add11(); break;
                        case 0x56: g_op_rc(); break;
                        case 0x57: g_op_sc(); break;
                        case 0x58: g_op_tabl(); break;
                        case 0x59: g_op_tam(); break;
                        case 0x5a: g_op_exbla(); break;
                        case 0x5b: g_op_tc(); break;
                        case 0x5c: op_ats(); break;
                        case 0x5d: op_atf(); break;
                        case 0x5e: op_atbp(); break;

                        case 0x68: op_rtn0(); break;
                        case 0x69: op_rtn1(); break;
                        case 0x6a: op_atpl(); break;
                        case 0x6b: g_op_lbl(); break;

                        case 0x70: op_idiv(); break;
                        case 0x71: op_inis(); break;
                        case 0x72: op_sbm(); break;
                        case 0x73: op_sabl(); break;
                        case 0x74: op_cend(); break;
                        case 0x75: op_tmel(); break;
                        case 0x76: op_rme(); break;
                        case 0x77: op_sme(); break;
                        case 0x78: op_pre(); break;
                        case 0x79: g_op_tb(); break; // TBA reads BA pin - shares g_op_tb's beta_input shape

                        default: g_op_illegal(); break;
                    }
                    break; // 0xff

            }
            break; // 0xfc
    } // 0xf0

    // SABM/SABL are each valid for exactly one following step.
    if( (alternativ_col_ram != 0x00) && (curr_opcode != 0x72) ){ alternativ_col_ram = 0x00; }
    if( (alternativ_line_ram != 0x00) && (curr_opcode != 0x73) ){ alternativ_line_ram = 0x00; }

    prev_opcode = curr_opcode; // for op_lax()'s "differs from previous opcode" check
}


// ----- LAX x: unlike the shared g_op_lax() (SM5A/SM510/SM511's lookahead-skip scheme,
// which only keeps the LAST of a run of consecutive LAX and checks opcode range 0x20 -
// wrong for SM530's own 0x10-0x1F LAX encoding), SM530 latches ACC only when this LAX's
// high nibble differs from the PREVIOUS instruction's high nibble (ported from
// reference/segment-main/sm510/sm530.lua's op_lax(): "if (op & ~0xf) ~= (prev_op & ~0xf)").
// Using the shared handler here was a bug: its hardcoded 0x20 check meant it would
// wrongly treat any real 0x20-0x2F instruction (LDA/EXC/EXCI/EXCD) right after a LAX as
// "another LAX to collapse" and silently skip it - e.g. it ate a real EXC in nstarfox's
// boot code, which is why the core never wrote to RAM.
void SM530::op_lax(){
    if((curr_opcode & 0xF0) != (prev_opcode & 0xF0)){
        accumulator = curr_opcode & 0x0F;
    }
}


// ---- "T" (TR): jump within the current page, low 6 bits of the opcode are the target word.
// Not a shared g_op_* (SM5A/SM510/SM511 each encode this slightly differently already), so this
// engine's convention of a per-core private method applies here the same way.
void SM530::op_t(){
    program_counter.word = curr_opcode & 0x3F;
}


// ----- ROM address instructions

void SM530::op_tl(){
    // TL xy: long jump. param's high 2 bits are folded into pu along with the opcode itself.
    uint8_t param = get_parameter_of_opcode();
    uint8_t pu = (curr_opcode << 2) | ((param >> 6) & 3);
    program_counter = branch_to(pu, param & 0x3f);
}

void SM530::op_trs(){
    // TRS x: indirect call through a vector on page 14. Only 1 stack level - reuses the
    // inherited s_buffer_program_counter as that single save slot (see init()'s comment).
    copy_buffer(program_counter, s_buffer_program_counter);

    ProgramCounter vector_addr = flat_to_pc(((uint16_t)14 << 6) | (curr_opcode & 0x3f));
    uint8_t jump = rom[vector_addr.col][vector_addr.line][vector_addr.word];
    uint8_t pu = ((jump >> 5 & 1) << 2) | ((jump >> 7 & 1) << 1) | (jump >> 6 & 1); // bitswap<3>(jump,5,7,6)
    program_counter = branch_to(pu, jump & 0x1f);
}

void SM530::op_atpl(){
    // Low jump: only the accumulator-sized low nibble of the current word changes (see
    // no_pc_increase(0x6a) - program_counter.word here is still the pre-fetch value, same
    // trick SM511_2's own op_atpl relies on).
    program_counter.word = (program_counter.word & 0x30) | (accumulator & 0x0F);
}

void SM530::op_rtn0(){
    copy_buffer(s_buffer_program_counter, program_counter);
}

void SM530::op_rtn1(){
    op_rtn0();
    skip_instruction();
}


// ----- RAM address instructions

void SM530::op_lb(){
    // LB x: SM530's own partial-load pattern (bits 1,2 of BL always forced on) - see
    // SM530.h's header comment; different from every other core's LB.
    ram_address.line = ((curr_opcode << 2) & 8) | (curr_opcode & 1) | 6;
    ram_address.col = (curr_opcode >> 2) & 3;
}

void SM530::op_sbm(){
    // SABM: set BM high bit for the next instruction only.
    alternativ_col_ram = ram_address.col | 0x04;
}

void SM530::op_sabl(){
    // SABL: set BL high bit for the next instruction only - new vs SM511, which has no
    // equivalent (its RAM only needed a column override, never a line one).
    alternativ_line_ram = ram_address.line | 0x08;
}

void SM530::op_incb(){
    // Overflows (and skips) on the 3rd bit, not the 4th - real SM530 quirk, see MAME's
    // op_incb() comment ("increment BL, but overflow on 3rd bit!").
    ram_address.line = (ram_address.line + 1) & 0x0F;
    if((ram_address.line & 0x07) == 0){ skip_instruction(); }
}

void SM530::op_decb(){
    if(ram_address.line == 0){
        skip_instruction();
        ram_address.line = SM530_RAM_LINE - 1;
    }
    else { ram_address.line = ram_address.line - 1; }
}


// ----- Data transfer instructions

void SM530::op_dta(){
    // DTA: transfer the 1/100s hardware counter to ACC - not "1 second passed" like SM511's DTA.
    accumulator = count_10ms;
}


// ----- Arithmetic instructions

void SM530::op_adx(){
    // ADX x: unlike the shared g_op_adx, SM530 always skips on carry - no 0x0A exception.
    accumulator = accumulator + (curr_opcode & 0x0F);
    if(accumulator > 0x0F){ skip_instruction(); }
    accumulator = accumulator & 0x0F;
}


// ----- Test instructions

void SM530::op_tg(){
    // TG x: skip next if gamma flag bit x is set, then clear that bit. bit0=10s, bit1=1s,
    // bit2=0.5s, bit3=0.1s - see the divider callback in SM530_savestate.cpp's neighbor,
    // step_clock_divider's SM530 override (declared here, defined alongside update_sound).
    uint8_t mask = 1 << (curr_opcode & 0x03);
    if(gamma_sm530 & mask){ skip_instruction(); }
    gamma_sm530 &= ~mask;
}


// ----- I/O instructions

void SM530::op_kta(){
    // KTA: input K (low nibble) to ACC. K is a single non-multiplexed 8-bit input on SM530,
    // not 8 multiplexed 4-bit groups - see k_input_sp_not_multiplex's doc comment in SM5XX.h.
    accumulator = k_input_sp_not_multiplex & 0x0F;
}

void SM530::op_keta(){
    // KETA: input K (high nibble, "KE") to ACC.
    accumulator = (k_input_sp_not_multiplex >> 4) & 0x0F;
}

void SM530::op_ats(){ s_output = accumulator; } // no known consumer for these games - see SM530.h
void SM530::op_atf(){ f_output = accumulator; } // ditto

void SM530::op_atbp(){ bp = accumulator; } // matches SM511_2::op_atbp's shape; also dead code for rendering

void SM530::op_sds(){ ds = true; }  // SDS: set display enable
void SM530::op_rds(){ ds = false; } // RDS: reset display enable


// ----- Special instructions

void SM530::op_idiv(){
    // IDIV: reset divider AND the 1s counter (SM511's IDIV only resets f_clock_divider).
    f_clock_divider = 0x0000;
    subdiv = 0;
    count_1s = 0;
}

void SM530::op_inis(){
    // INIS: reset the 1/100s counter only - no SM511 equivalent at all.
    count_10ms = 0;
}

void SM530::op_cend(){ is_sleep = true; } // same meaning as g_op_cend, just needs to be SM530's own
                                           // method since it's dispatched alongside SM530-only opcodes


// ----- Melody control (duplicated from SM511_2 - see SM530.cpp's melody section comment)

void SM530::op_tmel(){
    if(mes_melody_finish){
        skip_instruction();
        mes_melody_finish = false;
    }
}

void SM530::op_rme(){ me_melody_activate = false; }
void SM530::op_sme(){ me_melody_activate = true; }

void SM530::op_pre(){
    uint8_t param = get_parameter_of_opcode();
    load_new_note(&param);
}
