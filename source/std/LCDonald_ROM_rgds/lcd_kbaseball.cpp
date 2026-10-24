
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_kbaseball.h"


const uint8_t rom_GW_lcd_kbaseball[] = {
	0xFF, 0xFF, 0xFE, 0x09
}; const size_t size_rom_GW_lcd_kbaseball = sizeof(rom_GW_lcd_kbaseball)/sizeof(rom_GW_lcd_kbaseball[0]);

const uint8_t melody_GW_lcd_kbaseball[1] = {0}; 
	const size_t size_melody_GW_lcd_kbaseball = 0;

const std::string path_segment_lcd_kbaseball = "gfx/segment_lcd_kbaseball.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_kbaseball[] = {
	{ { 0,0,0 }, { 591,21 }, { 170,408 }, { 69,69 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 674,21 }, { 98,337 }, { 69,69 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 756,21 }, { 98,408 }, { 70,69 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 728,169 }, { 169,299 }, { 53,52 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 730,287 }, { 202,581 }, { 53,53 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 731,416 }, { 202,526 }, { 53,53 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 590,144 }, { 153,191 }, { 52,52 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 589,292 }, { 98,283 }, { 53,52 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 587,443 }, { 98,175 }, { 52,52 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 445,119 }, { 1,169 }, { 52,52 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 450,287 }, { 152,137 }, { 52,52 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 449,459 }, { 153,245 }, { 53,52 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 301,105 }, { 169,353 }, { 53,53 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 303,286 }, { 98,229 }, { 53,52 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 303,471 }, { 55,121 }, { 51,52 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 107,34 }, { 1,827 }, { 206,196 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 110,227 }, { 1,479 }, { 199,155 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 90,400 }, { 1,636 }, { 212,189 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 20,182 }, { 1,223 }, { 95,254 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_kbaseball = sizeof(segment_GW_lcd_kbaseball)/sizeof(segment_GW_lcd_kbaseball[0]); 
 const uint16_t segment_info_lcd_kbaseball[] = { 256, 1024, 1, 0, 848, 639}; 

const std::string path_background_lcd_kbaseball = "gfx/background_lcd_kbaseball.png";
const uint16_t background_info_lcd_kbaseball[] = { 1024, 1024, 1, 384, 848, 639, 1, 0, 0 }; 


const std::string path_console_lcd_kbaseball = "gfx/console_lcd_kbaseball.png";
const uint16_t console_info_lcd_kbaseball[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_kbaseball (
    "Knuckles Baseball", "KBASEBALL", "2005-03-18"
    , rom_GW_lcd_kbaseball, size_rom_GW_lcd_kbaseball
    , melody_GW_lcd_kbaseball, size_melody_GW_lcd_kbaseball
    , path_segment_lcd_kbaseball
    , segment_GW_lcd_kbaseball, size_segment_GW_lcd_kbaseball
    , segment_info_lcd_kbaseball
    , path_background_lcd_kbaseball
    , background_info_lcd_kbaseball
    , path_console_lcd_kbaseball
    , console_info_lcd_kbaseball
    , GW_rom::MANUFACTURER_MCDONALDS
);

