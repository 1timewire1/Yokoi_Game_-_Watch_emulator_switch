
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_ksoccer.h"
#include "lcd_ksoccer_intl.h"

extern const std::string path_segment_lcd_ksoccer;
extern const Segment segment_GW_lcd_ksoccer[];
extern const size_t size_segment_GW_lcd_ksoccer;
extern const uint16_t segment_info_lcd_ksoccer[];
extern const std::string path_background_lcd_ksoccer;
extern const uint16_t background_info_lcd_ksoccer[];


const uint8_t rom_GW_lcd_ksoccer_intl[] = {
	0xFF, 0xFF, 0xFE, 0x03
}; const size_t size_rom_GW_lcd_ksoccer_intl = sizeof(rom_GW_lcd_ksoccer_intl)/sizeof(rom_GW_lcd_ksoccer_intl[0]);

const uint8_t melody_GW_lcd_ksoccer_intl[1] = {0}; 
	const size_t size_melody_GW_lcd_ksoccer_intl = 0;

const std::string path_console_lcd_ksoccer_intl = "gfx/console_lcd_ksoccer_intl.png";
const uint16_t console_info_lcd_ksoccer_intl[] = { 1024, 512, 0, 32, 639, 480}; 




const GW_rom lcd_ksoccer_intl (
    "Knuckles Soccer (International)", "KSOCCER_INTL", "2003-01-01"
    , rom_GW_lcd_ksoccer_intl, size_rom_GW_lcd_ksoccer_intl
    , melody_GW_lcd_ksoccer_intl, size_melody_GW_lcd_ksoccer_intl
    , path_segment_lcd_ksoccer
    , segment_GW_lcd_ksoccer, size_segment_GW_lcd_ksoccer
    , segment_info_lcd_ksoccer
    , path_background_lcd_ksoccer
    , background_info_lcd_ksoccer
    , path_console_lcd_ksoccer_intl
    , console_info_lcd_ksoccer_intl
    , GW_rom::MANUFACTURER_MCDONALDS
);

