
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_artennis.h"
#include "lcd_artennis_espn.h"



const uint8_t rom_GW_lcd_artennis_espn[] = {
	0xFF, 0xFF, 0xFE, 0x12
}; const size_t size_rom_GW_lcd_artennis_espn = sizeof(rom_GW_lcd_artennis_espn)/sizeof(rom_GW_lcd_artennis_espn[0]);

const uint8_t melody_GW_lcd_artennis_espn[1] = {0}; 
	const size_t size_melody_GW_lcd_artennis_espn = 0;

const std::string path_segment_lcd_artennis_espn = "gfx/segment_lcd_artennis_espn.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_artennis_espn[] = {
	{ { 0,0,0 }, { 128,36 }, { 223,291 }, { 28,27 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 158,36 }, { 72,172 }, { 27,27 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 193,36 }, { 201,262 }, { 27,27 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 229,36 }, { 230,233 }, { 24,27 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 263,36 }, { 230,262 }, { 25,27 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 296,36 }, { 201,233 }, { 27,27 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 325,36 }, { 201,204 }, { 27,27 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 148,83 }, { 111,291 }, { 110,98 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 246,92 }, { 1,201 }, { 119,88 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 201,188 }, { 144,166 }, { 18,17 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 274,189 }, { 123,164 }, { 19,19 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 165,250 }, { 101,162 }, { 20,21 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 236,250 }, { 230,209 }, { 22,22 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 304,251 }, { 72,150 }, { 21,20 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 202,282 }, { 122,220 }, { 77,69 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 114,363 }, { 1,149 }, { 21,20 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 232,364 }, { 230,186 }, { 22,21 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 334,366 }, { 24,149 }, { 21,20 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 79,439 }, { 218,164 }, { 22,20 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 232,439 }, { 195,181 }, { 21,21 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 367,444 }, { 47,149 }, { 21,20 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 29,553 }, { 1,171 }, { 69,28 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 44,476 }, { 1,405 }, { 129,106 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 288,469 }, { 223,320 }, { 25,69 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 179,462 }, { 132,391 }, { 111,120 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 379,554 }, { 122,185 }, { 71,33 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 297,479 }, { 1,301 }, { 108,102 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_artennis_espn = sizeof(segment_GW_lcd_artennis_espn)/sizeof(segment_GW_lcd_artennis_espn[0]); 
 const uint16_t segment_info_lcd_artennis_espn[] = { 256, 512, 1, 0, 473, 596}; 

const std::string path_background_lcd_artennis_espn = "gfx/background_lcd_artennis_espn.png";
const uint16_t background_info_lcd_artennis_espn[] = { 512, 1024, 1, 427, 473, 596, 1, 0, 0 }; 


const std::string path_console_lcd_artennis_espn = "gfx/console_lcd_artennis_espn.png";
const uint16_t console_info_lcd_artennis_espn[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_artennis_espn (
    "ESPN25: Venus and Serena Williams - Tennis", "ARTENNIS_ESPN", "2004-04-30"
    , rom_GW_lcd_artennis_espn, size_rom_GW_lcd_artennis_espn
    , melody_GW_lcd_artennis_espn, size_melody_GW_lcd_artennis_espn
    , path_segment_lcd_artennis_espn
    , segment_GW_lcd_artennis_espn, size_segment_GW_lcd_artennis_espn
    , segment_info_lcd_artennis_espn
    , path_background_lcd_artennis_espn
    , background_info_lcd_artennis_espn
    , path_console_lcd_artennis_espn
    , console_info_lcd_artennis_espn
    , GW_rom::MANUFACTURER_MCDONALDS
);

