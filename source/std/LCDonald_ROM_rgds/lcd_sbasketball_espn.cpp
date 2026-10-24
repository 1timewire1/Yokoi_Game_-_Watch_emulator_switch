
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "lcd_sbasketball.h"
#include "lcd_sbasketball_espn.h"



const uint8_t rom_GW_lcd_sbasketball_espn[] = {
	0xFF, 0xFF, 0xFE, 0x11
}; const size_t size_rom_GW_lcd_sbasketball_espn = sizeof(rom_GW_lcd_sbasketball_espn)/sizeof(rom_GW_lcd_sbasketball_espn[0]);

const uint8_t melody_GW_lcd_sbasketball_espn[1] = {0}; 
	const size_t size_melody_GW_lcd_sbasketball_espn = 0;

const std::string path_segment_lcd_sbasketball_espn = "gfx/segment_lcd_sbasketball_espn.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_lcd_sbasketball_espn[] = {
	{ { 0,0,0 }, { 274,28 }, { 78,959 }, { 18,7 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 270,32 }, { 83,1351 }, { 5,20 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 291,34 }, { 75,1353 }, { 6,18 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 253,30 }, { 116,914 }, { 8,52 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 274,53 }, { 98,961 }, { 10,5 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 284,53 }, { 90,1365 }, { 8,6 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 269,57 }, { 116,892 }, { 6,20 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 291,58 }, { 116,871 }, { 6,19 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 256,136 }, { 78,968 }, { 47,47 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 272,76 }, { 101,1365 }, { 20,6 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 145,266 }, { 77,1568 }, { 47,47 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 247,281 }, { 75,1373 }, { 47,46 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 385,272 }, { 77,1519 }, { 47,47 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 106,315 }, { 1,1056 }, { 79,63 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 224,306 }, { 1,1121 }, { 98,72 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 380,325 }, { 1,955 }, { 75,60 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 92,345 }, { 1,1336 }, { 72,139 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 253,341 }, { 1,1904 }, { 84,143 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 398,352 }, { 1,1195 }, { 72,139 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 85,384 }, { 1,915 }, { 113,38 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 225,374 }, { 1,1017 }, { 122,37 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 363,392 }, { 1,875 }, { 113,38 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 101,491 }, { 82,1711 }, { 45,46 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 263,490 }, { 77,1470 }, { 47,47 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 412,502 }, { 75,1421 }, { 47,47 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 59,529 }, { 1,1759 }, { 80,143 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 251,536 }, { 1,1477 }, { 74,138 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 419,533 }, { 1,1617 }, { 79,140 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_lcd_sbasketball_espn = sizeof(segment_GW_lcd_sbasketball_espn)/sizeof(segment_GW_lcd_sbasketball_espn[0]); 
 const uint16_t segment_info_lcd_sbasketball_espn[] = { 128, 2048, 1, 0, 535, 677}; 

const std::string path_background_lcd_sbasketball_espn = "gfx/background_lcd_sbasketball_espn.png";
const uint16_t background_info_lcd_sbasketball_espn[] = { 1024, 1024, 1, 346, 535, 677, 1, 0, 0 }; 


const std::string path_console_lcd_sbasketball_espn = "gfx/console_lcd_sbasketball_espn.png";
const uint16_t console_info_lcd_sbasketball_espn[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom lcd_sbasketball_espn (
    "ESPN25: Vince Carter - Basketball", "SBASKETBALL_ESPN", "2004-04-30"
    , rom_GW_lcd_sbasketball_espn, size_rom_GW_lcd_sbasketball_espn
    , melody_GW_lcd_sbasketball_espn, size_melody_GW_lcd_sbasketball_espn
    , path_segment_lcd_sbasketball_espn
    , segment_GW_lcd_sbasketball_espn, size_segment_GW_lcd_sbasketball_espn
    , segment_info_lcd_sbasketball_espn
    , path_background_lcd_sbasketball_espn
    , background_info_lcd_sbasketball_espn
    , path_console_lcd_sbasketball_espn
    , console_info_lcd_sbasketball_espn
    , GW_rom::MANUFACTURER_MCDONALDS
);

