#include "switch_rumble.h"

#include <algorithm>

#include <switch.h>

#include "core/switch_settings.h"

namespace {

HidVibrationDeviceHandle g_handheld[2], g_joydual[2], g_fullkey[1];
bool g_ok_handheld = false, g_ok_joydual = false, g_ok_fullkey = false;

float g_target = 0.0f;   // this frame's request, 0..1
float g_sent = -1.0f;    // last amplitude sent
int g_pulse_frames = 0;  // a short tap/pulse still running
float g_pulse_amp = 0.0f;
// A menu tap takes the Vibration Strength as it is when it's sent, not when
// the button was pressed: the tap is requested before the menu handles that
// same press, so changing Vibration Strength would otherwise buzz at the
// level being left instead of the one just chosen.
bool g_pulse_current_strength = false;
int g_sustain = 0;       // frames in a row the buzzer has been on

float strength() {
    static const float kAmp[3] = {0.25f, 0.5f, 0.85f};
    return kAmp[g_vibration_strength < 3 ? g_vibration_strength : 1];
}

void send(float amp) {
    HidVibrationValue v;
    v.amp_low = amp;
    v.freq_low = 160.0f;
    v.amp_high = amp;
    v.freq_high = 320.0f;
    HidVibrationValue vv[2] = {v, v};
    // Whatever is connected right now gets it; the rest just fail quietly.
    if (g_ok_handheld && (hidGetNpadStyleSet(HidNpadIdType_Handheld) & HidNpadStyleTag_NpadHandheld)) {
        hidSendVibrationValues(g_handheld, vv, 2);
    }
    const u32 style = hidGetNpadStyleSet(HidNpadIdType_No1);
    if (g_ok_joydual && (style & HidNpadStyleTag_NpadJoyDual)) hidSendVibrationValues(g_joydual, vv, 2);
    if (g_ok_fullkey && (style & HidNpadStyleTag_NpadFullKey)) hidSendVibrationValues(g_fullkey, vv, 1);
}

void pulse(float amp, int frames) {
    g_pulse_amp = std::max(g_pulse_amp, amp);
    g_pulse_frames = std::max(g_pulse_frames, frames);
}

}  // namespace

void rumble_init() {
    g_ok_handheld = R_SUCCEEDED(hidInitializeVibrationDevices(g_handheld, 2, HidNpadIdType_Handheld, HidNpadStyleTag_NpadHandheld));
    g_ok_joydual = R_SUCCEEDED(hidInitializeVibrationDevices(g_joydual, 2, HidNpadIdType_No1, HidNpadStyleTag_NpadJoyDual));
    g_ok_fullkey = R_SUCCEEDED(hidInitializeVibrationDevices(g_fullkey, 1, HidNpadIdType_No1, HidNpadStyleTag_NpadFullKey));
}

void rumble_update() {
    float amp = g_target;
    if (g_pulse_frames > 0) {
        // (Menu Vibration is checked here too, for the same reason: so
        // switching it on taps and switching it off doesn't.)
        const float p = g_pulse_current_strength ? (g_vibration_menu ? strength() : 0.0f) : g_pulse_amp;
        amp = std::max(amp, p);
        if (--g_pulse_frames == 0) {
            g_pulse_amp = 0.0f;
            g_pulse_current_strength = false;
        }
    }
    g_target = 0.0f;  // the game frame (if any) asks again
    if (amp != g_sent) {
        send(amp);
        g_sent = amp;
    }
}

void rumble_game_frame(float activity, float pitch_hz, bool is_pet) {
    const bool on = activity > 0.05f;
    g_sustain = on ? g_sustain + 1 : 0;
    if (!on) return;
    bool want = false;
    switch (g_vibration_game) {
        case 1:  // With Sound
            want = true;
            break;
        case 2:  // Strong Events: a beep held ~0.3 s, or a low one
            want = g_sustain >= 18 || (pitch_hz > 0.0f && pitch_hz < 900.0f);
            break;
        default:
            break;
    }
    if (!want && is_pet && g_vibration_pets) want = true;
    if (want) g_target = std::max(g_target, strength());
}

void rumble_sample_played() {
    if (g_vibration_game == 1) pulse(strength(), 6);
}

// Same strength as in game, and long enough (~70 ms) that each level is
// felt the same as a game buzz instead of a weaker flick.
void rumble_menu_tap() {
    g_pulse_current_strength = true;
    g_pulse_frames = std::max(g_pulse_frames, 4);
}

void rumble_stop() {
    g_target = 0.0f;
    g_pulse_frames = 0;
    send(0.0f);
    g_sent = 0.0f;
}
