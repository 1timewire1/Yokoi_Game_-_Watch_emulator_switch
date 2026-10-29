#include "switch_menu.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include <switch.h>

#include "BrickCores/BrickRom.h"
#include "std/GW_ROM.h"
#include "std/debug_log.h"
#include "std/gw_pack.h"
#include "std/load_file.h"
#include "std/savestate.h"
#include "std/settings.h"

#include "../third_party/stb_image.h"

#include "pet_time.h"
#include "switch_game_loader.h"
#include "switch_layout.h"
#include "switch_runtime_state.h"
#include "switch_settings.h"
#include "render/switch_gl.h"
#include "render/switch_header_icon.h"
#include "render/switch_ui.h"

namespace {

// Hekate-Toolbox's dark theme palette (source/gui/theme.c, ColorSetId_Dark),
// reused here for visual consistency even though the renderer is entirely
// different (this project draws through GLES3 quads, not a software framebuffer).
constexpr float kBgR = 0x31 / 255.0f, kBgG = 0x31 / 255.0f, kBgB = 0x31 / 255.0f;
constexpr float kTextR = 1.0f, kTextG = 1.0f, kTextB = 1.0f;
constexpr float kAccentR = 0x59 / 255.0f, kAccentG = 0xED / 255.0f, kAccentB = 0xC0 / 255.0f;
constexpr float kHighlightR = 0x0A / 255.0f, kHighlightG = 0xBF / 255.0f, kHighlightB = 1.0f;

// Slimmer than Hekate's own 88/73 - their bars hold a title + a grid of
// icons; ours only need a short title/hint line each, so the extra height
// goes to the thumbnail instead.
constexpr int kTopBarH = 64;
constexpr int kBottomBarH = 56;
constexpr int kThumbMax = 460;

bool g_menu_active = true;
uint8_t g_selected_index = 0;

// Load-save-state prompt, shown instead of loading immediately when A is
// pressed on a game that already has a save file (source/std/savestate.h's
// save_state_exists()) - mirrors 3DS's STATE_SAVE_PROMPT: skipped entirely
// (game loads fresh, same as always) when no save exists for that game.
bool g_load_prompt_active = false;

// Experimental Games enable confirmation, layered on top of the settings
// screen (g_settings_active stays true throughout) when that row is about to
// flip Off -> On - see the row handler in update_settings_menu(). Turning it
// back Off never goes through this, only enabling does.
bool g_experimental_games_confirm_active = false;

// Same idea as g_experimental_games_confirm_active, for the Experimental
// Features row.
bool g_experimental_features_confirm_active = false;

// Settings sub-screen, opened with Plus from the game list (Plus is free
// there - during gameplay it's mapped to the G&W "A" button instead, and
// gameplay never routes input through switch_menu_update() at all). Values
// apply live (switch_screen_render() reads g_settings/g_screen_rotation
// every frame) and are written to disk immediately on each change via
// switch_settings_save(), so there's no separate save/cancel step to forget.
bool g_settings_active = false;
int g_settings_selected_row = 0;

// True when this settings screen was opened with ZL+ZR *during gameplay*
// (switch_menu_open_ingame_settings(), called from main.cpp) rather than with
// a bare Plus from the game list. Changes what the tail of the row list is
// (see build_settings_rows() below) and what closing the screen does: B
// resumes the paused game in place instead of returning to the game list.
bool g_settings_from_gameplay = false;

// Custom Background Color / Segments entry (via the on-screen keyboard - see
// open_background_custom_keyboard()/open_segments_custom_keyboard() below).
// These are pure UI-navigation state, never persisted: g_settings.background_color/
// segment_marking_alpha (which ARE persisted, same as always) already carry
// the actual applied value regardless of how it got set, and bg_color_display()/
// seg_display() already fall back to showing a raw value that doesn't match
// any preset - so on the next boot, a genuinely custom stored value still
// displays correctly without these flags needing to survive a restart. What
// they *do* need to survive is navigating within the settings screen: with
// only the applied value to go on, there'd be no way to tell "sitting on the
// Custom slot with nothing typed yet" (applied value defaults to Light
// Yellow/Light, per explicit request) apart from "sitting on the Light
// Yellow/Light preset itself" - see switch_menu_init() for how these seed
// from the loaded settings at boot.
bool g_bg_custom_selected = false;
std::string g_bg_custom_text;  // raw entered hex (no '#'), uppercase; empty = not yet entered
bool g_segcolor_custom_selected = false;
std::string g_segcolor_custom_text;  // same format as g_bg_custom_text
bool g_seg_custom_selected = false;
std::string g_seg_custom_text; // raw entered decimal 0-255; empty = not yet entered
bool g_panelgap_custom_selected = false;
std::string g_panelgap_custom_text; // raw entered decimal 0-30; empty = not yet entered
bool g_pettimecap_custom_selected = false;
std::string g_pettimecap_custom_text; // raw entered decimal 1-9999 (hours); empty = not yet entered

// Every row the settings screen can show, in display order. Which of these
// actually appear varies: BackgroundCustomEntry/SegmentsCustomEntry/
// PanelGapCustomEntry only show up while their parent row is in Custom mode
// (see build_settings_rows()), and ConsoleArt/LoadStateBehavior/LogFile/
// ExperimentalGames/ExperimentalFeatures vs. ReturnToMenu depend on g_settings_from_gameplay,
// same as before. Panel Gap is unconditional like Rotation/Background/
// Segments above it - see g_panel_gap_percent's doc comment in
// switch_settings.h for why it applies live instead of needing a reload.
enum class SettingsRow : uint8_t {
    Rotation,
    Background,
    BackgroundCustomEntry,
    BackgroundArt,
    SegmentColor,
    SegmentColorCustomEntry,
    Segments,
    SegmentsCustomEntry,
    ConsoleArt,
    PanelGap,
    PanelGapCustomEntry,
    LoadStateBehavior,
    CloneFilter,
    TimeSetMode,
    PetTime,
    PetTimeCap,
    PetTimeCapCustomEntry,
    PetTimeDisplay,
    LogFile,
    ExperimentalGames,
    ExperimentalFeatures,
    DosimeterReading,
    ReturnToMenu,
};

// The single source of truth for "what rows exist right now, in what order" -
// both render_settings_rows() and update_settings_menu() build this fresh
// each time rather than trusting a cached row count, so they can never
// disagree about what row index N currently means.
std::vector<SettingsRow> build_settings_rows() {
    std::vector<SettingsRow> rows;
    rows.push_back(SettingsRow::Rotation);
    rows.push_back(SettingsRow::Background);
    if (g_bg_custom_selected) rows.push_back(SettingsRow::BackgroundCustomEntry);
    rows.push_back(SettingsRow::BackgroundArt);
    rows.push_back(SettingsRow::SegmentColor);
    if (g_segcolor_custom_selected) rows.push_back(SettingsRow::SegmentColorCustomEntry);
    rows.push_back(SettingsRow::Segments);
    if (g_seg_custom_selected) rows.push_back(SettingsRow::SegmentsCustomEntry);
    rows.push_back(SettingsRow::PanelGap);
    if (g_panelgap_custom_selected) rows.push_back(SettingsRow::PanelGapCustomEntry);
    if (g_settings_from_gameplay) {
        // Only while Altair itself is loaded and Experimental Features is
        // on - see g_dosimeter_reading's doc comment in switch_settings.h.
        if (g_game && g_game->ref == "NALTAIR" && g_experimental_features_enabled) {
            rows.push_back(SettingsRow::DosimeterReading);
        }
        rows.push_back(SettingsRow::ReturnToMenu);
    } else {
        rows.push_back(SettingsRow::ConsoleArt);
        rows.push_back(SettingsRow::LoadStateBehavior);
        rows.push_back(SettingsRow::CloneFilter);
        rows.push_back(SettingsRow::TimeSetMode);
        // Pet Time is an Experimental Feature (pet_time_enabled()); the
        // limit and display only mean anything under Real Time.
        if (g_experimental_features_enabled) rows.push_back(SettingsRow::PetTime);
        if (g_experimental_features_enabled && g_pet_time_mode == PetTimeMode::RealTime) {
            rows.push_back(SettingsRow::PetTimeCap);
            if (g_pettimecap_custom_selected) rows.push_back(SettingsRow::PetTimeCapCustomEntry);
            rows.push_back(SettingsRow::PetTimeDisplay);
        }
        rows.push_back(SettingsRow::LogFile);
        rows.push_back(SettingsRow::ExperimentalGames);
        rows.push_back(SettingsRow::ExperimentalFeatures);
    }
    return rows;
}

// Action rows have no adjustable value - Left/Right do nothing, only A does
// (open the keyboard, or leave the game) - rendered as plain highlighted
// text spanning the row instead of a label/value pair with < > arrows.
bool settings_row_is_action(SettingsRow row) {
    return row == SettingsRow::ReturnToMenu
        || row == SettingsRow::BackgroundCustomEntry
        || row == SettingsRow::SegmentColorCustomEntry
        || row == SettingsRow::SegmentsCustomEntry
        || row == SettingsRow::PanelGapCustomEntry
        || row == SettingsRow::PetTimeCapCustomEntry;
}

// Mirrors Android's SettingsMenu.java 3-preset "Segments" spinner (None/
// Light/Dark) over the same underlying g_settings.segment_marking_alpha the
// 3DS exposes as a raw 0-255 stepper - see the settings-menu clarification
// this was designed from.
constexpr uint8_t kSegPresetAlpha[3] = {0, 5, 10};
constexpr const char* kSegPresetNames[3] = {"None", "Light", "Dark"};

// Same 3-preset-plus-Custom shape as Segments above, for the gap inserted
// below the top panel (see g_panel_gap_percent's doc comment in
// switch_settings.h) - a percentage of the top panel's own height.
constexpr uint8_t kPanelGapPresetPercent[3] = {0, 4, 10};
constexpr const char* kPanelGapPresetNames[3] = {"None", "Small", "Large"};

// Pet Time's catch-up limit (see g_pet_time_cap_hours in switch_settings.h):
// presets plus a Custom slot (hours, via keyboard). 0 = no limit.
constexpr int kNumPetTimeCapPresets = 5;
constexpr uint32_t kPetTimeCapPresetHours[kNumPetTimeCapPresets] = {8, 24, 72, 168, 0};
constexpr const char* kPetTimeCapPresetNames[kNumPetTimeCapPresets] = {"8 Hours", "24 Hours", "72 Hours", "1 Week",
                                                                        "Unlimited"};
constexpr uint32_t kPetTimeCapDefaultHours = 72;

// Nearest-index fallbacks (never exact for an off-preset value) - only ever
// used as a *starting point* to cycle Left/Right from, never to rewrite the
// loaded value itself. Display uses bg_color_display()/seg_display() below
// instead, which show the raw value honestly when it doesn't match a preset -
// this matters under g_debug_mode, where a hand-edited yokoi_settings.txt can
// legitimately hold a value outside the normal presets (see its doc comment
// in switch_settings.h).
// Game Background Color presets: the shared list (std/settings.h, also used
// by 3DS) plus Black, which is Switch-only - only this build has a Segment
// Color setting to pair it with (3DS would draw black segments on black).
struct SwitchColorPreset {
    const char* name;
    uint32_t color;
};
const SwitchColorPreset kBgPresets[] = {
    {BACKGROUND_PRESETS[0].name, BACKGROUND_PRESETS[0].color},
    {BACKGROUND_PRESETS[1].name, BACKGROUND_PRESETS[1].color},
    {BACKGROUND_PRESETS[2].name, BACKGROUND_PRESETS[2].color},
    {BACKGROUND_PRESETS[3].name, BACKGROUND_PRESETS[3].color},
    {BACKGROUND_PRESETS[4].name, BACKGROUND_PRESETS[4].color},
    {BACKGROUND_PRESETS[5].name, BACKGROUND_PRESETS[5].color},
    {"Black", 0x000000},
};
constexpr int kNumBgPresets = sizeof(kBgPresets) / sizeof(kBgPresets[0]);
static_assert(NUM_BG_PRESETS == 6, "kBgPresets mirrors the shared list - update it too");

// Segment Color presets (see g_segment_color in switch_settings.h). Auto is
// the default and the original look; Red pairs with the Black background for
// Tiger R-Zone's red-on-black screen.
const SwitchColorPreset kSegColorPresets[] = {
    {"Auto", kSegmentColorAuto},
    {"Black", 0x000000},
    {"Red", 0xFF2A1A},
    {"Green", 0x20C040},
    {"Blue", 0x2A5CFF},
    {"White", 0xFFFFFF},
};
constexpr int kNumSegColorPresets = sizeof(kSegColorPresets) / sizeof(kSegColorPresets[0]);

int find_segcolor_preset_index(uint32_t color) {
    for (int i = 0; i < kNumSegColorPresets; i++) {
        if (kSegColorPresets[i].color == color) return i;
    }
    return 0;
}

bool segcolor_matches_any_preset(uint32_t color) {
    for (int i = 0; i < kNumSegColorPresets; i++) {
        if (kSegColorPresets[i].color == color) return true;
    }
    return false;
}

std::string segcolor_display(uint32_t color) {
    for (int i = 0; i < kNumSegColorPresets; i++) {
        if (kSegColorPresets[i].color == color) return kSegColorPresets[i].name;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "#%06X", (unsigned)(color & 0xFFFFFFu));
    return std::string(buf);
}

int find_bg_preset_index(uint32_t color) {
    for (int i = 0; i < kNumBgPresets; i++) {
        if (kBgPresets[i].color == color) return i;
    }
    return 0;
}

int find_seg_preset_index(uint8_t alpha) {
    for (int i = 0; i < 3; i++) {
        if (kSegPresetAlpha[i] == alpha) return i;
    }
    return 1; // closest-match fallback, same as Android's findClosestSegPresetIndex()
}

int find_panelgap_preset_index(uint8_t percent) {
    for (int i = 0; i < 3; i++) {
        if (kPanelGapPresetPercent[i] == percent) return i;
    }
    return 0; // closest-match fallback - unlike Segments, 0 (matches the old
              // always-touching behavior) is the more sensible default guess
              // for an off-preset value than the middle preset.
}

// Exact-match checks (unlike the fallback finders above) - used to seed
// g_bg_custom_selected/g_seg_custom_selected from the loaded settings at
// boot (see switch_menu_init()): a value that doesn't match any preset can
// only have gotten there via a custom entry.
bool bg_matches_any_preset(uint32_t color) {
    for (int i = 0; i < kNumBgPresets; i++) {
        if (kBgPresets[i].color == color) return true;
    }
    return false;
}

bool seg_matches_any_preset(uint8_t alpha) {
    for (int i = 0; i < 3; i++) {
        if (kSegPresetAlpha[i] == alpha) return true;
    }
    return false;
}

int find_pettimecap_preset_index(uint32_t hours) {
    for (int i = 0; i < kNumPetTimeCapPresets; i++) {
        if (kPetTimeCapPresetHours[i] == hours) return i;
    }
    return 2;  // 72 Hours
}

bool pettimecap_matches_any_preset(uint32_t hours) {
    for (int i = 0; i < kNumPetTimeCapPresets; i++) {
        if (kPetTimeCapPresetHours[i] == hours) return true;
    }
    return false;
}

std::string pettimecap_display(uint32_t hours) {
    for (int i = 0; i < kNumPetTimeCapPresets; i++) {
        if (kPetTimeCapPresetHours[i] == hours) return kPetTimeCapPresetNames[i];
    }
    return std::to_string((unsigned)hours) + "h";
}

bool panelgap_matches_any_preset(uint8_t percent) {
    for (int i = 0; i < 3; i++) {
        if (kPanelGapPresetPercent[i] == percent) return true;
    }
    return false;
}

std::string bg_color_display(uint32_t color) {
    for (int i = 0; i < kNumBgPresets; i++) {
        if (kBgPresets[i].color == color) return kBgPresets[i].name;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "#%06X", (unsigned)(color & 0xFFFFFFu));
    return std::string(buf);
}

std::string seg_display(uint8_t alpha) {
    for (int i = 0; i < 3; i++) {
        if (kSegPresetAlpha[i] == alpha) return kSegPresetNames[i];
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", (unsigned)alpha);
    return std::string(buf);
}

std::string panelgap_display(uint8_t percent) {
    for (int i = 0; i < 3; i++) {
        if (kPanelGapPresetPercent[i] == percent) return kPanelGapPresetNames[i];
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%u%%", (unsigned)percent);
    return std::string(buf);
}

// Launches libnx's swkbd applet and blocks until the user confirms or
// cancels - the standard, simple way to take one-off text input in
// homebrew; no separate polling/async handling needed since the whole
// frame loop is already paused on this modal screen anyway. Returns false
// (leaving `out` untouched) on cancel or failure; true with `out` set to
// whatever the user typed (which may still need validating) otherwise.
bool show_keyboard(SwkbdType type, const char* header, const char* guide, const std::string& initial_text,
                    uint32_t max_len, std::string& out) {
    SwkbdConfig kbd;
    if (R_FAILED(swkbdCreate(&kbd, 0))) {
        return false;
    }
    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetType(&kbd, type);
    swkbdConfigSetHeaderText(&kbd, header);
    swkbdConfigSetGuideText(&kbd, guide);
    swkbdConfigSetStringLenMax(&kbd, max_len);
    swkbdConfigSetInitialCursorPos(&kbd, 1);
    if (!initial_text.empty()) {
        swkbdConfigSetInitialText(&kbd, initial_text.c_str());
    }

    char buf[32] = {0};
    const Result rc = swkbdShow(&kbd, buf, sizeof(buf));
    swkbdClose(&kbd);
    if (R_FAILED(rc)) {
        return false; // user cancelled - leave existing state untouched
    }
    out = buf;
    return true;
}

// Applies (or clears, on invalid input) a Custom Background Color entry.
// Per explicit spec: invalid input is silently ignored and reset to blank,
// which is what makes the applied color fall back to Light Yellow (see
// update_settings_menu()'s Background case) rather than leaving whatever
// was typed half-applied.
void open_background_custom_keyboard() {
    std::string text;
    if (!show_keyboard(SwkbdType_Normal, "Custom Background Color", "RRGGBB", g_bg_custom_text, 6, text)) {
        return; // cancelled
    }

    bool valid = (text.size() == 6);
    if (valid) {
        for (unsigned char c : text) {
            if (!std::isxdigit(c)) { valid = false; break; }
        }
    }

    if (valid) {
        for (char& c : text) c = (char)std::toupper((unsigned char)c);
        g_bg_custom_text = text;
        g_settings.background_color = (uint32_t)strtoul(text.c_str(), nullptr, 16);
    } else {
        g_bg_custom_text.clear();
        g_settings.background_color = BACKGROUND_PRESETS[0].color; // Light Yellow
    }
    switch_settings_save();
}

// Same as open_background_custom_keyboard(), for Segment Color - invalid
// input falls back to Auto.
void open_segment_color_custom_keyboard() {
    std::string text;
    if (!show_keyboard(SwkbdType_Normal, "Custom Segment Color", "RRGGBB", g_segcolor_custom_text, 6, text)) {
        return; // cancelled
    }

    bool valid = (text.size() == 6);
    if (valid) {
        for (unsigned char c : text) {
            if (!std::isxdigit(c)) { valid = false; break; }
        }
    }

    if (valid) {
        for (char& c : text) c = (char)std::toupper((unsigned char)c);
        g_segcolor_custom_text = text;
        g_segment_color = (uint32_t)strtoul(text.c_str(), nullptr, 16);
    } else {
        g_segcolor_custom_text.clear();
        g_segment_color = kSegmentColorAuto;
    }
    switch_settings_save();
}

// Same idea as open_background_custom_keyboard(), for the 0-255 Segments
// value - falls back to the Light preset's alpha (5) on invalid input.
void open_segments_custom_keyboard() {
    std::string text;
    if (!show_keyboard(SwkbdType_NumPad, "Custom Segment Marking", "0-255", g_seg_custom_text, 3, text)) {
        return; // cancelled
    }

    bool valid = !text.empty() && text.size() <= 3;
    if (valid) {
        for (unsigned char c : text) {
            if (!std::isdigit(c)) { valid = false; break; }
        }
    }
    const long value = valid ? strtol(text.c_str(), nullptr, 10) : -1;
    valid = valid && value >= 0 && value <= 255;

    if (valid) {
        g_seg_custom_text = text;
        g_settings.segment_marking_alpha = (uint8_t)value;
    } else {
        g_seg_custom_text.clear();
        g_settings.segment_marking_alpha = kSegPresetAlpha[1]; // Light
    }
    switch_settings_save();
}

// Same idea again, for the 0-30 Panel Gap percentage - falls back to the
// Small preset (4%) on invalid input.
void open_panelgap_custom_keyboard() {
    std::string text;
    if (!show_keyboard(SwkbdType_NumPad, "Custom Panel Gap", "0-30", g_panelgap_custom_text, 2, text)) {
        return; // cancelled
    }

    bool valid = !text.empty() && text.size() <= 2;
    if (valid) {
        for (unsigned char c : text) {
            if (!std::isdigit(c)) { valid = false; break; }
        }
    }
    const long value = valid ? strtol(text.c_str(), nullptr, 10) : -1;
    valid = valid && value >= 0 && value <= 30;

    if (valid) {
        g_panelgap_custom_text = text;
        g_panel_gap_percent = (uint8_t)value;
    } else {
        g_panelgap_custom_text.clear();
        g_panel_gap_percent = kPanelGapPresetPercent[1]; // Small
    }
    switch_settings_save();
    rebuild_layout_from_game(); // apply immediately - see the Left/Right case's comment
}

// Same again for Pet Time's catch-up limit, in hours (1-9999) - falls back to
// the 72 Hours preset on invalid input.
void open_pettimecap_custom_keyboard() {
    std::string text;
    if (!show_keyboard(SwkbdType_NumPad, "Custom Catch-Up Limit", "Hours, 1-9999", g_pettimecap_custom_text, 4, text)) {
        return; // cancelled
    }

    bool valid = !text.empty() && text.size() <= 4;
    if (valid) {
        for (unsigned char c : text) {
            if (!std::isdigit(c)) { valid = false; break; }
        }
    }
    const long value = valid ? strtol(text.c_str(), nullptr, 10) : -1;
    valid = valid && value >= 1 && value <= 9999;

    if (valid) {
        g_pettimecap_custom_text = text;
        g_pet_time_cap_hours = (uint32_t)value;
    } else {
        g_pettimecap_custom_text.clear();
        g_pet_time_cap_hours = kPetTimeCapDefaultHours;
    }
    switch_settings_save();
}

const char* rotation_label(ScreenRotation r) {
    switch (r) {
        case ScreenRotation::Deg90:  return "90";
        case ScreenRotation::Deg270: return "270";
        default:                     return "0";
    }
}

ScreenRotation cycle_rotation(ScreenRotation cur, int dir) {
    // Only 0/90/270 are offered (180 skipped, per explicit request) - a
    // plain mod-3 cycle over the enum's declaration order does that directly.
    int i = (int)cur;
    i = (i + dir) % 3;
    if (i < 0) i += 3;
    return (ScreenRotation)i;
}

const char* load_state_behavior_label(LoadStateBehavior b) {
    switch (b) {
        case LoadStateBehavior::AutoLoad:  return "Auto-Load";
        case LoadStateBehavior::NeverLoad: return "Never Load";
        default:                           return "Ask";
    }
}

LoadStateBehavior cycle_load_state_behavior(LoadStateBehavior cur, int dir) {
    int i = (int)cur;
    i = (i + dir) % 3;
    if (i < 0) i += 3;
    return (LoadStateBehavior)i;
}

// Resolves a saved ref back to a pack index, distinguishing "found at index
// 0" from "not found" via out_found (load_game(0)'s ref matching would
// otherwise look identical to a failed search) - used to check whether
// g_last_game_played_ref points at an experimental game when that setting gets turned
// off (see update_settings_menu()'s row 6 handler).
uint8_t find_game_index_by_ref(const std::string& ref, bool* out_found) {
    *out_found = false;
    const size_t n = get_nb_name();
    for (size_t i = 0; i < n; i++) {
        const GW_rom* g = load_game((uint8_t)i);
        if (g && g->ref == ref) {
            *out_found = true;
            return (uint8_t)i;
        }
    }
    return 0;
}

// The two Maker modes keep each manufacturer's games together (groups in
// mfr_rank() order) and sort *within* each group the same consistent way -
// by name or by release date - instead of the pack's own per-manufacturer
// ordering, which differed from maker to maker. Name/Date ignore the
// manufacturer entirely and sort the whole list. Minus cycles them in this
// order.
enum class SortMode : uint8_t { MakerName, MakerDate, Name, Date };
constexpr int kSortModeCount = 4;
SortMode g_sort_mode = SortMode::MakerName;
// Permutation of [0, get_nb_name()) in the current sort mode's order. Left/
// Right and the Up/Down group-jump both walk this instead of raw indices,
// so "next"/"first of the next group" have one implementation shared by
// all three modes (see select_next() and select_next_other_group() below).
std::vector<uint8_t> g_sorted_order;

GLuint g_thumb_tex = 0;
int g_thumb_w = 0, g_thumb_h = 0;
uint8_t g_loaded_thumb_index = 0xFF;

// Small header-bar icon shown just left of the "YOKOI"/"PAUSED" title (see
// switch_menu_render()) - decoded once from the compiled-in PNG bytes in
// switch_header_icon.h, never reloaded after that (unlike g_thumb_tex,
// which changes with the game-list selection).
GLuint g_header_icon_tex = 0;
int g_header_icon_w = 0, g_header_icon_h = 0;

void load_header_icon_if_needed() {
    if (g_header_icon_tex != 0) return;

    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load_from_memory(kHeaderIconPng, (int)kHeaderIconPngLen, &w, &h, &channels, 4);
    if (!pixels) {
        YOKOI_LOG("switch_menu: header icon decode failed: %s", stbi_failure_reason());
        return;
    }

    glGenTextures(1, &g_header_icon_tex);
    glBindTexture(GL_TEXTURE_2D, g_header_icon_tex);
    // NEAREST, not LINEAR (unlike thumbnails) - this is small pixel art
    // scaled up into the header bar, and should stay crisp rather than
    // blur at the edges.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);

    g_header_icon_w = w;
    g_header_icon_h = h;
}

std::string t3x_path_to_png_asset(const std::string& p) {
    if (p.empty()) return std::string();
    std::string base = p;
    size_t slash = base.find_last_of('/');
    if (slash != std::string::npos) base = base.substr(slash + 1);
    const std::string t3x = ".t3x";
    if (base.size() >= t3x.size() && base.compare(base.size() - t3x.size(), t3x.size(), t3x) == 0) {
        base.replace(base.size() - t3x.size(), t3x.size(), ".png");
    }
    return base;
}

// g_thumb_w/h are the *decoded texture's* pixel dimensions (needed to
// normalize UVs) - NOT the visible console art's dimensions. Console art PNGs
// from CONVERT_ROM are a sprite cropped out of a larger canvas via
// GW_rom::console_info (same as the in-game console overlay in
// switch_screen.cpp), so the crop rect has to be read separately at draw
// time from console_info, not assumed to be the whole image.
void reload_thumbnail_if_needed() {
    if (g_loaded_thumb_index == g_selected_index) {
        return;
    }
    if (g_thumb_tex) {
        glDeleteTextures(1, &g_thumb_tex);
        g_thumb_tex = 0;
    }
    g_thumb_w = g_thumb_h = 0;

    const GW_rom* g = load_game(g_selected_index);
    if (!g) {
        g_loaded_thumb_index = g_selected_index;
        return;
    }

    const std::string asset = t3x_path_to_png_asset(g->path_console);
    const uint8_t* data = nullptr;
    size_t size = 0;
    if (asset.empty() || !gw_pack::get_file_bytes(asset, data, size) || !data || size == 0) {
        YOKOI_LOG("switch_menu: thumbnail pack entry not found: %s", asset.c_str());
        g_loaded_thumb_index = g_selected_index;
        return;
    }

    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load_from_memory(data, (int)size, &w, &h, &channels, 4);
    if (!pixels) {
        YOKOI_LOG("switch_menu: thumbnail decode failed for %s: %s", asset.c_str(), stbi_failure_reason());
        g_loaded_thumb_index = g_selected_index;
        return;
    }

    glGenTextures(1, &g_thumb_tex);
    glBindTexture(GL_TEXTURE_2D, g_thumb_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);

    g_thumb_w = w;
    g_thumb_h = h;
    g_loaded_thumb_index = g_selected_index;
}

uint8_t get_mfr(uint8_t idx) {
    const GW_rom* g = load_game(idx);
    return g ? g->manufacturer : GW_rom::MANUFACTURER_NINTENDO;
}

const char* mfr_name(uint8_t mfr) {
    if (mfr == GW_rom::MANUFACTURER_TRONICA) return "TRONICA";
    if (mfr == GW_rom::MANUFACTURER_ELEKTRONIKA) return "ELEKTRONIKA";
    if (mfr == GW_rom::MANUFACTURER_TIGER) return "TIGER";
    if (mfr == GW_rom::MANUFACTURER_KONAMI) return "KONAMI";
    if (mfr == GW_rom::MANUFACTURER_NELSONIC) return "NELSONIC";
    if (mfr == GW_rom::MANUFACTURER_VTECH) return "V-TECH";
    if (mfr == GW_rom::MANUFACTURER_BANDAI) return "BANDAI";
    if (mfr == GW_rom::MANUFACTURER_EPOCH) return "EPOCH";
    if (mfr == GW_rom::MANUFACTURER_NIKKO) return "NIKKO";
    if (mfr == GW_rom::MANUFACTURER_TANDY) return "TANDY";
    if (mfr == GW_rom::MANUFACTURER_POPY) return "POPY";
    if (mfr == GW_rom::MANUFACTURER_GAKKEN) return "GAKKEN";
    if (mfr == GW_rom::MANUFACTURER_SPICA) return "SPICA";
    if (mfr == GW_rom::MANUFACTURER_APOLLO) return "APOLLO";
    if (mfr == GW_rom::MANUFACTURER_BRICK_GAME) return "BRICK GAME";
    if (mfr == GW_rom::MANUFACTURER_HIRO) return "HIRO";
    if (mfr == GW_rom::MANUFACTURER_NATURE_LAND) return "NATURE LAND";
    if (mfr == GW_rom::MANUFACTURER_SPARKLE_CRYSTAL) return "SPARKLE CRYSTAL";
    if (mfr == GW_rom::MANUFACTURER_GAMETECH) return "GAMETECH";
    if (mfr == GW_rom::MANUFACTURER_COMPILE) return "COMPILE";
    if (mfr == GW_rom::MANUFACTURER_VGS) return "VGS";
    if (mfr == GW_rom::MANUFACTURER_COOL_TEC) return "COOL TEC";
    if (mfr == GW_rom::MANUFACTURER_HARTUNG) return "HARTUNG";
    if (mfr == GW_rom::MANUFACTURER_EDLA) return "EDLA";
    if (mfr == GW_rom::MANUFACTURER_STADLBAUER) return "STADLBAUER";
    if (mfr == GW_rom::MANUFACTURER_UNBRANDED) return "UNBRANDED";
    if (mfr == GW_rom::MANUFACTURER_DAVID_AND_JOHN) return "DAVID AND JOHN";
    if (mfr == GW_rom::MANUFACTURER_MCDONALDS) return "MCDONALD'S";
    // LCDonald's white-label re-skin, credited as "Burger Bard" on its
    // (fictional) instruction sheets - see MANUFACTURER_BURGER_BARD in GW_ROM.h.
    if (mfr == GW_rom::MANUFACTURER_BURGER_BARD) return "BURGER BARD";
    return "NINTENDO";
}

// Menu display order of the manufacturer groups (Manufacturer sort, and the
// Up/Down group jump). The ids stored in the pack aren't in this order and
// must never change, so the order lives here: the three always-visible
// manufacturers first, then everything the "Experimental Games" setting
// hides. The multi-title makers come first, then the rest alphabetically
// (McDonald's among them - it has a real-ROM game too), then Unbranded.
// David & John and Burger Bard go last: they're original LCD-inspired
// simulations rather than emulated hardware.
int mfr_rank(uint8_t mfr) {
    switch (mfr) {
        case GW_rom::MANUFACTURER_NINTENDO:              return 0;
        case GW_rom::MANUFACTURER_TRONICA:               return 1;
        case GW_rom::MANUFACTURER_ELEKTRONIKA:           return 2;
        case GW_rom::MANUFACTURER_TIGER:                 return 3;
        case GW_rom::MANUFACTURER_KONAMI:                return 4;
        case GW_rom::MANUFACTURER_NELSONIC:              return 5;
        case GW_rom::MANUFACTURER_VTECH:                 return 6;
        case GW_rom::MANUFACTURER_BANDAI:                return 7;
        case GW_rom::MANUFACTURER_POPY:                  return 8;
        case GW_rom::MANUFACTURER_EPOCH:                 return 9;
        case GW_rom::MANUFACTURER_GAKKEN:                return 10;
        case GW_rom::MANUFACTURER_NIKKO:                 return 11;
        case GW_rom::MANUFACTURER_TANDY:                 return 12;
        case GW_rom::MANUFACTURER_HIRO:                  return 13;
        case GW_rom::MANUFACTURER_SPICA:                 return 14;
        case GW_rom::MANUFACTURER_APOLLO:                return 15;
        case GW_rom::MANUFACTURER_BRICK_GAME:            return 16;
        case GW_rom::MANUFACTURER_COMPILE:               return 17;
        case GW_rom::MANUFACTURER_COOL_TEC:              return 18;
        case GW_rom::MANUFACTURER_EDLA:                  return 19;
        case GW_rom::MANUFACTURER_GAMETECH:              return 20;
        case GW_rom::MANUFACTURER_HARTUNG:               return 21;
        case GW_rom::MANUFACTURER_MCDONALDS:             return 22;
        case GW_rom::MANUFACTURER_NATURE_LAND:           return 23;
        case GW_rom::MANUFACTURER_SPARKLE_CRYSTAL:       return 24;
        case GW_rom::MANUFACTURER_STADLBAUER:            return 25;
        case GW_rom::MANUFACTURER_VGS:                   return 26;
        case GW_rom::MANUFACTURER_UNBRANDED:             return 27;
        case GW_rom::MANUFACTURER_DAVID_AND_JOHN:        return 28;
        case GW_rom::MANUFACTURER_BURGER_BARD:           return 29;
        default:                                  return 100 + (int)mfr;
    }
}

// Every manufacturer except Nintendo, Tronica and Elektronika (the original,
// long-tested library) is hidden until "Experimental Games" is turned on.
bool is_experimental_games_manufacturer(uint8_t mfr) {
    switch (mfr) {
        case GW_rom::MANUFACTURER_TIGER:
        case GW_rom::MANUFACTURER_KONAMI:
        case GW_rom::MANUFACTURER_NELSONIC:
        case GW_rom::MANUFACTURER_VTECH:
        case GW_rom::MANUFACTURER_BANDAI:
        case GW_rom::MANUFACTURER_EPOCH:
        case GW_rom::MANUFACTURER_NIKKO:
        case GW_rom::MANUFACTURER_TANDY:
        case GW_rom::MANUFACTURER_POPY:
        case GW_rom::MANUFACTURER_GAKKEN:
        case GW_rom::MANUFACTURER_SPICA:
        case GW_rom::MANUFACTURER_APOLLO:
        case GW_rom::MANUFACTURER_BRICK_GAME:
        case GW_rom::MANUFACTURER_HIRO:
        case GW_rom::MANUFACTURER_NATURE_LAND:
        case GW_rom::MANUFACTURER_SPARKLE_CRYSTAL:
        case GW_rom::MANUFACTURER_GAMETECH:
        case GW_rom::MANUFACTURER_COMPILE:
        case GW_rom::MANUFACTURER_VGS:
        case GW_rom::MANUFACTURER_COOL_TEC:
        case GW_rom::MANUFACTURER_HARTUNG:
        case GW_rom::MANUFACTURER_EDLA:
        case GW_rom::MANUFACTURER_STADLBAUER:
        case GW_rom::MANUFACTURER_UNBRANDED:
        case GW_rom::MANUFACTURER_DAVID_AND_JOHN:
        case GW_rom::MANUFACTURER_MCDONALDS:
        case GW_rom::MANUFACTURER_BURGER_BARD:
            return true;
        default:
            return false;
    }
}

// g_debug_mode force-shows experimental games the same way it force-enables
// logging (see its doc comment in switch_settings.h) - without touching the
// stored g_experimental_games_enabled value either.
bool experimental_games_visible() {
    return g_experimental_games_enabled || g_debug_mode;
}

// See g_clone_filter_mode's doc comment in switch_settings.h for what these
// two tiers mean and why they're separate, hand-maintained lists rather than
// something derived from the pack at runtime (GNW_LIST.md's "Clone Of"
// column never reaches the runtime pack at all - it's purely a build-time
// concept in CONVERT_ROM's Python pipeline, used only to fall back to a
// parent game's ROM/artwork files when a clone doesn't have its own).
//
// kKnownDuplicateRefs - the entries GNW_LIST.md's own comments call out as
// genuinely the *same* release cataloged twice, kept as separate pack
// entries only because of a documented minor hardware/behavior difference:
//   CN-07 vs CN-17 Helmet, IP-05 vs IP-15 Judge, CM-72A vs CM-72 Mario's
//   Cement Factory.
constexpr const char* kKnownDuplicateRefs[] = {
    "CN_07", "IP_05", "CM_72A",
    // LCDonald "International" entries - same US-vs-international
    // box art tier as the three above, see standard_controls.md's McDonald's
    // section and GNW_LIST.md's LCDonald notes.
    "KSOCCER_INTL", "SGRINDER_INTL", "SSPEEDWAY_INTL", "SXTREMEBOARD_INTL",
    // Silver-body "Search for the Silver Game" contest unit - box art only
    // (shockey.lay's "Front (Rare)"/"Back (Rare)" views), same tier as the
    // International entries above.
    "SHOCKEY_RARE",
    // Digital Monster Ver.1 Japan's other ROM revision - same release.
    "DIGIMONJA",
};

// kKnownCloneRefs - every entry GNW_LIST.md's "Clone Of" column marks as a
// clone of another entry - a strict superset of kKnownDuplicateRefs above.
// Most of these are legitimately distinct, differently-titled games
// (Elektronika bootlegs, Tiger titles reusing another game's ports) that
// just happen to reuse another entry's hardware/ports in the build
// pipeline, not simple re-releases of the same game - hiding all of them
// is a much bigger hammer than kKnownDuplicateRefs.
constexpr const char* kKnownCloneRefs[] = {
    "BF_107", "DR_106", "MK_96", "EG_26", "CN_07", "IP_05", "CM_72A", "YM_105",
    "TG_18", "CC_38V", "SG_21", "FR_23",
    "IM_53", "IM_19", "IM_11", "IM_10", "IM_09", "IM_50", "IM_32", "IM_33",
    "IM_51", "IM_49", "IM_02", "IM_16", "IM_13", "IM_03", "IM_04", "IM_22", "IM_12",
    "TSUPERMAN", "TROBHOOD", "TROCKTEER",
    // LCDonald entries - both tiers, since this list is a superset of
    // kKnownDuplicateRefs (International + Silver Game box-art variants)
    // plus genuine clones: the ESPN25 cross-promotion editions (same game/
    // segments/logic as their Sonic-branded counterpart - or, for
    // KBASEBALL_ESPN, the actual origin; see below), and the 5 base games
    // that turned out to be re-skins of an earlier, differently-named
    // McDonald's game once real release dates were confirmed against
    // McDonald's own press releases (see GNW_LIST.md's LCDonald notes for
    // the full research and per-game dates):
    //   TSOCCER clones KSOCCER, CFLOWER/CFLOWER2 clone ABANANA,
    //   SSKATEBOARD clones SXTREMEBOARD, SBASKETBALL clones SHOCKEY,
    //   ARTENNIS clones ARVOLLEYBALL.
    "KSOCCER_INTL", "SGRINDER_INTL", "SSPEEDWAY_INTL", "SXTREMEBOARD_INTL", "SHOCKEY_RARE",
    "SSPEEDWAY_ESPN", "TSOCCER_ESPN", "SSKATEBOARD_ESPN",
    "SBASKETBALL_ESPN", "ARTENNIS_ESPN", "DIGIMONJA",
    "TSOCCER", "CFLOWER", "CFLOWER2", "SSKATEBOARD", "SBASKETBALL", "ARTENNIS",
    // KBASEBALL_ESPN ("ESPN25: Michael Vick - Football", 2004-04-30) is the
    // real origin here - Knuckles Baseball itself didn't release until
    // 2005-03-18, so KBASEBALL is the clone, not KBASEBALL_ESPN (the only
    // ESPN25 entry in this file that isn't itself the clone tier).
    "KBASEBALL",
    // Burger Bard (LCDonald's white-label build) - every entry is a re-skin
    // of a McDonald's game (or, for the *2 entries, of another Burger game's
    // art), so all 19 are clones of their ultimate-origin McDonald's game -
    // see GNW_LIST.md's Burger Bard notes. Under "Hide All Clones" the whole
    // Burger Bard group therefore disappears.
    "BB_VOLLEYBALL", "BB_FISHING", "BB_EGGCATCH", "BB_HEARTMOON", "BB_FOOTBALL",
    "BB_TREASURE", "BB_TREASURE2", "BB_PLATFORM", "BB_PLATFORM2", "BB_WBASKETBALL",
    "BB_RAILGRINDER", "BB_MAGICBATTLE", "BB_SKATEBOARD", "BB_SPEEDWAY",
    "BB_EXTREMEBOARD", "BB_TARGETSHOOT", "BB_SEASHELL", "BB_KOSOCCER", "BB_EGGCATCH2",
};

bool ref_in_list(const std::string& ref, const char* const* list, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (ref == list[i]) return true;
    }
    return false;
}

// True when `ref` should be hidden under `mode` - ShowAll never hides
// anything, HideDuplicates checks the small curated list, HideAllClones
// checks the full list (which already contains HideDuplicates' 3 entries,
// so no need to check both).
bool clone_hidden_under(const std::string& ref, CloneFilterMode mode) {
    switch (mode) {
        case CloneFilterMode::HideDuplicates:
            return ref_in_list(ref, kKnownDuplicateRefs, sizeof(kKnownDuplicateRefs) / sizeof(kKnownDuplicateRefs[0]));
        case CloneFilterMode::HideAllClones:
            return ref_in_list(ref, kKnownCloneRefs, sizeof(kKnownCloneRefs) / sizeof(kKnownCloneRefs[0]));
        default:
            return false;
    }
}

const char* clone_filter_label(CloneFilterMode m) {
    switch (m) {
        case CloneFilterMode::HideDuplicates: return "Hide Duplicates";
        case CloneFilterMode::HideAllClones:  return "Hide All Clones";
        default:                               return "Show All";
    }
}

CloneFilterMode cycle_clone_filter(CloneFilterMode cur, int dir) {
    int i = (int)cur;
    i = (i + dir) % 3;
    if (i < 0) i += 3;
    return (CloneFilterMode)i;
}

// See g_time_set_mode's doc comment in switch_settings.h.
const char* time_set_mode_label(TimeSetMode m) {
    return (m == TimeSetMode::Manual) ? "Manual" : "Automatic";
}

// See g_pet_time_mode / g_pet_time_display in switch_settings.h.
const char* pet_time_mode_label(PetTimeMode m) {
    switch (m) {
        case PetTimeMode::ClockSync: return "Clock Sync";
        case PetTimeMode::Paused:    return "Paused";
        default:                     return "Real Time";
    }
}

PetTimeMode cycle_pet_time_mode(PetTimeMode cur, int dir) {
    int i = ((int)cur + dir) % 3;
    if (i < 0) i += 3;
    return (PetTimeMode)i;
}

const char* pet_time_display_label(PetTimeDisplay d) {
    return (d == PetTimeDisplay::FastForward) ? "Fast-Forward" : "Progress Bar";
}

TimeSetMode cycle_time_set_mode(TimeSetMode cur, int dir) {
    int i = (int)cur;
    i = (i + dir) % 2;
    if (i < 0) i += 2;
    return (TimeSetMode)i;
}

const char* sort_mode_label(SortMode m) {
    switch (m) {
        case SortMode::Name: return "Sort by: Name";
        case SortMode::Date: return "Sort by: Date";
        case SortMode::MakerDate: return "Sort by: Maker (Date)";
        default: return "Sort by: Maker (Name)";
    }
}

// What Up/Down jumps by in the current sort mode - shown in the hint bar.
const char* sort_mode_group_label(SortMode m) {
    switch (m) {
        case SortMode::Name: return "Letter";
        case SortMode::Date: return "Year";
        default: return "Manufacturer";
    }
}

std::string name_of(uint8_t idx) {
    const GW_rom* g = load_game(idx);
    return g ? g->name : std::string();
}

std::string date_of(uint8_t idx) {
    const GW_rom* g = load_game(idx);
    return g ? g->date : std::string();
}

std::string ref_of(uint8_t idx) {
    const GW_rom* g = load_game(idx);
    return g ? g->ref : std::string();
}

bool name_less_ci(const std::string& a, const std::string& b) {
    const size_t n = std::min(a.size(), b.size());
    for (size_t i = 0; i < n; i++) {
        const unsigned char ca = (unsigned char)std::tolower((unsigned char)a[i]);
        const unsigned char cb = (unsigned char)std::tolower((unsigned char)b[i]);
        if (ca != cb) return ca < cb;
    }
    return a.size() < b.size();
}

// Dates are stored "YYYY-MM-DD" - lexicographic string compare already sorts
// them chronologically, this is just for the Up/Down year-jump grouping key.
int parse_year(const std::string& date) {
    if (date.size() < 4) return 0;
    int y = 0;
    for (int i = 0; i < 4; i++) {
        if (date[i] < '0' || date[i] > '9') return 0;
        y = y * 10 + (date[i] - '0');
    }
    return y;
}

// The value Up/Down groups by in the current sort mode - manufacturer rank,
// first letter (uppercased), or year. Same-key entries must be contiguous in
// g_sorted_order for select_next_other_group() below to work, which holds by
// construction since rebuild_sorted_order() sorts by exactly this key first.
int group_key(uint8_t idx) {
    switch (g_sort_mode) {
        case SortMode::Name: {
            const std::string n = name_of(idx);
            return n.empty() ? 0 : std::toupper((unsigned char)n[0]);
        }
        case SortMode::Date:
            return parse_year(date_of(idx));
        default:
            return mfr_rank(get_mfr(idx));
    }
}

// A game is experimental if its manufacturer is, or if it runs on one of the
// BrickEmuPy-derived cores (a BRK1 container ROM, source/BrickCores/) - those
// can sit in an otherwise-ungated group (e.g. Pocket Pikachu under Nintendo).
bool is_experimental_game(uint8_t idx) {
    const GW_rom* g = load_game(idx);
    if (!g) return false;
    return is_experimental_games_manufacturer(g->manufacturer) || BrickRom::is_container(g->rom, g->size_rom);
}

// Rebuilds g_sorted_order - the single list every bit of navigation, the
// bottom counter, and Up/Down group-jumping all walk (see select_next(),
// select_next_other_group(), position_of_selected() below). Filtering
// experimental games and duplicate/clone entries out here, rather
// than skipping them individually at each call site, is what makes them
// disappear from navigation *and* the visible count at the same time with
// no other code needing to know about it.
void rebuild_sorted_order() {
    const size_t n = get_nb_name();
    const bool show_experimental_games = experimental_games_visible();

    g_sorted_order.clear();
    g_sorted_order.reserve(n);
    for (size_t i = 0; i < n; i++) {
        if (!show_experimental_games && is_experimental_game((uint8_t)i)) {
            continue;
        }
        if (clone_hidden_under(ref_of((uint8_t)i), g_clone_filter_mode)) {
            continue;
        }
        g_sorted_order.push_back((uint8_t)i);
    }

    // Date order everywhere: ISO date strings sort chronologically as text,
    // ties broken by name so equal-date games always land in a stable,
    // predictable order.
    auto date_then_name = [](uint8_t a, uint8_t b) {
        const std::string da = date_of(a), db = date_of(b);
        if (da != db) return da < db;
        return name_less_ci(name_of(a), name_of(b));
    };

    switch (g_sort_mode) {
        case SortMode::Name:
            std::stable_sort(g_sorted_order.begin(), g_sorted_order.end(), [](uint8_t a, uint8_t b) {
                return name_less_ci(name_of(a), name_of(b));
            });
            break;
        case SortMode::Date:
            std::stable_sort(g_sorted_order.begin(), g_sorted_order.end(), date_then_name);
            break;
        case SortMode::MakerDate:
            std::stable_sort(g_sorted_order.begin(), g_sorted_order.end(), [&](uint8_t a, uint8_t b) {
                const int ra = mfr_rank(get_mfr(a)), rb = mfr_rank(get_mfr(b));
                if (ra != rb) return ra < rb;
                return date_then_name(a, b);
            });
            break;
        default: // MakerName
            std::stable_sort(g_sorted_order.begin(), g_sorted_order.end(), [](uint8_t a, uint8_t b) {
                const int ra = mfr_rank(get_mfr(a)), rb = mfr_rank(get_mfr(b));
                if (ra != rb) return ra < rb;
                return name_less_ci(name_of(a), name_of(b));
            });
            break;
    }

    // g_selected_index can point at a game that just got filtered out (experimental
    // games toggled off while one was selected) - land on whatever's first
    // in the new order instead of leaving it dangling on something hidden.
    bool selected_visible = false;
    for (uint8_t idx : g_sorted_order) {
        if (idx == g_selected_index) {
            selected_visible = true;
            break;
        }
    }
    if (!selected_visible && !g_sorted_order.empty()) {
        g_selected_index = g_sorted_order[0];
    }
}

void set_selected(uint8_t idx) {
    g_selected_index = idx;
}

uint8_t wrap_index(int i, int n) {
    while (i < 0) i += n;
    i = i % n;
    return (uint8_t)i;
}

int position_of_selected() {
    for (size_t i = 0; i < g_sorted_order.size(); i++) {
        if (g_sorted_order[i] == g_selected_index) return (int)i;
    }
    return 0;
}

// Steps through the current sort order, wrapping - manufacturer boundaries
// (or any other grouping) don't stop this, only select_next_other_group() cares.
void select_next(int dir) {
    const int n = (int)g_sorted_order.size();
    if (n <= 0) return;
    const int pos = wrap_index(position_of_selected() + dir, n);
    set_selected(g_sorted_order[pos]);
}

// Jumps to the first game (in sort order) of the next/prev group - "group"
// meaning manufacturer, first letter, or year depending on g_sort_mode.
// Two phases: find which group is next in the requested direction, then land
// on that group's first member in forward list order regardless of which
// direction we searched - so this always means "first of that group", not
// "last of it", even when jumping backwards.
void select_next_other_group(int dir) {
    const int n = (int)g_sorted_order.size();
    if (n <= 0) return;

    const int pos = position_of_selected();
    const int cur_key = group_key(g_sorted_order[pos]);

    int new_key = cur_key;
    bool found = false;
    for (int step = 1; step <= n; step++) {
        const int cand_pos = wrap_index(pos + dir * step, n);
        const int k = group_key(g_sorted_order[cand_pos]);
        if (k != cur_key) {
            new_key = k;
            found = true;
            break;
        }
    }
    if (!found) return; // only one group total

    for (int i = 0; i < n; i++) {
        if (group_key(g_sorted_order[i]) == new_key) {
            set_selected(g_sorted_order[i]);
            return;
        }
    }
}

const char* settings_row_label(SettingsRow row) {
    switch (row) {
        case SettingsRow::Rotation:              return "Rotation";
        case SettingsRow::Background:             return "Game Background Color";
        case SettingsRow::BackgroundCustomEntry:  return "Enter Custom Color";
        case SettingsRow::SegmentColor:            return "Segment Color";
        case SettingsRow::SegmentColorCustomEntry: return "Enter Custom Color";
        case SettingsRow::Segments:                return "Segment Marking";
        case SettingsRow::SegmentsCustomEntry:     return "Enter Custom Value";
        case SettingsRow::BackgroundArt:            return "Background Art";
        case SettingsRow::ConsoleArt:               return "Display Single Screen Console Art";
        case SettingsRow::PanelGap:                 return "Panel Gap";
        case SettingsRow::PanelGapCustomEntry:      return "Enter Custom Value";
        case SettingsRow::LoadStateBehavior:        return "Load State Behavior";
        case SettingsRow::CloneFilter:               return "Duplicate & Clone Games";
        case SettingsRow::TimeSetMode:                return "Time Set Mode";
        case SettingsRow::PetTime:                    return "Pet Time";
        case SettingsRow::PetTimeCap:                 return "Catch-Up Limit";
        case SettingsRow::PetTimeCapCustomEntry:      return "Enter Custom Hours";
        case SettingsRow::PetTimeDisplay:             return "Catch-Up Display";
        case SettingsRow::LogFile:                   return "Log File";
        case SettingsRow::ExperimentalGames:                  return "Experimental Games";
        case SettingsRow::ExperimentalFeatures:                return "Experimental Features";
        case SettingsRow::DosimeterReading:            return "Dosimeter Reading";
        case SettingsRow::ReturnToMenu:                return "Return to Main Menu";
    }
    return "";
}

// Only called for non-action rows (render_settings_rows() below skips this
// for action rows, which have no value column at all).
std::string settings_row_value(SettingsRow row) {
    switch (row) {
        case SettingsRow::Rotation:
            return rotation_label(g_screen_rotation);
        case SettingsRow::Background:
            return g_bg_custom_selected
                ? (g_bg_custom_text.empty() ? std::string("Custom") : ("Custom: #" + g_bg_custom_text))
                : bg_color_display(g_settings.background_color);
        case SettingsRow::SegmentColor:
            return g_segcolor_custom_selected
                ? (g_segcolor_custom_text.empty() ? std::string("Custom") : ("Custom: #" + g_segcolor_custom_text))
                : segcolor_display(g_segment_color);
        case SettingsRow::Segments:
            return g_seg_custom_selected
                ? (g_seg_custom_text.empty() ? std::string("Custom") : ("Custom: " + g_seg_custom_text))
                : seg_display(g_settings.segment_marking_alpha);
        case SettingsRow::BackgroundArt:
            return g_hide_background_art ? "Hidden" : "Shown";
        case SettingsRow::ConsoleArt:
            return g_hide_console_art ? "Hidden" : "Shown";
        case SettingsRow::PanelGap:
            return g_panelgap_custom_selected
                ? (g_panelgap_custom_text.empty() ? std::string("Custom") : ("Custom: " + g_panelgap_custom_text + "%"))
                : panelgap_display(g_panel_gap_percent);
        case SettingsRow::LoadStateBehavior:
            return load_state_behavior_label(g_load_state_behavior);
        case SettingsRow::CloneFilter:
            return clone_filter_label(g_clone_filter_mode);
        case SettingsRow::TimeSetMode:
            return time_set_mode_label(g_time_set_mode);
        case SettingsRow::PetTime:
            return pet_time_mode_label(g_pet_time_mode);
        case SettingsRow::PetTimeCap:
            return g_pettimecap_custom_selected
                ? (g_pettimecap_custom_text.empty() ? std::string("Custom") : ("Custom: " + g_pettimecap_custom_text + "h"))
                : pettimecap_display(g_pet_time_cap_hours);
        case SettingsRow::PetTimeDisplay:
            return pet_time_display_label(g_pet_time_display);
        case SettingsRow::LogFile:
            return g_log_file_enabled ? "On" : "Off";
        case SettingsRow::ExperimentalGames:
            return g_experimental_games_enabled ? "On" : "Off";
        case SettingsRow::ExperimentalFeatures:
            return g_experimental_features_enabled ? "On" : "Off";
        case SettingsRow::DosimeterReading:
            return std::to_string((unsigned)g_dosimeter_reading);
        default:
            return "";
    }
}

// logical_w/logical_h are already rotation-swapped by the caller (see
// switch_menu_render()) - this just lays rows out against them like any
// other menu content.
//
// The row count varies at runtime (Custom-entry rows appear/disappear, and
// with Panel Gap + Segments + Panel Gap's own Custom slot all open at once
// the full list can run past what fits between the top and bottom bars) -
// rather than letting it overflow into/past the bottom bar, this scrolls a
// window of rows that follows the current selection. Short lists (the
// common case) render exactly as before, no indicators, no space lost.
void render_settings_rows(int logical_w, int logical_h) {
    const std::vector<SettingsRow> rows = build_settings_rows();
    const float label_size = 30.0f;
    const float row_h = 56.0f;
    // Trimmed from the original +48/-12 margins (which left room to spare)
    // to reclaim a bit more vertical budget for content rows now that
    // scrolling needs some of that space for its own indicators below.
    const float start_y = (float)kTopBarH + 20.0f;
    const float label_x = 56.0f;
    const float value_x = (float)logical_w * 0.6f;

    // Leaves a little breathing room above the bottom bar rather than
    // packing the last row flush against it.
    const float avail_h = (float)(logical_h - kBottomBarH) - start_y - 4.0f;
    int max_slots = (int)(avail_h / row_h);
    if (max_slots < 1) max_slots = 1;

    const int row_count = (int)rows.size();
    // Compact bands (not full row_h slots) reserved above/below the content
    // for "more above"/"more below" indicators. The top one needs to be
    // tall enough that its own text clears the *next* row's selection-highlight
    // rect (which starts 8px above that row's own y) - too little clearance
    // here is exactly what made it collide with the first content row
    // before. The bottom one has no such neighbor to clear, so it can stay
    // noticeably smaller.
    const float indicator_h_top = 40.0f;
    const float indicator_h_bottom = 22.0f;
    const bool scrolling = row_count > max_slots;
    const int content_slots = scrolling
        ? std::max(1, (int)((avail_h - indicator_h_top - indicator_h_bottom) / row_h))
        : max_slots;

    int scroll_top = 0;
    if (scrolling) {
        const int max_scroll_top = row_count - content_slots;
        scroll_top = std::max(0, std::min(g_settings_selected_row - content_slots / 2, max_scroll_top));
        if (g_settings_selected_row < scroll_top) scroll_top = g_settings_selected_row;
        if (g_settings_selected_row >= scroll_top + content_slots) scroll_top = g_settings_selected_row - content_slots + 1;
    }
    const int visible_count = std::min(content_slots, row_count - scroll_top);
    const float content_start_y = scrolling ? start_y + indicator_h_top : start_y;

    if (scrolling) {
        if (scroll_top > 0) {
            switch_ui_draw_text(label_x, start_y + 4.0f, 18.0f, kTextR, kTextG, kTextB, 0.6f, "^ more above");
        }
        if (scroll_top + visible_count < row_count) {
            switch_ui_draw_text(label_x, content_start_y + (float)content_slots * row_h + 4.0f, 18.0f, kTextR, kTextG, kTextB, 0.6f, "v more below");
        }
    }

    for (int vi = 0; vi < visible_count; vi++) {
        const int i = scroll_top + vi;
        const SettingsRow row = rows[i];
        const float y = content_start_y + (float)vi * row_h;
        const bool sel = (i == g_settings_selected_row);
        const bool action_row = settings_row_is_action(row);
        // Hidden debug mode locks every settings *value* row except Rotation -
        // see g_debug_mode's doc comment in switch_settings.h. "Return to
        // Main Menu" is a navigation action, not a setting, so it's excluded
        // too - it isn't locked by update_settings_menu() either. The three
        // Custom-entry rows are locked like any other value row, but in
        // practice can never even be reached under debug mode: Background/
        // Segments/Panel Gap (their parent rows) are themselves locked, so
        // Left/Right can never select the Custom slot that would make these
        // appear.
        const bool locked = g_debug_mode && row != SettingsRow::Rotation && row != SettingsRow::ReturnToMenu;
        // The three Custom-entry rows read as nested under their parent -
        // indent the label only, keep the selection highlight full-width.
        const bool nested = (row == SettingsRow::BackgroundCustomEntry) || (row == SettingsRow::SegmentColorCustomEntry)
            || (row == SettingsRow::SegmentsCustomEntry)
            || (row == SettingsRow::PanelGapCustomEntry)
            || (row == SettingsRow::PetTimeCapCustomEntry);
        const float indent = nested ? 24.0f : 0.0f;

        if (sel) {
            switch_ui_draw_rect(label_x - 20.0f, y - 8.0f, (float)logical_w - (label_x - 20.0f) * 2.0f, label_size + 16.0f,
                                 kHighlightR, kHighlightG, kHighlightB, 0.15f);
        }

        const char* label = settings_row_label(row);
        // The longer labels (e.g. "Display Single Screen Console Art") run
        // into the value column at 90/270 (logical_w is portrait-narrow
        // there) - scale this row's label down to fit rather than overlap.
        float this_label_size = label_size;
        const float max_label_w = value_x - (label_x + indent) - 20.0f;
        const float label_w = switch_ui_text_width(label, this_label_size);
        if (label_w > max_label_w && max_label_w > 10.0f) {
            this_label_size = std::max(14.0f, label_size * (max_label_w / label_w));
        }
        const float label_alpha = locked ? 0.4f : (sel ? 1.0f : 0.75f);
        switch_ui_draw_text(label_x + indent, y, this_label_size, kTextR, kTextG, kTextB, label_alpha, label);

        if (action_row) {
            continue;
        }

        std::string value = settings_row_value(row);
        // Locked rows skip the < > cycle-arrow decoration too - nothing here
        // can actually be changed with Left/Right while debug mode holds.
        if (sel && !locked) value = "< " + value + " >";

        // Same shrink-to-fit as the label above - long values (a full
        // "< Custom: #RRGGBB >" hex string, especially at 90/270 where
        // logical_w is portrait-narrow) would otherwise run off the right
        // edge instead of wrapping or scrolling.
        float this_value_size = label_size;
        const float max_value_w = (float)logical_w - value_x - 20.0f;
        const float value_w = switch_ui_text_width(value, this_value_size);
        if (value_w > max_value_w && max_value_w > 10.0f) {
            this_value_size = std::max(14.0f, label_size * (max_value_w / value_w));
        }
        switch_ui_draw_text(value_x, y, this_value_size, kHighlightR, kHighlightG, kHighlightB, locked ? 0.4f : 1.0f, value);
    }
}

// logical_w/logical_h are already rotation-swapped by the caller, same as
// render_settings_rows() above.
void render_load_prompt(int logical_w, int logical_h) {
    const GW_rom* g = load_game(g_selected_index);
    const std::string name = g ? g->name : std::string("?");
    const float cx = (float)logical_w * 0.5f;
    const float cy = (float)logical_h * 0.5f;

    const std::string title = "Load Save State?";
    const float title_size = 36.0f;
    const float title_w = switch_ui_text_width(title, title_size, /*bold=*/true);
    const float title_y = cy - 110.0f;
    switch_ui_draw_text(cx - title_w * 0.5f, title_y, title_size, kTextR, kTextG, kTextB, 1.0f, title, /*bold=*/true);

    const float name_size = 24.0f;
    const float name_w = switch_ui_text_width(name, name_size);
    switch_ui_draw_text(cx - name_w * 0.5f, title_y + 54.0f, name_size, kHighlightR, kHighlightG, kHighlightB, 0.9f, name);

    const float opt_size = 28.0f;
    const float icon_pad = 10.0f;
    const float opt_y1 = title_y + 130.0f;
    const float opt_y2 = opt_y1 + 50.0f;
    const float opt_y3 = opt_y2 + 50.0f;

    auto draw_option = [&](float y, SwitchButtonIcon icon, const std::string& text) {
        const float w = switch_ui_button_icon_width(opt_size, icon) + icon_pad + switch_ui_text_width(text, opt_size);
        float x = cx - w * 0.5f;
        switch_ui_draw_button_icon(x, y, opt_size, kTextR, kTextG, kTextB, 1.0f, icon);
        x += switch_ui_button_icon_width(opt_size, icon) + icon_pad;
        switch_ui_draw_text(x, y, opt_size, kTextR, kTextG, kTextB, 1.0f, text);
    };

    draw_option(opt_y1, SwitchButtonIcon::A, "Load Save");
    draw_option(opt_y2, SwitchButtonIcon::B, "Start Fresh");
    draw_option(opt_y3, SwitchButtonIcon::Minus, "Back to Game Select");
}

// logical_w/logical_h are already rotation-swapped by the caller, same as
// render_settings_rows()/render_load_prompt() above. switch_ui has no text
// wrapping, so the warning is pre-split into fixed lines sized to fit the
// menu's usual content width rather than measured/wrapped at draw time.
// Both enable-confirmation screens (Experimental Games / Experimental
// Features) share this layout and only differ in their warning text.
void render_enable_confirm(int logical_w, int logical_h, const char* const* lines, int num_lines) {
    const float cx = (float)logical_w * 0.5f;
    const float line_size = 26.0f;
    const float line_gap = 10.0f;
    float y = (float)logical_h * 0.5f - 150.0f;

    for (int i = 0; i < num_lines; i++) {
        const float w = switch_ui_text_width(lines[i], line_size);
        switch_ui_draw_text(cx - w * 0.5f, y, line_size, kTextR, kTextG, kTextB, 1.0f, lines[i]);
        y += line_size + line_gap;
    }

    const float opt_size = 28.0f;
    const float icon_pad = 10.0f;
    const float opt_y1 = y + 30.0f;
    const float opt_y2 = opt_y1 + 50.0f;

    auto draw_option = [&](float oy, SwitchButtonIcon icon, const std::string& text) {
        const float w = switch_ui_button_icon_width(opt_size, icon) + icon_pad + switch_ui_text_width(text, opt_size);
        float x = cx - w * 0.5f;
        switch_ui_draw_button_icon(x, oy, opt_size, kTextR, kTextG, kTextB, 1.0f, icon);
        x += switch_ui_button_icon_width(opt_size, icon) + icon_pad;
        switch_ui_draw_text(x, oy, opt_size, kTextR, kTextG, kTextB, 1.0f, text);
    };

    draw_option(opt_y1, SwitchButtonIcon::A, "Enable");
    draw_option(opt_y2, SwitchButtonIcon::B, "Cancel");
}

void render_experimental_games_confirm(int logical_w, int logical_h) {
    static const char* const kLines[] = {
        "WARNING: These games are experimental",
        "and largely untested. You may experience",
        "unexpected issues while playing them.",
        "Do you still wish to enable?",
    };
    render_enable_confirm(logical_w, logical_h, kLines, 4);
}

void render_experimental_features_confirm(int logical_w, int logical_h) {
    static const char* const kLines[] = {
        "WARNING: These features are experimental",
        "and largely untested. You may experience",
        "unexpected issues while using them.",
        "Do you still wish to enable?",
    };
    render_enable_confirm(logical_w, logical_h, kLines, 4);
}

} // namespace

void switch_menu_init() {
    switch_ui_init();
    load_header_icon_if_needed();
    g_selected_index = switch_get_default_game_index();
    rebuild_sorted_order();

    // A loaded value that doesn't match any preset can only have gotten
    // there via a custom entry in an earlier session - start the row on the
    // Custom slot (and pre-fill the entry text) so it reads accurately and
    // Left/Right doesn't have to cycle all the way around to get back to it.
    g_bg_custom_selected = !bg_matches_any_preset(g_settings.background_color);
    if (g_bg_custom_selected) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%06X", (unsigned)(g_settings.background_color & 0xFFFFFFu));
        g_bg_custom_text = buf;
    }
    g_segcolor_custom_selected = !segcolor_matches_any_preset(g_segment_color);
    if (g_segcolor_custom_selected) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%06X", (unsigned)(g_segment_color & 0xFFFFFFu));
        g_segcolor_custom_text = buf;
    }
    g_seg_custom_selected = !seg_matches_any_preset(g_settings.segment_marking_alpha);
    if (g_seg_custom_selected) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%u", (unsigned)g_settings.segment_marking_alpha);
        g_seg_custom_text = buf;
    }
    g_panelgap_custom_selected = !panelgap_matches_any_preset(g_panel_gap_percent);
    if (g_panelgap_custom_selected) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%u", (unsigned)g_panel_gap_percent);
        g_panelgap_custom_text = buf;
    }
    g_pettimecap_custom_selected = !pettimecap_matches_any_preset(g_pet_time_cap_hours);
    if (g_pettimecap_custom_selected) {
        g_pettimecap_custom_text = std::to_string((unsigned)g_pet_time_cap_hours);
    }

    g_menu_active = true;
}

void switch_menu_shutdown() {
    if (g_thumb_tex) glDeleteTextures(1, &g_thumb_tex);
    g_thumb_tex = 0;
    if (g_header_icon_tex) glDeleteTextures(1, &g_header_icon_tex);
    g_header_icon_tex = 0;
    switch_ui_shutdown();
}

bool switch_menu_is_active() {
    return g_menu_active;
}

static void update_load_prompt(uint64_t kDown) {
    if (kDown & HidNpadButton_A) {
        YOKOI_LOG("switch_menu: load-save-state prompt confirmed for index %u", (unsigned)g_selected_index);
        switch_load_game_by_index_and_init(g_selected_index);
        if (load_game_state(g_cpu.get(), g_selected_index)) {
            pet_time_on_state_loaded();  // time away since that save - see pet_time.h
        }
        g_load_prompt_active = false;
        g_menu_active = false;
    } else if (kDown & HidNpadButton_B) {
        YOKOI_LOG("switch_menu: load-save-state prompt declined for index %u, starting fresh", (unsigned)g_selected_index);
        switch_load_game_by_index_and_init(g_selected_index);
        g_load_prompt_active = false;
        g_menu_active = false;
    } else if (kDown & HidNpadButton_Minus) {
        // Backs out of the prompt entirely - neither loads the save nor
        // starts fresh, just returns to the game list (g_menu_active was
        // never cleared getting here, so there's nothing else to restore).
        YOKOI_LOG("switch_menu: load-save-state prompt cancelled for index %u, returning to game select", (unsigned)g_selected_index);
        g_load_prompt_active = false;
    }
}

static void update_experimental_games_confirm(uint64_t kDown) {
    if (kDown & HidNpadButton_A) {
        YOKOI_LOG("switch_menu: experimental games enable confirmed");
        g_experimental_games_enabled = true;
        switch_settings_save();
        g_experimental_games_confirm_active = false;
        rebuild_sorted_order(); // newly-visible games need to enter the navigable set
    } else if (kDown & HidNpadButton_B) {
        YOKOI_LOG("switch_menu: experimental games enable cancelled");
        g_experimental_games_confirm_active = false;
    }
}

// Mirrors update_experimental_games_confirm() above - except there's no
// game list to rebuild: Experimental Features only gates features, never games.
static void update_experimental_features_confirm(uint64_t kDown) {
    if (kDown & HidNpadButton_A) {
        YOKOI_LOG("switch_menu: experimental features enable confirmed");
        g_experimental_features_enabled = true;
        // Turning it on reveals the Pet Time rows above this one - keep the
        // cursor on this row rather than on whatever slid into its index.
        const std::vector<SettingsRow> rows = build_settings_rows();
        for (size_t i = 0; i < rows.size(); i++)
            if (rows[i] == SettingsRow::ExperimentalFeatures) g_settings_selected_row = (int)i;
        switch_settings_save();
        g_experimental_features_confirm_active = false;
    } else if (kDown & HidNpadButton_B) {
        YOKOI_LOG("switch_menu: experimental features enable cancelled");
        g_experimental_features_confirm_active = false;
    }
}

static void update_settings_menu(uint64_t kDown) {
    const std::vector<SettingsRow> rows = build_settings_rows();
    const int row_count = (int)rows.size();
    if (g_settings_selected_row >= row_count) {
        // The row list can shrink out from under the current selection - e.g.
        // Left/Right just left Custom mode on a row above the one selected,
        // dropping its now-gone Custom-entry row. Land on the new last row
        // rather than reading out of bounds.
        g_settings_selected_row = row_count - 1;
    }

    if (kDown & HidNpadButton_Down) {
        g_settings_selected_row = (g_settings_selected_row + 1) % row_count;
    }
    if (kDown & HidNpadButton_Up) {
        g_settings_selected_row = (g_settings_selected_row - 1 + row_count) % row_count;
    }

    const SettingsRow row = rows[g_settings_selected_row];
    const bool is_action = settings_row_is_action(row);
    // Hidden debug mode locks every row except Rotation - see g_debug_mode's
    // doc comment in switch_settings.h. switch_settings_save() below also
    // no-ops entirely under debug mode, so even Rotation - the one row still
    // adjustable here - only ever changes in memory, never on disk.
    // "Return to Main Menu" is a navigation action, not a setting, so it's
    // exempted here the same way it's exempted from the dimming in
    // render_settings_rows().
    const bool row_locked = g_debug_mode && row != SettingsRow::Rotation && row != SettingsRow::ReturnToMenu;

    if (!is_action && !row_locked && (kDown & (HidNpadButton_Right | HidNpadButton_Left))) {
        const int dir = (kDown & HidNpadButton_Right) ? 1 : -1;
        switch (row) {
            case SettingsRow::Rotation:
                g_screen_rotation = cycle_rotation(g_screen_rotation, dir);
                break;
            case SettingsRow::Background: {
                // One extra slot past the presets for Custom - cycling onto
                // it doesn't open the keyboard by itself, it just reveals the
                // BackgroundCustomEntry row below (see build_settings_rows())
                // for that.
                int slot = g_bg_custom_selected ? kNumBgPresets : find_bg_preset_index(g_settings.background_color);
                slot = (slot + dir + (kNumBgPresets + 1)) % (kNumBgPresets + 1);
                if (slot == kNumBgPresets) {
                    g_bg_custom_selected = true;
                    // No value typed yet this session - default to Light
                    // Yellow per explicit spec until one is. A remembered
                    // value from earlier this session is re-applied as-is.
                    g_settings.background_color = g_bg_custom_text.empty()
                        ? BACKGROUND_PRESETS[0].color
                        : (uint32_t)strtoul(g_bg_custom_text.c_str(), nullptr, 16);
                } else {
                    g_bg_custom_selected = false;
                    g_settings.background_color = kBgPresets[slot].color;
                }
                break;
            }
            case SettingsRow::SegmentColor: {
                // Same Custom-slot flow as Background above; no value typed
                // yet this session means Auto until one is.
                int slot = g_segcolor_custom_selected ? kNumSegColorPresets : find_segcolor_preset_index(g_segment_color);
                slot = (slot + dir + (kNumSegColorPresets + 1)) % (kNumSegColorPresets + 1);
                if (slot == kNumSegColorPresets) {
                    g_segcolor_custom_selected = true;
                    g_segment_color = g_segcolor_custom_text.empty()
                        ? kSegmentColorAuto
                        : (uint32_t)strtoul(g_segcolor_custom_text.c_str(), nullptr, 16);
                } else {
                    g_segcolor_custom_selected = false;
                    g_segment_color = kSegColorPresets[slot].color;
                }
                break;
            }
            case SettingsRow::Segments: {
                int slot = g_seg_custom_selected ? 3 : find_seg_preset_index(g_settings.segment_marking_alpha);
                slot = (slot + dir + 4) % 4;
                if (slot == 3) {
                    g_seg_custom_selected = true;
                    // Defaults to the Light preset's alpha until a value is
                    // typed, per explicit spec.
                    g_settings.segment_marking_alpha = g_seg_custom_text.empty()
                        ? kSegPresetAlpha[1]
                        : (uint8_t)std::clamp((int)strtol(g_seg_custom_text.c_str(), nullptr, 10), 0, 255);
                } else {
                    g_seg_custom_selected = false;
                    g_settings.segment_marking_alpha = kSegPresetAlpha[slot];
                }
                break;
            }
            case SettingsRow::BackgroundArt:
                g_hide_background_art = !g_hide_background_art;
                break;
            case SettingsRow::ConsoleArt:
                g_hide_console_art = !g_hide_console_art;
                break;
            case SettingsRow::PanelGap: {
                int slot = g_panelgap_custom_selected ? 3 : find_panelgap_preset_index(g_panel_gap_percent);
                slot = (slot + dir + 4) % 4;
                if (slot == 3) {
                    g_panelgap_custom_selected = true;
                    // Defaults to the Small preset's percentage until a
                    // value is typed, per the same spec as Background/Segments.
                    g_panel_gap_percent = g_panelgap_custom_text.empty()
                        ? kPanelGapPresetPercent[1]
                        : (uint8_t)std::clamp((int)strtol(g_panelgap_custom_text.c_str(), nullptr, 10), 0, 30);
                } else {
                    g_panelgap_custom_selected = false;
                    g_panel_gap_percent = kPanelGapPresetPercent[slot];
                }
                // Unlike Rotation/Background/Segments (read fresh every
                // frame by the renderer, no cached geometry), the gap is
                // baked into g_top_off_x/g_bottom_off_y/g_combined_canvas_*
                // by rebuild_layout_from_game() - re-run it now so this
                // applies immediately instead of only on the next game
                // load. Safe to call whether or not a game is currently
                // loaded (no-ops instantly if g_game is null - see its own
                // doc comment in switch_layout.cpp).
                rebuild_layout_from_game();
                break;
            }
            case SettingsRow::LoadStateBehavior:
                g_load_state_behavior = cycle_load_state_behavior(g_load_state_behavior, dir);
                break;
            case SettingsRow::CloneFilter: {
                g_clone_filter_mode = cycle_clone_filter(g_clone_filter_mode, dir);
                if (!g_last_game_played_ref.empty()) {
                    bool found = false;
                    const uint8_t idx = find_game_index_by_ref(g_last_game_played_ref, &found);
                    if (found && clone_hidden_under(ref_of(idx), g_clone_filter_mode)) {
                        // Same idea as ExperimentalGames below - the
                        // saved last-played game just became hidden under
                        // the new filter tier, so point lastgameplayed
                        // somewhere still visible instead of leaving a
                        // technically-invalid reference on disk.
                        g_last_game_played_ref = "AC_01";
                    }
                }
                rebuild_sorted_order();
                break;
            }
            case SettingsRow::TimeSetMode:
                g_time_set_mode = cycle_time_set_mode(g_time_set_mode, dir);
                break;
            case SettingsRow::PetTime:
                g_pet_time_mode = cycle_pet_time_mode(g_pet_time_mode, dir);
                break;
            case SettingsRow::PetTimeCap: {
                int slot = g_pettimecap_custom_selected ? kNumPetTimeCapPresets
                                                        : find_pettimecap_preset_index(g_pet_time_cap_hours);
                slot = (slot + dir + kNumPetTimeCapPresets + 1) % (kNumPetTimeCapPresets + 1);
                if (slot == kNumPetTimeCapPresets) {
                    g_pettimecap_custom_selected = true;
                    // The 72 Hours default until a value is typed, like the
                    // other Custom slots.
                    g_pet_time_cap_hours = g_pettimecap_custom_text.empty()
                        ? kPetTimeCapDefaultHours
                        : (uint32_t)std::clamp((int)strtol(g_pettimecap_custom_text.c_str(), nullptr, 10), 1, 9999);
                } else {
                    g_pettimecap_custom_selected = false;
                    g_pet_time_cap_hours = kPetTimeCapPresetHours[slot];
                }
                break;
            }
            case SettingsRow::PetTimeDisplay:
                g_pet_time_display = (g_pet_time_display == PetTimeDisplay::Progress) ? PetTimeDisplay::FastForward
                                                                                     : PetTimeDisplay::Progress;
                break;
            case SettingsRow::LogFile:
                g_log_file_enabled = !g_log_file_enabled;
                debug_log::set_enabled(g_log_file_enabled);
                break;
            case SettingsRow::ExperimentalGames:
                if (!g_experimental_games_enabled) {
                    // Turning ON needs confirmation first - don't flip yet,
                    // see update_experimental_games_confirm(). The trailing
                    // switch_settings_save() below still runs, harmlessly
                    // rewriting the same unchanged values (same as any other
                    // row's Left/Right press that happens not to change
                    // anything, e.g. Rotation already at 0°).
                    g_experimental_games_confirm_active = true;
                } else {
                    // Turning OFF is instant, no confirmation.
                    g_experimental_games_enabled = false;
                    if (!g_last_game_played_ref.empty()) {
                        bool found = false;
                        const uint8_t idx = find_game_index_by_ref(g_last_game_played_ref, &found);
                        if (found && is_experimental_game(idx)) {
                            // The saved last-played game just became hidden -
                            // point lastgameplayed at a known-safe game
                            // instead of leaving a technically-invalid
                            // reference on disk. Its save file is untouched;
                            // only this pointer moves, and only in memory
                            // until switch_settings_save() below writes it.
                            g_last_game_played_ref = "AC_01";
                        }
                    }
                    rebuild_sorted_order();
                }
                break;
            case SettingsRow::ExperimentalFeatures:
                if (!g_experimental_features_enabled) {
                    // Same flow as ExperimentalGames above - confirmation
                    // gates enabling, see update_experimental_features_confirm().
                    g_experimental_features_confirm_active = true;
                } else {
                    // Turning OFF is instant, no confirmation. Nothing to
                    // hide from the game list - features only.
                    g_experimental_features_enabled = false;
                }
                break;
            case SettingsRow::DosimeterReading:
                g_dosimeter_reading = (uint8_t)((g_dosimeter_reading + dir + 16) % 16);
                break;
            default:
                break;
        }
        switch_settings_save();
    }

    if (kDown & HidNpadButton_A) {
        if (row == SettingsRow::ReturnToMenu) {
            // g_menu_active was already true (that's why this screen is even
            // showing), so the normal game list is what renders next. Unlike
            // the old direct L+R+Plus-from-gameplay behavior, the game does
            // NOT stay loaded in the background from here - this is the one
            // place that actually commits to leaving it (as opposed to just
            // pausing to show this screen, or resuming via B), so it's the
            // one place that must save + unload: the game list is never
            // allowed to be reached with something still loaded, so there's
            // nothing left to lose if the player quits from hbmenu/HOME
            // (e.g. via a forwarder) afterward.
            switch_unload_current_game();
            g_settings_active = false;
            g_settings_from_gameplay = false;
            return;
        }
        if (!row_locked) {
            if (row == SettingsRow::BackgroundCustomEntry) {
                open_background_custom_keyboard();
            } else if (row == SettingsRow::SegmentColorCustomEntry) {
                open_segment_color_custom_keyboard();
            } else if (row == SettingsRow::SegmentsCustomEntry) {
                open_segments_custom_keyboard();
            } else if (row == SettingsRow::PanelGapCustomEntry) {
                open_panelgap_custom_keyboard();
            } else if (row == SettingsRow::PetTimeCapCustomEntry) {
                open_pettimecap_custom_keyboard();
            }
        }
    }

    // Either button closes settings. From the game list, that means back to
    // the list (g_menu_active stays true). Opened mid-game instead, B/Plus
    // resume the paused game in place - g_menu_active flips back off, which
    // is also what stops main.cpp from stepping the CPU while this screen is
    // up in the first place (see switch_menu_open_ingame_settings()).
    if ((kDown & HidNpadButton_Plus) || (kDown & HidNpadButton_B)) {
        g_settings_active = false;
        if (g_settings_from_gameplay) {
            g_settings_from_gameplay = false;
            g_menu_active = false;
        }
    }
}

void switch_menu_update(uint64_t kDown, uint64_t kHeld) {
    if (!g_menu_active) {
        // B is deliberately not a back-out-to-menu shortcut here: several
        // G&W games use B as a real gameplay input, so only the L+R+Plus
        // combo (handled in main.cpp, via switch_menu_return_to_menu())
        // returns to the menu from gameplay.
        return;
    }

    if (g_load_prompt_active) {
        update_load_prompt(kDown);
        return;
    }

    // Checked before g_settings_active - this is a modal layered on top of
    // the settings screen (which stays active underneath), not a separate
    // top-level screen like the load prompt above.
    if (g_experimental_games_confirm_active) {
        update_experimental_games_confirm(kDown);
        return;
    }

    if (g_experimental_features_confirm_active) {
        update_experimental_features_confirm(kDown);
        return;
    }

    if (g_settings_active) {
        update_settings_menu(kDown);
        return;
    }

    if (kDown & HidNpadButton_Plus) {
        g_settings_active = true;
        g_settings_from_gameplay = false;
        g_settings_selected_row = 0;
        return;
    }

    if (kDown & HidNpadButton_Right) select_next(+1);
    if (kDown & HidNpadButton_Left) select_next(-1);
    if (kDown & HidNpadButton_Up) select_next_other_group(+1);
    if (kDown & HidNpadButton_Down) select_next_other_group(-1);

    if (kDown & HidNpadButton_Minus) {
        g_sort_mode = (SortMode)(((int)g_sort_mode + 1) % kSortModeCount);
        rebuild_sorted_order();
    }

    if (kDown & HidNpadButton_A) {
        // Hidden debug mode bypasses save states entirely (see g_debug_mode's
        // doc comment in switch_settings.h) - forcing has_save false here is
        // enough to route straight to the always-fresh path below, same as a
        // game with no save at all.
        const bool has_save = !g_debug_mode && save_state_exists(g_selected_index);

        if (has_save && g_load_state_behavior == LoadStateBehavior::Ask) {
            YOKOI_LOG("switch_menu: A pressed on index %u, save exists (Ask) - showing load prompt", (unsigned)g_selected_index);
            g_load_prompt_active = true;
        } else if (has_save && g_load_state_behavior == LoadStateBehavior::AutoLoad) {
            YOKOI_LOG("switch_menu: A pressed on index %u, save exists (Auto-Load) - loading directly", (unsigned)g_selected_index);
            switch_load_game_by_index_and_init(g_selected_index);
            if (load_game_state(g_cpu.get(), g_selected_index)) {
                pet_time_on_state_loaded();  // time away since that save - see pet_time.h
            }
            g_menu_active = false;
        } else {
            // No save, Never Load, or debug mode - always fresh. Never
            // deletes an existing save file (matching both 3DS and Android -
            // neither calls delete_game_state() either); it just sits until
            // the next real exit overwrites it (or forever, under debug mode).
            YOKOI_LOG("switch_menu: A pressed on index %u, starting fresh", (unsigned)g_selected_index);
            switch_load_game_by_index_and_init(g_selected_index);
            g_menu_active = false;
        }
    }
}

void switch_menu_return_to_menu() {
    g_menu_active = true;
}

void switch_menu_open_ingame_settings() {
    // switch_menu_return_to_menu() is what actually pauses the game: it sets
    // g_menu_active, and main.cpp only steps the CPU and calls
    // switch_screen_render() when switch_menu_is_active() is false - so this
    // alone stops the simulation exactly like the old straight-to-menu
    // L+R+Plus behavior did. The settings screen is then layered on top of
    // that already-paused state.
    switch_menu_return_to_menu();
    g_settings_active = true;
    g_settings_from_gameplay = true;
    g_settings_selected_row = 0;
}

void switch_menu_render(int viewport_w, int viewport_h) {
    reload_thumbnail_if_needed();

    // viewport_w/viewport_h are the true (always-landscape) physical
    // dimensions - the actual glViewport() call always uses them as-is.
    // Everything else below lays out in "logical" space, which is
    // width/height-swapped at 90/270 so the whole menu (not just gameplay)
    // rotates - see switch_ui_begin_frame()'s and switch_screen_render()'s
    // matching uRotCosSin setup for why swapping the fit/layout dimensions
    // here and rotating the final NDC output there is lossless either way.
    int logical_w = viewport_w, logical_h = viewport_h;
    if (g_screen_rotation != ScreenRotation::Deg0) {
        std::swap(logical_w, logical_h);
    }

    glViewport(0, 0, viewport_w, viewport_h);
    glClearColor(kBgR, kBgG, kBgB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    switch_ui_begin_frame(logical_w, logical_h);

    switch_ui_draw_rect(0.0f, 0.0f, (float)logical_w, (float)logical_h, kBgR, kBgG, kBgB, 1.0f);
    switch_ui_draw_rect(0.0f, (float)(kTopBarH - 1), (float)logical_w, 1.0f, kTextR, kTextG, kTextB, 0.4f);
    switch_ui_draw_rect(0.0f, (float)(logical_h - kBottomBarH), (float)logical_w, 1.0f, kTextR, kTextG, kTextB, 0.4f);

    // Small header icon, left of the title - drawn at a fixed square size
    // matching the title's own line height rather than the source PNG's
    // native 48x48, so it scales consistently regardless of that asset's
    // actual dimensions.
    float title_x = 24.0f;
    if (g_header_icon_tex != 0) {
        constexpr float kIconSize = 28.0f;
        switch_ui_draw_texture_plain(g_header_icon_tex, title_x, 16.0f, kIconSize, kIconSize);
        title_x += kIconSize + 10.0f;
    }
    switch_ui_draw_text(title_x, 16.0f, 28.0f, kTextR, kTextG, kTextB, 1.0f, g_settings_from_gameplay ? "PAUSED" : "YOKOI", /*bold=*/true);

    if (g_load_prompt_active) {
        render_load_prompt(logical_w, logical_h);
        return;
    }

    if (g_experimental_games_confirm_active) {
        render_experimental_games_confirm(logical_w, logical_h);
        return;
    }

    if (g_experimental_features_confirm_active) {
        render_experimental_features_confirm(logical_w, logical_h);
        return;
    }

    if (g_settings_active) {
        // Debug mode locks every row but Rotation to whatever's already on
        // disk (see g_debug_mode's doc comment in switch_settings.h) - this
        // tag is the only in-app indication that's happening, since it's
        // otherwise silent by design (hidden, manual-only flag).
        const std::string title = g_debug_mode ? "Settings (DEBUG)" : "Settings";
        const float title_size = 20.0f;
        const float title_w = switch_ui_text_width(title, title_size);
        switch_ui_draw_text((float)logical_w - 24.0f - title_w, 20.0f, title_size, kTextR, kTextG, kTextB, 0.85f, title);

        render_settings_rows(logical_w, logical_h);

        const float hint_size = 20.0f;
        const float hint_y = (float)(logical_h - kBottomBarH + 14);
        const float icon_pad = 6.0f;
        const float group_pad = 22.0f;
        float hx = 24.0f;

        auto put_icon = [&](SwitchButtonIcon icon) {
            switch_ui_draw_button_icon(hx, hint_y, hint_size, kTextR, kTextG, kTextB, 0.85f, icon);
            hx += switch_ui_button_icon_width(hint_size, icon) + icon_pad;
        };
        auto put_text = [&](const std::string& s) {
            switch_ui_draw_text(hx, hint_y, hint_size, kTextR, kTextG, kTextB, 0.85f, s);
            hx += switch_ui_text_width(s, hint_size) + group_pad;
        };

        put_icon(SwitchButtonIcon::DpadUp);
        put_icon(SwitchButtonIcon::DpadDown);
        put_text("Select");
        put_icon(SwitchButtonIcon::DpadLeft);
        put_icon(SwitchButtonIcon::DpadRight);
        put_text("Change");
        // "A" only does something on an action row (Return to Main Menu, or
        // either Custom-entry row) - shown whenever one of those can
        // currently be reached, not just in the from-gameplay case that used
        // to be the only source of one.
        if (g_settings_from_gameplay || g_bg_custom_selected || g_segcolor_custom_selected || g_seg_custom_selected || g_panelgap_custom_selected
            || g_pettimecap_custom_selected) {
            put_icon(SwitchButtonIcon::A);
            put_text("Confirm");
        }
        put_icon(SwitchButtonIcon::B);
        put_text(g_settings_from_gameplay ? "Resume" : "Back");
        return;
    }

    {
        const std::string sort_label = sort_mode_label(g_sort_mode);
        const float sort_size = 20.0f;
        const float sort_w = switch_ui_text_width(sort_label, sort_size);
        switch_ui_draw_text((float)logical_w - 24.0f - sort_w, 20.0f, sort_size, kTextR, kTextG, kTextB, 0.85f, sort_label);
    }

    const GW_rom* g = load_game(g_selected_index);
    const std::string name = g ? g->name : std::string("?");
    const std::string mfr = mfr_name(get_mfr(g_selected_index));
    const std::string date_str = g ? g->date : std::string();
    char counter[32];
    // Position within the *current sort order*, not the raw pack index -
    // otherwise this wouldn't track what Left/Right is actually stepping
    // through once sort mode isn't Manufacturer. Same reasoning extends to
    // the total: g_sorted_order.size() is the visible-game count (it already
    // excludes hidden experimental games - see rebuild_sorted_order()), not
    // get_nb_name()'s raw total including anything currently hidden.
    snprintf(counter, sizeof(counter), "%u / %u", (unsigned)(position_of_selected() + 1), (unsigned)g_sorted_order.size());
    const std::string counter_str(counter);

    // Text sizes are now real pixel heights (switch_ui switched from a
    // bitmap font to FreeType - see switch_ui.cpp), doubled from the first
    // pass per feedback. Bars sized down (kTopBarH/kBottomBarH above) so the
    // thumbnail box and text have room to grow instead of shrinking to fit.
    const float mfr_size = 32.0f;
    const float name_size = 40.0f;
    const float date_size = 22.0f;
    const float counter_size = 22.0f;
    const float frame_pad = 10.0f;
    const float gap_thumb_to_text = 18.0f;
    const float gap_between_lines = 6.0f;

    // Date line's height is always reserved (even when a game has no date)
    // so the layout doesn't jump around between games.
    const float text_block_h = gap_thumb_to_text + mfr_size + gap_between_lines + name_size
        + gap_between_lines + date_size + gap_between_lines + counter_size;

    const float margin = 14.0f;
    const float avail_top = kTopBarH + margin;
    const float avail_bottom = logical_h - kBottomBarH - margin;
    const float avail_h = avail_bottom - avail_top;

    // Console art across the whole catalog turns out to share one aspect
    // ratio (a MAME screenshot on a fixed console template), so size the box
    // itself to that aspect - contain-fit against the available width/height
    // budget - instead of forcing a square and letterboxing the image inside
    // it. This is why the box is now visibly shorter than it used to be:
    // that letterbox space is what freed up room for the date line above.
    float aspect = 4.0f / 3.0f; // sane fallback if a game is missing console_info
    if (g && g->console_info && g->console_info[4] > 0 && g->console_info[5] > 0) {
        aspect = (float)g->console_info[4] / (float)g->console_info[5];
    }

    const float max_box_w = (float)kThumbMax;
    const float max_box_h = avail_h - text_block_h - frame_pad * 2.0f;
    float box_w = max_box_w;
    float box_h = box_w / aspect;
    if (box_h > max_box_h) {
        box_h = max_box_h;
        box_w = box_h * aspect;
    }

    // box_h's formula above makes content_h == avail_h whenever it isn't
    // capped by max_box_w, i.e. there's usually ~0 slack to center within -
    // so shift_up has to be clamped against avail_top, or it just pushes the
    // card above the top bar's separator line instead of centering it.
    const float content_h = frame_pad * 2.0f + box_h + text_block_h;
    const float shift_up = 8.0f;
    float content_top = avail_top + (avail_h - content_h) * 0.5f - shift_up;
    if (content_top < avail_top) content_top = avail_top;

    const float cx = logical_w * 0.5f;
    const float box_x = cx - box_w * 0.5f;
    const float box_y = content_top + frame_pad;

    // Framed-card look: an accent-colored backdrop around the box, matching
    // Hekate's highlighted-selection treatment.
    switch_ui_draw_rect(box_x - frame_pad, box_y - frame_pad, box_w + frame_pad * 2.0f, box_h + frame_pad * 2.0f,
                         kAccentR, kAccentG, kAccentB, 0.85f);
    switch_ui_draw_rect(box_x, box_y, box_w, box_h, kBgR * 1.4f, kBgG * 1.4f, kBgB * 1.4f, 1.0f);

    if (g_thumb_tex != 0 && g && g->console_info) {
        const uint16_t* ci = g->console_info;
        switch_ui_draw_texture(g_thumb_tex, box_x, box_y, box_w, box_h,
                                (float)g_thumb_w, (float)g_thumb_h, (float)ci[2], (float)ci[3], (float)ci[4], (float)ci[5]);
    }

    float text_y = box_y + box_h + frame_pad + gap_thumb_to_text;

    const float mfr_w = switch_ui_text_width(mfr, mfr_size);
    switch_ui_draw_text(cx - mfr_w * 0.5f, text_y, mfr_size, kHighlightR, kHighlightG, kHighlightB, 1.0f, mfr);
    text_y += mfr_size + gap_between_lines;

    const float name_w = switch_ui_text_width(name, name_size);
    switch_ui_draw_text(cx - name_w * 0.5f, text_y, name_size, kTextR, kTextG, kTextB, 1.0f, name);
    text_y += name_size + gap_between_lines;

    if (!date_str.empty()) {
        const float date_w = switch_ui_text_width(date_str, date_size);
        switch_ui_draw_text(cx - date_w * 0.5f, text_y, date_size, kTextR, kTextG, kTextB, 0.6f, date_str);
    }
    text_y += date_size + gap_between_lines;

    const float counter_w = switch_ui_text_width(counter_str, counter_size);
    switch_ui_draw_text(cx - counter_w * 0.5f, text_y, counter_size, kTextR, kTextG, kTextB, 0.6f, counter_str);

    // Nintendo button-prompt glyphs instead of plain text/ASCII arrows, same
    // as Hekate-Toolbox's own control hints (switch_ui.cpp's
    // switch_ui_draw_button_icon - see its header comment for where the
    // codepoints come from). switch_ui has no rich-text markup for mixing
    // icons and text, so this composes the line by hand, tracking cursor_x
    // as each piece's width is drawn.
    {
        const float hint_size = 20.0f;
        const float hint_y = (float)(logical_h - kBottomBarH + 14);
        const float icon_pad = 6.0f;
        const float group_pad = 22.0f;
        float hx = 24.0f;

        auto put_icon = [&](SwitchButtonIcon icon) {
            switch_ui_draw_button_icon(hx, hint_y, hint_size, kTextR, kTextG, kTextB, 0.85f, icon);
            hx += switch_ui_button_icon_width(hint_size, icon) + icon_pad;
        };
        auto put_text = [&](const std::string& s) {
            switch_ui_draw_text(hx, hint_y, hint_size, kTextR, kTextG, kTextB, 0.85f, s);
            hx += switch_ui_text_width(s, hint_size) + group_pad;
        };

        put_icon(SwitchButtonIcon::A);
        put_text("Play");
        put_icon(SwitchButtonIcon::DpadLeft);
        put_icon(SwitchButtonIcon::DpadRight);
        put_text("Game");
        put_icon(SwitchButtonIcon::DpadUp);
        put_icon(SwitchButtonIcon::DpadDown);
        put_text(sort_mode_group_label(g_sort_mode));
        put_icon(SwitchButtonIcon::Minus);
        put_text("Sort");
        put_icon(SwitchButtonIcon::Plus);
        put_text("Settings");
    }
}
