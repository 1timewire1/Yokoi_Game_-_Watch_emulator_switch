#include "switch_sound.h"

#include <algorithm>
#include <cstring>
#include <malloc.h>

#include "SM5XX/SM5XX.h"

Virtual_Sound g_sound;

constexpr float LIMIT_SQUARE = 0.8f;
constexpr int kVoiceId = 0;
constexpr int kSampleRate = 48000;

void Virtual_Sound::configure_sound() {
    static const AudioRendererConfig arConfig = {
        .output_rate = AudioRendererOutputRate_48kHz,
        .num_voices = 4,
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

void Virtual_Sound::play_sample() {
    if (!voice_ok) {
        return;
    }

    const int16_t last_value = (curr_sequence > 0) ? buffer_sound[curr_buffer][curr_sequence - 1] : 0;
    while (curr_sequence < curr_size_buffer[curr_buffer] && curr_sequence < length_max_sequence) {
        buffer_sound[curr_buffer][curr_sequence] = last_value;
        curr_sequence += 1;
    }

    const size_t nsamples = curr_size_buffer[curr_buffer];
    AudioDriverWaveBuf& wb = wavebuf[curr_buffer];
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
