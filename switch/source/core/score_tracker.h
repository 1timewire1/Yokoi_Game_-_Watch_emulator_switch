#pragma once

#include <cstdint>
#include <string>
#include <vector>

// High scores for the emulated games that show a score (Settings > Game
// List > High Scores). The score is read off the LCD itself: the Game &
// Watch games show it on the same four digits as their clock, so a
// calibration run (setting the clock to known times and seeing which
// segments light) told which segment is which part of each digit - the
// per-game data in score_layouts.inc. While the clock's AM/PM mark is off
// the digits are the score; a value that holds for a few frames is offered
// to high_scores.h as the record of the mode the player started (Game A /
// Game B). Games without data simply have no records (pets, watches, games
// whose digits also show a timer).
//
// The McDonald's / Burger Bard games keep their own Endless record
// (main.cpp + render/lcdonald_hud.h); they're listed here too, read-only.

// Whether `ref` has records here (its layout, or the McDonald's Endless one).
bool score_tracker_has_records(const std::string& ref);
// Its records for the menus: label ("Game A", "Game B", "Endless") + best (0 = none yet).
struct ScoreRecord {
    const char* label;
    uint32_t best;
};
std::vector<ScoreRecord> score_tracker_records(const std::string& ref);
// Deletes this game's records (pause menu > High Scores).
void score_tracker_clear(const std::string& ref);

// A game was loaded (switch_game_loader.cpp): picks its layout.
void score_tracker_game_loaded(const std::string& ref);
// The player pressed Game A (0) / Game B (1) - the mode the score counts for.
void score_tracker_set_mode(int mode);
// Once a frame while the game runs (main.cpp), after its steps.
void score_tracker_update();
// McDonald's / Burger Bard games (main.cpp), Normal and Endless mode each
// with its own record: a run starts / its running score / the "HI" of a mode
// its HUD shows (0 = none), all following Settings > Game List > High Scores
// and Record On Screen.
void score_tracker_lcd_start(const std::string& ref, bool endless);
void score_tracker_note_lcd(const std::string& ref, bool endless, uint32_t score);
uint32_t score_tracker_lcd_hi(const std::string& ref, bool endless, bool on_start_screen);

// Achievements, per game, for every game with records: reaching 25, 50,
// 100, 200 and 500 points (x10 on the VFD tabletop games, which score in
// tens) and beating your own record. Kept with the records ("REF.ach", a
// bit mask) and deleted with them. Their banner follows Settings > Game
// List > Achievement Alerts (g_ach_notify).
constexpr int kAchievementCount = 6;
// Bit i set = achievement i got (0..4 the points, 5 the record).
uint8_t score_tracker_achievements(const std::string& ref);
// Got since the game was loaded.
uint8_t score_tracker_session_achievements();
// "25 points", "Beat your record" (translated).
std::string score_tracker_achievement_name(const std::string& ref, int i);
// A McDonald's / Burger Bard run ended (main.cpp, while it's stopped):
// "When the Game Ends" shows its achievements now.
void score_tracker_run_ended();

// "High score" box / "New record!" banner over the game (Settings > Game
// List > Record On Screen).
void score_tracker_render(int viewport_w, int viewport_h);
