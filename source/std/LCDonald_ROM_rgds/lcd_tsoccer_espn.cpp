
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_tsoccer.h"
#include "lcd_tsoccer_espn.h"



const uint8_t rom_GW_lcd_tsoccer_espn[] = {
	0xFF, 0xFF, 0xFE, 0x08
}; const size_t size_rom_GW_lcd_tsoccer_espn = sizeof(rom_GW_lcd_tsoccer_espn)/sizeof(rom_GW_lcd_tsoccer_espn[0]);

const uint8_t melody_GW_lcd_tsoccer_espn[1] = {0}; 
	const size_t size_melody_GW_lcd_tsoccer_espn = 0;

const std::string path_segment_lcd_tsoccer_espn = "gfx/segment_lcd_tsoccer_espn.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_tsoccer_espn[] = {
	{ { 0,0,0 }, { 27,21 }, { 1,524 }, { 75,54 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 111,21 }, { 78,514 }, { 75,54 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 192,21 }, { 152,636 }, { 76,54 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 209,162 }, { 218,597 }, { 37,37 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 287,158 }, { 1,420 }, { 38,37 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 363,160 }, { 1,381 }, { 36,37 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 167,217 }, { 67,461 }, { 52,51 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 275,215 }, { 121,452 }, { 50,51 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 378,216 }, { 67,410 }, { 50,49 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 134,293 }, { 194,849 }, { 59,60 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 275,289 }, { 194,787 }, { 59,60 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 413,293 }, { 193,727 }, { 59,58 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 98,388 }, { 152,570 }, { 64,64 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 274,386 }, { 155,505 }, { 63,63 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 441,384 }, { 1,459 }, { 64,63 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 56,459 }, { 1,692 }, { 190,106 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 238,459 }, { 1,580 }, { 149,110 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 365,458 }, { 1,800 }, { 191,109 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 181,591 }, { 1,911 }, { 236,112 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_tsoccer_espn = sizeof(segment_GW_lcd_tsoccer_espn)/sizeof(segment_GW_lcd_tsoccer_espn[0]); 
 const uint16_t segment_info_lcd_tsoccer_espn[] = { 256, 1024, 1, 0, 577, 703}; 

const std::string path_background_lcd_tsoccer_espn = "gfx/background_lcd_tsoccer_espn.png";
const uint16_t background_info_lcd_tsoccer_espn[] = { 1024, 1024, 1, 320, 577, 703, 1, 0, 0 }; 


const std::string path_console_lcd_tsoccer_espn = "gfx/console_lcd_tsoccer_espn.png";
const uint16_t console_info_lcd_tsoccer_espn[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_tsoccer_espn (
    "ESPN25: Mia Hamm - Soccer", "TSOCCER_ESPN", "2004-04-30"
    , rom_GW_lcd_tsoccer_espn, size_rom_GW_lcd_tsoccer_espn
    , melody_GW_lcd_tsoccer_espn, size_melody_GW_lcd_tsoccer_espn
    , path_segment_lcd_tsoccer_espn
    , segment_GW_lcd_tsoccer_espn, size_segment_GW_lcd_tsoccer_espn
    , segment_info_lcd_tsoccer_espn
    , path_background_lcd_tsoccer_espn
    , background_info_lcd_tsoccer_espn
    , path_console_lcd_tsoccer_espn
    , console_info_lcd_tsoccer_espn
    , GW_rom::MANUFACTURER_MCDONALDS
);

