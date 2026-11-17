#include "menu_sound.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <malloc.h>
#include <switch.h>

#include "switch_sound.h"
#include "menu_music.h"
#include "core/switch_settings.h"

namespace {

// Voice ids: 0 tone synth, 1-8 lcdonald_sound.cpp (stereo), 9-11
// step_sound.cpp, 12-13 these, 14 the startup splash - see
// switch_sound.cpp's renderer config (num_voices) for the budget.
constexpr int kFirstVoiceId = 12;
constexpr int kNumVoices = 2;
constexpr int kRate = 48000;

struct Beep { float freq, dur, vol; };
struct ClipDef { Beep beeps[3]; int n; };
// Move: one short tick. Change: low-high. Open: rising three notes. Back: high-low.
const ClipDef kDefs[4] = {
    {{{2400.0f, 0.018f, 0.6f}}, 1},
    {{{1800.0f, 0.025f, 0.6f}, {2700.0f, 0.025f, 0.6f}}, 2},
    {{{2000.0f, 0.040f, 1.0f}, {3000.0f, 0.040f, 1.0f}, {4000.0f, 0.070f, 1.0f}}, 3},
    {{{2600.0f, 0.030f, 0.7f}, {1700.0f, 0.050f, 0.7f}}, 2},
};
constexpr float kGapSec = 0.02f;

struct Clip { size_t offset, samples; };
Clip g_clips[4];
size_t g_pool_size = 0;

void* g_pool = nullptr;
int g_pool_id = -1;
bool g_ready = false;
uint32_t g_generation = 0;
AudioDriverWaveBuf g_wavebufs[kNumVoices] = {};
bool g_voice_ok[kNumVoices] = {};
int g_next = 0;

size_t clip_samples(const ClipDef& d) {
    size_t n = 0;
    for (int i = 0; i < d.n; ++i) n += (size_t)(d.beeps[i].dur * kRate) + (size_t)(kGapSec * kRate);
    return n;
}

void synth(const ClipDef& d, int16_t* out) {
    size_t pos = 0;
    for (int b = 0; b < d.n; ++b) {
        const size_t len = (size_t)(d.beeps[b].dur * kRate);
        for (size_t i = 0; i < len; ++i) {
            const float phase = std::fmod((float)i * d.beeps[b].freq / kRate, 1.0f);
            float env = 1.0f;
            if (i < 30) env = (float)i / 30.0f;
            if (i + 60 > len) env = std::min(env, (float)(len - i) / 60.0f);
            out[pos++] = (int16_t)((phase < 0.5f ? 1.0f : -1.0f) * env * d.beeps[b].vol * 9000.0f);
        }
        pos += (size_t)(kGapSec * kRate);  // already zeroed
    }
}

bool ensure_ready() {
    AudioDriver* drv = g_sound.driver();
    if (!drv) return g_ready = false;
    if (g_ready && g_generation == g_sound.reinit_generation()) return true;

    // Same recovery as step_sound.cpp: after a driver rebuild the old
    // registrations are gone, only our own buffer is left to reset.
    if (g_pool) free(g_pool);
    g_pool = nullptr;
    for (auto& wb : g_wavebufs) wb = {};
    for (auto& ok : g_voice_ok) ok = false;
    g_next = 0;
    g_generation = g_sound.reinit_generation();

    size_t off = 0;
    for (int i = 0; i < 4; ++i) {
        g_clips[i].offset = off;
        g_clips[i].samples = clip_samples(kDefs[i]);
        off = (off + g_clips[i].samples * sizeof(int16_t) + 0xFF) & ~size_t(0xFF);
    }
    g_pool_size = (off + 0xFFF) & ~size_t(0xFFF);
    g_pool = memalign(0x1000, g_pool_size);
    if (!g_pool) return g_ready = false;
    memset(g_pool, 0, g_pool_size);
    for (int i = 0; i < 4; ++i) synth(kDefs[i], (int16_t*)((uint8_t*)g_pool + g_clips[i].offset));
    armDCacheFlush(g_pool, g_pool_size);
    g_pool_id = audrvMemPoolAdd(drv, g_pool, g_pool_size);
    audrvMemPoolAttach(drv, g_pool_id);

    g_ready = false;
    for (int i = 0; i < kNumVoices; i++) {
        const int id = kFirstVoiceId + i;
        g_voice_ok[i] = audrvVoiceInit(drv, id, 1, PcmFormat_Int16, kRate);
        if (!g_voice_ok[i]) continue;
        audrvVoiceSetDestinationMix(drv, id, AUDREN_FINAL_MIX_ID);
        audrvVoiceSetMixFactor(drv, id, 1.0f, 0, 0);
        audrvVoiceSetMixFactor(drv, id, 1.0f, 0, 1);
        audrvVoiceStart(drv, id);
        g_ready = true;
    }
    audrvUpdate(drv);
    return g_ready;
}

}  // namespace

void menu_sound_play(MenuSound which) {
    if (!g_menu_sounds) return;
    const float vol = switch_sound_volume();
    if (vol <= 0.0f) return;
    if (!ensure_ready()) return;
    menu_music_duck();
    AudioDriver* drv = g_sound.driver();
    const Clip& c = g_clips[(int)which];
    // A free voice; if both are still busy (buttons mashed) this beep is skipped.
    for (int n = 0; n < kNumVoices; n++) {
        const int idx = (g_next + n) % kNumVoices;
        AudioDriverWaveBuf& wb = g_wavebufs[idx];
        const AudioDriverWaveBufState st = wb.state;
        if (!g_voice_ok[idx] || (st != AudioDriverWaveBufState_Free && st != AudioDriverWaveBufState_Done)) continue;
        const int id = kFirstVoiceId + idx;
        audrvVoiceSetMixFactor(drv, id, vol, 0, 0);
        audrvVoiceSetMixFactor(drv, id, vol, 0, 1);
        // Only the caller-owned fields (see lcdonald_sound.cpp's play_cached()).
        wb.data_pcm16 = (int16_t*)((uint8_t*)g_pool + c.offset);
        wb.size = (uint32_t)(c.samples * sizeof(int16_t));
        wb.start_sample_offset = 0;
        wb.end_sample_offset = (int32_t)c.samples;
        wb.is_looping = false;
        audrvVoiceAddWaveBuf(drv, id, &wb);
        audrvVoiceStart(drv, id);
        g_next = (idx + 1) % kNumVoices;
        break;
    }
    audrvUpdate(drv);
}
