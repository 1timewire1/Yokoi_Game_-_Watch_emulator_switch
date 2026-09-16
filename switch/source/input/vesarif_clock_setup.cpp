#include "vesarif_clock_setup.h"

#include "SM5XX/SM5XX.h"

namespace {

// (group, line) targets - see vesarif_clock_setup.h's doc comment. Matches
// MAME's hh_sm510.cpp INPUT_PORTS_START(vesarif): IN.0 bit2 = Time, IN.1
// bit0 = Left-Up (hour), IN.1 bit3 = Right-Up (minute) - Left-Down/
// Right-Down are electrically equivalent alternates, not used here since
// one physical button per field is enough to script the sequence.
constexpr int kTimeGroup = 0, kTimeLine = 2;
constexpr int kHourGroup = 1, kHourLine = 0;
constexpr int kMinuteGroup = 1, kMinuteLine = 3;

constexpr int kTapHoldFrames = 6;
constexpr int kTapReleaseFrames = 10;
constexpr int kSingleHoldFrames = 10;
constexpr int kSingleReleaseFrames = 30;

enum class Phase { kTimeEnter, kHour, kMinute, kTimeConfirm, kDone };

Phase g_phase = Phase::kDone;
int g_taps_remaining = 0;
bool g_held = false;
int g_frames_left = 0;
uint8_t g_hour = 0;
uint8_t g_minute = 0;

int group_for(Phase p) {
    switch (p) {
        case Phase::kTimeEnter: return kTimeGroup;
        case Phase::kHour: return kHourGroup;
        case Phase::kMinute: return kMinuteGroup;
        case Phase::kTimeConfirm: return kTimeGroup;
        default: return -1;
    }
}
int line_for(Phase p) {
    switch (p) {
        case Phase::kTimeEnter: return kTimeLine;
        case Phase::kHour: return kHourLine;
        case Phase::kMinute: return kMinuteLine;
        case Phase::kTimeConfirm: return kTimeLine;
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
        case Phase::kTimeEnter: return Phase::kHour;
        case Phase::kHour: return Phase::kMinute;
        case Phase::kMinute: return Phase::kTimeConfirm;
        default: return Phase::kDone;
    }
}

int taps_for(Phase p, uint8_t hour, uint8_t minute) {
    switch (p) {
        case Phase::kTimeEnter: return 1;
        case Phase::kHour: return hour; // 0-23, one tap per hour
        case Phase::kMinute: return minute; // 0-59, one tap per minute
        case Phase::kTimeConfirm: return 1;
        default: return 0;
    }
}

void enter_phase(Phase p) {
    g_phase = p;
    g_taps_remaining = (p == Phase::kDone) ? 0 : taps_for(p, g_hour, g_minute);
    g_held = false;
    g_frames_left = 0;
}

} // namespace

void vesarif_clock_setup_begin(uint8_t hour, uint8_t minute) {
    g_hour = hour;
    g_minute = minute;
    enter_phase(Phase::kTimeEnter);
}

bool vesarif_clock_setup_active() {
    return g_phase != Phase::kDone;
}

void vesarif_clock_setup_advance(SM5XX* cpu) {
    if (!cpu) return;

    // See uchitari_clock_setup.cpp's identical loop for why the "last tap
    // finished its release" check belongs here rather than transitioning
    // the instant taps_remaining hits 0 inside the release branch below -
    // skipping the last tap's release wait was a real bug caught there.
    while (g_phase != Phase::kDone && g_taps_remaining == 0 && !g_held && g_frames_left <= 0) {
        enter_phase(next_phase(g_phase));
    }
    if (g_phase == Phase::kDone) return;

    int group = group_for(g_phase), line = line_for(g_phase);

    if (g_frames_left <= 0) {
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
