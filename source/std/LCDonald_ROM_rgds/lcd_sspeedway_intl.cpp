
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sspeedway.h"
#include "lcd_sspeedway_intl.h"

extern const std::string path_segment_lcd_sspeedway;
extern const Segment segment_GW_lcd_sspeedway[];
extern const size_t size_segment_GW_lcd_sspeedway;
extern const uint16_t segment_info_lcd_sspeedway[];
extern const std::string path_background_lcd_sspeedway;
extern const uint16_t background_info_lcd_sspeedway[];


const uint8_t rom_GW_lcd_sspeedway_intl[] = {
	0xFF, 0xFF, 0xFE, 0x05
}; const size_t size_rom_GW_lcd_sspeedway_intl = sizeof(rom_GW_lcd_sspeedway_intl)/sizeof(rom_GW_lcd_sspeedway_intl[0]);

const uint8_t melody_GW_lcd_sspeedway_intl[1] = {0}; 
	const size_t size_melody_GW_lcd_sspeedway_intl = 0;

const std::string path_console_lcd_sspeedway_intl = "gfx/console_lcd_sspeedway_intl.png";
const uint16_t console_info_lcd_sspeedway_intl[] = { 1024, 512, 0, 33, 638, 479}; 




const GW_rom lcd_sspeedway_intl (
    "Sonic's Speedway (International)", "SSPEEDWAY_INTL", "2003-01-01"
    , rom_GW_lcd_sspeedway_intl, size_rom_GW_lcd_sspeedway_intl
    , melody_GW_lcd_sspeedway_intl, size_melody_GW_lcd_sspeedway_intl
    , path_segment_lcd_sspeedway
    , segment_GW_lcd_sspeedway, size_segment_GW_lcd_sspeedway
    , segment_info_lcd_sspeedway
    , path_background_lcd_sspeedway
    , background_info_lcd_sspeedway
    , path_console_lcd_sspeedway_intl
    , console_info_lcd_sspeedway_intl
    , GW_rom::MANUFACTURER_MCDONALDS
);

