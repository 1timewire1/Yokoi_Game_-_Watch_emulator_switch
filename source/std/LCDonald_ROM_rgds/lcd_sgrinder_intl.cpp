
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sgrinder.h"
#include "lcd_sgrinder_intl.h"

extern const std::string path_segment_lcd_sgrinder;
extern const Segment segment_GW_lcd_sgrinder[];
extern const size_t size_segment_GW_lcd_sgrinder;
extern const uint16_t segment_info_lcd_sgrinder[];
extern const std::string path_background_lcd_sgrinder;
extern const uint16_t background_info_lcd_sgrinder[];


const uint8_t rom_GW_lcd_sgrinder_intl[] = {
	0xFF, 0xFF, 0xFE, 0x04
}; const size_t size_rom_GW_lcd_sgrinder_intl = sizeof(rom_GW_lcd_sgrinder_intl)/sizeof(rom_GW_lcd_sgrinder_intl[0]);

const uint8_t melody_GW_lcd_sgrinder_intl[1] = {0}; 
	const size_t size_melody_GW_lcd_sgrinder_intl = 0;

const std::string path_console_lcd_sgrinder_intl = "gfx/console_lcd_sgrinder_intl.png";
const uint16_t console_info_lcd_sgrinder_intl[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_sgrinder_intl (
    "Shadow Grinder (International)", "SGRINDER_INTL", "2003-01-01"
    , rom_GW_lcd_sgrinder_intl, size_rom_GW_lcd_sgrinder_intl
    , melody_GW_lcd_sgrinder_intl, size_melody_GW_lcd_sgrinder_intl
    , path_segment_lcd_sgrinder
    , segment_GW_lcd_sgrinder, size_segment_GW_lcd_sgrinder
    , segment_info_lcd_sgrinder
    , path_background_lcd_sgrinder
    , background_info_lcd_sgrinder
    , path_console_lcd_sgrinder_intl
    , console_info_lcd_sgrinder_intl
    , GW_rom::MANUFACTURER_MCDONALDS
);

