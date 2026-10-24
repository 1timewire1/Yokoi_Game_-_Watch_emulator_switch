
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_targetshoot.h"


const uint8_t rom_GW_bb_targetshoot[] = {
	0xFF, 0xFF, 0xFE, 0x24
}; const size_t size_rom_GW_bb_targetshoot = sizeof(rom_GW_bb_targetshoot)/sizeof(rom_GW_bb_targetshoot[0]);

const uint8_t melody_GW_bb_targetshoot[1] = {0}; 
	const size_t size_melody_GW_bb_targetshoot = 0;

const std::string path_segment_bb_targetshoot = "gfx/segment_bb_targetshoot.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_targetshoot[] = {
	{ { 0,0,0 }, { 25,23 }, { 338,1195 }, { 80,126 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 105,23 }, { 420,1195 }, { 80,126 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 185,25 }, { 349,1068 }, { 80,125 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 432,23 }, { 153,771 }, { 34,129 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 470,23 }, { 100,757 }, { 34,129 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 509,22 }, { 45,747 }, { 34,129 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 222,103 }, { 433,1078 }, { 74,115 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 288,98 }, { 1,859 }, { 42,125 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 378,99 }, { 433,958 }, { 69,118 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 224,194 }, { 349,870 }, { 65,83 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 321,151 }, { 100,888 }, { 51,87 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 368,150 }, { 238,812 }, { 49,88 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 168,221 }, { 1,1071 }, { 83,92 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 254,241 }, { 1,986 }, { 92,83 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 404,238 }, { 86,1071 }, { 83,92 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 186,293 }, { 95,977 }, { 68,92 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 327,263 }, { 189,807 }, { 47,93 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 366,257 }, { 45,878 }, { 53,97 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 127,301 }, { 433,838 }, { 69,118 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 282,302 }, { 449,1326 }, { 51,123 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 359,306 }, { 349,955 }, { 82,111 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 148,393 }, { 289,833 }, { 45,67 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 308,404 }, { 1,796 }, { 40,61 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 367,406 }, { 336,810 }, { 43,58 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 16,419 }, { 171,1166 }, { 165,283 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 192,435 }, { 171,902 }, { 176,262 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 373,421 }, { 1,1165 }, { 168,284 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 30,61 }, { 1,1451 }, { 507,596 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 32,156 }, { 349,1323 }, { 98,126 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_targetshoot = sizeof(segment_GW_bb_targetshoot)/sizeof(segment_GW_bb_targetshoot[0]); 
 const uint16_t segment_info_bb_targetshoot[] = { 512, 2048, 1, 0, 564, 719}; 

const std::string path_background_bb_targetshoot = "gfx/background_bb_targetshoot.png";
const uint16_t background_info_bb_targetshoot[] = { 1024, 1024, 1, 304, 564, 719, 1, 0, 0 }; 


const std::string path_console_bb_targetshoot = "gfx/console_bb_targetshoot.png";
const uint16_t console_info_bb_targetshoot[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom bb_targetshoot (
    "Taco Sal's Target Shooting", "BB_TARGETSHOOT", "2025-10-20"
    , rom_GW_bb_targetshoot, size_rom_GW_bb_targetshoot
    , melody_GW_bb_targetshoot, size_melody_GW_bb_targetshoot
    , path_segment_bb_targetshoot
    , segment_GW_bb_targetshoot, size_segment_GW_bb_targetshoot
    , segment_info_bb_targetshoot
    , path_background_bb_targetshoot
    , background_info_bb_targetshoot
    , path_console_bb_targetshoot
    , console_info_bb_targetshoot
    , GW_rom::MANUFACTURER_BURGER_BARD
);

