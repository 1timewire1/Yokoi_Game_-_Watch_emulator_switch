
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_football.h"


const uint8_t rom_GW_bb_football[] = {
	0xFF, 0xFF, 0xFE, 0x19
}; const size_t size_rom_GW_bb_football = sizeof(rom_GW_bb_football)/sizeof(rom_GW_bb_football[0]);

const uint8_t melody_GW_bb_football[1] = {0}; 
	const size_t size_melody_GW_bb_football = 0;

const std::string path_segment_bb_football = "gfx/segment_bb_football.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_football[] = {
	{ { 0,0,0 }, { 450,27 }, { 205,567 }, { 76,65 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 530,27 }, { 1,237 }, { 76,65 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 609,27 }, { 79,240 }, { 76,65 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 615,196 }, { 444,710 }, { 65,60 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 607,256 }, { 313,250 }, { 76,52 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 606,314 }, { 84,183 }, { 71,55 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 503,140 }, { 239,249 }, { 72,56 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 487,206 }, { 157,200 }, { 77,52 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 472,273 }, { 157,254 }, { 80,51 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 402,95 }, { 391,251 }, { 77,51 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 375,169 }, { 1,134 }, { 78,50 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 363,300 }, { 391,198 }, { 77,51 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 300,68 }, { 1,186 }, { 81,49 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 263,162 }, { 313,196 }, { 75,52 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 254,374 }, { 444,772 }, { 63,62 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 53,27 }, { 205,634 }, { 237,200 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 159,161 }, { 326,403 }, { 168,229 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 36,326 }, { 205,836 }, { 267,187 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 36,246 }, { 326,304 }, { 139,97 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 321,318 }, { 1,307 }, { 323,209 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 273,16 }, { 1,518 }, { 202,505 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_football = sizeof(segment_GW_bb_football)/sizeof(segment_GW_bb_football[0]); 
 const uint16_t segment_info_bb_football[] = { 512, 1024, 1, 0, 718, 542}; 

const std::string path_background_bb_football = "gfx/background_bb_football.png";
const uint16_t background_info_bb_football[] = { 1024, 1024, 1, 481, 718, 542, 1, 0, 0 }; 


const std::string path_console_bb_football = "gfx/console_bb_football.png";
const uint16_t console_info_bb_football[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_football (
    "King Beareon's Football Catch", "BB_FOOTBALL", "2025-10-20"
    , rom_GW_bb_football, size_rom_GW_bb_football
    , melody_GW_bb_football, size_melody_GW_bb_football
    , path_segment_bb_football
    , segment_GW_bb_football, size_segment_GW_bb_football
    , segment_info_bb_football
    , path_background_bb_football
    , background_info_bb_football
    , path_console_bb_football
    , console_info_bb_football
    , GW_rom::MANUFACTURER_BURGER_BARD
);

