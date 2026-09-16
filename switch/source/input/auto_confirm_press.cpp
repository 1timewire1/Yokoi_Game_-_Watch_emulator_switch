#include "auto_confirm_press.h"

#include "SM5XX/SM5XX.h"

namespace {

// Same single-press timing used throughout this session's other
// clock-setup work (Mode/Time/Alarm confirms) - proven reliable.
constexpr int kHoldFrames = 10;

bool g_active = false;
int g_frames_left = 0;
int8_t g_group = -1;
int8_t g_line = -1;
bool g_not_multiplex = false;

} // namespace

void auto_confirm_press_begin(int8_t group, int8_t line, bool not_multiplex) {
    g_group = group;
    g_line = line;
    g_not_multiplex = not_multiplex;
    g_active = true;
    g_frames_left = kHoldFrames;
}

bool auto_confirm_press_active() {
    return g_active;
}

void auto_confirm_press_advance(SM5XX* cpu) {
    if (!cpu || !g_active) return;

    cpu->input_set(g_group, g_line, true, g_not_multiplex);
    g_frames_left--;
    if (g_frames_left <= 0) {
        cpu->input_set(g_group, g_line, false, g_not_multiplex);
        g_active = false;
    }
}
