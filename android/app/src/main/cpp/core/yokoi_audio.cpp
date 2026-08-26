#include "yokoi_audio.h"

#include <aaudio/AAudio.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <vector>

#include "SM5XX/SM5XX.h"

namespace {
std::atomic<bool> g_audio_can_run{false};

// ---------------------------
// Audio (very minimal buzzer)
// ---------------------------
std::mutex g_audio_mutex;
std::vector<int16_t> g_audio_ring;
size_t g_audio_r = 0;
size_t g_audio_w = 0;
int g_audio_sample_rate = 0;
uint16_t g_audio_wait = 0;
bool g_audio_curr_value = false;

// ---------------------------
// AAudio output (minSdk=26)
// ---------------------------
std::mutex g_aaudio_mutex;
AAudioStream* g_aaudio_stream = nullptr;
std::atomic<bool> g_aaudio_running{false};
std::atomic<int> g_aaudio_output_rate{0};

struct AAudioResamplerState {
    int source_rate = 0;
    int output_rate = 0;
    float src_pos = 0.0f;
    std::vector<int16_t> src;
    int src_count = 0;
    int src_index = 0;
};

AAudioResamplerState g_aaudio_rs;

static void audio_reset_locked() {
    std::fill(g_audio_ring.begin(), g_audio_ring.end(), 0);
    g_audio_r = 0;
    g_audio_w = 0;
    g_audio_wait = 0;
    g_audio_curr_value = false;
}

static void audio_push_sample_locked(int16_t s) {
    if (g_audio_ring.empty()) {
        return;
    }

    g_audio_ring[g_audio_w] = s;
    size_t next_w = g_audio_w + 1;
    if (next_w >= g_audio_ring.size()) {
        next_w = 0;
    }

    // Overwrite oldest if full.
    if (next_w == g_audio_r) {
        size_t next_r = g_audio_r + 1;
        if (next_r >= g_audio_ring.size()) {
            next_r = 0;
        }
        g_audio_r = next_r;
    }
    g_audio_w = next_w;
}

static int get_source_audio_rate_locked() {
    if (g_audio_sample_rate > 0) {
        return g_audio_sample_rate;
    }
    return 32768;
}

static int audio_ring_read_locked(int16_t* out, int frames) {
    if (!out || frames <= 0 || g_audio_ring.empty()) {
        return 0;
    }
    int got = 0;
    for (; got < frames; got++) {
        if (g_audio_r == g_audio_w) {
            break;
        }
        out[got] = g_audio_ring[g_audio_r];
        g_audio_r = (g_audio_r + 1) % g_audio_ring.size();
    }
    return got;
}

static aaudio_data_callback_result_t aaudio_data_cb(AAudioStream* /*stream*/, void* /*userData*/, void* audioData, int32_t numFrames) {
    int16_t* out = reinterpret_cast<int16_t*>(audioData);
    if (!out || numFrames <= 0) {
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    if (!g_aaudio_running.load() || !g_audio_can_run.load()) {
        std::fill(out, out + numFrames, 0);
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    const int out_rate = g_aaudio_output_rate.load();
    int source_rate = 0;
    {
        std::lock_guard<std::mutex> audio_lock(g_audio_mutex);
        source_rate = get_source_audio_rate_locked();
    }
    if (out_rate <= 0 || source_rate <= 0) {
        std::fill(out, out + numFrames, 0);
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    // Reset/resume resampler state if rates changed.
    if (g_aaudio_rs.source_rate != source_rate || g_aaudio_rs.output_rate != out_rate) {
        g_aaudio_rs.source_rate = source_rate;
        g_aaudio_rs.output_rate = out_rate;
        g_aaudio_rs.src_pos = 0.0f;
        g_aaudio_rs.src.clear();
        g_aaudio_rs.src_count = 0;
        g_aaudio_rs.src_index = 0;
    }

    if (source_rate == out_rate) {
        std::lock_guard<std::mutex> audio_lock(g_audio_mutex);
        const int got = audio_ring_read_locked(out, (int)numFrames);
        if (got < numFrames) {
            std::fill(out + got, out + numFrames, 0);
        }
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    // Linear resample from source_rate to out_rate.
    const float step = (float)source_rate / (float)out_rate;

    // Ensure we have a source buffer.
    if (g_aaudio_rs.src.empty()) {
        g_aaudio_rs.src.resize(1024);
        g_aaudio_rs.src_count = 0;
        g_aaudio_rs.src_index = 0;
        g_aaudio_rs.src_pos = 0.0f;
    }

    for (int i = 0; i < numFrames; i++) {
        int i0 = g_aaudio_rs.src_index + (int)g_aaudio_rs.src_pos;
        float frac = g_aaudio_rs.src_pos - (float)((int)g_aaudio_rs.src_pos);

        // Make sure we have i0 and i0+1 available.
        while (i0 + 1 >= g_aaudio_rs.src_count) {
            // Shift remaining samples down.
            int remain = std::max(0, g_aaudio_rs.src_count - g_aaudio_rs.src_index);
            if (remain > 0) {
                memmove(g_aaudio_rs.src.data(), g_aaudio_rs.src.data() + g_aaudio_rs.src_index, (size_t)remain * sizeof(int16_t));
            }
            g_aaudio_rs.src_index = 0;
            g_aaudio_rs.src_count = remain;
            g_aaudio_rs.src_pos = 0.0f;
            i0 = 0;
            frac = 0.0f;

            const int need = (int)g_aaudio_rs.src.size() - g_aaudio_rs.src_count;
            if (need <= 0) {
                break;
            }
            // Pull more source samples from the native ring.
            int got = 0;
            {
                std::lock_guard<std::mutex> audio_lock(g_audio_mutex);
                got = audio_ring_read_locked(g_aaudio_rs.src.data() + g_aaudio_rs.src_count, need);
            }
            if (got > 0) {
                g_aaudio_rs.src_count += got;
            }
            if (g_aaudio_rs.src_count <= 1) {
                break;
            }
        }

        int16_t s0 = (g_aaudio_rs.src_count > 0 && i0 < g_aaudio_rs.src_count) ? g_aaudio_rs.src[i0] : 0;
        int16_t s1 = (g_aaudio_rs.src_count > 1 && (i0 + 1) < g_aaudio_rs.src_count) ? g_aaudio_rs.src[i0 + 1] : s0;
        out[i] = (int16_t)((float)s0 + ((float)s1 - (float)s0) * frac);

        g_aaudio_rs.src_pos += step;
        int adv = (int)g_aaudio_rs.src_pos;
        if (adv > 0) {
            g_aaudio_rs.src_index += adv;
            g_aaudio_rs.src_pos -= (float)adv;
        }
    }

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

static void aaudio_start_stream_locked() {
    if (g_aaudio_stream) {
        return;
    }

    AAudioStreamBuilder* builder = nullptr;
    if (AAudio_createStreamBuilder(&builder) != AAUDIO_OK || !builder) {
        return;
    }

    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setChannelCount(builder, 1);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    // Let the system pick the native rate (we resample if needed).
    AAudioStreamBuilder_setSampleRate(builder, 0);
    AAudioStreamBuilder_setDataCallback(builder, aaudio_data_cb, nullptr);

    AAudioStream* stream = nullptr;
    aaudio_result_t openRes = AAudioStreamBuilder_openStream(builder, &stream);
    AAudioStreamBuilder_delete(builder);
    builder = nullptr;

    if (openRes != AAUDIO_OK || !stream) {
        return;
    }

    g_aaudio_stream = stream;
    const int sr = AAudioStream_getSampleRate(stream);
    g_aaudio_output_rate.store(sr > 0 ? sr : 0);

    // Start.
    if (AAudioStream_requestStart(stream) != AAUDIO_OK) {
        AAudioStream_close(stream);
        g_aaudio_stream = nullptr;
        g_aaudio_output_rate.store(0);
        return;
    }
    g_aaudio_running.store(true);
}

static void aaudio_stop_stream_locked() {
    g_aaudio_running.store(false);
    if (g_aaudio_stream) {
        AAudioStream_requestStop(g_aaudio_stream);
        AAudioStream_close(g_aaudio_stream);
        g_aaudio_stream = nullptr;
        g_aaudio_output_rate.store(0);
    }
    g_aaudio_rs = AAudioResamplerState{};
}
} // namespace

void yokoi_audio_set_can_run(bool can_run) {
    g_audio_can_run.store(can_run);
}

void yokoi_audio_reconfigure_from_cpu(SM5XX* cpu) {
    if (!cpu) {
        return;
    }

    uint16_t div = cpu->sound_divide_frequency ? (uint16_t)cpu->sound_divide_frequency : (uint16_t)1;
    int rate = (int)(cpu->frequency / (uint32_t)div);
    if (rate <= 0) {
        rate = 32768;
    }

    std::lock_guard<std::mutex> lock(g_audio_mutex);
    g_audio_sample_rate = rate;
    // Keep ring buffer relatively small to avoid building up noticeable latency.
    // We still overwrite on full (dropping oldest) to favor "latest" audio.
    const size_t target = (size_t)g_audio_sample_rate / 2u; // ~0.5s
    g_audio_ring.assign(std::max<size_t>(target, 2048u), 0);
    audio_reset_locked();
}

void yokoi_audio_update_step(SM5XX* cpu) {
    if (!cpu) {
        return;
    }

    uint16_t div = cpu->sound_divide_frequency ? (uint16_t)cpu->sound_divide_frequency : (uint16_t)1;
    g_audio_wait += 1;
    g_audio_curr_value = g_audio_curr_value || cpu->get_active_sound();
    if (g_audio_wait < div) {
        return;
    }
    g_audio_wait = 0;

    // Square wave amplitude (matches 3DS behavior: on/off sample stream).
    constexpr float kLimit = 0.8f;
    int16_t sample = g_audio_curr_value ? (int16_t)(32767.0f * kLimit) : (int16_t)0;
    g_audio_curr_value = false;

    std::lock_guard<std::mutex> lock(g_audio_mutex);
    audio_push_sample_locked(sample);
}

int yokoi_audio_get_source_rate() {
    std::lock_guard<std::mutex> lock(g_audio_mutex);
    return get_source_audio_rate_locked();
}

int yokoi_audio_read(int16_t* out, int frames) {
    if (!out || frames <= 0) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(g_audio_mutex);
    const int got = audio_ring_read_locked(out, frames);
    if (got < frames) {
        std::fill(out + got, out + frames, 0);
    }
    return frames;
}

void yokoi_aaudio_start_stream() {
    std::lock_guard<std::mutex> lock(g_aaudio_mutex);
    aaudio_start_stream_locked();
}

void yokoi_aaudio_stop_stream() {
    std::lock_guard<std::mutex> lock(g_aaudio_mutex);
    aaudio_stop_stream_locked();
}
