
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sskateboard.h"
#include "lcd_sskateboard_espn.h"



const uint8_t rom_GW_lcd_sskateboard_espn[] = {
	0xFF, 0xFF, 0xFE, 0x0E
}; const size_t size_rom_GW_lcd_sskateboard_espn = sizeof(rom_GW_lcd_sskateboard_espn)/sizeof(rom_GW_lcd_sskateboard_espn[0]);

const uint8_t melody_GW_lcd_sskateboard_espn[1] = {0}; 
	const size_t size_melody_GW_lcd_sskateboard_espn = 0;

const std::string path_segment_lcd_sskateboard_espn = "gfx/segment_lcd_sskateboard_espn.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sskateboard_espn[] = {
	{ { 0,0,0 }, { 8,11 }, { 75,317 }, { 32,41 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 45,11 }, { 75,274 }, { 32,41 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 82,11 }, { 75,231 }, { 32,41 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 218,13 }, { 54,121 }, { 23,37 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 248,13 }, { 79,109 }, { 23,37 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 279,13 }, { 105,192 }, { 22,37 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 145,58 }, { 104,111 }, { 21,35 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 119,52 }, { 109,328 }, { 18,30 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 142,115 }, { 1,124 }, { 25,41 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 177,52 }, { 104,79 }, { 19,30 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 112,80 }, { 77,85 }, { 20,22 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 139,100 }, { 26,82 }, { 33,13 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 185,82 }, { 54,97 }, { 21,22 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 94,88 }, { 1,83 }, { 23,39 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 135,181 }, { 75,451 }, { 37,60 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 200,88 }, { 28,119 }, { 24,39 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 80,125 }, { 30,185 }, { 34,29 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 127,159 }, { 30,160 }, { 53,21 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 205,125 }, { 85,148 }, { 31,33 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 65,138 }, { 1,167 }, { 27,47 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 224,138 }, { 75,183 }, { 28,46 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 45,179 }, { 75,408 }, { 46,41 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 117,249 }, { 1,216 }, { 72,28 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 233,177 }, { 75,360 }, { 40,46 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 6,186 }, { 1,321 }, { 65,87 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 111,284 }, { 1,410 }, { 72,101 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 243,199 }, { 1,246 }, { 61,73 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sskateboard_espn = sizeof(segment_GW_lcd_sskateboard_espn)/sizeof(segment_GW_lcd_sskateboard_espn[0]); 
 const uint16_t segment_info_lcd_sskateboard_espn[] = { 128, 512, 1, 0, 314, 395}; 

const std::string path_background_lcd_sskateboard_espn = "gfx/background_lcd_sskateboard_espn.png";
const uint16_t background_info_lcd_sskateboard_espn[] = { 512, 512, 1, 116, 314, 395, 1, 0, 0 }; 


const std::string path_console_lcd_sskateboard_espn = "gfx/console_lcd_sskateboard_espn.png";
const uint16_t console_info_lcd_sskateboard_espn[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_sskateboard_espn (
    "ESPN25: Tony Hawk - Skateboarding", "SSKATEBOARD_ESPN", "2004-04-30"
    , rom_GW_lcd_sskateboard_espn, size_rom_GW_lcd_sskateboard_espn
    , melody_GW_lcd_sskateboard_espn, size_melody_GW_lcd_sskateboard_espn
    , path_segment_lcd_sskateboard_espn
    , segment_GW_lcd_sskateboard_espn, size_segment_GW_lcd_sskateboard_espn
    , segment_info_lcd_sskateboard_espn
    , path_background_lcd_sskateboard_espn
    , background_info_lcd_sskateboard_espn
    , path_console_lcd_sskateboard_espn
    , console_info_lcd_sskateboard_espn
    , GW_rom::MANUFACTURER_MCDONALDS
);

