#pragma once

#include <cstdint>

class SM5XX;

void yokoi_audio_set_can_run(bool can_run);

// Reconfigure source rate and reset buffers based on the current CPU.
void yokoi_audio_reconfigure_from_cpu(SM5XX* cpu);

// Push one audio step based on CPU state. Intended to be called regularly from the emulation loop.
void yokoi_audio_update_step(SM5XX* cpu);

// Returns the current source sample rate (best effort).
int yokoi_audio_get_source_rate();

// Reads up to `frames` mono int16 samples into `out`.
// Returns number of frames written.
int yokoi_audio_read(int16_t* out, int frames);

// AAudio control. Safe to call multiple times.
void yokoi_aaudio_start_stream();
void yokoi_aaudio_stop_stream();
