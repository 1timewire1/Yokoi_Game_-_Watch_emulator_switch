#include "switch_sound.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <malloc.h>

#include "SM5XX/SM5XX.h"

Virtual_Sound g_sound;

constexpr int kVoiceId = 0;
constexpr int kSampleRate = 48000;

// Output level. The DC blocker turns the 0..1 buzzer signal into roughly
// -0.5..+0.5, so 0.8 gives the same peak-to-peak loudness the old 0..0.8
// unipolar square had.
constexpr float kGain = 0.8f;
// Gentle low-pass: takes the edge off the raw square wave without dulling
// it (the real piezo is far from a perfect square either).
constexpr float kLowPassHz = 9000.0f;
// DC blocker cut-off: removes the offset of the unipolar square so tones
// starting/stopping don't "pop".
constexpr float kHighPassHz = 15.0f;

// Frames of silence queued before playback starts (and again after any
// underrun). 3 frames = ~50 ms of headroom against frame-time jitter.
constexpr int kPrebufferFrames = 3;
// If more than this many slots are queued, latency is drifting up: drop
// this frame's audio to pull it back.
constexpr int kMaxQueuedFrames = NB_BUFFER - 2;

// Self-heal thresholds - see play_sample().
constexpr int kStuckFramesThreshold = 180;  // ~3s @60fps
constexpr int kRetryCooldownFrames = 300;   // ~5s @60fps between reinit attempts

static const float kLpCoef = 1.0f - std::exp(-2.0f * 3.14159265f * kLowPassHz / kSampleRate);
static const float kHpCoef = std::exp(-2.0f * 3.14159265f * kHighPassHz / kSampleRate);

void Virtual_Sound::configure_sound() {
    static const AudioRendererConfig arConfig = {
        .output_rate = AudioRendererOutputRate_48kHz,
        // Voice 0 is the tone synth (mono, 1 channel); lcdonald_sound.cpp
        // uses voices 1..8 as stereo sample voices. The renderer budgets
        // one "channel resource" per configured voice slot and each
        // channel of an initialized voice consumes one, so a stereo voice
        // costs two: with num_voices = 8 only voices 1-3 came up
        // (1 + 3*2 = 7 of 8) and voices 4-6 failed audrvVoiceInit() -
        // confirmed from a real log. 1 + 8*2 = 17, so 20 leaves headroom.
        .num_voices = 20,
        .num_effects = 0,
        .num_sinks = 1,
        .num_mix_objs = 1,
        .num_mix_buffers = 2,
    };

    audren_ok = R_SUCCEEDED(audrenInitialize(&arConfig));
    if (!audren_ok) {
        return;
    }

    driver_ok = R_SUCCEEDED(audrvCreate(&drv, &arConfig, 2));
    if (!driver_ok) {
        return;
    }

    static const uint8_t sink_channels[] = {0, 1};
    audrvDeviceSinkAdd(&drv, AUDREN_DEFAULT_DEVICE_NAME, 2, sink_channels);
    audrvUpdate(&drv);
    audrenStartAudioRenderer();
}

void Virtual_Sound::free_buffers() {
    if (voice_ok) {
        audrvVoiceStop(&drv, kVoiceId);
        audrvVoiceDrop(&drv, kVoiceId);
        voice_ok = false;
    }
    if (mempool_id >= 0) {
        audrvMemPoolDetach(&drv, mempool_id);
        audrvMemPoolRemove(&drv, mempool_id);
        mempool_id = -1;
    }
    if (mempool_ptr) {
        free(mempool_ptr);
        mempool_ptr = nullptr;
    }
    mempool_size = 0;
    for (auto& b : buffer_sound) {
        b = nullptr;
    }
    for (auto& wb : wavebuf) {
        wb = {};
    }
    free(staging);
    staging = nullptr;
    free(silence);
    silence = nullptr;
    curr_sequence = 0;
    primed = false;
}

void Virtual_Sound::reset_dsp_state() {
    curr_wait_before_update = 0;
    curr_value = false;
    out_frac = 0.0f;
    out_acc = 0.0f;
    lp_state = 0.0f;
    hp_prev_in = 0.0f;
    hp_state = 0.0f;
}

void Virtual_Sound::initialize(uint32_t v_freq, uint32_t v_divide_freq, float v_fps_screen) {
    if (!driver_ok) {
        return;
    }

    free_buffers();
    stuck_frames = 0;

    base_freq = v_freq ? v_freq : 32768;
    divide_freq = v_divide_freq ? v_divide_freq : 1;
    fps_screen = (v_fps_screen > 1.0f) ? v_fps_screen : 60.0f;

    // One update_sound() "tick" (divide_freq CPU steps) lasts this many
    // 48 kHz output samples.
    tick_len = (float)kSampleRate * (float)divide_freq / (float)base_freq;

    samples_per_frame = (uint32_t)(kSampleRate / fps_screen + 0.5f);
    // Room for a frame that runs long (e.g. CPU catch-up) without dropping.
    length_max_sequence = samples_per_frame * 2 + SEQUENCE_SECURITY_ADD;

    // Each slot 64-byte aligned inside a 4 KiB-aligned mempool.
    const size_t per_buffer_bytes = (length_max_sequence * sizeof(int16_t) + 0x3F) & ~size_t(0x3F);
    mempool_size = (per_buffer_bytes * NB_BUFFER + 0xFFF) & ~size_t(0xFFF);
    mempool_ptr = memalign(0x1000, mempool_size);
    staging = (int16_t*)malloc(length_max_sequence * sizeof(int16_t));
    silence = (int16_t*)calloc(samples_per_frame, sizeof(int16_t));
    if (!mempool_ptr || !staging || !silence) {
        free_buffers();
        return;
    }
    memset(mempool_ptr, 0, mempool_size);
    armDCacheFlush(mempool_ptr, mempool_size);

    mempool_id = audrvMemPoolAdd(&drv, mempool_ptr, mempool_size);
    audrvMemPoolAttach(&drv, mempool_id);

    for (size_t i = 0; i < NB_BUFFER; i++) {
        buffer_sound[i] = (int16_t*)((uint8_t*)mempool_ptr + i * per_buffer_bytes);
        wavebuf[i] = {};
    }

    curr_buffer = 0;
    curr_sequence = 0;
    primed = false;
    reset_dsp_state();

    // Native 48 kHz: we hand audren data at its own rate, so its resampler
    // is never involved.
    voice_ok = audrvVoiceInit(&drv, kVoiceId, 1 /* mono */, PcmFormat_Int16, kSampleRate);
    if (voice_ok) {
        audrvVoiceSetDestinationMix(&drv, kVoiceId, AUDREN_FINAL_MIX_ID);
        audrvVoiceSetMixFactor(&drv, kVoiceId, 1.0f, 0, 0);
        audrvVoiceSetMixFactor(&drv, kVoiceId, 1.0f, 0, 1);
        audrvVoiceStart(&drv, kVoiceId);
    }
    audrvUpdate(&drv);
}

void Virtual_Sound::full_reinit() {
    Exit();
    configure_sound();
    initialize(base_freq, divide_freq, fps_screen);
    reinit_generation_++;
}

// Low-pass + DC-block one 48 kHz sample and append it to the staging buffer.
void Virtual_Sound::push_output_sample(float area) {
    lp_state += kLpCoef * (area - lp_state);
    const float y = lp_state - hp_prev_in + kHpCoef * hp_state;
    hp_prev_in = lp_state;
    hp_state = y;

    if (curr_sequence < length_max_sequence) {
        float s = y * kGain * 32767.0f;
        s = std::min(32767.0f, std::max(-32768.0f, s));
        staging[curr_sequence++] = (int16_t)std::lrintf(s);
    }
}

// Called once per emulated CPU step (base_freq times per emulated second).
void Virtual_Sound::update_sound(SM5XX* cpu) {
    if (!voice_ok || !cpu || !staging) {
        return;
    }

    // Same semantics as the 3DS backend: the pin counts as "on" for this
    // tick if it was on at any point during the divide_freq steps.
    curr_wait_before_update += 1;
    curr_value = curr_value || cpu->get_active_sound();
    if (curr_wait_before_update < divide_freq) {
        return;
    }
    curr_wait_before_update = 0;

    const float level = curr_value ? 1.0f : 0.0f;
    curr_value = false;

    // Exact area sampling: spread this tick (tick_len output samples long)
    // over the 48 kHz output grid. Edges that fall mid-sample give an
    // intermediate value instead of snapping, which is what kills the
    // aliasing of a naive 32768 -> 48000 conversion.
    float remaining = tick_len;
    while (remaining > 0.0f) {
        const float take = std::min(remaining, 1.0f - out_frac);
        out_acc += level * take;
        out_frac += take;
        remaining -= take;
        if (out_frac >= 0.99999f) {
            push_output_sample(out_acc);
            out_acc = 0.0f;
            out_frac = 0.0f;
        }
    }
}

bool Virtual_Sound::slot_is_free(uint8_t slot) const {
    const auto st = wavebuf[slot].state;
    return st == AudioDriverWaveBufState_Free || st == AudioDriverWaveBufState_Done;
}

int Virtual_Sound::count_in_flight() const {
    int n = 0;
    for (uint8_t i = 0; i < NB_BUFFER; i++) {
        if (!slot_is_free(i)) {
            n++;
        }
    }
    return n;
}

// Copies `nsamples` into the next slot and queues it. Only ever writes to a
// slot audren has finished with.
bool Virtual_Sound::submit_slot(const int16_t* src, uint32_t nsamples) {
    if (nsamples == 0) {
        return true;
    }
    if (!slot_is_free(curr_buffer)) {
        return false;
    }
    nsamples = std::min(nsamples, length_max_sequence);

    int16_t* dst = buffer_sound[curr_buffer];
    memcpy(dst, src, nsamples * sizeof(int16_t));
    armDCacheFlush(dst, nsamples * sizeof(int16_t));

    AudioDriverWaveBuf& wb = wavebuf[curr_buffer];
    wb = {};
    wb.data_pcm16 = dst;
    wb.size = nsamples * sizeof(int16_t);
    wb.start_sample_offset = 0;
    wb.end_sample_offset = (int32_t)nsamples;
    wb.is_looping = false;

    if (!audrvVoiceAddWaveBuf(&drv, kVoiceId, &wb)) {
        wb = {};
        return false;
    }
    curr_buffer = (curr_buffer + 1) % NB_BUFFER;
    return true;
}

// Called once per rendered frame. Hands the samples generated since the
// last call to audren, keeping a small cushion queued. Also doubles as the
// self-heal tick: the audren session has been observed to die after
// sleep/resume or many game loads; if no slot frees up for ~3 s the whole
// pipeline is rebuilt (same approach as D.Smile NX's switch_audio.cpp).
void Virtual_Sound::play_sample() {
    if (!voice_ok) {
        if (retry_cooldown > 0) {
            retry_cooldown--;
            return;
        }
        full_reinit();
        if (!voice_ok) {
            retry_cooldown = kRetryCooldownFrames;
            return;
        }
    }

    // Refresh wavebuf states before looking at them.
    audrvUpdate(&drv);

    const int in_flight = count_in_flight();

    // First frame, or the queue ran dry (underrun): put a cushion of
    // silence in front so the next frames have room to arrive late.
    if (!primed || in_flight == 0) {
        for (int i = 0; i < kPrebufferFrames; i++) {
            submit_slot(silence, samples_per_frame);
        }
        primed = true;
    } else if (in_flight > kMaxQueuedFrames) {
        // Latency crept up (emulation ran slightly faster than playback):
        // drop this frame to bring it back down. If this never clears up,
        // nothing is being consumed any more: the session is dead.
        curr_sequence = 0;
        if (++stuck_frames > kStuckFramesThreshold) {
            full_reinit();
            return;
        }
        audrvUpdate(&drv);
        return;
    }

    if (curr_sequence > 0) {
        if (submit_slot(staging, curr_sequence)) {
            stuck_frames = 0;
        } else if (++stuck_frames > kStuckFramesThreshold) {
            full_reinit();
            return;
        }
        curr_sequence = 0;
    }

    audrvVoiceStart(&drv, kVoiceId);
    audrvUpdate(&drv);
}

void Virtual_Sound::Quit_Game() {
    free_buffers();
}

void Virtual_Sound::Exit() {
    Quit_Game();
    if (driver_ok) {
        audrvClose(&drv);
        driver_ok = false;
    }
    if (audren_ok) {
        audrenExit();
        audren_ok = false;
    }
}

void switch_sound_reconfigure(SM5XX* cpu) {
    if (!cpu) {
        return;
    }
    const uint32_t div = cpu->sound_divide_frequency ? cpu->sound_divide_frequency : 1;
    g_sound.initialize(cpu->frequency, div, 60.0f);
}
