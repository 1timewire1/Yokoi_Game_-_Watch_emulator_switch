#include "menu_touch.h"

#include <cmath>
#include <utility>
#include <vector>

#include <switch.h>

namespace {

struct Target {
    float x, y, w, h;
    uint64_t button;  // a plain button target; 0 for an item
    bool selected;
    std::function<void()> select;
    uint64_t confirm;
};

// Targets of the frame being drawn - after it's drawn, of the frame on
// screen, which is what the next poll hit-tests against.
std::vector<Target> g_targets;
int g_lw = 1280, g_lh = 720, g_rot = 0;

// The touch screen is 1280x720 in the console's own (landscape) orientation.
constexpr float kPanelW = 1280.0f, kPanelH = 720.0f;
constexpr float kTapSlop = 22.0f;    // moved less than this: a tap
constexpr float kSwipeMin = 90.0f;   // a horizontal swipe at least this long
constexpr float kDragStep = 58.0f;   // vertical drag: one Up/Down per this much

bool g_down = false;          // a finger is on the screen
bool g_ignore = false;        // ...but it was already there: ignore it until it lifts
bool g_was_idle = true;       // no poll last frame (gameplay)
bool g_moved = false, g_vertical = false;
float g_x0 = 0, g_y0 = 0, g_x = 0, g_y = 0, g_drag = 0;

// Physical touch point -> menu logical coordinates. The inverse of what
// switch_ui_begin_frame() does to the menu's vertices for its rotation:
// logical -> NDC p, then rotated = (p.x cos - p.y sin, p.x sin + p.y cos)
// with (cos, sin) = (1, 0) / (0, -1) at 90 / (0, 1) at 270.
void to_logical(float tx, float ty, float& lx, float& ly) {
    const float qx = tx / kPanelW * 2.0f - 1.0f, qy = 1.0f - ty / kPanelH * 2.0f;
    float px = qx, py = qy;
    if (g_rot == 1) {        // 90: q = (p.y, -p.x)
        px = -qy; py = qx;
    } else if (g_rot == 2) { // 270: q = (-p.y, p.x)
        px = qy; py = -qx;
    }
    lx = (px + 1.0f) * 0.5f * (float)g_lw;
    ly = (1.0f - py) * 0.5f * (float)g_lh;
}

uint64_t tap_at(float x, float y) {
    // The last target registered is drawn on top: search backwards.
    for (int i = (int)g_targets.size() - 1; i >= 0; --i) {
        const Target& t = g_targets[i];
        if (x < t.x || y < t.y || x >= t.x + t.w || y >= t.y + t.h) continue;
        if (t.button) return t.button;
        if (t.selected) return t.confirm;
        std::function<void()> select = t.select;  // it may redraw nothing, but don't hold a reference
        if (select) select();
        return 0;
    }
    return 0;
}

}  // namespace

void menu_touch_begin_frame(int logical_w, int logical_h, int rotation) {
    g_targets.clear();
    g_lw = logical_w > 0 ? logical_w : 1280;
    g_lh = logical_h > 0 ? logical_h : 720;
    g_rot = rotation;
}

void menu_touch_button(float x, float y, float w, float h, uint64_t button) {
    if (!button || w <= 0 || h <= 0) return;
    g_targets.push_back({x, y, w, h, button, false, nullptr, 0});
}

void menu_touch_item(float x, float y, float w, float h, bool selected, std::function<void()> select, uint64_t confirm) {
    if (w <= 0 || h <= 0) return;
    g_targets.push_back({x, y, w, h, 0, selected, std::move(select), confirm});
}

void menu_touch_idle() {
    g_was_idle = true;
}

uint64_t menu_touch_poll() {
    HidTouchScreenState ts = {};
    const bool touching = hidGetTouchScreenStates(&ts, 1) && ts.count > 0;
    if (g_was_idle) {
        // First menu frame after gameplay (or the first one at all): a
        // finger already down belongs to whatever was on screen before.
        g_was_idle = false;
        g_down = touching;
        g_ignore = touching;
        return 0;
    }
    uint64_t out = 0;
    if (touching) {
        float lx, ly;
        to_logical((float)ts.touches[0].x, (float)ts.touches[0].y, lx, ly);
        if (!g_down) {
            g_down = true;
            g_ignore = false;
            g_moved = g_vertical = false;
            g_x0 = g_x = lx;
            g_y0 = g_y = ly;
            g_drag = 0.0f;
            return 0;
        }
        if (g_ignore) return 0;
        const float dx = lx - g_x0, dy = ly - g_y0;
        if (std::fabs(dx) > kTapSlop || std::fabs(dy) > kTapSlop) g_moved = true;
        // A drag that's mostly up/down scrolls while the finger moves: the
        // content follows the finger, so moving up shows what's below.
        if (g_moved && (g_vertical || std::fabs(dy) > std::fabs(dx))) {
            g_vertical = true;
            g_drag += ly - g_y;
            if (g_drag <= -kDragStep) { out |= HidNpadButton_Down; g_drag += kDragStep; }
            else if (g_drag >= kDragStep) { out |= HidNpadButton_Up; g_drag -= kDragStep; }
        }
        g_x = lx;
        g_y = ly;
        return out;
    }
    if (!g_down) return 0;
    g_down = false;
    if (g_ignore) {
        g_ignore = false;
        return 0;
    }
    if (!g_moved) return tap_at(g_x0, g_y0);
    const float dx = g_x - g_x0, dy = g_y - g_y0;
    if (!g_vertical && std::fabs(dx) >= kSwipeMin && std::fabs(dx) > std::fabs(dy) * 1.5f) {
        // Swiping left brings in what's on the right: the next one.
        out |= dx < 0 ? HidNpadButton_Right : HidNpadButton_Left;
    }
    return out;
}
