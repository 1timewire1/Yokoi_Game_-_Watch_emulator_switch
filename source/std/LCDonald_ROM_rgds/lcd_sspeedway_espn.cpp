
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sspeedway.h"
#include "lcd_sspeedway_espn.h"



const uint8_t rom_GW_lcd_sspeedway_espn[] = {
	0xFF, 0xFF, 0xFE, 0x05
}; const size_t size_rom_GW_lcd_sspeedway_espn = sizeof(rom_GW_lcd_sspeedway_espn)/sizeof(rom_GW_lcd_sspeedway_espn[0]);

const uint8_t melody_GW_lcd_sspeedway_espn[1] = {0}; 
	const size_t size_melody_GW_lcd_sspeedway_espn = 0;

const std::string path_segment_lcd_sspeedway_espn = "gfx/segment_lcd_sspeedway_espn.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sspeedway_espn[] = {
	{ { 0,0,0 }, { 35,16 }, { 192,973 }, { 62,50 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 94,17 }, { 192,870 }, { 61,49 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 151,16 }, { 192,921 }, { 62,50 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 142,84 }, { 191,791 }, { 61,31 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 207,84 }, { 191,759 }, { 62,30 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 275,83 }, { 183,620 }, { 60,31 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 166,125 }, { 191,824 }, { 62,44 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 235,124 }, { 146,322 }, { 83,42 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 325,122 }, { 146,280 }, { 80,40 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 164,177 }, { 151,366 }, { 91,66 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 276,171 }, { 151,434 }, { 98,69 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 381,171 }, { 1,222 }, { 128,64 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 122,249 }, { 1,288 }, { 143,105 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 274,248 }, { 1,395 }, { 148,108 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 434,243 }, { 1,505 }, { 171,107 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 220,349 }, { 183,653 }, { 67,92 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 414,336 }, { 174,509 }, { 73,103 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 33,361 }, { 1,884 }, { 189,139 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 262,370 }, { 1,614 }, { 180,131 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 487,367 }, { 1,747 }, { 188,135 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sspeedway_espn = sizeof(segment_GW_lcd_sspeedway_espn)/sizeof(segment_GW_lcd_sspeedway_espn[0]); 
 const uint16_t segment_info_lcd_sspeedway_espn[] = { 256, 1024, 1, 0, 689, 533}; 

const std::string path_background_lcd_sspeedway_espn = "gfx/background_lcd_sspeedway_espn.png";
const uint16_t background_info_lcd_sspeedway_espn[] = { 1024, 1024, 1, 490, 689, 533, 1, 0, 0 }; 


const std::string path_console_lcd_sspeedway_espn = "gfx/console_lcd_sspeedway_espn.png";
const uint16_t console_info_lcd_sspeedway_espn[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_sspeedway_espn (
    "ESPN25: Tony Stewart - Racing", "SSPEEDWAY_ESPN", "2004-04-30"
    , rom_GW_lcd_sspeedway_espn, size_rom_GW_lcd_sspeedway_espn
    , melody_GW_lcd_sspeedway_espn, size_melody_GW_lcd_sspeedway_espn
    , path_segment_lcd_sspeedway_espn
    , segment_GW_lcd_sspeedway_espn, size_segment_GW_lcd_sspeedway_espn
    , segment_info_lcd_sspeedway_espn
    , path_background_lcd_sspeedway_espn
    , background_info_lcd_sspeedway_espn
    , path_console_lcd_sspeedway_espn
    , console_info_lcd_sspeedway_espn
    , GW_rom::MANUFACTURER_MCDONALDS
);

