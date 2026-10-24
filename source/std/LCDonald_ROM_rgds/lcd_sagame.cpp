
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sagame.h"


const uint8_t rom_GW_lcd_sagame[] = {
	0xFF, 0xFF, 0xFE, 0x01
}; const size_t size_rom_GW_lcd_sagame = sizeof(rom_GW_lcd_sagame)/sizeof(rom_GW_lcd_sagame[0]);

const uint8_t melody_GW_lcd_sagame[1] = {0}; 
	const size_t size_melody_GW_lcd_sagame = 0;

const std::string path_segment_lcd_sagame = "gfx/segment_lcd_sagame.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sagame[] = {
	{ { 0,0,0 }, { 610,14 }, { 1,11 }, { 39,57 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 651,15 }, { 191,153 }, { 39,56 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 692,14 }, { 42,11 }, { 39,57 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 36,17 }, { 1,289 }, { 110,152 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 240,84 }, { 149,460 }, { 105,104 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 431,84 }, { 1,184 }, { 105,103 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 625,84 }, { 113,211 }, { 104,104 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 7,195 }, { 1,1677 }, { 186,184 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 199,195 }, { 1,1863 }, { 187,184 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 391,195 }, { 1,1491 }, { 186,184 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 585,195 }, { 1,751 }, { 185,183 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 26,242 }, { 1,443 }, { 146,121 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 234,241 }, { 108,98 }, { 81,111 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 7,388 }, { 1,1121 }, { 186,183 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 199,388 }, { 1,1306 }, { 187,183 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 391,388 }, { 1,936 }, { 186,183 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 585,388 }, { 1,566 }, { 185,183 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 40,431 }, { 113,317 }, { 123,124 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 238,431 }, { 1,70 }, { 80,112 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sagame = sizeof(segment_GW_lcd_sagame)/sizeof(segment_GW_lcd_sagame[0]); 
 const uint16_t segment_info_lcd_sagame[] = { 256, 2048, 1, 0, 785, 604}; 

const std::string path_background_lcd_sagame = "gfx/background_lcd_sagame.png";
const uint16_t background_info_lcd_sagame[] = { 1024, 1024, 1, 419, 785, 604, 1, 0, 0 }; 


const std::string path_console_lcd_sagame = "gfx/console_lcd_sagame.png";
const uint16_t console_info_lcd_sagame[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_sagame (
    "Sonic Action Game", "SAGAME", "2003-06-27"
    , rom_GW_lcd_sagame, size_rom_GW_lcd_sagame
    , melody_GW_lcd_sagame, size_melody_GW_lcd_sagame
    , path_segment_lcd_sagame
    , segment_GW_lcd_sagame, size_segment_GW_lcd_sagame
    , segment_info_lcd_sagame
    , path_background_lcd_sagame
    , background_info_lcd_sagame
    , path_console_lcd_sagame
    , console_info_lcd_sagame
    , GW_rom::MANUFACTURER_MCDONALDS
);

