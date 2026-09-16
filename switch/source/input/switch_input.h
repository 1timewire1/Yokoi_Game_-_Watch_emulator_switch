#pragma once

#include <cstdint>

// Sets up the default pad (handheld + first connected controller), plus a
// second pad for Player 2 - see switch_input_poll_p2()'s doc comment.
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

// Player 2 - fully automatic, no setting (a second controller being
// connected *is* two-player mode, checked live every frame - see
// standard_controls.md's BX_301/AK_302/HK_303 row). Only the three "Vs."
// games actually have a genuine second player's worth of input in the
// emulated hardware itself (see BX_301's set_input() in virtual_input.h,
// the only class in the codebase that branches on the `player` parameter
// at all) - main.cpp only ever calls this while one of those three is
// loaded. Read from a second, separately-initialized PadState
// (HidNpadIdType_No2, no Handheld merge - handheld mode is inherently
// single-controller), returned as a CTL_* mask so it can go straight into
// yokoi_input_apply_controller_mask(..., player=2) exactly like Player 1's
// own mask. Safe to call every frame regardless of whether anything's
// actually connected to that slot: libnx just reports no buttons held, so
// Player 2's input naturally stays all-released - same as real Vs.
// hardware with no second controller attached, no separate connected/
// disconnected branch needed at the call site.
//
// Confirmed working on hardware with all three Vs. games in Game Mode B and
// two controllers connected: the second controller drives only Player 2's
// character, Player 1's stays independent - see switch/README.md's
// "Two-player controller support" section.
uint32_t switch_input_poll_p2();

// Whether a second controller is currently connected. Doesn't call
// padUpdate() itself - call switch_input_poll_p2() first each frame (the
// only place that updates the underlying PadState), same ordering
// requirement D.Smile NX's own switch_input_player2_ready() documents.
bool switch_input_player2_ready();
