
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_treasure.h"


const uint8_t rom_GW_bb_treasure[] = {
	0xFF, 0xFF, 0xFE, 0x1A
}; const size_t size_rom_GW_bb_treasure = sizeof(rom_GW_bb_treasure)/sizeof(rom_GW_bb_treasure[0]);

const uint8_t melody_GW_bb_treasure[1] = {0}; 
	const size_t size_melody_GW_bb_treasure = 0;

const std::string path_segment_bb_treasure = "gfx/segment_bb_treasure.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_treasure[] = {
	{ { 0,0,0 }, { 195,6 }, { 687,639 }, { 208,113 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 204,120 }, { 1,362 }, { 90,31 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 299,120 }, { 93,363 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 12,166 }, { 81,117 }, { 86,106 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 73,116 }, { 823,469 }, { 128,168 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 211,151 }, { 1,226 }, { 77,119 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 303,151 }, { 80,226 }, { 77,119 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 375,115 }, { 897,852 }, { 112,171 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 478,151 }, { 81,8 }, { 83,107 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 207,270 }, { 93,396 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 302,270 }, { 185,363 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 9,273 }, { 169,127 }, { 82,96 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 58,286 }, { 687,754 }, { 155,153 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 210,301 }, { 934,347 }, { 78,120 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 302,301 }, { 934,225 }, { 78,120 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 383,290 }, { 897,701 }, { 109,149 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 488,323 }, { 159,238 }, { 77,107 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 207,423 }, { 185,396 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 302,423 }, { 897,669 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 41,417 }, { 687,468 }, { 134,169 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 213,455 }, { 687,347 }, { 78,119 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 305,455 }, { 1,104 }, { 78,119 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 385,438 }, { 823,318 }, { 109,149 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 484,472 }, { 166,6 }, { 77,109 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 209,576 }, { 1,395 }, { 90,31 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 304,576 }, { 238,286 }, { 90,30 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 174,604 }, { 687,909 }, { 208,114 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 23,16 }, { 1,428 }, { 518,595 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 33,26 }, { 521,395 }, { 164,628 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_treasure = sizeof(segment_GW_bb_treasure)/sizeof(segment_GW_bb_treasure[0]); 
 const uint16_t segment_info_bb_treasure[] = { 1024, 1024, 1, 0, 572, 719}; 

const std::string path_background_bb_treasure = "gfx/background_bb_treasure.png";
const uint16_t background_info_bb_treasure[] = { 1024, 1024, 1, 304, 572, 719, 1, 0, 0 }; 


const std::string path_console_bb_treasure = "gfx/console_bb_treasure.png";
const uint16_t console_info_bb_treasure[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_treasure (
    "Bard's Burger Hunt", "BB_TREASURE", "2025-10-20"
    , rom_GW_bb_treasure, size_rom_GW_bb_treasure
    , melody_GW_bb_treasure, size_melody_GW_bb_treasure
    , path_segment_bb_treasure
    , segment_GW_bb_treasure, size_segment_GW_bb_treasure
    , segment_info_bb_treasure
    , path_background_bb_treasure
    , background_info_bb_treasure
    , path_console_bb_treasure
    , console_info_bb_treasure
    , GW_rom::MANUFACTURER_BURGER_BARD
);

