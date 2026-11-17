#include "menu_music.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <malloc.h>
#include <memory>
#include <random>
#include <switch.h>

#include "switch_sound.h"
#include "core/switch_settings.h"
#include "std/platform_paths.h"

// Header-only here - the implementation is compiled once in
// third_party/stb_vorbis.cpp (built with STB_VORBIS_NO_STDIO, so files are
// read into memory and decoded with stb_vorbis_open_memory()).
#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_STDIO
#include "../third_party/stb_vorbis.h"

namespace {

// Voice ids: 0 tone synth, 1-8 lcdonald_sound.cpp (stereo), 9-11
// step_sound.cpp, 12-13 menu_sound.cpp, 14 the splash, 15 this one
// (stereo, so two of switch_sound.cpp's num_voices channel resources).
constexpr int kVoiceId = 15;
constexpr int kChannels = 2;
constexpr int kBufFrames = 4096;   // ~85 ms at 48 kHz per buffer
constexpr int kNumBufs = 4;
constexpr size_t kMaxFileBytes = 48u * 1024u * 1024u;

// ---------------------------------------------------------------- tunes ---
// Same synthesis as the previews: each note is its own waveform burst with a
// short attack, an optional exponential decay and a short release; the
// tracks are summed and the mix normalized to 0.45 of full scale.
struct Note { int8_t midi; uint8_t quarters; };   // midi -1 = rest; length in 1/4 beats
struct Track { const Note* notes; int n; uint8_t kind; float vol; float decay; };  // kind: 0 square, 1 pulse 25%, 2 triangle, 3 sine
struct Song { float bpm; const Track* tracks; int ntracks; };
constexpr int kNumSongs = kNumBuiltinTunes;
#include "menu_music_songs.inc"

constexpr int kRate = 48000;

float osc(int kind, float ph) {
    switch (kind) {
        case 0: return ph < 0.5f ? 1.0f : -1.0f;
        case 1: return ph < 0.25f ? 1.0f : -1.0f;
        case 2: return 4.0f * std::fabs(ph - 0.5f) - 1.0f;
        default: return std::sin(6.28318531f * ph);
    }
}

// Renders one loop of a tune (mono, 48 kHz). nullptr on out of memory.
int16_t* render_song(const Song& s, size_t& frames) {
    const double beat = 60.0 / s.bpm;
    size_t total = 0;
    for (int t = 0; t < s.ntracks; ++t) {
        size_t len = 0;
        for (int i = 0; i < s.tracks[t].n; ++i) len += (size_t)(s.tracks[t].notes[i].quarters * 0.25 * beat * kRate);
        total = std::max(total, len);
    }
    std::unique_ptr<float[]> mix(new (std::nothrow) float[total]());
    if (!mix) return nullptr;
    for (int t = 0; t < s.ntracks; ++t) {
        const Track& tr = s.tracks[t];
        size_t pos = 0;
        for (int i = 0; i < tr.n; ++i) {
            const size_t len = (size_t)(tr.notes[i].quarters * 0.25 * beat * kRate);
            if (tr.notes[i].midi >= 0) {
                const float f = 440.0f * std::pow(2.0f, (tr.notes[i].midi - 69) / 12.0f);
                const size_t att = std::min<size_t>(200, len), rel = std::min<size_t>(600, len);
                for (size_t k = 0; k < len && pos + k < total; ++k) {
                    const float tsec = (float)k / kRate;
                    float env = k < att ? (float)k / (float)att : 1.0f;
                    if (tr.decay > 0.0f) env *= std::exp(-tsec / tr.decay);
                    if (k >= len - rel) env *= (float)(len - k) / (float)rel;
                    const float ph = std::fmod((float)((double)f * k / kRate), 1.0f);
                    mix[pos + k] += osc(tr.kind, ph) * env * tr.vol;
                }
            }
            pos += len;
        }
    }
    float peak = 1e-6f;
    for (size_t i = 0; i < total; ++i) peak = std::max(peak, std::fabs(mix[i]));
    int16_t* out = (int16_t*)malloc(total * sizeof(int16_t));
    if (!out) return nullptr;
    const float g = 0.45f / peak * 32767.0f;
    for (size_t i = 0; i < total; ++i) out[i] = (int16_t)std::lrintf(mix[i] * g);
    frames = total;
    return out;
}

// -------------------------------------------------------------- sources ---
// Each fills interleaved stereo int16 and returns the frames written;
// fewer than asked (0 included) = the file ended (tunes never end).
struct Source {
    virtual ~Source() = default;
    virtual int rate() const = 0;
    virtual int read(int16_t* out, int frames) = 0;
    virtual void rewind() = 0;
};

struct TuneSource : Source {
    int16_t* pcm = nullptr;
    size_t frames = 0, pos = 0;
    ~TuneSource() override { free(pcm); }
    int rate() const override { return kRate; }
    int read(int16_t* out, int n) override {
        for (int i = 0; i < n; ++i) {
            out[2 * i] = out[2 * i + 1] = pcm[pos];
            if (++pos >= frames) pos = 0;
        }
        return n;
    }
    void rewind() override { pos = 0; }
};

struct OggSource : Source {
    std::vector<uint8_t> data;
    stb_vorbis* v = nullptr;
    int sample_rate = 0;
    ~OggSource() override { if (v) stb_vorbis_close(v); }
    int rate() const override { return sample_rate; }
    int read(int16_t* out, int n) override {
        int got = 0;
        while (got < n) {
            const int r = stb_vorbis_get_samples_short_interleaved(v, kChannels, out + got * kChannels, (n - got) * kChannels);
            if (r <= 0) break;
            got += r;
        }
        return got;
    }
    void rewind() override { stb_vorbis_seek_start(v); }
};

struct WavSource : Source {
    FILE* f = nullptr;
    long data_start = 0;
    uint32_t data_bytes = 0, done = 0;
    int ch = 0, sample_rate = 0;
    std::vector<int16_t> tmp;
    ~WavSource() override { if (f) fclose(f); }
    int rate() const override { return sample_rate; }
    int read(int16_t* out, int n) override {
        const uint32_t frame_bytes = (uint32_t)ch * 2;
        const uint32_t left = (data_bytes - done) / frame_bytes;
        n = (int)std::min<uint32_t>((uint32_t)n, left);
        if (n <= 0) return 0;
        tmp.resize((size_t)n * ch);
        const size_t r = fread(tmp.data(), frame_bytes, (size_t)n, f);
        done += (uint32_t)r * frame_bytes;
        for (size_t i = 0; i < r; ++i) {
            out[2 * i] = tmp[i * ch];
            out[2 * i + 1] = tmp[i * ch + (ch > 1 ? 1 : 0)];
        }
        return (int)r;
    }
    void rewind() override { fseek(f, data_start, SEEK_SET); done = 0; }
};

uint32_t rd32(const uint8_t* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
uint16_t rd16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }

std::unique_ptr<Source> open_wav(const std::string& path) {
    auto s = std::make_unique<WavSource>();
    s->f = fopen(path.c_str(), "rb");
    if (!s->f) return nullptr;
    uint8_t hdr[12];
    if (fread(hdr, 1, 12, s->f) != 12 || memcmp(hdr, "RIFF", 4) || memcmp(hdr + 8, "WAVE", 4)) return nullptr;
    bool fmt_ok = false;
    for (;;) {
        uint8_t ck[8];
        if (fread(ck, 1, 8, s->f) != 8) return nullptr;
        const uint32_t size = rd32(ck + 4);
        if (!memcmp(ck, "fmt ", 4)) {
            uint8_t fmt[16];
            if (size < 16 || fread(fmt, 1, 16, s->f) != 16) return nullptr;
            const uint16_t tag = rd16(fmt), bits = rd16(fmt + 14);
            s->ch = rd16(fmt + 2);
            s->sample_rate = (int)rd32(fmt + 4);
            // 16-bit PCM only (WAVE_FORMAT_PCM or EXTENSIBLE), mono or stereo.
            fmt_ok = (tag == 1 || tag == 0xFFFE) && bits == 16 && (s->ch == 1 || s->ch == 2);
            fseek(s->f, (long)(size - 16 + (size & 1)), SEEK_CUR);
        } else if (!memcmp(ck, "data", 4)) {
            if (!fmt_ok) return nullptr;
            s->data_start = ftell(s->f);
            s->data_bytes = size;
            return s;
        } else {
            fseek(s->f, (long)(size + (size & 1)), SEEK_CUR);
        }
    }
}

std::unique_ptr<Source> open_ogg(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return nullptr;
    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    auto s = std::make_unique<OggSource>();
    if (size <= 0 || (size_t)size > kMaxFileBytes) { fclose(f); return nullptr; }
    s->data.resize((size_t)size);
    const size_t r = fread(s->data.data(), 1, (size_t)size, f);
    fclose(f);
    if (r != (size_t)size) return nullptr;
    int err = 0;
    s->v = stb_vorbis_open_memory(s->data.data(), (int)size, &err, nullptr);
    if (!s->v) return nullptr;
    const stb_vorbis_info info = stb_vorbis_get_info(s->v);
    s->sample_rate = (int)info.sample_rate;
    if (s->sample_rate < 8000 || s->sample_rate > 96000) return nullptr;
    return s;
}

bool ends_with_ci(const std::string& s, const char* suf) {
    const size_t n = strlen(suf);
    if (s.size() < n) return false;
    for (size_t i = 0; i < n; ++i)
        if (tolower((unsigned char)s[s.size() - n + i]) != suf[i]) return false;
    return true;
}

// ---------------------------------------------------------------- state ---
std::vector<std::string> g_files;
bool g_scanned = false;
std::mt19937 g_rng((uint32_t)armGetSystemTick());

std::unique_ptr<Source> g_src;
bool g_loop = true;          // false = "All (random)": the next file when one ends
int g_src_rate = 0;
bool g_playing = false;
int g_key_music = -1;        // the settings g_src was opened for
std::string g_key_track;
int g_last_tune = -1;
std::string g_current_file;  // the custom file playing, if any

void* g_pool = nullptr;
size_t g_pool_size = 0;
int g_pool_id = -1;
bool g_voice_on = false;
uint32_t g_generation = 0;
AudioDriverWaveBuf g_wb[kNumBufs] = {};

float g_fade = 0.0f;         // 0 -> 1 over the first ~0.6 s
uint64_t g_duck_tick = 0;    // when the last beep started (0 = none yet)

void release_voice() {
    AudioDriver* drv = g_sound.driver();
    if (g_voice_on && drv && g_generation == g_sound.reinit_generation()) {
        audrvVoiceStop(drv, kVoiceId);
        audrvUpdate(drv);
        audrvVoiceDrop(drv, kVoiceId);
        audrvUpdate(drv);
    }
    g_voice_on = false;
    for (auto& wb : g_wb) wb = {};
}

bool ensure_pool() {
    AudioDriver* drv = g_sound.driver();
    if (!drv) return false;
    if (g_pool && g_generation == g_sound.reinit_generation()) return true;
    // After a driver rebuild (or the first time): the old registration is
    // gone with the old driver; keep the memory, register it again.
    g_voice_on = false;
    for (auto& wb : g_wb) wb = {};
    if (!g_pool) {
        g_pool_size = ((size_t)kNumBufs * kBufFrames * kChannels * sizeof(int16_t) + 0xFFF) & ~size_t(0xFFF);
        g_pool = memalign(0x1000, g_pool_size);
        if (!g_pool) return false;
        memset(g_pool, 0, g_pool_size);
    }
    g_generation = g_sound.reinit_generation();
    g_pool_id = audrvMemPoolAdd(drv, g_pool, g_pool_size);
    if (g_pool_id < 0) return false;
    audrvMemPoolAttach(drv, g_pool_id);
    return true;
}

bool ensure_voice(int rate) {
    AudioDriver* drv = g_sound.driver();
    if (!drv || !ensure_pool()) return false;
    if (g_voice_on && g_src_rate == rate) return true;
    release_voice();
    if (!audrvVoiceInit(drv, kVoiceId, kChannels, PcmFormat_Int16, (uint32_t)rate)) return false;
    audrvVoiceSetDestinationMix(drv, kVoiceId, AUDREN_FINAL_MIX_ID);
    // Stereo voice: channel 0 -> left, channel 1 -> right.
    audrvVoiceSetMixFactor(drv, kVoiceId, 0.0f, 0, 0);
    audrvVoiceSetMixFactor(drv, kVoiceId, 0.0f, 0, 1);
    audrvVoiceSetMixFactor(drv, kVoiceId, 0.0f, 1, 0);
    audrvVoiceSetMixFactor(drv, kVoiceId, 0.0f, 1, 1);
    audrvVoiceStart(drv, kVoiceId);
    g_voice_on = true;
    g_src_rate = rate;
    return true;
}

std::unique_ptr<Source> open_file(const std::string& name) {
    const std::string path = menu_music_folder() + "/" + name;
    if (ends_with_ci(name, ".ogg")) return open_ogg(path);
    if (ends_with_ci(name, ".wav")) return open_wav(path);
    return nullptr;
}

// A random file other than the current one when there's a choice.
std::unique_ptr<Source> open_random_file(const std::string& avoid) {
    if (g_files.empty()) return nullptr;
    for (int tries = 0; tries < 4; ++tries) {
        const std::string& pick = g_files[g_rng() % g_files.size()];
        if (pick == avoid && g_files.size() > 1) continue;
        if (auto s = open_file(pick)) { g_current_file = pick; return s; }
    }
    return nullptr;
}

std::unique_ptr<Source> open_tune(int idx) {
    auto s = std::make_unique<TuneSource>();
    s->pcm = render_song(kSongs[idx], s->frames);
    if (!s->pcm || !s->frames) return nullptr;
    g_last_tune = idx;
    return s;
}

// Opens what the settings ask for; nullptr = nothing to play.
std::unique_ptr<Source> open_for_settings() {
    g_loop = true;
    const int m = g_menu_music;
    if (m >= 1 && m <= kNumBuiltinTunes) return open_tune(m - 1);
    if (m == kMenuMusicRandom) {
        int idx = (int)(g_rng() % kNumBuiltinTunes);
        if (idx == g_last_tune) idx = (idx + 1) % kNumBuiltinTunes;
        return open_tune(idx);
    }
    if (m == kMenuMusicCustom) {
        if (!g_scanned) menu_music_rescan();
        if (!g_menu_music_track.empty()) return open_file(g_menu_music_track);
        g_loop = false;
        return open_random_file("");
    }
    return nullptr;
}

void stop() {
    release_voice();
    g_src.reset();
    g_playing = false;
}

float duck_gain() {
    static const float kLevels[4] = {1.0f, 0.6f, 0.35f, 0.12f};
    const float lvl = kLevels[g_menu_music_duck < 4 ? g_menu_music_duck : 2];
    if (!g_duck_tick || lvl >= 1.0f) return 1.0f;
    const float since = (float)armTicksToNs(armGetSystemTick() - g_duck_tick) / 1e9f;
    constexpr float kHold = 0.12f, kRelease = 0.25f;   // the longest beep is ~0.2 s
    if (since < kHold) return lvl;
    if (since > kHold + kRelease) return 1.0f;
    return lvl + (1.0f - lvl) * (since - kHold) / kRelease;
}

}  // namespace

std::string menu_music_folder() { return storage_path("yokoi_music"); }

const std::vector<std::string>& menu_music_files() {
    if (!g_scanned) menu_music_rescan();
    return g_files;
}

void menu_music_rescan() {
    g_scanned = true;
    g_files.clear();
    std::error_code ec;
    const std::filesystem::path dir(menu_music_folder());
    if (!std::filesystem::exists(dir, ec)) {
        std::filesystem::create_directories(dir, ec);
        return;
    }
    for (const auto& e : std::filesystem::directory_iterator(dir, ec)) {
        if (!e.is_regular_file(ec)) continue;
        const std::string name = e.path().filename().string();
        if (ends_with_ci(name, ".ogg") || ends_with_ci(name, ".wav")) g_files.push_back(name);
    }
    std::sort(g_files.begin(), g_files.end());
}

void menu_music_duck() {
    g_duck_tick = armGetSystemTick();
}

void menu_music_update(bool menus_up) {
    if (!menus_up || g_menu_music == 0 || !g_sound.driver()) {
        if (g_playing || g_src) stop();
        g_key_music = -1;
        return;
    }
    // (Re)start when it wasn't playing or the settings changed.
    if (!g_playing || g_key_music != g_menu_music || g_key_track != g_menu_music_track) {
        stop();
        g_key_music = g_menu_music;
        g_key_track = g_menu_music_track;
        g_src = open_for_settings();
        g_playing = true;   // even when nothing opened: don't retry every frame
        g_fade = 0.0f;
    }
    if (!g_src) return;
    if (!ensure_voice(g_src->rate())) return;
    AudioDriver* drv = g_sound.driver();

    // Keep every free buffer filled and queued.
    for (int i = 0; i < kNumBufs; ++i) {
        AudioDriverWaveBuf& wb = g_wb[i];
        if (wb.state != AudioDriverWaveBufState_Free && wb.state != AudioDriverWaveBufState_Done) continue;
        int16_t* buf = (int16_t*)((uint8_t*)g_pool + (size_t)i * kBufFrames * kChannels * sizeof(int16_t));
        int got = g_src->read(buf, kBufFrames);
        if (got < kBufFrames) {
            if (g_loop) {
                g_src->rewind();
                got += g_src->read(buf + got * kChannels, kBufFrames - got);
            } else {
                // "All (random)": next file. Restarts the voice if its
                // sample rate differs - simplest is to start over cleanly.
                auto next = open_random_file(g_current_file);
                if (!next) { stop(); g_playing = true; return; }
                const bool same_rate = next->rate() == g_src->rate();
                g_src = std::move(next);
                if (!same_rate) { release_voice(); return; }
                got += g_src->read(buf + got * kChannels, kBufFrames - got);
            }
        }
        if (got <= 0) continue;
        if (got < kBufFrames) memset(buf + got * kChannels, 0, (size_t)(kBufFrames - got) * kChannels * sizeof(int16_t));
        armDCacheFlush(buf, (size_t)kBufFrames * kChannels * sizeof(int16_t));
        // Caller-owned fields only (see lcdonald_sound.cpp's play_cached());
        // offsets are in frames, each buffer its own slice of the pool.
        wb.data_pcm16 = buf;
        wb.size = (uint32_t)(kBufFrames * kChannels * sizeof(int16_t));
        wb.start_sample_offset = 0;
        wb.end_sample_offset = kBufFrames;
        wb.is_looping = false;
        audrvVoiceAddWaveBuf(drv, kVoiceId, &wb);
    }
    audrvVoiceStart(drv, kVoiceId);

    g_fade = std::min(1.0f, g_fade + 1.0f / 36.0f);   // ~0.6 s at 60 fps
    const float vol = (float)std::min<uint8_t>(g_menu_music_volume, 100) / 100.0f * switch_sound_volume() * g_fade * duck_gain();
    audrvVoiceSetMixFactor(drv, kVoiceId, vol, 0, 0);
    audrvVoiceSetMixFactor(drv, kVoiceId, vol, 1, 1);
    audrvUpdate(drv);
}
