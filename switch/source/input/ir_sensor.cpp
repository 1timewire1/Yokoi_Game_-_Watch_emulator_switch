#include "ir_sensor.h"

#include <switch.h>

#include "brick_input_map.h"
#include "switch_input.h"
#include "core/switch_settings.h"
#include "std/debug_log.h"

namespace {

bool g_irs_ok = false;
bool g_running = false;
IrsIrCameraHandle g_handle;
float g_baseline = -1.0f;
int g_above_frames = 0;
bool g_wave_sent = false;
uint64_t g_awake_until_ns = 0;  // Auto mode: the camera rests after this

uint64_t now_ns() { return armTicksToNs(armGetSystemTick()); }
constexpr uint64_t kAwakeNs = 60'000'000'000ull;  // a minute

void start() {
    if (g_running) return;
    if (!g_irs_ok) {
        g_irs_ok = R_SUCCEEDED(irsInitialize());
        if (!g_irs_ok) return;
    }
    // Detached Joy-Con on player 1, or the attached one in handheld mode.
    const HidNpadIdType id = (switch_input_style_set() & HidNpadStyleTag_NpadHandheld) ? HidNpadIdType_Handheld : HidNpadIdType_No1;
    if (R_FAILED(irsGetIrCameraHandle(&g_handle, id))) return;
    IrsMomentProcessorConfig cfg;
    irsGetMomentProcessorDefaultConfig(&cfg);
    if (R_FAILED(irsRunMomentProcessor(g_handle, &cfg))) return;
    g_running = true;
    g_baseline = -1.0f;
    g_above_frames = 0;
    g_wave_sent = false;
}

}  // namespace

void ir_sensor_stop() {
    if (g_running) irsStopImageProcessor(g_handle);
    g_running = false;
    if (g_irs_ok) irsExit();
    g_irs_ok = false;
}

void ir_sensor_update(bool game_has_ir_buttons, bool game_listening) {
    if (g_ir_sensor == 0 || !game_has_ir_buttons) {
        ir_sensor_stop();
        g_awake_until_ns = 0;
        return;
    }
    // Auto: only while the game is listening to its sensor (the modes that
    // use it - with a few seconds' grace, the line may be strobed), and
    // never more than a minute without a press or a hand.
    if (g_ir_sensor == 1) {
        const uint64_t now = now_ns();
        static uint64_t s_last_listen = 0;
        static bool s_was_listening = false;
        if (game_listening) s_last_listen = now;
        const bool listening = s_last_listen != 0 && now - s_last_listen < 3'000'000'000ull;
        if (listening != s_was_listening) {
            YOKOI_LOG("ir_sensor: game %s its sensor - camera %s", listening ? "listens to" : "stopped using",
                      listening ? "on" : "off");
            if (listening) g_awake_until_ns = now + kAwakeNs;
            s_was_listening = listening;
        }
        if (switch_input_raw_down()) g_awake_until_ns = now + kAwakeNs;
        if (!listening || now > g_awake_until_ns) {
            if (g_running) {
                irsStopImageProcessor(g_handle);
                g_running = false;
            }
            brick_input_ir_sensor(false);
            return;
        }
    }
    start();
    if (!g_running) return;

    IrsMomentProcessorState st;
    s32 total = 0;
    if (R_FAILED(irsGetMomentProcessorStates(g_handle, &st, 1, &total)) || total <= 0) return;
    float sum = 0.0f;
    constexpr int kBlocks = (int)(sizeof(st.statistic) / sizeof(st.statistic[0]));
    for (int i = 0; i < kBlocks; i++) sum += st.statistic[i].average_intensity;
    const float level = sum / (float)kBlocks;  // 0-255

    // The room's own level, followed slowly while nothing is in front.
    if (g_baseline < 0.0f) g_baseline = level;
    static const float kRise[3] = {45.0f, 25.0f, 12.0f};  // Low / Medium / High sensitivity
    const float rise = kRise[g_ir_sensitivity < 3 ? g_ir_sensitivity : 1];
    const bool hand = level - g_baseline > rise;
    if (!hand) g_baseline += (level - g_baseline) * 0.02f;

    if (hand) {
        if (g_ir_sensor == 1) g_awake_until_ns = now_ns() + kAwakeNs;
        g_above_frames++;
        // A hand coming in: one Wave.
        if (!g_wave_sent) {
            g_wave_sent = true;
            brick_input_ir_wave();
        }
    } else {
        g_above_frames = 0;
        g_wave_sent = false;
    }
    // Held over the camera for more than ~0.6 s: Sensor stays pressed.
    brick_input_ir_sensor(g_above_frames > 36);
}
