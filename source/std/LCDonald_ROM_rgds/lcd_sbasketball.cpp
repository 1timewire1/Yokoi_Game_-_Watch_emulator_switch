
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sbasketball.h"


const uint8_t rom_GW_lcd_sbasketball[] = {
	0xFF, 0xFF, 0xFE, 0x11
}; const size_t size_rom_GW_lcd_sbasketball = sizeof(rom_GW_lcd_sbasketball)/sizeof(rom_GW_lcd_sbasketball[0]);

const uint8_t melody_GW_lcd_sbasketball[1] = {0}; 
	const size_t size_melody_GW_lcd_sbasketball = 0;

const std::string path_segment_lcd_sbasketball = "gfx/segment_lcd_sbasketball.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sbasketball[] = {
	{ { 0,0,0 }, { 418,56 }, { 67,407 }, { 35,7 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 410,61 }, { 238,954 }, { 9,20 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 452,62 }, { 134,720 }, { 10,19 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 389,61 }, { 238,976 }, { 10,47 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 420,81 }, { 238,924 }, { 16,6 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 439,81 }, { 238,915 }, { 13,7 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 410,86 }, { 238,932 }, { 9,20 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 452,86 }, { 134,741 }, { 10,20 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 241,105 }, { 132,351 }, { 45,46 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 416,105 }, { 67,399 }, { 37,6 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 92,228 }, { 179,306 }, { 45,44 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 238,228 }, { 132,305 }, { 45,44 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 383,228 }, { 179,352 }, { 46,45 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 61,256 }, { 1,416 }, { 104,72 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 209,256 }, { 107,399 }, { 104,72 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 354,256 }, { 149,473 }, { 105,72 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 63,307 }, { 150,655 }, { 101,106 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 208,307 }, { 150,547 }, { 101,106 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 356,307 }, { 134,763 }, { 102,106 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 42,345 }, { 1,565 }, { 145,74 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 188,345 }, { 1,490 }, { 146,73 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 335,346 }, { 1,641 }, { 147,73 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 63,427 }, { 67,335 }, { 63,62 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 235,428 }, { 1,288 }, { 61,62 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 400,429 }, { 1,352 }, { 64,62 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 15,496 }, { 1,870 }, { 131,153 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 218,498 }, { 134,871 }, { 96,152 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 376,498 }, { 1,716 }, { 131,152 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sbasketball = sizeof(segment_GW_lcd_sbasketball)/sizeof(segment_GW_lcd_sbasketball[0]); 
 const uint16_t segment_info_lcd_sbasketball[] = { 256, 1024, 1, 0, 535, 677}; 

const std::string path_background_lcd_sbasketball = "gfx/background_lcd_sbasketball.png";
const uint16_t background_info_lcd_sbasketball[] = { 1024, 1024, 1, 346, 535, 677, 1, 0, 0 }; 


const std::string path_console_lcd_sbasketball = "gfx/console_lcd_sbasketball.png";
const uint16_t console_info_lcd_sbasketball[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_sbasketball (
    "Shadow Basketball", "SBASKETBALL", "2005-03-18"
    , rom_GW_lcd_sbasketball, size_rom_GW_lcd_sbasketball
    , melody_GW_lcd_sbasketball, size_melody_GW_lcd_sbasketball
    , path_segment_lcd_sbasketball
    , segment_GW_lcd_sbasketball, size_segment_GW_lcd_sbasketball
    , segment_info_lcd_sbasketball
    , path_background_lcd_sbasketball
    , background_info_lcd_sbasketball
    , path_console_lcd_sbasketball
    , console_info_lcd_sbasketball
    , GW_rom::MANUFACTURER_MCDONALDS
);

