
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_artennis.h"


const uint8_t rom_GW_lcd_artennis[] = {
	0xFF, 0xFF, 0xFE, 0x12
}; const size_t size_rom_GW_lcd_artennis = sizeof(rom_GW_lcd_artennis)/sizeof(rom_GW_lcd_artennis[0]);

const uint8_t melody_GW_lcd_artennis[1] = {0}; 
	const size_t size_melody_GW_lcd_artennis = 0;

const std::string path_segment_lcd_artennis = "gfx/segment_lcd_artennis.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_artennis[] = {
	{ { 0,0,0 }, { 101,17 }, { 58,294 }, { 49,39 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 151,17 }, { 86,455 }, { 34,39 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 189,18 }, { 99,694 }, { 26,38 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 225,17 }, { 1,256 }, { 31,39 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 265,17 }, { 99,734 }, { 26,39 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 295,17 }, { 86,414 }, { 33,39 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 329,17 }, { 73,496 }, { 50,39 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 126,64 }, { 1,681 }, { 96,92 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 247,65 }, { 1,587 }, { 96,92 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 147,157 }, { 71,258 }, { 34,34 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 292,156 }, { 1,187 }, { 34,33 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 96,224 }, { 37,187 }, { 34,33 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 225,227 }, { 37,153 }, { 34,32 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 343,227 }, { 34,222 }, { 34,34 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 193,256 }, { 1,386 }, { 83,76 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 65,329 }, { 70,222 }, { 34,34 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 208,328 }, { 34,258 }, { 35,34 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 360,328 }, { 73,187 }, { 34,33 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 43,431 }, { 73,153 }, { 34,32 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 221,427 }, { 1,297 }, { 35,35 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 393,430 }, { 1,152 }, { 34,33 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 18,510 }, { 1,334 }, { 55,50 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 51,468 }, { 1,900 }, { 110,123 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 174,483 }, { 73,537 }, { 50,48 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 214,466 }, { 1,464 }, { 70,121 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 395,510 }, { 58,335 }, { 55,49 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 307,468 }, { 1,775 }, { 110,123 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_artennis = sizeof(segment_GW_lcd_artennis)/sizeof(segment_GW_lcd_artennis[0]); 
 const uint16_t segment_info_lcd_artennis[] = { 128, 1024, 1, 0, 473, 596}; 

const std::string path_background_lcd_artennis = "gfx/background_lcd_artennis.png";
const uint16_t background_info_lcd_artennis[] = { 512, 1024, 1, 427, 473, 596, 1, 0, 0 }; 


const std::string path_console_lcd_artennis = "gfx/console_lcd_artennis.png";
const uint16_t console_info_lcd_artennis[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_artennis (
    "Amy & Rouge Tennis", "ARTENNIS", "2005-03-18"
    , rom_GW_lcd_artennis, size_rom_GW_lcd_artennis
    , melody_GW_lcd_artennis, size_melody_GW_lcd_artennis
    , path_segment_lcd_artennis
    , segment_GW_lcd_artennis, size_segment_GW_lcd_artennis
    , segment_info_lcd_artennis
    , path_background_lcd_artennis
    , background_info_lcd_artennis
    , path_console_lcd_artennis
    , console_info_lcd_artennis
    , GW_rom::MANUFACTURER_MCDONALDS
);

