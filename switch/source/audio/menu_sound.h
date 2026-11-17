#pragma once

// Short piezo-style UI beeps for the game list / settings screens
// (Settings > Sound > Menu Sounds). Synthesized once on first use (square
// waves, no audio files) and played on two mono voices of g_sound's shared
// driver, at the current Volume (silent while Mute is on).
enum class MenuSound { Move, Change, Open, Back };
void menu_sound_play(MenuSound which);
