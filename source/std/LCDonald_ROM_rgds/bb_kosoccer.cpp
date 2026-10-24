
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_kosoccer.h"


const uint8_t rom_GW_bb_kosoccer[] = {
	0xFF, 0xFF, 0xFE, 0x26
}; const size_t size_rom_GW_bb_kosoccer = sizeof(rom_GW_bb_kosoccer)/sizeof(rom_GW_bb_kosoccer[0]);

const uint8_t melody_GW_bb_kosoccer[1] = {0}; 
	const size_t size_melody_GW_bb_kosoccer = 0;

const std::string path_segment_bb_kosoccer = "gfx/segment_bb_kosoccer.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_kosoccer[] = {
	{ { 0,0,0 }, { 38,23 }, { 124,418 }, { 84,94 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 124,23 }, { 404,833 }, { 83,94 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 209,23 }, { 404,929 }, { 84,94 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 194,121 }, { 470,679 }, { 39,39 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 274,118 }, { 470,638 }, { 39,39 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 347,117 }, { 470,720 }, { 40,39 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 157,167 }, { 404,393 }, { 54,54 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 268,167 }, { 404,449 }, { 54,55 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 379,165 }, { 404,506 }, { 55,55 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 117,229 }, { 398,563 }, { 63,64 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 264,236 }, { 404,695 }, { 64,64 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 411,233 }, { 404,629 }, { 64,64 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 74,303 }, { 368,761 }, { 70,70 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 263,313 }, { 440,761 }, { 69,70 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 434,315 }, { 124,346 }, { 69,70 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 26,368 }, { 1,514 }, { 208,240 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 200,374 }, { 211,332 }, { 191,202 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 348,375 }, { 211,536 }, { 185,218 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 173,581 }, { 1,222 }, { 261,108 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 45,130 }, { 1,756 }, { 365,267 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 442,23 }, { 1,351 }, { 121,161 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_kosoccer = sizeof(segment_GW_bb_kosoccer)/sizeof(segment_GW_bb_kosoccer[0]); 
 const uint16_t segment_info_bb_kosoccer[] = { 512, 1024, 1, 0, 591, 719}; 

const std::string path_background_bb_kosoccer = "gfx/background_bb_kosoccer.png";
const uint16_t background_info_bb_kosoccer[] = { 1024, 1024, 1, 304, 591, 719, 1, 0, 0 }; 


const std::string path_console_bb_kosoccer = "gfx/console_bb_kosoccer.png";
const uint16_t console_info_bb_kosoccer[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom bb_kosoccer (
    "Kola Bear's Soccer", "BB_KOSOCCER", "2025-10-20"
    , rom_GW_bb_kosoccer, size_rom_GW_bb_kosoccer
    , melody_GW_bb_kosoccer, size_melody_GW_bb_kosoccer
    , path_segment_bb_kosoccer
    , segment_GW_bb_kosoccer, size_segment_GW_bb_kosoccer
    , segment_info_bb_kosoccer
    , path_background_bb_kosoccer
    , background_info_bb_kosoccer
    , path_console_bb_kosoccer
    , console_info_bb_kosoccer
    , GW_rom::MANUFACTURER_BURGER_BARD
);

