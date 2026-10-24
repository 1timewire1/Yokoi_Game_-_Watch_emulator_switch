
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_arvolleyball.h"


const uint8_t rom_GW_lcd_arvolleyball[] = {
	0xFF, 0xFF, 0xFE, 0x0B
}; const size_t size_rom_GW_lcd_arvolleyball = sizeof(rom_GW_lcd_arvolleyball)/sizeof(rom_GW_lcd_arvolleyball[0]);

const uint8_t melody_GW_lcd_arvolleyball[1] = {0}; 
	const size_t size_melody_GW_lcd_arvolleyball = 0;

const std::string path_segment_lcd_arvolleyball = "gfx/segment_lcd_arvolleyball.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_arvolleyball[] = {
	{ { 0,0,0 }, { 141,48 }, { 1,901 }, { 45,35 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 189,48 }, { 88,942 }, { 31,37 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 222,48 }, { 91,868 }, { 24,35 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 256,48 }, { 89,986 }, { 30,37 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 297,48 }, { 89,1463 }, { 24,35 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 324,48 }, { 91,905 }, { 29,35 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 358,48 }, { 48,903 }, { 41,33 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 161,104 }, { 1,1630 }, { 108,127 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 277,103 }, { 1,1500 }, { 107,128 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 128,190 }, { 80,1250 }, { 43,42 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 373,190 }, { 1,981 }, { 42,42 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 118,280 }, { 1,938 }, { 42,41 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 257,278 }, { 45,938 }, { 41,41 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 388,278 }, { 45,981 }, { 42,42 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 222,315 }, { 1,1221 }, { 77,71 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 84,396 }, { 54,1061 }, { 49,50 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 241,396 }, { 1,1061 }, { 49,49 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 395,395 }, { 54,1113 }, { 49,52 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 21,498 }, { 1,1167 }, { 52,52 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 230,487 }, { 1,1112 }, { 51,53 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 456,499 }, { 55,1167 }, { 52,52 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 33,629 }, { 1,1025 }, { 56,34 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 41,562 }, { 1,1904 }, { 123,143 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 201,560 }, { 1,1294 }, { 121,57 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 228,554 }, { 1,1353 }, { 86,145 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 441,631 }, { 59,1025 }, { 56,34 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 368,562 }, { 1,1759 }, { 123,143 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_arvolleyball = sizeof(segment_GW_lcd_arvolleyball)/sizeof(segment_GW_lcd_arvolleyball[0]); 
 const uint16_t segment_info_lcd_arvolleyball[] = { 128, 2048, 1, 0, 560, 718}; 

const std::string path_background_lcd_arvolleyball = "gfx/background_lcd_arvolleyball.png";
const uint16_t background_info_lcd_arvolleyball[] = { 1024, 1024, 1, 305, 560, 718, 1, 0, 0 }; 


const std::string path_console_lcd_arvolleyball = "gfx/console_lcd_arvolleyball.png";
const uint16_t console_info_lcd_arvolleyball[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_arvolleyball (
    "Amy & Rouge Volleyball", "ARVOLLEYBALL", "2004-12-03"
    , rom_GW_lcd_arvolleyball, size_rom_GW_lcd_arvolleyball
    , melody_GW_lcd_arvolleyball, size_melody_GW_lcd_arvolleyball
    , path_segment_lcd_arvolleyball
    , segment_GW_lcd_arvolleyball, size_segment_GW_lcd_arvolleyball
    , segment_info_lcd_arvolleyball
    , path_background_lcd_arvolleyball
    , background_info_lcd_arvolleyball
    , path_console_lcd_arvolleyball
    , console_info_lcd_arvolleyball
    , GW_rom::MANUFACTURER_MCDONALDS
);

