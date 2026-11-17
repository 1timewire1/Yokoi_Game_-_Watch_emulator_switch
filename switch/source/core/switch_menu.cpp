#include "switch_menu.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include <switch.h>

#include "BrickCores/BrickRom.h"
#include "BrickCores/BrickCore.h"
#include "std/GW_ROM.h"
#include "std/debug_log.h"
#include "std/gw_pack.h"
#include "std/load_file.h"
#include "std/platform_paths.h"
#include "std/savestate.h"
#include "std/settings.h"

#include "../third_party/stb_image.h"

#include "pet_time.h"
#include "SM5XX/SM5XX.h"
#include "save_slots.h"
#include "switch_segments_state.h"
#include "switch_game_loader.h"
#include "switch_layout.h"
#include "switch_runtime_state.h"
#include "switch_settings.h"
#include "render/switch_gl.h"
#include "render/switch_header_icon.h"
#include "render/switch_ui.h"
#include "render/switch_i18n.h"
#include "render/switch_screen.h"
#include "render/switch_transition.h"
#include "audio/menu_music.h"
#include "input/control_remap.h"
#include "game_controls.h"
#include "input/onscreen_keypad.h"
#include "input/menu_touch.h"
#include "input/time_control.h"
#include "link_session.h"
#include "high_scores.h"
#include "score_tracker.h"
#include "tamago_fig_icon.h"

namespace {

// Hekate-Toolbox's dark theme palette (source/gui/theme.c, ColorSetId_Dark),
// reused here for visual consistency even though the renderer is entirely
// different (this project draws through GLES3 quads, not a software framebuffer).
// These are now the *current theme's* colours (settings "Themes" ->
// apply_menu_theme() below); Dark, theme 0, is the original palette.
float kBgR = 0x31 / 255.0f, kBgG = 0x31 / 255.0f, kBgB = 0x31 / 255.0f;
float kTextR = 1.0f, kTextG = 1.0f, kTextB = 1.0f;
float kAccentR = 0x59 / 255.0f, kAccentG = 0xED / 255.0f, kAccentB = 0xC0 / 255.0f;
float kHighlightR = 0x0A / 255.0f, kHighlightG = 0xBF / 255.0f, kHighlightB = 1.0f;
float kTileR = 0x3D / 255.0f, kTileG = 0x3D / 255.0f, kTileB = 0x3D / 255.0f;    // cards / tiles / rows
float kPanelR = 0x2A / 255.0f, kPanelG = 0x2A / 255.0f, kPanelB = 0x2A / 255.0f; // settings sidebar
float kSelR = 0x21 / 255.0f, kSelG = 0x55 / 255.0f, kSelB = 0x70 / 255.0f;       // selected tile/row fill

struct MenuTheme {
    const char* name;
    uint32_t bg, panel, tile, text, accent, highlight, sel;
};
constexpr MenuTheme kMenuThemes[] = {
    {"Dark",           0x313131, 0x2A2A2A, 0x3D3D3D, 0xFFFFFF, 0x59EDC0, 0x0ABFFF, 0x215570},
    {"OLED Black",     0x000000, 0x0A0A0A, 0x171717, 0xFFFFFF, 0x59EDC0, 0x0ABFFF, 0x173A4C},
    {"Light",          0xE9E9E9, 0xDADADA, 0xFFFFFF, 0x202020, 0x0E8C6E, 0x0A7FD0, 0xBFE0F5},
    {"Game & Watch",   0xEDE6D2, 0xE0D8C0, 0xF8F3E6, 0x2B2B2B, 0xC8102E, 0xB5121B, 0xF2CBC4},
    {"Game Boy",       0x8BAC0F, 0x7C9A0D, 0x9BBC0F, 0x0F380F, 0x306230, 0x0F380F, 0xB5CC4A},
    {"Ocean",          0x0F2233, 0x0B1A28, 0x18324A, 0xEAF4FF, 0x4FD1C5, 0x3AA0FF, 0x1D4E7A},
    {"Neon",           0x1A1026, 0x140C1E, 0x2A1A3D, 0xF5EEFF, 0xFF4FD8, 0x9D5CFF, 0x4A2A78},
    {"Famicom",        0x2B1010, 0x220B0B, 0x4A1A1A, 0xF5E6C8, 0xE8B04A, 0xFF5A4A, 0x6E2424},
    {"Virtual Boy",    0x000000, 0x0A0000, 0x1E0000, 0xFF2A2A, 0xFF2A2A, 0xFF5050, 0x4A0000},
    {"Dracula",        0x282A36, 0x21222C, 0x343746, 0xF8F8F2, 0x50FA7B, 0xBD93F9, 0x44475A},
    {"Nord",           0x2E3440, 0x272C36, 0x3B4252, 0xECEFF4, 0xA3BE8C, 0x88C0D0, 0x434C5E},
    {"Gruvbox",        0x282828, 0x1D2021, 0x3C3836, 0xEBDBB2, 0xB8BB26, 0xFABD2F, 0x504945},
    {"Monokai",        0x272822, 0x1E1F1C, 0x3E3D32, 0xF8F8F2, 0xA6E22E, 0xF92672, 0x49483E},
    {"Solarized Dark", 0x002B36, 0x00212B, 0x073642, 0xEEE8D5, 0x2AA198, 0x268BD2, 0x0E4A5A},
    {"Solarized Light",0xFDF6E3, 0xEEE8D5, 0xFFFDF5, 0x073642, 0x2AA198, 0x268BD2, 0xE3DCC6},
    {"Sepia",          0x3B2F25, 0x32271F, 0x4C3D30, 0xF3E3C8, 0xD9A55B, 0xE0B97A, 0x6A5440},
    {"Forest",         0x1B2A1F, 0x15221A, 0x26392B, 0xE6F2E0, 0x8FD18A, 0x5FBF7A, 0x2F5236},
    {"Sunset",         0x2A1630, 0x221128, 0x3D2042, 0xFFE9DC, 0xFF9E5E, 0xFF5E7E, 0x5A2A55},
    {"Sakura",         0xFCEFF3, 0xF5DDE5, 0xFFFFFF, 0x4A2B36, 0xE76F9A, 0xD94C7F, 0xF7CFDC},
    {"Mint",           0xE8F7F1, 0xD7EFE5, 0xFFFFFF, 0x1E3A33, 0x1FA67A, 0x169C8C, 0xC5EBDD},
    {"Pastel",         0xF4F1FB, 0xE7E2F5, 0xFFFFFF, 0x3A3550, 0x7AC7A8, 0x8C7AE6, 0xDCD4F7},
    {"High Contrast",  0x000000, 0x000000, 0x111111, 0xFFFFFF, 0xFFFF00, 0x00FFFF, 0x003A5A},
};
constexpr int kNumMenuThemes = (int)(sizeof(kMenuThemes) / sizeof(kMenuThemes[0]));

// Same order as switch_screen.cpp's ScreenFilter enum.
constexpr const char* kScreenFilterNames[] = {
    "None", "Scanlines", "LCD Grid", "Dot Matrix", "CRT", "Vignette", "Green Phosphor", "Amber",
    "Sepia", "Cool", "Warm", "Night", "Virtual Boy", "Magnifier",
};
constexpr int kNumScreenFilters = (int)(sizeof(kScreenFilterNames) / sizeof(kScreenFilterNames[0]));

void set_rgb(uint32_t c, float& r, float& g, float& b) {
    r = ((c >> 16) & 0xFF) / 255.0f;
    g = ((c >> 8) & 0xFF) / 255.0f;
    b = (c & 0xFF) / 255.0f;
}

void apply_menu_theme() {
    if (g_menu_theme >= kNumMenuThemes) g_menu_theme = 0;
    const MenuTheme& t = kMenuThemes[g_menu_theme];
    set_rgb(t.bg, kBgR, kBgG, kBgB);
    set_rgb(t.panel, kPanelR, kPanelG, kPanelB);
    set_rgb(t.tile, kTileR, kTileG, kTileB);
    set_rgb(t.text, kTextR, kTextG, kTextB);
    set_rgb(t.accent, kAccentR, kAccentG, kAccentB);
    set_rgb(t.highlight, kHighlightR, kHighlightG, kHighlightB);
    set_rgb(t.sel, kSelR, kSelG, kSelB);
}

// Slimmer than Hekate's own 88/73 - their bars hold a title + a grid of
// icons; ours only need a short title/hint line each, so the extra height
// goes to the thumbnail instead.
constexpr int kTopBarH = 64;
constexpr int kBottomBarH = 56;
constexpr int kThumbMax = 460;

// The menu's logical width this frame (720 in portrait) - the hint bars
// shrink to fit it.
int g_menu_lw = 1280;

// The button a prompt icon stands for - a tap on that hint presses it
// (input/menu_touch.h).
uint64_t icon_button(SwitchButtonIcon i) {
    switch (i) {
        case SwitchButtonIcon::A:         return HidNpadButton_A;
        case SwitchButtonIcon::B:         return HidNpadButton_B;
        case SwitchButtonIcon::X:         return HidNpadButton_X;
        case SwitchButtonIcon::Y:         return HidNpadButton_Y;
        case SwitchButtonIcon::L:         return HidNpadButton_L;
        case SwitchButtonIcon::R:         return HidNpadButton_R;
        case SwitchButtonIcon::ZL:        return HidNpadButton_ZL;
        case SwitchButtonIcon::ZR:        return HidNpadButton_ZR;
        case SwitchButtonIcon::DpadUp:    return HidNpadButton_Up;
        case SwitchButtonIcon::DpadDown:  return HidNpadButton_Down;
        case SwitchButtonIcon::DpadLeft:  return HidNpadButton_Left;
        case SwitchButtonIcon::DpadRight: return HidNpadButton_Right;
        case SwitchButtonIcon::Plus:      return HidNpadButton_Plus;
        case SwitchButtonIcon::Minus:     return HidNpadButton_Minus;
    }
    return 0;
}

bool menu_portrait();

bool g_menu_active = true;
uint16_t g_selected_index = 0;
// The selection is a folder's row (the Game Pocket Computer inside Epoch):
// its art is the folder's own console picture, kFolderArt, when the pack has
// one - g_selected_index is then the folder's first game, for everything else.
bool g_sel_is_folder = false;
// Dot-matrix cartridge systems (Game Pocket Computer "GPC", Game Master "HGM"): Filled / Solid only.
static bool is_dot_matrix_ref(const std::string& r) { return r.compare(0, 3, "GPC") == 0 || r.compare(0, 3, "HGM") == 0; }
// A folder's own console picture (blank screen), by the folder's maker id.
const char* folder_art(uint8_t folder) {
    if (folder == GW_rom::MANUFACTURER_HARTUNG_GM) return "console_hgm_folder.png";
    if (folder == GW_rom::MANUFACTURER_TIGER_GC) return "console_tgc_folder.png";
    if (folder == GW_rom::MANUFACTURER_MICROVISION) return "console_mv_folder.png";
    if (folder == GW_rom::MANUFACTURER_TIGER_RZ) return "console_rz_folder.png";
    if (folder == GW_rom::MANUFACTURER_ENTEX_AV) return "console_eav_folder.png";
    return "console_gpc_folder.png";
}

// Load-save-state prompt, shown instead of loading immediately when A is
// pressed on a game that already has a save file (source/std/savestate.h's
// save_state_exists()) - mirrors 3DS's STATE_SAVE_PROMPT: skipped entirely
// (game loads fresh, same as always) when no save exists for that game.
bool g_load_prompt_active = false;

// Save slot screens (see the "Save slots" block further down).
enum class SlotScreen : uint8_t { None, Entry, Save, Load, Simulate };
enum class SlotConfirm : uint8_t { None, Overwrite, Delete };
SlotScreen g_slot_screen = SlotScreen::None;
// Pause menu > Connect to Another Game: the device picker (render_link_picker()).
bool g_link_picker = false;
// Pause menu > Gotchi Figure (TamaTown Tama-Go): the figures list (render_fig_picker()).
bool g_fig_picker = false;
int g_fig_sel = 0;
extern const char* const kTmgFigureNames[13];
void draw_disc(float cx, float cy, float rad, float r, float g, float b, float a);
void draw_contacts_pill(float px, float py, uint8_t c);
std::vector<LinkCandidate> g_link_cands;
int g_link_sel = 0;
SlotConfirm g_slot_confirm = SlotConfirm::None;
int g_slot_sel = 0;
int g_slot_last_card = 0;   // card to return to from the New Game button
std::string g_slot_toast;   // "Saved to Slot 2" etc., shown on the pause list
int g_slot_toast_frames = 0;
bool g_pet_prompt_active = false;
int64_t g_pet_prompt_age = 0;
int g_pet_prompt_sel = 0;
// Undo Simulate: a copy was taken for the game still running (the CPU
// instance it was taken from - a new game load replaces g_cpu).
bool g_undo_available = false;
const void* g_undo_cpu = nullptr;
bool undo_simulate_available() { return g_undo_available && g_cpu && g_cpu.get() == g_undo_cpu; }

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

// The game-list settings screen is split into sections (a sidebar on the
// left, L/R to switch); opened mid-game it stays the single short list it
// always was. Virtual Pets only exists while Experimental Features is on.
enum class SettingsSection : uint8_t { Menu, Display, Screen, Themes, GameList, Sound, Vibration, SavesClock, VirtualPets, Advanced };
// Themes isn't in the sidebar any more: it's a sub-screen of Menu, opened
// from its "Themes" row (B goes back to Menu).
bool g_themes_sub = false;
// Same for Music (Menu's "Music" row, index 1).
bool g_music_sub = false;
int g_settings_section = 0;  // index into settings_sections()

// The pause menu (settings opened mid-game) has its own sections, also
// switched with L/R: Game Session (resume, save/load, leave), Screen,
// Sound, Vibration and - for virtual pets only - Time.
enum class PauseSection : uint8_t { Session, Screen, Sound, Vibration, Time, Controls, Figures };
int g_pause_section = 0;  // index into pause_sections()

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
    MenuStyle,
    MakerOrder,
    MakerColors,
    Records,
    RecordHud,
    RecordsList,
    RecordsClearAll,
    RecordsGame,
    AchNotify,
    AlertColor,
    AchGame,
    MenuTheme,
    MenuFont,
    Ghosting,
    FlickerFilter,
    PixelStyle,
    ScreenFilter,
    FilterStrength,
    CloneFilter,
    TimeSetMode,
    PetTime,
    PetTimeCap,
    PetTimeCapCustomEntry,
    PetTimeDisplay,
    LogFile,
    SplashScreen,
    ThemesSub,
    MusicSub,
    PauseCombo,
    OnScreenKeypad,
    MenuMusic,
    MenuMusicTrack,
    MenuMusicVolume,
    MenuMusicDuck,
    MenuRotation,
    MenuTouch,
    DateFormat,
    ScreenPalette,
    Transition,
    ScreenSize,
    Resume,
    Restart,
    LinkConnect,
    LinkSound,
    LinkSoundButton,
    VibrationGame,
    VibrationPets,
    VibrationMenu,
    VibrationStrength,
    PetAlertLight,
    MotionPedometer,
    PedometerSensitivity,
    MotionTapSensor,
    TapSensitivity,
    FastForward,
    FastForwardSpeed,
    ButtonResponse,
    Rewind,
    RewindLength,
    IrSensor,
    IrSensitivity,
    Language,
    SaveState,
    LoadState,
    SimulateTime,
    UndoSimulate,
    Volume,
    Mute,
    SoundStyle,
    GameComSound,
    GameComSpeed,
    GameComNoise,
    GameComPalette,
    GameComGhost,
    RZoneStyle,
    AdvVisionScreen,
    TamaGoFigure,
    GameMasterSpeed,
    MenuSounds,
    ExperimentalGames,
    ExperimentalFeatures,
    DosimeterReading,
    ReturnToMenu,
};

std::vector<SettingsSection> settings_sections() {
    std::vector<SettingsSection> v = {SettingsSection::Menu, SettingsSection::Display, SettingsSection::Screen, SettingsSection::GameList,
                                      SettingsSection::Sound, SettingsSection::Vibration, SettingsSection::SavesClock};
    if (g_experimental_features_enabled) v.push_back(SettingsSection::VirtualPets);
    v.push_back(SettingsSection::Advanced);
    return v;
}

const char* settings_section_label(SettingsSection s) {
    switch (s) {
        case SettingsSection::Menu:        return "Menu";
        case SettingsSection::Display:     return "Display";
        case SettingsSection::Themes:      return "Themes";
        case SettingsSection::Screen:      return "Screen";
        case SettingsSection::GameList:    return "Game List";
        case SettingsSection::Sound:       return "Sound";
        case SettingsSection::Vibration:   return "Controller Features";
        case SettingsSection::SavesClock:  return "Saves & Clock";
        case SettingsSection::VirtualPets: return "Virtual Pets";
        default:                           return "Advanced";
    }
}

std::vector<PauseSection> pause_sections() {
    std::vector<PauseSection> v = {PauseSection::Session, PauseSection::Screen, PauseSection::Sound, PauseSection::Vibration};
    if (!g_debug_mode && pet_time_current_is_pet()) v.push_back(PauseSection::Time);
    if (g_game && g_cpu) v.push_back(PauseSection::Controls);
    // TamaTown Tama-Go only: its Gotchi figures, a section of their own.
    if (g_game && g_cpu && g_game->ref == "TMGTAMAGO") v.push_back(PauseSection::Figures);
    return v;
}

const char* pause_section_label(PauseSection s) {
    switch (s) {
        case PauseSection::Session:   return "Game Session";
        case PauseSection::Screen:    return "Screen";
        case PauseSection::Sound:     return "Sound";
        case PauseSection::Vibration: return "Controller Features";
        case PauseSection::Controls:  return "Controls";
        case PauseSection::Figures:   return "Figures";
        default:                      return "Time";
    }
}

PauseSection current_pause_section() {
    const std::vector<PauseSection> v = pause_sections();
    if (g_pause_section >= (int)v.size()) g_pause_section = (int)v.size() - 1;
    if (g_pause_section < 0) g_pause_section = 0;
    return v[g_pause_section];
}

SettingsSection current_settings_section() {
    const std::vector<SettingsSection> v = settings_sections();
    if (g_settings_section >= (int)v.size()) g_settings_section = (int)v.size() - 1;
    if (g_settings_section < 0) g_settings_section = 0;
    return v[g_settings_section];
}

// Display (where things go) and Screen (how the game's screen looks) - both
// sections' rows, also shown together in the mid-game list.
void push_display_rows(std::vector<SettingsRow>& rows) {
    rows.push_back(SettingsRow::Rotation);
    rows.push_back(SettingsRow::ScreenSize);
    rows.push_back(SettingsRow::BackgroundArt);
    rows.push_back(SettingsRow::PanelGap);
    if (g_panelgap_custom_selected) rows.push_back(SettingsRow::PanelGapCustomEntry);
}

void push_screen_rows(std::vector<SettingsRow>& rows) {
    rows.push_back(SettingsRow::ScreenPalette);
    rows.push_back(SettingsRow::Background);
    if (g_bg_custom_selected) rows.push_back(SettingsRow::BackgroundCustomEntry);
    rows.push_back(SettingsRow::SegmentColor);
    if (g_segcolor_custom_selected) rows.push_back(SettingsRow::SegmentColorCustomEntry);
    rows.push_back(SettingsRow::Segments);
    if (g_seg_custom_selected) rows.push_back(SettingsRow::SegmentsCustomEntry);
    // Game.com: its own ghosting row instead (GameComGhost).
    if (!(g_game && g_game->ref.compare(0, 3, "TGC") == 0)) rows.push_back(SettingsRow::Ghosting);
    rows.push_back(SettingsRow::FlickerFilter);
    rows.push_back(SettingsRow::ScreenFilter);
    rows.push_back(SettingsRow::FilterStrength);
}

// The single source of truth for "what rows exist right now, in what order" -
// both render_settings_rows() and update_settings_menu() build this fresh
// each time rather than trusting a cached row count, so they can never
// disagree about what row index N currently means. From the game list it's
// the current section's rows only.
// The running game is on the M37520 core (Pocket Pikachu) - the only one the
// Button Response row applies to.
bool game_has_m37520() {
    return g_cpu && g_cpu->get_cpu_type_id() == CPU_TYPE_BRICK &&
           static_cast<BrickCore*>(g_cpu.get())->rom_info().core == "M37520";
}

std::vector<SettingsRow> build_settings_rows() {
    std::vector<SettingsRow> rows;
    if (g_settings_from_gameplay) {
        switch (current_pause_section()) {
            case PauseSection::Controls:
                break;  // its own screen - see render_controls_screen()
            case PauseSection::Figures:
                rows.push_back(SettingsRow::TamaGoFigure);
                break;
            case PauseSection::Session:
                rows.push_back(SettingsRow::Resume);
                // Debug mode has no save states at all (see g_debug_mode).
                if (!g_debug_mode && g_cpu) {
                    rows.push_back(SettingsRow::SaveState);
                    rows.push_back(SettingsRow::LoadState);
                    rows.push_back(SettingsRow::Restart);
                    if (g_records_enabled && g_game && score_tracker_has_records(g_game->ref)) rows.push_back(SettingsRow::RecordsGame);
                    if (g_records_enabled && g_game && score_tracker_has_records(g_game->ref)) rows.push_back(SettingsRow::AchGame);
                    if (g_game && link_kind_of(g_game->ref) != LinkKind::None) rows.push_back(SettingsRow::LinkConnect);
                    if (link_session_active()) rows.push_back(SettingsRow::LinkSound);
                    rows.push_back(SettingsRow::FastForward);
                    rows.push_back(SettingsRow::Rewind);
                    if (game_has_m37520()) rows.push_back(SettingsRow::ButtonResponse);
                }
                // Only while Altair itself is loaded and Experimental Features is
                // on - see g_dosimeter_reading's doc comment in switch_settings.h.
                if (g_game && g_game->ref == "NALTAIR" && g_experimental_features_enabled) {
                    rows.push_back(SettingsRow::DosimeterReading);
                }
                rows.push_back(SettingsRow::ReturnToMenu);
                break;
            case PauseSection::Screen:
                push_display_rows(rows);
                push_screen_rows(rows);
                // Only for the Brick Games with a grid of filled dots.
                if (switch_screen_pixel_style_applies()) rows.push_back(SettingsRow::PixelStyle);
                if (g_game && g_game->ref.compare(0, 3, "TGC") == 0) rows.push_back(SettingsRow::GameComGhost);
                if (g_game && g_game->ref.compare(0, 2, "RZ") == 0) rows.push_back(SettingsRow::RZoneStyle);
                if (g_game && g_game->ref.compare(0, 3, "EAV") == 0) rows.push_back(SettingsRow::AdvVisionScreen);
                // Console case: only for one-screen games that actually have
                // case artwork (true dual-screen games never draw it).
                if (g_game && g_game->console_info && !g_split_two_screens_to_panels)
                    rows.push_back(SettingsRow::ConsoleArt);
                if (onscreen_keypad_game_has_one()) rows.push_back(SettingsRow::OnScreenKeypad);
                break;
            case PauseSection::Sound:
                rows.push_back(SettingsRow::Volume);
                rows.push_back(SettingsRow::Mute);
                rows.push_back(SettingsRow::SoundStyle);
                // Only in the games of those systems.
                if (g_game && g_game->ref.compare(0, 3, "TGC") == 0) {
                    rows.push_back(SettingsRow::GameComSound);
                    rows.push_back(SettingsRow::GameComSpeed);
                    rows.push_back(SettingsRow::GameComNoise);
                }
                if (g_game && g_game->ref.compare(0, 3, "HGM") == 0) rows.push_back(SettingsRow::GameMasterSpeed);
                break;
            case PauseSection::Vibration:
                rows.push_back(SettingsRow::VibrationGame);
                rows.push_back(SettingsRow::VibrationPets);
                rows.push_back(SettingsRow::VibrationStrength);
                rows.push_back(SettingsRow::PetAlertLight);
                rows.push_back(SettingsRow::MotionPedometer);
                if (g_motion_pedometer) rows.push_back(SettingsRow::PedometerSensitivity);
            rows.push_back(SettingsRow::MotionTapSensor);
            if (g_motion_tap_sensor) rows.push_back(SettingsRow::TapSensitivity);
            rows.push_back(SettingsRow::IrSensor);
            if (g_ir_sensor) rows.push_back(SettingsRow::IrSensitivity);
                break;
            case PauseSection::Time:
                rows.push_back(SettingsRow::SimulateTime);
                if (undo_simulate_available()) rows.push_back(SettingsRow::UndoSimulate);
                // Pet Time itself is an Experimental Feature (see the
                // Virtual Pets section).
                if (g_experimental_features_enabled) {
                    rows.push_back(SettingsRow::PetTime);
                    if (g_pet_time_mode == PetTimeMode::RealTime) {
                        rows.push_back(SettingsRow::PetTimeCap);
                        if (g_pettimecap_custom_selected) rows.push_back(SettingsRow::PetTimeCapCustomEntry);
                    }
                }
                break;
        }
        return rows;
    }
    if (g_themes_sub) {
        rows.push_back(SettingsRow::MenuTheme);
        rows.push_back(SettingsRow::MenuFont);
        return rows;
    }
    if (g_music_sub) {
        rows.push_back(SettingsRow::MenuMusic);
        if (g_menu_music == kMenuMusicCustom) rows.push_back(SettingsRow::MenuMusicTrack);
        rows.push_back(SettingsRow::MenuMusicVolume);
        rows.push_back(SettingsRow::MenuMusicDuck);
        return rows;
    }
    switch (current_settings_section()) {
        case SettingsSection::Menu:
            rows.push_back(SettingsRow::ThemesSub);
            rows.push_back(SettingsRow::MusicSub);
            rows.push_back(SettingsRow::PauseCombo);
            rows.push_back(SettingsRow::Language);
            rows.push_back(SettingsRow::MenuStyle);
            rows.push_back(SettingsRow::MakerOrder);
            rows.push_back(SettingsRow::MakerColors);
            rows.push_back(SettingsRow::Transition);
            rows.push_back(SettingsRow::MenuRotation);
            rows.push_back(SettingsRow::MenuTouch);
            rows.push_back(SettingsRow::DateFormat);
            rows.push_back(SettingsRow::MenuSounds);
            rows.push_back(SettingsRow::VibrationMenu);
            rows.push_back(SettingsRow::SplashScreen);
            break;
        case SettingsSection::Display:
            push_display_rows(rows);
            rows.push_back(SettingsRow::ConsoleArt);
            break;
        case SettingsSection::Themes:
            rows.push_back(SettingsRow::MenuTheme);
            rows.push_back(SettingsRow::MenuFont);
            break;
        case SettingsSection::Screen:
            push_screen_rows(rows);
            break;
        case SettingsSection::Sound:
            rows.push_back(SettingsRow::Volume);
            rows.push_back(SettingsRow::Mute);
            rows.push_back(SettingsRow::SoundStyle);
            break;
        case SettingsSection::Vibration:
            rows.push_back(SettingsRow::VibrationGame);
            rows.push_back(SettingsRow::VibrationPets);
            rows.push_back(SettingsRow::VibrationStrength);
            rows.push_back(SettingsRow::PetAlertLight);
            rows.push_back(SettingsRow::MotionPedometer);
            if (g_motion_pedometer) rows.push_back(SettingsRow::PedometerSensitivity);
            rows.push_back(SettingsRow::MotionTapSensor);
            if (g_motion_tap_sensor) rows.push_back(SettingsRow::TapSensitivity);
            rows.push_back(SettingsRow::IrSensor);
            if (g_ir_sensor) rows.push_back(SettingsRow::IrSensitivity);
            break;
        case SettingsSection::GameList:
            rows.push_back(SettingsRow::CloneFilter);
            rows.push_back(SettingsRow::ExperimentalGames);
            rows.push_back(SettingsRow::Records);
            if (g_records_enabled) {
                rows.push_back(SettingsRow::RecordHud);
                rows.push_back(SettingsRow::AchNotify);
                rows.push_back(SettingsRow::AlertColor);
                rows.push_back(SettingsRow::RecordsList);
                rows.push_back(SettingsRow::RecordsClearAll);
            }
            break;
        case SettingsSection::SavesClock:
            rows.push_back(SettingsRow::LoadStateBehavior);
            rows.push_back(SettingsRow::TimeSetMode);
            rows.push_back(SettingsRow::FastForward);
            if (g_fastforward_button) rows.push_back(SettingsRow::FastForwardSpeed);
            rows.push_back(SettingsRow::Rewind);
            if (g_rewind_button) rows.push_back(SettingsRow::RewindLength);
            break;
        case SettingsSection::VirtualPets:
            // Pet Time is an Experimental Feature (pet_time_enabled()); the
            // limit and display only mean anything under Real Time.
            rows.push_back(SettingsRow::PetTime);
            if (g_pet_time_mode == PetTimeMode::RealTime) {
                rows.push_back(SettingsRow::PetTimeCap);
                if (g_pettimecap_custom_selected) rows.push_back(SettingsRow::PetTimeCapCustomEntry);
                rows.push_back(SettingsRow::PetTimeDisplay);
            }
            break;
        case SettingsSection::Advanced:
            rows.push_back(SettingsRow::ExperimentalFeatures);
            rows.push_back(SettingsRow::LogFile);
            break;
    }
    return rows;
}

const char* menu_style_label(MenuStyle m) {
    return m == MenuStyle::Classic ? "Classic" : m == MenuStyle::Carousel ? "Carousel" : m == MenuStyle::List ? "List" : "Makers";
}
const char* maker_order_label(MakerOrder m) {
    switch (m) {
        case MakerOrder::MostGames: return "Most Games";
        case MakerOrder::Era:       return "Era";
        default:                    return "A-Z";
    }
}

// Action rows have no adjustable value - Left/Right do nothing, only A does
// (open the keyboard, or leave the game) - rendered as plain highlighted
// text spanning the row instead of a label/value pair with < > arrows.
bool settings_row_is_action(SettingsRow row) {
    return row == SettingsRow::ReturnToMenu
        || row == SettingsRow::ThemesSub
        || row == SettingsRow::MusicSub
        || row == SettingsRow::Resume
        || row == SettingsRow::Restart
        || row == SettingsRow::RecordsClearAll
        || row == SettingsRow::RecordsGame
        || row == SettingsRow::AchGame
        || row == SettingsRow::LinkConnect
        || row == SettingsRow::SaveState
        || row == SettingsRow::LoadState
        || row == SettingsRow::SimulateTime
        || row == SettingsRow::UndoSimulate
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
    swkbdConfigSetHeaderText(&kbd, switch_tr_c(header));
    swkbdConfigSetGuideText(&kbd, switch_tr_c(guide));
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
uint16_t find_game_index_by_ref(const std::string& ref, bool* out_found) {
    *out_found = false;
    const size_t n = get_nb_name();
    for (size_t i = 0; i < n; i++) {
        const GW_rom* g = load_game((uint16_t)i);
        if (g && g->ref == ref) {
            *out_found = true;
            return (uint16_t)i;
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
std::vector<uint16_t> g_sorted_order;

GLuint g_thumb_tex = 0;
int g_thumb_w = 0, g_thumb_h = 0;
uint16_t g_loaded_thumb_index = 0xFFFF;
bool g_loaded_thumb_folder = false;

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
    if (g_loaded_thumb_index == g_selected_index && g_loaded_thumb_folder == g_sel_is_folder) {
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
        g_loaded_thumb_folder = g_sel_is_folder;
        return;
    }

    std::string asset = t3x_path_to_png_asset(g->path_console);
    const uint8_t* data = nullptr;
    size_t size = 0;
    if (g_sel_is_folder && gw_pack::get_file_bytes(folder_art(g->manufacturer), data, size) && data && size) asset = folder_art(g->manufacturer);
    if (asset.empty() || !gw_pack::get_file_bytes(asset, data, size) || !data || size == 0) {
        YOKOI_LOG("switch_menu: thumbnail pack entry not found: %s", asset.c_str());
        g_loaded_thumb_index = g_selected_index;
        g_loaded_thumb_folder = g_sel_is_folder;
        return;
    }

    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load_from_memory(data, (int)size, &w, &h, &channels, 4);
    if (!pixels) {
        YOKOI_LOG("switch_menu: thumbnail decode failed for %s: %s", asset.c_str(), stbi_failure_reason());
        g_loaded_thumb_index = g_selected_index;
        g_loaded_thumb_folder = g_sel_is_folder;
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
        g_loaded_thumb_folder = g_sel_is_folder;
}

uint8_t get_mfr(uint16_t idx) {
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
    if (mfr == GW_rom::MANUFACTURER_EDLA) return "EDLA";
    if (mfr == GW_rom::MANUFACTURER_STADLBAUER) return "STADLBAUER";
    if (mfr == GW_rom::MANUFACTURER_UNBRANDED) return "UNBRANDED";
    if (mfr == GW_rom::MANUFACTURER_COLECO) return "COLECO";
    if (mfr == GW_rom::MANUFACTURER_ENTEX) return "ENTEX INDUSTRIES";
    if (mfr == GW_rom::MANUFACTURER_TOMY) return "TOMY COMPANY";
    if (mfr == GW_rom::MANUFACTURER_HITORIKKO) return "HITORIKKO";
    if (mfr == GW_rom::MANUFACTURER_PARKER_BROTHERS) return "PARKER BROTHERS";
    if (mfr == GW_rom::MANUFACTURER_BAMBINO) return "BAMBINO";
    if (mfr == GW_rom::MANUFACTURER_MICROVISION) return "MILTON BRADLEY MICROVISION";
    if (mfr == GW_rom::MANUFACTURER_EPOCH_GPC) return "EPOCH GAME POCKET COMPUTER";
    if (mfr == GW_rom::MANUFACTURER_HARTUNG) return "HARTUNG";
    if (mfr == GW_rom::MANUFACTURER_HARTUNG_GM) return "HARTUNG GAME MASTER";
    if (mfr == GW_rom::MANUFACTURER_TIGER_GC) return "TIGER GAME.COM";
    if (mfr == GW_rom::MANUFACTURER_MILTON_BRADLEY) return "MILTON BRADLEY";
    if (mfr == GW_rom::MANUFACTURER_TIGER_RZ) return "TIGER R-ZONE XPG";
    if (mfr == GW_rom::MANUFACTURER_ENTEX_AV) return "ENTEX ADVENTURE VISION";
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
        case GW_rom::MANUFACTURER_EPOCH_GPC:             return 9;   // a folder inside Epoch
        case GW_rom::MANUFACTURER_HARTUNG_GM:            return 24;  // a folder inside Hartung
        case GW_rom::MANUFACTURER_TIGER_GC:              return 3;   // a folder inside Tiger
        case GW_rom::MANUFACTURER_TIGER_RZ:              return 3;   // a folder inside Tiger
        case GW_rom::MANUFACTURER_ENTEX_AV:              return 22;  // a folder inside Entex
        case GW_rom::MANUFACTURER_GAKKEN:                return 10;
        case GW_rom::MANUFACTURER_NIKKO:                 return 11;
        case GW_rom::MANUFACTURER_TANDY:                 return 12;
        case GW_rom::MANUFACTURER_HIRO:                  return 13;
        case GW_rom::MANUFACTURER_SPICA:                 return 14;
        case GW_rom::MANUFACTURER_APOLLO:                return 15;
        case GW_rom::MANUFACTURER_BAMBINO:               return 16;
        case GW_rom::MANUFACTURER_BRICK_GAME:            return 17;
        case GW_rom::MANUFACTURER_COLECO:                return 18;
        case GW_rom::MANUFACTURER_COMPILE:               return 19;
        case GW_rom::MANUFACTURER_COOL_TEC:              return 20;
        case GW_rom::MANUFACTURER_EDLA:                  return 21;
        case GW_rom::MANUFACTURER_ENTEX:                 return 22;
        case GW_rom::MANUFACTURER_GAMETECH:              return 23;
        case GW_rom::MANUFACTURER_HARTUNG:               return 24;
        case GW_rom::MANUFACTURER_HITORIKKO:             return 25;
        case GW_rom::MANUFACTURER_MCDONALDS:             return 26;
        case GW_rom::MANUFACTURER_MICROVISION:           return 27;
        case GW_rom::MANUFACTURER_MILTON_BRADLEY:        return 27;  // holds the Microvision folder
        case GW_rom::MANUFACTURER_NATURE_LAND:           return 28;
        case GW_rom::MANUFACTURER_PARKER_BROTHERS:       return 29;
        case GW_rom::MANUFACTURER_SPARKLE_CRYSTAL:       return 30;
        case GW_rom::MANUFACTURER_STADLBAUER:            return 31;
        case GW_rom::MANUFACTURER_TOMY:                  return 32;
        case GW_rom::MANUFACTURER_VGS:                   return 33;
        case GW_rom::MANUFACTURER_UNBRANDED:             return 34;
        case GW_rom::MANUFACTURER_DAVID_AND_JOHN:        return 35;
        case GW_rom::MANUFACTURER_BURGER_BARD:           return 36;
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
        case GW_rom::MANUFACTURER_COLECO:
        case GW_rom::MANUFACTURER_ENTEX:
        case GW_rom::MANUFACTURER_TOMY:
        case GW_rom::MANUFACTURER_HITORIKKO:
        case GW_rom::MANUFACTURER_PARKER_BROTHERS:
        case GW_rom::MANUFACTURER_BAMBINO:
        case GW_rom::MANUFACTURER_MICROVISION:
        case GW_rom::MANUFACTURER_MILTON_BRADLEY:
        case GW_rom::MANUFACTURER_EPOCH_GPC:
        case GW_rom::MANUFACTURER_HARTUNG_GM:
        case GW_rom::MANUFACTURER_TIGER_GC:
        case GW_rom::MANUFACTURER_TIGER_RZ:
        case GW_rom::MANUFACTURER_ENTEX_AV:
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
    // BrickEmuPy: Big Cat is Super Gyaoppi's game, the European V3 the US ROM.
    "BIGCAT9IN1", "TAMAGOTCHIV3EU",
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
    switch (d) {
        case PetTimeDisplay::FastForward: return "Fast-Forward";
        case PetTimeDisplay::Clock:       return "Clock";
        case PetTimeDisplay::DayNight:    return "Day & Night";
        case PetTimeDisplay::ScreenBar:   return "Screen & Bar";
        default:                          return "Progress Bar";
    }
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

std::string name_of(uint16_t idx) {
    const GW_rom* g = load_game(idx);
    return g ? g->name : std::string();
}

std::string date_of(uint16_t idx) {
    const GW_rom* g = load_game(idx);
    return g ? g->date : std::string();
}

std::string ref_of(uint16_t idx) {
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
// A game's date ("YYYY-MM-DD", or just "YYYY" / "YYYY-MM") as Settings >
// Menu > Date Format writes it. Anything else is shown as it is.
std::string format_date(const std::string& d) {
    if (g_date_format == 0 || d.size() < 4) return d;
    for (int i = 0; i < 4; i++) if (!isdigit((unsigned char)d[i])) return d;
    const std::string y = d.substr(0, 4);
    const std::string m = d.size() >= 7 ? d.substr(5, 2) : std::string();
    const std::string dd = d.size() >= 10 ? d.substr(8, 2) : std::string();
    if (m.empty()) return y;
    if (dd.empty()) return m + "/" + y;
    return g_date_format == 1 ? dd + "/" + m + "/" + y : m + "/" + dd + "/" + y;
}

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
int group_key(uint16_t idx) {
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
bool is_experimental_game(uint16_t idx) {
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
        if (!show_experimental_games && is_experimental_game((uint16_t)i)) {
            continue;
        }
        if (clone_hidden_under(ref_of((uint16_t)i), g_clone_filter_mode)) {
            continue;
        }
        g_sorted_order.push_back((uint16_t)i);
    }

    // Date order everywhere: ISO date strings sort chronologically as text,
    // ties broken by name so equal-date games always land in a stable,
    // predictable order.
    auto date_then_name = [](uint16_t a, uint16_t b) {
        const std::string da = date_of(a), db = date_of(b);
        if (da != db) return da < db;
        return name_less_ci(name_of(a), name_of(b));
    };

    switch (g_sort_mode) {
        case SortMode::Name:
            std::stable_sort(g_sorted_order.begin(), g_sorted_order.end(), [](uint16_t a, uint16_t b) {
                return name_less_ci(name_of(a), name_of(b));
            });
            break;
        case SortMode::Date:
            std::stable_sort(g_sorted_order.begin(), g_sorted_order.end(), date_then_name);
            break;
        case SortMode::MakerDate:
            std::stable_sort(g_sorted_order.begin(), g_sorted_order.end(), [&](uint16_t a, uint16_t b) {
                const int ra = mfr_rank(get_mfr(a)), rb = mfr_rank(get_mfr(b));
                if (ra != rb) return ra < rb;
                return date_then_name(a, b);
            });
            break;
        default: // MakerName
            std::stable_sort(g_sorted_order.begin(), g_sorted_order.end(), [](uint16_t a, uint16_t b) {
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
    for (uint16_t idx : g_sorted_order) {
        if (idx == g_selected_index) {
            selected_visible = true;
            break;
        }
    }
    if (!selected_visible && !g_sorted_order.empty()) {
        g_selected_index = g_sorted_order[0];
    }
}

void set_selected(uint16_t idx) {
    g_selected_index = idx;
    g_sel_is_folder = false;
}

uint16_t wrap_index(int i, int n) {
    while (i < 0) i += n;
    i = i % n;
    return (uint16_t)i;
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

// The row waiting for its second A press to delete records (Resume = none).
SettingsRow g_confirm_row = SettingsRow::Resume;

// "A 0420  ·  B 0185" - a game's records (core/score_tracker.h), "-" for none yet.
std::string records_line(const std::string& ref) {
    std::string out;
    for (const ScoreRecord& r : score_tracker_records(ref)) {
        if (!out.empty()) out += "  \xC2\xB7  ";
        char num[16];
        if (r.best) snprintf(num, sizeof(num), "%04u", (unsigned)r.best);
        else snprintf(num, sizeof(num), "-");
        const std::string label = std::string(r.label) == "Game A" ? "A" : std::string(r.label) == "Game B" ? "B" : switch_tr(r.label);
        out += label + " " + num;
    }
    return out;
}

// "25 · 50 · 100  (3/6)" - the achievements got (core/score_tracker.h);
// with "When the Game Ends", plus the ones got in this session.
std::string achievements_line(const std::string& ref) {
    const uint8_t got = score_tracker_achievements(ref);
    std::string out;
    int n = 0;
    for (int i = 0; i < kAchievementCount; i++) {
        if (!(got & (1u << i))) continue;
        n++;
        if (i < 5) {
            std::string name = score_tracker_achievement_name(ref, i);
            const size_t sp = name.find(' ');
            if (!out.empty()) out += " \xC2\xB7 ";
            out += sp == std::string::npos ? name : name.substr(0, sp);
        }
    }
    if (got & (1u << 5)) out += (out.empty() ? "" : " \xC2\xB7 ") + switch_tr("Record");
    char count[24];
    snprintf(count, sizeof(count), "%s(%d/%d)", out.empty() ? "" : "  ", n, kAchievementCount);
    out += count;
    const uint8_t now = score_tracker_session_achievements();
    if (g_ach_notify == 1 && now) {
        int k = 0;
        for (int i = 0; i < kAchievementCount; i++) k += (now >> i) & 1;
        char nw[48];
        snprintf(nw, sizeof(nw), "  +%d ", k);
        out += nw + switch_tr("new");
    }
    return out;
}

const char* settings_row_label(SettingsRow row) {
    switch (row) {
        case SettingsRow::Rotation:              return "Game Rotation";
        case SettingsRow::MenuRotation:          return "Menu Rotation";
        case SettingsRow::MenuTouch:             return "Touch Controls";
        case SettingsRow::DateFormat:            return "Date Format";
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
        case SettingsRow::MenuStyle:                 return "Menu Style";
        case SettingsRow::MenuTheme:                 return "Menu Theme";
        case SettingsRow::MenuFont:                  return "Menu Font";
        case SettingsRow::Ghosting:                  return "LCD Ghosting";
        case SettingsRow::FlickerFilter:             return "Anti-Flicker Filter";
        case SettingsRow::PixelStyle:                return "Pixel Style";
        case SettingsRow::ScreenFilter:              return "Screen Filter";
        case SettingsRow::FilterStrength:            return "Filter Strength";
        case SettingsRow::MakerOrder:                return "Maker Order";
        case SettingsRow::MakerColors:               return "Maker Colors";
        case SettingsRow::Records:                   return "High Scores";
        case SettingsRow::RecordHud:                 return "Record On Screen";
        case SettingsRow::RecordsList:               return "Show Records in List";
        case SettingsRow::RecordsClearAll:           return "Clear All Records";
        case SettingsRow::RecordsGame:               return "High Scores";
        case SettingsRow::AchNotify:                 return "Achievement Alerts";
        case SettingsRow::AlertColor:                return "Alert Colour";
        case SettingsRow::AchGame:                   return "Achievements";
        case SettingsRow::TimeSetMode:                return "Time Set Mode";
        case SettingsRow::PetTime:                    return "Pet Time";
        case SettingsRow::PetTimeCap:                 return "Catch-Up Limit";
        case SettingsRow::PetTimeCapCustomEntry:      return "Enter Custom Hours";
        case SettingsRow::PetTimeDisplay:             return "Catch-Up Display";
        case SettingsRow::LogFile:                   return "Log File";
        case SettingsRow::SplashScreen:              return "Splash Screen";
        case SettingsRow::ThemesSub:                 return "Themes  >";
        case SettingsRow::MusicSub:                  return "Music  >";
        case SettingsRow::PauseCombo:                return "Open Pause Menu";
        case SettingsRow::OnScreenKeypad:            return "On-Screen Keypad";
        case SettingsRow::MenuMusic:                 return "Background Music";
        case SettingsRow::MenuMusicTrack:            return "Track";
        case SettingsRow::MenuMusicVolume:           return "Music Volume";
        case SettingsRow::MenuMusicDuck:             return "Lower With Beeps";
        case SettingsRow::ScreenPalette:             return "Screen Palette";
        case SettingsRow::Transition:                return "Transition";
        case SettingsRow::ScreenSize:                return "Screen Size";
        case SettingsRow::Resume:                    return "Resume";
        case SettingsRow::Restart:                   return "Restart";
        case SettingsRow::LinkConnect:               return link_session_active() ? "Disconnect" : "Connect to Another Game";
        case SettingsRow::LinkSound:                 return "Sound From";
        case SettingsRow::LinkSoundButton:           return "Sound Switch Button";
        case SettingsRow::VibrationGame:             return "Game Vibration";
        case SettingsRow::VibrationPets:             return "Pet Alert Vibration";
        case SettingsRow::VibrationMenu:             return "Menu Vibration";
        case SettingsRow::VibrationStrength:         return "Vibration Strength";
        case SettingsRow::PetAlertLight:             return "Pet Alert Light";
        case SettingsRow::MotionPedometer:           return "Pedometer: Shake the Controller";
        case SettingsRow::PedometerSensitivity:      return "Pedometer Sensitivity";
        case SettingsRow::MotionTapSensor:           return "Tap Sensor: Knock the Controller";
        case SettingsRow::TapSensitivity:            return "Tap Sensor Sensitivity";
        case SettingsRow::FastForward:               return "Fast-Forward";
        case SettingsRow::FastForwardSpeed:          return "Fast-Forward Speed";
        case SettingsRow::ButtonResponse:            return "Button Response";
        case SettingsRow::Rewind:                    return "Rewind";
        case SettingsRow::RewindLength:              return "Rewind Length";
        case SettingsRow::IrSensor:                  return "IR Sensor: Joy-Con Camera";
        case SettingsRow::IrSensitivity:             return "IR Sensor Sensitivity";
        case SettingsRow::Language:                  return "Language";
        case SettingsRow::SaveState:                 return "Save State";
        case SettingsRow::LoadState:                 return "Load State";
        case SettingsRow::SimulateTime:              return "Simulate Time";
        case SettingsRow::UndoSimulate:              return "Undo Simulate";
        case SettingsRow::Volume:                    return "Volume";
        case SettingsRow::Mute:                      return "Mute";
        case SettingsRow::SoundStyle:                return "Sound Style";
        case SettingsRow::GameComSound:              return "Game.com Sound";
        case SettingsRow::GameComSpeed:              return "Game.com Speed";
        case SettingsRow::GameComNoise:              return "Noise Channel (approx.)";
        case SettingsRow::GameComPalette:            return "Game.com Palette";
        case SettingsRow::GameComGhost:              return "Game.com Ghosting";
        case SettingsRow::RZoneStyle:                return "R-Zone Screen";
        case SettingsRow::AdvVisionScreen:           return "Adventure Vision Screen";
        case SettingsRow::TamaGoFigure:              return "Gotchi Figure";
        case SettingsRow::GameMasterSpeed:           return "Game Master Speed";
        case SettingsRow::MenuSounds:                return "Menu Sounds";
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
        case SettingsRow::MenuRotation:
            return rotation_label(g_menu_rotation);
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
        case SettingsRow::MenuStyle:
            return menu_style_label(g_menu_style);
        case SettingsRow::MenuTheme:
            return kMenuThemes[g_menu_theme < kNumMenuThemes ? g_menu_theme : 0].name;
        case SettingsRow::MenuFont:
            return switch_ui_font_name(g_menu_font);
        case SettingsRow::Ghosting: {
            static const char* const kGhost[] = {"Off", "Low", "Medium", "High"};
            return kGhost[g_ghosting <= 3 ? g_ghosting : 0];
        }
        case SettingsRow::PixelStyle:
        {
            static const char* const kPixel[] = {"Frame and Centre", "Filled", "Solid"};
            // Game Pocket Computer: no Classic - it reads as Filled
            if (g_game && is_dot_matrix_ref(g_game->ref)) return g_pixel_style == 2 ? "Solid" : "Filled";
            return kPixel[g_pixel_style <= 2 ? g_pixel_style : 0];
        }
        case SettingsRow::FlickerFilter: {
            static const char* const kFlicker[] = {"Auto", "On", "Off"};
            return kFlicker[g_flicker_filter <= 2 ? g_flicker_filter : 0];
        }
        case SettingsRow::ScreenFilter:
            return kScreenFilterNames[g_screen_filter < kNumScreenFilters ? g_screen_filter : 0];
        case SettingsRow::FilterStrength: {
            static const char* const kStr[] = {"Low", "Medium", "High"};
            return kStr[g_filter_strength <= 2 ? g_filter_strength : 1];
        }
        case SettingsRow::MakerOrder:
            return maker_order_label(g_maker_order);
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
        case SettingsRow::SplashScreen:
            return g_splash_screen ? "On" : "Off";
        case SettingsRow::MakerColors:
            return g_maker_colors ? "On" : "Off";
        case SettingsRow::MenuTouch:
            return g_menu_touch ? "On" : "Off";
        case SettingsRow::DateFormat: {
            static const char* const kF[3] = {"YYYY-MM-DD", "DD/MM/YYYY", "MM/DD/YYYY"};
            return kF[g_date_format <= 2 ? g_date_format : 0];
        }
        case SettingsRow::Records:
            return g_records_enabled ? "On" : "Off";
        case SettingsRow::RecordHud:
            return g_record_hud == 2 ? "Always" : g_record_hud == 1 ? "When Beaten" : "Off";
        case SettingsRow::RecordsList:
            return g_records_in_list ? "On" : "Off";
        case SettingsRow::RecordsClearAll:
            return g_confirm_row == row ? switch_tr("Press A again to delete") : std::string();
        case SettingsRow::RecordsGame:
            if (g_confirm_row == row) return switch_tr("Press A again to delete");
            return g_game ? records_line(g_game->ref) : std::string();
        case SettingsRow::AchNotify:
            return g_ach_notify == 2 ? "Off" : g_ach_notify == 1 ? "When the Game Ends" : "During the Game";
        case SettingsRow::AlertColor:
            return g_alert_color ? "Theme" : "Golden";
        case SettingsRow::AchGame:
            return g_game ? achievements_line(g_game->ref) : std::string();
        case SettingsRow::ScreenPalette:
            return kScreenPalettes[g_screen_palette < kNumScreenPalettes ? g_screen_palette : 0].name;
        case SettingsRow::Transition:
            return transition_name(g_transition);
        case SettingsRow::ScreenSize: {
            static const char* const kSizes[kNumScreenSizes] = {"Small (70%)", "Medium (85%)", "Normal (100%)",
                                                                "Large (115%)", "Stretched (16:9)", "Pixel Exact"};
            return kSizes[g_screen_size < kNumScreenSizes ? g_screen_size : 2];
        }
        case SettingsRow::VibrationGame: {
            static const char* const kV[3] = {"Off", "With Sound", "Strong Events"};
            return kV[g_vibration_game <= 2 ? g_vibration_game : 0];
        }
        case SettingsRow::VibrationPets:
            return g_vibration_pets ? "On" : "Off";
        case SettingsRow::VibrationMenu:
            return g_vibration_menu ? "On" : "Off";
        case SettingsRow::VibrationStrength: {
            static const char* const kS[3] = {"Soft", "Medium", "Strong"};
            return kS[g_vibration_strength <= 2 ? g_vibration_strength : 1];
        }
        case SettingsRow::PetAlertLight:
            return g_pet_alert_light ? "On" : "Off";
        case SettingsRow::MotionPedometer:
            return g_motion_pedometer ? "On" : "Off";
        case SettingsRow::MotionTapSensor:
            return g_motion_tap_sensor ? "On" : "Off";
        case SettingsRow::FastForward:
            return time_control_button_label(g_fastforward_button);
        case SettingsRow::LinkSound:
            return link_session_sound_player() == 1 ? "Player 2" : "Player 1";
        case SettingsRow::LinkSoundButton:
            return time_control_button_label(g_link_sound_button);
        case SettingsRow::ButtonResponse: {
            static const char* const kR[4] = {"Original", "Fast", "Very Fast", "Instant"};
            return kR[g_button_response <= 3 ? g_button_response : 2];
        }
        case SettingsRow::FastForwardSpeed: {
            static const char* const kF[3] = {"2x", "4x", "8x"};
            return kF[g_fastforward_speed <= 2 ? g_fastforward_speed : 1];
        }
        case SettingsRow::Rewind:
            return time_control_button_label(g_rewind_button);
        case SettingsRow::RewindLength: {
            static const char* const kR[4] = {"5 s", "10 s", "30 s", "60 s"};
            return kR[g_rewind_length <= 3 ? g_rewind_length : 2];
        }
        case SettingsRow::TapSensitivity: {
            static const char* const kT[3] = {"Low", "Medium", "High"};
            return kT[g_tap_sensitivity <= 2 ? g_tap_sensitivity : 1];
        }
        case SettingsRow::IrSensor: {
            static const char* const kM[3] = {"Off", "Auto (saves battery)", "Always"};
            return kM[g_ir_sensor <= 2 ? g_ir_sensor : 1];
        }
        case SettingsRow::IrSensitivity: {
            static const char* const kI[3] = {"Low", "Medium", "High"};
            return kI[g_ir_sensitivity <= 2 ? g_ir_sensitivity : 1];
        }
        case SettingsRow::PedometerSensitivity: {
            static const char* const kP[3] = {"Low", "Medium", "High"};
            return kP[g_pedometer_sensitivity <= 2 ? g_pedometer_sensitivity : 1];
        }
        case SettingsRow::Language:
            return switch_language_name(g_language);
        case SettingsRow::Volume:
            return std::to_string((unsigned)g_sound_volume) + " %";
        case SettingsRow::Mute:
            return g_sound_mute ? "On" : "Off";
        case SettingsRow::SoundStyle: {
            static const char* const kStyles[kNumSoundStyles] = {"Clean", "Piezo", "Octave Down", "Flute", "Radio"};
            return kStyles[g_sound_style < kNumSoundStyles ? g_sound_style : 0];
        }
        case SettingsRow::GameComSound:    return g_tgc_sound ? "Basic" : "Full";
        case SettingsRow::GameComSpeed:    return g_tgc_speed ? "+20%" : "Normal";
        case SettingsRow::GameComNoise:    return g_tgc_noise ? "On" : "Off";
        case SettingsRow::GameComPalette:  return g_tgc_palette ? "Green" : "Real";
        case SettingsRow::AdvVisionScreen: return g_eav_screen ? "Smooth" : "LEDs";
        case SettingsRow::TamaGoFigure: {
            return kTmgFigureNames[g_tmg_figure <= 12 ? g_tmg_figure : 0];
        }
        case SettingsRow::RZoneStyle: {
            static const char* const k[3] = {"White", "Red", "LCD (no light)"};
            return k[g_rz_style <= 2 ? g_rz_style : 0];
        }
        case SettingsRow::GameComGhost: {
            static const char* const k[3] = {"Off", "Soft", "Strong"};
            return k[g_tgc_ghost <= 2 ? g_tgc_ghost : 0];
        }        case SettingsRow::GameMasterSpeed: return g_hgm_speed ? "Fast (x2)" : "Normal";
        case SettingsRow::MenuSounds:
            return g_menu_sounds ? "On" : "Off";
        case SettingsRow::MenuMusic: {
            static const char* const kM[kNumMenuMusicChoices] = {"Off", "Stroll", "Arcade", "LCD Clock", "Nocturne", "Waltz",
                                                                 "Funk", "Adventure", "Rain", "Random", "Custom Music"};
            return kM[g_menu_music < kNumMenuMusicChoices ? g_menu_music : 0];
        }
        case SettingsRow::MenuMusicTrack:
            if (menu_music_files().empty()) return "No files";
            return g_menu_music_track.empty() ? "All (random)" : g_menu_music_track;
        case SettingsRow::MenuMusicVolume:
            return std::to_string((unsigned)g_menu_music_volume) + " %";
        case SettingsRow::PauseCombo:
            return pause_combo_label(g_pause_combo);
        case SettingsRow::OnScreenKeypad:
            return g_onscreen_keypad ? "On" : "Off";
        case SettingsRow::MenuMusicDuck: {
            static const char* const kD[4] = {"Not at all", "A little", "Medium", "A lot"};
            return kD[g_menu_music_duck < 4 ? g_menu_music_duck : 2];
        }
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
float settings_sidebar_w(int logical_w) { return logical_w >= 1000 ? 290.0f : 200.0f; }

// Section list on the left of the game-list settings screen, plus the
// current section's title above its rows.
void draw_text_fit(float x, float y, float size, float max_w, float r, float g, float b, float a, std::string s, bool bold);

void settings_go_section(int i);

void render_settings_sidebar(int logical_w, int logical_h) {
    const float w = settings_sidebar_w(logical_w);
    switch_ui_draw_rect(0.0f, (float)kTopBarH, w, (float)(logical_h - kTopBarH - kBottomBarH), kPanelR, kPanelG, kPanelB, 1.0f);
    std::vector<const char*> secs;
    if (g_settings_from_gameplay) {
        for (PauseSection p : pause_sections()) secs.push_back(pause_section_label(p));
    } else {
        for (SettingsSection s : settings_sections()) secs.push_back(settings_section_label(s));
    }
    const int cur = g_settings_from_gameplay ? g_pause_section : g_settings_section;
    const float size = logical_w >= 1000 ? 26.0f : 21.0f;
    for (size_t i = 0; i < secs.size(); i++) {
        const float y = (float)kTopBarH + 22.0f + (float)i * 58.0f;
        const bool sel = ((int)i == cur);
        menu_touch_item(0.0f, y - 18.0f, w, 58.0f, sel, [i] { settings_go_section((int)i); }, 0);
        if (sel) {
            switch_ui_draw_rect(12.0f, y - 10.0f, w - 24.0f, size + 20.0f, kSelR, kSelG, kSelB, 1.0f);
            switch_ui_draw_rect(12.0f, y - 10.0f, 5.0f, size + 20.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        }
        // Shrunk to fit the sidebar when a name is long ("Funciones del mando").
        draw_text_fit(30.0f, y, size, w - 30.0f - 16.0f, kTextR, kTextG, kTextB, sel ? 1.0f : 0.7f, secs[i], /*bold=*/true);
    }
    switch_ui_draw_text(w + 36.0f, (float)kTopBarH + 16.0f, 28.0f, kAccentR, kAccentG, kAccentB, 1.0f,
                        g_settings_from_gameplay ? pause_section_label(current_pause_section())
                                                 : (g_themes_sub ? "Menu  >  Themes" : g_music_sub ? "Menu  >  Music" : settings_section_label(current_settings_section())),
                        /*bold=*/true);
}

// Themes section: a small card per theme (its background, a tile, the
// accent and highlight colours, its name), the current one outlined.
// Top of the explanation box drawn this frame (0 = none) - the theme
// previews stop above it.
float g_help_box_top = 0.0f;

void render_theme_previews(int logical_w, int logical_h) {
    const float x0 = settings_sidebar_w(logical_w) + 36.0f;
    const float avail_w = (float)logical_w - x0 - 24.0f;
    const int cols = logical_w >= 1000 ? 4 : 2;
    const float gap = 14.0f;
    const float cw = (avail_w - gap * (cols - 1)) / cols, ch = 84.0f;
    const float y0 = (float)kTopBarH + 62.0f + 56.0f * 2.0f + 20.0f;
    // Only as many rows as fit, scrolled to keep the current theme in view.
    // Above the explanation box when it's showing (Y), else down to the bar.
    const float bottom = g_help_box_top > 0.0f ? g_help_box_top - 10.0f : (float)(logical_h - kBottomBarH);
    const int fit_rows = std::max(1, (int)((bottom - y0 + gap) / (ch + gap)));
    const int total_rows = (kNumMenuThemes + cols - 1) / cols;
    const int first_row = std::max(0, std::min((int)g_menu_theme / cols - fit_rows / 2, total_rows - fit_rows));
    for (int i = first_row * cols; i < kNumMenuThemes && i < (first_row + fit_rows) * cols; i++) {
        const MenuTheme& t = kMenuThemes[i];
        const float x = x0 + (i % cols) * (cw + gap), y = y0 + (i / cols - first_row) * (ch + gap);
        float r, g, b;
        if (i == g_menu_theme) switch_ui_draw_rect(x - 4.0f, y - 4.0f, cw + 8.0f, ch + 8.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        set_rgb(t.bg, r, g, b);
        switch_ui_draw_rect(x, y, cw, ch, r, g, b, 1.0f);
        set_rgb(t.tile, r, g, b);
        switch_ui_draw_rect(x + 10.0f, y + 38.0f, cw - 20.0f, 18.0f, r, g, b, 1.0f);
        set_rgb(t.sel, r, g, b);
        switch_ui_draw_rect(x + 10.0f, y + 60.0f, cw - 20.0f, 16.0f, r, g, b, 1.0f);
        set_rgb(t.highlight, r, g, b);
        switch_ui_draw_rect(x + 10.0f, y + 60.0f, 5.0f, 16.0f, r, g, b, 1.0f);
        set_rgb(t.accent, r, g, b);
        switch_ui_draw_rect(x + 18.0f, y + 51.0f, (cw - 36.0f) * 0.55f, 3.0f, r, g, b, 1.0f);
        set_rgb(t.text, r, g, b);
        switch_ui_draw_text(x + 10.0f, y + 10.0f, 20.0f, r, g, b, 1.0f, t.name, /*bold=*/true);
    }
}

// The explanation box under the rows for the selected option (two lines,
// translated like the rest). nullptr = no box for that row.
const char* const* settings_row_help(SettingsRow row) {
    static const char* const kPetLight[2] = {"When a pet beeps on its own, the HOME light of the right Joy-Con",
                                             "pulses until you press any button."};
    static const char* const kPedometer[2] = {"Each shake of the controller is one step (Pocket Pikachu, D-Tector,",
                                              "D-Power). The X button still works."};
    static const char* const kPedoSens[2] = {"How hard you have to shake for a step: High counts gentle",
                                             "movements, Low only strong ones."};
    static const char* const kPause[2] = {"The buttons that open the pause menu during a game. They can't be",
                                          "given to game functions in pause menu > Controls."};
    static const char* const kKeypad[2] = {"Shows the game's keypad next to the screen: tap it in handheld",
                                           "mode, or press R3 to use it with the controller."};
    static const char* const kTap[2] = {"Tamagotchis with a tap sensor (Tenshitchi, Umi/Mori de Hakken, Genjintchi,",
                                        "TamaOtch) feel a knock on the controller or a touch on the screen. X still works."};
    static const char* const kTapSens[2] = {"How hard a knock has to be: High notices light taps,",
                                            "Low only firm ones (so walking around doesn't trigger it)."};
    static const char* const kIr[2] = {"D-Tector: pass a hand in front of the right Joy-Con's IR camera (Wave) or hold it (Sensor).",
                                       "Auto: only in the D-Tector modes that use the sensor (and 1 min without a press). Always: all game."};
    static const char* const kIrSens[2] = {"How close the hand has to be: High notices it from further away,",
                                           "Low only very close (if light in the room sets it off)."};
    static const char* const kFF[2] = {"Hold this button in a game to run it faster (sound muted).",
                                       "Off by default. Only free buttons are offered; the game no longer gets it."};
    static const char* const kBtnResp[4][2] = {
        {"As the real device: it answers a press after ~200 ms, while it",
         "finishes drawing on its slow clock."},
        {"After each press the chip works 2x faster for a moment: about 100 ms.",
         "An animation may speed up a little right after a press. The clock stays real."},
        {"After each press the chip works 4x faster for a moment: about 60 ms.",
         "An animation may speed up a little right after a press. The clock stays real."},
        {"After each press the chip works 8x faster for a moment: about 30 ms.",
         "An animation may speed up a little right after a press. The clock stays real."},
    };
    static const char* const kFFSpeed[2] = {"How much faster the game runs while you hold the fast-forward button.", ""};
    static const char* const kRW[2] = {"Hold this button in a game to go back in time, step by step.",
                                       "Off by default. Only free buttons are offered; the game no longer gets it."};
    static const char* const kRWLen[2] = {"How far back you can rewind. Longer keeps more snapshots in memory.",
                                          "McDonald's games can't be rewound."};
    static const char* const k_PetDisplay2[2] = {"Clock: a clock face spins through the hours your pet lives,",
                                                 "with its time in the middle and a ring for the progress."};
    static const char* const k_PetDisplay3[2] = {"Day & Night: the sun and the moon cross the sky as the hours go by,",
                                                 "with a dot for each day."};
    static const char* const k_PetDisplay4[2] = {"Screen & Bar: you watch the pet's own screen at high speed,",
                                                 "with the progress bar over it."};
    static const char* const kDuck[2] = {"How much the music drops while a menu beep plays,",
                                         "so the two don't clash."};
    static const char* const k_Rotation[2] = {"Turns the game screen, for playing with the console upright.",
        "The menus have their own Menu Rotation (Settings > Menu)."};
    static const char* const k_ScreenSize[2] = {"How big the game is drawn. Normal is the largest size without distortion.",
        "Stretched fills the screen (distorts); Pixel Exact keeps sharp, whole pixels."};
    static const char* const k_BackgroundArt[2] = {"The printed picture behind the LCD (scenery, labels). Hidden leaves only the",
        "segments on a plain colour. Panorama and Table Top games always keep it."};
    static const char* const k_PanelGap[2] = {"Space between the two screens of the double-screen games (Multi Screen),",
        "like the hinge of the real console. At 0% the two screens touch."};
    static const char* const k_ConsoleArt[2] = {"Shows the console's case (the plastic shell artwork) under one-screen games.",
        "Off: the game screen uses that space instead and looks bigger."};
    static const char* const k_ScreenPalette[2] = {"Recolours the whole LCD at once (background and segments): backlit, Game Boy...",
        "Original keeps each game's own colours."};
    static const char* const k_Background[2] = {"The colour behind the segments, like the LCD's glass.",
        "Custom lets you type any colour (hex)."};
    static const char* const k_SegmentColor[2] = {"The colour of the lit segments (the drawings that move on the LCD).",
        "Auto uses each game's own."};
    static const char* const k_Segments[2] = {"How visible the switched-off segments are, as on a real LCD",
        "where you can faintly see them. 0% hides them completely."};
    static const char* const kFlickerHelp[3][2] = {
        {"On for Game & Watch style games (and Big Cat 9 in 1), off for virtual pets",
         "and watches (Tamagotchi, Digimon, Pikachu...): it would erase moving dots."},
        {"A dot only shows once it stays lit for two pictures: hides flicker,",
         "but on screens that scroll (a pet's clock) the moving dots can vanish."},
        {"Every dot shows as soon as it lights up. Moving pictures stay whole,",
         "though a game whose segments flicker may flicker on screen."},
    };
    static const char* const k_Ghosting[2] = {"Segments fade out slowly instead of switching off at once,",
        "like the slow response of a real LCD. Off is the original sharp look."};
    static const char* const k_ScreenFilter[2] = {"An effect over the game screen: scanlines, LCD grid, CRT, colour tints...",
        "Never drawn over the console's case. Filter Strength sets how strong."};
    static const char* const k_FilterStrength[2] = {"How strong the screen filter is.",
        ""};
    static const char* const k_CloneFilter0[2] = {"Show All: every game in the pack, including near-identical versions.",
        "Hide Duplicates hides re-catalogued copies; Hide All Clones hides every clone too."};
    static const char* const k_CloneFilter1[2] = {"Hide Duplicates: hides only the games that are really the same release",
        "listed twice (e.g. two board revisions of Helmet). Nothing else changes."};
    static const char* const k_CloneFilter2[2] = {"Hide All Clones: also hides every game marked as a clone of another,",
        "including different games that reuse another's hardware (e.g. Elektronika ones)."};
    static const char* const k_ExperimentalGames[2] = {"Shows the games added later, less tested: Tiger, Konami, Nelsonic, V-Tech,",
        "McDonald's and others. They may have bugs. Saves are never deleted."};
    static const char* const k_ExperimentalFeatures[2] = {"Turns on features still being tested: Virtual Pets settings, the on-screen",
        "keypad, Altair's dosimeter, the Tronica calculator keypads..."};
    static const char* const k_LogFile[2] = {"Writes a log to yokoi_logs/, useful for reporting a problem.",
        "Normally off; turn it on only when you want to send a log."};
    static const char* const k_LoadState0[2] = {"Ask: when a game has a save, you choose to continue or start again.",
        ""};
    static const char* const k_LoadState1[2] = {"Auto-Load: a game with a save continues straight away where you left it.",
        ""};
    static const char* const k_LoadState2[2] = {"Never Load: games always start fresh. Your saves are kept, not deleted,",
        "and leaving a game still saves it."};
    static const char* const k_TimeSet0[2] = {"Automatic: games that block until you set their clock get it set and",
        "confirmed for you, so they're playable at once."};
    static const char* const k_TimeSet1[2] = {"Manual: nothing is pressed for you - set and confirm the clock yourself,",
        "like on the real console. Applies the next time a game is loaded."};
    static const char* const k_PetTime0[2] = {"Real Time: the pet lives the time you were away (up to the Catch-Up Limit):",
        "it ages and gets hungry, then its clock is set to now."};
    static const char* const k_PetTime1[2] = {"Clock Sync: only its clock is set to the real time when you come back.",
        "The pet doesn't age or get hungry while you're away."};
    static const char* const k_PetTime2[2] = {"Paused: nothing happens while you're away - the pet and its clock",
        "pick up exactly where you left them."};
    static const char* const k_PetCap[2] = {"The most time a pet lives through when you come back (Real Time).",
        "Anything beyond it is skipped: the clock jumps, the pet doesn't age through it."};
    static const char* const k_PetDisplay0[2] = {"Progress Bar: a screen with a bar while the pet catches up on the time away.",
        ""};
    static const char* const k_PetDisplay1[2] = {"Fast-Forward: you watch the pet's own screen run at high speed",
        "while it catches up, with the time left shown small."};
    static const char* const k_Sound0[2] = {"Clean: the buzzer as the emulator plays it, clear and smooth.",
        ""};
    static const char* const k_Sound1[2] = {"Piezo: sounds like the small piezo speaker of the real console.",
        ""};
    static const char* const k_Sound2[2] = {"Octave Down: the same tunes one octave lower, softer on the ears.",
        ""};
    static const char* const k_Sound3[2] = {"Flute: a rounder, flute-like tone instead of the square buzz.",
        ""};
    static const char* const k_Sound4[2] = {"Radio: as if it came out of an old radio, with a little hiss.",
        ""};
    static const char* const k_CustomColor[2] = {"Opens the keyboard to type a colour in hex (RRGGBB), e.g. B0BA9E.",
        ""};
    static const char* const k_CustomValue[2] = {"Opens the keyboard to type an exact value.",
        ""};
    static const char* const k_CustomHours[2] = {"Opens the keyboard to type the limit in hours (1-9999).",
        ""};
    static const char* const k_MenuStyle[2] = {"How the game list starts: Classic shows a grid of makers;",
        "Carousel shows them one at a time, big, with their game count."};
    static const char* const k_MenuStyleList[2] = {"How the game list starts: Classic shows a grid of makers;",
        "List puts the makers in a column, with the chosen one's games beside it."};
    static const char* const k_MakerOrder[2] = {"Order of the makers: A-Z, the ones with most games first,",
        "or by era (the year of each maker's oldest game)."};
    static const char* const k_MenuTheme[2] = {"The colours of the menus (background, tiles, highlight).",
        "The cards below preview each theme."};
    static const char* const k_MenuFont[2] = {"The letters used in the menus. System is the Switch's own font.",
        ""};
    static const char* const k_ThemesSub[2] = {"Opens the menu themes and fonts.",
        ""};
    static const char* const k_MusicSub[2] = {"Opens the background music: built-in tunes or your own",
        "from the SD card, its volume and how it dips for the beeps."};
    static const char* const k_MenuMusic[2] = {"Music in the game list and settings (never in game or in the pause menu).",
        "Random picks a tune each time; Custom Music plays your files from yokoi_music/."};
    static const char* const k_MenuMusicTrack[2] = {"Which of your files plays (.ogg or 16-bit .wav in sdmc:/switch/yokoi/yokoi_music/).",
        "All (random) plays one after another in random order."};
    static const char* const k_MenuMusicVolume[2] = {"The music's own volume. It also follows Sound > Volume and Mute.",
        ""};
    static const char* const k_MenuRotation[2] = {"Turns the menus (game list, settings, pause), separately",
        "from the game's own rotation."};
    static const char* const k_Transition[2] = {"The animation when you move from one game to the next",
        "in the one-at-a-time list and the carousel. Random changes it every time."};
    static const char* const k_Records[2] = {"Keeps the best score of the games that show one (Game & Watch style so far),",
        "for Game A and Game B. Off: no records, nothing on screen."};
    static const char* const k_RecordHud0[2] = {"Nothing over the game. The records are still kept.", ""};
    static const char* const k_RecordHud1[2] = {"A \"New high score!\" banner for 2 seconds when you beat your record.", ""};
    static const char* const k_RecordHud2[2] = {"The record of the mode you play, top right, the whole game,",
        "and the banner when you beat it."};
    static const char* const k_RecordsList[2] = {"The game's records under its name in the game list.", ""};
    static const char* const k_RecordsClearAll[2] = {"Deletes the records of every game. Press A twice.", ""};
    static const char* const k_RecordsGame[2] = {"This game's best scores. Press A twice to delete them.", ""};
    static const char* const k_AchNotify0[2] = {"A banner at the bottom for 3 seconds the moment you get an achievement:",
        "25, 50, 100, 200 and 500 points, and beating your record."};
    static const char* const k_AchNotify1[2] = {"No interruptions: the achievements you got show one after another",
        "when the game ends, and in the pause menu."};
    static const char* const k_AchNotify2[2] = {"No banner. The achievements are still kept; see them in the pause menu.", ""};
    static const char* const k_AchGame[2] = {"25, 50, 100, 200 and 500 points (x10 on the VFD tabletop games) and beating",
        "your record. They are deleted with the game's high scores."};
    static const char* const k_DateFormat[2] = {"How the game lists write a game's date: year-month-day,",
        "day/month/year or month/day/year."};
    static const char* const k_MenuTouch[2] = {"Handheld: tap an item to choose it and tap it again (or a hint at the bottom) to",
        "use it; swipe left/right to go to the next one, drag up/down to move through lists."};
    static const char* const k_MakerColors[2] = {"Gives each maker its own colour: a stripe on the maker tiles, a band on the",
        "carousel cards, the frame in the one-at-a-time list. It follows the theme."};
    static const char* const k_SplashScreen[2] = {"The short YOKOI animation with its piezo sound when the app starts.",
        "Skip it with +."};
    static const char* const k_Language[2] = {"The language of the menus.",
        ""};
    static const char* const k_MenuSounds[2] = {"Short beeps when you move and choose in the menus.",
        "They never play in a game."};
    static const char* const k_Volume[2] = {"The volume of everything: games, menu beeps and music.",
        ""};
    static const char* const k_Mute[2] = {"Silences everything at once without losing your volume setting.",
        ""};
    static const char* const k_VibGame0[2] = {"Off: games don't vibrate (pets can still, with Pet Alert Vibration).",
        ""};
    static const char* const k_VibGame1[2] = {"With Sound: the controller buzzes along with the game's buzzer.",
        ""};
    static const char* const k_VibGame2[2] = {"Strong Events: only long or low beeps vibrate - often a miss,",
        "a crash or a lost life."};
    static const char* const k_VibPets[2] = {"Virtual pets vibrate with their beeps even when Game Vibration is off.",
        "Handy with the volume down, so you notice when they call."};
    static const char* const k_VibMenu[2] = {"A short tap in the controller for each button in the menus.",
        ""};
    static const char* const k_VibStrength[2] = {"How strong every vibration is: Soft, Medium or Strong.",
        ""};
    static const char* const k_LinkConnect[2] = {"Plays two devices at once on this Switch, joined by their cable or infrared:",
        "Digimon battles, Tamagotchi V3 visits... Player 1 uses Left/Down/Right, player 2 Y/X/A."};
    static const char* const k_LinkSound[2][2] = {
        {"You hear player 1's device (left half). Only one can play at a time.", ""},
        {"You hear player 2's device (right half). Only one can play at a time.", ""},
    };
    static const char* const k_LinkSoundButton[2] = {"Each press makes you hear the other device - handy docked, where there's no",
        "touch screen. If a player's function is on that button (Controls), it does that instead."};
    static const char* const k_LinkDisconnect[2] = {"Separates the two devices. The second one's game is kept in its auto save",
        "(unless it was a copy of this one)."};
    static const char* const k_Restart[2] = {"Starts the game over, as if you switched it off and on. Whatever you",
        "haven't saved is lost; your save slots stay as they are."};
    static const char* const k_Resume[2] = {"Back to the game, exactly where you left it.",
        ""};
    static const char* const k_SaveState[2] = {"Saves the game right now in one of the slots, with a picture of the screen.",
        ""};
    static const char* const k_LoadStateRow[2] = {"Goes back to a saved game. The Auto save slot is the one",
        "made when you last left the game."};
    static const char* const k_SimulateTime[2] = {"The pet lives a few hours at high speed (it ages, gets hungry...).",
        "A copy is kept first, so Undo Simulate can bring it back."};
    static const char* const k_UndoSimulate[2] = {"Puts the pet back as it was before the last Simulate Time.",
        ""};
    static const char* const k_Dosimeter[2] = {"Altair's radiation dial: the reading the game's dosimeter sees.",
        ""};
    static const char* const k_ReturnToMenu[2] = {"Saves and closes the game, back to the game list.",
        ""};

    switch (row) {
        case SettingsRow::BackgroundCustomEntry:
        case SettingsRow::SegmentColorCustomEntry: return k_CustomColor;
        case SettingsRow::SegmentsCustomEntry:
        case SettingsRow::PanelGapCustomEntry:     return k_CustomValue;
        case SettingsRow::PetTimeCapCustomEntry:   return k_CustomHours;
        case SettingsRow::MenuStyle:       return g_menu_style == MenuStyle::List ? k_MenuStyleList : k_MenuStyle;
        case SettingsRow::MakerOrder:      return k_MakerOrder;
        case SettingsRow::MenuTheme:       return k_MenuTheme;
        case SettingsRow::MenuFont:        return k_MenuFont;
        case SettingsRow::ThemesSub:       return k_ThemesSub;
        case SettingsRow::MusicSub:        return k_MusicSub;
        case SettingsRow::MenuMusic:       return k_MenuMusic;
        case SettingsRow::MenuMusicTrack:  return k_MenuMusicTrack;
        case SettingsRow::MenuMusicVolume: return k_MenuMusicVolume;
        case SettingsRow::MenuRotation:    return k_MenuRotation;
        case SettingsRow::Transition:      return k_Transition;
        case SettingsRow::SplashScreen:    return k_SplashScreen;
        case SettingsRow::MakerColors:     return k_MakerColors;
        case SettingsRow::MenuTouch:       return k_MenuTouch;
        case SettingsRow::DateFormat:      return k_DateFormat;
        case SettingsRow::Records:         return k_Records;
        case SettingsRow::RecordHud:
            return g_record_hud == 2 ? k_RecordHud2 : g_record_hud == 1 ? k_RecordHud1 : k_RecordHud0;
        case SettingsRow::RecordsList:     return k_RecordsList;
        case SettingsRow::RecordsClearAll: return k_RecordsClearAll;
        case SettingsRow::RecordsGame:     return k_RecordsGame;
        case SettingsRow::AchNotify:
            return g_ach_notify == 2 ? k_AchNotify2 : g_ach_notify == 1 ? k_AchNotify1 : k_AchNotify0;
        case SettingsRow::AlertColor: {
            static const char* const k[2] = {"Colour of the \"New high score!\" and achievement alerts:", "Golden, or the colours of the menu theme (Settings > Themes)."};
            return k;
        }
        case SettingsRow::AchGame:         return k_AchGame;
        case SettingsRow::Language:        return k_Language;
        case SettingsRow::MenuSounds:      return k_MenuSounds;
        case SettingsRow::Volume:          return k_Volume;
        case SettingsRow::Mute:            return k_Mute;
        case SettingsRow::VibrationGame:
            return g_vibration_game == 1 ? k_VibGame1 : g_vibration_game == 2 ? k_VibGame2 : k_VibGame0;
        case SettingsRow::VibrationPets:     return k_VibPets;
        case SettingsRow::VibrationMenu:     return k_VibMenu;
        case SettingsRow::VibrationStrength: return k_VibStrength;
        case SettingsRow::Resume:            return k_Resume;
        case SettingsRow::Restart:           return k_Restart;
        case SettingsRow::LinkConnect:       return link_session_active() ? k_LinkDisconnect : k_LinkConnect;
        case SettingsRow::LinkSound:         return k_LinkSound[link_session_sound_player() == 1 ? 1 : 0];
        case SettingsRow::LinkSoundButton:   return k_LinkSoundButton;
        case SettingsRow::SaveState:         return k_SaveState;
        case SettingsRow::LoadState:         return k_LoadStateRow;
        case SettingsRow::SimulateTime:      return k_SimulateTime;
        case SettingsRow::UndoSimulate:      return k_UndoSimulate;
        case SettingsRow::DosimeterReading:  return k_Dosimeter;
        case SettingsRow::ReturnToMenu:      return k_ReturnToMenu;
        case SettingsRow::Rotation:       return k_Rotation;
        case SettingsRow::ScreenSize:     return k_ScreenSize;
        case SettingsRow::BackgroundArt:  return k_BackgroundArt;
        case SettingsRow::PanelGap:       return k_PanelGap;
        case SettingsRow::ConsoleArt:     return k_ConsoleArt;
        case SettingsRow::ScreenPalette:  return k_ScreenPalette;
        case SettingsRow::Background:     return k_Background;
        case SettingsRow::SegmentColor:   return k_SegmentColor;
        case SettingsRow::Segments:       return k_Segments;
        case SettingsRow::Ghosting:       return k_Ghosting;
        case SettingsRow::FlickerFilter:  return kFlickerHelp[g_flicker_filter <= 2 ? g_flicker_filter : 0];
        case SettingsRow::PixelStyle: {
            static const char* const k_PixelStyle[2] = {"How this screen's dots are drawn: Classic as the game's own art,",
                "Filled as solid squares with a gap, Solid as joined blocks."};
            return k_PixelStyle;
        }
        case SettingsRow::ScreenFilter:   return k_ScreenFilter;
        case SettingsRow::FilterStrength: return k_FilterStrength;
        case SettingsRow::CloneFilter:
            return g_clone_filter_mode == CloneFilterMode::HideDuplicates ? k_CloneFilter1
                 : g_clone_filter_mode == CloneFilterMode::HideAllClones  ? k_CloneFilter2 : k_CloneFilter0;
        case SettingsRow::ExperimentalGames:    return k_ExperimentalGames;
        case SettingsRow::ExperimentalFeatures: return k_ExperimentalFeatures;
        case SettingsRow::LogFile:              return k_LogFile;
        case SettingsRow::LoadStateBehavior:
            return g_load_state_behavior == LoadStateBehavior::AutoLoad  ? k_LoadState1
                 : g_load_state_behavior == LoadStateBehavior::NeverLoad ? k_LoadState2 : k_LoadState0;
        case SettingsRow::TimeSetMode:
            return g_time_set_mode == TimeSetMode::Manual ? k_TimeSet1 : k_TimeSet0;
        case SettingsRow::PetTime:
            return g_pet_time_mode == PetTimeMode::ClockSync ? k_PetTime1
                 : g_pet_time_mode == PetTimeMode::Paused    ? k_PetTime2 : k_PetTime0;
        case SettingsRow::PetTimeCap:     return k_PetCap;
        case SettingsRow::PetTimeDisplay:
            switch (g_pet_time_display) {
                case PetTimeDisplay::FastForward: return k_PetDisplay1;
                case PetTimeDisplay::Clock:       return k_PetDisplay2;
                case PetTimeDisplay::DayNight:    return k_PetDisplay3;
                case PetTimeDisplay::ScreenBar:   return k_PetDisplay4;
                default:                          return k_PetDisplay0;
            }
        case SettingsRow::SoundStyle: {
            static const char* const* const kStyles[5] = {k_Sound0, k_Sound1, k_Sound2, k_Sound3, k_Sound4};
            return kStyles[g_sound_style < 5 ? g_sound_style : 0];
        }
        case SettingsRow::GameComSound: {
            static const char* const k[2] = {"Full: voices, music and tones, as in MAME.", "Basic: the simple buzzer."};
            return k;
        }
        case SettingsRow::GameComSpeed: {
            static const char* const k[2] = {"Normal: MAME's speed. +20%: closer to the real console,", "but some countdowns run fast."};
            return k;
        }
        case SettingsRow::GameComNoise: {
            static const char* const k[2] = {"Explosions and hits on the noise channel.", "An approximation: MAME doesn't have it."};
            return k;
        }
        case SettingsRow::GameComPalette: {
            static const char* const k[2] = {"Real: the greys of the real screen.", "Green: green-tinted screen."};
            return k;
        }
        case SettingsRow::GameComGhost: {
            static const char* const k[2] = {"The real screen is slow: moving things leave a trail.", "Soft or Strong imitate it and make motion look smoother."};
            return k;
        }
        case SettingsRow::TamaGoFigure: {
            static const char* const k[2] = {"The figure standing on the Tama-Go and its contacts (CD1 / CD2).", "A opens the list."};
            return k;
        }
        case SettingsRow::AdvVisionScreen: {
            static const char* const k[2] = {"LEDs: the rows of LEDs apart, as on the real console.", "Smooth: one continuous picture, no gaps."};
            return k;
        }
        case SettingsRow::RZoneStyle: {
            static const char* const k[2] = {"White: crimson, lit near white. Red: dark, lit bright red.", "LCD (no light): plain LCD, for your own palette and colours."};
            return k;
        }
        case SettingsRow::GameMasterSpeed: {
            static const char* const k[2] = {"Normal: MAME's speed.", "Fast (x2): as a real console on video."};
            return k;
        }
        case SettingsRow::PetAlertLight:        return kPetLight;
        case SettingsRow::MotionPedometer:      return kPedometer;
        case SettingsRow::PedometerSensitivity: return kPedoSens;
        case SettingsRow::MotionTapSensor:      return kTap;
        case SettingsRow::FastForward:          return kFF;
        case SettingsRow::FastForwardSpeed:     return kFFSpeed;
        case SettingsRow::ButtonResponse:       return kBtnResp[g_button_response <= 3 ? g_button_response : 2];
        case SettingsRow::Rewind:               return kRW;
        case SettingsRow::RewindLength:         return kRWLen;
        case SettingsRow::TapSensitivity:       return kTapSens;
        case SettingsRow::IrSensor:             return kIr;
        case SettingsRow::IrSensitivity:        return kIrSens;
        case SettingsRow::PauseCombo:           return kPause;
        case SettingsRow::OnScreenKeypad:       return kKeypad;
        case SettingsRow::MenuMusicDuck:        return kDuck;
        default:                                return nullptr;
    }
}

void render_settings_rows(int logical_w, int logical_h) {
    const std::vector<SettingsRow> rows = build_settings_rows();
    const float label_size = 30.0f;
    const float row_h = 56.0f;
    // Trimmed from the original +48/-12 margins (which left room to spare)
    // to reclaim a bit more vertical budget for content rows now that
    // scrolling needs some of that space for its own indicators below.
    // From the game list the rows sit right of the section sidebar, under
    // the section's title (see render_settings_sidebar()).
    const float x0 = settings_sidebar_w(logical_w);
    const float start_y = (float)kTopBarH + 62.0f;
    const float label_x = x0 + 36.0f;
    const float value_x = x0 + ((float)logical_w - x0) * 0.6f;

    // Leaves a little breathing room above the bottom bar rather than
    // packing the last row flush against it.
    // The selected option's explanation box takes the bottom of the area.
    const char* const* help = (g_settings_help && g_settings_selected_row >= 0 && g_settings_selected_row < (int)rows.size())
        ? settings_row_help(rows[g_settings_selected_row]) : nullptr;
    const float help_h = 82.0f;
    const float avail_h = (float)(logical_h - kBottomBarH) - start_y - 4.0f - (help ? help_h + 14.0f : 0.0f);
    g_help_box_top = help ? (float)(logical_h - kBottomBarH) - help_h - 10.0f : 0.0f;
    if (help) {
        const float bx = label_x - 20.0f, bw = (float)logical_w - bx - 20.0f;
        const float by = (float)(logical_h - kBottomBarH) - help_h - 10.0f;
        switch_ui_draw_rect(bx, by, bw, help_h, kTextR, kTextG, kTextB, 0.25f);
        switch_ui_draw_rect(bx + 1.0f, by + 1.0f, bw - 2.0f, help_h - 2.0f, kPanelR, kPanelG, kPanelB, 1.0f);
        draw_text_fit(bx + 18.0f, by + 14.0f, 20.0f, bw - 36.0f, kTextR, kTextG, kTextB, 1.0f, help[0], false);
        draw_text_fit(bx + 18.0f, by + 44.0f, 20.0f, bw - 36.0f, kTextR, kTextG, kTextB, 1.0f, help[1], false);
    }
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

        {
            // Tap: select the row; tap it again: an action row runs (A), a
            // value row steps on (Right) - or back, on the value's left half.
            const float hx = label_x - 20.0f, hw = (float)logical_w - hx - 20.0f;
            menu_touch_item(hx, y - 8.0f, hw, row_h, sel, [i] { g_settings_selected_row = i; },
                            action_row ? HidNpadButton_A : HidNpadButton_Right);
            if (sel && !action_row && !locked) {
                const float vw = (float)logical_w - 20.0f - (value_x - 10.0f);
                menu_touch_button(value_x - 10.0f, y - 8.0f, vw * 0.5f, row_h, HidNpadButton_Left);
                menu_touch_button(value_x - 10.0f + vw * 0.5f, y - 8.0f, vw * 0.5f, row_h, HidNpadButton_Right);
            }
        }

        if (sel) {
            const float hl_x = label_x - 20.0f;
            const float hl_w = (float)logical_w - hl_x - 20.0f;
            switch_ui_draw_rect(hl_x, y - 8.0f, hl_w, label_size + 16.0f, kHighlightR, kHighlightG, kHighlightB, 0.15f);
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

        if (row == SettingsRow::TamaGoFigure) {
            // The figure on, then its two pins (Pause > Figures).
            const std::string name = settings_row_value(row);
            switch_ui_draw_text(value_x, y, label_size, kAccentR, kAccentG, kAccentB, 1.0f, name, true);
            if (g_tmg_figure) draw_contacts_pill(value_x + switch_ui_text_width(name, label_size, true) + 20.0f, y - 2.0f, tmg_contacts_of(g_tmg_figure));
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
        menu_touch_button(x - 20.0f, y - 12.0f, w + 40.0f, opt_size + 24.0f, icon_button(icon));
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
        menu_touch_button(x - 20.0f, oy - 12.0f, w + 40.0f, opt_size + 24.0f, icon_button(icon));
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

// ===========================================================================
// Manufacturer browser (the default game-list view)
//
// Three views share the game list: Makers (a grid of every manufacturer that
// has a visible game, alphabetical, behind two fixed "Favorites" / "Recent"
// tiles), MakerGames (one tile's games: a list on one side, the selected
// game's console art and details on the other) and Classic (the original
// one-game-at-a-time screen, all games, reached with Y from Makers - B goes
// back). Everything here walks g_sorted_order, so the experimental-games and
// clone filters apply exactly as in the classic view.
//
// Favorites (Y in MakerGames) and Recent (the last kRecentMax games started)
// are stored by ref, one per line, in favorites.txt / recent.txt next to the
// settings file.
// ===========================================================================
enum class MenuView : uint8_t { Makers, MakerGames, Classic };
MenuView g_view = MenuView::Makers;

enum class TileKind : uint8_t { Favorites, Recent, Maker };
struct MakerTile {
    TileKind kind;
    uint8_t mfr;
    int count;
    std::string label;
};
std::vector<MakerTile> g_tiles;
int g_tile_cursor = 0;
int g_tile_scroll_row = 0;

std::vector<uint16_t> g_group_games;  // the open tile's games, in display order
int g_group_cursor = 0;
int g_group_scroll = 0;
bool g_group_by_date = false;  // Minus in MakerGames: name <-> date order

// Folders: a system with cartridges is listed inside its maker instead of
// as a maker of its own - the Epoch Game Pocket Computer inside Epoch. In
// the maker's game list it is one row (kFolderEntry) at the top; A opens
// it (g_folder = its id) and the list shows its games; B goes back to the
// maker's list, on the folder's row.
// A folder's row in a list: kFolderBase + the folder's maker id.
constexpr uint16_t kFolderBase = 0xFE00;
inline bool is_folder_entry(uint16_t e) { return (e & 0xFF00) == kFolderBase; }
inline uint8_t entry_folder(uint16_t e) { return (uint8_t)(e & 0xFF); }
uint8_t g_folder = 0;  // the open folder's manufacturer id, 0 = none

uint8_t folder_parent(uint8_t mfr) {
    if (mfr == GW_rom::MANUFACTURER_EPOCH_GPC) return GW_rom::MANUFACTURER_EPOCH;
    if (mfr == GW_rom::MANUFACTURER_HARTUNG_GM) return GW_rom::MANUFACTURER_HARTUNG;
    if (mfr == GW_rom::MANUFACTURER_TIGER_GC) return GW_rom::MANUFACTURER_TIGER;
    if (mfr == GW_rom::MANUFACTURER_MICROVISION) return GW_rom::MANUFACTURER_MILTON_BRADLEY;
    if (mfr == GW_rom::MANUFACTURER_TIGER_RZ) return GW_rom::MANUFACTURER_TIGER;
    if (mfr == GW_rom::MANUFACTURER_ENTEX_AV) return GW_rom::MANUFACTURER_ENTEX;
    return mfr;
}
// The folder listed inside `parent`, 0 if it has none.
std::vector<uint8_t> folders_of(uint8_t parent) {
    std::vector<uint8_t> v;
    for (int m = 1; m < 256; m++)
        if (m != parent && folder_parent((uint8_t)m) == parent) v.push_back((uint8_t)m);
    return v;
}
const char* folder_label(uint8_t folder) {
    if (folder == GW_rom::MANUFACTURER_EPOCH_GPC) return "Game Pocket Computer";
    if (folder == GW_rom::MANUFACTURER_HARTUNG_GM) return "Game Master";
    if (folder == GW_rom::MANUFACTURER_TIGER_GC) return "Game.com";
    if (folder == GW_rom::MANUFACTURER_MICROVISION) return "Microvision";
    if (folder == GW_rom::MANUFACTURER_TIGER_RZ) return "R-Zone Xtreme Pocket Game";
    if (folder == GW_rom::MANUFACTURER_ENTEX_AV) return "Adventure Vision";
    return "";
}

std::vector<std::string> g_favorites;  // refs, in the order they were added
std::vector<std::string> g_recent;     // refs, most recent first
bool g_lists_loaded = false;
constexpr size_t kRecentMax = 20;

std::vector<std::string> read_ref_list(const char* file) {
    std::vector<std::string> out;
    FILE* f = fopen(config_path(file).c_str(), "r");
    if (!f) return out;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        std::string s(line);
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ')) s.pop_back();
        if (!s.empty()) out.push_back(s);
    }
    fclose(f);
    return out;
}

void write_ref_list(const char* file, const std::vector<std::string>& refs) {
    FILE* f = fopen(config_path(file).c_str(), "w");
    if (!f) {
        YOKOI_LOG("switch_menu: can't write %s", file);
        return;
    }
    for (const std::string& r : refs) fprintf(f, "%s\n", r.c_str());
    fclose(f);
}

void load_ref_lists_if_needed() {
    if (g_lists_loaded) return;
    g_favorites = read_ref_list("favorites.txt");
    g_recent = read_ref_list("recent.txt");
    g_lists_loaded = true;
}

bool is_favorite(const std::string& ref) {
    return std::find(g_favorites.begin(), g_favorites.end(), ref) != g_favorites.end();
}

void toggle_favorite(uint16_t idx) {
    const std::string ref = ref_of(idx);
    if (ref.empty()) return;
    auto it = std::find(g_favorites.begin(), g_favorites.end(), ref);
    if (it != g_favorites.end()) g_favorites.erase(it);
    else g_favorites.push_back(ref);
    write_ref_list("favorites.txt", g_favorites);
}

// Called on every game start from the game list (all load paths).
void note_recent(uint16_t idx) {
    load_ref_lists_if_needed();
    const std::string ref = ref_of(idx);
    if (ref.empty()) return;
    g_recent.erase(std::remove(g_recent.begin(), g_recent.end(), ref), g_recent.end());
    g_recent.insert(g_recent.begin(), ref);
    if (g_recent.size() > kRecentMax) g_recent.resize(kRecentMax);
    write_ref_list("recent.txt", g_recent);
}

bool is_visible_game(uint16_t idx) {
    return std::find(g_sorted_order.begin(), g_sorted_order.end(), idx) != g_sorted_order.end();
}

// The visible games of a ref list, in list order (refs no longer in the pack,
// or hidden by the current filters, are skipped - not removed from the file).
std::vector<uint16_t> games_of_ref_list(const std::vector<std::string>& refs) {
    std::vector<uint16_t> out;
    for (const std::string& r : refs) {
        bool found = false;
        const uint16_t idx = find_game_index_by_ref(r, &found);
        if (found && is_visible_game(idx)) out.push_back(idx);
    }
    return out;
}

void rebuild_tiles() {
    load_ref_lists_if_needed();
    std::vector<int> counts(256, 0);
    for (uint16_t idx : g_sorted_order) counts[folder_parent(get_mfr(idx))]++;

    g_tiles.clear();
    g_tiles.push_back({TileKind::Favorites, 0, (int)games_of_ref_list(g_favorites).size(), "Favorites"});
    g_tiles.push_back({TileKind::Recent, 0, (int)games_of_ref_list(g_recent).size(), "Recent"});
    // Year of each maker's oldest (visible) game, for the Era order.
    std::vector<int> first_year(256, 9999);
    for (uint16_t idx : g_sorted_order) {
        const int y = parse_year(date_of(idx));
        const uint8_t m = folder_parent(get_mfr(idx));
        if (y > 0) first_year[m] = std::min(first_year[m], y);
    }
    std::vector<MakerTile> makers;
    for (int m = 0; m < 256; m++) {
        if (counts[m] > 0) makers.push_back({TileKind::Maker, (uint8_t)m, counts[m], mfr_name((uint8_t)m)});
    }
    std::sort(makers.begin(), makers.end(), [&](const MakerTile& a, const MakerTile& b) {
        if (g_maker_order == MakerOrder::MostGames && a.count != b.count) return a.count > b.count;
        if (g_maker_order == MakerOrder::Era && first_year[a.mfr] != first_year[b.mfr]) return first_year[a.mfr] < first_year[b.mfr];
        return name_less_ci(a.label, b.label);
    });
    g_tiles.insert(g_tiles.end(), makers.begin(), makers.end());
    if (g_tile_cursor >= (int)g_tiles.size()) g_tile_cursor = (int)g_tiles.size() - 1;
    if (g_tile_cursor < 0) g_tile_cursor = 0;
}

// A tile's games, in the order its list shows them.
std::vector<uint16_t> games_of_tile(const MakerTile& t) {
    std::vector<uint16_t> v;
    if (t.kind == TileKind::Favorites) return games_of_ref_list(g_favorites);
    if (t.kind == TileKind::Recent) return games_of_ref_list(g_recent);
    const std::vector<uint8_t> folders = folders_of(t.mfr);
    const bool in_folder = g_folder && g_folder != t.mfr && folder_parent(g_folder) == t.mfr;
    std::vector<uint8_t> with_games;
    for (uint16_t idx : g_sorted_order) {
        const uint8_t m = get_mfr(idx);
        if (m == (in_folder ? g_folder : t.mfr)) v.push_back(idx);
        if (!in_folder && m != t.mfr && folder_parent(m) == t.mfr &&
            std::find(with_games.begin(), with_games.end(), m) == with_games.end()) with_games.push_back(m);
    }
    std::stable_sort(v.begin(), v.end(), [](uint16_t a, uint16_t b) {
        if (g_group_by_date) {
            const std::string da = date_of(a), db = date_of(b);
            if (da != db) return da < db;
        }
        return name_less_ci(name_of(a), name_of(b));
    });
    // The folders first, in a fixed order.
    if (!in_folder) {
        std::vector<uint16_t> rows;
        for (uint8_t f : folders)
            if (std::find(with_games.begin(), with_games.end(), f) != with_games.end()) rows.push_back((uint16_t)(kFolderBase + f));
        v.insert(v.begin(), rows.begin(), rows.end());
    }
    return v;
}

// The open folder's games (or, outside it, the folder's own) - for the
// folder row's count and its console art.
std::vector<uint16_t> games_of_folder(uint8_t folder) {
    std::vector<uint16_t> v;
    for (uint16_t idx : g_sorted_order) {
        if (get_mfr(idx) == folder) v.push_back(idx);
    }
    std::stable_sort(v.begin(), v.end(), [](uint16_t a, uint16_t b) { return name_less_ci(name_of(a), name_of(b)); });
    return v;
}

// What a list row stands for: a game, or the folder - whose stand-in for
// the details panel / thumbnail is its first game (they share one console).
uint16_t row_game(uint16_t entry, uint8_t parent) {
    (void)parent;
    if (!is_folder_entry(entry)) return entry;
    const std::vector<uint16_t> f = games_of_folder(entry_folder(entry));
    return f.empty() ? 0 : f[0];
}

void rebuild_group_games() {
    g_group_games.clear();
    if (g_tiles.empty()) return;
    g_group_games = games_of_tile(g_tiles[g_tile_cursor]);
}

// Keeps g_selected_index on the list cursor (it drives the thumbnail, the
// load prompt and launching), clamping the cursor first.
void sync_group_selection() {
    if (g_group_games.empty()) {
        g_group_cursor = 0;
        return;
    }
    if (g_group_cursor >= (int)g_group_games.size()) g_group_cursor = (int)g_group_games.size() - 1;
    if (g_group_cursor < 0) g_group_cursor = 0;
    g_selected_index = g_group_games[g_group_cursor];
    g_sel_is_folder = is_folder_entry(g_selected_index);
    if (g_sel_is_folder && !g_tiles.empty()) g_selected_index = row_game(g_selected_index, g_tiles[g_tile_cursor].mfr);
}

bool group_cursor_on_folder() {
    return !g_group_games.empty() && g_group_cursor < (int)g_group_games.size() && is_folder_entry(g_group_games[g_group_cursor]);
}

// Opens the tile under the cursor, landing on `prefer` if it's in it.
void open_tile(uint16_t prefer = 0xFFFF) {
    rebuild_group_games();
    g_group_cursor = 0;
    g_group_scroll = 0;
    for (size_t i = 0; i < g_group_games.size(); i++) {
        if (g_group_games[i] == prefer) g_group_cursor = (int)i;
    }
    sync_group_selection();
    g_view = MenuView::MakerGames;
}

int tile_columns(int logical_w) { return logical_w >= 1000 ? 5 : 3; }

void draw_star(float cx, float cy, float r, float red, float green, float blue, float a) {
    float pts[20];
    for (int i = 0; i < 10; i++) {
        const float ang = -1.5707963f + (float)i * 0.6283185f;
        const float rr = (i % 2 == 0) ? r : r * 0.45f;
        pts[i * 2] = cx + rr * cosf(ang);
        pts[i * 2 + 1] = cy + rr * sinf(ang);
    }
    // Not convex: fan from the centre one point-pair at a time.
    for (int i = 0; i < 10; i++) {
        const int j = (i + 1) % 10;
        const float tri[6] = {cx, cy, pts[i * 2], pts[i * 2 + 1], pts[j * 2], pts[j * 2 + 1]};
        switch_ui_draw_polygon(tri, 3, red, green, blue, a);
    }
}

void draw_clock(float cx, float cy, float r, float red, float green, float blue, float a) {
    const int n = 28;
    for (int i = 0; i < n; i++) {  // ring as quads
        const float a0 = (float)i / n * 6.2831853f, a1 = (float)(i + 1) / n * 6.2831853f;
        const float ri = r * 0.72f;
        const float q[8] = {cx + r * cosf(a0), cy + r * sinf(a0), cx + r * cosf(a1), cy + r * sinf(a1),
                            cx + ri * cosf(a1), cy + ri * sinf(a1), cx + ri * cosf(a0), cy + ri * sinf(a0)};
        switch_ui_draw_polygon(q, 4, red, green, blue, a);
    }
    const float w = r * 0.14f;
    switch_ui_draw_rect(cx - w, cy - r * 0.55f, w * 2.0f, r * 0.55f + w, red, green, blue, a);
    switch_ui_draw_rect(cx - w, cy - w, r * 0.45f + w, w * 2.0f, red, green, blue, a);
}

// Bottom hint bar, same composition as the classic view's.
// Bottom-bar button hints: groups of icon(s) then a word, left to right.
// Shrunk as one to fit the menu's width (the portrait menu is only 720
// wide). Each icon and its word can be tapped (input/menu_touch.h): the word
// presses the group's last icon.
struct HintItem {
    bool is_icon;
    SwitchButtonIcon icon;
    std::string text;
};

void draw_hint_items(const std::vector<HintItem>& items, int logical_h) {
    if (items.empty()) return;
    auto width_at = [&](float sz) {
        float w = 0.0f;
        for (const HintItem& it : items) {
            w += it.is_icon ? switch_ui_button_icon_width(sz, it.icon) + sz * 0.3f
                            : switch_ui_text_width(it.text, sz) + sz * 1.1f;
        }
        return w;
    };
    const float avail = (float)g_menu_lw - 40.0f;
    float size = 20.0f;
    while (size > 12.0f && width_at(size) > avail) size -= 0.5f;
    const float icon_pad = size * 0.3f, group_pad = size * 1.1f;
    const float bar_y = (float)(logical_h - kBottomBarH);
    const float y = bar_y + (kBottomBarH - size) * 0.5f - 1.0f;
    float x = 20.0f;
    uint64_t last = 0;
    for (const HintItem& it : items) {
        if (it.is_icon) {
            const float w = switch_ui_button_icon_width(size, it.icon);
            last = icon_button(it.icon);
            menu_touch_button(x - icon_pad * 0.5f, bar_y, w + icon_pad, (float)kBottomBarH, last);
            switch_ui_draw_button_icon(x, y, size, kTextR, kTextG, kTextB, 0.85f, it.icon);
            x += w + icon_pad;
        } else {
            const float w = switch_ui_text_width(it.text, size);
            menu_touch_button(x - icon_pad * 0.5f, bar_y, w + group_pad * 0.5f + icon_pad * 0.5f, (float)kBottomBarH, last);
            switch_ui_draw_text(x, y, size, kTextR, kTextG, kTextB, 0.85f, it.text);
            x += w + group_pad;
        }
    }
}

struct HintBar {
    std::vector<HintItem> items;
    int logical_h;
    explicit HintBar(int lh) : logical_h(lh) {}
    void icon(SwitchButtonIcon i) { items.push_back({true, i, std::string()}); }
    void text(const std::string& s) { items.push_back({false, SwitchButtonIcon::A, s}); }
    ~HintBar() { draw_hint_items(items, logical_h); }
};

void draw_top_right(int logical_w, const std::string& s) {
    const float size = 20.0f;
    switch_ui_draw_text((float)logical_w - 24.0f - switch_ui_text_width(s, size), 20.0f, size, kTextR, kTextG, kTextB, 0.85f, s);
}

// Draws `s` at most `max_w` wide: shrinks the size down to 70%, then cuts
// it with "...".
void draw_text_fit(float x, float y, float size, float max_w, float r, float g, float b, float a, std::string s, bool bold) {
    float sz = size;
    while (sz > size * 0.7f && switch_ui_text_width(s, sz, bold) > max_w) sz -= 1.0f;
    if (switch_ui_text_width(s, sz, bold) > max_w) {
        while (s.size() > 1 && switch_ui_text_width(s + "...", sz, bold) > max_w) s.pop_back();
        s += "...";
    }
    switch_ui_draw_text(x, y + (size - sz) * 0.5f, sz, r, g, b, a, s, bold);
}

// ---------------------------------------------------------------------------
// Pause menu header: the running game's console art (the same picture as its
// single-screen console overlay) and "Name (Maker)" in place of the app icon
// and "PAUSED". Loaded when the game changes, kept until the next one.
GLuint g_pause_art_tex = 0;
int g_pause_art_w = 0, g_pause_art_h = 0;
int g_pause_art_index = -1;

void reload_pause_art_if_needed() {
    if (!g_game) return;
    if (g_pause_art_index == (int)g_game_index) return;
    if (g_pause_art_tex) glDeleteTextures(1, &g_pause_art_tex);
    g_pause_art_tex = 0;
    g_pause_art_w = g_pause_art_h = 0;
    g_pause_art_index = (int)g_game_index;

    const std::string asset = t3x_path_to_png_asset(g_game->path_console);
    const uint8_t* data = nullptr;
    size_t size = 0;
    if (asset.empty() || !gw_pack::get_file_bytes(asset, data, size) || !data || size == 0) return;
    int w = 0, h = 0, channels = 0;
    stbi_uc* pixels = stbi_load_from_memory(data, (int)size, &w, &h, &channels, 4);
    if (!pixels) return;
    glGenTextures(1, &g_pause_art_tex);
    glBindTexture(GL_TEXTURE_2D, g_pause_art_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    g_pause_art_w = w;
    g_pause_art_h = h;
}

// The maker as written normally ("Nintendo", "McDonald's"), for the pause
// header - mfr_name() is the all-caps form the maker list uses.
std::string maker_display_name(uint8_t mfr) {
    switch (mfr) {
        case GW_rom::MANUFACTURER_VTECH:      return "VTech";
        case GW_rom::MANUFACTURER_VGS:        return "VGS";
        case GW_rom::MANUFACTURER_MCDONALDS:  return "McDonald's";
        case GW_rom::MANUFACTURER_ENTEX:      return "Entex";
        case GW_rom::MANUFACTURER_TOMY:       return "Tomy";
        case GW_rom::MANUFACTURER_DAVID_AND_JOHN: return "David and John";
        case GW_rom::MANUFACTURER_EPOCH_GPC:  return "Epoch Game Pocket Computer";
        case GW_rom::MANUFACTURER_HARTUNG_GM: return "Hartung Game Master";
        case GW_rom::MANUFACTURER_TIGER_GC:   return "Tiger Game.com";
        case GW_rom::MANUFACTURER_TIGER_RZ:   return "R-Zone Xtreme Pocket Game";
        case GW_rom::MANUFACTURER_ENTEX_AV:   return "Entex Adventure Vision";
        case GW_rom::MANUFACTURER_UNBRANDED:  return switch_tr("Unbranded");
        default: break;
    }
    std::string s = mfr_name(mfr);
    bool start = true;
    for (char& c : s) {
        if (std::isalpha((unsigned char)c)) {
            c = start ? (char)std::toupper((unsigned char)c) : (char)std::tolower((unsigned char)c);
            start = false;
        } else {
            start = (c == ' ' || c == '-');
        }
    }
    return s;
}

// Draws the art and "Name (Maker)" from x, up to max_x. Returns false when
// there's no game to show (the caller draws the usual icon and title).
bool draw_pause_header(float x, float max_x) {
    if (!g_settings_from_gameplay || !g_game) return false;
    reload_pause_art_if_needed();
    constexpr float kArtH = 52.0f, kArtMaxW = 72.0f;
    if (g_pause_art_tex != 0 && g_game->console_info && g_game->console_info[4] > 0 && g_game->console_info[5] > 0) {
        const uint16_t* ci = g_game->console_info;
        float ah = kArtH, aw = ah * (float)ci[4] / (float)ci[5];
        if (aw > kArtMaxW) { aw = kArtMaxW; ah = aw * (float)ci[5] / (float)ci[4]; }
        const float ay = ((float)kTopBarH - ah) * 0.5f;
        switch_ui_draw_texture(g_pause_art_tex, x, ay, aw, ah, (float)g_pause_art_w, (float)g_pause_art_h, (float)ci[2],
                               (float)ci[3], (float)ci[4], (float)ci[5]);
        x += aw + 12.0f;
    }
    const float name_size = 28.0f, mk_size = 22.0f;
    // Linked: "+ the other game" instead of the maker.
    const std::string partner = link_session_partner_name();
    const std::string mk = partner.empty() ? "(" + maker_display_name(g_game->manufacturer) + ")" : "+ " + partner;
    const float mk_w = switch_ui_text_width(mk, mk_size);
    const float name_max = std::max(60.0f, max_x - x - mk_w - 12.0f);
    float nr = kTextR, ng = kTextG, nb = kTextB;
    draw_text_fit(x, 16.0f, name_size, name_max, nr, ng, nb, 1.0f, g_game->name, /*bold=*/true);
    float sz = name_size;
    std::string shown = g_game->name;
    while (sz > name_size * 0.7f && switch_ui_text_width(shown, sz, true) > name_max) sz -= 1.0f;
    const float name_w = std::min(name_max, switch_ui_text_width(shown, sz, true));
    switch_ui_draw_text(x + name_w + 12.0f, 20.0f, mk_size, kTextR, kTextG, kTextB, 0.6f, mk);
    return true;
}

// ---------------------------------------------------------------------------
// Maker colours (Settings > Menu > Maker Colors): one fixed colour per maker,
// loosely after its brand, drawn as a stripe on the grid tiles and a band on
// the carousel cards. Only that mark is coloured - tile, text and highlight
// stay the theme's. Light themes get a darker shade (still readable on a
// white tile); one-colour themes (Game Boy, Virtual Boy) get shades between
// the theme's own tile and text colours instead, so they keep their look.
// ---------------------------------------------------------------------------
uint32_t maker_base_rgb(const MakerTile& t) {
    if (t.kind == TileKind::Favorites) return 0xFFCC3D;
    if (t.kind == TileKind::Recent) return 0x37D0FF;
    static constexpr uint32_t kMakerRgb[] = {
        0xE4002B,  //  0 Nintendo
        0x1E88E5,  //  1 Tronica
        0xC62828,  //  2 Elektronika
        0xF57C00,  //  3 Tiger
        0x9C7A5A,  //  4 David & John
        0x888888,  //  5 (retired)
        0xFFC72C,  //  6 McDonald's
        0xC0703A,  //  7 Burger Bard
        0xD81B60,  //  8 Konami
        0x00ACC1,  //  9 Nelsonic
        0x3949AB,  // 10 V-Tech
        0x2F6FE0,  // 11 Bandai
        0x00897B,  // 12 Epoch
        0x8D6E63,  // 13 Nikko
        0x5E35B1,  // 14 Tandy
        0xEC407A,  // 15 Popy
        0x43A047,  // 16 Gakken
        0xAB47BC,  // 17 Spica
        0xE0A020,  // 18 Apollo
        0x7CB342,  // 19 Brick Game
        0xD36BA6,  // 20 Hiro
        0x558B2F,  // 21 Nature Land
        0x4DD0E1,  // 22 Sparkle Crystal
        0x6D8BD8,  // 23 Gametech
        0x8E5BD8,  // 24 Compile
        0xFF7043,  // 25 VGS
        0x26A69A,  // 26 Cool Tec
        0xB5863B,  // 27 Hartung
        0x5C9BD6,  // 28 Edla
        0x9E9D24,  // 29 Stadlbauer
        0x78909C,  // 30 Unbranded
        0xD84A3A,  // 31 Coleco
        0xEF8A17,  // 32 Entex
        0x1565C0,  // 33 TOMY
        0xF2C230,  // 34 Hitorikko
        0x6A1B9A,  // 35 Parker Brothers
        0xF06292,  // 36 Bambino
        0xC8A15A,  // 37 Milton Bradley Microvision
        0x00897B,  // 38 Epoch Game Pocket Computer (Epoch's colour: its folder)
        0x37474F,  // 39 (unused)
        0x37474F,  // 40 Hartung Game Master
        0xE65100,  // 41 Tiger Game.com
        0xC8A15A,  // 42 Milton Bradley (Microvision's colour)
        0xE65100,  // 43 Tiger R-Zone
    };
    if (t.mfr < sizeof(kMakerRgb) / sizeof(kMakerRgb[0])) return kMakerRgb[t.mfr];
    // A future maker: a hue of its own (golden-angle steps), mid saturation.
    const float h = std::fmod(t.mfr * 0.618034f, 1.0f) * 6.0f;
    const float x = 1.0f - std::fabs(std::fmod(h, 2.0f) - 1.0f);
    float r = 0, g = 0, b = 0;
    switch ((int)h) {
        case 0: r = 1; g = x; break;
        case 1: r = x; g = 1; break;
        case 2: g = 1; b = x; break;
        case 3: g = x; b = 1; break;
        case 4: r = x; b = 1; break;
        default: r = 1; b = x; break;
    }
    auto c = [](float v) { return (uint32_t)(40.0f + v * 180.0f); };
    return c(r) << 16 | c(g) << 8 | c(b);
}

void maker_color(const MakerTile& t, float& r, float& g, float& b) {
    const char* theme = kMenuThemes[g_menu_theme < kNumMenuThemes ? g_menu_theme : 0].name;
    if (!strcmp(theme, "Game Boy") || !strcmp(theme, "Virtual Boy")) {
        // one-colour themes: a shade between the tile and the text colour
        const float f = 0.35f + 0.55f * std::fmod((float)(t.kind == TileKind::Maker ? t.mfr + 2 : (int)t.kind) * 0.618034f, 1.0f);
        r = kTileR + (kTextR - kTileR) * f;
        g = kTileG + (kTextG - kTileG) * f;
        b = kTileB + (kTextB - kTileB) * f;
        return;
    }
    set_rgb(maker_base_rgb(t), r, g, b);
    const float tile_lum = 0.299f * kTileR + 0.587f * kTileG + 0.114f * kTileB;
    const float lum = 0.299f * r + 0.587f * g + 0.114f * b;
    if (tile_lum > 0.6f && lum > 0.45f) {
        // light theme: darken bright colours so they stand out on a white tile
        const float k = 0.45f / lum;
        r *= k; g *= k; b *= k;
    }
}

// The same colour for a maker id (the one-at-a-time list and the maker's
// game list, which only know the selected game's maker).
void maker_color_of(uint8_t mfr, float& r, float& g, float& b) {
    MakerTile t{TileKind::Maker, mfr, 0, std::string()};
    maker_color(t, r, g, b);
}

void render_makers(int logical_w, int logical_h) {
    char total[64];
    snprintf(total, sizeof(total), "%u games  |  %s", (unsigned)g_sorted_order.size(), maker_order_label(g_maker_order));
    draw_top_right(logical_w, total);

    const int cols = tile_columns(logical_w);
    const float margin = 24.0f, gap_x = 10.0f, gap_y = 8.0f, tile_h = 66.0f;
    const float top = kTopBarH + 14.0f, bottom = (float)(logical_h - kBottomBarH) - 10.0f;
    const float tile_w = ((float)logical_w - margin * 2.0f - gap_x * (cols - 1)) / cols;
    const int rows_visible = std::max(1, (int)((bottom - top + gap_y) / (tile_h + gap_y)));
    const int total_rows = ((int)g_tiles.size() + cols - 1) / cols;

    const int cur_row = g_tile_cursor / cols;
    if (cur_row < g_tile_scroll_row) g_tile_scroll_row = cur_row;
    if (cur_row >= g_tile_scroll_row + rows_visible) g_tile_scroll_row = cur_row - rows_visible + 1;
    if (g_tile_scroll_row > std::max(0, total_rows - rows_visible)) g_tile_scroll_row = std::max(0, total_rows - rows_visible);

    int max_count = 1;
    for (const MakerTile& t : g_tiles) if (t.kind == TileKind::Maker) max_count = std::max(max_count, t.count);

    for (int i = g_tile_scroll_row * cols; i < (int)g_tiles.size() && i < (g_tile_scroll_row + rows_visible) * cols; i++) {
        const MakerTile& t = g_tiles[i];
        const int c = i % cols, r = i / cols - g_tile_scroll_row;
        const float x = margin + c * (tile_w + gap_x), y = top + r * (tile_h + gap_y);
        const bool sel = (i == g_tile_cursor);
        menu_touch_item(x, y, tile_w, tile_h, sel, [i] { g_tile_cursor = i; }, HidNpadButton_A);
        if (sel) {
            switch_ui_draw_rect(x - 3.0f, y - 3.0f, tile_w + 6.0f, tile_h + 6.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
            switch_ui_draw_rect(x, y, tile_w, tile_h, kSelR, kSelG, kSelB, 1.0f);
        } else {
            switch_ui_draw_rect(x, y, tile_w, tile_h, kTileR, kTileG, kTileB, 1.0f);
        }
        float mr = 0, mg = 0, mb = 0;
        if (g_maker_colors) {
            maker_color(t, mr, mg, mb);
            switch_ui_draw_rect(x, y, 7.0f, tile_h, mr, mg, mb, 1.0f);
        }
        // Text area: right of the colour strip (and of the Favorites /
        // Recent icon). Name, "N games" and the count bar are centred in it,
        // as one block centred vertically in the tile.
        float ax = x + 12.0f;
        if (t.kind == TileKind::Favorites) {
            draw_star(x + 32.0f, y + tile_h * 0.5f, 17.0f, 1.0f, 0.80f, 0.24f, 1.0f);
            ax = x + 58.0f;
        } else if (t.kind == TileKind::Recent) {
            draw_clock(x + 32.0f, y + tile_h * 0.5f, 15.0f, kAccentR, kAccentG, kAccentB, 1.0f);
            ax = x + 58.0f;
        }
        const float aw = x + tile_w - 10.0f - ax, acx = ax + aw * 0.5f;
        // A name that doesn't fit on one line even a little smaller goes on
        // two, split at the space nearest its middle ("MILTON BRADLEY" /
        // "MICROVISION").
        float size = 22.0f;
        while (size > 18.0f && switch_ui_text_width(t.label, size, true) > aw) size -= 1.0f;
        std::vector<std::string> lines{t.label};
        if (switch_ui_text_width(t.label, size, true) > aw) {
            size_t best = std::string::npos;
            for (size_t p = t.label.find(' '); p != std::string::npos; p = t.label.find(' ', p + 1)) {
                const auto dist = [&](size_t q) { return std::labs((long)q * 2 - (long)t.label.size()); };
                if (best == std::string::npos || dist(p) < dist(best)) best = p;
            }
            if (best != std::string::npos) {
                lines = {t.label.substr(0, best), t.label.substr(best + 1)};
                size = 16.0f;
            }
        }
        auto widest = [&](float sz) {
            float m = 0.0f;
            for (const std::string& l : lines) m = std::max(m, switch_ui_text_width(l, sz, true));
            return m;
        };
        while (size > 11.0f && widest(size) > aw) size -= 1.0f;
        char cnt[24];
        snprintf(cnt, sizeof(cnt), t.count == 1 ? "%d game" : "%d games", t.count);
        const std::string cnt_t = switch_tr(cnt);
        const float cs = lines.size() > 1 ? 14.0f : 17.0f;
        const float line_h = size * 1.1f;
        const float block_h = line_h * (float)lines.size() + 2.0f + cs;
        float ty = y + (tile_h - 9.0f - block_h) * 0.5f;
        for (const std::string& l : lines) {
            switch_ui_draw_text(acx - switch_ui_text_width(l, size, true) * 0.5f, ty, size, kTextR, kTextG, kTextB, 1.0f, l, true);
            ty += line_h;
        }
        switch_ui_draw_text(acx - switch_ui_text_width(cnt_t, cs) * 0.5f, ty + 2.0f, cs, kTextR, kTextG, kTextB, 0.6f, cnt_t);
        if (t.kind == TileKind::Maker) {
            const float bw = std::max(5.0f, (tile_w - 32.0f) * (float)t.count / (float)max_count);
            const bool hl = sel;
            const float bx = acx - bw * 0.5f;
            if (g_maker_colors) switch_ui_draw_rect(bx, y + tile_h - 7.0f, bw, 3.0f, mr, mg, mb, 1.0f);
            else switch_ui_draw_rect(bx, y + tile_h - 7.0f, bw, 3.0f, hl ? kHighlightR : kAccentR, hl ? kHighlightG : kAccentG,
                                     hl ? kHighlightB : kAccentB, 1.0f);
        }
    }

    // Scroll bar, only when the grid doesn't fit.
    if (total_rows > rows_visible) {
        const float bar_h = bottom - top;
        const float thumb_h = bar_h * rows_visible / total_rows;
        const float thumb_y = top + (bar_h - thumb_h) * g_tile_scroll_row / std::max(1, total_rows - rows_visible);
        switch_ui_draw_rect((float)logical_w - 10.0f, top, 4.0f, bar_h, kTextR, kTextG, kTextB, 0.15f);
        switch_ui_draw_rect((float)logical_w - 10.0f, thumb_y, 4.0f, thumb_h, kTextR, kTextG, kTextB, 0.6f);
    }

    HintBar h(logical_h);
    h.icon(SwitchButtonIcon::A);
    h.text("Open");
    h.icon(SwitchButtonIcon::Y);
    h.text("All Games");
    h.icon(SwitchButtonIcon::Plus);
    h.text("Settings");
}

// ---------------------------------------------------------------------------
// Carousel maker menu (Menu Style = Carousel): the same tiles as the grid
// (Favorites, Recent, then the makers), one big card in the middle and two
// smaller ones each side. Left/Right move one, L/R jump to the next/previous
// first letter. With Transition = Slide the cards glide into place; other
// transitions are drawn over the whole content area (switch_transition).
// ---------------------------------------------------------------------------
float g_car_pos = 0.0f;       // animated position (tile index, fractional)
uint64_t g_car_tick = 0;

char tile_letter(const MakerTile& t) {
    if (t.kind != TileKind::Maker || t.label.empty()) return 0;
    return (char)toupper((unsigned char)t.label[0]);
}

void carousel_jump_letter(int dir) {
    const int n = (int)g_tiles.size();
    if (n == 0) return;
    const char cur = tile_letter(g_tiles[g_tile_cursor]);
    for (int s = 1; s < n; s++) {
        const int i = g_tile_cursor + dir * s;
        if (i < 0 || i >= n) break;
        const char c = tile_letter(g_tiles[i]);
        if (c && c != cur) {
            // Backwards: land on the first tile of that letter.
            int j = i;
            if (dir < 0) while (j > 0 && tile_letter(g_tiles[j - 1]) == c) j--;
            g_tile_cursor = j;
            return;
        }
    }
}

void render_maker_carousel(int logical_w, int logical_h) {
    const int n = (int)g_tiles.size();
    char pos[32];
    snprintf(pos, sizeof(pos), "%d / %d", n ? g_tile_cursor + 1 : 0, n);
    draw_top_right(logical_w, pos);
    if (n == 0) return;

    // Glide toward the cursor (Slide); otherwise jump straight there.
    const uint64_t now = armGetSystemTick();
    const float dt = g_car_tick ? std::min(0.1f, (float)armTicksToNs(now - g_car_tick) / 1e9f) : 0.0f;
    g_car_tick = now;
    const float target = (float)g_tile_cursor;
    // The ring wraps: keep the animated position on the near side of the
    // cursor, so last -> first glides one step instead of across them all.
    while (target - g_car_pos > n * 0.5f) g_car_pos += (float)n;
    while (target - g_car_pos < -n * 0.5f) g_car_pos -= (float)n;
    if (g_transition == kTrSlide && dt > 0.0f) {
        if (std::fabs(target - g_car_pos) > 3.0f) g_car_pos = target - (target > g_car_pos ? 3.0f : -3.0f);
        g_car_pos += (target - g_car_pos) * std::min(1.0f, dt * 12.0f);
        if (std::fabs(target - g_car_pos) < 0.01f) g_car_pos = target;
    } else {
        g_car_pos = target;
    }

    const float W = (float)logical_w;
    const float cy = (float)kTopBarH + ((float)(logical_h - kBottomBarH - kTopBarH)) * 0.47f;
    const float near_dx = W * 0.27f, far_dx = W * 0.44f;
    auto place = [&](float off, float& x, float& s) {
        const float a = std::fabs(off);
        s = 1.0f - 0.28f * std::min(1.0f, a) - 0.17f * std::max(0.0f, std::min(1.0f, a - 1.0f));
        const float d = near_dx * std::min(1.0f, a) + (far_dx - near_dx) * std::max(0.0f, std::min(1.0f, a - 1.0f));
        x = W * 0.5f + (off < 0 ? -d : d);
    };
    // Each card's offset from the centre, around the ring (so the first
    // cards also show up after the last). Farthest first, so nearer ones
    // are drawn on top.
    auto ring_off = [&](int i) {
        float off = std::fmod((float)i - g_car_pos, (float)n);
        if (off > n * 0.5f) off -= (float)n;
        if (off < -n * 0.5f) off += (float)n;
        return off;
    };
    std::vector<int> order;
    for (int i = 0; i < n; i++) if (std::fabs(ring_off(i)) < 2.6f) order.push_back(i);
    std::sort(order.begin(), order.end(), [&](int a, int b) { return std::fabs(ring_off(a)) > std::fabs(ring_off(b)); });
    const float base_w = std::min(400.0f, W * 0.33f), base_h = base_w * 0.8f;
    for (int i : order) {
        const MakerTile& t = g_tiles[i];
        const float off = ring_off(i);
        float x, s;
        place(off, x, s);
        const float w = base_w * s, h = base_h * s;
        const float x0 = x - w * 0.5f, y0 = cy - h * 0.5f;
        const bool center = std::fabs(off) < 0.5f;
        menu_touch_item(x0, y0, w, h, i == g_tile_cursor, [i] { g_tile_cursor = i; }, HidNpadButton_A);
        if (center) switch_ui_draw_rect(x0 - 5.0f, y0 - 5.0f, w + 10.0f, h + 10.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        switch_ui_draw_rect(x0, y0, w, h, kTileR, kTileG, kTileB, 1.0f);
        if (center) switch_ui_draw_rect(x0, y0, w, h, kTextR, kTextG, kTextB, 0.05f);
        if (g_maker_colors) {
            float mr, mg, mb;
            maker_color(t, mr, mg, mb);
            switch_ui_draw_rect(x0, y0, w, 14.0f * s, mr, mg, mb, center ? 1.0f : 0.75f);
        }
        // Name: largest size that fits the card. A name too long for one
        // line at full size goes on two, split at the space nearest its
        // middle ("MILTON BRADLEY" / "MICROVISION").
        float size = (center ? 44.0f : 30.0f) * (center ? 1.0f : s / 0.72f);
        std::vector<std::string> lines{t.label};
        if (switch_ui_text_width(t.label, size, true) > w - 24.0f) {
            size_t best = std::string::npos;
            for (size_t p = t.label.find(' '); p != std::string::npos; p = t.label.find(' ', p + 1)) {
                const auto dist = [&](size_t q) { return (long)q * 2 - (long)t.label.size(); };
                if (best == std::string::npos || std::labs(dist(p)) < std::labs(dist(best))) best = p;
            }
            if (best != std::string::npos) lines = {t.label.substr(0, best), t.label.substr(best + 1)};
        }
        auto widest = [&](float sz) {
            float m = 0.0f;
            for (const std::string& l : lines) m = std::max(m, switch_ui_text_width(l, sz, true));
            return m;
        };
        while (size > 12.0f && widest(size) > w - 24.0f) size -= 1.0f;
        const float a = center ? 1.0f : 0.6f;
        char cnt[24];
        snprintf(cnt, sizeof(cnt), t.count == 1 ? "%d game" : "%d games", t.count);
        const float cs = (center ? 28.0f : 20.0f) * (center ? 1.0f : s / 0.72f);
        const std::string& cs_t = switch_tr(cnt);
        // The name and the count as one block, centred in the card.
        const float line_h = size * 1.12f, gap = size * 0.35f;
        const float block_h = line_h * (float)lines.size() + gap + cs;
        float name_y = cy - block_h * 0.5f;
        if (t.kind == TileKind::Favorites) draw_star(x, name_y - size * 0.7f, size * 0.45f, 1.0f, 0.80f, 0.24f, a);
        if (t.kind == TileKind::Recent) draw_clock(x, name_y - size * 0.7f, size * 0.4f, kAccentR, kAccentG, kAccentB, a);
        for (const std::string& l : lines) {
            switch_ui_draw_text(x - switch_ui_text_width(l, size, true) * 0.5f, name_y, size, kTextR, kTextG, kTextB, a, l, true);
            name_y += line_h;
        }
        const float count_y = name_y + gap;
        if (center) {
            switch_ui_draw_text(x - switch_ui_text_width(cs_t, cs) * 0.5f, count_y, cs, kHighlightR, kHighlightG, kHighlightB, 1.0f, cs_t);
        } else {
            switch_ui_draw_text(x - switch_ui_text_width(cs_t, cs) * 0.5f, count_y, cs, kTextR, kTextG, kTextB, 0.45f, cs_t);
        }
    }

    // Letter strip (first letters of the makers, in tile order).
    std::string letters;
    for (const MakerTile& t : g_tiles) {
        const char c = tile_letter(t);
        if (c && letters.find(c) == std::string::npos) letters += c;
    }
    const char cur = tile_letter(g_tiles[g_tile_cursor]);
    const float step = 30.0f, ly = (float)(logical_h - kBottomBarH) - 46.0f;
    float lx = W * 0.5f - letters.size() * step * 0.5f;
    for (char c : letters) {
        const std::string s(1, c);
        if (c == cur) switch_ui_draw_rect(lx + 1.0f, ly - 4.0f, step - 2.0f, 30.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        switch_ui_draw_text(lx + (step - switch_ui_text_width(s, 18.0f, true)) * 0.5f, ly, 18.0f, kTextR, kTextG, kTextB,
                            c == cur ? 1.0f : 0.45f, s, true);
        lx += step;
    }

    HintBar h(logical_h);
    h.icon(SwitchButtonIcon::DpadLeft);
    h.icon(SwitchButtonIcon::DpadRight);
    h.text("Choose");
    h.icon(SwitchButtonIcon::L);
    h.icon(SwitchButtonIcon::R);
    h.text("Letter");
    h.icon(SwitchButtonIcon::A);
    h.text("Open");
    h.icon(SwitchButtonIcon::Y);
    h.text("All Games");
    h.icon(SwitchButtonIcon::Plus);
    h.text("Settings");
}

int g_nav_dir = 1;  // last move in the one-at-a-time view (+1 / -1), for transitions

void render_maker_games(int logical_w, int logical_h) {
    const MakerTile& t = g_tiles[g_tile_cursor];
    {
        char right[64];
        // Inside a folder: "EPOCH > GAME POCKET COMPUTER  |  6 games".
        std::string label = t.label;
        if (g_folder && g_folder != t.mfr && folder_parent(g_folder) == t.mfr) {
            std::string f = folder_label(g_folder);
            for (char& c : f) c = (char)toupper((unsigned char)c);
            label += " > " + f;
        }
        snprintf(right, sizeof(right), "%s  |  %u %s", label.c_str(), (unsigned)g_group_games.size(),
                 g_group_games.size() == 1 ? "game" : "games");
        draw_top_right(logical_w, right);
    }

    const bool landscape = logical_w >= logical_h;
    const float margin = 24.0f;
    const float top = kTopBarH + 16.0f, bottom = (float)(logical_h - kBottomBarH) - 12.0f;

    // Panel rects: details (art + text) and the list.
    float dx, dy, dw, dh, lx, ly, lw, lh;
    if (landscape) {
        dx = margin; dy = top; dw = (float)logical_w * 0.46f - margin; dh = bottom - top;
        lx = (float)logical_w * 0.48f; ly = top; lw = (float)logical_w - lx - margin; lh = bottom - top;
    } else {
        dx = margin; dy = top; dw = (float)logical_w - margin * 2.0f; dh = (bottom - top) * 0.48f;
        lx = margin; ly = top + dh + 12.0f; lw = dw; lh = bottom - ly;
    }

    if (g_group_games.empty()) {
        const std::string msg = (t.kind == TileKind::Favorites) ? "No favorites yet - press Y on a game to add it."
                              : (t.kind == TileKind::Recent)    ? "No games played yet."
                                                                : "No games.";
        const float sz = 24.0f;
        switch_ui_draw_text((float)logical_w * 0.5f - switch_ui_text_width(msg, sz) * 0.5f, (top + bottom) * 0.5f - sz, sz,
                            kTextR, kTextG, kTextB, 0.7f, msg);
        HintBar h(logical_h);
        h.icon(SwitchButtonIcon::B);
        h.text("Back");
        return;
    }

    // --- details: console art, maker, name, date (+ its records) ---
    const GW_rom* g = load_game(g_selected_index);
    const float name_size = 28.0f, mfr_size = 22.0f, date_size = 20.0f, rec_size = 20.0f;
    // On a folder's row the panel shows the folder (its console art is its
    // first game's - they share the one console).
    const bool folder_sel = group_cursor_on_folder();
    const bool show_rec = !folder_sel && g && g_records_enabled && g_records_in_list && score_tracker_has_records(g->ref);
    const float text_h = 14.0f + mfr_size + 6.0f + name_size + 6.0f + date_size + (show_rec ? 8.0f + rec_size : 0.0f);
    float aspect = 4.0f / 3.0f;
    if (g && g->console_info && g->console_info[4] > 0 && g->console_info[5] > 0) {
        aspect = (float)g->console_info[4] / (float)g->console_info[5];
    }
    float bw = dw, bh = bw / aspect;
    if (bh > dh - text_h) { bh = dh - text_h; bw = bh * aspect; }
    const float bx = dx + (dw - bw) * 0.5f;
    const float by = dy + std::max(0.0f, (dh - text_h - bh) * 0.5f);
    menu_touch_button(bx, by, bw, bh, HidNpadButton_A);  // tap the console: play it
    switch_ui_draw_rect(bx, by, bw, bh, kTileR, kTileG, kTileB, 1.0f);
    if (g_thumb_tex != 0 && g && g->console_info) {
        const uint16_t* ci = g->console_info;
        switch_ui_draw_texture(g_thumb_tex, bx, by, bw, bh, (float)g_thumb_w, (float)g_thumb_h, (float)ci[2], (float)ci[3],
                               (float)ci[4], (float)ci[5]);
    }
    float ty = by + bh + 14.0f;
    const float cx = dx + dw * 0.5f;
    const std::string mfr = mfr_name(get_mfr(g_selected_index));
    {
        float mr = kHighlightR, mg = kHighlightG, mb = kHighlightB;
        if (g_maker_colors) maker_color_of(get_mfr(g_selected_index), mr, mg, mb);
        switch_ui_draw_text(cx - switch_ui_text_width(mfr, mfr_size) * 0.5f, ty, mfr_size, mr, mg, mb, 1.0f, mfr);
    }
    ty += mfr_size + 6.0f;
    const uint8_t sel_folder = folder_sel ? entry_folder(g_group_games[g_group_cursor]) : 0;
    const std::string name = folder_sel ? std::string(folder_label(sel_folder)) : g ? g->name : std::string("?");
    {
        float sz = name_size;
        while (sz > 18.0f && switch_ui_text_width(name, sz, true) > dw) sz -= 1.0f;
        draw_text_fit(cx - std::min(dw, switch_ui_text_width(name, sz, true)) * 0.5f, ty, sz, dw, kTextR, kTextG, kTextB, 1.0f, name, true);
    }
    ty += name_size + 6.0f;
    std::string date = g ? format_date(g->date) : std::string();
    if (folder_sel) {
        const int fn = (int)games_of_folder(sel_folder).size();
        char cnt[24];
        snprintf(cnt, sizeof(cnt), fn == 1 ? "%d game" : "%d games", fn);
        date = switch_tr(cnt);
    }
    if (!date.empty()) {
        switch_ui_draw_text(cx - switch_ui_text_width(date, date_size) * 0.5f, ty, date_size, kTextR, kTextG, kTextB, 0.6f, date);
    }
    if (show_rec) {
        ty += date_size + 8.0f;
        const std::string rec = switch_tr("High Scores") + ":  " + records_line(g->ref);
        const float rw = std::min(dw, switch_ui_text_width(rec, rec_size));
        draw_text_fit(cx - rw * 0.5f, ty, rec_size, dw, 1.0f, 0.77f, 0.31f, 1.0f, rec, false);
    }

    // --- list ---
    const float row_h = 46.0f, row_gap = 4.0f;
    const int rows_visible = std::max(1, (int)((lh + row_gap) / (row_h + row_gap)));
    const int n = (int)g_group_games.size();
    if (g_group_cursor < g_group_scroll) g_group_scroll = g_group_cursor;
    if (g_group_cursor >= g_group_scroll + rows_visible) g_group_scroll = g_group_cursor - rows_visible + 1;
    if (g_group_scroll > std::max(0, n - rows_visible)) g_group_scroll = std::max(0, n - rows_visible);

    for (int i = g_group_scroll; i < n && i < g_group_scroll + rows_visible; i++) {
        const uint16_t idx = g_group_games[i];
        const float y = ly + (i - g_group_scroll) * (row_h + row_gap);
        const bool sel = (i == g_group_cursor);
        menu_touch_item(lx, y, lw, row_h + row_gap, sel, [i] { g_group_cursor = i; sync_group_selection(); }, HidNpadButton_A);
        if (sel) {
            switch_ui_draw_rect(lx - 3.0f, y - 3.0f, lw + 6.0f, row_h + 6.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
            switch_ui_draw_rect(lx, y, lw, row_h, kSelR, kSelG, kSelB, 1.0f);
        } else {
            switch_ui_draw_rect(lx, y, lw, row_h, kTileR, kTileG, kTileB, 1.0f);
        }
        if (is_folder_entry(idx)) {
            // The folder: its name, and how many games it holds + an arrow.
            const uint8_t folder = entry_folder(idx);
            char cnt[24];
            const int fn = (int)games_of_folder(folder).size();
            snprintf(cnt, sizeof(cnt), fn == 1 ? "%d game" : "%d games", fn);
            const std::string cs = switch_tr(cnt) + "  >";
            float fr = kHighlightR, fg = kHighlightG, fb = kHighlightB;
            if (g_maker_colors) maker_color_of(t.mfr, fr, fg, fb);
            const float cw = switch_ui_text_width(cs, 18.0f);
            switch_ui_draw_rect(lx, y, 6.0f, row_h, fr, fg, fb, 1.0f);
            draw_text_fit(lx + 14.0f, y + 11.0f, 22.0f, lw - 42.0f - cw, kTextR, kTextG, kTextB, 1.0f, folder_label(folder), true);
            switch_ui_draw_text(lx + lw - 14.0f - cw, y + 13.0f, 18.0f, fr, fg, fb, 1.0f, cs);
            continue;
        }
        const std::string ref = ref_of(idx);
        const bool fav = is_favorite(ref);
        const std::string year = date_of(idx).substr(0, 4);
        const float year_w = switch_ui_text_width(year, 18.0f);
        const float right_pad = 14.0f + year_w + (fav ? 34.0f : 0.0f);
        // In Favorites/Recent the maker matters too: "Name  ·  MAKER" would be
        // too long, so it's left to the details panel.
        draw_text_fit(lx + 14.0f, y + 11.0f, 22.0f, lw - 28.0f - right_pad, kTextR, kTextG, kTextB, 1.0f, name_of(idx), false);
        switch_ui_draw_text(lx + lw - 14.0f - year_w, y + 13.0f, 18.0f, kTextR, kTextG, kTextB, 0.6f, year);
        if (fav) draw_star(lx + lw - 14.0f - year_w - 20.0f, y + row_h * 0.5f, 11.0f, 1.0f, 0.80f, 0.24f, 1.0f);
    }
    if (n > rows_visible) {
        const float thumb_h = lh * rows_visible / n;
        const float thumb_y = ly + (lh - thumb_h) * g_group_scroll / std::max(1, n - rows_visible);
        switch_ui_draw_rect(lx + lw + 8.0f, ly, 4.0f, lh, kTextR, kTextG, kTextB, 0.15f);
        switch_ui_draw_rect(lx + lw + 8.0f, thumb_y, 4.0f, thumb_h, kTextR, kTextG, kTextB, 0.6f);
    }

    HintBar h(logical_h);
    const bool on_folder = group_cursor_on_folder();
    h.icon(SwitchButtonIcon::A);
    h.text(on_folder ? "Open" : "Play");
    h.icon(SwitchButtonIcon::B);
    h.text("Back");
    if (!on_folder) {
        h.icon(SwitchButtonIcon::Y);
        h.text(is_favorite(ref_of(g_selected_index)) ? "Unfavorite" : "Favorite");
    }
    if (t.kind == TileKind::Maker) {
        h.icon(SwitchButtonIcon::Minus);
        h.text(g_group_by_date ? "Sort: Date" : "Sort: Name");
        h.icon(SwitchButtonIcon::L);
        h.icon(SwitchButtonIcon::R);
        h.text("Maker");
    }
}

// ---------------------------------------------------------------------------
// List maker menu (Menu Style = List): the tiles (Favorites, Recent, the
// makers) as a column of rows - colour stripe, name, game count and a bar -
// and beside it the chosen one: its name and years, then its games, each
// with a small picture of its console and its full date. Up/Down move, L/R
// a page, A opens it (the usual one-tile list). Portrait: the column on
// top, the games below.
// ---------------------------------------------------------------------------
int g_list_scroll = 0;
// A / Right on the column moves the cursor into the chosen one's games
// (same screen); B / Left goes back to the column.
bool g_list_in_games = false;
int g_list_game = 0, g_list_game_scroll = 0;
int g_list_game_count = 0, g_list_game_fit = 1;  // of the last frame drawn
uint16_t g_list_entry = 0;  // the row under the game cursor (kFolderEntry = a folder), last frame

// Console art for the game rows: just the console, cropped out of its
// canvas (GW_rom::console_info) and shrunk to row size, kept for the last
// kListThumbMax games shown. Decoding a console PNG is slow, so at most one
// is loaded per frame - the others show a moment later.
struct ListThumb {
    unsigned tex = 0;
    int w = 0, h = 0;
    uint64_t used = 0;
};
constexpr size_t kListThumbMax = 40;
constexpr int kListThumbH = 96;  // stored height in pixels
std::vector<std::pair<uint16_t, ListThumb>> g_list_thumbs;
uint64_t g_list_frame = 0;
int g_list_loads_left = 0;  // decodes still allowed this frame

const ListThumb* list_thumb(uint16_t idx) {
    for (auto& p : g_list_thumbs)
        if (p.first == idx) { p.second.used = g_list_frame; return &p.second; }
    if (g_list_loads_left <= 0) return nullptr;  // a later frame
    g_list_loads_left--;
    ListThumb t;
    t.used = g_list_frame;
    // The folder's row (key 0xFF00 + folder id): its own console art
    const bool folder = idx >= 0xFF00;
    const uint8_t fid = (uint8_t)(idx & 0xFF);
    const GW_rom* g = load_game(folder ? row_game((uint16_t)(kFolderBase + fid), folder_parent(fid)) : idx);
    if (g && g->console_info) {
        std::string asset = t3x_path_to_png_asset(g->path_console);
        const uint8_t* data = nullptr;
        size_t size = 0;
        if (folder && gw_pack::get_file_bytes(folder_art(fid), data, size) && data && size) asset = folder_art(fid);
        if (!asset.empty() && gw_pack::get_file_bytes(asset, data, size) && data && size) {
            int w = 0, h = 0, ch = 0;
            stbi_uc* px = stbi_load_from_memory(data, (int)size, &w, &h, &ch, 4);
            if (px) {
                const uint16_t* ci = g->console_info;
                const int cx = std::min((int)ci[2], w - 1), cy = std::min((int)ci[3], h - 1);
                const int cw = std::max(1, std::min((int)ci[4], w - cx)), chh = std::max(1, std::min((int)ci[5], h - cy));
                const int oh = std::min(kListThumbH, chh), ow = std::max(1, cw * oh / chh);
                // Box-average the crop down to ow x oh.
                std::vector<uint8_t> out((size_t)ow * oh * 4);
                for (int oy = 0; oy < oh; oy++) {
                    const int sy0 = cy + oy * chh / oh, sy1 = std::max(sy0 + 1, cy + (oy + 1) * chh / oh);
                    for (int ox = 0; ox < ow; ox++) {
                        const int sx0 = cx + ox * cw / ow, sx1 = std::max(sx0 + 1, cx + (ox + 1) * cw / ow);
                        unsigned acc[4] = {0, 0, 0, 0}, n = 0;
                        for (int yy = sy0; yy < sy1; yy += 2)
                            for (int xx = sx0; xx < sx1; xx += 2) {
                                const stbi_uc* q = px + ((size_t)yy * w + xx) * 4;
                                acc[0] += q[0]; acc[1] += q[1]; acc[2] += q[2]; acc[3] += q[3];
                                n++;
                            }
                        uint8_t* o = &out[((size_t)oy * ow + ox) * 4];
                        for (int k = 0; k < 4; k++) o[k] = (uint8_t)(acc[k] / (n ? n : 1));
                    }
                }
                stbi_image_free(px);
                glGenTextures(1, &t.tex);
                glBindTexture(GL_TEXTURE_2D, t.tex);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ow, oh, 0, GL_RGBA, GL_UNSIGNED_BYTE, out.data());
                t.w = ow;
                t.h = oh;
            }
        }
    }
    // Full: drop the one shown longest ago.
    if (g_list_thumbs.size() >= kListThumbMax) {
        size_t old = 0;
        for (size_t i = 1; i < g_list_thumbs.size(); i++)
            if (g_list_thumbs[i].second.used < g_list_thumbs[old].second.used) old = i;
        if (g_list_thumbs[old].second.tex) glDeleteTextures(1, &g_list_thumbs[old].second.tex);
        g_list_thumbs.erase(g_list_thumbs.begin() + old);
    }
    g_list_thumbs.push_back({idx, t});
    return &g_list_thumbs.back().second;
}

int list_rows_visible(int logical_w, int logical_h) {
    const bool landscape = logical_w >= logical_h;
    const float top = kTopBarH + 16.0f;
    const float bottom = landscape ? (float)(logical_h - kBottomBarH) - 12.0f : top + ((float)(logical_h - kBottomBarH) - top) * 0.5f;
    return std::max(1, (int)((bottom - top + 6.0f) / 46.0f));
}

void render_maker_list(int logical_w, int logical_h) {
    char total[64];
    snprintf(total, sizeof(total), "%u games  |  %s", (unsigned)g_sorted_order.size(), maker_order_label(g_maker_order));
    draw_top_right(logical_w, total);
    const int n = (int)g_tiles.size();
    HintBar hints(logical_h);
    if (g_list_in_games) {
        const bool on_folder = is_folder_entry(g_list_entry);
        hints.icon(SwitchButtonIcon::A);
        hints.text(on_folder ? "Open" : "Play");
        hints.icon(SwitchButtonIcon::DpadUp);
        hints.icon(SwitchButtonIcon::DpadDown);
        hints.text("Game");
        if (!on_folder) {
            hints.icon(SwitchButtonIcon::Y);
            hints.text(is_favorite(ref_of(g_selected_index)) ? "Unfavorite" : "Favorite");
        }
        hints.icon(SwitchButtonIcon::B);
        hints.text("Back");
    } else {
        hints.icon(SwitchButtonIcon::A);
        hints.text("Open");
        hints.icon(SwitchButtonIcon::DpadUp);
        hints.icon(SwitchButtonIcon::DpadDown);
        hints.text("Maker");
        hints.icon(SwitchButtonIcon::L);
        hints.icon(SwitchButtonIcon::R);
        hints.text("Page");
        hints.icon(SwitchButtonIcon::Y);
        hints.text("All Games");
        hints.icon(SwitchButtonIcon::Plus);
        hints.text("Settings");
    }
    if (n == 0) return;

    const bool landscape = logical_w >= logical_h;
    const float margin = 24.0f, top = kTopBarH + 16.0f, bottom = (float)(logical_h - kBottomBarH) - 12.0f;
    // Column rect and panel rect.
    float lx, lw, ly, lh, px, pw, py, ph;
    if (landscape) {
        lx = margin; lw = (float)logical_w * 0.37f; ly = top; lh = bottom - top;
        px = lx + lw + 34.0f; pw = (float)logical_w - margin - px; py = top; ph = bottom - top;
    } else {
        lx = margin; lw = (float)logical_w - margin * 2.0f; ly = top; lh = (bottom - top) * 0.5f;
        px = margin; pw = lw; py = ly + lh + 12.0f; ph = bottom - py;
    }

    // --- the column ---
    const float row_h = 46.0f, row_gap = 6.0f;
    const int rows = list_rows_visible(logical_w, logical_h);
    if (g_tile_cursor < g_list_scroll) g_list_scroll = g_tile_cursor;
    if (g_tile_cursor >= g_list_scroll + rows) g_list_scroll = g_tile_cursor - rows + 1;
    if (g_list_scroll > std::max(0, n - rows)) g_list_scroll = std::max(0, n - rows);
    int max_count = 1;
    for (const MakerTile& t : g_tiles) if (t.kind == TileKind::Maker) max_count = std::max(max_count, t.count);
    for (int i = g_list_scroll; i < n && i < g_list_scroll + rows; i++) {
        const MakerTile& t = g_tiles[i];
        const float y = ly + (float)(i - g_list_scroll) * row_h;
        const float h = row_h - row_gap;
        const bool sel = i == g_tile_cursor;
        menu_touch_item(lx, y, lw, row_h, sel && !g_list_in_games, [i] { g_tile_cursor = i; g_list_in_games = false; g_list_game = 0; }, HidNpadButton_A);
        if (sel) {
            // In the games, the maker keeps a dimmer frame.
            switch_ui_draw_rect(lx - 3.0f, y - 3.0f, lw + 6.0f, h + 6.0f, kHighlightR, kHighlightG, kHighlightB, g_list_in_games ? 0.35f : 1.0f);
            switch_ui_draw_rect(lx, y, lw, h, kSelR, kSelG, kSelB, 1.0f);
        } else {
            switch_ui_draw_rect(lx, y, lw, h, kTileR, kTileG, kTileB, 1.0f);
        }
        float mr = kAccentR, mg = kAccentG, mb = kAccentB;
        if (t.kind == TileKind::Favorites) { mr = 1.0f; mg = 0.80f; mb = 0.24f; }
        else if (g_maker_colors) maker_color(t, mr, mg, mb);
        switch_ui_draw_rect(lx, y, 6.0f, h, mr, mg, mb, 1.0f);
        char cnt[24];
        snprintf(cnt, sizeof(cnt), t.count == 1 ? "%d game" : "%d games", t.count);
        const std::string cs = switch_tr(cnt);
        const float cw = switch_ui_text_width(cs, 17.0f);
        switch_ui_draw_text(lx + lw - 14.0f - cw, y + 11.0f, 17.0f, kTextR, kTextG, kTextB, 0.6f, cs);
        // The name shrinks (then is cut) to stay clear of the count.
        draw_text_fit(lx + 20.0f, y + 8.0f, 20.0f, lw - 20.0f - cw - 28.0f, kTextR, kTextG, kTextB, 1.0f, t.label, true);
        if (t.kind == TileKind::Maker) {
            const float bw = std::max(5.0f, (lw - 40.0f) * (float)t.count / (float)max_count);
            switch_ui_draw_rect(lx + 20.0f, y + h - 6.0f, bw, 3.0f, mr, mg, mb, 1.0f);
        }
    }
    if (n > rows) {
        const float th = lh * rows / n, tyy = ly + (lh - th) * g_list_scroll / std::max(1, n - rows);
        switch_ui_draw_rect(lx + lw + 10.0f, ly, 4.0f, lh, kTextR, kTextG, kTextB, 0.15f);
        switch_ui_draw_rect(lx + lw + 10.0f, tyy, 4.0f, th, kTextR, kTextG, kTextB, 0.6f);
    }

    // --- the chosen one ---
    const MakerTile& t = g_tiles[g_tile_cursor];
    // Each tile's games, worked out once (again when the tiles change).
    static std::vector<std::vector<uint16_t>> s_cache;
    static std::vector<bool> s_have;
    static const std::vector<MakerTile>* s_of = nullptr;
    static size_t s_n = 0;
    static uint64_t s_sig = 0;
    const uint64_t sig = (uint64_t)g_sorted_order.size() * 131u + (g_group_by_date ? 1u : 0u) + g_favorites.size() * 7919u + g_recent.size() * 104729u
                       + (uint64_t)g_folder * 1299709u;
    if (s_of != &g_tiles || s_n != g_tiles.size() || s_sig != sig) {
        s_cache.assign(g_tiles.size(), {});
        s_have.assign(g_tiles.size(), false);
        s_of = &g_tiles;
        s_n = g_tiles.size();
        s_sig = sig;
    }
    if (!s_have[g_tile_cursor]) {
        s_cache[g_tile_cursor] = games_of_tile(t);
        s_have[g_tile_cursor] = true;
    }
    const std::vector<uint16_t>& s_games = s_cache[g_tile_cursor];
    g_list_frame++;
    g_list_loads_left = 2;
    float mr = kAccentR, mg = kAccentG, mb = kAccentB;
    if (t.kind == TileKind::Favorites) { mr = 1.0f; mg = 0.80f; mb = 0.24f; }
    else if (g_maker_colors) maker_color(t, mr, mg, mb);
    switch_ui_draw_rect(px, py, pw, ph, kPanelR, kPanelG, kPanelB, 1.0f);
    switch_ui_draw_rect(px, py, pw, 10.0f, mr, mg, mb, 1.0f);
    draw_text_fit(px + 26.0f, py + 26.0f, 38.0f, pw - 52.0f, kTextR, kTextG, kTextB, 1.0f, t.label, true);
    // Inside a folder: its name on a second line ("> GAME POCKET COMPUTER").
    const bool in_folder = g_folder && t.kind == TileKind::Maker && g_folder != t.mfr && folder_parent(g_folder) == t.mfr;
    const float sub_dy = in_folder ? 40.0f : 0.0f;
    if (in_folder) {
        std::string f = folder_label(g_folder);
        for (char& c : f) c = (char)toupper((unsigned char)c);
        draw_text_fit(px + 26.0f, py + 70.0f, 28.0f, pw - 52.0f, kTextR, kTextG, kTextB, 1.0f, "> " + f, true);
    }
    {
        int y0 = 0, y1 = 0;
        for (uint16_t idx : s_games) {
            if (is_folder_entry(idx)) continue;
            const int yr = parse_year(date_of(idx));
            if (yr <= 0) continue;
            if (!y0 || yr < y0) y0 = yr;
            if (yr > y1) y1 = yr;
        }
        char cnt[24];
        snprintf(cnt, sizeof(cnt), s_games.size() == 1 ? "%d game" : "%d games", (int)s_games.size());
        std::string sub = switch_tr(cnt);
        if (y0) sub += y0 == y1 ? "  ·  " + std::to_string(y0) : "  ·  " + std::to_string(y0) + " - " + std::to_string(y1);
        switch_ui_draw_text(px + 26.0f, py + 74.0f + sub_dy, 20.0f, mr, mg, mb, 1.0f, sub);
    }
    const float gx = px + 26.0f, gw = pw - 52.0f, grow = 60.0f;
    float gy = py + 112.0f + sub_dy;
    const int fit = std::max(0, (int)((py + ph - 30.0f - gy) / grow));
    g_list_game_count = (int)s_games.size();
    g_list_game_fit = std::max(1, fit);
    if (g_list_game >= g_list_game_count) g_list_game = std::max(0, g_list_game_count - 1);
    if (g_list_game < g_list_game_scroll) g_list_game_scroll = g_list_game;
    if (g_list_game >= g_list_game_scroll + g_list_game_fit) g_list_game_scroll = g_list_game - g_list_game_fit + 1;
    if (!g_list_in_games) g_list_game_scroll = 0;
    g_list_entry = (g_list_game < g_list_game_count) ? s_games[g_list_game] : 0;
    if (g_list_in_games && g_list_game < g_list_game_count) g_selected_index = row_game(s_games[g_list_game], t.mfr);
    g_sel_is_folder = g_list_in_games && is_folder_entry(g_list_entry);
    for (int i = g_list_game_scroll; i < (int)s_games.size() && i < g_list_game_scroll + fit; i++) {
        const bool folder_row = is_folder_entry(s_games[i]);
        const uint16_t idx = row_game(s_games[i], t.mfr);
        const float h = grow - 6.0f;
        const bool gsel = g_list_in_games && i == g_list_game;
        // Tap a game: the cursor goes to it; tap it again: play.
        menu_touch_item(gx, gy, gw, grow, gsel, [i] { g_list_in_games = true; g_list_game = i; }, HidNpadButton_A);
        if (gsel) {
            switch_ui_draw_rect(gx - 3.0f, gy - 3.0f, gw + 6.0f, h + 6.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
            switch_ui_draw_rect(gx, gy, gw, h, kSelR, kSelG, kSelB, 1.0f);
        } else {
            switch_ui_draw_rect(gx, gy, gw, h, kTileR, kTileG, kTileB, 1.0f);
        }
        // Picture in the middle; name on its left, full date centred on its right.
        const float thh = h - 8.0f;
        float tw = thh * 4.0f / 3.0f;
        const GW_rom* g = load_game(idx);
        const ListThumb* th = list_thumb(folder_row ? (uint16_t)(0xFF00 + entry_folder(s_games[i])) : idx);
        if (g && g->console_info && g->console_info[4] > 0 && g->console_info[5] > 0)
            tw = thh * (float)g->console_info[4] / (float)g->console_info[5];
        tw = std::min(tw, gw * 0.3f);
        const float tx = gx + (gw - tw) * 0.5f, ty = gy + 4.0f;
        // Not loaded yet: nothing - it just appears a frame or two later.
        if (th && th->tex) switch_ui_draw_texture_plain(th->tex, tx, ty, tw, thh);
        if (folder_row) {
            // The folder: its name, and its game count + an arrow on the right.
            const uint8_t folder = entry_folder(s_games[i]);
            const int fn = (int)games_of_folder(folder).size();
            char cnt[24];
            snprintf(cnt, sizeof(cnt), fn == 1 ? "%d game" : "%d games", fn);
            const std::string cs = switch_tr(cnt) + "  >";
            switch_ui_draw_rect(gx, gy, 6.0f, h, mr, mg, mb, 1.0f);
            draw_text_fit(gx + 14.0f, gy + (h - 19.0f) * 0.5f, 19.0f, tx - gx - 28.0f, kTextR, kTextG, kTextB, 1.0f, folder_label(folder), true);
            const float cw = std::min(switch_ui_text_width(cs, 17.0f), gx + gw - tx - tw - 30.0f);
            draw_text_fit(gx + gw - 14.0f - cw, gy + (h - 17.0f) * 0.5f, 17.0f, gx + gw - tx - tw - 30.0f, mr, mg, mb, 1.0f, cs, false);
            gy += grow;
            continue;
        }
        draw_text_fit(gx + 14.0f, gy + (h - 19.0f) * 0.5f, 19.0f, tx - gx - 28.0f, kTextR, kTextG, kTextB, 1.0f, name_of(idx), false);
        const std::string date = format_date(date_of(idx));
        const float rx0 = tx + tw, rx1 = gx + gw;
        // Right-aligned against the row's edge.
        const float dw = std::min(switch_ui_text_width(date, 17.0f), rx1 - rx0 - 30.0f);
        draw_text_fit(rx1 - 14.0f - dw, gy + (h - 17.0f) * 0.5f, 17.0f, rx1 - rx0 - 30.0f, kTextR, kTextG, kTextB, 0.65f, date, false);
        gy += grow;
    }
    // Spare loads this frame: the next and previous makers' first games, so
    // they're usually ready when the cursor gets there.
    for (int d : {1, -1}) {
        const int ti = g_tile_cursor + d;
        if (g_list_loads_left <= 0 || ti < 0 || ti >= (int)g_tiles.size()) continue;
        if (!s_have[ti]) {
            s_cache[ti] = games_of_tile(g_tiles[ti]);
            s_have[ti] = true;
        }
        for (int k = 0; k < (int)s_cache[ti].size() && k < fit && g_list_loads_left > 0; k++) list_thumb(s_cache[ti][k]);
    }
    if (!g_list_in_games && (int)s_games.size() > fit && fit > 0) {
        char more[48];
        snprintf(more, sizeof(more), switch_tr("and %d more").c_str(), (int)s_games.size() - fit);
        switch_ui_draw_text(gx, gy + 2.0f, 17.0f, kTextR, kTextG, kTextB, 0.6f, std::string("... ") + more);
    }
}

// Moves the tile cursor to the previous/next manufacturer tile (skipping
// Favorites/Recent) and opens it - L/R inside MakerGames.
void step_maker_tile(int dir) {
    const int n = (int)g_tiles.size();
    for (int s = 1; s < n; s++) {
        const int i = ((g_tile_cursor + dir * s) % n + n) % n;
        if (g_tiles[i].kind == TileKind::Maker) {
            g_tile_cursor = i;
            g_folder = 0;
            open_tile();
            return;
        }
    }
}

} // namespace

void switch_menu_init() {
    switch_ui_init();
    switch_ui_set_font(g_menu_font);
    apply_menu_theme();
    load_header_icon_if_needed();
    g_selected_index = switch_get_default_game_index();
    rebuild_sorted_order();
    rebuild_tiles();
    g_view = (g_menu_style == MenuStyle::Classic) ? MenuView::Classic : MenuView::Makers;
    // Start on the last-played game's manufacturer tile.
    {
        const uint8_t m = get_mfr(g_selected_index);
        for (size_t i = 0; i < g_tiles.size(); i++) {
            if (g_tiles[i].kind == TileKind::Maker && g_tiles[i].mfr == m) g_tile_cursor = (int)i;
        }
    }

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
    if (g_pause_art_tex) glDeleteTextures(1, &g_pause_art_tex);
    g_pause_art_tex = 0;
    g_pause_art_index = -1;
    switch_ui_shutdown();
}

bool switch_menu_is_active() {
    return g_menu_active;
}

bool switch_menu_settings_open() {
    return g_settings_active;
}


namespace {
// ---------------------------------------------------------------------------
// Save slots (core/save_slots.h): the card screen shown when entering a game
// that has saves (Load State Behavior = Ask), the pause menu's Save State /
// Load State / Simulate Time screens, and the pet "time away" question.
// ---------------------------------------------------------------------------

// Simulate Time's choices, in minutes, then Custom (hours, typed in).
constexpr int kSimMinutes[] = {15, 30, 60, 120, 180, 300, 360, 600, 720, 1440, 2880, 4320, 10080};
constexpr const char* kSimLabels[] = {"15 minutes", "30 minutes", "1 hour", "2 hours", "3 hours", "5 hours", "6 hours",
                                      "10 hours", "12 hours", "24 hours", "2 days", "3 days", "1 week", "Custom..."};
constexpr int kNumSimFixed = (int)(sizeof(kSimMinutes) / sizeof(kSimMinutes[0]));
constexpr int kNumSimOptions = kNumSimFixed + 1;  // + Custom

uint16_t slot_game_index() { return g_slot_screen == SlotScreen::Entry ? g_selected_index : g_game_index; }
int slot_card_count() { return g_slot_screen == SlotScreen::Save ? kNumSaveSlots : kNumSaveSlots + 1; }
// Card index -> slot number (the Save screen has no Auto save card).
int slot_of_card(int card) { return g_slot_screen == SlotScreen::Save ? card + 1 : card; }

void open_slot_screen(SlotScreen s) {
    g_slot_screen = s;
    g_slot_confirm = SlotConfirm::None;
    g_slot_sel = (s == SlotScreen::Simulate) ? 2 : 0;
    if (s != SlotScreen::Simulate) save_slots_refresh(s == SlotScreen::Entry ? g_selected_index : g_game_index);
    if (s == SlotScreen::Entry) {
        // Start on the newest save.
        int best = 0;
        int64_t best_t = -1;
        for (int i = 0; i <= kNumSaveSlots; ++i) {
            const SaveSlotInfo& in = save_slot_info(i);
            if (in.exists && in.saved_at > best_t) { best = i; best_t = in.saved_at; }
        }
        g_slot_sel = best;
    }
}

void close_slot_screen() {
    g_slot_screen = SlotScreen::None;
    g_slot_confirm = SlotConfirm::None;
    save_slots_free();
}

// Leaves every menu screen and lets the game run.
void resume_game_from_menu() {
    close_slot_screen();
    g_pet_prompt_active = false;
    g_settings_active = false;
    g_settings_from_gameplay = false;
    g_menu_active = false;
}

void show_slot_toast(const std::string& text) {
    g_slot_toast = text;
    g_slot_toast_frames = 120;
}

// After a slot's state is loaded into g_cpu: Pet Time, then into the game.
// Older manual slots of a pet under Real Time ask first (catch up the time
// since, or load as is) - the automatic save keeps its usual behavior.
void finish_slot_load(int slot) {
    g_cpu->segments_state_are_update = true;
    update_segments_from_cpu(g_cpu.get());
    const int64_t age = pet_time_state_age();
    if (slot > 0 && age >= 3600 && pet_time_would_catch_up()) {
        close_slot_screen();
        g_pet_prompt_active = true;
        g_pet_prompt_age = age;
        g_pet_prompt_sel = 0;
        return;
    }
    pet_time_on_state_loaded();
    resume_game_from_menu();
}

void load_slot_now(int slot) {
    if (g_slot_screen == SlotScreen::Entry) {
        note_recent(g_selected_index);
        switch_load_game_by_index_and_init(g_selected_index);
    }
    if (!g_cpu || !load_game_state_slot(g_cpu.get(), g_game_index, slot)) {
        YOKOI_LOG("switch_menu: loading slot %d failed", slot);
        if (g_slot_screen == SlotScreen::Entry) {
            resume_game_from_menu();  // the game is loaded fresh - play that
        } else {
            show_slot_toast("Couldn't load that save");
            close_slot_screen();
        }
        return;
    }
    YOKOI_LOG("switch_menu: loaded slot %d of index %u", slot, (unsigned)g_game_index);
    finish_slot_load(slot);
}

void save_slot_now(int slot) {
    const bool ok = g_cpu && save_game_state_slot(g_cpu.get(), g_game_index, slot);
    if (ok) save_slot_write_thumbnail(g_game_index, slot);
    YOKOI_LOG("switch_menu: save to slot %d %s", slot, ok ? "ok" : "FAILED");
    show_slot_toast(ok ? ("Saved to Slot " + std::to_string(slot)) : std::string("This game can't be saved"));
    close_slot_screen();
}

void delete_slot_now(int slot) {
    const uint16_t idx = slot_game_index();
    delete_game_state_slot(idx, slot);
    save_slot_delete_thumbnail(idx, slot);
    save_slots_refresh(idx);
    // Nothing left to load on the entry screen -> straight back to the list.
    if (g_slot_screen == SlotScreen::Entry) {
        bool any = false;
        for (int i = 0; i <= kNumSaveSlots; ++i) any = any || save_slot_info(i).exists;
        if (!any) close_slot_screen();
    }
}

void simulate_now(int minutes) {
    if (!g_cpu || minutes <= 0) return;
    // The copy Undo Simulate goes back to.
    g_undo_available = save_game_state_slot(g_cpu.get(), g_game_index, kUndoSlot);
    g_undo_cpu = g_cpu.get();
    YOKOI_LOG("switch_menu: simulate %d min (undo copy %s)", minutes, g_undo_available ? "saved" : "FAILED");
    pet_time_simulate((int64_t)minutes * 60);
    resume_game_from_menu();
}

void undo_simulate_now() {
    if (!g_cpu || !undo_simulate_available()) return;
    if (load_game_state_slot(g_cpu.get(), g_game_index, kUndoSlot)) {
        pet_time_after_undo();
        delete_game_state_slot(g_game_index, kUndoSlot);
        g_undo_available = false;
        resume_game_from_menu();
    } else {
        show_slot_toast("Couldn't undo");
    }
}

bool menu_portrait() { return g_menu_rotation != ScreenRotation::Deg0; }

void update_slot_screen(uint64_t kDown) {
    if (g_slot_screen == SlotScreen::Simulate) {
        if (kDown & HidNpadButton_Down) g_slot_sel = (g_slot_sel + 1) % kNumSimOptions;
        if (kDown & HidNpadButton_Up) g_slot_sel = (g_slot_sel + kNumSimOptions - 1) % kNumSimOptions;
        if (kDown & HidNpadButton_B) { close_slot_screen(); return; }
        if (kDown & HidNpadButton_A) {
            if (g_slot_sel < kNumSimFixed) {
                simulate_now(kSimMinutes[g_slot_sel]);
            } else {
                std::string text;
                if (show_keyboard(SwkbdType_NumPad, "Simulate Time", "Hours (1-9999)", "", 4, text)) {
                    const int h = atoi(text.c_str());
                    if (h >= 1 && h <= 9999) simulate_now(h * 60);
                }
            }
        }
        return;
    }

    const int n = slot_card_count();
    const bool entry = g_slot_screen == SlotScreen::Entry;
    if (g_slot_confirm != SlotConfirm::None) {
        const int slot = slot_of_card(g_slot_sel);
        if (kDown & HidNpadButton_A) {
            const SlotConfirm c = g_slot_confirm;
            g_slot_confirm = SlotConfirm::None;
            if (c == SlotConfirm::Overwrite) save_slot_now(slot);
            else delete_slot_now(slot);
        } else if (kDown & (HidNpadButton_B | HidNpadButton_Y)) {
            g_slot_confirm = SlotConfirm::None;
        }
        return;
    }
    if (kDown & HidNpadButton_B) { close_slot_screen(); return; }
    if (menu_portrait() && (kDown & (HidNpadButton_Up | HidNpadButton_Down))) {
        // Portrait: the cards are a column - Up/Down walk it, New Game last.
        const int last = entry ? n : n - 1;
        if (kDown & HidNpadButton_Down) g_slot_sel = g_slot_sel >= last ? 0 : g_slot_sel + 1;
        else g_slot_sel = g_slot_sel <= 0 ? last : g_slot_sel - 1;
        return;
    }
    if (g_slot_sel < n) {
        if (kDown & HidNpadButton_Right) g_slot_sel = (g_slot_sel + 1) % n;
        if (kDown & HidNpadButton_Left) g_slot_sel = (g_slot_sel + n - 1) % n;
        if (entry && (kDown & HidNpadButton_Down)) { g_slot_last_card = g_slot_sel; g_slot_sel = n; return; }
    } else if (kDown & HidNpadButton_Up) {  // on New Game
        g_slot_sel = g_slot_last_card;
        return;
    }
    if (kDown & HidNpadButton_A) {
        if (entry && g_slot_sel == n) {  // New Game
            YOKOI_LOG("switch_menu: entry screen - new game for index %u", (unsigned)g_selected_index);
            note_recent(g_selected_index);
            switch_load_game_by_index_and_init(g_selected_index);
            resume_game_from_menu();
            return;
        }
        const int slot = slot_of_card(g_slot_sel);
        const bool exists = save_slot_info(slot).exists;
        if (g_slot_screen == SlotScreen::Save) {
            if (exists) g_slot_confirm = SlotConfirm::Overwrite;
            else save_slot_now(slot);
        } else if (exists) {
            load_slot_now(slot);
        }
        return;
    }
    if (kDown & HidNpadButton_Y) {
        const int slot = g_slot_sel < n ? slot_of_card(g_slot_sel) : 0;
        if (slot > 0 && save_slot_info(slot).exists) g_slot_confirm = SlotConfirm::Delete;
    }
}

void update_pet_prompt(uint64_t kDown) {
    if (kDown & (HidNpadButton_Left | HidNpadButton_Right)) g_pet_prompt_sel ^= 1;
    if (kDown & HidNpadButton_A) {
        if (g_pet_prompt_sel == 0) pet_time_on_state_loaded();  // catch the time up
        else pet_time_sync_clock();                             // as it was, clock set to now
        resume_game_from_menu();
    }
}

// ---- drawing ----
void draw_outline(float x, float y, float w, float h, float t, float r, float g, float b, float a) {
    switch_ui_draw_rect(x, y, w, t, r, g, b, a);
    switch_ui_draw_rect(x, y + h - t, w, t, r, g, b, a);
    switch_ui_draw_rect(x, y, t, h, r, g, b, a);
    switch_ui_draw_rect(x + w - t, y, t, h, r, g, b, a);
}

void draw_text_centered(float cx, float y, float size, float r, float g, float b, float a, const std::string& s, bool bold = false) {
    switch_ui_draw_text(cx - switch_ui_text_width(s, size, bold) * 0.5f, y, size, r, g, b, a, s, bold);
}

// Bottom-bar hints: pairs of (icon, text).
void draw_hints(int logical_h, std::initializer_list<std::pair<SwitchButtonIcon, const char*>> items) {
    HintBar h(logical_h);
    for (const auto& it : items) {
        h.icon(it.first);
        h.text(it.second);
    }
}

void render_slot_cards(int logical_w, int logical_h) {
    const bool entry = g_slot_screen == SlotScreen::Entry;
    const GW_rom* g = load_game(slot_game_index());
    const std::string game_name = g ? g->name : std::string("?");
    const char* what = entry ? "Load Game" : (g_slot_screen == SlotScreen::Save ? "Save State" : "Load State");
    switch_ui_draw_text((float)logical_w - 24.0f - switch_ui_text_width(what, 20.0f), 20.0f, 20.0f, kTextR, kTextG, kTextB, 0.85f, what);
    draw_text_centered((float)logical_w * 0.5f, (float)kTopBarH + 22.0f, 28.0f, kTextR, kTextG, kTextB, 1.0f, game_name, true);

    const int n = slot_card_count();
    const bool portrait = logical_w < logical_h;
    // The text of one card: name, when, "where you left off".
    auto card_text = [&](int slot, const SaveSlotInfo& in, float cx, float ty, bool left_aligned) {
        const std::string name = slot == 0 ? std::string("Auto save") : ("Slot " + std::to_string(slot));
        const std::string when = in.exists ? std::string(in.saved_at ? save_slot_time_text(in.saved_at) : "Saved") : std::string("Empty");
        auto put = [&](float y, float size, float r, float g, float b, float a, const std::string& t, bool bold) {
            if (left_aligned) switch_ui_draw_text(cx, y, size, r, g, b, a, t, bold);
            else draw_text_centered(cx, y, size, r, g, b, a, t, bold);
        };
        put(ty, 26.0f, kTextR, kTextG, kTextB, 1.0f, name, true);
        put(ty + 38.0f, 19.0f, kTextR, kTextG, kTextB, 0.7f, when, false);
        if (slot == 0 && in.exists) put(ty + 68.0f, 17.0f, kAccentR, kAccentG, kAccentB, 1.0f, "where you left off", false);
    };
    auto card_thumb = [&](const SaveSlotInfo& in, float tx, float ty, float tw, float th) {
        if (in.tex) {
            switch_ui_draw_texture_plain(in.tex, tx, ty, tw, th);
        } else {
            switch_ui_draw_rect(tx, ty, tw, th, kBgR, kBgG, kBgB, 1.0f);
            draw_outline(tx, ty, tw, th, 2.0f, kTextR, kTextG, kTextB, 0.25f);
            draw_text_centered(tx + tw * 0.5f, ty + th * 0.5f - 20.0f, 34.0f, kTextR, kTextG, kTextB, 0.4f,
                               in.exists ? "?" : (g_slot_screen == SlotScreen::Save ? "+" : "-"));
        }
    };
    float y;
    if (portrait) {
        // Portrait: one card per row - the picture on the left, the text
        // beside it - so nothing has to squeeze into a quarter of 720.
        const float mx = 40.0f, cw = (float)logical_w - mx * 2.0f, ch = 140.0f, gap = 20.0f;
        const float y0 = (float)kTopBarH + 76.0f;
        for (int i = 0; i < n; ++i) {
            const int slot = slot_of_card(i);
            const SaveSlotInfo& in = save_slot_info(slot);
            const float cy = y0 + i * (ch + gap);
            const bool sel = (i == g_slot_sel);
            menu_touch_item(mx, cy, cw, ch, sel, [i] { g_slot_sel = i; }, HidNpadButton_A);
            switch_ui_draw_rect(mx, cy, cw, ch, kTileR, kTileG, kTileB, 1.0f);
            if (sel) draw_outline(mx - 5.0f, cy - 5.0f, cw + 10.0f, ch + 10.0f, 4.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
            const float th = ch - 28.0f, tw = th * 16.0f / 9.0f;
            card_thumb(in, mx + 14.0f, cy + 14.0f, tw, th);
            card_text(slot, in, mx + 14.0f + tw + 24.0f, cy + 22.0f, true);
        }
        y = y0 + n * (ch + gap) + 16.0f;
    } else {
        const float gap = 26.0f;
        const float cw = std::min(270.0f, ((float)logical_w - 80.0f - gap * (n - 1)) / n), ch = cw * 1.1f;
        const float x0 = ((float)logical_w - (n * cw + (n - 1) * gap)) * 0.5f;
        const float y0 = (float)kTopBarH + 82.0f;
        for (int i = 0; i < n; ++i) {
            const int slot = slot_of_card(i);
            const SaveSlotInfo& in = save_slot_info(slot);
            const float x = x0 + i * (cw + gap);
            const bool sel = (i == g_slot_sel);
            menu_touch_item(x, y0, cw, ch, sel, [i] { g_slot_sel = i; }, HidNpadButton_A);
            switch_ui_draw_rect(x, y0, cw, ch, kTileR, kTileG, kTileB, 1.0f);
            if (sel) draw_outline(x - 5.0f, y0 - 5.0f, cw + 10.0f, ch + 10.0f, 4.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
            const float tx = x + 14.0f, ty = y0 + 14.0f, tw = cw - 28.0f, th = tw * 9.0f / 16.0f;
            card_thumb(in, tx, ty, tw, th);
            card_text(slot, in, x + cw * 0.5f, ty + th + 16.0f, false);
        }
        y = y0 + ch + 36.0f;
    }
    if (entry) {
        const float bw = 340.0f, bh = 50.0f, bx = ((float)logical_w - bw) * 0.5f;
        const bool sel = g_slot_sel == n;
        menu_touch_item(bx, y, bw, bh, sel, [n] { if (g_slot_sel < n) g_slot_last_card = g_slot_sel; g_slot_sel = n; }, HidNpadButton_A);
        if (sel) switch_ui_draw_rect(bx, y, bw, bh, kSelR, kSelG, kSelB, 1.0f);
        draw_outline(bx, y, bw, bh, 2.0f, sel ? kHighlightR : kTextR, sel ? kHighlightG : kTextG, sel ? kHighlightB : kTextB, sel ? 1.0f : 0.35f);
        draw_text_centered((float)logical_w * 0.5f, y + 11.0f, 24.0f, kTextR, kTextG, kTextB, 1.0f, "New Game", sel);
        y += bh + 16.0f;
    }
    if (g_slot_confirm != SlotConfirm::None) {
        const int slot = slot_of_card(g_slot_sel);
        const std::string q = (g_slot_confirm == SlotConfirm::Overwrite ? "Overwrite Slot " : "Delete Slot ") + std::to_string(slot) +
                              "?   A: Yes   B: No";
        draw_text_centered((float)logical_w * 0.5f, y, 24.0f, kAccentR, kAccentG, kAccentB, 1.0f, q, true);
    }

    const SwitchButtonIcon choose = portrait ? SwitchButtonIcon::DpadDown : SwitchButtonIcon::DpadLeft;
    if (g_slot_screen == SlotScreen::Save) {
        draw_hints(logical_h, {{choose, "Choose"}, {SwitchButtonIcon::A, "Save here"}, {SwitchButtonIcon::B, "Back"}});
    } else {
        draw_hints(logical_h, {{choose, "Choose"}, {SwitchButtonIcon::A, "Load"}, {SwitchButtonIcon::Y, "Delete slot"},
                               {SwitchButtonIcon::B, "Back"}});
    }
}

// Pause menu > Connect to Another Game: picks the second device
// (core/link_session.h) - the compatible games, the loaded one first.
void render_link_picker(int logical_w, int logical_h) {
    switch_ui_draw_text((float)logical_w - 24.0f - switch_ui_text_width("Connect to Another Game", 20.0f), 20.0f, 20.0f,
                        kTextR, kTextG, kTextB, 0.85f, "Connect to Another Game");
    const float x = 60.0f;
    float y = (float)kTopBarH + 24.0f;
    switch_ui_draw_text(x, y, 28.0f, kTextR, kTextG, kTextB, 1.0f, "Which device does it connect to?", true);
    y += 40.0f;
    switch_ui_draw_text(x, y, 19.0f, kTextR, kTextG, kTextB, 0.7f,
                        "Only the compatible ones: Digimon V1-V4 together, Tamagotchi V3 US/EU together.");
    y += 46.0f;
    const int n = (int)g_link_cands.size();
    const int first = std::max(0, std::min(g_link_sel - 3, n - 7));
    for (int i = first; i < n && i < first + 7; ++i) {
        const LinkCandidate& c = g_link_cands[i];
        const bool sel = i == g_link_sel;
        menu_touch_item(x - 16.0f, y - 8.0f, (float)logical_w - 2.0f * x + 32.0f, 56.0f, sel, [i] { g_link_sel = i; }, HidNpadButton_A);
        if (sel) {
            switch_ui_draw_rect(x - 16.0f, y - 8.0f, (float)logical_w - 2.0f * x + 32.0f, 46.0f, kSelR, kSelG, kSelB, 1.0f);
            switch_ui_draw_rect(x - 16.0f, y - 8.0f, 4.0f, 46.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        }
        switch_ui_draw_text(x, y, 24.0f, kTextR, kTextG, kTextB, 1.0f, c.name, sel);
        const std::string note = switch_tr(c.same_game ? "a copy of this game" : c.has_save ? "its auto save" : "a new game");
        switch_ui_draw_text((float)logical_w - x - switch_ui_text_width(note, 19.0f), y + 3.0f, 19.0f, kTextR, kTextG, kTextB, 0.6f, note);
        y += 56.0f;
    }
    switch_ui_draw_text(x, (float)logical_h - 110.0f, 19.0f, kTextR, kTextG, kTextB, 0.7f,
                        "Player 1 (left half): Left / Down / Right.  Player 2 (right half): Y / X / A.");
    draw_hints(logical_h, {{SwitchButtonIcon::DpadUp, "Choose"}, {SwitchButtonIcon::A, "Connect"}, {SwitchButtonIcon::B, "Back"}});
}

void update_link_picker(uint64_t kDown) {
    const int n = (int)g_link_cands.size();
    if (n == 0 || (kDown & HidNpadButton_B)) {
        g_link_picker = false;
        return;
    }
    if (kDown & HidNpadButton_Down) g_link_sel = (g_link_sel + 1) % n;
    if (kDown & HidNpadButton_Up) g_link_sel = (g_link_sel + n - 1) % n;
    if (kDown & HidNpadButton_A) {
        if (link_session_start(g_link_cands[g_link_sel])) {
            // Straight into the two-device screen.
            g_link_picker = false;
            g_settings_active = false;
            g_settings_from_gameplay = false;
            g_menu_active = false;
        }
    }
}

// Pause menu > Gotchi Figure: which figure stands on the Tama-Go and which
// of its two base pins touch (CD1 / CD2) - as the user's tamago port's
// figures menu. Up / Down choose, Left / Right change the contacts, A puts
// the figure on (or takes it off: "None"), B goes back. The game itself is
// paused meanwhile; switch_screen.cpp applies the choice when it runs.
const char* const kTmgFigureNames[13] = {"None", "Mametchi", "Memetchi", "Kuchipatchi", "Chamametchi", "Violetchi", "Ringotchi",
                                         "Kuromametchi", "Sebiretchi", "Ponytchi", "Ichigotchi", "Shimashimatchi", "Makiko"};
GLuint g_fig_icon_tex = 0;

void ensure_fig_icon_tex() {
    if (g_fig_icon_tex) return;
    glGenTextures(1, &g_fig_icon_tex);
    glBindTexture(GL_TEXTURE_2D, g_fig_icon_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, kTamagoFigIcon);
}

void draw_disc(float cx, float cy, float rad, float r, float g, float b, float a) {
    float xy[2 * 16];
    for (int i = 0; i < 16; i++) {
        xy[2 * i] = cx + rad * cosf((float)i * 6.2831853f / 16.0f);
        xy[2 * i + 1] = cy + rad * sinf((float)i * 6.2831853f / 16.0f);
    }
    switch_ui_draw_polygon(xy, 16, r, g, b, a);
}

const char* contacts_label(uint8_t c) { return c == 1 ? "CD1" : c == 2 ? "CD2" : c == 3 ? "CD1+CD2" : "-"; }

// The two pins of a figure's base, lit = touching, and their name (176 x 32).
void draw_contacts_pill(float px, float py, uint8_t c) {
    const float pw = 176.0f, ph = 32.0f;
    switch_ui_draw_rect(px, py, pw, ph, 0.09f, 0.09f, 0.09f, 1.0f);
    for (int k = 0; k < 2; k++) {
        const bool on = (c >> k) & 1;
        draw_disc(px + 20.0f + 20.0f * (float)k, py + ph * 0.5f, 7.0f, on ? kHighlightR : 0.32f, on ? kHighlightG : 0.32f,
                  on ? kHighlightB : 0.32f, 1.0f);
    }
    switch_ui_draw_text(px + 62.0f, py + 6.0f, 18.0f, kTextR, kTextG, kTextB, 1.0f, contacts_label(c), true);
}

void render_fig_picker(int logical_w, int logical_h) {
    ensure_fig_icon_tex();
    switch_ui_draw_text((float)logical_w - 24.0f - switch_ui_text_width("Gotchi Figure", 20.0f), 20.0f, 20.0f, kTextR, kTextG, kTextB, 0.85f,
                        "Gotchi Figure");
    const float x = 60.0f;
    float y = (float)kTopBarH + 20.0f;
    switch_ui_draw_text(x, y, 28.0f, kTextR, kTextG, kTextB, 1.0f, "Gotchi Figure", true);
    y += 38.0f;
    std::string now = switch_tr("On:") + std::string(" ");
    if (g_tmg_figure) now += switch_tr(kTmgFigureNames[g_tmg_figure]) + std::string(" (") + contacts_label(tmg_contacts_of(g_tmg_figure)) + ")";
    else now += switch_tr("None");
    switch_ui_draw_text(x, y, 19.0f, kAccentR, kAccentG, kAccentB, 1.0f, now, true);
    y += 40.0f;
    // The list as shown: None, then the figures under their wave's heading
    // (-1 = "Wave 1", -2 = "Wave 2"); the cursor only lands on figures.
    static const int kList[15] = {0, -1, 1, 2, 3, 4, 5, 6, -2, 7, 8, 9, 10, 11, 12};
    int sel_pos = 0;
    for (int p = 0; p < 15; p++)
        if (kList[p] == g_fig_sel) sel_pos = p;
    const float rowh = 52.0f, headh = 40.0f, w = (float)logical_w - 2.0f * x;
    const int vis = std::max(3, (int)(((float)logical_h - y - 80.0f) / rowh));
    const int first = std::max(0, std::min(sel_pos - vis / 2, 15 - vis));
    for (int p = first; p < 15 && p < first + vis; ++p) {
        const int i = kList[p];
        if (i < 0) {
            const char* head = i == -1 ? "Wave 1" : "Wave 2";
            switch_ui_draw_text(x, y + 2.0f, 20.0f, kAccentR, kAccentG, kAccentB, 1.0f, head, true);
            const float hx = x + switch_ui_text_width(head, 20.0f, true) + 14.0f;
            switch_ui_draw_rect(hx, y + 13.0f, x + w - hx, 2.0f, kTextR, kTextG, kTextB, 0.3f);
            y += headh;
            continue;
        }
        const bool sel = i == g_fig_sel;
        menu_touch_item(x - 16.0f, y - 6.0f, w + 32.0f, rowh, sel, [i] { g_fig_sel = i; }, HidNpadButton_A);
        switch_ui_draw_rect(x - 16.0f, y - 6.0f, w + 32.0f, rowh - 6.0f, sel ? kSelR : kTileR, sel ? kSelG : kTileG, sel ? kSelB : kTileB, 1.0f);
        if (sel) switch_ui_draw_rect(x - 16.0f, y - 6.0f, 4.0f, rowh - 6.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        if (i == 0) {
            // "None": a crossed circle.
            draw_disc(x + 16.0f, y + 17.0f, 13.0f, kTextR, kTextG, kTextB, 0.8f);
            draw_disc(x + 16.0f, y + 17.0f, 10.0f, sel ? kSelR : kTileR, sel ? kSelG : kTileG, sel ? kSelB : kTileB, 1.0f);
            const float d[8] = {x + 7.0f, y + 24.0f, x + 9.0f, y + 26.0f, x + 25.0f, y + 10.0f, x + 23.0f, y + 8.0f};
            switch_ui_draw_polygon(d, 4, kTextR, kTextG, kTextB, 0.8f);
        } else {
            switch_ui_draw_texture_plain(g_fig_icon_tex, x, y - 2.0f, 36.0f, 36.0f);
        }
        const std::string name = switch_tr(kTmgFigureNames[i]);
        switch_ui_draw_text(x + 52.0f, y + 2.0f, 24.0f, kTextR, kTextG, kTextB, 1.0f, name, sel);
        if (i && i == g_tmg_figure) {
            const float nx = x + 52.0f + switch_ui_text_width(name, 24.0f, sel) + 14.0f;
            draw_disc(nx + 5.0f, y + 17.0f, 5.0f, kAccentR, kAccentG, kAccentB, 1.0f);
            switch_ui_draw_text(nx + 16.0f, y + 6.0f, 17.0f, kAccentR, kAccentG, kAccentB, 1.0f, switch_tr("on"), true);
        }
        if (i) {
            const float pw = 176.0f, px = x + w - pw - 30.0f, py = y + 1.0f;
            draw_contacts_pill(px, py, tmg_contacts_of(i));
            if (sel) {
                switch_ui_draw_button_icon(px - 30.0f, py + 4.0f, 22.0f, kAccentR, kAccentG, kAccentB, 1.0f, SwitchButtonIcon::DpadLeft);
                switch_ui_draw_button_icon(px + pw + 6.0f, py + 4.0f, 22.0f, kAccentR, kAccentG, kAccentB, 1.0f, SwitchButtonIcon::DpadRight);
            }
        }
        y += rowh;
    }
    draw_hints(logical_h, {{SwitchButtonIcon::DpadUp, "Choose"}, {SwitchButtonIcon::DpadLeft, "Contacts"},
                           {SwitchButtonIcon::A, "Put on"}, {SwitchButtonIcon::B, "Back"}});
}

void update_fig_picker(uint64_t kDown) {
    const int n = 13;
    if (kDown & HidNpadButton_B) {
        g_fig_picker = false;
        switch_settings_save();
        return;
    }
    if (kDown & HidNpadButton_Down) g_fig_sel = (g_fig_sel + 1) % n;
    if (kDown & HidNpadButton_Up) g_fig_sel = (g_fig_sel + n - 1) % n;
    if (g_fig_sel > 0 && (kDown & (HidNpadButton_Left | HidNpadButton_Right))) {
        // CD1+CD2 -> CD1 -> CD2 -> CD1+CD2 (right), the other way round (left).
        static const uint8_t kOrder[3] = {3, 1, 2};
        const uint8_t c = tmg_contacts_of(g_fig_sel);
        int k = c == 1 ? 1 : c == 2 ? 2 : 0;
        k = (k + ((kDown & HidNpadButton_Right) ? 1 : 2)) % 3;
        const int sh = 2 * (g_fig_sel - 1);
        g_tmg_contacts = (g_tmg_contacts & ~(3u << sh)) | ((uint32_t)kOrder[k] << sh);
    }
    if (kDown & HidNpadButton_A) {
        g_tmg_figure = (uint8_t)g_fig_sel;
        g_fig_picker = false;
        switch_settings_save();
    }
}

void render_simulate_screen(int logical_w, int logical_h) {
    switch_ui_draw_text((float)logical_w - 24.0f - switch_ui_text_width("Simulate Time", 20.0f), 20.0f, 20.0f, kTextR, kTextG, kTextB, 0.85f,
                        "Simulate Time");
    const float x = 60.0f;
    float y = (float)kTopBarH + 24.0f;
    switch_ui_draw_text(x, y, 28.0f, kTextR, kTextG, kTextB, 1.0f, "Simulate Time", true);
    y += 40.0f;
    switch_ui_draw_text(x, y, 19.0f, kTextR, kTextG, kTextB, 0.7f, "Your pet lives this long at high speed. B stops it early.");
    y += 46.0f;
    // As many rows as fit, scrolling with the cursor.
    const int vis = std::max(3, (int)(((float)logical_h - y - 70.0f) / 56.0f));
    const int first = std::max(0, std::min(g_slot_sel - vis / 2, kNumSimOptions - vis));
    for (int i = first; i < kNumSimOptions && i < first + vis; ++i) {
        const bool sel = i == g_slot_sel;
        menu_touch_item(x - 16.0f, y - 8.0f, 460.0f, 56.0f, sel, [i] { g_slot_sel = i; }, HidNpadButton_A);
        if (sel) {
            switch_ui_draw_rect(x - 16.0f, y - 8.0f, 460.0f, 46.0f, kSelR, kSelG, kSelB, 1.0f);
            switch_ui_draw_rect(x - 16.0f, y - 8.0f, 4.0f, 46.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        }
        switch_ui_draw_text(x, y, 24.0f, kTextR, kTextG, kTextB, 1.0f, kSimLabels[i], sel);
        y += 56.0f;
    }
    // What happens afterwards, for the current Pet Time mode.
    const float px = std::max(560.0f, (float)logical_w - 430.0f), py = (float)kTopBarH + 110.0f;
    switch_ui_draw_rect(px, py, 380.0f, 250.0f, kPanelR, kPanelG, kPanelB, 1.0f);
    const bool keeps = !g_experimental_features_enabled || g_pet_time_mode == PetTimeMode::Paused;
    const char* lines[] = {
        "Before it starts:", "- a copy is kept for Undo Simulate", "", "Afterwards:",
        keeps ? "- the clock stays that far ahead" : "- the clock goes back to the real time",
        keeps ? "  (Pet Time: Paused)" : "  (the pet keeps the time it lived)",
    };
    float ly = py + 18.0f;
    for (int i = 0; i < 6; ++i) {
        switch_ui_draw_text(px + 18.0f, ly, 19.0f, kTextR, kTextG, kTextB, (i == 0 || i == 3) ? 1.0f : 0.7f, lines[i], i == 0 || i == 3);
        ly += 32.0f;
    }
    draw_hints(logical_h, {{SwitchButtonIcon::DpadUp, "Choose"}, {SwitchButtonIcon::A, "Simulate"}, {SwitchButtonIcon::B, "Back"}});
}

void render_pet_prompt(int logical_w, int logical_h) {
    const float w = 700.0f, h = 280.0f, x = ((float)logical_w - w) * 0.5f, y = ((float)logical_h - h) * 0.5f;
    switch_ui_draw_rect(x, y, w, h, kPanelR, kPanelG, kPanelB, 1.0f);
    draw_outline(x, y, w, h, 2.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
    const int64_t hours = g_pet_prompt_age / 3600;
    const std::string age = hours >= 48 ? (std::to_string(hours / 24) + " days") : (std::to_string(hours) + " hours");
    switch_ui_draw_text(x + 30.0f, y + 26.0f, 26.0f, kTextR, kTextG, kTextB, 1.0f, "This save is " + age + " old", true);
    switch_ui_draw_text(x + 30.0f, y + 70.0f, 20.0f, kTextR, kTextG, kTextB, 0.75f, "Should your pet live that time now?");
    const char* opts[2] = {"Catch up", "Load as is"};
    for (int i = 0; i < 2; ++i) {
        const float bx = x + 30.0f + i * 330.0f, by = y + 130.0f, bw = 310.0f, bh = 52.0f;
        const bool sel = g_pet_prompt_sel == i;
        menu_touch_item(bx, by, bw, bh, sel, [i] { g_pet_prompt_sel = i; }, HidNpadButton_A);
        if (sel) switch_ui_draw_rect(bx, by, bw, bh, kSelR, kSelG, kSelB, 1.0f);
        draw_outline(bx, by, bw, bh, 2.0f, sel ? kHighlightR : kTextR, sel ? kHighlightG : kTextG, sel ? kHighlightB : kTextB, sel ? 1.0f : 0.35f);
        draw_text_centered(bx + bw * 0.5f, by + 12.0f, 24.0f, kTextR, kTextG, kTextB, 1.0f, opts[i], sel);
    }
    switch_ui_draw_text(x + 30.0f, y + 210.0f, 17.0f, kTextR, kTextG, kTextB, 0.6f,
                        "Catch up: it lives those hours (up to your cap). Load as is: as saved, clock set to now.");
    draw_hints(logical_h, {{SwitchButtonIcon::DpadLeft, "Choose"}, {SwitchButtonIcon::A, "OK"}});
}

}  // namespace

static void update_load_prompt(uint64_t kDown) {
    if (kDown & HidNpadButton_A) {
        YOKOI_LOG("switch_menu: load-save-state prompt confirmed for index %u", (unsigned)g_selected_index);
        note_recent(g_selected_index);
        switch_load_game_by_index_and_init(g_selected_index);
        if (load_game_state(g_cpu.get(), g_selected_index)) {
            pet_time_on_state_loaded();  // time away since that save - see pet_time.h
        }
        g_load_prompt_active = false;
        g_menu_active = false;
    } else if (kDown & HidNpadButton_B) {
        YOKOI_LOG("switch_menu: load-save-state prompt declined for index %u, starting fresh", (unsigned)g_selected_index);
        note_recent(g_selected_index);
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

// X on a settings row: back to its factory value (the ones
// switch_settings_load() starts from). Returns false when there's nothing
// to reset (an action row, or a value that isn't a setting).
static bool reset_row_to_default(SettingsRow row) {
    const AppSettings defaults;
    switch (row) {
        case SettingsRow::Rotation:          g_screen_rotation = ScreenRotation::Deg0; break;
        case SettingsRow::MenuRotation:      g_menu_rotation = ScreenRotation::Deg0; break;
        case SettingsRow::ScreenSize:        g_screen_size = 2; break;
        case SettingsRow::BackgroundArt:     g_hide_background_art = false; break;
        case SettingsRow::ConsoleArt:
            g_hide_console_art = true;
            rebuild_layout_from_game();  // the case strip changes the canvas size
            break;
        case SettingsRow::PanelGap:
            g_panel_gap_percent = 0;
            g_panelgap_custom_selected = false;
            rebuild_layout_from_game();
            break;
        case SettingsRow::ScreenPalette:     g_screen_palette = 0; break;
        case SettingsRow::Background:
            g_settings.background_color = defaults.background_color;
            g_bg_custom_selected = false;
            break;
        case SettingsRow::SegmentColor:
            g_segment_color = kSegmentColorAuto;
            g_segcolor_custom_selected = false;
            break;
        case SettingsRow::Segments:
            g_settings.segment_marking_alpha = defaults.segment_marking_alpha;
            g_seg_custom_selected = false;
            break;
        case SettingsRow::Ghosting:          g_ghosting = 0; break;
        case SettingsRow::FlickerFilter:     g_flicker_filter = 0; break;
        case SettingsRow::PixelStyle:        g_pixel_style = 0; break;
        case SettingsRow::ScreenFilter:      g_screen_filter = 0; break;
        case SettingsRow::FilterStrength:    g_filter_strength = 1; break;
        case SettingsRow::MenuTheme:
            g_menu_theme = 0;
            apply_menu_theme();
            break;
        case SettingsRow::MenuFont:
            g_menu_font = 0;
            switch_ui_set_font(0);
            break;
        case SettingsRow::MenuStyle:
            g_menu_style = MenuStyle::Makers;
            g_view = MenuView::Makers;
            break;
        case SettingsRow::Transition:        g_transition = 1; break;
        case SettingsRow::MakerOrder:
            g_maker_order = MakerOrder::AZ;
            rebuild_tiles();
            break;
        case SettingsRow::CloneFilter:
            g_clone_filter_mode = CloneFilterMode::ShowAll;
            rebuild_sorted_order();
            break;
        case SettingsRow::ExperimentalGames: g_experimental_games_enabled = false; break;
        case SettingsRow::ExperimentalFeatures: g_experimental_features_enabled = false; break;
        case SettingsRow::Volume:            g_sound_volume = 100; break;
        case SettingsRow::Mute:              g_sound_mute = false; break;
        case SettingsRow::SoundStyle:        g_sound_style = 0; break;
        case SettingsRow::GameComSound:      g_tgc_sound = 0; break;
        case SettingsRow::GameComSpeed:      g_tgc_speed = 0; break;
        case SettingsRow::GameComNoise:      g_tgc_noise = 0; break;
        case SettingsRow::GameComPalette:    g_tgc_palette = 0; break;
        case SettingsRow::GameComGhost:      g_tgc_ghost = 0; break;
        case SettingsRow::RZoneStyle:        g_rz_style = 0; break;
        case SettingsRow::AdvVisionScreen:   g_eav_screen = 0; break;
        case SettingsRow::TamaGoFigure:      g_tmg_figure = 0; break;
        case SettingsRow::GameMasterSpeed:   g_hgm_speed = 0; break;
        case SettingsRow::MenuSounds:        g_menu_sounds = true; break;
        case SettingsRow::MenuMusic:         g_menu_music = 0; break;
        case SettingsRow::MenuMusicTrack:    g_menu_music_track.clear(); break;
        case SettingsRow::MenuMusicVolume:   g_menu_music_volume = 60; break;
        case SettingsRow::MenuMusicDuck:     g_menu_music_duck = 2; break;
        case SettingsRow::PauseCombo:        g_pause_combo = 0; break;
        case SettingsRow::OnScreenKeypad:    g_onscreen_keypad = true; break;
        case SettingsRow::VibrationGame:     g_vibration_game = 0; break;
        case SettingsRow::VibrationPets:     g_vibration_pets = false; break;
        case SettingsRow::VibrationMenu:     g_vibration_menu = false; break;
        case SettingsRow::VibrationStrength: g_vibration_strength = 1; break;
        case SettingsRow::PetAlertLight:     g_pet_alert_light = true; break;
        case SettingsRow::MotionPedometer:   g_motion_pedometer = true; break;
        case SettingsRow::PedometerSensitivity: g_pedometer_sensitivity = 1; break;
        case SettingsRow::MotionTapSensor:   g_motion_tap_sensor = true; break;
        case SettingsRow::FastForward:       g_fastforward_button = 0; break;
        case SettingsRow::FastForwardSpeed:  g_fastforward_speed = 1; break;
        case SettingsRow::ButtonResponse:    g_button_response = 2; break;
        case SettingsRow::LinkSound:         link_session_set_sound_player(0); break;
        case SettingsRow::LinkSoundButton:   g_link_sound_button = 1; break;
        case SettingsRow::Rewind:            g_rewind_button = 0; break;
        case SettingsRow::RewindLength:      g_rewind_length = 2; break;
        case SettingsRow::TapSensitivity:    g_tap_sensitivity = 1; break;
        case SettingsRow::IrSensor:          g_ir_sensor = 1; break;
        case SettingsRow::IrSensitivity:     g_ir_sensitivity = 1; break;
        case SettingsRow::LoadStateBehavior: g_load_state_behavior = LoadStateBehavior::Ask; break;
        case SettingsRow::TimeSetMode:       g_time_set_mode = TimeSetMode::Automatic; break;
        case SettingsRow::PetTime:           g_pet_time_mode = PetTimeMode::RealTime; break;
        case SettingsRow::PetTimeCap:
            g_pet_time_cap_hours = 72;
            g_pettimecap_custom_selected = false;
            break;
        case SettingsRow::PetTimeDisplay:    g_pet_time_display = PetTimeDisplay::Progress; break;
        case SettingsRow::LogFile:
            g_log_file_enabled = false;
            debug_log::set_enabled(g_log_file_enabled || g_debug_mode);
            break;
        case SettingsRow::SplashScreen:      g_splash_screen = true; break;
        case SettingsRow::MakerColors:       g_maker_colors = true; break;
        case SettingsRow::MenuTouch:         g_menu_touch = true; break;
        case SettingsRow::DateFormat:        g_date_format = 0; break;
        case SettingsRow::Records:           g_records_enabled = true; break;
        case SettingsRow::RecordHud:         g_record_hud = 1; break;
        case SettingsRow::AchNotify:         g_ach_notify = 0; break;
        case SettingsRow::AlertColor:        g_alert_color = 0; break;
        case SettingsRow::RecordsList:       g_records_in_list = true; break;
        case SettingsRow::Language:          g_language = 0; break;
        case SettingsRow::DosimeterReading:  g_dosimeter_reading = 0; break;
        default:
            return false;
    }
    return true;
}

// ---- Pause menu > Controls (core/game_controls.h, input/control_remap.h) ----
// Joy-Con drawing on the left, the game's functions and their buttons on
// the right. A on a row waits for the new button (pressing the one it
// already has keeps it); X puts that row back, Y the whole game.

bool g_ctl_waiting = false;
// Controls: when the wait for a new button began - no press within 3 s
// leaves the function unassigned (control_remap_unassign()).
uint64_t g_ctl_wait_tick = 0;
constexpr double kCtlWaitSeconds = 3.0;
void start_ctl_wait() {
    g_ctl_waiting = true;
    g_ctl_wait_tick = armGetSystemTick();
}
double ctl_wait_elapsed() { return armTicksToNs(armGetSystemTick() - g_ctl_wait_tick) / 1e9; }
// "Press a button... 3" - the seconds left before it's left unassigned.
std::string ctl_wait_text() {
    const int left = std::max(1, (int)(kCtlWaitSeconds - ctl_wait_elapsed() + 0.999));
    return switch_tr("Press a button...") + " " + std::to_string(left);
}
bool g_ctl_seen = false;   // control_names.txt re-read on entering the section

bool controls_section_open() {
    return g_settings_active && g_settings_from_gameplay && current_pause_section() == PauseSection::Controls;
}

std::string controls_ref() {
    if (link_session_active()) return kLinkControlsRef;  // the two linked players' buttons
    return g_game ? g_game->ref : std::string();
}

void ui_circle(float cx, float cy, float r, float red, float green, float blue, float a) {
    float xy[2 * 32];
    for (int i = 0; i < 32; i++) {
        const float t = 6.2831853f * (float)i / 32.0f;
        xy[2 * i] = cx + std::cos(t) * r;
        xy[2 * i + 1] = cy + std::sin(t) * r;
    }
    switch_ui_draw_polygon(xy, 32, red, green, blue, a);
}

void ui_rrect(float x, float y, float w, float h, float rad, float red, float green, float blue, float a) {
    rad = std::max(0.0f, std::min(rad, std::min(w, h) * 0.5f));
    float xy[2 * 36];
    int n = 0;
    const float cx[4] = {x + w - rad, x + rad, x + rad, x + w - rad};
    const float cy[4] = {y + h - rad, y + h - rad, y + rad, y + rad};
    for (int c = 0; c < 4; c++) {
        for (int i = 0; i < 9; i++) {
            const float t = 1.5707963f * (float)(c + i / 8.0f);
            xy[2 * n] = cx[c] + std::cos(t) * rad;
            xy[2 * n + 1] = cy[c] + std::sin(t) * rad;
            n++;
        }
    }
    switch_ui_draw_polygon(xy, n, red, green, blue, a);
}

void ui_text_centered(float cx, float cy, float size, float red, float green, float blue, const char* s) {
    const float w = switch_ui_text_width(s, size, true);
    switch_ui_draw_text(cx - w * 0.5f, cy - size * 0.62f, size, red, green, blue, 1.0f, s, true);
}

enum class CtlCap { Off, Used, Selected, Reserved };

void cap_colors(CtlCap st, float& r, float& g, float& b, float& tr, float& tg, float& tb) {
    switch (st) {
        case CtlCap::Reserved: r = g = b = 0.27f; tr = tg = tb = 0.5f; break;
        case CtlCap::Selected: r = kHighlightR; g = kHighlightG; b = kHighlightB; tr = tg = tb = 0.06f; break;
        case CtlCap::Used:     r = g = b = 0.96f; tr = tg = tb = 0.1f; break;
        default:               r = 0.15f; g = 0.15f; b = 0.16f; tr = tg = tb = 0.7f; break;
    }
}

// The two Joy-Con, 360 x 420 at scale 1, top-left at (ox, oy). `state`
// holds each physical button's look (indexed by RemapButton).
void draw_joycons(float ox, float oy, const CtlCap state[kNumRemapButtons]) {
    auto X = [&](float v) { return ox + v; };
    auto Y = [&](float v) { return oy + v; };
    float r, g, b, tr, tg, tb;

    auto cap = [&](float cx, float cy, float rad, int btn, const char* label) {
        cap_colors(state[btn], r, g, b, tr, tg, tb);
        if (state[btn] == CtlCap::Selected) ui_circle(X(cx), Y(cy), rad + 5.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        ui_circle(X(cx), Y(cy) + 2.0f, rad, 0.06f, 0.06f, 0.06f, 1.0f);
        ui_circle(X(cx), Y(cy), rad, r, g, b, 1.0f);
        if (label && *label) ui_text_centered(X(cx), Y(cy), rad * 0.95f, tr, tg, tb, label);
    };
    auto arrow = [&](float cx, float cy, int btn, int dir) {  // 0 up 1 down 2 left 3 right
        cap(cx, cy, 17.0f, btn, nullptr);
        cap_colors(state[btn], r, g, b, tr, tg, tb);
        static const float kTri[4][6] = {{0, -6, -7, 5, 7, 5}, {0, 6, -7, -5, 7, -5}, {-6, 0, 5, -7, 5, 7}, {6, 0, -5, -7, -5, 7}};
        float xy[6];
        for (int i = 0; i < 3; i++) { xy[2 * i] = X(cx) + kTri[dir][2 * i]; xy[2 * i + 1] = Y(cy) + kTri[dir][2 * i + 1]; }
        switch_ui_draw_polygon(xy, 3, tr, tg, tb, 1.0f);
    };
    auto pill = [&](float x0, float y0, float x1, float y1, int btn, const char* label, float rad) {
        cap_colors(state[btn], r, g, b, tr, tg, tb);
        if (state[btn] == CtlCap::Selected) ui_rrect(X(x0) - 4, Y(y0) - 4, x1 - x0 + 8, y1 - y0 + 8, rad + 3, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        ui_rrect(X(x0), Y(y0), x1 - x0, y1 - y0, rad, 0.06f, 0.06f, 0.06f, 1.0f);
        ui_rrect(X(x0), Y(y0), x1 - x0, y1 - y0 - 3, rad, r, g, b, 1.0f);
        ui_text_centered(X((x0 + x1) * 0.5f), Y((y0 + y1) * 0.5f - 1), 16.0f, tr, tg, tb, label);
    };
    auto stick = [&](float cx, float cy, int btn, const char* label) {
        cap_colors(state[btn], r, g, b, tr, tg, tb);
        if (state[btn] == CtlCap::Selected) ui_circle(X(cx), Y(cy), 34.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
        ui_circle(X(cx), Y(cy), 30.0f, 0.07f, 0.07f, 0.07f, 1.0f);
        if (state[btn] == CtlCap::Off) { r = g = b = 0.22f; tr = tg = tb = 0.75f; }
        ui_circle(X(cx), Y(cy), 25.0f, r, g, b, 1.0f);
        ui_text_centered(X(cx), Y(cy), 16.0f, tr, tg, tb, label);
    };
    auto body = [&](float x, float y, float w, float h, float cr, float cg, float cb, bool left) {
        ui_rrect(X(x) + 4, Y(y) + 8, w, h, 62.0f, 0.0f, 0.0f, 0.0f, 0.35f);  // shadow
        ui_rrect(X(x), Y(y), w, h, 62.0f, cr, cg, cb, 1.0f);
        // Flat inner side with the rail.
        if (left) ui_rrect(X(x + w - 62), Y(y), 62.0f, h, 6.0f, cr, cg, cb, 1.0f);
        else ui_rrect(X(x), Y(y), 62.0f, h, 6.0f, cr, cg, cb, 1.0f);
        switch_ui_draw_rect(left ? X(x + w - 7) : X(x), Y(y + 6), 7.0f, h - 12.0f, 0.12f, 0.12f, 0.13f, 1.0f);
    };

    const float lx = 14, rx = 192, y = 44, w = 154, h = 340;
    pill(lx + 6, y - 34, lx + 78, y - 2, RB_ZL, "ZL", 14.0f);
    pill(rx + w - 78, y - 34, rx + w - 6, y - 2, RB_ZR, "ZR", 14.0f);
    body(lx, y, w, h, 0.0f, 0.67f, 0.88f, true);
    body(rx, y, w, h, 0.98f, 0.27f, 0.27f, false);
    pill(lx + 14, y + 6, lx + 92, y + 30, RB_L, "L", 10.0f);
    pill(rx + w - 92, y + 6, rx + w - 14, y + 30, RB_R, "R", 10.0f);
    // - and +
    cap_colors(state[RB_Minus], r, g, b, tr, tg, tb);
    if (state[RB_Minus] == CtlCap::Selected) ui_circle(X(lx + w - 34), Y(y + 56), 17.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
    ui_rrect(X(lx + w - 45), Y(y + 52), 22.0f, 8.0f, 3.0f, r, g, b, 1.0f);
    cap_colors(state[RB_Plus], r, g, b, tr, tg, tb);
    if (state[RB_Plus] == CtlCap::Selected) ui_circle(X(rx + 34), Y(y + 56), 17.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
    ui_rrect(X(rx + 23), Y(y + 52), 22.0f, 8.0f, 3.0f, r, g, b, 1.0f);
    ui_rrect(X(rx + 30), Y(y + 45), 8.0f, 22.0f, 3.0f, r, g, b, 1.0f);
    // Left stick + the four direction buttons
    const float lcx = lx + w / 2 - 4;
    stick(lcx, y + 112, RB_L3, "L3");
    arrow(lcx, y + 222 - 36, RB_Up, 0);
    arrow(lcx, y + 222 + 36, RB_Down, 1);
    arrow(lcx - 36, y + 222, RB_Left, 2);
    arrow(lcx + 36, y + 222, RB_Right, 3);
    ui_rrect(X(lx + w - 46), Y(y + 286), 18.0f, 18.0f, 3.0f, 0.1f, 0.1f, 0.1f, 1.0f);  // Capture
    // A/B/X/Y, right stick, Home
    const float fx = rx + w / 2 + 4, fy = y + 120;
    cap(fx, fy - 36, 17.0f, RB_X, "X");
    cap(fx, fy + 36, 17.0f, RB_B, "B");
    cap(fx - 36, fy, 17.0f, RB_Y, "Y");
    cap(fx + 36, fy, 17.0f, RB_A, "A");
    stick(rx + w / 2 + 4, y + 232, RB_R3, "R3");
    ui_circle(X(rx + 37), Y(y + 295), 11.0f, 0.1f, 0.1f, 0.1f, 1.0f);  // Home
    ui_circle(X(rx + 37), Y(y + 295), 6.0f, 0.35f, 0.35f, 0.35f, 1.0f);
    ui_circle(X(rx + 37), Y(y + 295), 4.0f, 0.1f, 0.1f, 0.1f, 1.0f);
}

bool button_icon_for(int b, SwitchButtonIcon& icon) {
    switch (b) {
        case RB_Up: icon = SwitchButtonIcon::DpadUp; return true;
        case RB_Down: icon = SwitchButtonIcon::DpadDown; return true;
        case RB_Left: icon = SwitchButtonIcon::DpadLeft; return true;
        case RB_Right: icon = SwitchButtonIcon::DpadRight; return true;
        case RB_A: icon = SwitchButtonIcon::A; return true;
        case RB_B: icon = SwitchButtonIcon::B; return true;
        case RB_X: icon = SwitchButtonIcon::X; return true;
        case RB_Y: icon = SwitchButtonIcon::Y; return true;
        case RB_L: icon = SwitchButtonIcon::L; return true;
        case RB_R: icon = SwitchButtonIcon::R; return true;
        case RB_ZL: icon = SwitchButtonIcon::ZL; return true;
        case RB_ZR: icon = SwitchButtonIcon::ZR; return true;
        case RB_Plus: icon = SwitchButtonIcon::Plus; return true;
        case RB_Minus: icon = SwitchButtonIcon::Minus; return true;
        default: return false;  // L3 / R3: text
    }
}

// The physical buttons of a row, right-aligned at `right`.
void draw_row_buttons(float right, float y, float size, const std::vector<int>& phys_in, float alpha) {
    // Unassigned buttons (kNoButton) aren't drawn; none left: "Unassigned".
    std::vector<int> phys;
    for (int p : phys_in)
        if (p >= 0 && p < kNumRemapButtons) phys.push_back(p);
    if (phys.empty()) {
        const std::string t = switch_tr("Unassigned");
        switch_ui_draw_text(right - switch_ui_text_width(t, size), y, size, kTextR, kTextG, kTextB, 0.5f * alpha, t);
        return;
    }
    const float sep = switch_ui_text_width("/", size);
    float total = 0.0f;
    for (size_t i = 0; i < phys.size(); i++) {
        SwitchButtonIcon ic;
        total += button_icon_for(phys[i], ic) ? switch_ui_button_icon_width(size, ic) : switch_ui_text_width(remap_button_label(phys[i]), size, true);
        if (i + 1 < phys.size()) total += sep + 12.0f;
    }
    float x = right - total;
    for (size_t i = 0; i < phys.size(); i++) {
        SwitchButtonIcon ic;
        if (button_icon_for(phys[i], ic)) {
            switch_ui_draw_button_icon(x, y, size, kHighlightR, kHighlightG, kHighlightB, alpha, ic);
            x += switch_ui_button_icon_width(size, ic);
        } else {
            const char* t = remap_button_label(phys[i]);
            switch_ui_draw_text(x, y, size, kHighlightR, kHighlightG, kHighlightB, alpha, t, true);
            x += switch_ui_text_width(t, size, true);
        }
        if (i + 1 < phys.size()) {
            switch_ui_draw_text(x + 6.0f, y, size, kTextR, kTextG, kTextB, 0.5f, "/");
            x += sep + 12.0f;
        }
    }
}

const char* pause_combo_note() {
    switch (g_pause_combo) {
        case 1:  return "L3 + R3 open the pause menu (Settings > Menu)";
        case 2:  return "L3 opens the pause menu (Settings > Menu)";
        case 3:  return "R3 opens the pause menu (Settings > Menu)";
        default: return "ZL + ZR open the pause menu (Settings > Menu)";
    }
}

// Pause menu > Controls with two devices linked (core/link_session.h): one
// column per player - its buttons, then its device's special functions -
// and, below, which player a shake of the controller counts steps for.
// Rows: player 1's (0..n1-1), player 2's, then the shake row if there is one.
// Rows of the linked Controls screen: each player's, then the shake row (if
// any), then the Sound Switch Button row - always the last one.
int link_ctl_count() {
    return link_control_player_rows(0) + link_control_player_rows(1) + (link_motion_choice() ? 1 : 0) + 1;
}

// Sound Switch Button (switch_settings.h): Off, ZL, ZR, L3, R3 - never the
// fast-forward / rewind buttons.
void cycle_link_sound_button(int dir) {
    int c = g_link_sound_button;
    for (int k = 0; k < kNumHotkeyChoices; k++) {
        c = (c + dir + kNumHotkeyChoices) % kNumHotkeyChoices;
        if (c == 0 || (c != g_fastforward_button && c != g_rewind_button)) break;
    }
    g_link_sound_button = (uint8_t)c;
    switch_settings_save();
}

void render_link_controls_screen(int logical_w, int logical_h) {
    const std::vector<ControlRow> rows = game_control_rows();
    const std::string ref = controls_ref();
    const int n1 = link_control_player_rows(0), n2 = link_control_player_rows(1);
    const int total = link_ctl_count();
    if (g_settings_selected_row >= total) g_settings_selected_row = std::max(0, total - 1);
    const float x0 = settings_sidebar_w(logical_w) + 12.0f;
    // Portrait: the two players one above the other, each the full width.
    const bool stacked = logical_w < logical_h;
    const float gap = 14.0f;
    const float colw = stacked ? (float)logical_w - 24.0f - x0 : ((float)logical_w - 24.0f - x0 - gap) * 0.5f;
    const float fullw = stacked ? colw : colw * 2.0f + gap;
    const float row_h = 40.0f, size = 21.0f, head_h = 46.0f, sec_h = 26.0f;

    float col_bottom = (float)kTopBarH + 12.0f;
    float next_top = col_bottom;
    for (int pl = 0; pl < 2; pl++) {
        const int n = pl ? n2 : n1, base = pl ? n1 : 0;
        int specials = 0;
        for (int i = 0; i < n; i++) specials += link_control_row_special(pl, i) ? 1 : 0;
        const float h = head_h + sec_h * (specials ? 2 : 1) + row_h * n + 10.0f;
        const float cx = stacked ? x0 : x0 + pl * (colw + gap);
        const float top = stacked ? next_top : (float)kTopBarH + 12.0f;
        next_top = top + h + gap;
        col_bottom = std::max(col_bottom, top + h);
        ui_rrect(cx, top, colw, h, 12.0f, 0.17f, 0.17f, 0.17f, 1.0f);
        ui_rrect(cx + 1, top + 1, colw - 2, h - 2, 11.0f, kPanelR * 0.5f, kPanelG * 0.5f, kPanelB * 0.5f, 1.0f);
        // Header: player badge (blue / red, the Joy-Cons' colours), device, sound.
        const float pr = pl ? 1.0f : 0.17f, pg = pl ? 0.35f : 0.70f, pb = pl ? 0.35f : 1.0f;
        ui_rrect(cx + 1, top + 1, colw - 2, head_h - 2, 11.0f, pr * 0.35f, pg * 0.35f, pb * 0.35f, 1.0f);
        const std::string badge = switch_tr(pl ? "Player 2" : "Player 1");
        const float bs = 16.0f, bw = switch_ui_text_width(badge, bs, true) + 18.0f;
        ui_rrect(cx + 12.0f, top + 11.0f, bw, bs + 10.0f, (bs + 10.0f) * 0.5f, pr, pg, pb, 1.0f);
        switch_ui_draw_text(cx + 21.0f, top + 15.0f, bs, 0.02f, 0.02f, 0.05f, 1.0f, badge, true);
        float name_max = colw - bw - 40.0f;
        if (link_session_sound_player() == pl) {
            const std::string snd = switch_tr("Sound");
            const float sw = switch_ui_text_width(snd, 15.0f) + 16.0f;
            ui_rrect(cx + colw - 12.0f - sw, top + 13.0f, sw, 22.0f, 11.0f, 0.25f, 0.25f, 0.25f, 1.0f);
            switch_ui_draw_text(cx + colw - 4.0f - sw, top + 15.0f, 15.0f, kTextR, kTextG, kTextB, 0.9f, snd);
            name_max -= sw + 10.0f;
        }
        draw_text_fit(cx + 22.0f + bw, top + 11.0f, 20.0f, name_max, kTextR, kTextG, kTextB, 1.0f, link_player_name(pl), true);

        float y = top + head_h;
        bool in_special = false;
        switch_ui_draw_text(cx + 16.0f, y + 4.0f, 14.0f, kTextR, kTextG, kTextB, 0.5f, switch_tr("BUTTONS"));
        y += sec_h;
        for (int i = 0; i < n; i++) {
            if (!in_special && link_control_row_special(pl, i)) {
                in_special = true;
                switch_ui_draw_text(cx + 16.0f, y + 4.0f, 14.0f, kTextR, kTextG, kTextB, 0.5f, switch_tr("SPECIAL FUNCTIONS"));
                y += sec_h;
            }
            const int idx = base + i;
            const bool sel = idx == g_settings_selected_row;
            menu_touch_item(cx + 2.0f, y, colw - 4.0f, row_h, sel, [idx] { g_settings_selected_row = idx; }, HidNpadButton_A);
            if (sel) {
                switch_ui_draw_rect(cx + 2.0f, y, colw - 4.0f, row_h - 2.0f, kSelR, kSelG, kSelB, 1.0f);
                switch_ui_draw_rect(cx + 2.0f, y, 4.0f, row_h - 2.0f, kHighlightR, kHighlightG, kHighlightB, 1.0f);
            }
            const float right = cx + colw - 16.0f;
            draw_text_fit(cx + 18.0f, y + 8.0f, size, colw - 150.0f, kTextR, kTextG, kTextB, sel ? 1.0f : 0.85f, rows[idx].name, false);
            if (sel && g_ctl_waiting) {
                const std::string t = ctl_wait_text();
                const float tw = switch_ui_text_width(t, size - 2.0f, true);
                switch_ui_draw_text(right - tw, y + 9.0f, size - 2.0f, 1.0f, 0.77f, 0.31f, 1.0f, t, true);
            } else {
                std::vector<int> phys;
                for (int k : rows[idx].keys) phys.push_back(control_remap_source(ref, k));
                draw_row_buttons(right, y + 8.0f, size, phys, 1.0f);
            }
            y += row_h;
        }
    }

    // The shake row, across both columns.
    if (link_motion_choice()) {
        const float my = col_bottom + 12.0f, mw = fullw;
        const bool sel = g_settings_selected_row == total - 2;
        menu_touch_item(x0, my, mw, 44.0f, sel, [total] { g_settings_selected_row = total - 2; }, HidNpadButton_Right);
        col_bottom = my + 44.0f;
        ui_rrect(x0, my, mw, 44.0f, 12.0f, 0.17f, 0.17f, 0.17f, 1.0f);
        ui_rrect(x0 + 1, my + 1, mw - 2, 42.0f, 11.0f, sel ? kSelR : kPanelR * 0.5f, sel ? kSelG : kPanelG * 0.5f,
                 sel ? kSelB : kPanelB * 0.5f, 1.0f);
        draw_text_fit(x0 + 18.0f, my + 11.0f, size, mw * 0.5f - 24.0f, kTextR, kTextG, kTextB, 1.0f, switch_tr("Shaking the controller counts steps for"), false);
        const int mp = link_motion_player();
        const std::string v = "< " + switch_tr(mp ? "Player 2" : "Player 1") + " (" + link_player_name(mp) + ") >";
        draw_text_fit(x0 + mw - 18.0f - std::min(switch_ui_text_width(v, size), mw * 0.45f), my + 11.0f, size, mw * 0.45f,
                      kHighlightR, kHighlightG, kHighlightB, 1.0f, v, false);
    }

    // The Sound Switch Button row, across both columns: the button that
    // makes you hear the other device (docked there's no touch screen).
    {
        const float my = col_bottom + 12.0f, mw = fullw;
        const bool sel = g_settings_selected_row == total - 1;
        menu_touch_item(x0, my, mw, 44.0f, sel, [total] { g_settings_selected_row = total - 1; }, HidNpadButton_Right);
        ui_rrect(x0, my, mw, 44.0f, 12.0f, 0.17f, 0.17f, 0.17f, 1.0f);
        ui_rrect(x0 + 1, my + 1, mw - 2, 42.0f, 11.0f, sel ? kSelR : kPanelR * 0.5f, sel ? kSelG : kPanelG * 0.5f,
                 sel ? kSelB : kPanelB * 0.5f, 1.0f);
        draw_text_fit(x0 + 18.0f, my + 11.0f, size, mw * 0.6f - 24.0f, kTextR, kTextG, kTextB, 1.0f, switch_tr("Sound Switch Button"), false);
        const std::string v = "< " + switch_tr(time_control_button_label(g_link_sound_button)) + " >";
        const float vw = switch_ui_text_width(v, size);
        switch_ui_draw_text(x0 + mw - 18.0f - vw, my + 11.0f, size, kHighlightR, kHighlightG, kHighlightB, 1.0f, v);
    }

    // Note box.
    const float note_h = 70.0f;
    const float ny = (float)(logical_h - kBottomBarH) - note_h - 4.0f;
    const float nw = fullw;
    ui_rrect(x0, ny, nw, note_h, 10.0f, 0.31f, 0.31f, 0.31f, 1.0f);
    ui_rrect(x0 + 1, ny + 1, nw - 2, note_h - 2, 9.0f, kPanelR, kPanelG, kPanelB, 1.0f);
    // Shrunk (then cut) to the box - in portrait it's only ~480 wide.
    draw_text_fit(x0 + 16.0f, ny + 10.0f, 18.0f, nw - 32.0f, kTextR, kTextG, kTextB, 1.0f,
                  switch_tr("Each player has their buttons and their device's special functions."), false);
    draw_text_fit(x0 + 16.0f, ny + 38.0f, 16.0f, nw - 32.0f, kTextR, kTextG, kTextB, 0.6f,
                  switch_tr("Player 1: left Joy-Con, player 2: right Joy-Con. If the button is already used, the two functions swap"), false);
}

void render_controls_screen(int logical_w, int logical_h) {
    if (link_session_active()) {
        render_link_controls_screen(logical_w, logical_h);
        return;
    }
    const std::vector<ControlRow> rows = game_control_rows();
    const std::string ref = controls_ref();
    const int n = (int)rows.size();
    if (g_settings_selected_row >= n) g_settings_selected_row = std::max(0, n - 1);
    const float x0 = settings_sidebar_w(logical_w);
    const bool wide = logical_w >= 1000;

    if (control_remap_customized(ref)) {
        const char* badge = "Custom for this game";
        const float bs = 18.0f, bw = switch_ui_text_width(badge, bs) + 24.0f;
        const float bx = (float)logical_w - 30.0f - bw, by = (float)kTopBarH + 16.0f;
        ui_rrect(bx, by, bw, bs + 12.0f, (bs + 12.0f) * 0.5f, 1.0f, 0.77f, 0.31f, 1.0f);
        ui_rrect(bx + 2, by + 2, bw - 4, bs + 8.0f, (bs + 8.0f) * 0.5f, 0.27f, 0.21f, 0.08f, 1.0f);
        switch_ui_draw_text(bx + 12.0f, by + 5.0f, bs, 1.0f, 0.77f, 0.31f, 1.0f, badge);
    }

    // Each physical button's look on the drawing.
    CtlCap state[kNumRemapButtons];
    for (auto& s : state) s = CtlCap::Off;
    for (int i = 0; i < n; i++)
        for (int k : rows[i].keys) {
            const int p = control_remap_source(ref, k);
            if (p >= 0 && p < kNumRemapButtons) state[p] = (i == g_settings_selected_row) ? CtlCap::Selected
                                                         : (state[p] == CtlCap::Selected ? CtlCap::Selected : CtlCap::Used);
        }
    const uint64_t pm = pause_combo_mask() | time_control_hotkey_mask();
    for (int b = 0; b < kNumRemapButtons; b++)
        if (remap_button_mask(b) & pm) state[b] = CtlCap::Reserved;

    float list_x = x0 + 36.0f;
    if (wide) {
        const float jx = x0 + 12.0f, jy = (float)kTopBarH + 48.0f;
        draw_joycons(jx, jy, state);
        // Legend
        struct L { float r, g, b; const char* t; } leg[3] = {
            {0.96f, 0.96f, 0.96f, "Used"}, {kHighlightR, kHighlightG, kHighlightB, "Selected"}, {0.27f, 0.27f, 0.27f, "Reserved (pause, fast-forward, rewind)"}};
        float lx = jx + 30.0f, ly = jy + 440.0f;
        for (int i = 0; i < 3; i++) {
            if (i == 2) { lx = jx + 30.0f; ly += 30.0f; }
            ui_rrect(lx, ly + 2.0f, 18.0f, 18.0f, 5.0f, leg[i].r, leg[i].g, leg[i].b, 1.0f);
            switch_ui_draw_text(lx + 26.0f, ly, 18.0f, kTextR, kTextG, kTextB, 0.6f, leg[i].t);
            lx += 26.0f + switch_ui_text_width(leg[i].t, 18.0f) + 30.0f;
        }
        list_x = jx + 380.0f;
    }

    const float right = (float)logical_w - 40.0f;
    const float start_y = (float)kTopBarH + 72.0f, row_h = 50.0f, size = 25.0f;
    const float note_h = 76.0f;
    const float list_bottom = (float)(logical_h - kBottomBarH) - note_h - 8.0f;
    const int fit = std::max(1, (int)((list_bottom - start_y) / row_h));
    int first = 0;
    if (n > fit) first = std::max(0, std::min(g_settings_selected_row - fit / 2, n - fit));

    if (n == 0) {
        switch_ui_draw_text(list_x, start_y, size, kTextR, kTextG, kTextB, 0.7f, "This game has no button list.");
    }
    for (int i = first; i < n && i < first + fit; i++) {
        const float y = start_y + (float)(i - first) * row_h;
        const bool sel = i == g_settings_selected_row;
        menu_touch_item(list_x - 14.0f, y - 8.0f, (float)logical_w - 24.0f - (list_x - 14.0f), row_h, sel, [i] { g_settings_selected_row = i; },
                        HidNpadButton_A);
        if (sel) switch_ui_draw_rect(list_x - 14.0f, y - 8.0f, (float)logical_w - 24.0f - (list_x - 14.0f), row_h - 6.0f, kSelR, kSelG, kSelB, 1.0f);
        const float label_w_max = right - list_x - 170.0f;
        draw_text_fit(list_x, y, size, label_w_max, kTextR, kTextG, kTextB, sel ? 1.0f : 0.85f, rows[i].name, false);
        if (sel && g_ctl_waiting) {
            const std::string t = ctl_wait_text();
            const float tw = switch_ui_text_width(t, size, true);
            switch_ui_draw_text(right - tw, y, size, 1.0f, 0.77f, 0.31f, 1.0f, t, true);
        } else {
            std::vector<int> phys;
            for (int k : rows[i].keys) phys.push_back(control_remap_source(ref, k));
            draw_row_buttons(right, y, size, phys, 1.0f);
        }
    }
    if (first > 0) switch_ui_draw_text(right - 20.0f, start_y - 30.0f, 18.0f, kTextR, kTextG, kTextB, 0.5f, "^ more above");
    if (first + fit < n) switch_ui_draw_text(list_x, list_bottom - 4.0f, 18.0f, kTextR, kTextG, kTextB, 0.5f, "v more below");

    // Note box under the list.
    const float ny = (float)(logical_h - kBottomBarH) - note_h;
    const float nx = list_x - 14.0f, nw = (float)logical_w - 24.0f - nx;
    ui_rrect(nx, ny, nw, note_h - 8.0f, 10.0f, 0.31f, 0.31f, 0.31f, 1.0f);
    ui_rrect(nx + 1, ny + 1, nw - 2, note_h - 10.0f, 9.0f, kPanelR, kPanelG, kPanelB, 1.0f);
    switch_ui_draw_text(nx + 18.0f, ny + 10.0f, 20.0f, kTextR, kTextG, kTextB, 1.0f, pause_combo_note());
    switch_ui_draw_text(nx + 18.0f, ny + 38.0f, 18.0f, kTextR, kTextG, kTextB, 0.6f,
                        "If the button is already used, the two functions swap");
}

void render_controls_hints(int logical_h) {
    if (g_ctl_waiting) {
        switch_ui_draw_text(24.0f, (float)(logical_h - kBottomBarH + 14), 20.0f, kTextR, kTextG, kTextB, 0.85f,
                            "Press the new button (the same one keeps it) - none in 3 s: unassigned");
        return;
    }
    if (link_session_active()) {
        draw_hints(logical_h, {{SwitchButtonIcon::DpadUp, "Select"}, {SwitchButtonIcon::DpadLeft, "Player"}, {SwitchButtonIcon::A, "Change Button"},
                               {SwitchButtonIcon::X, "Default"}, {SwitchButtonIcon::Y, "Reset All"}, {SwitchButtonIcon::B, "Resume"}});
        return;
    }
    draw_hints(logical_h, {{SwitchButtonIcon::DpadUp, "Select"}, {SwitchButtonIcon::A, "Change Button"}, {SwitchButtonIcon::X, "Default"},
                           {SwitchButtonIcon::Y, "Reset All"}, {SwitchButtonIcon::R, "Section"}, {SwitchButtonIcon::B, "Resume"}});
}

void close_settings_screen() {
    g_settings_active = false;
    g_themes_sub = false;
    g_music_sub = false;
    g_ctl_waiting = false;
    g_ctl_seen = false;
    if (g_settings_from_gameplay) {
        g_settings_from_gameplay = false;
        g_menu_active = false;
    }
}

// Pause menu > Controls input. Returns true when it handled kDown (L/R
// section changes outside the wait are left to update_settings_menu()).
bool update_controls_screen(uint64_t kDown) {
    if (!g_ctl_seen) {
        g_ctl_seen = true;
        game_controls_reload_names();
    }
    const std::vector<ControlRow> rows = game_control_rows();
    const std::string ref = controls_ref();
    const int n = (int)rows.size();
    // (Linked, the shake row sits past the control rows - see below.)
    if (!link_session_active() && g_settings_selected_row >= n) g_settings_selected_row = std::max(0, n - 1);

    if (g_ctl_waiting) {
        if (!kDown) {
            // Nothing pressed for 3 s: the function is left without a button.
            if (ctl_wait_elapsed() >= kCtlWaitSeconds) {
                if (n > 0 && g_settings_selected_row < n && !rows[g_settings_selected_row].keys.empty())
                    control_remap_unassign(ref, rows[g_settings_selected_row].keys[0]);
                g_ctl_waiting = false;
            }
            return true;
        }
        const uint64_t pm = pause_combo_mask() | time_control_hotkey_mask();
        const int b = remap_button_from_mask(kDown & ~pm);
        if (b < 0) {
            if (kDown & pm) show_slot_toast("That button is reserved (pause, fast-forward or rewind)");
            return true;
        }
        if (n > 0 && !rows[g_settings_selected_row].keys.empty())
            control_remap_assign(ref, rows[g_settings_selected_row].keys[0], b);
        g_ctl_waiting = false;
        return true;
    }

    if (kDown & (HidNpadButton_L | HidNpadButton_R)) return false;
    if (kDown & (HidNpadButton_B | HidNpadButton_Plus)) {
        close_settings_screen();
        return true;
    }
    if (link_session_active()) {
        // Two columns (render_link_controls_screen()): Left / Right go to the
        // other player's column; on the shake row they pick the player.
        const int n1 = link_control_player_rows(0), n2 = link_control_player_rows(1);
        const int total = link_ctl_count();
        if (total == 0) return true;
        int& sel = g_settings_selected_row;
        if (sel >= total) sel = total - 1;
        const bool motion_row = link_motion_choice() && sel == total - 2;
        const bool sound_row = sel == total - 1;
        if (kDown & HidNpadButton_Down) sel = (sel + 1) % total;
        else if (kDown & HidNpadButton_Up) sel = (sel - 1 + total) % total;
        else if (sound_row) {
            if (kDown & (HidNpadButton_Right | HidNpadButton_A)) cycle_link_sound_button(+1);
            else if (kDown & HidNpadButton_Left) cycle_link_sound_button(-1);
            else if (kDown & HidNpadButton_X) { g_link_sound_button = 1; switch_settings_save(); }
        } else if (kDown & (HidNpadButton_Left | HidNpadButton_Right)) {
            if (motion_row) link_set_motion_player(1 - link_motion_player());
            else if (sel < n1 && n2 > 0) sel = n1 + std::min(sel, n2 - 1);
            else if (sel >= n1 && n1 > 0) sel = std::min(sel - n1, n1 - 1);
        } else if (motion_row) {
            if (kDown & HidNpadButton_A) link_set_motion_player(1 - link_motion_player());
        } else if (sel < n) {
            const ControlRow& row = rows[sel];
            if (kDown & HidNpadButton_A) {
                if (!row.keys.empty()) start_ctl_wait();
            } else if (kDown & HidNpadButton_X) {
                if (!row.keys.empty()) control_remap_reset(ref, row.keys[0]);
            }
        }
        if (kDown & HidNpadButton_Y) control_remap_reset_all(ref);
        return true;
    }
    if (n == 0) return true;
    if (kDown & HidNpadButton_Down) g_settings_selected_row = (g_settings_selected_row + 1) % n;
    if (kDown & HidNpadButton_Up) g_settings_selected_row = (g_settings_selected_row - 1 + n) % n;
    const ControlRow& row = rows[g_settings_selected_row];
    if (kDown & HidNpadButton_A) {
        if (!row.keys.empty()) start_ctl_wait();
    } else if (kDown & HidNpadButton_X) {
        if (!row.keys.empty()) control_remap_reset(ref, row.keys[0]);
    } else if (kDown & HidNpadButton_Y) {
        control_remap_reset_all(ref);
    }
    return true;
}

namespace {
// A tap on a section in the sidebar - what L/R do, straight to that one.
// (Declared in the anonymous namespace above, next to the sidebar.)
void settings_go_section(int i) {
    g_themes_sub = false;
    g_music_sub = false;
    g_ctl_waiting = false;
    g_ctl_seen = false;
    if (g_settings_from_gameplay) g_pause_section = i;
    else g_settings_section = i;
    g_settings_selected_row = 0;
}
}  // namespace

static void update_settings_menu(uint64_t kDown) {
    // "Press A again to delete" only holds while nothing else is pressed.
    if (kDown & ~(uint64_t)HidNpadButton_A) g_confirm_row = SettingsRow::Resume;
    // Waiting for a new button in Controls: every button counts, L/R too.
    if (g_ctl_waiting && controls_section_open()) {
        update_controls_screen(kDown);
        return;
    }
    if (kDown & (HidNpadButton_L | HidNpadButton_R)) {
        g_themes_sub = false;
        g_music_sub = false;
        g_ctl_waiting = false;
        g_ctl_seen = false;
        const int dir = (kDown & HidNpadButton_R) ? 1 : -1;
        if (g_settings_from_gameplay) {
            const int n = (int)pause_sections().size();
            g_pause_section = (g_pause_section + dir + n) % n;
        } else {
            const int n = (int)settings_sections().size();
            g_settings_section = (g_settings_section + dir + n) % n;
        }
        g_settings_selected_row = 0;
        return;
    }
    if (controls_section_open()) {
        update_controls_screen(kDown);
        return;
    }
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
            case SettingsRow::MenuRotation:
                g_menu_rotation = cycle_rotation(g_menu_rotation, dir);
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
                // The case strip is part of the canvas size baked in by
                // rebuild_layout_from_game() - re-run it so the change shows
                // at once in the pause menu instead of on the next game load.
                rebuild_layout_from_game();
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
                    const uint16_t idx = find_game_index_by_ref(g_last_game_played_ref, &found);
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
                g_pet_time_display = (PetTimeDisplay)(((int)g_pet_time_display + dir + kNumPetTimeDisplays) % kNumPetTimeDisplays);
                break;
            case SettingsRow::LogFile:
                g_log_file_enabled = !g_log_file_enabled;
                debug_log::set_enabled(g_log_file_enabled);
                break;
            case SettingsRow::SplashScreen:
                g_splash_screen = !g_splash_screen;
                break;
            case SettingsRow::MakerColors:
                g_maker_colors = !g_maker_colors;
                break;
            case SettingsRow::MenuTouch:
                g_menu_touch = !g_menu_touch;
                break;
            case SettingsRow::DateFormat:
                g_date_format = (uint8_t)((g_date_format + dir + 3) % 3);
                break;
            case SettingsRow::Records:
                g_records_enabled = !g_records_enabled;
                break;
            case SettingsRow::RecordHud:
                g_record_hud = (uint8_t)((g_record_hud + dir + 3) % 3);
                break;
            case SettingsRow::RecordsList:
                g_records_in_list = !g_records_in_list;
                break;
            case SettingsRow::AchNotify:
                g_ach_notify = (uint8_t)((g_ach_notify + dir + 3) % 3);
                break;
            case SettingsRow::AlertColor:
                g_alert_color ^= 1;
                break;
            case SettingsRow::ScreenPalette:
                g_screen_palette = (uint8_t)((g_screen_palette + dir + kNumScreenPalettes) % kNumScreenPalettes);
                break;
            case SettingsRow::Transition:
                g_transition = (uint8_t)((g_transition + dir + kNumTransitions) % kNumTransitions);
                break;
            case SettingsRow::ScreenSize:
                g_screen_size = (uint8_t)((g_screen_size + dir + kNumScreenSizes) % kNumScreenSizes);
                break;
            case SettingsRow::VibrationGame:
                g_vibration_game = (uint8_t)((g_vibration_game + dir + 3) % 3);
                break;
            case SettingsRow::VibrationPets:
                g_vibration_pets = !g_vibration_pets;
                break;
            case SettingsRow::VibrationMenu:
                g_vibration_menu = !g_vibration_menu;
                break;
            case SettingsRow::VibrationStrength:
                g_vibration_strength = (uint8_t)((g_vibration_strength + dir + 3) % 3);
                break;
            case SettingsRow::PetAlertLight:
                g_pet_alert_light = !g_pet_alert_light;
                break;
            case SettingsRow::MotionPedometer:
                g_motion_pedometer = !g_motion_pedometer;
                break;
            case SettingsRow::MotionTapSensor:
                g_motion_tap_sensor = !g_motion_tap_sensor;
                break;
            case SettingsRow::FastForward:
                g_fastforward_button = (uint8_t)time_control_next_choice(g_fastforward_button, dir, g_rewind_button);
                break;
            case SettingsRow::FastForwardSpeed:
                g_fastforward_speed = (uint8_t)((g_fastforward_speed + dir + 3) % 3);
                break;
            case SettingsRow::ButtonResponse:
                g_button_response = (uint8_t)((g_button_response + dir + 4) % 4);
                break;
            case SettingsRow::LinkSound:
                link_session_set_sound_player(link_session_sound_player() ? 0 : 1);
                break;
            case SettingsRow::LinkSoundButton:
                cycle_link_sound_button(dir);
                break;
            case SettingsRow::Rewind:
                g_rewind_button = (uint8_t)time_control_next_choice(g_rewind_button, dir, g_fastforward_button);
                break;
            case SettingsRow::RewindLength:
                g_rewind_length = (uint8_t)((g_rewind_length + dir + 4) % 4);
                break;
            case SettingsRow::TapSensitivity:
                g_tap_sensitivity = (uint8_t)((g_tap_sensitivity + dir + 3) % 3);
                break;
            case SettingsRow::IrSensor:
                g_ir_sensor = (uint8_t)((g_ir_sensor + dir + 3) % 3);
                break;
            case SettingsRow::IrSensitivity:
                g_ir_sensitivity = (uint8_t)((g_ir_sensitivity + dir + 3) % 3);
                break;
            case SettingsRow::PedometerSensitivity:
                g_pedometer_sensitivity = (uint8_t)((g_pedometer_sensitivity + dir + 3) % 3);
                break;
            case SettingsRow::Language:
                g_language = (uint8_t)((g_language + dir + kNumLanguages) % kNumLanguages);
                break;
            case SettingsRow::Volume:
                // Stops at both ends rather than wrapping 100 -> 0.
                g_sound_volume = (uint8_t)std::clamp((int)g_sound_volume + dir * 10, 0, 100);
                break;
            case SettingsRow::Mute:
                g_sound_mute = !g_sound_mute;
                break;
            case SettingsRow::SoundStyle:
                g_sound_style = (uint8_t)((g_sound_style + dir + kNumSoundStyles) % kNumSoundStyles);
                break;
            case SettingsRow::GameComSound:    g_tgc_sound ^= 1; break;
            case SettingsRow::GameComSpeed:    g_tgc_speed ^= 1; break;
            case SettingsRow::GameComNoise:    g_tgc_noise ^= 1; break;
            case SettingsRow::GameComPalette:  g_tgc_palette ^= 1; break;
            case SettingsRow::GameComGhost:    g_tgc_ghost = (uint8_t)((g_tgc_ghost + dir + 3) % 3); break;
            case SettingsRow::RZoneStyle:      g_rz_style = (uint8_t)((g_rz_style + dir + 3) % 3); break;
            case SettingsRow::AdvVisionScreen: g_eav_screen ^= 1; break;
            case SettingsRow::GameMasterSpeed: g_hgm_speed ^= 1; break;
            case SettingsRow::MenuSounds:
                g_menu_sounds = !g_menu_sounds;
                break;
            case SettingsRow::MenuMusic:
                g_menu_music = (uint8_t)((g_menu_music + dir + kNumMenuMusicChoices) % kNumMenuMusicChoices);
                break;
            case SettingsRow::MenuMusicTrack: {
                // 0 = "All (random)", then each file in the folder.
                const auto& files = menu_music_files();
                const int n = (int)files.size() + 1;
                int cur = 0;
                for (int i = 0; i < (int)files.size(); ++i)
                    if (files[i] == g_menu_music_track) cur = i + 1;
                cur = (cur + dir + n) % n;
                if (cur == 0) g_menu_music_track.clear();
                else g_menu_music_track = files[cur - 1];
                break;
            }
            case SettingsRow::MenuMusicVolume:
                g_menu_music_volume = (uint8_t)std::clamp((int)g_menu_music_volume + dir * 10, 0, 100);
                break;
            case SettingsRow::MenuMusicDuck:
                g_menu_music_duck = (uint8_t)((g_menu_music_duck + dir + 4) % 4);
                break;
            case SettingsRow::PauseCombo:
                g_pause_combo = (uint8_t)((g_pause_combo + dir + kNumPauseCombos) % kNumPauseCombos);
                break;
            case SettingsRow::OnScreenKeypad:
                g_onscreen_keypad = !g_onscreen_keypad;
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
                        const uint16_t idx = find_game_index_by_ref(g_last_game_played_ref, &found);
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
            case SettingsRow::MenuStyle:
                g_menu_style = (MenuStyle)(((int)g_menu_style + dir + 4) % 4);
                g_view = (g_menu_style == MenuStyle::Classic) ? MenuView::Classic : MenuView::Makers;
                break;
            case SettingsRow::MakerOrder:
                g_maker_order = (MakerOrder)(((int)g_maker_order + dir + 3) % 3);
                rebuild_tiles();
                break;
            case SettingsRow::MenuTheme:
                g_menu_theme = (uint8_t)((g_menu_theme + dir + kNumMenuThemes) % kNumMenuThemes);
                apply_menu_theme();
                break;
            case SettingsRow::MenuFont: {
                const int n = switch_ui_font_count();
                g_menu_font = (uint8_t)((g_menu_font + dir + n) % n);
                switch_ui_set_font(g_menu_font);
                break;
            }
            case SettingsRow::Ghosting:
                g_ghosting = (uint8_t)((g_ghosting + dir + 4) % 4);
                break;
            case SettingsRow::FlickerFilter:
                g_flicker_filter = (uint8_t)((g_flicker_filter + dir + 3) % 3);
                break;
            case SettingsRow::PixelStyle:
                if (g_game && is_dot_matrix_ref(g_game->ref))
                    g_pixel_style = g_pixel_style == 2 ? 1 : 2;  // Filled <-> Solid
                else
                    g_pixel_style = (uint8_t)((g_pixel_style + dir + 3) % 3);
                break;
            case SettingsRow::ScreenFilter:
                g_screen_filter = (uint8_t)((g_screen_filter + dir + kNumScreenFilters) % kNumScreenFilters);
                break;
            case SettingsRow::FilterStrength:
                g_filter_strength = (uint8_t)((g_filter_strength + dir + 3) % 3);
                break;
            default:
                break;
        }
        switch_settings_save();
    }

    // X: this option back to its default.
    if ((kDown & HidNpadButton_X) && !is_action && !row_locked && reset_row_to_default(row)) {
        switch_settings_save();
    }
    // Y: show / hide the explanation boxes.
    if (kDown & HidNpadButton_Y) {
        g_settings_help = !g_settings_help;
        switch_settings_save();
    }

    if (kDown & HidNpadButton_A) {
        if (row == SettingsRow::AchGame) return;  // just shows them
        if (row == SettingsRow::Resume) {
            g_settings_active = false;
            g_settings_from_gameplay = false;
            g_menu_active = false;
            return;
        }
        if (row == SettingsRow::RecordsClearAll || (row == SettingsRow::RecordsGame && g_game)) {
            // Two presses: the first one asks (the row's value), the second deletes.
            if (g_confirm_row != row) {
                g_confirm_row = row;
                return;
            }
            g_confirm_row = SettingsRow::Resume;
            if (row == SettingsRow::RecordsGame) {
                score_tracker_clear(g_game->ref);
            } else {
                high_score_clear_all();
                high_score_flush();
            }
            show_slot_toast(switch_tr("Records deleted"));
            return;
        }
        if (row == SettingsRow::TamaGoFigure && g_game) {
            g_fig_sel = g_tmg_figure;
            g_fig_picker = true;
            return;
        }
        if (row == SettingsRow::LinkConnect && g_game) {
            if (link_session_active()) {
                link_session_stop();
            } else {
                g_link_cands = link_candidates();
                g_link_sel = 0;
                g_link_picker = !g_link_cands.empty();
            }
            return;
        }
        if (row == SettingsRow::Restart && g_game) {
            // A fresh load of the same game, without saving the running one
            // first (that's what "starting over" means) and without loading
            // any save - then straight back into it.
            YOKOI_LOG("switch_menu: restarting '%s'", g_game->ref.c_str());
            switch_load_game_by_index_and_init(g_game_index);
            g_settings_active = false;
            g_settings_from_gameplay = false;
            g_menu_active = false;
            return;
        }
        if (row == SettingsRow::ThemesSub) {
            g_themes_sub = true;
            g_settings_selected_row = 0;
            return;
        }
        if (row == SettingsRow::MusicSub) {
            g_music_sub = true;
            g_settings_selected_row = 0;
            menu_music_rescan();  // files copied to the SD since show up
            return;
        }
        if (row == SettingsRow::SaveState) { open_slot_screen(SlotScreen::Save); return; }
        if (row == SettingsRow::LoadState) { open_slot_screen(SlotScreen::Load); return; }
        if (row == SettingsRow::SimulateTime) { open_slot_screen(SlotScreen::Simulate); return; }
        if (row == SettingsRow::UndoSimulate) { undo_simulate_now(); return; }
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
    if ((g_themes_sub || g_music_sub) && (kDown & HidNpadButton_B)) {
        g_settings_selected_row = g_music_sub ? 1 : 0;  // back on the Music / Themes row
        g_themes_sub = false;
        g_music_sub = false;
        return;
    }
    if ((kDown & HidNpadButton_Plus) || (kDown & HidNpadButton_B)) {
        g_settings_active = false;
        g_themes_sub = false;
        g_music_sub = false;
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
        menu_touch_idle();
        return;
    }

    // Settings > Menu > Touch Controls: taps, swipes and drags on the last
    // frame drawn, as button presses (input/menu_touch.h).
    if (g_menu_touch) kDown |= menu_touch_poll();
    else menu_touch_idle();

    if (g_load_prompt_active) {
        update_load_prompt(kDown);
        return;
    }
    if (g_pet_prompt_active) {
        update_pet_prompt(kDown);
        return;
    }
    if (g_slot_screen != SlotScreen::None) {
        update_slot_screen(kDown);
        return;
    }
    if (g_link_picker) {
        update_link_picker(kDown);
        return;
    }
    if (g_fig_picker) {
        update_fig_picker(kDown);
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

    // The settings screen can change what's visible (experimental games,
    // clone filter) - the manufacturer tiles are rebuilt after it closes.
    static bool s_tiles_dirty = false;
    if (kDown & HidNpadButton_Plus) {
        g_settings_active = true;
        g_settings_from_gameplay = false;
        g_settings_selected_row = 0;
        s_tiles_dirty = true;
        return;
    }

    if (s_tiles_dirty) {
        s_tiles_dirty = false;
        rebuild_tiles();
        if (g_view == MenuView::MakerGames) {
            const uint16_t keep = g_selected_index;
            open_tile(keep);
        }
    }

    if (g_view == MenuView::Makers && g_menu_style == MenuStyle::Carousel) {
        const int n = (int)g_tiles.size();
        if (n > 0) {
            // Wraps around: past the last card comes the first again.
            if (kDown & HidNpadButton_Right) g_tile_cursor = (g_tile_cursor + 1) % n;
            if (kDown & HidNpadButton_Left) g_tile_cursor = (g_tile_cursor - 1 + n) % n;
            if (kDown & HidNpadButton_R) carousel_jump_letter(+1);
            if (kDown & HidNpadButton_L) carousel_jump_letter(-1);
            if (kDown & HidNpadButton_A) {
                g_folder = 0;
                open_tile(g_selected_index);
                return;
            }
        }
        if (kDown & HidNpadButton_Y) g_view = MenuView::Classic;
        return;
    }

    if (g_view == MenuView::Makers && g_menu_style == MenuStyle::List && g_list_in_games) {
        // In the chosen one's games: A plays (the shared launch code below).
        if (kDown & (HidNpadButton_B | HidNpadButton_Left)) {
            if (g_folder) {
                // Out of the folder, back on its row in the maker's games.
                g_folder = 0;
                g_list_game = 0;
                return;
            }
            g_list_in_games = false;
            return;
        }
        if ((kDown & HidNpadButton_A) && is_folder_entry(g_list_entry) && !g_tiles.empty()) {
            g_folder = entry_folder(g_list_entry);
            g_list_game = 0;
            g_list_game_scroll = 0;
            return;
        }
        const int n = g_list_game_count;
        if (n <= 0) { g_list_in_games = false; return; }
        if (kDown & HidNpadButton_Down) g_list_game = (g_list_game + 1) % n;
        if (kDown & HidNpadButton_Up) g_list_game = (g_list_game - 1 + n) % n;
        if (kDown & HidNpadButton_R) g_list_game = std::min(n - 1, g_list_game + g_list_game_fit);
        if (kDown & HidNpadButton_L) g_list_game = std::max(0, g_list_game - g_list_game_fit);
        if ((kDown & HidNpadButton_Y) && !is_folder_entry(g_list_entry)) {
            toggle_favorite(g_selected_index);
            rebuild_tiles();
        }
        if (!(kDown & HidNpadButton_A)) return;
        goto launch_selected;  // the shared launch code at the end
    } else if (g_view == MenuView::Makers && g_menu_style == MenuStyle::List) {
        const int n = (int)g_tiles.size();
        if (n > 0) {
            const bool rot = g_menu_rotation != ScreenRotation::Deg0;
            const int page = list_rows_visible(rot ? 720 : 1280, rot ? 1280 : 720);
            if (kDown & HidNpadButton_Down) g_tile_cursor = (g_tile_cursor + 1) % n;
            if (kDown & HidNpadButton_Up) g_tile_cursor = (g_tile_cursor - 1 + n) % n;
            if (kDown & HidNpadButton_R) g_tile_cursor = std::min(n - 1, g_tile_cursor + page);
            if (kDown & HidNpadButton_L) g_tile_cursor = std::max(0, g_tile_cursor - page);
            if (kDown & (HidNpadButton_Up | HidNpadButton_Down | HidNpadButton_L | HidNpadButton_R)) {
                g_list_game = 0;
                g_folder = 0;
            }
            if (kDown & (HidNpadButton_A | HidNpadButton_Right)) {
                // Into its games, on this same screen.
                g_list_in_games = true;
                g_list_game = 0;
                g_folder = 0;
                return;
            }
        }
        if (kDown & HidNpadButton_Y) g_view = MenuView::Classic;
        return;
    }

    if (g_view == MenuView::Makers) {
        const int n = (int)g_tiles.size();
        const int cols = tile_columns(g_menu_rotation == ScreenRotation::Deg0 ? 1280 : 720);
        if (n > 0) {
            if (kDown & HidNpadButton_Right) g_tile_cursor = std::min(n - 1, g_tile_cursor + 1);
            if (kDown & HidNpadButton_Left) g_tile_cursor = std::max(0, g_tile_cursor - 1);
            if (kDown & HidNpadButton_Down) g_tile_cursor = std::min(n - 1, g_tile_cursor + cols);
            if (kDown & HidNpadButton_Up) g_tile_cursor = std::max(0, g_tile_cursor - cols);
            if (kDown & HidNpadButton_A) {
                g_folder = 0;
                open_tile(g_selected_index);
                return;
            }
        }
        if (kDown & HidNpadButton_Y) g_view = MenuView::Classic;
        return;
    }

    if (g_view == MenuView::MakerGames) {
        const int n = (int)g_group_games.size();
        if (kDown & HidNpadButton_B) {
            if (g_folder) {
                // Out of the folder, back on its row (the first) in the maker's games.
                g_folder = 0;
                open_tile();
                return;
            }
            rebuild_tiles();  // Favorites/Recent counts may have changed
            g_view = MenuView::Makers;
            return;
        }
        if ((kDown & HidNpadButton_A) && group_cursor_on_folder()) {
            g_folder = entry_folder(g_group_games[g_group_cursor]);
            open_tile();
            return;
        }
        const bool is_maker = !g_tiles.empty() && g_tiles[g_tile_cursor].kind == TileKind::Maker;
        if (is_maker && (kDown & HidNpadButton_L)) { step_maker_tile(-1); return; }
        if (is_maker && (kDown & HidNpadButton_R)) { step_maker_tile(+1); return; }
        if (n == 0) return;
        if (kDown & HidNpadButton_Down) g_group_cursor = (g_group_cursor + 1) % n;
        if (kDown & HidNpadButton_Up) g_group_cursor = (g_group_cursor - 1 + n) % n;
        if (kDown & HidNpadButton_Right) g_group_cursor = std::min(n - 1, g_group_cursor + 8);
        if (kDown & HidNpadButton_Left) g_group_cursor = std::max(0, g_group_cursor - 8);
        sync_group_selection();
        if ((kDown & HidNpadButton_Y) && !group_cursor_on_folder()) {
            toggle_favorite(g_selected_index);
            if (g_tiles[g_tile_cursor].kind == TileKind::Favorites) {  // unfavorited: drop it from this list
                rebuild_group_games();
                sync_group_selection();
            }
        }
        if (is_maker && (kDown & HidNpadButton_Minus)) {
            const uint16_t keep = g_selected_index;
            g_group_by_date = !g_group_by_date;
            open_tile(keep);
        }
        if (n == 0 || !(kDown & HidNpadButton_A)) return;
        // A falls through to the shared launch code below.
    } else {
        // Classic view.
        if (kDown & HidNpadButton_B) {
            g_view = MenuView::Makers;
            return;
        }
        if (kDown & (HidNpadButton_Right | HidNpadButton_Up)) g_nav_dir = 1;
        if (kDown & (HidNpadButton_Left | HidNpadButton_Down)) g_nav_dir = -1;
        if (kDown & HidNpadButton_Right) select_next(+1);
        if (kDown & HidNpadButton_Left) select_next(-1);
        if (kDown & HidNpadButton_Up) select_next_other_group(+1);
        if (kDown & HidNpadButton_Down) select_next_other_group(-1);

        if (kDown & HidNpadButton_Minus) {
            g_sort_mode = (SortMode)(((int)g_sort_mode + 1) % kSortModeCount);
            rebuild_sorted_order();
        }
    }

launch_selected:
    if (kDown & HidNpadButton_A) {
        // Hidden debug mode bypasses save states entirely (see g_debug_mode's
        // doc comment in switch_settings.h) - forcing has_save false here is
        // enough to route straight to the always-fresh path below, same as a
        // game with no save at all.
        const bool has_save = !g_debug_mode && save_state_exists(g_selected_index);
        bool any_slot = has_save;
        for (int s = 1; s <= kNumSaveSlots && !g_debug_mode; ++s) any_slot = any_slot || save_state_slot_exists(g_selected_index, s);

        if (any_slot && g_load_state_behavior == LoadStateBehavior::Ask) {
            // The card screen (Auto save + slots + New Game) - see the
            // "Save slots" block above.
            YOKOI_LOG("switch_menu: A pressed on index %u, saves exist (Ask) - showing the load cards", (unsigned)g_selected_index);
            open_slot_screen(SlotScreen::Entry);
        } else if (has_save && g_load_state_behavior == LoadStateBehavior::AutoLoad) {
            YOKOI_LOG("switch_menu: A pressed on index %u, save exists (Auto-Load) - loading directly", (unsigned)g_selected_index);
            note_recent(g_selected_index);
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
            note_recent(g_selected_index);
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
    g_pause_section = 0;  // always opens on Game Session (Resume)
}

static void switch_menu_render_frame(int viewport_w, int viewport_h);

// Draws the menu; for the views with transitions (render/switch_transition.h)
// it goes through an off-screen buffer so the previous frame can be blended
// into the new one when the game / maker shown changes.
void switch_menu_render(int viewport_w, int viewport_h) {
    const bool overlay = g_settings_active || g_load_prompt_active || g_pet_prompt_active || g_slot_screen != SlotScreen::None ||
                         g_experimental_games_confirm_active || g_experimental_features_confirm_active;
    const bool classic = g_view == MenuView::Classic;
    const bool carousel = g_view == MenuView::Makers && g_menu_style == MenuStyle::Carousel;
    const bool want = !overlay && (classic || carousel) && g_transition != kTrOff && g_menu_rotation == ScreenRotation::Deg0;
    if (!want || !transition_capture_begin(viewport_w, viewport_h)) {
        transition_reset();
        switch_menu_render_frame(viewport_w, viewport_h);
        return;
    }
    switch_menu_render_frame(viewport_w, viewport_h);
    // What the frame shows: the game (one-at-a-time view) or the maker
    // card - except Slide in the Carousel, whose cards glide by themselves.
    static int s_last_tile = 0;
    uint64_t key;
    int dir = g_nav_dir;
    if (classic) {
        key = 0x10000ull | g_selected_index;
    } else {
        // Around the ring: last -> first counts as a step forward.
        const int n = (int)g_tiles.size();
        int diff = g_tile_cursor - s_last_tile;
        if (n > 0 && diff > n / 2) diff -= n;
        if (n > 0 && diff < -n / 2) diff += n;
        dir = diff >= 0 ? 1 : -1;
        key = (g_transition == kTrSlide) ? 0x20000ull : (0x30000ull | (uint64_t)g_tile_cursor);
    }
    s_last_tile = g_tile_cursor;
    const float bg[3] = {kBgR, kBgG, kBgB}, hl[3] = {kHighlightR, kHighlightG, kHighlightB};
    transition_capture_end(viewport_w, viewport_h, key, dir, (float)kTopBarH, (float)(viewport_h - kBottomBarH), bg, hl);
}

static void switch_menu_render_frame(int viewport_w, int viewport_h) {
    reload_thumbnail_if_needed();

    // viewport_w/viewport_h are the true (always-landscape) physical
    // dimensions - the actual glViewport() call always uses them as-is.
    // Everything else below lays out in "logical" space, which is
    // width/height-swapped at 90/270 so the whole menu (not just gameplay)
    // rotates - see switch_ui_begin_frame()'s and switch_screen_render()'s
    // matching uRotCosSin setup for why swapping the fit/layout dimensions
    // here and rotating the final NDC output there is lossless either way.
    int logical_w = viewport_w, logical_h = viewport_h;
    if (g_menu_rotation != ScreenRotation::Deg0) {
        std::swap(logical_w, logical_h);
    }
    switch_ui_set_rotation((int)g_menu_rotation);

    glViewport(0, 0, viewport_w, viewport_h);
    glClearColor(kBgR, kBgG, kBgB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    switch_ui_begin_frame(logical_w, logical_h);
    g_menu_lw = logical_w;
    menu_touch_begin_frame(logical_w, logical_h, (int)g_menu_rotation);

    switch_ui_draw_rect(0.0f, 0.0f, (float)logical_w, (float)logical_h, kBgR, kBgG, kBgB, 1.0f);
    switch_ui_draw_rect(0.0f, (float)(kTopBarH - 1), (float)logical_w, 1.0f, kTextR, kTextG, kTextB, 0.4f);
    switch_ui_draw_rect(0.0f, (float)(logical_h - kBottomBarH), (float)logical_w, 1.0f, kTextR, kTextG, kTextB, 0.4f);

    // Small header icon, left of the title - drawn at a fixed square size
    // matching the title's own line height rather than the source PNG's
    // native 48x48, so it scales consistently regardless of that asset's
    // actual dimensions.
    float title_x = 24.0f;
    // In the pause menu: the game's console art and "Name (Maker)", left of
    // the "Settings" tag on the right.
    const float header_max_x = (float)logical_w - 24.0f - switch_ui_text_width("Settings (DEBUG)", 20.0f) - 24.0f;
    if (!draw_pause_header(title_x, header_max_x)) {
        if (g_header_icon_tex != 0) {
            constexpr float kIconSize = 28.0f;
            switch_ui_draw_texture_plain(g_header_icon_tex, title_x, 16.0f, kIconSize, kIconSize);
            title_x += kIconSize + 10.0f;
        }
        switch_ui_draw_text(title_x, 16.0f, 28.0f, kTextR, kTextG, kTextB, 1.0f, g_settings_from_gameplay ? "PAUSED" : "YOKOI", /*bold=*/true);
    }

    if (g_load_prompt_active) {
        render_load_prompt(logical_w, logical_h);
        return;
    }
    if (g_pet_prompt_active) {
        render_pet_prompt(logical_w, logical_h);
        return;
    }
    if (g_slot_screen == SlotScreen::Simulate) {
        render_simulate_screen(logical_w, logical_h);
        return;
    }
    if (g_slot_screen != SlotScreen::None) {
        render_slot_cards(logical_w, logical_h);
        return;
    }
    if (g_link_picker) {
        render_link_picker(logical_w, logical_h);
        return;
    }
    if (g_fig_picker) {
        render_fig_picker(logical_w, logical_h);
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

        render_settings_sidebar(logical_w, logical_h);
        if (controls_section_open()) render_controls_screen(logical_w, logical_h);
        else render_settings_rows(logical_w, logical_h);
        if (g_slot_toast_frames > 0) {
            // "Saved to Slot 2" - a couple of seconds, bottom right.
            g_slot_toast_frames--;
            const float ts = 22.0f, tw = switch_ui_text_width(g_slot_toast, ts, true);
            const float tx = (float)logical_w - 30.0f - tw, ty = (float)(logical_h - kBottomBarH) - 52.0f;
            switch_ui_draw_rect(tx - 14.0f, ty - 10.0f, tw + 28.0f, ts + 20.0f, kSelR, kSelG, kSelB, 0.95f);
            switch_ui_draw_text(tx, ty, ts, kTextR, kTextG, kTextB, 1.0f, g_slot_toast, true);
        }
        if (!g_settings_from_gameplay && g_themes_sub) {
            render_theme_previews(logical_w, logical_h);
        }
        if (controls_section_open()) {
            render_controls_hints(logical_h);
            return;
        }

        HintBar hb(logical_h);
        auto put_icon = [&](SwitchButtonIcon icon) { hb.icon(icon); };
        auto put_text = [&](const std::string& s) { hb.text(s); };

        put_icon(SwitchButtonIcon::DpadUp);
        put_icon(SwitchButtonIcon::DpadDown);
        put_text("Select");
        put_icon(SwitchButtonIcon::DpadLeft);
        put_icon(SwitchButtonIcon::DpadRight);
        put_text("Change");
        put_icon(SwitchButtonIcon::L);
        put_icon(SwitchButtonIcon::R);
        put_text("Section");
        put_icon(SwitchButtonIcon::X);
        put_text("Reset");
        put_icon(SwitchButtonIcon::Y);
        put_text("Help");
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

    if (g_view == MenuView::Makers) {
        if (g_menu_style == MenuStyle::Carousel) render_maker_carousel(logical_w, logical_h);
        else if (g_menu_style == MenuStyle::List) render_maker_list(logical_w, logical_h);
        else render_makers(logical_w, logical_h);
        return;
    }
    if (g_view == MenuView::MakerGames && !g_tiles.empty()) {
        render_maker_games(logical_w, logical_h);
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
    const std::string date_str = g ? format_date(g->date) : std::string();
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
    // With Maker Colors on, the frame (and the maker name below) take the
    // maker's colour, so moving from one maker's games to the next shows.
    float fr = kAccentR, fg = kAccentG, fb = kAccentB, fa = 0.85f;
    float nr = kHighlightR, ng = kHighlightG, nb = kHighlightB;
    if (g_maker_colors) {
        maker_color_of(get_mfr(g_selected_index), fr, fg, fb);
        fa = 1.0f;
        nr = fr; ng = fg; nb = fb;
    }
    menu_touch_button(box_x - frame_pad, box_y - frame_pad, box_w + frame_pad * 2.0f, box_h + frame_pad * 2.0f, HidNpadButton_A);
    switch_ui_draw_rect(box_x - frame_pad, box_y - frame_pad, box_w + frame_pad * 2.0f, box_h + frame_pad * 2.0f,
                         fr, fg, fb, fa);
    switch_ui_draw_rect(box_x, box_y, box_w, box_h, kTileR, kTileG, kTileB, 1.0f);

    if (g_thumb_tex != 0 && g && g->console_info) {
        const uint16_t* ci = g->console_info;
        switch_ui_draw_texture(g_thumb_tex, box_x, box_y, box_w, box_h,
                                (float)g_thumb_w, (float)g_thumb_h, (float)ci[2], (float)ci[3], (float)ci[4], (float)ci[5]);
    }

    float text_y = box_y + box_h + frame_pad + gap_thumb_to_text;

    const float mfr_w = switch_ui_text_width(mfr, mfr_size);
    switch_ui_draw_text(cx - mfr_w * 0.5f, text_y, mfr_size, nr, ng, nb, 1.0f, mfr);
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

    // Its records (Settings > Game List > Show Records in List), beside the card.
    if (g && g_records_enabled && g_records_in_list && score_tracker_has_records(g->ref)) {
        const std::vector<ScoreRecord> recs = score_tracker_records(g->ref);
        const float px = box_x + box_w + frame_pad + 28.0f, pw = 250.0f;
        const float ts = 17.0f, rs = 24.0f, rh = rs + 10.0f;
        const float ph = 20.0f + ts + 10.0f + rh * (float)recs.size();
        if (!recs.empty() && px + pw <= (float)logical_w - 16.0f) {
            const float py = box_y + (box_h - ph) * 0.5f;
            switch_ui_draw_rect(px - 1.0f, py - 1.0f, pw + 2.0f, ph + 2.0f, kTextR, kTextG, kTextB, 0.25f);
            switch_ui_draw_rect(px, py, pw, ph, kTileR, kTileG, kTileB, 1.0f);
            switch_ui_draw_text(px + 16.0f, py + 12.0f, ts, 1.0f, 0.77f, 0.31f, 1.0f, switch_tr("HIGH SCORES"), true);
            float ry = py + 22.0f + ts;
            for (const ScoreRecord& r : recs) {
                char num[16];
                if (r.best) snprintf(num, sizeof(num), "%04u", (unsigned)r.best);
                else snprintf(num, sizeof(num), "-");
                switch_ui_draw_text(px + 16.0f, ry, rs, kTextR, kTextG, kTextB, 0.9f, r.label);
                switch_ui_draw_text(px + pw - 16.0f - switch_ui_text_width(num, rs, true), ry, rs, kTextR, kTextG, kTextB, 1.0f, num, true);
                ry += rh;
            }
        }
    }

    // Nintendo button-prompt glyphs instead of plain text/ASCII arrows, same
    // as Hekate-Toolbox's own control hints (switch_ui.cpp's
    // switch_ui_draw_button_icon - see its header comment for where the
    // codepoints come from). switch_ui has no rich-text markup for mixing
    // icons and text, so this composes the line by hand, tracking cursor_x
    // as each piece's width is drawn.
    {
        HintBar hb(logical_h);
        auto put_icon = [&](SwitchButtonIcon icon) { hb.icon(icon); };
        auto put_text = [&](const std::string& s) { hb.text(s); };

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
        put_icon(SwitchButtonIcon::B);
        put_text("Maker Menu");  // B: back to the by-maker menu
        put_icon(SwitchButtonIcon::Plus);
        put_text("Settings");
    }
}

// See switch_menu.h.
void switch_menu_frame_color(uint16_t game_index, float& r, float& g, float& b) {
    r = kAccentR;
    g = kAccentG;
    b = kAccentB;
    if (g_maker_colors) maker_color_of(get_mfr(game_index), r, g, b);
}

// The record / achievement alerts' colours (score_tracker.cpp), following
// Settings > Alert Colour: 1 = the menu theme's accent and panel.
void menu_theme_alert_colors(float& ar, float& ag, float& ab, float& pr, float& pg, float& pb) {
    ar = kAccentR; ag = kAccentG; ab = kAccentB;
    pr = kPanelR; pg = kPanelG; pb = kPanelB;
}
