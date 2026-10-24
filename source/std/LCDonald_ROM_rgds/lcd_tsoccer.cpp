
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_tsoccer.h"


const uint8_t rom_GW_lcd_tsoccer[] = {
	0xFF, 0xFF, 0xFE, 0x08
}; const size_t size_rom_GW_lcd_tsoccer = sizeof(rom_GW_lcd_tsoccer)/sizeof(rom_GW_lcd_tsoccer[0]);

const uint8_t melody_GW_lcd_tsoccer[1] = {0}; 
	const size_t size_melody_GW_lcd_tsoccer = 0;

const std::string path_segment_lcd_tsoccer = "gfx/segment_lcd_tsoccer.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_tsoccer[] = {
	{ { 0,0,0 }, { 50,34 }, { 1,91 }, { 75,54 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 134,34 }, { 1,35 }, { 75,54 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 216,34 }, { 1,147 }, { 76,54 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 203,158 }, { 208,138 }, { 40,39 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 285,158 }, { 208,97 }, { 39,39 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 364,158 }, { 208,56 }, { 39,39 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 161,216 }, { 203,179 }, { 51,52 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 273,216 }, { 203,233 }, { 52,52 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 380,217 }, { 79,149 }, { 51,52 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 121,293 }, { 78,81 }, { 63,64 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 267,294 }, { 143,20 }, { 63,63 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 414,293 }, { 78,15 }, { 63,64 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 112,386 }, { 143,85 }, { 63,64 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 267,390 }, { 135,151 }, { 65,66 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 442,390 }, { 135,219 }, { 66,66 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 87,472 }, { 1,203 }, { 132,96 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 265,464 }, { 137,287 }, { 78,110 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 380,472 }, { 1,301 }, { 134,96 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 181,591 }, { 1,399 }, { 236,112 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_tsoccer = sizeof(segment_GW_lcd_tsoccer)/sizeof(segment_GW_lcd_tsoccer[0]); 
 const uint16_t segment_info_lcd_tsoccer[] = { 256, 512, 1, 0, 577, 703}; 

const std::string path_background_lcd_tsoccer = "gfx/background_lcd_tsoccer.png";
const uint16_t background_info_lcd_tsoccer[] = { 1024, 1024, 1, 320, 577, 703, 1, 0, 0 }; 


const std::string path_console_lcd_tsoccer = "gfx/console_lcd_tsoccer.png";
const uint16_t console_info_lcd_tsoccer[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_tsoccer (
    "Tails Soccer", "TSOCCER", "2005-03-18"
    , rom_GW_lcd_tsoccer, size_rom_GW_lcd_tsoccer
    , melody_GW_lcd_tsoccer, size_melody_GW_lcd_tsoccer
    , path_segment_lcd_tsoccer
    , segment_GW_lcd_tsoccer, size_segment_GW_lcd_tsoccer
    , segment_info_lcd_tsoccer
    , path_background_lcd_tsoccer
    , background_info_lcd_tsoccer
    , path_console_lcd_tsoccer
    , console_info_lcd_tsoccer
    , GW_rom::MANUFACTURER_MCDONALDS
);

