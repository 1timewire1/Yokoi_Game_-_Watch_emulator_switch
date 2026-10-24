
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sxtremeboard.h"
#include "lcd_sxtremeboard_intl.h"

extern const std::string path_segment_lcd_sxtremeboard;
extern const Segment segment_GW_lcd_sxtremeboard[];
extern const size_t size_segment_GW_lcd_sxtremeboard;
extern const uint16_t segment_info_lcd_sxtremeboard[];
extern const std::string path_background_lcd_sxtremeboard;
extern const uint16_t background_info_lcd_sxtremeboard[];


const uint8_t rom_GW_lcd_sxtremeboard_intl[] = {
	0xFF, 0xFF, 0xFE, 0x07
}; const size_t size_rom_GW_lcd_sxtremeboard_intl = sizeof(rom_GW_lcd_sxtremeboard_intl)/sizeof(rom_GW_lcd_sxtremeboard_intl[0]);

const uint8_t melody_GW_lcd_sxtremeboard_intl[1] = {0}; 
	const size_t size_melody_GW_lcd_sxtremeboard_intl = 0;

const std::string path_console_lcd_sxtremeboard_intl = "gfx/console_lcd_sxtremeboard_intl.png";
const uint16_t console_info_lcd_sxtremeboard_intl[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_sxtremeboard_intl (
    "Sonic Extreme Boarding (International)", "SXTREMEBOARD_INTL", "2004-01-01"
    , rom_GW_lcd_sxtremeboard_intl, size_rom_GW_lcd_sxtremeboard_intl
    , melody_GW_lcd_sxtremeboard_intl, size_melody_GW_lcd_sxtremeboard_intl
    , path_segment_lcd_sxtremeboard
    , segment_GW_lcd_sxtremeboard, size_segment_GW_lcd_sxtremeboard
    , segment_info_lcd_sxtremeboard
    , path_background_lcd_sxtremeboard
    , background_info_lcd_sxtremeboard
    , path_console_lcd_sxtremeboard_intl
    , console_info_lcd_sxtremeboard_intl
    , GW_rom::MANUFACTURER_MCDONALDS
);

