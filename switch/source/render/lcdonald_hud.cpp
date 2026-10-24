#include "lcdonald_hud.h"

#include <algorithm>
#include <string>
#include <utility>

#include "LCDonald/LCDonald_fake_cpu.h"
#include "core/switch_settings.h"
#include "switch_ui.h"

namespace {

// Seven-segment digit geometry, as fractions of the digit height. Each
// segment is a hexagonal bar (pointed ends, like a real LCD/DSEG glyph)
// and the whole digit leans right by kShear - the same italic look as the
// original app's score font. Shapes prototyped on the host first
// (CONVERT_ROM/tmp/seg7_proto.py) before being ported here.
constexpr float kDigitWidth = 0.56f;
constexpr float kThickness = 0.115f;
constexpr float kGap = 0.022f;
constexpr float kShear = 0.15f;
constexpr float kAdvanceExtra = 1.3f;  // extra spacing between digits, in thicknesses

// Which of a..g (bit 0 = a ... bit 6 = g) light for each digit.
constexpr unsigned char kDigitSegments[10] = {
    0x3F,  // 0: abcdef
    0x06,  // 1: bc
    0x5B,  // 2: abdeg
    0x4F,  // 3: abcdg
    0x66,  // 4: bcfg
    0x6D,  // 5: acdfg
    0x7D,  // 6: acdefg
    0x07,  // 7: abc
    0x7F,  // 8: abcdefg
    0x6F,  // 9: abcdfg
};

struct Pt {
    float x, y;
};

// Draws one hexagonal segment: `pts` in the digit's un-sheared local space,
// sheared and translated into screen space here.
void draw_segment(const Pt (&pts)[6], float ox, float oy, float digit_h) {
    float xy[12];
    for (int i = 0; i < 6; i++) {
        xy[2 * i] = ox + pts[i].x + (digit_h - pts[i].y) * kShear;
        xy[2 * i + 1] = oy + pts[i].y;
    }
    switch_ui_draw_polygon(xy, 6, 1.0f, 1.0f, 1.0f, 1.0f);
}

void draw_hz(float y, float ax, float bx, float t, float ox, float oy, float h) {
    const Pt p[6] = {{ax, y}, {ax + t / 2, y - t / 2}, {bx - t / 2, y - t / 2},
                     {bx, y}, {bx - t / 2, y + t / 2}, {ax + t / 2, y + t / 2}};
    draw_segment(p, ox, oy, h);
}

void draw_vt(float x, float ya, float yb, float t, float ox, float oy, float h) {
    const Pt p[6] = {{x, ya}, {x + t / 2, ya + t / 2}, {x + t / 2, yb - t / 2},
                     {x, yb}, {x - t / 2, yb - t / 2}, {x - t / 2, ya + t / 2}};
    draw_segment(p, ox, oy, h);
}

void draw_digit(int digit, float ox, float oy, float h) {
    const float w = h * kDigitWidth;
    const float t = h * kThickness;
    const float gap = h * kGap;
    const float xl = t / 2, xr = w - t / 2;
    const float yt = t / 2, ym = h / 2, yb = h - t / 2;
    const unsigned char m = kDigitSegments[digit];
    if (m & 0x01) draw_hz(yt, xl + gap, xr - gap, t, ox, oy, h);       // a
    if (m & 0x02) draw_vt(xr, yt + gap, ym - gap, t, ox, oy, h);       // b
    if (m & 0x04) draw_vt(xr, ym + gap, yb - gap, t, ox, oy, h);       // c
    if (m & 0x08) draw_hz(yb, xl + gap, xr - gap, t, ox, oy, h);       // d
    if (m & 0x10) draw_vt(xl, ym + gap, yb - gap, t, ox, oy, h);       // e
    if (m & 0x20) draw_vt(xl, yt + gap, ym - gap, t, ox, oy, h);       // f
    if (m & 0x40) draw_hz(ym, xl + gap, xr - gap, t, ox, oy, h);       // g
}

// LCDonald's own counter is "{0:000}" - at least three digits, growing
// (rather than wrapping) past 999.
void draw_score(int logical_w, int score) {
    std::string digits = std::to_string(std::max(0, score));
    while (digits.size() < 3) digits.insert(digits.begin(), '0');

    // Shrink the digits when there are more than three so the readout still
    // fits the narrow side margin next to the LCD (Switch's 16:9 screen
    // leaves roughly 160px either side of these games' near-square LCDs).
    float h = 54.0f;
    if (digits.size() > 3) h = h * 3.0f / (float)digits.size();

    const float w = h * kDigitWidth;
    const float t = h * kThickness;
    const float advance = w + t * kAdvanceExtra;
    const float total_w = advance * (float)digits.size() - t * kAdvanceExtra + h * kShear;

    const float pad = 10.0f;
    const float margin_right = 20.0f, margin_top = 18.0f;
    const float x0 = (float)logical_w - margin_right - total_w;
    const float y0 = margin_top;

    // Faint dark plate so the digits stay legible if a wide LCD's art ever
    // reaches into the margin.
    switch_ui_draw_rect(x0 - pad, y0 - pad, total_w + 2 * pad, h + 2 * pad, 0.0f, 0.0f, 0.0f, 0.55f);
    for (size_t i = 0; i < digits.size(); i++) {
        draw_digit(digits[i] - '0', x0 + (float)i * advance, y0, h);
    }
}

// "+ Normal" / "- Endless" prompt, bottom right, right-aligned.
void draw_start_hint(int logical_w, int logical_h) {
    const float size = 22.0f;
    const float line_h = 32.0f;
    const float margin_right = 20.0f, margin_bottom = 20.0f;
    const float gap = 8.0f;

    const std::pair<SwitchButtonIcon, const char*> rows[2] = {
        {SwitchButtonIcon::Plus, "Normal"},
        {SwitchButtonIcon::Minus, "Endless"},
    };
    float y = (float)logical_h - margin_bottom - line_h * 2.0f;
    for (const auto& row : rows) {
        const float tw = switch_ui_text_width(row.second, size);
        const float iw = switch_ui_button_icon_width(size, row.first);
        const float x_text = (float)logical_w - margin_right - tw;
        const float x_icon = x_text - gap - iw;
        switch_ui_draw_button_icon(x_icon, y, size, 1.0f, 1.0f, 1.0f, 0.95f, row.first);
        switch_ui_draw_text(x_text, y, size, 1.0f, 1.0f, 1.0f, 0.95f, row.second);
        y += line_h;
    }
}

}  // namespace

void lcdonald_hud_render(int viewport_w, int viewport_h, LCDonald_fake_cpu* cpu) {
    if (!cpu || !cpu->program()) return;

    // Same rotation-aware logical space switch_menu_render() lays out in:
    // swap at 90/270 and let switch_ui's own rotation uniform do the rest.
    int logical_w = viewport_w, logical_h = viewport_h;
    if (g_screen_rotation != ScreenRotation::Deg0) {
        std::swap(logical_w, logical_h);
    }

    switch_ui_begin_frame(logical_w, logical_h);
    if (cpu->is_endless()) {
        draw_score(logical_w, cpu->endless_score());
    }
    if (cpu->is_stopped()) {
        draw_start_hint(logical_w, logical_h);
    }
}
