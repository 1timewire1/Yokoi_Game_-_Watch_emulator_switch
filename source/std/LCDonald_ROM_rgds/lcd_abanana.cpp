
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_abanana.h"


const uint8_t rom_GW_lcd_abanana[] = {
	0xFF, 0xFF, 0xFE, 0x06
}; const size_t size_rom_GW_lcd_abanana = sizeof(rom_GW_lcd_abanana)/sizeof(rom_GW_lcd_abanana[0]);

const uint8_t melody_GW_lcd_abanana[1] = {0}; 
	const size_t size_melody_GW_lcd_abanana = 0;

const std::string path_segment_lcd_abanana = "gfx/segment_lcd_abanana.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_abanana[] = {
	{ { 0,0,0 }, { 266,16 }, { 1,928 }, { 95,58 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 368,16 }, { 1,988 }, { 96,58 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 470,16 }, { 99,979 }, { 96,58 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 72,119 }, { 98,921 }, { 90,56 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 254,119 }, { 1,870 }, { 90,56 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 444,119 }, { 93,863 }, { 89,56 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 74,193 }, { 136,1165 }, { 98,61 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 253,193 }, { 135,1102 }, { 98,61 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 442,193 }, { 135,1039 }, { 97,61 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 72,273 }, { 137,1360 }, { 104,64 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 250,273 }, { 137,1294 }, { 104,64 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 439,273 }, { 137,1228 }, { 104,64 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 56,360 }, { 1,1174 }, { 133,82 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 235,360 }, { 1,1342 }, { 134,82 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 423,360 }, { 1,1258 }, { 134,82 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 135,432 }, { 1,1111 }, { 132,61 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 338,432 }, { 1,1048 }, { 132,61 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 14,493 }, { 1,1840 }, { 186,207 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 218,484 }, { 1,1426 }, { 179,203 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 412,493 }, { 1,1631 }, { 186,207 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_abanana = sizeof(segment_GW_lcd_abanana)/sizeof(segment_GW_lcd_abanana[0]); 
 const uint16_t segment_info_lcd_abanana[] = { 256, 2048, 1, 0, 612, 714}; 

const std::string path_background_lcd_abanana = "gfx/background_lcd_abanana.png";
const uint16_t background_info_lcd_abanana[] = { 1024, 1024, 1, 309, 612, 714, 1, 0, 0 }; 


const std::string path_console_lcd_abanana = "gfx/console_lcd_abanana.png";
const uint16_t console_info_lcd_abanana[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_abanana (
    "AiAi Banana Catch", "ABANANA", "2003-06-27"
    , rom_GW_lcd_abanana, size_rom_GW_lcd_abanana
    , melody_GW_lcd_abanana, size_melody_GW_lcd_abanana
    , path_segment_lcd_abanana
    , segment_GW_lcd_abanana, size_segment_GW_lcd_abanana
    , segment_info_lcd_abanana
    , path_background_lcd_abanana
    , background_info_lcd_abanana
    , path_console_lcd_abanana
    , console_info_lcd_abanana
    , GW_rom::MANUFACTURER_MCDONALDS
);

