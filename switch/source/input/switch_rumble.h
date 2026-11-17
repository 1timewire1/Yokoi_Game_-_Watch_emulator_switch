#pragma once

#include <cstdint>

// Controller vibration (HD rumble) - Settings > Vibration:
//  - Game Vibration: Off / With Sound (buzzes while the game's buzzer
//    sounds) / Strong Events (only long or low beeps - often a miss, a
//    crash, a lost life)
//  - Pet Alerts: virtual pets buzz with their beeps even when Game
//    Vibration is Off (handy with the volume down)
//  - Menu Vibration: a short tap per menu button press
//  - Vibration Strength: Soft / Medium / Strong
// Handheld Joy-Con, a Joy-Con pair or a Pro Controller on player 1.

void rumble_init();
// Once per main-loop iteration, before anything else: sends what the
// previous frame asked for (and stops it when nothing did).
void rumble_update();
// Gameplay frame: how much of the last frame the buzzer was on (0..1) and
// its pitch (Hz, 0 = unknown), and whether the game is a pet.
void rumble_game_frame(float activity, float pitch_hz, bool is_pet);
// A McDonald's sample started (no buzzer to measure there).
void rumble_sample_played();
// Menu button feedback.
void rumble_menu_tap();
// Stop now (leaving the app).
void rumble_stop();
