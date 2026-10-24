
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_extremeboard.h"


const uint8_t rom_GW_bb_extremeboard[] = {
	0xFF, 0xFF, 0xFE, 0x23
}; const size_t size_rom_GW_bb_extremeboard = sizeof(rom_GW_bb_extremeboard)/sizeof(rom_GW_bb_extremeboard[0]);

const uint8_t melody_GW_bb_extremeboard[1] = {0}; 
	const size_t size_melody_GW_bb_extremeboard = 0;

const std::string path_segment_bb_extremeboard = "gfx/segment_bb_extremeboard.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_extremeboard[] = {
	{ { 0,0,0 }, { 26,13 }, { 301,1079 }, { 153,211 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 175,66 }, { 456,1611 }, { 51,93 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 291,12 }, { 301,934 }, { 139,143 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 496,66 }, { 456,1706 }, { 51,94 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 547,11 }, { 148,1071 }, { 151,219 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 253,108 }, { 474,1546 }, { 34,63 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 435,109 }, { 409,1738 }, { 34,62 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 318,148 }, { 436,1393 }, { 17,31 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 353,166 }, { 402,1335 }, { 13,23 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 380,149 }, { 436,1360 }, { 17,31 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 302,182 }, { 402,1392 }, { 32,32 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 343,195 }, { 402,1301 }, { 32,32 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 384,181 }, { 436,1301 }, { 32,32 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 16,242 }, { 456,1802 }, { 45,115 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 236,178 }, { 456,1335 }, { 48,47 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 436,185 }, { 461,1434 }, { 48,48 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 649,256 }, { 409,1802 }, { 45,115 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 16,196 }, { 1,1528 }, { 202,237 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 155,187 }, { 402,1426 }, { 57,56 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 335,230 }, { 461,1384 }, { 48,48 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 495,199 }, { 409,1546 }, { 63,63 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 494,209 }, { 205,1529 }, { 202,236 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 213,235 }, { 409,1687 }, { 33,49 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 465,242 }, { 471,1495 }, { 36,49 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 133,299 }, { 1,1301 }, { 173,225 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 364,266 }, { 409,1484 }, { 60,60 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 409,291 }, { 205,1292 }, { 195,235 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 290,295 }, { 442,899 }, { 62,178 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 399,319 }, { 409,1919 }, { 69,128 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 305,288 }, { 1,1061 }, { 145,238 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 128,14 }, { 1,1767 }, { 398,280 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 21,454 }, { 148,1001 }, { 121,68 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_extremeboard = sizeof(segment_GW_bb_extremeboard)/sizeof(segment_GW_bb_extremeboard[0]); 
 const uint16_t segment_info_bb_extremeboard[] = { 512, 2048, 1, 0, 719, 540}; 

const std::string path_background_bb_extremeboard = "gfx/background_bb_extremeboard.png";
const uint16_t background_info_bb_extremeboard[] = { 1024, 1024, 1, 483, 719, 540, 1, 0, 0 }; 


const std::string path_console_bb_extremeboard = "gfx/console_bb_extremeboard.png";
const uint16_t console_info_bb_extremeboard[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_extremeboard (
    "Vibes Extreme Skateboarding", "BB_EXTREMEBOARD", "2025-10-20"
    , rom_GW_bb_extremeboard, size_rom_GW_bb_extremeboard
    , melody_GW_bb_extremeboard, size_melody_GW_bb_extremeboard
    , path_segment_bb_extremeboard
    , segment_GW_bb_extremeboard, size_segment_GW_bb_extremeboard
    , segment_info_bb_extremeboard
    , path_background_bb_extremeboard
    , background_info_bb_extremeboard
    , path_console_bb_extremeboard
    , console_info_bb_extremeboard
    , GW_rom::MANUFACTURER_BURGER_BARD
);

