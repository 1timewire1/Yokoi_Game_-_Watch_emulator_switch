
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_eggcatch.h"


const uint8_t rom_GW_bb_eggcatch[] = {
	0xFF, 0xFF, 0xFE, 0x17
}; const size_t size_rom_GW_bb_eggcatch = sizeof(rom_GW_bb_eggcatch)/sizeof(rom_GW_bb_eggcatch[0]);

const uint8_t melody_GW_bb_eggcatch[1] = {0}; 
	const size_t size_melody_GW_bb_eggcatch = 0;

const std::string path_segment_bb_eggcatch = "gfx/segment_bb_eggcatch.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_eggcatch[] = {
	{ { 0,0,0 }, { 33,38 }, { 224,488 }, { 33,44 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 29,95 }, { 182,488 }, { 40,44 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 28,151 }, { 132,161 }, { 42,100 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 28,36 }, { 527,738 }, { 207,285 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 240,102 }, { 1,741 }, { 52,67 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 339,94 }, { 1,530 }, { 53,65 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 480,105 }, { 55,746 }, { 56,62 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 232,36 }, { 182,205 }, { 85,140 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 332,36 }, { 917,891 }, { 104,132 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 453,40 }, { 1,21 }, { 97,132 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 158,236 }, { 56,529 }, { 52,66 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 307,242 }, { 171,744 }, { 54,64 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 442,233 }, { 227,754 }, { 64,54 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 151,173 }, { 917,752 }, { 97,137 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 287,176 }, { 182,347 }, { 89,135 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 387,187 }, { 1,155 }, { 129,106 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 105,365 }, { 113,746 }, { 56,62 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 270,357 }, { 110,534 }, { 56,61 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 415,362 }, { 168,534 }, { 56,61 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 77,300 }, { 917,618 }, { 98,132 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 243,291 }, { 100,21 }, { 97,132 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 387,296 }, { 917,484 }, { 98,132 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 204,492 }, { 226,559 }, { 32,36 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 377,501 }, { 176,166 }, { 32,37 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 28,438 }, { 736,513 }, { 179,248 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 207,426 }, { 736,763 }, { 177,260 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 375,438 }, { 1,263 }, { 179,248 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 180,344 }, { 1,597 }, { 232,139 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 29,315 }, { 1,810 }, { 524,213 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_eggcatch = sizeof(segment_GW_bb_eggcatch)/sizeof(segment_GW_bb_eggcatch[0]); 
 const uint16_t segment_info_bb_eggcatch[] = { 1024, 1024, 1, 0, 579, 720}; 

const std::string path_background_bb_eggcatch = "gfx/background_bb_eggcatch.png";
const uint16_t background_info_bb_eggcatch[] = { 1024, 1024, 1, 303, 579, 720, 1, 0, 0 }; 


const std::string path_console_bb_eggcatch = "gfx/console_bb_eggcatch.png";
const uint16_t console_info_bb_eggcatch[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom bb_eggcatch (
    "Halmon's Egg Catch", "BB_EGGCATCH", "2025-10-20"
    , rom_GW_bb_eggcatch, size_rom_GW_bb_eggcatch
    , melody_GW_bb_eggcatch, size_melody_GW_bb_eggcatch
    , path_segment_bb_eggcatch
    , segment_GW_bb_eggcatch, size_segment_GW_bb_eggcatch
    , segment_info_bb_eggcatch
    , path_background_bb_eggcatch
    , background_info_bb_eggcatch
    , path_console_bb_eggcatch
    , console_info_bb_eggcatch
    , GW_rom::MANUFACTURER_BURGER_BARD
);

