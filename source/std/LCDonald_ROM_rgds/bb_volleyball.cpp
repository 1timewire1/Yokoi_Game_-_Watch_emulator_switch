
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_volleyball.h"


const uint8_t rom_GW_bb_volleyball[] = {
	0xFF, 0xFF, 0xFE, 0x15
}; const size_t size_rom_GW_bb_volleyball = sizeof(rom_GW_bb_volleyball)/sizeof(rom_GW_bb_volleyball[0]);

const uint8_t melody_GW_bb_volleyball[1] = {0}; 
	const size_t size_melody_GW_bb_volleyball = 0;

const std::string path_segment_bb_volleyball = "gfx/segment_bb_volleyball.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_volleyball[] = {
	{ { 0,0,0 }, { 149,23 }, { 361,605 }, { 44,36 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 196,23 }, { 489,605 }, { 31,36 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 230,23 }, { 522,605 }, { 22,36 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 264,23 }, { 453,605 }, { 34,36 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 309,23 }, { 293,541 }, { 22,36 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 334,23 }, { 361,567 }, { 31,36 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 368,23 }, { 407,605 }, { 44,36 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 151,75 }, { 709,570 }, { 150,216 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 250,61 }, { 547,800 }, { 174,223 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 144,190 }, { 1,427 }, { 44,44 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 386,187 }, { 978,630 }, { 45,44 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 112,271 }, { 978,723 }, { 45,45 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 261,293 }, { 978,676 }, { 45,45 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 409,270 }, { 1,473 }, { 45,44 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 134,305 }, { 709,468 }, { 293,100 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 71,371 }, { 57,465 }, { 53,53 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 253,343 }, { 113,521 }, { 54,53 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 438,372 }, { 113,466 }, { 54,53 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 31,467 }, { 57,520 }, { 54,54 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 247,403 }, { 237,527 }, { 54,55 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 480,468 }, { 1,519 }, { 54,55 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 38,526 }, { 303,579 }, { 56,62 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 63,461 }, { 723,788 }, { 161,235 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 196,528 }, { 1,576 }, { 176,65 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 212,445 }, { 886,770 }, { 133,253 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 471,527 }, { 179,520 }, { 56,62 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 341,462 }, { 547,563 }, { 160,235 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 22,570 }, { 861,574 }, { 69,45 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 222,588 }, { 179,584 }, { 122,57 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 473,571 }, { 932,574 }, { 69,45 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 10,7 }, { 1,643 }, { 544,380 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 27,118 }, { 861,621 }, { 115,147 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_volleyball = sizeof(segment_GW_bb_volleyball)/sizeof(segment_GW_bb_volleyball[0]); 
 const uint16_t segment_info_bb_volleyball[] = { 1024, 1024, 1, 0, 561, 719}; 

const std::string path_background_bb_volleyball = "gfx/background_bb_volleyball.png";
const uint16_t background_info_bb_volleyball[] = { 1024, 1024, 1, 304, 561, 719, 1, 0, 0 }; 


const std::string path_console_bb_volleyball = "gfx/console_bb_volleyball.png";
const uint16_t console_info_bb_volleyball[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom bb_volleyball (
    "Curly & Frenchie's Volleyball", "BB_VOLLEYBALL", "2025-10-20"
    , rom_GW_bb_volleyball, size_rom_GW_bb_volleyball
    , melody_GW_bb_volleyball, size_melody_GW_bb_volleyball
    , path_segment_bb_volleyball
    , segment_GW_bb_volleyball, size_segment_GW_bb_volleyball
    , segment_info_bb_volleyball
    , path_background_bb_volleyball
    , background_info_bb_volleyball
    , path_console_bb_volleyball
    , console_info_bb_volleyball
    , GW_rom::MANUFACTURER_BURGER_BARD
);

