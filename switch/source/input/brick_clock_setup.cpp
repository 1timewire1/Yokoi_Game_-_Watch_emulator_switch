#include "brick_clock_setup.h"

#include <ctime>
#include <string>

#include "BrickCores/BrickCore.h"

namespace {

// Frame timings (60 fps).
constexpr int kSettleFrames = 60;     // boot + a save state's load
constexpr int kPressFrames = 12;      // button held
constexpr int kStartWaitFrames = 30;  // release -> the watch has left its test screen

enum class Step { Settle, Press, StartWait, Done };

Step g_step = Step::Done;
int g_frames = 0;
std::string g_button;

void go(Step s, int frames) {
    g_step = s;
    g_frames = frames;
}

void write_now(BrickCore* core) {
    const time_t now = time(nullptr);
    const tm* t = localtime(&now);
    if (t) core->set_clock(t->tm_hour, t->tm_min, t->tm_sec);
}

}  // namespace

void brick_clock_setup_begin(bool enabled) {
    g_step = Step::Done;
    g_button.clear();
    if (enabled) go(Step::Settle, kSettleFrames);
}

bool brick_clock_setup_active() {
    return g_step != Step::Done;
}

void brick_clock_setup_advance(BrickCore* core) {
    if (!core || g_step == Step::Done) return;
    if (!core->has_clock()) {
        g_step = Step::Done;
        return;
    }
    if (--g_frames > 0) return;

    switch (g_step) {
        case Step::Settle:
            g_button = core->clock_start_button();
            if (!g_button.empty() && core->display_test_pattern()) {
                core->set_button(g_button, true);
                go(Step::Press, kPressFrames);
            } else {
                write_now(core);
                g_step = Step::Done;
            }
            break;
        case Step::Press:
            core->set_button(g_button, false);
            go(Step::StartWait, kStartWaitFrames);
            break;
        case Step::StartWait:
        case Step::Done:
            write_now(core);
            g_step = Step::Done;
            break;
    }
}
