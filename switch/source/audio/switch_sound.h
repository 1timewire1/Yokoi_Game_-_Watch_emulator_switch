#pragma once

// Switch audio backend. Interface mirrors source/virtual_i_o/3ds_sound.h
// (Virtual_Sound built on ndsp) since libnx's audrv is push/wavebuf-based
// just like ndsp.
//
// Differences vs. the 3DS backend (why it sounded bad on Switch):
//  - The 3DS DSP runs at ~32.7 kHz, so the SM5xx 32768 Hz 1-bit buzzer
//    stream can be played almost untouched. audren only runs at 48 kHz, so
//    feeding it a raw 32768 Hz square wave either plays it at the wrong
//    speed (voice declared at 48 kHz) or goes through audren's very basic
//    resampler, which smears the hard edges into aliasing / harsh buzz.
//    Here the buzzer is converted to 48 kHz ourselves using exact area
//    (box) sampling of the 1-bit signal, then a gentle low-pass and a DC
//    blocker, so the voice plays native 48 kHz PCM with no resampling.
//  - Audio is generated into a staging buffer and only copied into a
//    wavebuf slot that audren is NOT reading, with a 3-frame pre-buffer
//    (re-primed after any underrun) so frame-time jitter doesn't produce
//    gaps/clicks.

#include <cstdint>
#include <switch.h>

class SM5XX;

constexpr uint8_t NB_BUFFER = 8;          // wavebuf slots (one frame of audio each)
constexpr uint8_t SEQUENCE_SECURITY_ADD = 64;

class Virtual_Sound {
    public:
        void configure_sound();
        void initialize(uint32_t v_freq, uint32_t v_divide_freq, float fps_screen);
        void update_sound(SM5XX* cpu);
        void play_sample();
        void Quit_Game();
        void Exit();

        // Grants lcdonald_sound.cpp access to the one shared AudioDriver for
        // its own sample voices + mempool (real decoded PCM samples, not a
        // procedurally generated tone - see that file's own doc comment).
        // Switch only supports one AudioRenderer session per process (see
        // configure_sound()), so a fully independent driver isn't an option;
        // nullptr until configure_sound() succeeds.
        AudioDriver* driver() { return driver_ok ? &drv : nullptr; }

        // Bumped every full_reinit() (see play_sample()'s self-heal doc
        // comment). full_reinit() tears down and recreates `drv` wholesale
        // via audrvClose(), which silently invalidates any voice/mempool
        // lcdonald_sound.cpp added to it - it compares this against what it
        // last saw to know its own state needs recreating too.
        uint32_t reinit_generation() const { return reinit_generation_; }

    private:
        void free_buffers();
        void full_reinit();
        void reset_dsp_state();
        void push_output_sample(float area);
        int count_in_flight() const;
        bool slot_is_free(uint8_t slot) const;
        bool submit_slot(const int16_t* src, uint32_t nsamples);

        AudioDriver drv{};
        bool audren_ok = false;
        bool driver_ok = false;
        bool voice_ok = false;

        void* mempool_ptr = nullptr;
        size_t mempool_size = 0;
        int mempool_id = -1;

        int16_t* buffer_sound[NB_BUFFER] = {};   // slots inside the audren mempool
        AudioDriverWaveBuf wavebuf[NB_BUFFER] = {};
        int16_t* staging = nullptr;               // CPU-side buffer filled by update_sound()
        int16_t* silence = nullptr;               // zeroed, used for pre-buffering

        uint8_t curr_buffer = 0;                  // next slot to submit into
        uint32_t curr_sequence = 0;               // samples currently in `staging`
        uint32_t length_max_sequence = 0;         // capacity of each slot / staging (samples)
        uint32_t samples_per_frame = 0;           // ~800 @ 48 kHz / 60 fps
        uint16_t curr_wait_before_update = 0;
        bool primed = false;

        bool curr_value = false;
        float fps_screen = 0.0f;
        uint32_t base_freq = 0;
        uint32_t divide_freq = 1;

        // 1-bit -> 48 kHz area resampler
        float tick_len = 0.0f;   // length of one sound tick, in output samples (~1.46)
        float out_frac = 0.0f;   // how much of the current output sample is filled
        float out_acc = 0.0f;    // integrated "pin high" area in the current output sample

        // one-pole low-pass + DC blocker
        float lp_state = 0.0f;
        float hp_prev_in = 0.0f;
        float hp_state = 0.0f;

        // Self-heal state (audren session sometimes dies after sleep/resume).
        int stuck_frames = 0;
        int retry_cooldown = 0;

        uint32_t reinit_generation_ = 0;
};

extern Virtual_Sound g_sound;

// Configures g_sound for the currently loaded game (frequency/divide_freq
// come from the CPU instance). Equivalent to yokoi_audio_reconfigure_from_cpu.
void switch_sound_reconfigure(SM5XX* cpu);
