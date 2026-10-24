#pragma once

class LCDonald_fake_cpu;

// Plays a McDonald's (LCDonald-ported) game's queued .ogg sound effects -
// the one place in this engine that needs a real audio codec (stb_vorbis,
// see third_party/stb_vorbis.h) rather than synthesizing a tone, since these
// are hand-simulated games with real sample-based sound design in the
// original, not something emulated from ROM melody data. Drains
// cpu->program()->take_queued_sounds() and plays each one on its own audrv
// voice (see switch_sound.h's driver()/reinit_generation()), decoding once
// per distinct file and caching the PCM for reuse (many of these get played
// many times over a single game - "hit.ogg" especially). Call once per real
// frame alongside apply_lcdonald_input() (see main.cpp) - anything less
// frequent would let take_queued_sounds() silently drop sounds queued
// between calls, since it drains rather than accumulates.
void lcdonald_sound_play_queued(LCDonald_fake_cpu* cpu);

// Clears the decoded-sample cache - call whenever a different McDonald's
// game is selected (see switch_game_loader.cpp's game-id dispatch), so at
// most one game's own local sounds plus the shared common/ set are ever
// cached at once rather than accumulating across every game played this
// session.
void lcdonald_sound_reset();
