
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_skateboard.h"


const uint8_t rom_GW_bb_skateboard[] = {
	0xFF, 0xFF, 0xFE, 0x21
}; const size_t size_rom_GW_bb_skateboard = sizeof(rom_GW_bb_skateboard)/sizeof(rom_GW_bb_skateboard[0]);

const uint8_t melody_GW_bb_skateboard[1] = {0}; 
	const size_t size_melody_GW_bb_skateboard = 0;

const std::string path_segment_bb_skateboard = "gfx/segment_bb_skateboard.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_skateboard[] = {
	{ { 0,0,0 }, { 15,21 }, { 856,240 }, { 71,88 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 87,21 }, { 929,240 }, { 71,88 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 160,21 }, { 760,227 }, { 71,88 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 314,21 }, { 879,330 }, { 79,90 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 393,21 }, { 775,317 }, { 79,90 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 472,21 }, { 879,422 }, { 80,89 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 284,134 }, { 671,155 }, { 34,39 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 210,129 }, { 533,3 }, { 34,39 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 278,234 }, { 533,320 }, { 48,54 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 353,134 }, { 569,4 }, { 34,38 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 191,109 }, { 833,166 }, { 72,72 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 266,114 }, { 729,153 }, { 72,72 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 334,113 }, { 655,196 }, { 72,73 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 185,209 }, { 973,183 }, { 48,55 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 266,363 }, { 671,80 }, { 63,71 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 371,212 }, { 583,320 }, { 48,54 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 157,180 }, { 671,408 }, { 102,103 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 252,205 }, { 775,409 }, { 102,102 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 343,183 }, { 655,271 }, { 103,103 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 138,313 }, { 907,166 }, { 64,72 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 402,314 }, { 960,348 }, { 63,72 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 101,275 }, { 533,376 }, { 136,135 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 229,324 }, { 517,182 }, { 136,136 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 364,276 }, { 533,44 }, { 136,136 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 13,379 }, { 1,69 }, { 191,249 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 227,452 }, { 363,70 }, { 152,248 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 392,378 }, { 194,67 }, { 167,251 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 24,112 }, { 1,320 }, { 530,191 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 21,636 }, { 1,2 }, { 530,63 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_skateboard = sizeof(segment_GW_bb_skateboard)/sizeof(segment_GW_bb_skateboard[0]); 
 const uint16_t segment_info_bb_skateboard[] = { 1024, 512, 1, 0, 570, 720}; 

const std::string path_background_bb_skateboard = "gfx/background_bb_skateboard.png";
const uint16_t background_info_bb_skateboard[] = { 1024, 1024, 1, 303, 570, 720, 1, 0, 0 }; 


const std::string path_console_bb_skateboard = "gfx/console_bb_skateboard.png";
const uint16_t console_info_bb_skateboard[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_skateboard (
    "Vibes Skateboarding", "BB_SKATEBOARD", "2025-10-20"
    , rom_GW_bb_skateboard, size_rom_GW_bb_skateboard
    , melody_GW_bb_skateboard, size_melody_GW_bb_skateboard
    , path_segment_bb_skateboard
    , segment_GW_bb_skateboard, size_segment_GW_bb_skateboard
    , segment_info_bb_skateboard
    , path_background_bb_skateboard
    , background_info_bb_skateboard
    , path_console_bb_skateboard
    , console_info_bb_skateboard
    , GW_rom::MANUFACTURER_BURGER_BARD
);

