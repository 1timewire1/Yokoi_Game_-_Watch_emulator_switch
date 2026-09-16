#include "yokoi_cpu_utils.h"

#include <time.h>

#include "SM5XX/SM5XX.h"
#include "SM5XX/SM5A/SM5A.h"
#include "SM5XX/SM510/SM510.h"
#include "SM5XX/SM511_SM512/SM511_2.h"
#include "SM5XX/SM530/SM530.h"
#include "David_and_John/David_And_John_fake_cpu.h"

void yokoi_cpu_set_time_if_needed(SM5XX* cpu) {
    if (!cpu || cpu->is_time_set()) {
        return;
    }

    time_t now = time(nullptr);
    tm* t = localtime(&now);
    if (!t) {
        return;
    }

    cpu->set_time((uint8_t)t->tm_hour, (uint8_t)t->tm_min, (uint8_t)t->tm_sec);
    cpu->time_set(true);
}

bool yokoi_cpu_create_instance(std::unique_ptr<SM5XX>& out, const uint8_t* rom, uint16_t size_rom, bool has_melody_rom) {
    if (!rom || size_rom == 0) {
        return false;
    }

    if (size_rom == 4) { // No real CPU - "David and John" original/open-source games
        if (rom[0] == 0xFF && rom[1] == 0xFF && rom[2] == 0xFF) {
            out = std::make_unique<David_And_John_fake_cpu>();
            return true;
        }
    }
    if (size_rom == 1856) {
        out = std::make_unique<SM5A>();
        return true;
    }
    if (size_rom == 2048) { // 32 pages x 64 words - only SM530 dumps come in this size
        out = std::make_unique<SM530>();
        return true;
    }
    if (size_rom == 4096) {
        // Only SM511/SM512 support a melody ROM (SM510::load_rom_melody() is a no-op),
        // so a game shipping one can never actually be SM510 - trust that over the byte
        // heuristic below, which found a false negative for Blades of Steel (kblades):
        // its program ROM happens to have all-zero bytes at 704-719 despite being a real
        // SM511 game, so the heuristic picked SM510 and the wrong core silently corrupted
        // execution (froze at boot, every segment lit).
        if (has_melody_rom) {
            out = std::make_unique<SM511_2>();
            return true;
        }
        // Heuristic from 3DS main.cpp
        for (int i = 0; i < 16; i++) {
            if (rom[i + 704] != 0x00) { // Rom from sm510 has 63 word + 1 empty word per line/pages, sm511/12 has 64 word
                out = std::make_unique<SM511_2>();
                return true;
            }
        }
        out = std::make_unique<SM510>();
        return true;
    }
    return false;
}
