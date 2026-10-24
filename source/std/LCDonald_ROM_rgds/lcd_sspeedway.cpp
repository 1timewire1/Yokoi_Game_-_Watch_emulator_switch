
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sspeedway.h"


const uint8_t rom_GW_lcd_sspeedway[] = {
	0xFF, 0xFF, 0xFE, 0x05
}; const size_t size_rom_GW_lcd_sspeedway = sizeof(rom_GW_lcd_sspeedway)/sizeof(rom_GW_lcd_sspeedway[0]);

const uint8_t melody_GW_lcd_sspeedway[1] = {0}; 
	const size_t size_melody_GW_lcd_sspeedway = 0;

const std::string path_segment_lcd_sspeedway = "gfx/segment_lcd_sspeedway.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sspeedway[] = {
	{ { 0,0,0 }, { 35,16 }, { 118,389 }, { 62,50 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 94,17 }, { 118,338 }, { 61,49 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 151,16 }, { 182,389 }, { 62,50 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 378,81 }, { 181,314 }, { 51,31 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 443,81 }, { 1,350 }, { 51,30 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 507,81 }, { 54,350 }, { 43,30 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 294,113 }, { 178,779 }, { 75,42 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 380,113 }, { 178,735 }, { 74,42 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 473,115 }, { 181,347 }, { 60,40 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 199,161 }, { 147,563 }, { 105,59 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 337,161 }, { 147,500 }, { 90,61 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 461,161 }, { 147,441 }, { 82,57 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 114,221 }, { 1,462 }, { 144,81 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 309,226 }, { 141,624 }, { 111,79 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 464,227 }, { 1,382 }, { 115,78 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 229,309 }, { 179,823 }, { 67,93 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 423,311 }, { 179,918 }, { 76,105 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 51,320 }, { 1,865 }, { 176,158 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 296,318 }, { 1,545 }, { 138,158 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 501,318 }, { 1,705 }, { 175,158 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sspeedway = sizeof(segment_GW_lcd_sspeedway)/sizeof(segment_GW_lcd_sspeedway[0]); 
 const uint16_t segment_info_lcd_sspeedway[] = { 256, 1024, 1, 0, 689, 533}; 

const std::string path_background_lcd_sspeedway = "gfx/background_lcd_sspeedway.png";
const uint16_t background_info_lcd_sspeedway[] = { 1024, 1024, 1, 490, 689, 533, 1, 0, 0 }; 


const std::string path_console_lcd_sspeedway = "gfx/console_lcd_sspeedway.png";
const uint16_t console_info_lcd_sspeedway[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_sspeedway (
    "Sonic's Speedway", "SSPEEDWAY", "2003-06-27"
    , rom_GW_lcd_sspeedway, size_rom_GW_lcd_sspeedway
    , melody_GW_lcd_sspeedway, size_melody_GW_lcd_sspeedway
    , path_segment_lcd_sspeedway
    , segment_GW_lcd_sspeedway, size_segment_GW_lcd_sspeedway
    , segment_info_lcd_sspeedway
    , path_background_lcd_sspeedway
    , background_info_lcd_sspeedway
    , path_console_lcd_sspeedway
    , console_info_lcd_sspeedway
    , GW_rom::MANUFACTURER_MCDONALDS
);

