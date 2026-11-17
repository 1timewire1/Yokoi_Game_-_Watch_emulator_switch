#pragma once

// Settings > Controller Features > IR Sensor: the right Joy-Con's IR
// camera stands in for the D-Tector's sensors on top. A hand passing in
// front of the camera (the tip at the bottom of the Joy-Con) is a "Wave";
// a hand held over it keeps "Sensor" pressed. Uses the camera's moment
// processor (the brightness its own IR light bounces back); the camera
// only runs while such a game is being played - and in Auto mode only for
// a minute after a button press or a hand, to save battery. L / R keep
// working.
// Only the right Joy-Con has the camera (not the Pro Controller or Lite).

// Once per gameplay frame of a BrickEmuPy game, before apply_brick_input().
// `game_listening`: the game is driving its sensor line right now (true
// when that can't be told). In Auto the camera only runs while it does -
// in the D-Tector's modes that use the sensor, not the others.
void ir_sensor_update(bool game_has_ir_buttons, bool game_listening);
// Leaving the game / the app.
void ir_sensor_stop();
