
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_shockey.h"


const uint8_t rom_GW_lcd_shockey[] = {
	0xFF, 0xFF, 0xFE, 0x0A
}; const size_t size_rom_GW_lcd_shockey = sizeof(rom_GW_lcd_shockey)/sizeof(rom_GW_lcd_shockey[0]);

const uint8_t melody_GW_lcd_shockey[1] = {0}; 
	const size_t size_melody_GW_lcd_shockey = 0;

const std::string path_segment_lcd_shockey = "gfx/segment_lcd_shockey.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_shockey[] = {
	{ { 0,0,0 }, { 432,20 }, { 152,184 }, { 38,6 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 424,23 }, { 116,88 }, { 10,23 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 470,25 }, { 128,90 }, { 10,21 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 435,46 }, { 237,597 }, { 17,7 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 455,46 }, { 241,1016 }, { 14,7 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 191,12 }, { 1,323 }, { 143,81 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 424,52 }, { 152,90 }, { 9,21 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 470,52 }, { 140,90 }, { 10,21 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 232,71 }, { 153,389 }, { 64,43 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 431,72 }, { 1,2 }, { 39,7 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 76,182 }, { 192,147 }, { 63,43 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 234,182 }, { 192,102 }, { 63,43 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 390,182 }, { 146,344 }, { 64,43 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 52,230 }, { 1,11 }, { 112,75 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 209,233 }, { 115,12 }, { 113,74 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 367,232 }, { 1,88 }, { 113,75 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 55,281 }, { 132,726 }, { 107,118 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 214,282 }, { 132,606 }, { 107,118 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 371,282 }, { 128,486 }, { 107,118 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 36,323 }, { 1,244 }, { 150,77 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 195,325 }, { 1,165 }, { 149,77 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 352,324 }, { 1,406 }, { 149,78 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 56,414 }, { 152,192 }, { 73,50 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 231,414 }, { 116,113 }, { 74,50 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 405,414 }, { 153,434 }, { 73,50 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 14,471 }, { 1,667 }, { 129,177 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 218,473 }, { 1,846 }, { 147,177 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 386,473 }, { 1,492 }, { 125,173 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_shockey = sizeof(segment_GW_lcd_shockey)/sizeof(segment_GW_lcd_shockey[0]); 
 const uint16_t segment_info_lcd_shockey[] = { 256, 1024, 1, 0, 521, 668}; 

const std::string path_background_lcd_shockey = "gfx/background_lcd_shockey.png";
const uint16_t background_info_lcd_shockey[] = { 1024, 1024, 1, 355, 521, 668, 1, 0, 0 }; 


const std::string path_console_lcd_shockey = "gfx/console_lcd_shockey.png";
const uint16_t console_info_lcd_shockey[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_shockey (
    "Shadow Hockey", "SHOCKEY", "2004-12-03"
    , rom_GW_lcd_shockey, size_rom_GW_lcd_shockey
    , melody_GW_lcd_shockey, size_melody_GW_lcd_shockey
    , path_segment_lcd_shockey
    , segment_GW_lcd_shockey, size_segment_GW_lcd_shockey
    , segment_info_lcd_shockey
    , path_background_lcd_shockey
    , background_info_lcd_shockey
    , path_console_lcd_shockey
    , console_info_lcd_shockey
    , GW_rom::MANUFACTURER_MCDONALDS
);

