
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_cflower.h"


const uint8_t rom_GW_lcd_cflower[] = {
	0xFF, 0xFF, 0xFE, 0x0C
}; const size_t size_rom_GW_lcd_cflower = sizeof(rom_GW_lcd_cflower)/sizeof(rom_GW_lcd_cflower[0]);

const uint8_t melody_GW_lcd_cflower[1] = {0}; 
	const size_t size_melody_GW_lcd_cflower = 0;

const std::string path_segment_lcd_cflower = "gfx/segment_lcd_cflower.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_cflower[] = {
	{ { 0,0,0 }, { 13,11 }, { 93,89 }, { 32,32 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 50,11 }, { 1,59 }, { 32,32 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 86,11 }, { 35,58 }, { 32,32 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 54,93 }, { 49,191 }, { 46,34 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 136,96 }, { 1,156 }, { 46,32 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 240,93 }, { 82,311 }, { 37,38 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 47,159 }, { 1,93 }, { 44,29 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 145,155 }, { 82,271 }, { 36,38 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 241,161 }, { 49,123 }, { 46,32 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 31,222 }, { 82,231 }, { 36,38 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 140,223 }, { 82,351 }, { 45,34 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 256,223 }, { 47,92 }, { 44,29 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 18,288 }, { 1,124 }, { 44,30 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 138,282 }, { 49,157 }, { 47,32 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 254,283 }, { 1,190 }, { 46,35 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 88,282 }, { 82,450 }, { 43,61 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 185,282 }, { 82,387 }, { 43,61 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 21,324 }, { 1,417 }, { 79,94 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 120,327 }, { 1,227 }, { 79,92 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 211,325 }, { 1,321 }, { 79,94 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_cflower = sizeof(segment_GW_lcd_cflower)/sizeof(segment_GW_lcd_cflower[0]); 
 const uint16_t segment_info_lcd_cflower[] = { 128, 512, 1, 0, 322, 430}; 

const std::string path_background_lcd_cflower = "gfx/background_lcd_cflower.png";
const uint16_t background_info_lcd_cflower[] = { 512, 512, 1, 81, 322, 430, 1, 0, 0 }; 


const std::string path_console_lcd_cflower = "gfx/console_lcd_cflower.png";
const uint16_t console_info_lcd_cflower[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_cflower (
    "Cream Flower Catch", "CFLOWER", "2005-03-18"
    , rom_GW_lcd_cflower, size_rom_GW_lcd_cflower
    , melody_GW_lcd_cflower, size_melody_GW_lcd_cflower
    , path_segment_lcd_cflower
    , segment_GW_lcd_cflower, size_segment_GW_lcd_cflower
    , segment_info_lcd_cflower
    , path_background_lcd_cflower
    , background_info_lcd_cflower
    , path_console_lcd_cflower
    , console_info_lcd_cflower
    , GW_rom::MANUFACTURER_MCDONALDS
);

