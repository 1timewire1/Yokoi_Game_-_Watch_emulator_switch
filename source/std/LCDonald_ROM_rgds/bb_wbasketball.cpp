
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "bb_wbasketball.h"


const uint8_t rom_GW_bb_wbasketball[] = {
	0xFF, 0xFF, 0xFE, 0x1E
}; const size_t size_rom_GW_bb_wbasketball = sizeof(rom_GW_bb_wbasketball)/sizeof(rom_GW_bb_wbasketball[0]);

const uint8_t melody_GW_bb_wbasketball[1] = {0}; 
	const size_t size_melody_GW_bb_wbasketball = 0;

const std::string path_segment_bb_wbasketball = "gfx/segment_bb_wbasketball.png"; // Visual of segment -> Big unique texture
const Segment segment_GW_bb_wbasketball[] = {
	{ { 0,0,0 }, { 499,63 }, { 173,208 }, { 24,6 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 495,65 }, { 136,209 }, { 8,24 }, 0, 0, false, false, 0 }, { { 0,0,2 }, { 518,65 }, { 146,209 }, { 7,24 }, 0, 0, false, false, 0 }, { { 0,0,3 }, { 482,66 }, { 151,4 }, { 10,48 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 498,87 }, { 225,209 }, { 22,5 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 493,89 }, { 155,209 }, { 7,24 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 516,90 }, { 164,209 }, { 7,24 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 257,112 }, { 1,3 }, { 48,49 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 495,110 }, { 199,208 }, { 24,6 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 149,216 }, { 102,4 }, { 47,48 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 256,206 }, { 136,235 }, { 49,48 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 367,215 }, { 51,4 }, { 49,48 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 7,265 }, { 191,54 }, { 187,93 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 188,236 }, { 1,285 }, { 187,93 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 368,265 }, { 1,54 }, { 188,93 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 32,276 }, { 363,408 }, { 143,202 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 213,247 }, { 363,204 }, { 143,202 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 393,276 }, { 363,612 }, { 144,202 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 20,355 }, { 1,502 }, { 165,80 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 201,326 }, { 190,298 }, { 166,80 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 381,355 }, { 190,216 }, { 166,80 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 148,445 }, { 416,138 }, { 64,64 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 249,442 }, { 69,218 }, { 65,65 }, 0, 0, false, false, 0 }, { { 0,5,3 }, { 357,442 }, { 1,217 }, { 66,66 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 15,484 }, { 187,598 }, { 174,216 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 192,471 }, { 1,584 }, { 184,230 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 374,484 }, { 187,380 }, { 174,216 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 42,13 }, { 1,816 }, { 476,207 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 43,63 }, { 1,149 }, { 413,53 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_bb_wbasketball = sizeof(segment_GW_bb_wbasketball)/sizeof(segment_GW_bb_wbasketball[0]); 
 const uint16_t segment_info_bb_wbasketball[] = { 512, 1024, 1, 0, 569, 719}; 

const std::string path_background_bb_wbasketball = "gfx/background_bb_wbasketball.png";
const uint16_t background_info_bb_wbasketball[] = { 1024, 1024, 1, 304, 569, 719, 1, 0, 0 }; 


const std::string path_console_bb_wbasketball = "gfx/console_bb_wbasketball.png";
const uint16_t console_info_bb_wbasketball[] = { 1024, 512, 0, 32, 640, 480}; 




const GW_rom bb_wbasketball (
    "Gloomy Wolf Basketball", "BB_WBASKETBALL", "2025-10-20"
    , rom_GW_bb_wbasketball, size_rom_GW_bb_wbasketball
    , melody_GW_bb_wbasketball, size_melody_GW_bb_wbasketball
    , path_segment_bb_wbasketball
    , segment_GW_bb_wbasketball, size_segment_GW_bb_wbasketball
    , segment_info_bb_wbasketball
    , path_background_bb_wbasketball
    , background_info_bb_wbasketball
    , path_console_bb_wbasketball
    , console_info_bb_wbasketball
    , GW_rom::MANUFACTURER_BURGER_BARD
);

