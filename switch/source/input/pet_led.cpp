#include "pet_led.h"

#include <cstring>
#include <switch.h>

#include "core/switch_settings.h"

namespace {

bool g_ok = false;
bool g_on = false;
uint64_t g_last_input_ns = 0;

uint64_t now_ns() { return armTicksToNs(armGetSystemTick()); }

void set_pattern(const HidsysNotificationLedPattern& pattern) {
    // Handheld (the attached right Joy-Con) and player 1 (a detached
    // Joy-Con pair or a Pro Controller) - whichever are there.
    const HidNpadIdType ids[2] = {HidNpadIdType_Handheld, HidNpadIdType_No1};
    for (HidNpadIdType id : ids) {
        HidsysUniquePadId pads[2];
        s32 total = 0;
        if (R_FAILED(hidsysGetUniquePadsFromNpad(id, pads, 2, &total))) continue;
        for (s32 i = 0; i < total && i < 2; i++) hidsysSetNotificationLedPattern(&pattern, pads[i]);
    }
}

void light(bool on) {
    if (!g_ok || on == g_on) return;
    g_on = on;
    HidsysNotificationLedPattern p;
    memset(&p, 0, sizeof(p));
    if (on) {
        // A quick pulse, repeated until switched off.
        p.baseMiniCycleDuration = 0x4;       // 50 ms
        p.totalMiniCycles = 0x2;
        p.totalFullCycles = 0x0;             // forever
        p.startIntensity = 0x0;
        p.miniCycles[0].ledIntensity = 0xF;  // full
        p.miniCycles[0].transitionSteps = 0x4;
        p.miniCycles[0].finalStepDuration = 0x2;
        p.miniCycles[1].ledIntensity = 0x0;  // off
        p.miniCycles[1].transitionSteps = 0x4;
        p.miniCycles[1].finalStepDuration = 0x4;
    }
    set_pattern(p);
}

}  // namespace

void pet_led_init() {
    g_ok = R_SUCCEEDED(hidsysInitialize());
    g_last_input_ns = now_ns();
}

void pet_led_update(uint64_t raw_down) {
    if (raw_down) {
        g_last_input_ns = now_ns();
        light(false);
    }
    if (!g_pet_alert_light) light(false);
}

void pet_led_pet_frame(float buzzer_activity) {
    if (!g_pet_alert_light || buzzer_activity <= 0.0f) return;
    // Pets also beep on every key press: only a beep with nothing pressed
    // for 3 seconds counts as the pet calling.
    if (now_ns() - g_last_input_ns < 3'000'000'000ull) return;
    light(true);
}

void pet_led_shutdown() {
    light(false);
    if (g_ok) hidsysExit();
    g_ok = false;
}
