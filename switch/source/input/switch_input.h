#pragma once

#include <cstdint>

// Sets up the default pad (handheld + first connected controller).
void switch_input_init();

// Polls the pad once per frame, translates it into the CTL_* mask
// (yokoi_input_mapping.h) and publishes it via yokoi_controller_set_mask().
// Returns the raw libnx "buttons newly pressed this frame" mask (HidNpadButton_*)
// for callers that need edge-detection outside the CTL_* mapping (e.g. main.cpp's
// quit combo).
uint64_t switch_input_poll();

// Raw libnx "buttons currently held" mask (HidNpadButton_*), valid after the
// most recent switch_input_poll() call.
uint64_t switch_input_held();
