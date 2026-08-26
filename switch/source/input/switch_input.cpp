#include "switch_input.h"

#include <switch.h>

#include "yokoi_controller_state.h"
#include "yokoi_input_mapping.h"

namespace {
PadState s_pad;
uint64_t s_held = 0;
uint64_t s_down = 0;

// If `from` is set in `bits`, also sets `to` - used below to fold the stick
// pseudo-buttons libnx already synthesizes (HidNpadButton_Stick{L,R}*, from
// the stick position crossing libnx's own internal deadzone - no manual
// analog reading needed) into the real button bits everything else checks.
uint64_t merge_bit(uint64_t bits, uint64_t from, uint64_t to) {
    return (bits & from) ? (bits | to) : bits;
}
}

void switch_input_init() {
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&s_pad);
}

uint64_t switch_input_poll() {
    padUpdate(&s_pad);
    uint64_t held = padGetButtons(&s_pad);
    uint64_t down = padGetButtonsDown(&s_pad);

    // Left stick mirrors the D-Pad everywhere (menus and gameplay) - merged
    // directly into the raw D-Pad bits so every existing check (menu
    // navigation via switch_input_held()/this function's kDown return, and
    // the CTL_DPAD_* mask built below) picks it up for free, with no
    // separate stick-reading code path needed anywhere else.
    held = merge_bit(held, HidNpadButton_StickLUp, HidNpadButton_Up);
    held = merge_bit(held, HidNpadButton_StickLDown, HidNpadButton_Down);
    held = merge_bit(held, HidNpadButton_StickLLeft, HidNpadButton_Left);
    held = merge_bit(held, HidNpadButton_StickLRight, HidNpadButton_Right);
    down = merge_bit(down, HidNpadButton_StickLUp, HidNpadButton_Up);
    down = merge_bit(down, HidNpadButton_StickLDown, HidNpadButton_Down);
    down = merge_bit(down, HidNpadButton_StickLLeft, HidNpadButton_Left);
    down = merge_bit(down, HidNpadButton_StickLRight, HidNpadButton_Right);

    s_held = held;
    s_down = down;

    // Same role mapping as source/virtual_i_o/3ds_input.cpp:
    // L = Time, Plus = Game A, Minus = Game B, dpad/A-B-X-Y = play controls.
    uint32_t mask = 0;
    if (s_held & HidNpadButton_Up)    mask |= CTL_DPAD_UP;
    if (s_held & HidNpadButton_Down)  mask |= CTL_DPAD_DOWN;
    if (s_held & HidNpadButton_Left)  mask |= CTL_DPAD_LEFT;
    if (s_held & HidNpadButton_Right) mask |= CTL_DPAD_RIGHT;
    if (s_held & HidNpadButton_A)     mask |= CTL_A;
    if (s_held & HidNpadButton_B)     mask |= CTL_B;
    if (s_held & HidNpadButton_X)     mask |= CTL_X;
    if (s_held & HidNpadButton_Y)     mask |= CTL_Y;
    if (s_held & HidNpadButton_Plus)  mask |= CTL_START;
    if (s_held & HidNpadButton_Minus) mask |= CTL_SELECT;
    if (s_held & HidNpadButton_L)     mask |= CTL_L1;

    // Right stick mirrors A/B/X/Y, but gameplay-only: this mask is what
    // main.cpp's gameplay branch reads via yokoi_controller_get_mask() -
    // menu navigation never consults it (it reads switch_input_held()/this
    // function's raw kDown return instead), so folding these into the CTL_*
    // mask here rather than into s_held/s_down above is what keeps them out
    // of menus without needing a separate "are we in a menu" check at all.
    // X=up, A=right, B=down, Y=left - the physical diamond layout of the
    // buttons being mirrored, not reading order.
    if (held & HidNpadButton_StickRUp)    mask |= CTL_X;
    if (held & HidNpadButton_StickRRight) mask |= CTL_A;
    if (held & HidNpadButton_StickRDown)  mask |= CTL_B;
    if (held & HidNpadButton_StickRLeft)  mask |= CTL_Y;

    yokoi_controller_set_mask(mask);
    return s_down;
}

uint64_t switch_input_held() {
    return s_held;
}
