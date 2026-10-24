
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_seashell.h"


const uint8_t rom_GW_bb_seashell[] = {
	0xFF, 0xFF, 0xFE, 0x25
}; const size_t size_rom_GW_bb_seashell = sizeof(rom_GW_bb_seashell)/sizeof(rom_GW_bb_seashell[0]);

const uint8_t melody_GW_bb_seashell[1] = {0}; 
	const size_t size_melody_GW_bb_seashell = 0;

const std::string path_segment_bb_seashell = "gfx/segment_bb_seashell.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_seashell[] = {
	{ { 0,0,0 }, { 32,430 }, { 310,1428 }, { 106,106 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 139,430 }, { 310,1320 }, { 106,106 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 245,430 }, { 310,1212 }, { 106,106 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 32,116 }, { 396,910 }, { 84,88 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 118,96 }, { 320,820 }, { 83,88 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 205,75 }, { 235,852 }, { 83,89 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 290,55 }, { 310,943 }, { 84,89 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 379,25 }, { 1,946 }, { 307,194 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 357,175 }, { 454,1997 }, { 36,50 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 32,207 }, { 418,1446 }, { 87,88 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 119,207 }, { 418,1178 }, { 86,87 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 205,207 }, { 418,1267 }, { 86,88 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 291,207 }, { 418,1357 }, { 87,87 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 379,183 }, { 1,1339 }, { 307,195 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 376,339 }, { 454,1945 }, { 36,50 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 32,298 }, { 310,1123 }, { 86,87 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 118,314 }, { 398,1089 }, { 86,87 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 205,337 }, { 310,1034 }, { 86,87 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 290,347 }, { 398,1000 }, { 86,87 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 379,341 }, { 1,1142 }, { 307,195 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 36,27 }, { 1,1536 }, { 451,511 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 224,298 }, { 1,906 }, { 232,38 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_seashell = sizeof(segment_GW_bb_seashell)/sizeof(segment_GW_bb_seashell[0]); 
 const uint16_t segment_info_bb_seashell[] = { 512, 2048, 1, 0, 719, 561}; 

const std::string path_background_bb_seashell = "gfx/background_bb_seashell.png";
const uint16_t background_info_bb_seashell[] = { 1024, 1024, 1, 462, 719, 561, 1, 0, 0 }; 


const std::string path_console_bb_seashell = "gfx/console_bb_seashell.png";
const uint16_t console_info_bb_seashell[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_seashell (
    "Soakey's Shell Catch", "BB_SEASHELL", "2025-10-20"
    , rom_GW_bb_seashell, size_rom_GW_bb_seashell
    , melody_GW_bb_seashell, size_melody_GW_bb_seashell
    , path_segment_bb_seashell
    , segment_GW_bb_seashell, size_segment_GW_bb_seashell
    , segment_info_bb_seashell
    , path_background_bb_seashell
    , background_info_bb_seashell
    , path_console_bb_seashell
    , console_info_bb_seashell
    , GW_rom::MANUFACTURER_BURGER_BARD
);

