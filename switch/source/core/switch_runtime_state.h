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

extern std::unique_ptr<SM5XX> g_cpu;
extern const GW_rom* g_game;
extern std::unique_ptr<Virtual_Input> g_input;
extern uint8_t g_game_index;

extern std::vector<Segment> g_segments;
// Debounced/displayed lit-state per segment (what the renderer reads), and the
// raw previous-frame CPU state used to compute it. Single-threaded equivalent
// of Android's front/back snapshot pair (see yokoi_segments_state.cpp).
extern std::vector<uint8_t> g_seg_on;
extern std::vector<uint8_t> g_seg_raw_prev;

extern uint16_t g_segment_info[8];
extern bool g_double_in_one_screen;
extern uint8_t g_nb_screen;
extern bool g_split_two_screens_to_panels;

extern float g_canvas_w;
extern float g_canvas_h;
extern float g_screen_off_x[2];
extern float g_screen_off_y[2];

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

// GAME A / GAME B short-press pulses, fed into yokoi_input_apply_controller_mask.
extern std::atomic<int> g_gamea_pulse_frames;
extern std::atomic<int> g_gameb_pulse_frames;

// Bumped whenever a new game is loaded, so switch_screen knows to reload textures.
extern uint32_t g_texture_generation;
