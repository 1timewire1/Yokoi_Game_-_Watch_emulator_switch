#pragma once

class BrickCore;

// Feeds Switch input to a BrickEmuPy-ported core (source/BrickCores/). Every
// such game carries its own button list in its ROM container: each .brick
// button name with the Switch keys CONVERT_ROM/brick_import.py assigned it
// ("LEFT", "A", "PLUS+MINUS", "LEFT|Y" ... - BrickCore::bindings()). So
// there's no per-game code here: a binding is pressed while any of its "|"
// alternatives has all of its "+" keys held.
// A chord (two or more keys - used for reset-type buttons, so they can't be
// hit by accident) takes priority over single-key bindings sharing one of
// its keys, which stay released while the chord is held. Only changes are
// passed on to the core.
// A '*' in front of an alternative ("A|*X") makes it a turbo: held, it
// presses and releases the button 10 times a second (see kTurboFrames).
void apply_brick_input(BrickCore* core);

// The loaded game has a step sensor button ("btnStep" / "reflector").
bool brick_input_has_step();
// One step from shaking the controller (input/motion_steps.h): the step
// button is pressed for a few frames on the next apply_brick_input() calls.
void brick_input_motion_step();
// While linked (core/link_session.h, which doesn't call apply_brick_input()):
// whether a shake is pressing the step button this frame - counts the frame.
bool brick_input_take_motion_frame();

// The loaded game has IR-sensor buttons ("btnWave" / "btnSensor", the
// D-Tector) - see input/ir_sensor.h.
bool brick_input_has_ir();
// From the Joy-Con IR camera: a Wave (pressed for a few frames) and the
// Sensor held while a hand stays in front.
void brick_input_ir_wave();
void brick_input_ir_sensor(bool held);
