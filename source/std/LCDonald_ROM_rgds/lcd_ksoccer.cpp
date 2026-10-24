
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_ksoccer.h"


const uint8_t rom_GW_lcd_ksoccer[] = {
	0xFF, 0xFF, 0xFE, 0x03
}; const size_t size_rom_GW_lcd_ksoccer = sizeof(rom_GW_lcd_ksoccer)/sizeof(rom_GW_lcd_ksoccer[0]);

const uint8_t melody_GW_lcd_ksoccer[1] = {0}; 
	const size_t size_melody_GW_lcd_ksoccer = 0;

const std::string path_segment_lcd_ksoccer = "gfx/segment_lcd_ksoccer.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_ksoccer[] = {
	{ { 0,0,0 }, { 24,38 }, { 145,133 }, { 68,47 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 95,38 }, { 159,275 }, { 69,47 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 167,38 }, { 140,84 }, { 68,47 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 156,152 }, { 191,1 }, { 35,35 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 231,152 }, { 215,180 }, { 35,35 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 303,152 }, { 215,143 }, { 35,35 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 120,208 }, { 210,38 }, { 45,45 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 227,208 }, { 145,36 }, { 44,46 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 328,208 }, { 210,85 }, { 45,46 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 81,279 }, { 193,383 }, { 57,57 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 218,279 }, { 193,324 }, { 57,57 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 350,279 }, { 193,217 }, { 56,56 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 38,368 }, { 1,6 }, { 71,71 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 216,368 }, { 74,8 }, { 69,69 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 380,368 }, { 159,442 }, { 69,69 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 24,443 }, { 1,393 }, { 156,118 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 181,442 }, { 1,79 }, { 137,101 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 316,443 }, { 1,273 }, { 156,118 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 153,540 }, { 1,182 }, { 190,89 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_ksoccer = sizeof(segment_GW_lcd_ksoccer)/sizeof(segment_GW_lcd_ksoccer[0]); 
 const uint16_t segment_info_lcd_ksoccer[] = { 256, 512, 1, 0, 497, 631}; 

const std::string path_background_lcd_ksoccer = "gfx/background_lcd_ksoccer.png";
const uint16_t background_info_lcd_ksoccer[] = { 512, 1024, 1, 392, 497, 631, 1, 0, 0 }; 


const std::string path_console_lcd_ksoccer = "gfx/console_lcd_ksoccer.png";
const uint16_t console_info_lcd_ksoccer[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_ksoccer (
    "Knuckles Soccer", "KSOCCER", "2003-06-27"
    , rom_GW_lcd_ksoccer, size_rom_GW_lcd_ksoccer
    , melody_GW_lcd_ksoccer, size_melody_GW_lcd_ksoccer
    , path_segment_lcd_ksoccer
    , segment_GW_lcd_ksoccer, size_segment_GW_lcd_ksoccer
    , segment_info_lcd_ksoccer
    , path_background_lcd_ksoccer
    , background_info_lcd_ksoccer
    , path_console_lcd_ksoccer
    , console_info_lcd_ksoccer
    , GW_rom::MANUFACTURER_MCDONALDS
);

