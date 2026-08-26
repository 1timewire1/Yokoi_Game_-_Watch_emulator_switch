#pragma once

#include <atomic>
#include <cstdint>

class Virtual_Input;

// Controller mask (Android physical controller) that mirrors the 3DS mapping.
// Java sends the full held-state bitmask.
enum ControllerBits : uint32_t {
    CTL_DPAD_UP = 1u << 0,
    CTL_DPAD_DOWN = 1u << 1,
    CTL_DPAD_LEFT = 1u << 2,
    CTL_DPAD_RIGHT = 1u << 3,
    CTL_A = 1u << 4,
    CTL_B = 1u << 5,
    CTL_X = 1u << 6,
    CTL_Y = 1u << 7,
    CTL_START = 1u << 8,
    CTL_SELECT = 1u << 9,
    CTL_L1 = 1u << 10,
};

void yokoi_input_apply_action_mask(
    Virtual_Input* input,
    std::atomic<bool>& left_action_down,
    std::atomic<bool>& right_action_down,
    int touch_action_mask);

void yokoi_input_apply_controller_mask(
    Virtual_Input* input,
    uint32_t ctl_mask,
    int touch_action_mask,
    const std::atomic<int>& gamea_pulse_frames,
    const std::atomic<int>& gameb_pulse_frames);
