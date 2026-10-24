
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_speedway.h"


const uint8_t rom_GW_bb_speedway[] = {
	0xFF, 0xFF, 0xFE, 0x22
}; const size_t size_rom_GW_bb_speedway = sizeof(rom_GW_bb_speedway)/sizeof(rom_GW_bb_speedway[0]);

const uint8_t melody_GW_bb_speedway[1] = {0}; 
	const size_t size_melody_GW_bb_speedway = 0;

const std::string path_segment_bb_speedway = "gfx/segment_bb_speedway.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_speedway[] = {
	{ { 0,0,0 }, { 466,25 }, { 99,513 }, { 77,80 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 536,25 }, { 332,475 }, { 77,79 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 606,25 }, { 178,474 }, { 77,79 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 128,24 }, { 579,733 }, { 64,43 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 202,25 }, { 579,688 }, { 64,43 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 282,24 }, { 813,618 }, { 64,43 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 161,62 }, { 257,489 }, { 67,64 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 230,63 }, { 813,515 }, { 96,64 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 341,64 }, { 911,515 }, { 96,64 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 164,129 }, { 1,501 }, { 96,92 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 260,125 }, { 385,556 }, { 139,92 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 401,124 }, { 191,555 }, { 139,92 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 110,224 }, { 385,650 }, { 192,126 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 297,217 }, { 579,534 }, { 131,127 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 423,218 }, { 191,649 }, { 192,127 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 198,318 }, { 881,581 }, { 111,215 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 418,305 }, { 882,798 }, { 113,225 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 37,351 }, { 645,844 }, { 235,179 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 266,349 }, { 1,595 }, { 188,181 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 449,351 }, { 645,663 }, { 234,179 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 38,43 }, { 712,528 }, { 99,133 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 39,108 }, { 1,778 }, { 642,245 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_speedway = sizeof(segment_GW_bb_speedway)/sizeof(segment_GW_bb_speedway[0]); 
 const uint16_t segment_info_bb_speedway[] = { 1024, 1024, 1, 0, 719, 556}; 

const std::string path_background_bb_speedway = "gfx/background_bb_speedway.png";
const uint16_t background_info_bb_speedway[] = { 1024, 1024, 1, 467, 719, 556, 1, 0, 0 }; 


const std::string path_console_bb_speedway = "gfx/console_bb_speedway.png";
const uint16_t console_info_bb_speedway[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom bb_speedway (
    "Burger Bard Speedway Racing", "BB_SPEEDWAY", "2025-10-20"
    , rom_GW_bb_speedway, size_rom_GW_bb_speedway
    , melody_GW_bb_speedway, size_melody_GW_bb_speedway
    , path_segment_bb_speedway
    , segment_GW_bb_speedway, size_segment_GW_bb_speedway
    , segment_info_bb_speedway
    , path_background_bb_speedway
    , background_info_bb_speedway
    , path_console_bb_speedway
    , console_info_bb_speedway
    , GW_rom::MANUFACTURER_BURGER_BARD
);

