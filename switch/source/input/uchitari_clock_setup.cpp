#include "uchitari_clock_setup.h"

#include "SM5XX/SM5XX.h"

namespace {

// (group, line) targets - see uchitari_clock_setup.h's doc comment.
// Mode/Alarm share group 1; Hour is group 3 line 2, Minute is group 2 line 0
// (matches MAME's hh_sm510.cpp INPUT_PORTS_START(uchitari): IN.1 bit1 =
// Mode, IN.3 bit2 = Hour (shared with digit "1"), IN.2 bit0 = Minute
// (shared with digit "0"), IN.1 bit2 = Alarm (shared with Exam Level)).
constexpr int kModeGroup = 1, kModeLine = 1;
constexpr int kHourGroup = 3, kHourLine = 2;
constexpr int kMinuteGroup = 2, kMinuteLine = 0;
constexpr int kAlarmGroup = 1, kAlarmLine = 2;

// Frames held/released per press. Hour/Minute use the timing confirmed
// reliable by this session's sweep (anything faster started dropping
// taps); Mode/Alarm use a longer, extra-safe duration since they're each
// pressed only once and there's no cost to being generous there.
constexpr int kTapHoldFrames = 6;
constexpr int kTapReleaseFrames = 10;
constexpr int kSingleHoldFrames = 10;
constexpr int kSingleReleaseFrames = 30;

enum class Phase { kMode, kHour, kMinute, kAlarm, kDone };

Phase g_phase = Phase::kDone;
int g_taps_remaining = 0; // remaining presses in the current phase
bool g_held = false; // currently in the "held" half of the current press
int g_frames_left = 0; // frames left in the current held/released half

int group_for(Phase p) {
    switch (p) {
        case Phase::kMode: return kModeGroup;
        case Phase::kHour: return kHourGroup;
        case Phase::kMinute: return kMinuteGroup;
        case Phase::kAlarm: return kAlarmGroup;
        default: return -1;
    }
}
int line_for(Phase p) {
    switch (p) {
        case Phase::kMode: return kModeLine;
        case Phase::kHour: return kHourLine;
        case Phase::kMinute: return kMinuteLine;
        case Phase::kAlarm: return kAlarmLine;
        default: return -1;
    }
}
int hold_frames_for(Phase p) {
    return (p == Phase::kHour || p == Phase::kMinute) ? kTapHoldFrames : kSingleHoldFrames;
}
int release_frames_for(Phase p) {
    return (p == Phase::kHour || p == Phase::kMinute) ? kTapReleaseFrames : kSingleReleaseFrames;
}

Phase next_phase(Phase p) {
    switch (p) {
        case Phase::kMode: return Phase::kHour;
        case Phase::kHour: return Phase::kMinute;
        case Phase::kMinute: return Phase::kAlarm;
        default: return Phase::kDone;
    }
}

// Taps needed to enter this phase's target value, given the hour/minute
// requested in begin(). Mode/Alarm are always exactly one press.
int taps_for(Phase p, uint8_t hour, uint8_t minute) {
    switch (p) {
        case Phase::kMode: return 1;
        case Phase::kHour: return hour; // 0-23, one tap per hour (wraps like the real device)
        case Phase::kMinute: return minute; // 0-59, one tap per minute
        case Phase::kAlarm: return 1;
        default: return 0;
    }
}

uint8_t g_hour = 0;
uint8_t g_minute = 0;

void enter_phase(Phase p) {
    g_phase = p;
    g_taps_remaining = (p == Phase::kDone) ? 0 : taps_for(p, g_hour, g_minute);
    g_held = false;
    g_frames_left = 0;
    // A phase needing zero taps (e.g. hour == 0) just falls through
    // immediately on the next advance() call below rather than looping
    // here, keeping this function trivial or the caller's loop bounded.
}

} // namespace

void uchitari_clock_setup_begin(uint8_t hour, uint8_t minute) {
    g_hour = hour;
    g_minute = minute;
    enter_phase(Phase::kMode);
}

bool uchitari_clock_setup_active() {
    return g_phase != Phase::kDone;
}

void uchitari_clock_setup_advance(SM5XX* cpu) {
    if (!cpu) return;

    // Skip phases that need zero taps (e.g. minute == 0), and phases whose
    // last tap has finished its full release wait (taps_remaining==0,
    // !held, frames_left<=0) - both mean "nothing left to do here," so
    // move to the next phase. Can chain through several empty phases in
    // one call. Doing the "last tap finished" check here too (rather than
    // transitioning the instant taps_remaining hits 0 inside the release
    // branch below) matters: every tap, including a phase's last one,
    // needs its full release_frames wait honored before the next phase's
    // first press - skipping it for the last tap was an earlier bug here
    // that dropped presses at phase boundaries (confirmed by testing:
    // hour==0 or minute==0 would leave the clock not ticking afterward).
    while (g_phase != Phase::kDone && g_taps_remaining == 0 && !g_held && g_frames_left <= 0) {
        enter_phase(next_phase(g_phase));
    }
    if (g_phase == Phase::kDone) return;

    int group = group_for(g_phase), line = line_for(g_phase);

    if (g_frames_left <= 0) {
        // Transition: flip held<->released, or finish this tap.
        if (!g_held) {
            g_held = true;
            g_frames_left = hold_frames_for(g_phase);
            cpu->input_set(group, line, true);
        } else {
            g_held = false;
            cpu->input_set(group, line, false);
            g_taps_remaining--;
            g_frames_left = release_frames_for(g_phase);
        }
    }
    g_frames_left--;
}
