
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sgrinder.h"


const uint8_t rom_GW_lcd_sgrinder[] = {
	0xFF, 0xFF, 0xFE, 0x04
}; const size_t size_rom_GW_lcd_sgrinder = sizeof(rom_GW_lcd_sgrinder)/sizeof(rom_GW_lcd_sgrinder[0]);

const uint8_t melody_GW_lcd_sgrinder[1] = {0}; 
	const size_t size_melody_GW_lcd_sgrinder = 0;

const std::string path_segment_lcd_sgrinder = "gfx/segment_lcd_sgrinder.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sgrinder[] = {
	{ { 0,0,0 }, { 97,37 }, { 85,263 }, { 90,88 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 565,47 }, { 192,875 }, { 36,53 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 603,47 }, { 177,425 }, { 35,53 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 641,47 }, { 214,425 }, { 35,53 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 47,136 }, { 1,691 }, { 134,172 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 28,262 }, { 1,865 }, { 189,158 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 209,306 }, { 85,353 }, { 82,125 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 285,323 }, { 165,736 }, { 82,127 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 360,345 }, { 165,608 }, { 82,126 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 26,349 }, { 1,588 }, { 162,101 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 434,364 }, { 165,480 }, { 82,126 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 508,383 }, { 1,333 }, { 82,125 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 27,450 }, { 85,524 }, { 66,62 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 584,404 }, { 1,460 }, { 82,126 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 659,423 }, { 192,930 }, { 59,93 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sgrinder = sizeof(segment_GW_lcd_sgrinder)/sizeof(segment_GW_lcd_sgrinder[0]); 
 const uint16_t segment_info_lcd_sgrinder[] = { 256, 1024, 1, 0, 718, 557}; 

const std::string path_background_lcd_sgrinder = "gfx/background_lcd_sgrinder.png";
const uint16_t background_info_lcd_sgrinder[] = { 1024, 1024, 1, 466, 718, 557, 1, 0, 0 }; 


const std::string path_console_lcd_sgrinder = "gfx/console_lcd_sgrinder.png";
const uint16_t console_info_lcd_sgrinder[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_sgrinder (
    "Shadow Grinder", "SGRINDER", "2003-06-27"
    , rom_GW_lcd_sgrinder, size_rom_GW_lcd_sgrinder
    , melody_GW_lcd_sgrinder, size_melody_GW_lcd_sgrinder
    , path_segment_lcd_sgrinder
    , segment_GW_lcd_sgrinder, size_segment_GW_lcd_sgrinder
    , segment_info_lcd_sgrinder
    , path_background_lcd_sgrinder
    , background_info_lcd_sgrinder
    , path_console_lcd_sgrinder
    , console_info_lcd_sgrinder
    , GW_rom::MANUFACTURER_MCDONALDS
);

