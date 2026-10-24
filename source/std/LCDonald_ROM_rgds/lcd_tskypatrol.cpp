
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_tskypatrol.h"


const uint8_t rom_GW_lcd_tskypatrol[] = {
	0xFF, 0xFF, 0xFE, 0x02
}; const size_t size_rom_GW_lcd_tskypatrol = sizeof(rom_GW_lcd_tskypatrol)/sizeof(rom_GW_lcd_tskypatrol[0]);

const uint8_t melody_GW_lcd_tskypatrol[1] = {0}; 
	const size_t size_melody_GW_lcd_tskypatrol = 0;

const std::string path_segment_lcd_tskypatrol = "gfx/segment_lcd_tskypatrol.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_tskypatrol[] = {
	{ { 0,0,0 }, { 0,0 }, { 182,643 }, { 61,64 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 74,0 }, { 182,775 }, { 62,64 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 149,0 }, { 182,709 }, { 62,64 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 15,83 }, { 1,492 }, { 81,81 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 122,83 }, { 84,492 }, { 81,81 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 229,83 }, { 167,492 }, { 81,81 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 336,83 }, { 1,409 }, { 81,81 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 463,34 }, { 1,874 }, { 179,149 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 406,161 }, { 182,933 }, { 65,90 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 19,241 }, { 167,247 }, { 81,79 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 122,241 }, { 1,245 }, { 81,79 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 232,241 }, { 167,328 }, { 82,79 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 340,241 }, { 84,245 }, { 81,79 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 463,194 }, { 1,724 }, { 179,148 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 408,316 }, { 182,841 }, { 65,90 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 19,385 }, { 84,409 }, { 81,81 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 124,385 }, { 167,409 }, { 81,81 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 233,385 }, { 1,326 }, { 81,81 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 340,385 }, { 84,326 }, { 81,81 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 463,353 }, { 1,575 }, { 179,147 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_tskypatrol = sizeof(segment_GW_lcd_tskypatrol)/sizeof(segment_GW_lcd_tskypatrol[0]); 
 const uint16_t segment_info_lcd_tskypatrol[] = { 256, 1024, 1, 0, 642, 500}; 

const std::string path_background_lcd_tskypatrol = "gfx/background_lcd_tskypatrol.png";
const uint16_t background_info_lcd_tskypatrol[] = { 1024, 512, 1, 11, 642, 500, 1, 0, 0 }; 


const std::string path_console_lcd_tskypatrol = "gfx/console_lcd_tskypatrol.png";
const uint16_t console_info_lcd_tskypatrol[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_tskypatrol (
    "Tails Sky Patrol", "TSKYPATROL", "2003-06-27"
    , rom_GW_lcd_tskypatrol, size_rom_GW_lcd_tskypatrol
    , melody_GW_lcd_tskypatrol, size_melody_GW_lcd_tskypatrol
    , path_segment_lcd_tskypatrol
    , segment_GW_lcd_tskypatrol, size_segment_GW_lcd_tskypatrol
    , segment_info_lcd_tskypatrol
    , path_background_lcd_tskypatrol
    , background_info_lcd_tskypatrol
    , path_console_lcd_tskypatrol
    , console_info_lcd_tskypatrol
    , GW_rom::MANUFACTURER_MCDONALDS
);

