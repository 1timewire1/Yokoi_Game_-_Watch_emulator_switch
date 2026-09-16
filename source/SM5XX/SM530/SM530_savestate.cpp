#include "SM530.h"
#include <stdio.h>
#include <string.h>

bool SM530::save_state(FILE* file) {
    if(!file) return false;

    fwrite(&program_counter, sizeof(ProgramCounter), 1, file);
    fwrite(&s_buffer_program_counter, sizeof(ProgramCounter), 1, file);
    fwrite(&ram_address, sizeof(RAMAddress), 1, file);
    fwrite(&accumulator, sizeof(uint8_t), 1, file);
    fwrite(&carry, sizeof(bool), 1, file);

    fwrite(ram, sizeof(uint8_t), SM530_RAM_COL * SM530_RAM_LINE, file);

    fwrite(&f_clock_divider, sizeof(uint16_t), 1, file);
    fwrite(&gamma_flag_second, sizeof(bool), 1, file);
    fwrite(&gamma_sm530, sizeof(uint8_t), 1, file);
    fwrite(&subdiv, sizeof(uint16_t), 1, file);
    fwrite(&count_1s, sizeof(uint8_t), 1, file);
    fwrite(&count_10ms, sizeof(uint8_t), 1, file);

    fwrite(&ds, sizeof(bool), 1, file);
    fwrite(&bp, sizeof(uint8_t), 1, file);

    fwrite(&rom_melody_address, sizeof(uint8_t), 1, file);
    fwrite(&me_melody_activate, sizeof(bool), 1, file);

    fwrite(&prev_opcode, sizeof(uint8_t), 1, file);

    return true;
}

bool SM530::load_state(FILE* file) {
    if(!file) return false;

    if(fread(&program_counter, sizeof(ProgramCounter), 1, file) != 1) return false;
    if(fread(&s_buffer_program_counter, sizeof(ProgramCounter), 1, file) != 1) return false;
    if(fread(&ram_address, sizeof(RAMAddress), 1, file) != 1) return false;
    if(fread(&accumulator, sizeof(uint8_t), 1, file) != 1) return false;
    if(fread(&carry, sizeof(bool), 1, file) != 1) return false;

    if(fread(ram, sizeof(uint8_t), SM530_RAM_COL * SM530_RAM_LINE, file) != SM530_RAM_COL * SM530_RAM_LINE) return false;

    if(fread(&f_clock_divider, sizeof(uint16_t), 1, file) != 1) return false;
    if(fread(&gamma_flag_second, sizeof(bool), 1, file) != 1) return false;
    if(fread(&gamma_sm530, sizeof(uint8_t), 1, file) != 1) return false;
    if(fread(&subdiv, sizeof(uint16_t), 1, file) != 1) return false;
    if(fread(&count_1s, sizeof(uint8_t), 1, file) != 1) return false;
    if(fread(&count_10ms, sizeof(uint8_t), 1, file) != 1) return false;

    if(fread(&ds, sizeof(bool), 1, file) != 1) return false;
    if(fread(&bp, sizeof(uint8_t), 1, file) != 1) return false;

    if(fread(&rom_melody_address, sizeof(uint8_t), 1, file) != 1) return false;
    if(fread(&me_melody_activate, sizeof(bool), 1, file) != 1) return false;

    if(fread(&prev_opcode, sizeof(uint8_t), 1, file) != 1) return false;

    update_segment();
    segments_state_are_update = true;

    return true;
}
