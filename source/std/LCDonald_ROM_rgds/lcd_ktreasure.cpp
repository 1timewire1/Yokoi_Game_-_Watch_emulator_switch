
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_ktreasure.h"


const uint8_t rom_GW_lcd_ktreasure[] = {
	0xFF, 0xFF, 0xFE, 0x10
}; const size_t size_rom_GW_lcd_ktreasure = sizeof(rom_GW_lcd_ktreasure)/sizeof(rom_GW_lcd_ktreasure[0]);

const uint8_t melody_GW_lcd_ktreasure[1] = {0}; 
	const size_t size_melody_GW_lcd_ktreasure = 0;

const std::string path_segment_lcd_ktreasure = "gfx/segment_lcd_ktreasure.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_ktreasure[] = {
	{ { 0,0,0 }, { 212,15 }, { 1,417 }, { 172,94 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 203,120 }, { 413,232 }, { 89,30 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 298,120 }, { 413,200 }, { 89,30 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 6,198 }, { 164,104 }, { 82,74 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 90,174 }, { 277,274 }, { 95,106 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 209,153 }, { 167,180 }, { 79,116 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 301,153 }, { 85,181 }, { 80,116 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 409,183 }, { 248,164 }, { 80,108 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 487,193 }, { 164,28 }, { 82,74 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 206,268 }, { 413,168 }, { 89,30 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 301,268 }, { 423,136 }, { 88,30 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 7,341 }, { 248,88 }, { 82,74 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 83,294 }, { 349,382 }, { 99,129 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 210,302 }, { 83,63 }, { 79,116 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 303,302 }, { 1,42 }, { 79,116 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 402,312 }, { 85,299 }, { 84,116 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 487,334 }, { 330,188 }, { 81,74 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 205,420 }, { 1,10 }, { 89,30 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 303,420 }, { 332,136 }, { 89,30 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 82,442 }, { 175,298 }, { 100,118 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 212,456 }, { 374,264 }, { 79,116 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 305,456 }, { 1,160 }, { 80,116 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 407,438 }, { 1,278 }, { 82,137 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 486,477 }, { 248,12 }, { 82,74 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 207,575 }, { 332,74 }, { 89,29 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 303,575 }, { 332,105 }, { 90,29 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 211,614 }, { 175,418 }, { 172,93 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_ktreasure = sizeof(segment_GW_lcd_ktreasure)/sizeof(segment_GW_lcd_ktreasure[0]); 
 const uint16_t segment_info_lcd_ktreasure[] = { 512, 512, 1, 0, 569, 715}; 

const std::string path_background_lcd_ktreasure = "gfx/background_lcd_ktreasure.png";
const uint16_t background_info_lcd_ktreasure[] = { 1024, 1024, 1, 308, 569, 715, 1, 0, 0 }; 


const std::string path_console_lcd_ktreasure = "gfx/console_lcd_ktreasure.png";
const uint16_t console_info_lcd_ktreasure[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_ktreasure (
    "Knuckles Treasure Hunt", "KTREASURE", "2004-12-03"
    , rom_GW_lcd_ktreasure, size_rom_GW_lcd_ktreasure
    , melody_GW_lcd_ktreasure, size_melody_GW_lcd_ktreasure
    , path_segment_lcd_ktreasure
    , segment_GW_lcd_ktreasure, size_segment_GW_lcd_ktreasure
    , segment_info_lcd_ktreasure
    , path_background_lcd_ktreasure
    , background_info_lcd_ktreasure
    , path_console_lcd_ktreasure
    , console_info_lcd_ktreasure
    , GW_rom::MANUFACTURER_MCDONALDS
);

