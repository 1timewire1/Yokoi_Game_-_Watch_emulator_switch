
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_cflower2.h"


const uint8_t rom_GW_lcd_cflower2[] = {
	0xFF, 0xFF, 0xFE, 0x13
}; const size_t size_rom_GW_lcd_cflower2 = sizeof(rom_GW_lcd_cflower2)/sizeof(rom_GW_lcd_cflower2[0]);

const uint8_t melody_GW_lcd_cflower2[1] = {0}; 
	const size_t size_melody_GW_lcd_cflower2 = 0;

const std::string path_segment_lcd_cflower2 = "gfx/segment_lcd_cflower2.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_cflower2[] = {
	{ { 0,0,0 }, { 31,5 }, { 1,352 }, { 41,42 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 78,5 }, { 1,145 }, { 41,42 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 125,5 }, { 44,145 }, { 41,42 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 93,105 }, { 60,450 }, { 57,43 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 191,106 }, { 58,638 }, { 65,44 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 314,106 }, { 60,399 }, { 47,49 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 84,177 }, { 61,309 }, { 57,38 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 206,179 }, { 50,349 }, { 46,48 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 322,180 }, { 60,495 }, { 60,41 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 66,264 }, { 1,396 }, { 47,49 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 200,263 }, { 1,447 }, { 57,43 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 341,263 }, { 1,309 }, { 58,38 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 47,331 }, { 1,269 }, { 57,38 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 104,324 }, { 1,629 }, { 55,53 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 162,325 }, { 58,584 }, { 54,52 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 217,331 }, { 56,538 }, { 61,44 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 284,328 }, { 1,576 }, { 53,51 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 343,330 }, { 1,492 }, { 57,44 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 58,383 }, { 1,684 }, { 93,110 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 175,385 }, { 1,908 }, { 98,115 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 295,386 }, { 1,796 }, { 94,110 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 50,506 }, { 1,540 }, { 35,34 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 95,506 }, { 1,189 }, { 49,38 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 147,506 }, { 60,269 }, { 50,38 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 198,506 }, { 1,229 }, { 50,38 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 251,506 }, { 53,229 }, { 50,38 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 302,506 }, { 52,189 }, { 49,38 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 357,506 }, { 87,153 }, { 35,34 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_cflower2 = sizeof(segment_GW_lcd_cflower2)/sizeof(segment_GW_lcd_cflower2[0]); 
 const uint16_t segment_info_lcd_cflower2[] = { 128, 1024, 1, 0, 434, 566}; 

const std::string path_background_lcd_cflower2 = "gfx/background_lcd_cflower2.png";
const uint16_t background_info_lcd_cflower2[] = { 512, 1024, 1, 457, 434, 566, 1, 0, 0 }; 


const std::string path_console_lcd_cflower2 = "gfx/console_lcd_cflower2.png";
const uint16_t console_info_lcd_cflower2[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_cflower2 (
    "Cream Flower Catch", "CFLOWER2", "2004-12-03"
    , rom_GW_lcd_cflower2, size_rom_GW_lcd_cflower2
    , melody_GW_lcd_cflower2, size_melody_GW_lcd_cflower2
    , path_segment_lcd_cflower2
    , segment_GW_lcd_cflower2, size_segment_GW_lcd_cflower2
    , segment_info_lcd_cflower2
    , path_background_lcd_cflower2
    , background_info_lcd_cflower2
    , path_console_lcd_cflower2
    , console_info_lcd_cflower2
    , GW_rom::MANUFACTURER_MCDONALDS
);

