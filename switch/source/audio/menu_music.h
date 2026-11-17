#pragma once

#include <string>
#include <vector>

// Background music for the game list / settings (Settings > Menu > Music).
// Eight built-in tunes (synthesized here, no audio files) or the user's own
// .ogg / .wav files from sdmc:/switch/yokoi/yokoi_music/. Plays only while
// the menus are up with no game loaded (never in-game or in the pause
// menu), on its own stereo voice of g_sound's shared driver, at Music
// Volume x Settings > Sound > Volume (silent while Mute is on). It dips
// while a menu beep plays (Lower With Beeps) so the two never fight.

// Settings > Menu > Music > Background Music values.
constexpr int kNumBuiltinTunes = 8;
constexpr int kMenuMusicRandom = kNumBuiltinTunes + 1;   // 9
constexpr int kMenuMusicCustom = kNumBuiltinTunes + 2;   // 10
constexpr int kNumMenuMusicChoices = kNumBuiltinTunes + 3;  // Off + 8 + Random + Custom

// Once per frame: `menus_up` = the game list / settings are on screen with
// no game loaded. Starts, keeps fed, restarts (setting changed) or stops
// the music.
void menu_music_update(bool menus_up);

// A menu beep just started (audio/menu_sound.cpp): the music dips for it.
void menu_music_duck();

// The .ogg/.wav files found in the music folder (names only, sorted).
// Re-read from the SD on every menu_music_rescan() (opening the Music
// submenu), so files copied there show up without restarting.
const std::vector<std::string>& menu_music_files();
void menu_music_rescan();
// Full path of the music folder (created on first rescan if missing).
std::string menu_music_folder();
