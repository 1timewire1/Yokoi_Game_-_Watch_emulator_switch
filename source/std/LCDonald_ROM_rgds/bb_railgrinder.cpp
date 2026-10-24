
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_railgrinder.h"


const uint8_t rom_GW_bb_railgrinder[] = {
	0xFF, 0xFF, 0xFE, 0x1F
}; const size_t size_rom_GW_bb_railgrinder = sizeof(rom_GW_bb_railgrinder)/sizeof(rom_GW_bb_railgrinder[0]);

const uint8_t melody_GW_bb_railgrinder[1] = {0}; 
	const size_t size_melody_GW_bb_railgrinder = 0;

const std::string path_segment_bb_railgrinder = "gfx/segment_bb_railgrinder.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_railgrinder[] = {
	{ { 0,0,0 }, { 196,28 }, { 1,305 }, { 162,234 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 555,49 }, { 1,10 }, { 44,50 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 593,49 }, { 273,8 }, { 45,50 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 632,49 }, { 320,8 }, { 45,50 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 36,96 }, { 241,332 }, { 183,232 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 29,247 }, { 1,147 }, { 188,156 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 209,291 }, { 190,21 }, { 81,124 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 287,309 }, { 191,186 }, { 81,124 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 361,330 }, { 273,60 }, { 81,124 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 37,343 }, { 1,541 }, { 238,188 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 435,349 }, { 426,439 }, { 81,125 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 510,368 }, { 274,186 }, { 81,124 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 42,310 }, { 241,566 }, { 268,163 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 586,389 }, { 426,312 }, { 81,125 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 661,408 }, { 357,236 }, { 59,94 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 324,101 }, { 1,731 }, { 354,292 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 360,48 }, { 1,62 }, { 187,83 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_railgrinder = sizeof(segment_GW_bb_railgrinder)/sizeof(segment_GW_bb_railgrinder[0]); 
 const uint16_t segment_info_bb_railgrinder[] = { 512, 1024, 1, 0, 720, 558}; 

const std::string path_background_bb_railgrinder = "gfx/background_bb_railgrinder.png";
const uint16_t background_info_bb_railgrinder[] = { 1024, 1024, 1, 465, 720, 558, 1, 0, 0 }; 


const std::string path_console_bb_railgrinder = "gfx/console_bb_railgrinder.png";
const uint16_t console_info_bb_railgrinder[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom bb_railgrinder (
    "Vibes Rail Grinder", "BB_RAILGRINDER", "2025-10-20"
    , rom_GW_bb_railgrinder, size_rom_GW_bb_railgrinder
    , melody_GW_bb_railgrinder, size_melody_GW_bb_railgrinder
    , path_segment_bb_railgrinder
    , segment_GW_bb_railgrinder, size_segment_GW_bb_railgrinder
    , segment_info_bb_railgrinder
    , path_background_bb_railgrinder
    , background_info_bb_railgrinder
    , path_console_bb_railgrinder
    , console_info_bb_railgrinder
    , GW_rom::MANUFACTURER_BURGER_BARD
);

