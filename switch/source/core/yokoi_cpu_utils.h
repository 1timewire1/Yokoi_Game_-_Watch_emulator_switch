#pragma once

#include <cstdint>
#include <memory>

class SM5XX;

void yokoi_cpu_set_time_if_needed(SM5XX* cpu);

bool yokoi_cpu_create_instance(std::unique_ptr<SM5XX>& out, const uint8_t* rom, uint16_t size_rom);
