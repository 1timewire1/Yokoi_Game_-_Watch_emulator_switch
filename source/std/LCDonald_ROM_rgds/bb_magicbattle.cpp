
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_magicbattle.h"


const uint8_t rom_GW_bb_magicbattle[] = {
	0xFF, 0xFF, 0xFE, 0x20
}; const size_t size_rom_GW_bb_magicbattle = sizeof(rom_GW_bb_magicbattle)/sizeof(rom_GW_bb_magicbattle[0]);

const uint8_t melody_GW_bb_magicbattle[1] = {0}; 
	const size_t size_melody_GW_bb_magicbattle = 0;

const std::string path_segment_bb_magicbattle = "gfx/segment_bb_magicbattle.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_magicbattle[] = {
	{ { 0,0,0 }, { 494,39 }, { 247,620 }, { 20,5 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 491,41 }, { 1,598 }, { 6,20 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 509,41 }, { 9,598 }, { 6,20 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 493,59 }, { 291,620 }, { 18,5 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 158,584 }, { 1,620 }, { 244,102 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 489,61 }, { 17,598 }, { 6,20 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 507,62 }, { 25,598 }, { 6,20 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 223,635 }, { 859,545 }, { 118,28 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 490,78 }, { 269,620 }, { 20,5 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 128,355 }, { 517,622 }, { 87,100 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 238,277 }, { 423,627 }, { 92,95 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 341,354 }, { 606,666 }, { 86,100 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 164,510 }, { 740,512 }, { 59,78 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 192,438 }, { 247,628 }, { 174,94 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 337,513 }, { 801,515 }, { 56,75 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 18,431 }, { 696,769 }, { 171,254 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 217,369 }, { 896,759 }, { 122,264 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 368,431 }, { 523,768 }, { 171,255 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 182,588 }, { 869,994 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 327,532 }, { 869,963 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 358,589 }, { 869,932 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 152,244 }, { 896,667 }, { 74,90 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 241,188 }, { 654,512 }, { 84,78 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 334,240 }, { 896,575 }, { 74,90 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 22,116 }, { 1,240 }, { 183,173 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 181,36 }, { 696,592 }, { 198,175 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 356,111 }, { 1,415 }, { 183,175 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 21,260 }, { 1,724 }, { 520,299 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 22,39 }, { 186,574 }, { 466,44 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_magicbattle = sizeof(segment_GW_bb_magicbattle)/sizeof(segment_GW_bb_magicbattle[0]); 
 const uint16_t segment_info_bb_magicbattle[] = { 1024, 1024, 1, 0, 561, 719}; 

const std::string path_background_bb_magicbattle = "gfx/background_bb_magicbattle.png";
const uint16_t background_info_bb_magicbattle[] = { 1024, 1024, 1, 304, 561, 719, 1, 0, 0 }; 


const std::string path_console_bb_magicbattle = "gfx/console_bb_magicbattle.png";
const uint16_t console_info_bb_magicbattle[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_magicbattle (
    "Bat Magic's Magic Battle", "BB_MAGICBATTLE", "2025-10-20"
    , rom_GW_bb_magicbattle, size_rom_GW_bb_magicbattle
    , melody_GW_bb_magicbattle, size_melody_GW_bb_magicbattle
    , path_segment_bb_magicbattle
    , segment_GW_bb_magicbattle, size_segment_GW_bb_magicbattle
    , segment_info_bb_magicbattle
    , path_background_bb_magicbattle
    , background_info_bb_magicbattle
    , path_console_bb_magicbattle
    , console_info_bb_magicbattle
    , GW_rom::MANUFACTURER_BURGER_BARD
);

