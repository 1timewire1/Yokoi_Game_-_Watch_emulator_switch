
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_platform2.h"


const uint8_t rom_GW_bb_platform2[] = {
	0xFF, 0xFF, 0xFE, 0x1D
}; const size_t size_rom_GW_bb_platform2 = sizeof(rom_GW_bb_platform2)/sizeof(rom_GW_bb_platform2[0]);

const uint8_t melody_GW_bb_platform2[1] = {0}; 
	const size_t size_melody_GW_bb_platform2 = 0;

const std::string path_segment_bb_platform2 = "gfx/segment_bb_platform2.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_platform2[] = {
	{ { 0,0,0 }, { 474,119 }, { 474,1943 }, { 35,50 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 511,118 }, { 474,1995 }, { 36,52 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 549,119 }, { 474,1891 }, { 35,50 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 39,15 }, { 1,1103 }, { 164,161 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 204,30 }, { 1,1017 }, { 106,84 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 373,30 }, { 1,931 }, { 105,84 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 568,30 }, { 109,1015 }, { 106,84 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 5,177 }, { 288,1878 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 182,178 }, { 288,1707 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 358,178 }, { 1,1266 }, { 172,169 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 534,178 }, { 1,1608 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 16,186 }, { 351,1554 }, { 119,151 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 195,207 }, { 351,1270 }, { 121,128 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 6,352 }, { 176,1536 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 182,354 }, { 1,1437 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 356,356 }, { 175,1195 }, { 173,168 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 534,355 }, { 176,1365 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 23,359 }, { 351,1400 }, { 108,152 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 196,383 }, { 350,1139 }, { 120,129 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 211,126 }, { 167,1101 }, { 258,36 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 380,242 }, { 1,1779 }, { 285,268 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_platform2 = sizeof(segment_GW_bb_platform2)/sizeof(segment_GW_bb_platform2[0]); 
 const uint16_t segment_info_bb_platform2[] = { 512, 2048, 1, 0, 719, 553}; 

const std::string path_background_bb_platform2 = "gfx/background_bb_platform2.png";
const uint16_t background_info_bb_platform2[] = { 1024, 1024, 1, 470, 719, 553, 1, 0, 0 }; 


const std::string path_console_bb_platform2 = "gfx/console_bb_platform2.png";
const uint16_t console_info_bb_platform2[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_platform2 (
    "Burger Bard's Platform Adventure", "BB_PLATFORM2", "2025-10-20"
    , rom_GW_bb_platform2, size_rom_GW_bb_platform2
    , melody_GW_bb_platform2, size_melody_GW_bb_platform2
    , path_segment_bb_platform2
    , segment_GW_bb_platform2, size_segment_GW_bb_platform2
    , segment_info_bb_platform2
    , path_background_bb_platform2
    , background_info_bb_platform2
    , path_console_bb_platform2
    , console_info_bb_platform2
    , GW_rom::MANUFACTURER_BURGER_BARD
);

