#pragma once

// Switch runtime globals. Trimmed version of
// android/app/src/main/cpp/core/yokoi_runtime_state.h: no menu state machine,
// no touch input, no separate emulation thread/GL-context-per-thread bookkeeping,
// since the Switch build runs a single main loop (poll input -> step CPU ->
// update segments -> render -> swap), same shape as source/main.cpp on 3DS.

#include <atomic>
#include <cstdint>
#include <memory>
#include <vector>

#include "std/GW_ROM.h"
#include "std/segment.h"

class SM5XX;
class Virtual_Input;
class RZoneIO;

extern std::unique_ptr<SM5XX> g_cpu;
// Tiger R-Zone cartridge I/O (source/virtual_i_o/rzone_io.h), attached to
// g_cpu as its port_device for the three R-Zone games, null otherwise. Set up
// by switch_load_game_by_index_and_init(); owned here, not by the CPU.
extern std::unique_ptr<RZoneIO> g_rzone_io;
extern const GW_rom* g_game;
extern std::unique_ptr<Virtual_Input> g_input;
extern uint16_t g_game_index;

extern std::vector<Segment> g_segments;
// Debounced/displayed lit-state per segment (what the renderer reads), and the
// raw previous-frame CPU state used to compute it. Single-threaded equivalent
// of Android's front/back snapshot pair (see yokoi_segments_state.cpp).
extern std::vector<uint8_t> g_seg_on;
extern std::vector<uint8_t> g_seg_raw_prev;

// Sized for up to 3 screens (the widest known device, Tronica's tri-screen
// "Super Tri-Screen" units - see rebuild_layout_from_game()): 4 header ints
// + 2 per screen.
extern uint16_t g_segment_info[10];
extern bool g_double_in_one_screen;
extern uint8_t g_nb_screen;
extern bool g_split_two_screens_to_panels;

extern float g_canvas_w;
extern float g_canvas_h;
extern float g_screen_off_x[3];
extern float g_screen_off_y[3];

extern float g_top_canvas_w;
extern float g_top_canvas_h;
extern float g_bottom_canvas_w;
extern float g_bottom_canvas_h;
extern float g_combined_canvas_w;
extern float g_combined_canvas_h;
extern float g_top_off_x;
extern float g_bottom_off_x;
extern float g_bottom_off_y;

extern uint32_t g_rate_accu;

constexpr int kTimeSetGracePeriod = 500;
extern int g_time_set_grace_counter;

constexpr float kTargetFps = 60.0f;

// Frames left to keep calling SM5XX::reassert_combined_hour() after a game
// loads - separate from kTimeSetGracePeriod above (which re-runs the full,
// syscall-driven yokoi_cpu_set_time_if_needed() for a much shorter window,
// counted in executed instructions rather than frames). Sized in real time
// since the SM530 Nelsonic games' boot-default hour reset this exists to
// outlast fires 3-5 real seconds after load regardless of instruction
// count; reassert_combined_hour() itself is cheap (no syscalls, just two
// RAM writes) and a no-op for every other game, so a generous 8-second
// margin here costs nothing.
constexpr int kHourReassertGraceFrames = (int)(8.0f * kTargetFps);
extern int g_hour_reassert_grace_frames;

// GAME A / GAME B short-press pulses, fed into yokoi_input_apply_controller_mask.
extern std::atomic<int> g_gamea_pulse_frames;
extern std::atomic<int> g_gameb_pulse_frames;

// Bumped whenever a new game is loaded, so switch_screen knows to reload textures.
extern uint32_t g_texture_generation;
