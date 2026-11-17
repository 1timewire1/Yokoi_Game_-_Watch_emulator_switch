#include "step_sound.h"

#include <cstring>
#include <malloc.h>
#include <switch.h>

#include "switch_sound.h"
#include "step_sound_data.h"

namespace {

// Voice 0 is switch_sound.cpp's tone voice and 1-8 lcdonald_sound.cpp's
// stereo sample voices (two channels each); these three mono ones come next
// (menu_sound.cpp and the splash use 12-14): a step sound (~250 ms, the hit and its
// rebound) can still be ringing out when the next step comes (X held: one
// every ~267 ms, or a quicker tap), so each step gets its own voice and they
// overlap like the real thing instead of cutting each other.
constexpr int kFirstVoiceId = 9;
constexpr int kNumVoices = 3;
struct Clip {
    const int16_t* pcm;
    size_t bytes;
    size_t offset;  // in the mempool, 0x100-aligned
};
// Laid out back to back in one mempool.
constexpr size_t align100(size_t v) { return (v + 0xFF) & ~size_t(0xFF); }
constexpr size_t kOffPikachu = 0;
constexpr size_t kOffDTector = align100(kOffPikachu + sizeof(kStepPikachu));
constexpr size_t kOffDPower = align100(kOffDTector + sizeof(kStepDTector));
constexpr size_t kPoolSize = (kOffDPower + sizeof(kStepDPower) + 0xFFF) & ~size_t(0xFFF);
const Clip kClips[] = {
    {kStepPikachu, sizeof(kStepPikachu), kOffPikachu},
    {kStepDTector, sizeof(kStepDTector), kOffDTector},
    {kStepDPower, sizeof(kStepDPower), kOffDPower},
};

void* g_pool = nullptr;
int g_pool_id = -1;
bool g_ready = false;
uint32_t g_generation = 0;
AudioDriverWaveBuf g_wavebufs[kNumVoices] = {};
bool g_voice_ok[kNumVoices] = {};
int g_next = 0;

// (Re)creates the voice and its mempool - first use, and again after
// switch_sound.cpp's self-heal replaced the shared driver (which takes every
// voice/mempool with it - see lcdonald_sound.cpp's teardown_voice()).
bool ensure_ready() {
    AudioDriver* drv = g_sound.driver();
    if (!drv) return g_ready = false;
    if (g_ready && g_generation == g_sound.reinit_generation()) return true;

    // The old driver (and so this voice/pool's registration) is already
    // gone; only our own buffer and bookkeeping are left to reset.
    if (g_pool) free(g_pool);
    g_pool = nullptr;
    for (auto& wb : g_wavebufs) wb = {};
    for (auto& ok : g_voice_ok) ok = false;
    g_next = 0;
    g_generation = g_sound.reinit_generation();

    g_pool = memalign(0x1000, kPoolSize);
    if (!g_pool) return g_ready = false;
    memset(g_pool, 0, kPoolSize);
    for (const Clip& c : kClips) memcpy((uint8_t*)g_pool + c.offset, c.pcm, c.bytes);
    armDCacheFlush(g_pool, kPoolSize);
    g_pool_id = audrvMemPoolAdd(drv, g_pool, kPoolSize);
    audrvMemPoolAttach(drv, g_pool_id);

    g_ready = false;
    for (int i = 0; i < kNumVoices; i++) {
        const int id = kFirstVoiceId + i;
        g_voice_ok[i] = audrvVoiceInit(drv, id, 1, PcmFormat_Int16, kStepSampleRate);
        if (!g_voice_ok[i]) continue;
        audrvVoiceSetDestinationMix(drv, id, AUDREN_FINAL_MIX_ID);
        audrvVoiceSetMixFactor(drv, id, 1.0f, 0, 0);  // mono -> left
        audrvVoiceSetMixFactor(drv, id, 1.0f, 0, 1);  // mono -> right
        audrvVoiceStart(drv, id);
        g_ready = true;
    }
    audrvUpdate(drv);
    return g_ready;
}

}  // namespace

void step_sound_play(StepSound which) {
    if (!ensure_ready()) return;
    AudioDriver* drv = g_sound.driver();
    // The next voice whose last sound has finished; if all three are still
    // playing (steps faster than ~12 a second, only by mashing X) this one is skipped.
    const Clip& c = kClips[(int)which];
    for (int n = 0; n < kNumVoices; n++) {
        const int idx = (g_next + n) % kNumVoices;
        AudioDriverWaveBuf& wb = g_wavebufs[idx];
        const AudioDriverWaveBufState st = wb.state;
        if (!g_voice_ok[idx] || (st != AudioDriverWaveBufState_Free && st != AudioDriverWaveBufState_Done)) continue;
        // Only the caller-owned fields (see lcdonald_sound.cpp's play_cached()).
        wb.data_pcm16 = (int16_t*)((uint8_t*)g_pool + c.offset);
        wb.size = (uint32_t)c.bytes;
        wb.start_sample_offset = 0;
        wb.end_sample_offset = (int32_t)(c.bytes / sizeof(int16_t));
        wb.is_looping = false;
        // Settings > Sound > Volume / Mute.
        audrvVoiceSetMixFactor(drv, kFirstVoiceId + idx, switch_sound_volume(), 0, 0);
        audrvVoiceSetMixFactor(drv, kFirstVoiceId + idx, switch_sound_volume(), 0, 1);
        audrvVoiceAddWaveBuf(drv, kFirstVoiceId + idx, &wb);
        audrvVoiceStart(drv, kFirstVoiceId + idx);
        g_next = (idx + 1) % kNumVoices;
        break;
    }
    audrvUpdate(drv);
}
