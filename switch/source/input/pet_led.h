#pragma once

#include <cstdint>

// Settings > Controller Features > Pet Alert Light: when a virtual pet
// beeps on its own (no button pressed for a few seconds - pets also beep
// as key feedback), the HOME button light of the right Joy-Con / Pro
// Controller pulses until any button is pressed. Uses hidsys's
// notification LED; if that service isn't available it simply does nothing.
void pet_led_init();
// Once per frame with the physical buttons newly pressed (any press - in
// the menus too - turns the light off).
void pet_led_update(uint64_t raw_down);
// Gameplay frame of a pet: how much its buzzer was on (0..1).
void pet_led_pet_frame(float buzzer_activity);
// Light off (leaving the app).
void pet_led_shutdown();
