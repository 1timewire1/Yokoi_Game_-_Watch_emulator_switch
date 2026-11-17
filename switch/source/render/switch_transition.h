#pragma once

#include <cstdint>

// Transitions between games / makers (Settings > Game List > Transition):
// the game list's one-at-a-time view (All Games) and the Carousel maker
// menu. switch_menu_render() draws the whole menu frame into an off-screen
// buffer; when what it shows changes (a new game or maker), the previous
// frame and the new one are blended over the content area (between the top
// and bottom bars) with the chosen effect for ~0.3 s. Only with the screen
// unrotated - rotated, frames are simply shown as they come.
enum : uint8_t {
    kTrOff = 0, kTrSlide, kTrFade, kTrZoom, kTrFlip, kTrCube, kTrWipe, kTrBlinds, kTrBounce, kTrPixelate, kTrLcd,
    kTrRandom, kNumTransitions
};
const char* transition_name(uint8_t t);

// Starts drawing a frame into the off-screen buffer (false: not available,
// draw straight to the screen as usual).
bool transition_capture_begin(int viewport_w, int viewport_h);
// Ends it and puts the frame on screen. `key` names what the frame shows -
// a change starts a transition from the previous frame; `dir` (+1 / -1) is
// which way the player moved. y0..y1 is the content area (logical pixels);
// bg / hl are the theme's background and highlight colours.
void transition_capture_end(int viewport_w, int viewport_h, uint64_t key, int dir, float y0, float y1, const float bg[3],
                            const float hl[3]);
// Forget the previous frame (leaving the views that use transitions).
void transition_reset();
