// Public-domain Ogg Vorbis decoder (stb_vorbis.h) - vendored for McDonald's
// (LCDonald-ported) games' real .ogg sample effects, the only thing in this
// engine that needs an actual audio codec (every other game's sound is
// synthesized - square-wave tones from ROM melody data, or David & John's
// piezo-beep synthesis). Same header/implementation split as stb_image.h/
// stb_image.cpp: any TU that only needs to call stb_vorbis_decode_memory()
// #defines STB_VORBIS_HEADER_ONLY before including the header; this is the
// one TU compiling the actual implementation.
// Only stb_vorbis_decode_memory() is used (decoding pack-embedded byte
// blobs, see lcdonald_sound.cpp) - never a real file path, so the FILE*-
// based API is left out entirely.
#define STB_VORBIS_NO_STDIO
#include "stb_vorbis.h"
