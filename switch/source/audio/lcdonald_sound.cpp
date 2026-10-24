#include "lcdonald_sound.h"

#include <cstring>
#include <malloc.h>
#include <string>
#include <switch.h>
#include <unordered_map>
#include <vector>

#include "switch_sound.h"
#include "std/debug_log.h"
#include "std/gw_pack.h"
#include "LCDonald/LCDonald_fake_cpu.h"
#include "LCDonald/LCDonald_program.h"

// Header-only here (just calling stb_vorbis_decode_memory()) - the actual
// implementation is compiled once in third_party/stb_vorbis.cpp.
#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_STDIO
#include "../third_party/stb_vorbis.h"

namespace {

// Voice 0 is switch_sound.cpp's synthesized-tone voice - these are extra,
// independent voices on the same shared driver (see switch_sound.h's
// driver()/reinit_generation() doc comments for why a fully separate
// AudioDriver isn't an option on Switch). A single voice plays whatever is
// queued on it strictly one clip after another, never mixed - which made
// every sound wait behind whatever was already playing (e.g. a 2s
// game_start blocking every hit/move/score for its whole duration) and
// dropped the overflow once the queue filled. Real overlap needs one voice
// per concurrent clip, like the original's independent sound streams.
// switch_sound.cpp's renderer config reserves enough voices/channel
// resources for these (each stereo voice costs two).
constexpr int kFirstSampleVoiceId = 1;
constexpr int kNumSampleVoices = 8;
constexpr int kSampleChannels = 2;
// Every LCDonald .ogg asset checked during development was 44100Hz stereo -
// a file that doesn't match is skipped in decode_and_cache() rather than
// played at the wrong pitch/speed.
constexpr int kSampleRate = 44100;
// Comfortably covers one game's own local sounds plus the shared common/
// set at once - the longest single clip (common/game_win.ogg) is ~14s,
// ~2.5MB decoded, and lcdonald_sound_reset() (see its own doc comment)
// keeps this from ever needing to hold more than one game's worth.
constexpr size_t kPoolSize = 16 * 1024 * 1024;

struct CachedSample {
    // Non-const: AudioDriverWaveBuf::data_pcm16 (libnx) is s16*, not const
    // s16* - this memory is only ever read after decode_and_cache() writes
    // it once, audrv just isn't aware of that.
    int16_t* pcm = nullptr;
    uint32_t frame_count = 0;
};

void* g_pool_ptr = nullptr;
int g_pool_id = -1;
size_t g_pool_used = 0;
bool g_voice_ready = false;
uint32_t g_last_seen_generation = 0;

// One wavebuf per voice: a voice is "free" once its clip's wavebuf reaches
// Free/Done, so whether a voice is busy is just its wavebuf's state.
bool g_voice_ok[kNumSampleVoices] = {};
AudioDriverWaveBuf g_wavebufs[kNumSampleVoices] = {};
int g_next_voice = 0;

std::unordered_map<std::string, CachedSample> g_cache;

// Only ever called when the underlying AudioDriver itself was torn down
// and recreated out from under this voice (switch_sound.cpp's self-heal -
// see reinit_generation()'s doc comment), NOT on every McDonald's game
// switch. An earlier version called this from lcdonald_sound_reset() too -
// i.e. on every single game load, not just the rare driver-reinit case -
// which meant this voice got Stop()+Drop()+MemPool-Remove()'d and
// recreated from scratch dozens of times in an ordinary play session
// instead of a handful. That repeated churn was the actual cause of a real
// crash inside libnx's own audrvVoiceDrop() (a null-pointer walk of the
// voice's internal wavebuf list) - see the 2026-09-23 incident this fixes,
// and plausibly also of *non*-crashing sounds silently failing to play
// (churn in the voice's mix routing, not a cache problem) - reported in
// the same session. lcdonald_sound_reset() now just clears the decode
// cache and rewinds the pool's bump allocator instead, touching neither
// the voice nor the mempool at all.
//
// Once the driver itself is gone (audrvClose(), inside switch_sound.cpp's
// full_reinit()), whatever this voice/mempool were registered as on the
// *old* driver is already gone with it - calling audrvVoiceStop/Drop or
// audrvMemPoolDetach/Remove against the *new* driver g_sound.driver() now
// returns would operate on a same-numbered but unrelated (or not-yet-
// initialized) voice/pool, not a real teardown. Only this side's own
// bookkeeping and the memalign'd pool buffer (ours, not the driver's)
// still need cleaning up here.
void teardown_voice() {
    if (g_pool_ptr) {
        free(g_pool_ptr);
    }
    g_pool_ptr = nullptr;
    g_pool_id = -1;
    g_pool_used = 0;
    g_voice_ready = false;
    g_cache.clear();
    for (auto& wb : g_wavebufs) wb = {};
    for (auto& ok : g_voice_ok) ok = false;
    g_next_voice = 0;
}

// Lazily (re)creates the voice/mempool - on first use, and again whenever
// switch_sound.cpp's self-heal has torn down and recreated the shared
// driver out from under this voice (audrvClose() invalidates every voice/
// mempool on it, not just voice 0's - see reinit_generation()'s doc comment).
bool ensure_voice_ready() {
    AudioDriver* drv = g_sound.driver();
    if (!drv) {
        g_voice_ready = false;
        return false;
    }

    if (g_voice_ready && g_last_seen_generation == g_sound.reinit_generation()) {
        return true;
    }

    teardown_voice();
    g_last_seen_generation = g_sound.reinit_generation();

    g_pool_ptr = memalign(0x1000, kPoolSize);
    if (!g_pool_ptr) {
        return false;
    }
    memset(g_pool_ptr, 0, kPoolSize);

    g_pool_id = audrvMemPoolAdd(drv, g_pool_ptr, kPoolSize);
    audrvMemPoolAttach(drv, g_pool_id);

    g_voice_ready = false;
    for (int i = 0; i < kNumSampleVoices; i++) {
        const int id = kFirstSampleVoiceId + i;
        g_voice_ok[i] = audrvVoiceInit(drv, id, kSampleChannels, PcmFormat_Int16, kSampleRate);
        YOKOI_LOG("lcdonald_sound: voice %d init %s", id, g_voice_ok[i] ? "ok" : "FAILED");
        if (!g_voice_ok[i]) continue;
        audrvVoiceSetDestinationMix(drv, id, AUDREN_FINAL_MIX_ID);
        // Straight stereo passthrough: voice channel 0 (L) -> mix output 0
        // (L), voice channel 1 (R) -> mix output 1 (R). No cross-mixing
        // (0->1 / 1->0 stay at their default 0 factor).
        audrvVoiceSetMixFactor(drv, id, 1.0f, 0, 0);
        audrvVoiceSetMixFactor(drv, id, 1.0f, 1, 1);
        audrvVoiceStart(drv, id);
        g_voice_ready = true;  // usable as long as at least one voice came up
    }
    audrvUpdate(drv);
    return g_voice_ready;
}

// "../common/X.ogg" -> "sound_lcd_common_X.ogg" (shared across every game);
// a bare "X.ogg" -> "sound_lcd_<short_name>_X.ogg" (that game's own sound -
// namespaced since several games reuse the same bare filename, e.g.
// "hit.ogg", for a completely different clip). Matches how
// generate_lcdonald_pack.py names these files when bundling them into
// gfx2x_open_source/.
std::string resolve_pack_name(const std::string& short_name, const std::string& raw) {
    const std::string prefix = "../common/";
    if (raw.compare(0, prefix.size(), prefix) == 0) {
        return "sound_lcd_common_" + raw.substr(prefix.size());
    }
    return "sound_lcd_" + short_name + "_" + raw;
}

const CachedSample* decode_and_cache(const std::string& pack_name) {
    auto it = g_cache.find(pack_name);
    if (it != g_cache.end()) {
        return &it->second;
    }

    const uint8_t* bytes = nullptr;
    size_t size = 0;
    if (!gw_pack::get_file_bytes(pack_name, bytes, size) || !bytes || size == 0) {
        return nullptr;
    }

    int channels = 0, sample_rate = 0;
    short* decoded = nullptr;
    int frame_count = stb_vorbis_decode_memory(bytes, (int)size, &channels, &sample_rate, &decoded);
    if (frame_count <= 0 || !decoded) {
        if (decoded) free(decoded);
        YOKOI_LOG("lcdonald_sound: decode failed for %s", pack_name.c_str());
        return nullptr;
    }
    if (channels != kSampleChannels || sample_rate != kSampleRate) {
        YOKOI_LOG("lcdonald_sound: unexpected format for %s (%d ch, %d Hz)", pack_name.c_str(), channels, sample_rate);
        free(decoded);
        return nullptr;
    }

    const size_t bytes_needed = (size_t)frame_count * channels * sizeof(int16_t);
    if (g_pool_used + bytes_needed > kPoolSize) {
        YOKOI_LOG("lcdonald_sound: sample pool exhausted, dropping %s", pack_name.c_str());
        free(decoded);
        return nullptr;
    }

    // Each clip starts on a 0x100 boundary rather than wherever the previous
    // one happened to end - the DSP reads wavebuf data straight from the
    // mempool, and nothing here documents a looser alignment guarantee.
    g_pool_used = (g_pool_used + 0xFF) & ~size_t(0xFF);
    if (g_pool_used + bytes_needed > kPoolSize) {
        YOKOI_LOG("lcdonald_sound: sample pool exhausted, dropping %s", pack_name.c_str());
        free(decoded);
        return nullptr;
    }
    int16_t* dst = (int16_t*)((uint8_t*)g_pool_ptr + g_pool_used);
    memcpy(dst, decoded, bytes_needed);
    armDCacheFlush(dst, bytes_needed);
    g_pool_used += bytes_needed;
    free(decoded);

    CachedSample sample;
    sample.pcm = dst;
    sample.frame_count = (uint32_t)frame_count;
    auto inserted = g_cache.emplace(pack_name, sample);
    return &inserted.first->second;
}

// Failure-only logging (per-sound success lines were only needed while
// diagnosing missing sounds), bounded so a stuck condition can't flood the log.
int g_log_budget = 400;

bool play_cached(const CachedSample& sample, const char* pack_name) {
    AudioDriver* drv = g_sound.driver();
    if (!drv) return false;

    // First voice (scanning from wherever the last one left off, so load
    // spreads evenly) whose previous clip has finished - each voice plays
    // exactly one clip at a time, so overlapping sounds land on different
    // voices and mix, instead of queueing behind each other on one.
    for (int n = 0; n < kNumSampleVoices; n++) {
        const int idx = (g_next_voice + n) % kNumSampleVoices;
        AudioDriverWaveBuf& wb = g_wavebufs[idx];
        const AudioDriverWaveBufState st = wb.state;
        if (!g_voice_ok[idx] || (st != AudioDriverWaveBufState_Free && st != AudioDriverWaveBufState_Done)) {
            continue;
        }
        const int voice_id = kFirstSampleVoiceId + idx;

        // Only the caller-owned fields - NOT `wb = {}` first. AudioDriverWaveBuf::
        // next (driver.h) is an intrusive linked-list pointer audrv itself
        // writes into and walks internally to chain a voice's queued wavebufs;
        // state/sequence_id/context_* are likewise driver-managed. Blindly
        // zeroing the whole struct here (an earlier version did) stomps that
        // bookkeeping out from under the driver on every reused slot, which
        // eventually corrupts the list enough to crash audrvVoiceDrop() (a
        // null-pointer walk) during a later teardown - matches switch_sound.cpp's
        // tone-synth voice, which was already careful to only touch these fields.
        wb.data_pcm16 = sample.pcm;
        wb.size = (uint32_t)sample.frame_count * kSampleChannels * sizeof(int16_t);
        wb.start_sample_offset = 0;
        wb.end_sample_offset = (int32_t)sample.frame_count;
        wb.is_looping = false;

        const bool queued = audrvVoiceAddWaveBuf(drv, voice_id, &wb);
        audrvVoiceStart(drv, voice_id);
        audrvUpdate(drv);
        if (!queued && g_log_budget > 0) {
            g_log_budget--;
            YOKOI_LOG("lcdonald_sound: driver rejected %s on voice %d (state=%d)", pack_name, voice_id, (int)wb.state);
        }
        if (queued) {
            g_next_voice = (idx + 1) % kNumSampleVoices;
            return true;
        }
        // Rejected by the driver - try the next idle voice instead of
        // silently losing the sound.
    }

    if (g_log_budget > 0) {
        g_log_budget--;
        YOKOI_LOG("lcdonald_sound: no free voice for %s", pack_name);
    }
    // Every voice is mid-clip (8 overlapping sounds) - dropped rather than
    // cutting off something still playing.
    return false;
}

}  // namespace

void lcdonald_sound_play_queued(LCDonald_fake_cpu* cpu) {
    if (!cpu || !cpu->program()) return;

    std::vector<std::string> sounds = cpu->program()->take_queued_sounds();
    if (sounds.empty()) return;

    if (!ensure_voice_ready()) return;

    // Burger Bard games run a McDonald's game's program, so short_name() is
    // that base game's and its sound files are used as-is (the Burger assets'
    // sounds are byte-identical to the base game's - see
    // CONVERT_ROM/generate_lcdonald_pack.py's bundle_sounds()) - except the
    // few local clips that do differ, bundled as "sound_lcd_burger_<folder>_
    // <file>" and preferred when present.
    const std::string short_name = cpu->program()->short_name();
    const bool burger = cpu->program()->is_burger();
    const std::string burger_prefix = burger ? "sound_lcd_burger_" + cpu->program()->burger_folder() + "_" : std::string();
    for (const std::string& raw : sounds) {
        const std::string pack_name = resolve_pack_name(short_name, raw);
        const CachedSample* sample = nullptr;
        if (burger && raw.compare(0, 3, "../") != 0) {
            sample = decode_and_cache(burger_prefix + raw);
        }
        if (!sample) sample = decode_and_cache(pack_name);
        if (sample) {
            play_cached(*sample, pack_name.c_str());
        } else if (g_log_budget > 0) {
            g_log_budget--;
            YOKOI_LOG("lcdonald_sound: no sample for '%s' (resolved %s)", raw.c_str(), pack_name.c_str());
        }
    }
}

void lcdonald_sound_reset() {
    // Cheap: forgets cached decodes and rewinds the pool's bump allocator
    // so the next game's sounds decode fresh into the same still-alive
    // mempool/voice - never touches the voice or mempool objects
    // themselves (see teardown_voice()'s doc comment for why tearing those
    // down here was the actual bug). If a sound from the *previous* game
    // is still mid-playback at the exact moment this runs, its PCM memory
    // can get overwritten by the next thing decoded into the pool from
    // offset 0 - a possible brief glitch, not a crash, and unlikely in
    // practice (these clips are short and a game switch is a deliberate
    // user action) - accepted for now rather than adding real generation-
    // tracked eviction for a cosmetic edge case.
    g_cache.clear();
    g_pool_used = 0;
}
