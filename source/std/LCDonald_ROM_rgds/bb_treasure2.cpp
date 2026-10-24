
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_treasure2.h"


const uint8_t rom_GW_bb_treasure2[] = {
	0xFF, 0xFF, 0xFE, 0x1B
}; const size_t size_rom_GW_bb_treasure2 = sizeof(rom_GW_bb_treasure2)/sizeof(rom_GW_bb_treasure2[0]);

const uint8_t melody_GW_bb_treasure2[1] = {0}; 
	const size_t size_melody_GW_bb_treasure2 = 0;

const std::string path_segment_bb_treasure2 = "gfx/segment_bb_treasure2.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_treasure2[] = {
	{ { 0,0,0 }, { 185,5 }, { 1,164 }, { 233,112 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 204,120 }, { 923,776 }, { 90,31 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 299,120 }, { 93,18 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 10,177 }, { 236,3 }, { 79,98 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 88,116 }, { 236,103 }, { 117,173 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 214,151 }, { 355,7 }, { 73,119 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 309,151 }, { 836,904 }, { 72,119 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 392,121 }, { 680,850 }, { 79,173 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 477,142 }, { 761,807 }, { 81,95 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 207,270 }, { 751,775 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 302,270 }, { 843,744 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 7,326 }, { 672,754 }, { 77,94 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 88,286 }, { 465,129 }, { 107,147 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 215,303 }, { 910,904 }, { 72,119 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 309,303 }, { 430,7 }, { 73,119 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 385,292 }, { 355,128 }, { 108,148 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 488,318 }, { 923,809 }, { 77,93 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 207,423 }, { 751,743 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 302,423 }, { 843,712 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 82,436 }, { 574,734 }, { 96,141 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 216,456 }, { 505,8 }, { 73,119 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 310,456 }, { 761,904 }, { 73,119 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 387,437 }, { 574,877 }, { 104,146 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 487,460 }, { 844,808 }, { 77,94 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 209,576 }, { 1,17 }, { 90,31 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 304,576 }, { 672,711 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 183,606 }, { 1,50 }, { 233,112 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 23,21 }, { 1,278 }, { 527,110 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 20,15 }, { 1,390 }, { 187,633 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_treasure2 = sizeof(segment_GW_bb_treasure2)/sizeof(segment_GW_bb_treasure2[0]); 
 const uint16_t segment_info_bb_treasure2[] = { 1024, 1024, 1, 0, 572, 719}; 

const std::string path_background_bb_treasure2 = "gfx/background_bb_treasure2.png";
const uint16_t background_info_bb_treasure2[] = { 1024, 1024, 1, 304, 572, 719, 1, 0, 0 }; 


const std::string path_console_bb_treasure2 = "gfx/console_bb_treasure2.png";
const uint16_t console_info_bb_treasure2[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_treasure2 (
    "Halmon's Egg Hunt", "BB_TREASURE2", "2025-10-20"
    , rom_GW_bb_treasure2, size_rom_GW_bb_treasure2
    , melody_GW_bb_treasure2, size_melody_GW_bb_treasure2
    , path_segment_bb_treasure2
    , segment_GW_bb_treasure2, size_segment_GW_bb_treasure2
    , segment_info_bb_treasure2
    , path_background_bb_treasure2
    , background_info_bb_treasure2
    , path_console_bb_treasure2
    , console_info_bb_treasure2
    , GW_rom::MANUFACTURER_BURGER_BARD
);

