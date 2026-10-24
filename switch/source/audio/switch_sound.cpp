#include "switch_sound.h"

#include <algorithm>
#include <cstring>
#include <malloc.h>

#include "SM5XX/SM5XX.h"

Virtual_Sound g_sound;

constexpr float LIMIT_SQUARE = 0.8f;
constexpr int kVoiceId = 0;
constexpr int kSampleRate = 48000;

// Self-heal thresholds - see play_sample()'s doc comment for what these
// guard against. Same values D.Smile NX's switch_audio.cpp settled on for
// the identical underlying issue.
constexpr int kStuckFramesThreshold = 180;  // ~3s @60fps - well past any normal backpressure drop
constexpr int kRetryCooldownFrames = 300;   // ~5s @60fps between reinit attempts, so a genuinely
                                             // dead audio service isn't hammered every frame

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
}

void Virtual_Sound::initialize(uint32_t v_freq, uint32_t v_divide_freq, float v_fps_screen) {
    if (!driver_ok) {
        return;
    }

    free_buffers();
    stuck_frames = 0;

    base_freq = v_freq;
    divide_freq = v_divide_freq ? v_divide_freq : 1;
    fps_screen = v_fps_screen;

    length_max_sequence = (uint32_t)(base_freq / (fps_screen * divide_freq)) + 1;
    length_max_sequence += SEQUENCE_SECURITY_ADD;

    const size_t per_buffer_bytes = length_max_sequence * sizeof(int16_t);
    mempool_size = (per_buffer_bytes * NB_BUFFER + 0xFFF) & ~size_t(0xFFF);
    mempool_ptr = memalign(0x1000, mempool_size);
    if (!mempool_ptr) {
        return;
    }
    memset(mempool_ptr, 0, mempool_size);

    mempool_id = audrvMemPoolAdd(&drv, mempool_ptr, mempool_size);
    audrvMemPoolAttach(&drv, mempool_id);

    for (size_t i = 0; i < NB_BUFFER; i++) {
        buffer_sound[i] = (int16_t*)((uint8_t*)mempool_ptr + i * per_buffer_bytes);
        curr_size_buffer[i] = base_freq / (uint32_t)(fps_screen * divide_freq);
        wavebuf[i] = {};
    }

    curr_buffer = 0;
    curr_sequence = 0;
    curr_wait_before_update = 0;
    accu_freq_sequence = 0;
    curr_value = false;

    voice_ok = audrvVoiceInit(&drv, kVoiceId, 1 /* mono */, PcmFormat_Int16, kSampleRate);
    if (voice_ok) {
        audrvVoiceSetDestinationMix(&drv, kVoiceId, AUDREN_FINAL_MIX_ID);
        audrvVoiceSetMixFactor(&drv, kVoiceId, 1.0f, 0, 0);
        audrvVoiceSetMixFactor(&drv, kVoiceId, 1.0f, 0, 1);
        audrvVoiceStart(&drv, kVoiceId);
    }
    audrvUpdate(&drv);
}

// Full teardown + recreate of the renderer/driver/voice/buffers, restoring
// the last-known per-game frequency/divide_freq/fps_screen (Exit() doesn't
// touch those fields) - see play_sample()'s doc comment for when this runs.
void Virtual_Sound::full_reinit() {
    Exit();
    configure_sound();
    initialize(base_freq, divide_freq, fps_screen);
    reinit_generation_++;
}

void Virtual_Sound::update_sound(SM5XX* cpu) {
    if (!voice_ok || !cpu) {
        return;
    }

    curr_wait_before_update += 1;
    curr_value = curr_value || cpu->get_active_sound();
    if (curr_wait_before_update < divide_freq) {
        return;
    }
    curr_wait_before_update = 0;

    if (curr_sequence < curr_size_buffer[curr_buffer] && curr_sequence < length_max_sequence) {
        buffer_sound[curr_buffer][curr_sequence] = curr_value ? (int16_t)(INT16_MAX * LIMIT_SQUARE) : 0;
        curr_sequence += 1;
    }
    curr_value = false;
}

// Called once per rendered frame regardless of state, so it doubles as the
// self-heal "tick": libnx's audrv/audren session (created once at startup
// by configure_sound(), unlike the per-game initialize() call) has been
// observed to get stuck - going silent for the rest of the process's life -
// after the console is put to sleep and resumed, or after enough game loads
// without a full relaunch. A relaunch (which re-runs configure_sound() from
// scratch) always fixed it by hand; this automates that exact recovery
// instead of requiring one. Ported from D.Smile NX's switch_audio.cpp
// (switch_audio_push()), which hit and fixed the identical issue in its own
// audrv-based Switch audio backend.
void Virtual_Sound::play_sample() {
    if (!voice_ok) {
        // Never initialized (or an earlier self-heal attempt also failed) -
        // retry on a cooldown instead of every frame, so a genuinely dead
        // audio service isn't hammered with reinit attempts 60 times/sec.
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

    // Drop this frame's audio if the slot we'd reuse is still in flight,
    // rather than overwriting a buffer audrv's own wavebuf queue may still
    // be reading from (normal, occasional backpressure). But since this
    // only ever retries the *same* slot (curr_buffer doesn't advance below
    // on this path), staying stuck on one slot for way longer than a slow
    // frame or two means that wavebuf's state will never come unstuck on
    // its own - so if it drags on, self-heal the whole audio pipeline.
    AudioDriverWaveBuf& wb = wavebuf[curr_buffer];
    if (wb.state != AudioDriverWaveBufState_Free && wb.state != AudioDriverWaveBufState_Done) {
        if (++stuck_frames > kStuckFramesThreshold) {
            full_reinit();
        }
        return;
    }
    stuck_frames = 0;

    const int16_t last_value = (curr_sequence > 0) ? buffer_sound[curr_buffer][curr_sequence - 1] : 0;
    while (curr_sequence < curr_size_buffer[curr_buffer] && curr_sequence < length_max_sequence) {
        buffer_sound[curr_buffer][curr_sequence] = last_value;
        curr_sequence += 1;
    }

    const size_t nsamples = curr_size_buffer[curr_buffer];
    wb.data_pcm16 = buffer_sound[curr_buffer];
    wb.size = nsamples * sizeof(int16_t);
    wb.start_sample_offset = 0;
    wb.end_sample_offset = (int32_t)nsamples;
    wb.is_looping = false;

    armDCacheFlush(buffer_sound[curr_buffer], nsamples * sizeof(int16_t));
    audrvVoiceAddWaveBuf(&drv, kVoiceId, &wb);
    audrvVoiceStart(&drv, kVoiceId);
    audrvUpdate(&drv);

    curr_buffer = (curr_buffer + 1) % NB_BUFFER;
    curr_sequence = 0;

    accu_freq_sequence = accu_freq_sequence + base_freq;
    curr_size_buffer[curr_buffer] = (size_t)(accu_freq_sequence / (fps_screen * divide_freq));
    if (curr_size_buffer[curr_buffer] > length_max_sequence) {
        curr_size_buffer[curr_buffer] = length_max_sequence;
    }
    accu_freq_sequence = accu_freq_sequence - (uint32_t)(curr_size_buffer[curr_buffer] * fps_screen * divide_freq);
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
