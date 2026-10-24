
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sxtremeboard.h"


const uint8_t rom_GW_lcd_sxtremeboard[] = {
	0xFF, 0xFF, 0xFE, 0x07
}; const size_t size_rom_GW_lcd_sxtremeboard = sizeof(rom_GW_lcd_sxtremeboard)/sizeof(rom_GW_lcd_sxtremeboard[0]);

const uint8_t melody_GW_lcd_sxtremeboard[1] = {0}; 
	const size_t size_melody_GW_lcd_sxtremeboard = 0;

const std::string path_segment_lcd_sxtremeboard = "gfx/segment_lcd_sxtremeboard.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sxtremeboard[] = {
	{ { 0,0,0 }, { 0,15 }, { 1,816 }, { 109,104 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 116,48 }, { 1,58 }, { 46,46 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 192,0 }, { 1,384 }, { 98,98 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 318,48 }, { 1,106 }, { 47,46 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 374,14 }, { 1,708 }, { 104,106 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 171,80 }, { 101,476 }, { 26,27 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 283,80 }, { 101,448 }, { 26,26 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 211,99 }, { 83,487 }, { 16,16 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 235,112 }, { 118,786 }, { 9,10 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 252,99 }, { 108,798 }, { 16,16 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 201,118 }, { 102,109 }, { 21,21 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 229,128 }, { 102,155 }, { 22,21 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 257,118 }, { 102,132 }, { 22,21 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 31,119 }, { 83,537 }, { 33,32 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 167,122 }, { 102,206 }, { 25,26 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 288,122 }, { 101,420 }, { 26,26 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 411,126 }, { 83,505 }, { 35,30 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 0,144 }, { 1,301 }, { 113,81 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 111,120 }, { 1,154 }, { 48,49 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 227,154 }, { 102,178 }, { 25,26 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 321,121 }, { 51,155 }, { 49,48 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 375,144 }, { 1,608 }, { 105,98 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 102,185 }, { 83,571 }, { 33,35 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 353,179 }, { 1,23 }, { 30,33 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 65,222 }, { 1,922 }, { 115,101 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 215,188 }, { 51,105 }, { 49,48 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 303,217 }, { 1,205 }, { 96,94 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 189,228 }, { 99,267 }, { 28,32 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 258,229 }, { 99,234 }, { 28,31 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 200,239 }, { 1,484 }, { 80,122 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sxtremeboard = sizeof(segment_GW_lcd_sxtremeboard)/sizeof(segment_GW_lcd_sxtremeboard[0]); 
 const uint16_t segment_info_lcd_sxtremeboard[] = { 128, 1024, 1, 0, 480, 361}; 

const std::string path_background_lcd_sxtremeboard = "gfx/background_lcd_sxtremeboard.png";
const uint16_t background_info_lcd_sxtremeboard[] = { 512, 512, 1, 150, 480, 361, 1, 0, 0 }; 


const std::string path_console_lcd_sxtremeboard = "gfx/console_lcd_sxtremeboard.png";
const uint16_t console_info_lcd_sxtremeboard[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_sxtremeboard (
    "Sonic Extreme Boarding", "SXTREMEBOARD", "2004-12-03"
    , rom_GW_lcd_sxtremeboard, size_rom_GW_lcd_sxtremeboard
    , melody_GW_lcd_sxtremeboard, size_melody_GW_lcd_sxtremeboard
    , path_segment_lcd_sxtremeboard
    , segment_GW_lcd_sxtremeboard, size_segment_GW_lcd_sxtremeboard
    , segment_info_lcd_sxtremeboard
    , path_background_lcd_sxtremeboard
    , background_info_lcd_sxtremeboard
    , path_console_lcd_sxtremeboard
    , console_info_lcd_sxtremeboard
    , GW_rom::MANUFACTURER_MCDONALDS
);

