#include "e0c6200_clock_setup.h"

#include <ctime>

#include "E0C6200/E0C6S46.h"
#include "core/e0c6200_clock.h"

namespace {

// Frame timings (60 fps). The harness showed the set screen appearing
// ~1 s after the open press; everything here leaves generous margins.
constexpr int kSettleFrames = 180;       // boot animation before the first press
constexpr int kPressFrames = 12;         // button held
constexpr int kOpenWaitFrames = 90;      // open release -> set screen showing
constexpr int kConfirmWaitFrames = 30;   // confirm release -> done

enum class Step { Settle, OpenPress, OpenWait, ConfirmPress, ConfirmWait, Done };

const E0C6200ClockLayout* g_layout = nullptr;
Step g_step = Step::Done;
int g_frames = 0;

void go(Step s, int frames) {
    g_step = s;
    g_frames = frames;
}

}  // namespace

void e0c6200_clock_setup_begin(const std::string& ref, bool enabled) {
    g_step = Step::Done;
    g_layout = enabled ? e0c6200_clock_layout(ref) : nullptr;
    if (g_layout) go(Step::Settle, kSettleFrames);
}

bool e0c6200_clock_setup_active() {
    return g_step != Step::Done;
}

void e0c6200_clock_setup_advance(E0C6S46* cpu) {
    if (!cpu || !g_layout || g_step == Step::Done) return;
    if (--g_frames > 0) return;

    const E0C6200ClockLayout& l = *g_layout;
    switch (g_step) {
        case Step::Settle:
            if (e0c6200_clock_is_set(cpu, l)) {  // restored from a save state - leave it alone
                g_step = Step::Done;
                return;
            }
            cpu->set_k_line(l.open_line, false);  // buttons are active low
            go(Step::OpenPress, kPressFrames);
            break;
        case Step::OpenPress:
            cpu->set_k_line(l.open_line, true);
            go(Step::OpenWait, kOpenWaitFrames);
            break;
        case Step::OpenWait:
            e0c6200_clock_write(cpu, l, time(nullptr));
            cpu->set_k_line(l.confirm_line, false);
            go(Step::ConfirmPress, kPressFrames);
            break;
        case Step::ConfirmPress:
            cpu->set_k_line(l.confirm_line, true);
            go(Step::ConfirmWait, kConfirmWaitFrames);
            break;
        case Step::ConfirmWait:
        case Step::Done:
            g_step = Step::Done;
            break;
    }
}
