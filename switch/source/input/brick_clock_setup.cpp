#include "brick_clock_setup.h"

#include <ctime>
#include <string>

#include "BrickCores/BrickCore.h"

namespace {

// Frame timings (60 fps).
constexpr int kSettleFrames = 60;      // boot + a save state's load
constexpr int kPressFrames = 12;       // button held
constexpr int kStartWaitFrames = 30;   // release -> the watch has left its test screen
constexpr int kConfirmWaitFrames = 60; // release -> next look at the "unset" flag
constexpr int kMaxConfirmPresses = 10; // intro screens can swallow a press or two

enum class Step { Settle, Press, StartWait, ConfirmPress, ConfirmWait, Done };

Step g_step = Step::Done;
int g_frames = 0;
int g_presses = 0;
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

// No write before the press: Pocket Pikachu's intro/set screen goes wrong if
// its time cells change under it. It confirms its own default time; the
// real one goes in once the game reports the clock set.
void press_confirm(BrickCore* core) {
    core->set_button(g_button, true);
    g_presses++;
    go(Step::ConfirmPress, kPressFrames);
}

}  // namespace

void brick_clock_setup_begin(bool enabled) {
    g_step = Step::Done;
    g_button.clear();
    g_presses = 0;
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
            if (!core->clock_start_button().empty() && core->display_test_pattern()) {
                g_button = core->clock_start_button();
                core->set_button(g_button, true);
                go(Step::Press, kPressFrames);
            } else if (!core->clock_confirm_button().empty() && core->clock_unset()) {
                g_button = core->clock_confirm_button();
                press_confirm(core);
            } else {
                write_now(core);
                g_step = Step::Done;
            }
            break;
        case Step::Press:
            core->set_button(g_button, false);
            go(Step::StartWait, kStartWaitFrames);
            break;
        case Step::ConfirmPress:
            core->set_button(g_button, false);
            go(Step::ConfirmWait, kConfirmWaitFrames);
            break;
        case Step::ConfirmWait:
            // Keep pressing through the intro until the game reports its
            // clock set; then write the time again (seconds included - the
            // confirm restarted them from 0).
            if (core->clock_unset() && g_presses < kMaxConfirmPresses) {
                press_confirm(core);
            } else {
                write_now(core);
                g_step = Step::Done;
            }
            break;
        case Step::StartWait:
        case Step::Done:
            write_now(core);
            g_step = Step::Done;
            break;
    }
}
