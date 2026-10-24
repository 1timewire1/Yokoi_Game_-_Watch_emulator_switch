
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_platform.h"


const uint8_t rom_GW_bb_platform[] = {
	0xFF, 0xFF, 0xFE, 0x1C
}; const size_t size_rom_GW_bb_platform = sizeof(rom_GW_bb_platform)/sizeof(rom_GW_bb_platform[0]);

const uint8_t melody_GW_bb_platform[1] = {0}; 
	const size_t size_melody_GW_bb_platform = 0;

const std::string path_segment_bb_platform = "gfx/segment_bb_platform.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_platform[] = {
	{ { 0,0,0 }, { 474,119 }, { 474,1608 }, { 35,50 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 511,118 }, { 474,1660 }, { 36,52 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 549,119 }, { 474,1556 }, { 35,50 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 65,6 }, { 351,1449 }, { 109,171 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 220,13 }, { 176,945 }, { 91,162 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 390,13 }, { 269,878 }, { 91,162 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 573,13 }, { 413,1714 }, { 92,162 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 5,177 }, { 295,1878 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 182,178 }, { 1,1451 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 358,178 }, { 176,1109 }, { 172,169 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 534,178 }, { 176,1451 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 15,195 }, { 351,1303 }, { 109,144 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 195,207 }, { 351,1173 }, { 121,128 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 6,352 }, { 1,1280 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 182,354 }, { 176,1280 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 356,356 }, { 1,939 }, { 173,168 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 534,355 }, { 1,1109 }, { 173,169 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 27,363 }, { 362,889 }, { 92,151 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 196,383 }, { 350,1042 }, { 120,129 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 158,20 }, { 1,1622 }, { 410,153 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 372,240 }, { 1,1777 }, { 292,270 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_platform = sizeof(segment_GW_bb_platform)/sizeof(segment_GW_bb_platform[0]); 
 const uint16_t segment_info_bb_platform[] = { 512, 2048, 1, 0, 719, 553}; 

const std::string path_background_bb_platform = "gfx/background_bb_platform.png";
const uint16_t background_info_bb_platform[] = { 1024, 1024, 1, 470, 719, 553, 1, 0, 0 }; 


const std::string path_console_bb_platform = "gfx/console_bb_platform.png";
const uint16_t console_info_bb_platform[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom bb_platform (
    "Kola Bear's Platform Adventure", "BB_PLATFORM", "2025-10-20"
    , rom_GW_bb_platform, size_rom_GW_bb_platform
    , melody_GW_bb_platform, size_melody_GW_bb_platform
    , path_segment_bb_platform
    , segment_GW_bb_platform, size_segment_GW_bb_platform
    , segment_info_bb_platform
    , path_background_bb_platform
    , background_info_bb_platform
    , path_console_bb_platform
    , console_info_bb_platform
    , GW_rom::MANUFACTURER_BURGER_BARD
);

