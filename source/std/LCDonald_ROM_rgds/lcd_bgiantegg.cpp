
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_bgiantegg.h"


const uint8_t rom_GW_lcd_bgiantegg[] = {
	0xFF, 0xFF, 0xFE, 0x14
}; const size_t size_rom_GW_lcd_bgiantegg = sizeof(rom_GW_lcd_bgiantegg)/sizeof(rom_GW_lcd_bgiantegg[0]);

const uint8_t melody_GW_lcd_bgiantegg[1] = {0}; 
	const size_t size_melody_GW_lcd_bgiantegg = 0;

const std::string path_segment_lcd_bgiantegg = "gfx/segment_lcd_bgiantegg.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_bgiantegg[] = {
	{ { 0,0,0 }, { 30,26 }, { 95,382 }, { 21,29 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 55,27 }, { 70,407 }, { 23,31 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 84,26 }, { 50,460 }, { 38,52 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 122,17 }, { 1,916 }, { 123,107 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 123,157 }, { 32,402 }, { 25,23 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 167,156 }, { 96,822 }, { 26,25 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 212,156 }, { 95,413 }, { 25,25 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 118,152 }, { 90,440 }, { 36,34 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 163,151 }, { 90,476 }, { 37,36 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 206,151 }, { 77,639 }, { 38,36 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 107,214 }, { 1,413 }, { 29,27 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 165,213 }, { 92,725 }, { 29,28 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 223,213 }, { 96,849 }, { 30,29 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 94,199 }, { 62,572 }, { 56,56 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 152,199 }, { 62,514 }, { 56,56 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 209,199 }, { 1,571 }, { 59,57 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 84,271 }, { 92,788 }, { 34,31 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 158,270 }, { 1,442 }, { 33,31 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 232,269 }, { 36,427 }, { 32,31 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 77,263 }, { 77,677 }, { 49,46 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 152,263 }, { 1,523 }, { 47,46 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 225,262 }, { 1,475 }, { 47,46 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 116,290 }, { 92,755 }, { 34,31 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 198,290 }, { 96,880 }, { 28,34 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 35,308 }, { 1,821 }, { 93,93 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 136,310 }, { 1,630 }, { 74,93 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 225,310 }, { 1,725 }, { 89,94 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_bgiantegg = sizeof(segment_GW_lcd_bgiantegg)/sizeof(segment_GW_lcd_bgiantegg[0]); 
 const uint16_t segment_info_lcd_bgiantegg[] = { 128, 1024, 1, 0, 350, 435}; 

const std::string path_background_lcd_bgiantegg = "gfx/background_lcd_bgiantegg.png";
const uint16_t background_info_lcd_bgiantegg[] = { 512, 512, 1, 76, 350, 435, 1, 0, 0 }; 


const std::string path_console_lcd_bgiantegg = "gfx/console_lcd_bgiantegg.png";
const uint16_t console_info_lcd_bgiantegg[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_bgiantegg (
    "Billy's Giant Egg", "BGIANTEGG", "2004-12-03"
    , rom_GW_lcd_bgiantegg, size_rom_GW_lcd_bgiantegg
    , melody_GW_lcd_bgiantegg, size_melody_GW_lcd_bgiantegg
    , path_segment_lcd_bgiantegg
    , segment_GW_lcd_bgiantegg, size_segment_GW_lcd_bgiantegg
    , segment_info_lcd_bgiantegg
    , path_background_lcd_bgiantegg
    , background_info_lcd_bgiantegg
    , path_console_lcd_bgiantegg
    , console_info_lcd_bgiantegg
    , GW_rom::MANUFACTURER_MCDONALDS
);

