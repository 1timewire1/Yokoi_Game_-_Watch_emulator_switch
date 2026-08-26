#include "yokoi_input_mapping.h"

// NOTE: virtual_input.h defines a non-inline function `get_input_config(...)` in the header.
// Multiple Android translation units include this header, which causes duplicate-symbol
// link failures. Keep upstream code untouched by renaming the symbol in this TU.
#define get_input_config get_input_config__android_input_mapping_tu
#include "virtual_i_o/virtual_input.h"
#undef get_input_config

void yokoi_input_apply_action_mask(
    Virtual_Input* input,
    std::atomic<bool>& left_action_down,
    std::atomic<bool>& right_action_down,
    int touch_action_mask) {
    const bool want_left = (touch_action_mask & 0x01) != 0;
    const bool want_right = (touch_action_mask & 0x02) != 0;

    if (input) {
        // Only apply touch-based ACTION presses to parts that are configured as an ACTION button.
        if (input->left_configuration == CONF_1_BUTTON_ACTION) {
            if (left_action_down.load() != want_left) {
                input->set_input(PART_LEFT, BUTTON_ACTION, want_left);
                left_action_down.store(want_left);
            }
        }
        if (input->right_configuration == CONF_1_BUTTON_ACTION) {
            if (right_action_down.load() != want_right) {
                input->set_input(PART_RIGHT, BUTTON_ACTION, want_right);
                right_action_down.store(want_right);
            }
        }
    } else {
        // No mapping available; clear any prior state.
        left_action_down.store(false);
        right_action_down.store(false);
    }
}

void yokoi_input_apply_controller_mask(
    Virtual_Input* input,
    uint32_t ctl_mask,
    int touch_action_mask,
    const std::atomic<int>& gamea_pulse_frames,
    const std::atomic<int>& gameb_pulse_frames) {
    if (!input) {
        return;
    }

    const bool d_up = (ctl_mask & CTL_DPAD_UP) != 0;
    const bool d_down = (ctl_mask & CTL_DPAD_DOWN) != 0;
    const bool d_left = (ctl_mask & CTL_DPAD_LEFT) != 0;
    const bool d_right = (ctl_mask & CTL_DPAD_RIGHT) != 0;

    const bool a = (ctl_mask & CTL_A) != 0;
    const bool b = (ctl_mask & CTL_B) != 0;
    const bool x = (ctl_mask & CTL_X) != 0;
    const bool y = (ctl_mask & CTL_Y) != 0;

    const bool start = (ctl_mask & CTL_START) != 0;
    const bool select = (ctl_mask & CTL_SELECT) != 0;
    const bool l1 = (ctl_mask & CTL_L1) != 0;

    const bool touch_left = (touch_action_mask & 0x01) != 0;
    const bool touch_right = (touch_action_mask & 0x02) != 0;

    // Match 3DS mapping from source/main.cpp::input_get()
    // Setup buttons.
    input->set_input(PART_SETUP, BUTTON_TIME, l1);
    input->set_input(PART_SETUP, BUTTON_GAMEA, start || (gamea_pulse_frames.load() > 0));
    input->set_input(PART_SETUP, BUTTON_GAMEB, select || (gameb_pulse_frames.load() > 0));

    // Left controls.
    if (input->left_configuration == CONF_1_BUTTON_ACTION) {
        bool check = false;
        if (input->right_configuration == CONF_1_BUTTON_ACTION) {
            check = d_up || d_down || d_left || y;
        } else {
            check = d_up || d_down || d_left || d_right;
        }
        check = check || touch_left;
        input->set_input(PART_LEFT, BUTTON_ACTION, check);
    } else {
        input->set_input(PART_LEFT, BUTTON_LEFT, d_left);
        input->set_input(PART_LEFT, BUTTON_RIGHT, d_right);
        input->set_input(PART_LEFT, BUTTON_UP, d_up);
        input->set_input(PART_LEFT, BUTTON_DOWN, d_down);
    }

    // Right controls.
    if (input->right_configuration == CONF_1_BUTTON_ACTION) {
        bool check = false;
        if (input->left_configuration == CONF_1_BUTTON_ACTION) {
            check = a || b || x || d_right;
        } else {
            check = a || b || x || y;
        }
        check = check || touch_right;
        input->set_input(PART_RIGHT, BUTTON_ACTION, check);
    } else {
        // Note: 3DS maps right-side directions to face buttons.
        input->set_input(PART_RIGHT, BUTTON_LEFT, y);
        input->set_input(PART_RIGHT, BUTTON_RIGHT, a);
        input->set_input(PART_RIGHT, BUTTON_UP, x);
        input->set_input(PART_RIGHT, BUTTON_DOWN, b);
    }
}
