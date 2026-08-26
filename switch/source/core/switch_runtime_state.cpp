#include "switch_runtime_state.h"

#include "SM5XX/SM5XX.h"
#include "virtual_i_o/virtual_input.h"

std::unique_ptr<SM5XX> g_cpu;
const GW_rom* g_game = nullptr;
std::unique_ptr<Virtual_Input> g_input;
uint8_t g_game_index = 0;

std::vector<Segment> g_segments;
std::vector<uint8_t> g_seg_on;
std::vector<uint8_t> g_seg_raw_prev;

uint16_t g_segment_info[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool g_double_in_one_screen = false;
uint8_t g_nb_screen = 1;
bool g_split_two_screens_to_panels = false;

float g_canvas_w = 1.0f;
float g_canvas_h = 1.0f;
float g_screen_off_x[2] = {0.0f, 0.0f};
float g_screen_off_y[2] = {0.0f, 0.0f};

float g_top_canvas_w = 1.0f;
float g_top_canvas_h = 1.0f;
float g_bottom_canvas_w = 0.0f;
float g_bottom_canvas_h = 0.0f;
float g_combined_canvas_w = 1.0f;
float g_combined_canvas_h = 1.0f;
float g_top_off_x = 0.0f;
float g_bottom_off_x = 0.0f;
float g_bottom_off_y = 0.0f;

uint32_t g_rate_accu = 0;
int g_time_set_grace_counter = 0;

std::atomic<int> g_gamea_pulse_frames{0};
std::atomic<int> g_gameb_pulse_frames{0};

uint32_t g_texture_generation = 0;
