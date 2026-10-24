
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_fishing.h"


const uint8_t rom_GW_bb_fishing[] = {
	0xFF, 0xFF, 0xFE, 0x16
}; const size_t size_rom_GW_bb_fishing = sizeof(rom_GW_bb_fishing)/sizeof(rom_GW_bb_fishing[0]);

const uint8_t melody_GW_bb_fishing[1] = {0}; 
	const size_t size_melody_GW_bb_fishing = 0;

const std::string path_segment_bb_fishing = "gfx/segment_bb_fishing.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_fishing[] = {
	{ { 0,0,0 }, { 38,28 }, { 211,285 }, { 33,52 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 79,28 }, { 211,231 }, { 33,52 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 120,28 }, { 211,177 }, { 33,52 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 315,19 }, { 1,17 }, { 244,131 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 149,22 }, { 1,339 }, { 534,172 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 426,76 }, { 372,113 }, { 139,100 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 230,147 }, { 372,215 }, { 203,122 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 430,168 }, { 196,187 }, { 13,150 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 30,272 }, { 577,231 }, { 138,89 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 162,275 }, { 684,425 }, { 147,86 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 294,275 }, { 658,142 }, { 135,87 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 416,317 }, { 567,3 }, { 19,24 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 452,273 }, { 513,121 }, { 143,92 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 430,321 }, { 842,335 }, { 12,88 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 31,352 }, { 567,29 }, { 146,90 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 163,354 }, { 247,1 }, { 159,88 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 294,353 }, { 717,239 }, { 129,90 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 416,408 }, { 196,152 }, { 20,23 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 445,352 }, { 711,331 }, { 129,92 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 569,282 }, { 247,91 }, { 123,246 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 430,412 }, { 964,429 }, { 13,82 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 33,437 }, { 537,421 }, { 145,90 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 165,440 }, { 408,25 }, { 157,86 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 297,441 }, { 833,425 }, { 129,86 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 417,493 }, { 218,152 }, { 20,23 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 445,427 }, { 577,322 }, { 132,97 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 27,85 }, { 1,150 }, { 193,187 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 450,177 }, { 715,47 }, { 72,93 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_fishing = sizeof(segment_GW_bb_fishing)/sizeof(segment_GW_bb_fishing[0]); 
 const uint16_t segment_info_bb_fishing[] = { 1024, 512, 1, 0, 719, 554}; 

const std::string path_background_bb_fishing = "gfx/background_bb_fishing.png";
const uint16_t background_info_bb_fishing[] = { 1024, 1024, 1, 469, 719, 554, 1, 0, 0 }; 


const std::string path_console_bb_fishing = "gfx/console_bb_fishing.png";
const uint16_t console_info_bb_fishing[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom bb_fishing (
    "Burger Bard Fishing", "BB_FISHING", "2025-10-20"
    , rom_GW_bb_fishing, size_rom_GW_bb_fishing
    , melody_GW_bb_fishing, size_melody_GW_bb_fishing
    , path_segment_bb_fishing
    , segment_GW_bb_fishing, size_segment_GW_bb_fishing
    , segment_info_bb_fishing
    , path_background_bb_fishing
    , background_info_bb_fishing
    , path_console_bb_fishing
    , console_info_bb_fishing
    , GW_rom::MANUFACTURER_BURGER_BARD
);

