#include "motion_steps.h"

#include <algorithm>
#include <cmath>
#include <switch.h>

#include "brick_input_map.h"
#include "switch_input.h"
#include "core/switch_settings.h"

namespace {

HidSixAxisSensorHandle g_handheld[1], g_fullkey[1], g_dual[2];
bool g_started = false;
bool g_armed = true;          // waits for the shake to settle before the next step
uint64_t g_last_step_ns = 0;
int g_tap_frames = 0;         // frames the tap sensor stays triggered
float g_prev_dev = 0.0f;

uint64_t now_ns() { return armTicksToNs(armGetSystemTick()); }

void start() {
    if (g_started) return;
    hidGetSixAxisSensorHandles(g_handheld, 1, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld);
    hidGetSixAxisSensorHandles(g_fullkey, 1, HidNpadIdType_No1, HidNpadStyleTag_NpadFullKey);
    hidGetSixAxisSensorHandles(g_dual, 2, HidNpadIdType_No1, HidNpadStyleTag_NpadJoyDual);
    hidStartSixAxisSensor(g_handheld[0]);
    hidStartSixAxisSensor(g_fullkey[0]);
    hidStartSixAxisSensor(g_dual[0]);
    hidStartSixAxisSensor(g_dual[1]);
    g_started = true;
    g_armed = true;
}

bool read(HidSixAxisSensorState& s) {
    const uint32_t style = switch_input_style_set();
    const uint32_t attrib = switch_input_attributes();
    if (style & HidNpadStyleTag_NpadHandheld) return hidGetSixAxisSensorStates(g_handheld[0], &s, 1) > 0;
    if (style & HidNpadStyleTag_NpadFullKey) return hidGetSixAxisSensorStates(g_fullkey[0], &s, 1) > 0;
    if (style & HidNpadStyleTag_NpadJoyDual) {
        // The right Joy-Con if it's there (where the step "button" is).
        if (attrib & HidNpadAttribute_IsRightConnected) return hidGetSixAxisSensorStates(g_dual[1], &s, 1) > 0;
        if (attrib & HidNpadAttribute_IsLeftConnected) return hidGetSixAxisSensorStates(g_dual[0], &s, 1) > 0;
    }
    return false;
}

}  // namespace

void motion_steps_stop() {
    if (!g_started) return;
    hidStopSixAxisSensor(g_handheld[0]);
    hidStopSixAxisSensor(g_fullkey[0]);
    hidStopSixAxisSensor(g_dual[0]);
    hidStopSixAxisSensor(g_dual[1]);
    g_started = false;
}

void motion_tap_update(bool game_has_tap_sensor) {
    if (g_tap_frames > 0) g_tap_frames--;
    if (!g_motion_tap_sensor || !game_has_tap_sensor) {
        g_tap_frames = 0;
        motion_steps_stop();
        return;
    }
    // A touch on the screen (handheld) taps the case for as long as it lasts.
    HidTouchScreenState ts = {};
    if (hidGetTouchScreenStates(&ts, 1) && ts.count > 0) g_tap_frames = std::max(g_tap_frames, 2);
    start();
    HidSixAxisSensorState s = {};
    if (!read(s)) return;
    const float ax = s.acceleration.x, ay = s.acceleration.y, az = s.acceleration.z;
    const float dev = std::fabs(std::sqrt(ax * ax + ay * ay + az * az) - 1.0f);
    // A knock is a short jolt, lighter than a step (Tap Sensor
    // Sensitivity: Low / Medium / High). Held ~0.4 s so TamaOtch's
    // rehearsal sees it bounce.
    static const float kTapThreshold[3] = {0.6f, 0.35f, 0.2f};
    const float th = kTapThreshold[g_tap_sensitivity < 3 ? g_tap_sensitivity : 1];
    if (dev > th && g_prev_dev <= th) g_tap_frames = 24;
    g_prev_dev = dev;
}

bool motion_tap_active() { return g_tap_frames > 0; }

void motion_steps_update(bool game_has_step_button) {
    if (!g_motion_pedometer || !game_has_step_button) {
        motion_steps_stop();
        return;
    }
    start();
    HidSixAxisSensorState s = {};
    if (!read(s)) return;
    // How far the total acceleration is from resting (1 G): a shake or a
    // footfall swings it well above and below.
    const float ax = s.acceleration.x, ay = s.acceleration.y, az = s.acceleration.z;
    const float dev = std::fabs(std::sqrt(ax * ax + ay * ay + az * az) - 1.0f);
    static const float kThreshold[3] = {0.9f, 0.55f, 0.3f};  // Low / Medium / High sensitivity
    const float th = kThreshold[g_pedometer_sensitivity < 3 ? g_pedometer_sensitivity : 1];
    if (!g_armed) {
        if (dev < th * 0.5f) g_armed = true;
        return;
    }
    // At most ~4 steps a second, like the X turbo.
    if (dev > th && now_ns() - g_last_step_ns >= 250'000'000ull) {
        g_last_step_ns = now_ns();
        g_armed = false;
        brick_input_motion_step();
    }
}
