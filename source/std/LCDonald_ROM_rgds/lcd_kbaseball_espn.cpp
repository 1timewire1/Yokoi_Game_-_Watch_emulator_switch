
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_kbaseball.h"
#include "lcd_kbaseball_espn.h"



const uint8_t rom_GW_lcd_kbaseball_espn[] = {
	0xFF, 0xFF, 0xFE, 0x09
}; const size_t size_rom_GW_lcd_kbaseball_espn = sizeof(rom_GW_lcd_kbaseball_espn)/sizeof(rom_GW_lcd_kbaseball_espn[0]);

const uint8_t melody_GW_lcd_kbaseball_espn[1] = {0}; 
	const size_t size_melody_GW_lcd_kbaseball_espn = 0;

const std::string path_segment_lcd_kbaseball_espn = "gfx/segment_lcd_kbaseball_espn.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_kbaseball_espn[] = {
	{ { 0,0,0 }, { 623,4 }, { 204,901 }, { 50,122 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 692,4 }, { 204,777 }, { 50,122 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 760,4 }, { 175,653 }, { 50,122 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 605,227 }, { 1,459 }, { 59,39 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 607,288 }, { 187,481 }, { 60,38 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 616,373 }, { 126,480 }, { 59,39 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 496,179 }, { 124,521 }, { 58,40 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 491,287 }, { 66,479 }, { 58,40 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 501,401 }, { 187,441 }, { 60,38 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 395,132 }, { 175,608 }, { 62,43 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 389,286 }, { 188,521 }, { 64,41 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 409,440 }, { 124,563 }, { 62,43 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 274,98 }, { 188,564 }, { 63,42 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 299,290 }, { 1,500 }, { 63,41 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 321,501 }, { 1,642 }, { 64,42 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 202,56 }, { 113,638 }, { 60,214 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 184,276 }, { 1,686 }, { 110,166 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 133,442 }, { 1,854 }, { 201,169 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 66,339 }, { 1,543 }, { 121,93 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_kbaseball_espn = sizeof(segment_GW_lcd_kbaseball_espn)/sizeof(segment_GW_lcd_kbaseball_espn[0]); 
 const uint16_t segment_info_lcd_kbaseball_espn[] = { 256, 1024, 1, 0, 848, 639}; 

const std::string path_background_lcd_kbaseball_espn = "gfx/background_lcd_kbaseball_espn.png";
const uint16_t background_info_lcd_kbaseball_espn[] = { 1024, 1024, 1, 384, 848, 639, 1, 0, 0 }; 


const std::string path_console_lcd_kbaseball_espn = "gfx/console_lcd_kbaseball_espn.png";
const uint16_t console_info_lcd_kbaseball_espn[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_kbaseball_espn (
    "ESPN25: Michael Vick - Football", "KBASEBALL_ESPN", "2004-04-30"
    , rom_GW_lcd_kbaseball_espn, size_rom_GW_lcd_kbaseball_espn
    , melody_GW_lcd_kbaseball_espn, size_melody_GW_lcd_kbaseball_espn
    , path_segment_lcd_kbaseball_espn
    , segment_GW_lcd_kbaseball_espn, size_segment_GW_lcd_kbaseball_espn
    , segment_info_lcd_kbaseball_espn
    , path_background_lcd_kbaseball_espn
    , background_info_lcd_kbaseball_espn
    , path_console_lcd_kbaseball_espn
    , console_info_lcd_kbaseball_espn
    , GW_rom::MANUFACTURER_MCDONALDS
);

