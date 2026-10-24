
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_bfishing.h"


const uint8_t rom_GW_lcd_bfishing[] = {
	0xFF, 0xFF, 0xFE, 0x0D
}; const size_t size_rom_GW_lcd_bfishing = sizeof(rom_GW_lcd_bfishing)/sizeof(rom_GW_lcd_bfishing[0]);

const uint8_t melody_GW_lcd_bfishing[1] = {0}; 
	const size_t size_melody_GW_lcd_bfishing = 0;

const std::string path_segment_lcd_bfishing = "gfx/segment_lcd_bfishing.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_bfishing[] = {
	{ { 0,0,0 }, { 45,69 }, { 318,216 }, { 38,60 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 93,69 }, { 358,194 }, { 38,60 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 139,69 }, { 318,278 }, { 39,60 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 366,90 }, { 289,110 }, { 93,68 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 449,92 }, { 361,256 }, { 111,88 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 180,83 }, { 1,342 }, { 256,169 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 519,164 }, { 361,430 }, { 146,81 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 377,219 }, { 361,346 }, { 139,82 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 520,271 }, { 308,275 }, { 4,63 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 49,292 }, { 112,254 }, { 101,86 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 238,296 }, { 103,94 }, { 91,82 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 422,307 }, { 217,25 }, { 92,71 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 521,337 }, { 438,223 }, { 38,31 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 607,304 }, { 103,178 }, { 106,74 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 520,373 }, { 502,365 }, { 4,63 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 57,408 }, { 1,18 }, { 105,72 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 237,390 }, { 215,247 }, { 91,91 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 404,399 }, { 1,260 }, { 109,80 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 521,440 }, { 318,182 }, { 38,32 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 602,402 }, { 1,92 }, { 96,79 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 697,344 }, { 259,340 }, { 100,171 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 520,481 }, { 502,300 }, { 4,63 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 57,508 }, { 196,98 }, { 91,78 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 236,520 }, { 211,180 }, { 105,65 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 413,501 }, { 1,173 }, { 100,85 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 521,547 }, { 398,222 }, { 38,32 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 613,517 }, { 108,23 }, { 107,69 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_bfishing = sizeof(segment_GW_lcd_bfishing)/sizeof(segment_GW_lcd_bfishing[0]); 
 const uint16_t segment_info_lcd_bfishing[] = { 512, 512, 1, 0, 828, 639}; 

const std::string path_background_lcd_bfishing = "gfx/background_lcd_bfishing.png";
const uint16_t background_info_lcd_bfishing[] = { 1024, 1024, 1, 384, 828, 639, 1, 0, 0 }; 


const std::string path_console_lcd_bfishing = "gfx/console_lcd_bfishing.png";
const uint16_t console_info_lcd_bfishing[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_bfishing (
    "Big's Fishing", "BFISHING", "2004-12-03"
    , rom_GW_lcd_bfishing, size_rom_GW_lcd_bfishing
    , melody_GW_lcd_bfishing, size_melody_GW_lcd_bfishing
    , path_segment_lcd_bfishing
    , segment_GW_lcd_bfishing, size_segment_GW_lcd_bfishing
    , segment_info_lcd_bfishing
    , path_background_lcd_bfishing
    , background_info_lcd_bfishing
    , path_console_lcd_bfishing
    , console_info_lcd_bfishing
    , GW_rom::MANUFACTURER_MCDONALDS
);

