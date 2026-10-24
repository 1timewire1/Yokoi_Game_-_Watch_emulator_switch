
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_shockey.h"
#include "lcd_shockey_rare.h"

extern const std::string path_segment_lcd_shockey;
extern const Segment segment_GW_lcd_shockey[];
extern const size_t size_segment_GW_lcd_shockey;
extern const uint16_t segment_info_lcd_shockey[];
extern const std::string path_background_lcd_shockey;
extern const uint16_t background_info_lcd_shockey[];


const uint8_t rom_GW_lcd_shockey_rare[] = {
	0xFF, 0xFF, 0xFE, 0x0A
}; const size_t size_rom_GW_lcd_shockey_rare = sizeof(rom_GW_lcd_shockey_rare)/sizeof(rom_GW_lcd_shockey_rare[0]);

const uint8_t melody_GW_lcd_shockey_rare[1] = {0}; 
	const size_t size_melody_GW_lcd_shockey_rare = 0;

const std::string path_console_lcd_shockey_rare = "gfx/console_lcd_shockey_rare.png";
const uint16_t console_info_lcd_shockey_rare[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_shockey_rare (
    "Shadow Hockey (Search for the Silver Game)", "SHOCKEY_RARE", "2004-12-03"
    , rom_GW_lcd_shockey_rare, size_rom_GW_lcd_shockey_rare
    , melody_GW_lcd_shockey_rare, size_melody_GW_lcd_shockey_rare
    , path_segment_lcd_shockey
    , segment_GW_lcd_shockey, size_segment_GW_lcd_shockey
    , segment_info_lcd_shockey
    , path_background_lcd_shockey
    , background_info_lcd_shockey
    , path_console_lcd_shockey_rare
    , console_info_lcd_shockey_rare
    , GW_rom::MANUFACTURER_MCDONALDS
);

