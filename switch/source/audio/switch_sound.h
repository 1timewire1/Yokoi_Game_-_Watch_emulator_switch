#pragma once

// Switch audio backend. Interface mirrors source/virtual_i_o/3ds_sound.h
// (Virtual_Sound built on ndsp) since libnx's audrv is push/wavebuf-based
// just like ndsp, rather than the pull/callback model Android's AAudio
// backend (yokoi_audio.cpp) uses.

#include <cstdint>
#include <switch.h>

class SM5XX;

constexpr uint8_t NB_BUFFER = 5;
constexpr uint8_t SEQUENCE_SECURITY_ADD = 4;

class Virtual_Sound {
    public:
        void configure_sound();
        void initialize(uint32_t v_freq, uint32_t v_divide_freq, float fps_screen);
        void update_sound(SM5XX* cpu);
        void play_sample();
        void Quit_Game();
        void Exit();

    private:
        void free_buffers();
        void full_reinit();

        AudioDriver drv{};
        bool audren_ok = false;
        bool driver_ok = false;
        bool voice_ok = false;

        void* mempool_ptr = nullptr;
        size_t mempool_size = 0;
        int mempool_id = -1;

        int16_t* buffer_sound[NB_BUFFER] = {};
        AudioDriverWaveBuf wavebuf[NB_BUFFER] = {};
        size_t curr_size_buffer[NB_BUFFER] = {};

        uint8_t curr_buffer = 0;
        uint32_t curr_sequence = 0;
        uint32_t length_max_sequence = 0;
        uint16_t curr_wait_before_update = 0;

        bool curr_value = false;
        uint32_t accu_freq_sequence = 0;
        float fps_screen = 0.0f;
        uint32_t base_freq = 0;
        uint32_t divide_freq = 1;

        // Self-heal state - see play_sample()'s doc comment for why this
        // exists (ported from D.Smile NX's switch_audio.cpp, which hit and
        // fixed the exact same "audio silently dies until relaunch" issue).
        int stuck_frames = 0;
        int retry_cooldown = 0;
};

extern Virtual_Sound g_sound;

// Configures g_sound for the currently loaded game (frequency/divide_freq
// come from the CPU instance). Equivalent to yokoi_audio_reconfigure_from_cpu.
void switch_sound_reconfigure(SM5XX* cpu);
