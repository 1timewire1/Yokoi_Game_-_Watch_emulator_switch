#pragma once

#include <cstdint>
#include <functional>

// Touch control for the menus (game list, settings, pause menu and their
// screens), Settings > Menu > Touch Controls. Handheld only - docked the
// Switch has no touch screen and nothing here does anything.
//
// While a menu frame is drawn, what can be tapped is registered here in the
// menu's logical (rotated) layout coordinates - the same ones it is drawn
// with. Once per frame, before the menu's own input code, menu_touch_poll()
// reads the touch screen and turns what the finger did on the last frame
// drawn into the button presses that menu already understands:
//   - tap a button hint at the bottom (or an option like "A Load Save") ->
//     that button;
//   - tap a list item / card / tile -> selects it; tap the one already
//     selected -> uses it (A, or what the item asks for);
//   - swipe left / right -> Right / Left (next / previous);
//   - drag up / down -> Down / Up, one step per row's worth of movement.
// So no screen's logic changes: touch only adds presses.

// Start of a menu frame: forgets the previous frame's targets.
// rotation: the menu's ScreenRotation value (0 = landscape, 1 = 90, 2 = 270).
void menu_touch_begin_frame(int logical_w, int logical_h, int rotation);

// A tap inside the rect presses `button` (HidNpadButton_* bits).
void menu_touch_button(float x, float y, float w, float h, uint64_t button);

// A selectable item: a tap selects it (calls `select`), a tap on it while it
// is the selected one presses `confirm` instead.
void menu_touch_item(float x, float y, float w, float h, bool selected, std::function<void()> select, uint64_t confirm);

// Once per frame while a menu is up, before its input is handled: the
// presses the touch screen produced this frame (0 most frames).
uint64_t menu_touch_poll();

// While no menu is up (gameplay): a finger already on the screen when a menu
// opens is ignored until it lifts, so a tap on the game's on-screen keypad
// can't land on the menu.
void menu_touch_idle();
