#pragma once

// Settings > Controller Features > Pedometer: Shake the Controller - in
// the games with a step sensor (Pocket Pikachu, D-Tector, D-Power
// Digivice), each shake of the controller is one step, like walking with
// the real thing. Reads the accelerometer of the right Joy-Con (or the
// left one alone, the handheld pair, or a Pro Controller); the sensors run
// only while such a game is being played. The X-button step (and its
// turbo) keeps working.

// Once per gameplay frame of a BrickEmuPy game, before apply_brick_input().
void motion_steps_update(bool game_has_step_button);
// Leaving the game / the app.
void motion_steps_stop();

// Settings > Controller Features > Tap Sensor: the Tamagotchis with a
// vibration ("tap the case") sensor - Tenshitchi, Umi/Mori de Hakken,
// Genjintchi, TamaOtch - feel a light knock or shake of the controller, or
// (handheld) a touch on the screen. Once per gameplay frame of an E0C6200
// game; motion_tap_active() then says whether the sensor is triggered.
void motion_tap_update(bool game_has_tap_sensor);
bool motion_tap_active();
