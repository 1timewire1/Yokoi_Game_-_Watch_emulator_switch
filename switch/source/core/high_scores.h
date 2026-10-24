#pragma once

#include <cstdint>
#include <string>

// Persistent per-game high scores, independent of save states - kept in
// yokoi_highscores.txt next to the settings file, one "REF.mode=score" line per
// record (e.g. "SHOCKEY.endless=42"). `mode` names a game mode ("endless" for
// the McDonald's / Burger Bard Endless mode; "a"/"b" for Game A/B once
// emulated games report scores). Loaded lazily on first use; written only when
// a record actually changes (high_score_flush()), and never under the hidden
// debug mode, which keeps everything else read-only too.

// Best score for ref+mode, or 0 if none recorded.
uint32_t high_score_get(const std::string& ref, const char* mode);

// Records `score` if it beats the stored best (in memory only); returns true
// when it did. Call as often as convenient - cheap.
bool high_score_note(const std::string& ref, const char* mode, uint32_t score);

// Writes the file if anything changed since the last flush.
void high_score_flush();
