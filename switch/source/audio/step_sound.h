#pragma once

// The pedometer sound of the virtual pets with a step sensor, played once per
// step - every press of their step button ("btnStep" / "reflector"),
// including each press of its turbo (X held = about 4 steps a second, see
// switch/source/input/brick_input_map.cpp) - on its own audrv voice, mixed
// over the game's sound. Each device has its own, recorded from a real one
// (step_sound_data.h).
enum class StepSound { PocketPikachu, DTector, DPower };
void step_sound_play(StepSound which);
