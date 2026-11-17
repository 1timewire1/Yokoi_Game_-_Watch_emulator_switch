#include "onscreen_keypad.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <string>
#include <switch.h>

#include "SM5XX/SM5XX.h"
#include "control_remap.h"
#include "switch_input.h"
#include "core/switch_runtime_state.h"
#include "core/switch_settings.h"
#include "render/switch_ui.h"
#include "render/switch_i18n.h"
#include "std/GW_ROM.h"
#include "BrickCores/BrickCore.h"
#include "core/game_controls.h"
#include <vector>

namespace {

enum Style : uint8_t { Num, Op, Fn, Red, Gap };

struct Key {
    const char* label;
    Style style;
    int8_t group, line;  // cpu->input_set() target
};

// Number Muncher (main.cpp's kNummunchLayout targets).
const Key kNummunch[] = {
    {"GAME", Fn, 2, 0},  {"QUIZ", Fn, 2, 2},  {"COUNT", Fn, 2, 1}, {"OFF", Red, 1, 0},
    {"7", Num, 6, 2},    {"8", Num, 6, 1},    {"9", Num, 6, 0},    {"\xC3\xB7", Op, 7, 3},
    {"4", Num, 5, 1},    {"5", Num, 5, 0},    {"6", Num, 6, 3},    {"\xC3\x97", Op, 7, 2},
    {"1", Num, 4, 0},    {"2", Num, 5, 3},    {"3", Num, 5, 2},    {"-", Op, 7, 1},
    {"0", Num, 4, 1},    {"CALC", Fn, 1, 1},  {"=", Op, 1, 2},     {"+", Op, 7, 0},
    {"<", Fn, 4, 2},     {">", Fn, 4, 3}, {"CH +", Fn, 3, 0},  {"CH -", Fn, 3, 1},
    {"", Gap, -1, -1},   {"", Gap, -1, -1},   {"CH \xC3\x97", Fn, 3, 2}, {"CH \xC3\xB7", Fn, 3, 3},
};

// Uchitel' arifmetiki (kUchitariLayout). Minute shares 0's line, Hour 1's
// and Exam Level Alarm's on the real board, so they're one key each here.
const Key kUchitari[] = {
    {"MODE", Fn, 1, 1},  {"ALM", Fn, 1, 2},   {"E", Fn, 1, 0},     {"C", Red, 2, 1},
    {"7", Num, 4, 0},    {"8", Num, 5, 0},    {"9", Num, 6, 0},    {"\xC3\xB7", Op, 6, 1},
    {"4", Num, 6, 2},    {"5", Num, 2, 2},    {"6", Num, 3, 0},    {"\xC3\x97", Op, 5, 1},
    {"1", Num, 3, 2},    {"2", Num, 4, 2},    {"3", Num, 5, 2},    {"-", Op, 4, 1},
    {"0", Num, 2, 0},    {"EXAM", Fn, 0, 2},  {"ENTER", Op, 0, 0}, {"+", Op, 3, 1},
};

// Tronica calculator keypads (kShuttleVoyageCalcLayout; Space Adventure's
// C is on IN.4 bit 3 instead of bit 2).
const Key kShuttleVoyage[] = {
    {"C", Red, 4, 2},    {"%", Fn, 4, 1},     {"ALM", Fn, 4, 0},   {"\xC3\xB7", Op, 3, 3},
    {"7", Num, 1, 3},    {"8", Num, 2, 0},    {"9", Num, 2, 1},    {"\xC3\x97", Op, 3, 2},
    {"4", Num, 1, 0},    {"5", Num, 1, 1},    {"6", Num, 1, 2},    {"-", Op, 3, 1},
    {"1", Num, 0, 1},    {"2", Num, 0, 2},    {"3", Num, 0, 3},    {"+", Op, 3, 0},
    {"0", Num, 0, 0},    {".", Num, 2, 2},    {"=", Op, 2, 3},     {"", Gap, -1, -1},
};
const Key kSpaceAdventure[] = {
    {"C", Red, 4, 3},    {"%", Fn, 4, 1},     {"ALM", Fn, 4, 0},   {"\xC3\xB7", Op, 3, 3},
    {"7", Num, 1, 3},    {"8", Num, 2, 0},    {"9", Num, 2, 1},    {"\xC3\x97", Op, 3, 2},
    {"4", Num, 1, 0},    {"5", Num, 1, 1},    {"6", Num, 1, 2},    {"-", Op, 3, 1},
    {"1", Num, 0, 1},    {"2", Num, 0, 2},    {"3", Num, 0, 3},    {"+", Op, 3, 0},
    {"0", Num, 0, 0},    {".", Num, 2, 2},    {"=", Op, 2, 3},     {"", Gap, -1, -1},
};

// cols: keys per row. brick: a Microvision cartridge's keypad - its keys
// press the .brk bindings (mv_names) instead of input_set().
// r0..r1: the rows that have keys (a Microvision cartridge's keypad is drawn
// without its empty top/bottom rows, centred in the panel).
struct Layout { const Key* keys; int count; int cols = 4; bool brick = false; int r0 = 0, r1 = -1; };

// Milton Bradley Microvision (BrickCore "TMS1100"): the console's 3x4
// keypad, built from the cartridge's own bindings ("input.btn<NAME>=
// direct;BTN;<key 0-11>;..." in its .brk, named after its keypad overlay) -
// the keys it doesn't use are gaps.
std::string g_mv_ref;
Key g_mv_keys[12];
std::string g_mv_labels[12];
std::string g_mv_names[12];   // binding names, "" = no key
// The cartridge's key colour ("mv.keycolor" RRGGBB, from photos of it) and
// whether its keys have a white outline ("mv.keyborder", Super Blockbuster).
bool g_mv_has_color = false;
float g_mv_rgb[3] = {0.27f, 0.28f, 0.31f};
bool g_mv_border = false;

bool is_microvision() {
    return g_cpu && g_cpu->get_cpu_type_id() == 6 &&
           static_cast<const BrickCore*>(g_cpu.get())->rom_info().core == "TMS1100";
}

void build_microvision(const std::string& ref) {
    if (ref == g_mv_ref) return;
    g_mv_ref = ref;
    for (int i = 0; i < 12; i++) {
        g_mv_names[i].clear();
        g_mv_labels[i].clear();
        g_mv_keys[i] = {"", Gap, -1, -1};
    }
    const BrickCore* core = static_cast<const BrickCore*>(g_cpu.get());
    for (const auto& kv : core->rom_info().options.with_prefix("input.")) {
        // "direct;BTN;<n>;1;<keys>"
        const std::string& v = kv.second;
        if (v.compare(0, 11, "direct;BTN;") != 0) continue;
        const int n = atoi(v.c_str() + 11);
        if (n < 0 || n >= 12) continue;
        std::string label = kv.first;
        if (label.compare(0, 3, "btn") == 0) label = label.substr(3);
        for (char& c : label) if (c == '_') c = ' ';
        g_mv_names[n] = kv.first;
        g_mv_labels[n] = label;
        g_mv_keys[n] = {g_mv_labels[n].c_str(), Fn, -1, -1};
    }
    const BrickRom::Options& opt = core->rom_info().options;
    g_mv_has_color = opt.has("mv.keycolor");
    if (g_mv_has_color) {
        const unsigned long rgb = strtoul(opt.str("mv.keycolor").c_str(), nullptr, 16);
        g_mv_rgb[0] = ((rgb >> 16) & 0xFF) / 255.0f;
        g_mv_rgb[1] = ((rgb >> 8) & 0xFF) / 255.0f;
        g_mv_rgb[2] = (rgb & 0xFF) / 255.0f;
    }
    g_mv_border = opt.num("mv.keyborder", 0) != 0;
}

Layout layout_for_game() {
    if (!g_game) return {nullptr, 0};
    const std::string& ref = g_game->ref;
    if (is_microvision()) {
        build_microvision(ref);
        Layout lay{g_mv_keys, 12, 3, true};
        for (int n = 0; n < 12; n++) {
            if (g_mv_names[n].empty()) continue;
            if (lay.r1 < 0) lay.r0 = n / 3;
            lay.r1 = n / 3;
        }
        return lay;
    }
    if (ref == "NUMMUNCH") return {kNummunch, (int)(sizeof(kNummunch) / sizeof(Key))};
    if (ref == "UCHITARI") return {kUchitari, (int)(sizeof(kUchitari) / sizeof(Key))};
    // The Tronica keypads only exist with Experimental Features on anyway.
    if (ref == "SA_12") return {kSpaceAdventure, (int)(sizeof(kSpaceAdventure) / sizeof(Key))};
    if (ref == "MG_8" || ref == "TG_18") return {kShuttleVoyage, (int)(sizeof(kShuttleVoyage) / sizeof(Key))};
    return {nullptr, 0};
}

// The Switch button the player has on the row with this name (Controls'
// current assignment), "" if none.
std::string assigned_button(const std::string& label) {
    if (!g_game) return std::string();
    for (const ControlRow& r : game_control_rows()) {
        if (r.name != label || r.keys.empty()) continue;
        const int p = control_remap_source(g_game->ref, r.keys[0]);
        return p == kNoButton ? std::string("-") : std::string(remap_button_label(p));
    }
    return std::string();
}

// Screen layout (1280 x 720, the fixed viewport): game on the left, panel
// on the right.
constexpr float kGameFrac = 0.60f;
constexpr float kPanelX = 1280.0f * kGameFrac + 16.0f, kPanelY = 24.0f;
constexpr float kPanelW = 1280.0f - kPanelX - 20.0f, kPanelH = 720.0f - 48.0f;
constexpr float kHeaderH = 54.0f, kFooterH = 58.0f, kPad = 22.0f, kGap = 12.0f;

struct Rect { float x, y, w, h; };

Rect key_rect(const Layout& lay, int i) {
    const int kCols = lay.cols;
    const int rows = (lay.count + kCols - 1) / kCols;
    const float kw = (kPanelW - 2 * kPad - kGap * (kCols - 1)) / kCols;
    const float avail_h = kPanelH - kHeaderH - kFooterH;
    const float kh = std::min(kCols == 3 ? 96.0f : 70.0f, (avail_h - kGap * (rows - 1)) / rows);
    int r = i / kCols;
    const int c = i % kCols;
    float y0 = kPanelY + kHeaderH;
    if (lay.brick && lay.r1 >= lay.r0) {
        // only the rows that have keys, as a block in the middle of the panel
        const int nr = lay.r1 - lay.r0 + 1;
        const float block = nr * kh + (nr - 1) * kGap;
        y0 = kPanelY + kHeaderH + (avail_h - block) * 0.5f;
        r -= lay.r0;
    }
    return {kPanelX + kPad + c * (kw + kGap), y0 + r * (kh + kGap), kw, kh};
}

bool g_focus = false;       // the controller drives the keypad
int g_cursor = 0;
int g_touch_key = -1;       // key under a finger this frame
int g_pad_key = -1;         // key held with A while focused
uint64_t g_move_tick = 0;   // cursor repeat
uint64_t g_move_mask = 0;

uint64_t toggle_button() {
    // R3, unless R3 alone opens the pause menu.
    return g_pause_combo == 3 ? HidNpadButton_StickL : HidNpadButton_StickR;
}

int key_at(const Layout& lay, float x, float y) {
    const int kCols = lay.cols;
    const int rows = (lay.count + kCols - 1) / kCols;
    for (int i = 0; i < lay.count; i++) {
        if (lay.keys[i].style == Gap) continue;
        const Rect r = key_rect(lay, i);
        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) return i;
    }
    return -1;
}

void move_cursor(const Layout& lay, int dx, int dy) {
    const int kCols = lay.cols;
    const int rows = (lay.count + kCols - 1) / kCols;
    int r = g_cursor / kCols, c = g_cursor % kCols;
    for (int tries = 0; tries < kCols * rows; tries++) {
        c = (c + dx + kCols) % kCols;
        r = (r + dy + rows) % rows;
        const int i = r * kCols + c;
        if (i < lay.count && lay.keys[i].style != Gap) { g_cursor = i; return; }
        if (!dx && !dy) return;
    }
}

// ---- drawing ----
void rrect(float x, float y, float w, float h, float rad, float r, float g, float b, float a) {
    rad = std::max(0.0f, std::min(rad, std::min(w, h) * 0.5f));
    float xy[2 * 36];
    int n = 0;
    const float cx[4] = {x + w - rad, x + rad, x + rad, x + w - rad};
    const float cy[4] = {y + h - rad, y + h - rad, y + rad, y + rad};
    for (int c = 0; c < 4; c++)
        for (int i = 0; i < 9; i++) {
            const float t = 1.5707963f * (float)(c + i / 8.0f);
            xy[2 * n] = cx[c] + std::cos(t) * rad;
            xy[2 * n + 1] = cy[c] + std::sin(t) * rad;
            n++;
        }
    switch_ui_draw_polygon(xy, n, r, g, b, a);
}

void text_centered(float cx, float cy, float size, float r, float g, float b, const std::string& s) {
    const float w = switch_ui_text_width(s, size, true);
    switch_ui_draw_text(cx - w * 0.5f, cy - size * 0.62f, size, r, g, b, 1.0f, s, true);
}

}  // namespace

// Like every on-screen keypad (the Microvision's too), an Experimental Feature.
bool onscreen_keypad_game_has_one() { return layout_for_game().count > 0 && g_experimental_features_enabled; }

bool onscreen_keypad_visible() {
    return g_experimental_features_enabled && g_onscreen_keypad && g_cpu && layout_for_game().count > 0 &&
           g_screen_rotation == ScreenRotation::Deg0;
}

float onscreen_keypad_game_area() { return onscreen_keypad_visible() ? kGameFrac : 1.0f; }

void onscreen_keypad_update() {
    g_touch_key = -1;
    g_pad_key = -1;
    if (!onscreen_keypad_visible()) {
        g_focus = false;
        return;
    }
    const Layout lay = layout_for_game();
    if (g_cursor >= lay.count || lay.keys[g_cursor].style == Gap) g_cursor = 0;

    // Touch (handheld; docked there's simply never a touch).
    HidTouchScreenState ts = {};
    if (hidGetTouchScreenStates(&ts, 1) && ts.count > 0) {
        g_touch_key = key_at(lay, (float)ts.touches[0].x, (float)ts.touches[0].y);
        if (g_touch_key >= 0) g_cursor = g_touch_key;
    }

    const uint64_t down = switch_input_raw_down(), held = switch_input_raw_held();
    const uint64_t toggle = toggle_button();
    if ((down & toggle) && !(held & pause_combo_mask() & ~toggle)) {
        g_focus = !g_focus;
    } else if (g_focus && (down & HidNpadButton_B)) {
        g_focus = false;
    }
    if (!g_focus) return;

    // Cursor: a press moves once, holding repeats.
    constexpr uint64_t kDirs = HidNpadButton_Up | HidNpadButton_Down | HidNpadButton_Left | HidNpadButton_Right;
    const uint64_t now = armTicksToNs(armGetSystemTick());
    uint64_t step = down & kDirs;
    if (step) {
        g_move_mask = step;
        g_move_tick = now + 350'000'000ull;
    } else if ((held & g_move_mask) && now >= g_move_tick) {
        step = held & g_move_mask;
        g_move_tick = now + 110'000'000ull;
    } else if (!(held & g_move_mask)) {
        g_move_mask = 0;
    }
    if (step & HidNpadButton_Up) move_cursor(lay, 0, -1);
    if (step & HidNpadButton_Down) move_cursor(lay, 0, 1);
    if (step & HidNpadButton_Left) move_cursor(lay, -1, 0);
    if (step & HidNpadButton_Right) move_cursor(lay, 1, 0);
    if (held & HidNpadButton_A) g_pad_key = g_cursor;

    // The game gets nothing while the keypad has the controller.
    switch_input_clear_for_game();
}

void onscreen_keypad_apply(SM5XX* cpu) {
    static int s_brick_held = -1;  // Microvision key the keypad is holding down
    if (!cpu || !onscreen_keypad_visible()) {
        s_brick_held = -1;
        return;
    }
    const Layout lay = layout_for_game();
    if (lay.brick) {
        // BrickCore buttons stay as set: press on the touch/A, release after.
        BrickCore* core = static_cast<BrickCore*>(cpu);
        const int k = g_touch_key >= 0 ? g_touch_key : g_pad_key;
        const int now = (k >= 0 && k < 12 && !g_mv_names[k].empty()) ? k : -1;
        if (now != s_brick_held) {
            if (s_brick_held >= 0) core->set_button(g_mv_names[s_brick_held], false);
            if (now >= 0) core->set_button(g_mv_names[now], true);
            s_brick_held = now;
        }
        return;
    }
    for (int k : {g_touch_key, g_pad_key}) {
        if (k < 0 || k >= lay.count || lay.keys[k].group < 0) continue;
        cpu->input_set(lay.keys[k].group, lay.keys[k].line, true);
    }
}

void onscreen_keypad_render() {
    if (!onscreen_keypad_visible()) return;
    const Layout lay = layout_for_game();
    const int kCols = lay.cols;
    const int rows = (lay.count + kCols - 1) / kCols;
    const float hr = 0x0A / 255.0f, hg = 0xBF / 255.0f, hb = 1.0f;

    switch_ui_set_rotation(0);
    switch_ui_begin_frame(1280, 720);
    if (g_focus) rrect(kPanelX - 3, kPanelY - 3, kPanelW + 6, kPanelH + 6, 21.0f, hr, hg, hb, 1.0f);
    rrect(kPanelX, kPanelY, kPanelW, kPanelH, 18.0f, 0.17f, 0.17f, 0.19f, 1.0f);
    {
        // the title, centred at the top of the panel
        const std::string& title = switch_tr("Keypad");
        switch_ui_draw_text(kPanelX + (kPanelW - switch_ui_text_width(title, 24.0f, true)) * 0.5f, kPanelY + 16.0f, 24.0f,
                            1.0f, 1.0f, 1.0f, 1.0f, title, true);
    }

    for (int i = 0; i < lay.count; i++) {
        const Key& k = lay.keys[i];
        if (k.style == Gap) continue;
        const Rect r = key_rect(lay, i);
        const bool pressed = (i == g_touch_key || i == g_pad_key);
        float fr, fg, fb, tr, tg, tb;
        switch (k.style) {
            case Op:  fr = 1.0f; fg = 0.67f; fb = 0.24f; tr = tg = tb = 0.1f; break;
            case Fn:  fr = 0.27f; fg = 0.28f; fb = 0.31f; tr = tg = tb = 0.95f; break;
            case Red: fr = 0.86f; fg = 0.27f; fb = 0.24f; tr = tg = tb = 0.97f; break;
            default:  fr = 0.92f; fg = 0.92f; fb = 0.91f; tr = tg = tb = 0.1f; break;
        }
        if (lay.brick && g_mv_has_color) {
            // the cartridge's own key colour; dark text on light keys (Sea Duel)
            fr = g_mv_rgb[0]; fg = g_mv_rgb[1]; fb = g_mv_rgb[2];
            const float lum = 0.30f * fr + 0.59f * fg + 0.11f * fb;
            tr = tg = tb = lum > 0.55f ? 0.08f : 0.96f;
        }
        if (pressed) { fr *= 0.75f; fg *= 0.75f; fb *= 0.75f; }
        const float oy = pressed ? 3.0f : 0.0f;
        if (g_focus && i == g_cursor) rrect(r.x - 5, r.y - 5, r.w + 10, r.h + 14, 13.0f, hr, hg, hb, 1.0f);
        rrect(r.x, r.y + 4, r.w, r.h, 10.0f, 0.06f, 0.06f, 0.06f, 1.0f);
        if (lay.brick && g_mv_border) rrect(r.x - 2, r.y + oy - 2, r.w + 4, r.h + 4, 11.0f, 0.92f, 0.92f, 0.92f, 1.0f);
        rrect(r.x, r.y + oy, r.w, r.h, 10.0f, fr, fg, fb, 1.0f);
        const size_t len = strlen(k.label);
        float size = len <= 2 ? 26.0f : (len <= 5 ? 18.0f : 15.0f);
        if (lay.brick) {
            // the cartridge's name for the key, and the Switch button on it
            size = len <= 2 ? 26.0f : (len <= 7 ? 18.0f : 14.0f);
            while (size > 11.0f && switch_ui_text_width(k.label, size, true) > r.w - 10.0f) size -= 1.0f;
            text_centered(r.x + r.w * 0.5f, r.y + r.h * 0.36f + oy, size, tr, tg, tb, k.label);
            const std::string b = assigned_button(k.label);
            if (!b.empty()) {
                const float bs = 17.0f, bw = std::max(30.0f, switch_ui_text_width(b, bs, true) + 16.0f);
                rrect(r.x + (r.w - bw) * 0.5f, r.y + r.h * 0.58f + oy, bw, bs + 8.0f, (bs + 8.0f) * 0.5f, 0.92f, 0.92f, 0.91f, 1.0f);
                text_centered(r.x + r.w * 0.5f, r.y + r.h * 0.58f + oy + (bs + 8.0f) * 0.5f, bs, 0.1f, 0.1f, 0.1f, b);
            }
            continue;
        }
        text_centered(r.x + r.w * 0.5f, r.y + r.h * 0.5f + oy, size, tr, tg, tb, k.label);
    }

    const char* hint = g_focus ? "D-Pad / Stick  Move  -  A  Press  -  B  Back to game"
                               : (g_pause_combo == 3 ? "Tap a key  -  L3  Use the keypad with the controller"
                                                     : "Tap a key  -  R3  Use the keypad with the controller");
    switch_ui_draw_text(kPanelX + 20.0f, kPanelY + kPanelH - 40.0f, 17.0f, 1.0f, 1.0f, 1.0f, g_focus ? 1.0f : 0.6f, hint);
}
