
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sskateboard.h"


const uint8_t rom_GW_lcd_sskateboard[] = {
	0xFF, 0xFF, 0xFE, 0x0E
}; const size_t size_rom_GW_lcd_sskateboard = sizeof(rom_GW_lcd_sskateboard)/sizeof(rom_GW_lcd_sskateboard[0]);

const uint8_t melody_GW_lcd_sskateboard[1] = {0}; 
	const size_t size_melody_GW_lcd_sskateboard = 0;

const std::string path_segment_lcd_sskateboard = "gfx/segment_lcd_sskateboard.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sskateboard[] = {
	{ { 0,0,0 }, { 16,11 }, { 31,520 }, { 30,25 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 50,11 }, { 63,500 }, { 29,25 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 83,11 }, { 31,493 }, { 29,25 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 213,11 }, { 99,995 }, { 28,28 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 244,11 }, { 99,965 }, { 28,28 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 275,11 }, { 1,517 }, { 28,28 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 142,43 }, { 82,800 }, { 37,38 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 102,97 }, { 99,946 }, { 18,17 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 134,86 }, { 69,886 }, { 52,52 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 200,95 }, { 100,521 }, { 17,18 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 96,89 }, { 67,561 }, { 31,32 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 148,99 }, { 100,568 }, { 26,25 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 194,88 }, { 67,527 }, { 31,32 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 81,157 }, { 102,601 }, { 25,26 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 121,143 }, { 1,758 }, { 78,78 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 214,157 }, { 100,541 }, { 25,25 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 75,152 }, { 81,762 }, { 37,36 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 139,160 }, { 82,840 }, { 44,44 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 208,151 }, { 82,724 }, { 37,36 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 58,216 }, { 67,595 }, { 33,32 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 228,214 }, { 82,689 }, { 33,33 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 42,201 }, { 1,547 }, { 64,64 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 134,226 }, { 67,629 }, { 50,49 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 212,199 }, { 1,613 }, { 64,65 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 11,271 }, { 1,940 }, { 96,83 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 128,294 }, { 1,838 }, { 66,100 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 220,274 }, { 1,680 }, { 79,76 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sskateboard = sizeof(segment_GW_lcd_sskateboard)/sizeof(segment_GW_lcd_sskateboard[0]); 
 const uint16_t segment_info_lcd_sskateboard[] = { 128, 1024, 1, 0, 314, 395}; 

const std::string path_background_lcd_sskateboard = "gfx/background_lcd_sskateboard.png";
const uint16_t background_info_lcd_sskateboard[] = { 512, 512, 1, 116, 314, 395, 1, 0, 0 }; 


const std::string path_console_lcd_sskateboard = "gfx/console_lcd_sskateboard.png";
const uint16_t console_info_lcd_sskateboard[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_sskateboard (
    "Sonic Skateboard", "SSKATEBOARD", "2005-03-18"
    , rom_GW_lcd_sskateboard, size_rom_GW_lcd_sskateboard
    , melody_GW_lcd_sskateboard, size_melody_GW_lcd_sskateboard
    , path_segment_lcd_sskateboard
    , segment_GW_lcd_sskateboard, size_segment_GW_lcd_sskateboard
    , segment_info_lcd_sskateboard
    , path_background_lcd_sskateboard
    , background_info_lcd_sskateboard
    , path_console_lcd_sskateboard
    , console_info_lcd_sskateboard
    , GW_rom::MANUFACTURER_MCDONALDS
);

