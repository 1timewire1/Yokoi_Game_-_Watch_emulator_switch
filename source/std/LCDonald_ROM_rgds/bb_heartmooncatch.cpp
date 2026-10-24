
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_heartmooncatch.h"


const uint8_t rom_GW_bb_heartmooncatch[] = {
	0xFF, 0xFF, 0xFE, 0x18
}; const size_t size_rom_GW_bb_heartmooncatch = sizeof(rom_GW_bb_heartmooncatch)/sizeof(rom_GW_bb_heartmooncatch[0]);

const uint8_t melody_GW_bb_heartmooncatch[1] = {0}; 
	const size_t size_melody_GW_bb_heartmooncatch = 0;

const std::string path_segment_bb_heartmooncatch = "gfx/segment_bb_heartmooncatch.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_heartmooncatch[] = {
	{ { 0,0,0 }, { 28,23 }, { 842,120 }, { 98,100 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 125,23 }, { 842,324 }, { 99,100 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 223,23 }, { 842,222 }, { 99,100 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 176,124 }, { 90,151 }, { 67,86 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 244,124 }, { 1,148 }, { 87,89 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 330,118 }, { 943,249 }, { 68,85 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 145,205 }, { 162,87 }, { 66,75 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 278,208 }, { 842,30 }, { 88,88 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 359,195 }, { 162,9 }, { 65,76 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 125,284 }, { 942,132 }, { 67,88 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 217,269 }, { 1,57 }, { 87,89 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 347,288 }, { 943,336 }, { 67,88 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 104,376 }, { 90,75 }, { 70,74 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 28,392 }, { 285,44 }, { 50,51 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 186,372 }, { 1,3 }, { 51,52 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 268,344 }, { 932,30 }, { 88,88 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 351,413 }, { 294,185 }, { 51,52 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 399,370 }, { 159,164 }, { 70,73 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 26,446 }, { 678,264 }, { 162,247 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 193,422 }, { 519,239 }, { 157,272 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 352,446 }, { 678,15 }, { 162,247 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 88,589 }, { 395,184 }, { 37,57 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 26,583 }, { 90,5 }, { 64,68 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 178,572 }, { 229,3 }, { 54,82 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 301,579 }, { 230,97 }, { 65,65 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 326,530 }, { 347,194 }, { 46,47 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 451,587 }, { 231,173 }, { 61,64 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 416,590 }, { 297,125 }, { 38,58 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 13,122 }, { 1,243 }, { 516,268 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 335,24 }, { 842,426 }, { 176,85 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_heartmooncatch = sizeof(segment_GW_bb_heartmooncatch)/sizeof(segment_GW_bb_heartmooncatch[0]); 
 const uint16_t segment_info_bb_heartmooncatch[] = { 1024, 512, 1, 0, 540, 720}; 

const std::string path_background_bb_heartmooncatch = "gfx/background_bb_heartmooncatch.png";
const uint16_t background_info_bb_heartmooncatch[] = { 1024, 1024, 1, 303, 540, 720, 1, 0, 0 }; 


const std::string path_console_bb_heartmooncatch = "gfx/console_bb_heartmooncatch.png";
const uint16_t console_info_bb_heartmooncatch[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_heartmooncatch (
    "Honey's Heart-Moon Catch", "BB_HEARTMOON", "2025-10-20"
    , rom_GW_bb_heartmooncatch, size_rom_GW_bb_heartmooncatch
    , melody_GW_bb_heartmooncatch, size_melody_GW_bb_heartmooncatch
    , path_segment_bb_heartmooncatch
    , segment_GW_bb_heartmooncatch, size_segment_GW_bb_heartmooncatch
    , segment_info_bb_heartmooncatch
    , path_background_bb_heartmooncatch
    , background_info_bb_heartmooncatch
    , path_console_bb_heartmooncatch
    , console_info_bb_heartmooncatch
    , GW_rom::MANUFACTURER_BURGER_BARD
);

