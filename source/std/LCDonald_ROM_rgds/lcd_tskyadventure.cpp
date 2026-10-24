
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_tskyadventure.h"


const uint8_t rom_GW_lcd_tskyadventure[] = {
	0xFF, 0xFF, 0xFE, 0x0F
}; const size_t size_rom_GW_lcd_tskyadventure = sizeof(rom_GW_lcd_tskyadventure)/sizeof(rom_GW_lcd_tskyadventure[0]);

const uint8_t melody_GW_lcd_tskyadventure[1] = {0}; 
	const size_t size_melody_GW_lcd_tskyadventure = 0;

const std::string path_segment_lcd_tskyadventure = "gfx/segment_lcd_tskyadventure.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_tskyadventure[] = {
	{ { 0,0,0 }, { 9,7 }, { 97,203 }, { 27,28 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 37,7 }, { 66,174 }, { 27,28 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 66,7 }, { 33,173 }, { 26,28 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 207,7 }, { 1,163 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 237,7 }, { 95,173 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 266,7 }, { 61,144 }, { 25,28 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 76,92 }, { 1,193 }, { 30,28 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 134,92 }, { 93,233 }, { 30,29 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 190,92 }, { 66,204 }, { 29,27 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 81,121 }, { 103,342 }, { 21,20 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 138,121 }, { 103,321 }, { 21,19 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 195,121 }, { 33,233 }, { 22,19 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 65,144 }, { 86,399 }, { 34,34 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 130,144 }, { 86,364 }, { 35,33 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 199,143 }, { 86,435 }, { 35,34 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 66,180 }, { 28,146 }, { 28,25 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 134,179 }, { 88,119 }, { 27,25 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 203,178 }, { 88,146 }, { 28,25 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 46,207 }, { 59,264 }, { 42,39 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 130,207 }, { 86,471 }, { 41,40 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 204,206 }, { 59,305 }, { 42,40 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 53,249 }, { 59,233 }, { 32,29 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 133,250 }, { 1,223 }, { 30,29 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 214,249 }, { 33,203 }, { 31,28 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 7,284 }, { 1,430 }, { 83,81 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 121,282 }, { 1,254 }, { 56,91 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 208,282 }, { 1,347 }, { 83,81 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_tskyadventure = sizeof(segment_GW_lcd_tskyadventure)/sizeof(segment_GW_lcd_tskyadventure[0]); 
 const uint16_t segment_info_lcd_tskyadventure[] = { 128, 512, 1, 0, 299, 380}; 

const std::string path_background_lcd_tskyadventure = "gfx/background_lcd_tskyadventure.png";
const uint16_t background_info_lcd_tskyadventure[] = { 512, 512, 1, 131, 299, 380, 1, 0, 0 }; 


const std::string path_console_lcd_tskyadventure = "gfx/console_lcd_tskyadventure.png";
const uint16_t console_info_lcd_tskyadventure[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_tskyadventure (
    "Tails Sky Adventure", "TSKYADVENTURE", "2004-12-03"
    , rom_GW_lcd_tskyadventure, size_rom_GW_lcd_tskyadventure
    , melody_GW_lcd_tskyadventure, size_melody_GW_lcd_tskyadventure
    , path_segment_lcd_tskyadventure
    , segment_GW_lcd_tskyadventure, size_segment_GW_lcd_tskyadventure
    , segment_info_lcd_tskyadventure
    , path_background_lcd_tskyadventure
    , background_info_lcd_tskyadventure
    , path_console_lcd_tskyadventure
    , console_info_lcd_tskyadventure
    , GW_rom::MANUFACTURER_MCDONALDS
);

