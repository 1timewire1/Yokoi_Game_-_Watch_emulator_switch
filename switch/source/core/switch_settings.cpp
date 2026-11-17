#include "switch_settings.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "std/debug_log.h"
#include "std/platform_paths.h"
#include "std/settings.h"

ScreenRotation g_screen_rotation = ScreenRotation::Deg0;
ScreenRotation g_menu_rotation = ScreenRotation::Deg0;
bool g_hide_console_art = true;
bool g_hide_background_art = false;
LoadStateBehavior g_load_state_behavior = LoadStateBehavior::Ask;
CloneFilterMode g_clone_filter_mode = CloneFilterMode::ShowAll;
MenuStyle g_menu_style = MenuStyle::Makers;
MakerOrder g_maker_order = MakerOrder::AZ;
uint8_t g_menu_theme = 0;
uint8_t g_menu_font = 0;
uint8_t g_ghosting = 0;
uint8_t g_flicker_filter = 0;
uint8_t g_pixel_style = 0;
uint8_t g_screen_filter = 0;
uint8_t g_filter_strength = 1;
bool g_log_file_enabled = false;
bool g_splash_screen = true;
bool g_maker_colors = true;
bool g_menu_touch = true;
uint8_t g_link_sound_button = 1;
uint8_t g_date_format = 0;
bool g_records_enabled = true;
uint8_t g_record_hud = 1;
bool g_records_in_list = true;
uint8_t g_ach_notify = 0;
uint8_t g_alert_color = 0;
uint8_t g_language = 0;
uint8_t g_screen_size = 2;
uint8_t g_transition = 1;
uint8_t g_sound_volume = 100;
bool g_sound_mute = false;
uint8_t g_sound_style = 0;
uint8_t g_tgc_sound = 0;
uint8_t g_tgc_speed = 0;
uint8_t g_tgc_noise = 0;
uint8_t g_tgc_palette = 0;
uint8_t g_tgc_ghost = 0;
uint8_t g_rz_style = 0;
uint8_t g_eav_screen = 0;
uint8_t g_tmg_figure = 0;
uint32_t g_tmg_contacts = 0xFFFFFFu;
uint8_t g_hgm_speed = 0;
bool g_menu_sounds = true;
uint8_t g_menu_music = 0;
std::string g_menu_music_track;
uint8_t g_menu_music_volume = 60;
uint8_t g_menu_music_duck = 2;
uint8_t g_pause_combo = 0;
bool g_onscreen_keypad = true;
bool g_settings_help = true;
uint8_t g_fastforward_button = 0;
uint8_t g_fastforward_speed = 1;
uint8_t g_button_response = 2;
uint8_t g_rewind_button = 0;
uint8_t g_rewind_length = 2;
uint8_t g_vibration_game = 0;
bool g_vibration_pets = false;
bool g_vibration_menu = false;
uint8_t g_vibration_strength = 1;
bool g_pet_alert_light = true;
bool g_motion_pedometer = true;
uint8_t g_pedometer_sensitivity = 1;
bool g_motion_tap_sensor = true;
uint8_t g_tap_sensitivity = 1;
uint8_t g_ir_sensor = 1;
uint8_t g_ir_sensitivity = 1;
std::string g_last_game_played_ref;
bool g_experimental_games_enabled = false;
uint8_t g_panel_gap_percent = 0;
uint32_t g_segment_color = kSegmentColorAuto;
bool g_experimental_features_enabled = false;
bool g_debug_mode = false;
uint8_t g_dosimeter_reading = 0;
TimeSetMode g_time_set_mode = TimeSetMode::Automatic;
PetTimeMode g_pet_time_mode = PetTimeMode::RealTime;
uint32_t g_pet_time_cap_hours = 72;
PetTimeDisplay g_pet_time_display = PetTimeDisplay::Progress;

namespace {

std::string settings_txt_path() {
    return config_path("yokoi_settings.txt");
}

} // namespace

void switch_settings_load() {
    reset_settings_to_default();
    g_screen_rotation = ScreenRotation::Deg0;
    g_menu_rotation = ScreenRotation::Deg0;
    bool menu_rotation_found = false;
    g_hide_console_art = true;
    g_hide_background_art = false;
    g_load_state_behavior = LoadStateBehavior::Ask;
    g_clone_filter_mode = CloneFilterMode::ShowAll;
    g_menu_style = MenuStyle::Makers;
    g_maker_order = MakerOrder::AZ;
    g_menu_theme = 0;
    g_menu_font = 0;
    g_ghosting = 0;
    g_flicker_filter = 0;
    g_pixel_style = 0;
    g_screen_filter = 0;
    g_filter_strength = 1;
    g_log_file_enabled = false;
    g_splash_screen = true;
    g_maker_colors = true;
    g_menu_touch = true;
    g_link_sound_button = 1;
    g_date_format = 0;
    g_records_enabled = true;
    g_record_hud = 1;
    g_records_in_list = true;
    g_ach_notify = 0;
    g_alert_color = 0;
    g_language = 0;
    g_screen_size = 2;
    g_screen_palette = 0;
    g_transition = 1;
    g_sound_volume = 100;
    g_sound_mute = false;
    g_sound_style = 0;
    g_tgc_sound = g_tgc_speed = g_tgc_noise = g_tgc_palette = g_tgc_ghost = g_hgm_speed = g_rz_style = g_eav_screen = 0;
    g_menu_sounds = true;
    g_menu_music = 0;
    g_menu_music_track.clear();
    g_menu_music_volume = 60;
    g_menu_music_duck = 2;
    g_pause_combo = 0;
    g_onscreen_keypad = true;
    g_settings_help = true;
    g_fastforward_button = 0;
    g_fastforward_speed = 1;
    g_button_response = 2;
    g_rewind_button = 0;
    g_rewind_length = 2;
    g_vibration_game = 0;
    g_vibration_pets = false;
    g_vibration_menu = false;
    g_vibration_strength = 1;
    g_pet_alert_light = true;
    g_motion_pedometer = true;
    g_pedometer_sensitivity = 1;
    g_motion_tap_sensor = true;
    g_tap_sensitivity = 1;
    g_ir_sensor = 1;
    g_ir_sensitivity = 1;
    g_last_game_played_ref.clear();
    g_experimental_games_enabled = false;
    g_panel_gap_percent = 0;
    g_segment_color = kSegmentColorAuto;
    g_experimental_features_enabled = false;
    g_debug_mode = false;
    g_dosimeter_reading = 0;
    g_time_set_mode = TimeSetMode::Automatic;
    g_pet_time_mode = PetTimeMode::RealTime;
    g_pet_time_cap_hours = 72;
    g_pet_time_display = PetTimeDisplay::Progress;

    // Applied before anything below gets a chance to log, so even the
    // "no settings file" case respects logging being off by default -
    // debug_log's own module-level default is "enabled" (so platforms that
    // never call this behave as before), which would otherwise briefly win.
    // (g_debug_mode is always false here - debug=1 can only ever come from a
    // file that, by definition, doesn't exist in this branch - but including
    // it keeps this in sync with the real check further down.)
    debug_log::set_enabled(g_log_file_enabled || g_debug_mode);

    const std::string path = settings_txt_path();
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        return;
    }

    char line[512];  // menumusictrack holds a file name
    while (fgets(line, sizeof(line), file)) {
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';

        // Accepts decimal or 0x-prefixed hex on the value side (base 0).
        const long value = strtol(eq + 1, nullptr, 0);

        if (strcmp(line, "background_color") == 0) {
            g_settings.background_color = (uint32_t)value;
        } else if (strcmp(line, "segment_color") == 0) {
            const char* v = eq + 1;
            while (*v == ' ') v++;
            g_segment_color = (strncmp(v, "auto", 4) == 0) ? kSegmentColorAuto : ((uint32_t)value & 0xFFFFFFu);
        } else if (strcmp(line, "segment_marking_alpha") == 0) {
            g_settings.segment_marking_alpha = (uint8_t)value;
        } else if (strcmp(line, "rotation") == 0) {
            if (value == 1) g_screen_rotation = ScreenRotation::Deg90;
            else if (value == 2) g_screen_rotation = ScreenRotation::Deg270;
            else g_screen_rotation = ScreenRotation::Deg0;
        } else if (strcmp(line, "menurotation") == 0) {
            menu_rotation_found = true;
            if (value == 1) g_menu_rotation = ScreenRotation::Deg90;
            else if (value == 2) g_menu_rotation = ScreenRotation::Deg270;
            else g_menu_rotation = ScreenRotation::Deg0;
        } else if (strcmp(line, "hide_console_art") == 0) {
            g_hide_console_art = (value != 0);
        } else if (strcmp(line, "hide_background_art") == 0) {
            g_hide_background_art = (value != 0);
        } else if (strcmp(line, "loadstate") == 0) {
            if (value == 1) g_load_state_behavior = LoadStateBehavior::AutoLoad;
            else if (value == 2) g_load_state_behavior = LoadStateBehavior::NeverLoad;
            else g_load_state_behavior = LoadStateBehavior::Ask;
        } else if (strcmp(line, "clonefilter") == 0) {
            if (value == 1) g_clone_filter_mode = CloneFilterMode::HideDuplicates;
            else if (value == 2) g_clone_filter_mode = CloneFilterMode::HideAllClones;
            else g_clone_filter_mode = CloneFilterMode::ShowAll;
        } else if (strcmp(line, "menustyle") == 0) {
            g_menu_style = (value == 1) ? MenuStyle::Classic : (value == 2) ? MenuStyle::Carousel : (value == 3) ? MenuStyle::List : MenuStyle::Makers;
        } else if (strcmp(line, "makerorder") == 0) {
            if (value == 1) g_maker_order = MakerOrder::MostGames;
            else if (value == 2) g_maker_order = MakerOrder::Era;
            else g_maker_order = MakerOrder::AZ;
        } else if (strcmp(line, "menutheme") == 0) {
            g_menu_theme = (uint8_t)(value < 32 ? value : 0);
        } else if (strcmp(line, "menufont") == 0) {
            g_menu_font = (uint8_t)(value < 32 ? value : 0);
        } else if (strcmp(line, "ghosting") == 0) {
            g_ghosting = (uint8_t)(value <= 3 ? value : 0);
        } else if (strcmp(line, "pixelstyle") == 0) {
            g_pixel_style = (uint8_t)(value >= 0 && value <= 2 ? value : 0);  // 0 Classic, 1 Filled, 2 Solid
        } else if (strcmp(line, "flickerfilter") == 0) {
            g_flicker_filter = (uint8_t)(value >= 0 && value <= 2 ? value : 0);
        } else if (strcmp(line, "screenfilter") == 0) {
            g_screen_filter = (uint8_t)(value < 32 ? value : 0);
        } else if (strcmp(line, "filterstrength") == 0) {
            g_filter_strength = (uint8_t)(value <= 2 ? value : 1);
        } else if (strcmp(line, "volume") == 0) {
            g_sound_volume = (uint8_t)(value <= 100 ? (value / 10) * 10 : 100);
        } else if (strcmp(line, "mute") == 0) {
            g_sound_mute = (value != 0);
        } else if (strcmp(line, "soundstyle") == 0) {
            g_sound_style = (uint8_t)(value < kNumSoundStyles ? value : 0);
        } else if (strcmp(line, "tgcsound") == 0) {
            g_tgc_sound = (uint8_t)(value == 1 ? 1 : 0);
        } else if (strcmp(line, "tgcspeed") == 0) {
            g_tgc_speed = (uint8_t)(value == 1 ? 1 : 0);
        } else if (strcmp(line, "tgcnoise") == 0) {
            g_tgc_noise = (uint8_t)(value == 1 ? 1 : 0);
        } else if (strcmp(line, "tgcpalette") == 0) {
            g_tgc_palette = (uint8_t)(value == 1 ? 1 : 0);
        } else if (strcmp(line, "tgcghost") == 0) {
            g_tgc_ghost = (uint8_t)(value >= 0 && value <= 2 ? value : 0);
        } else if (strcmp(line, "tmgfigure") == 0) {
            g_tmg_figure = (uint8_t)(value >= 0 && value <= 12 ? value : 0);
        } else if (strcmp(line, "tmgcontacts") == 0) {
            g_tmg_contacts = (uint32_t)value & 0xFFFFFFu;
        } else if (strcmp(line, "eavscreen") == 0) {
            g_eav_screen = (uint8_t)(value == 1 ? 1 : 0);
        } else if (strcmp(line, "rzstyle") == 0) {
            g_rz_style = (uint8_t)(value >= 0 && value <= 2 ? value : 0);
        } else if (strcmp(line, "hgmspeed") == 0) {
            g_hgm_speed = (uint8_t)(value == 1 ? 1 : 0);
        } else if (strcmp(line, "vibration") == 0) {
            g_vibration_game = (uint8_t)(value <= 2 ? value : 0);
        } else if (strcmp(line, "vibrationpets") == 0) {
            g_vibration_pets = (value != 0);
        } else if (strcmp(line, "vibrationmenu") == 0) {
            g_vibration_menu = (value != 0);
        } else if (strcmp(line, "vibrationstrength") == 0) {
            g_vibration_strength = (uint8_t)(value <= 2 ? value : 1);
        } else if (strcmp(line, "petalertlight") == 0) {
            g_pet_alert_light = (value != 0);
        } else if (strcmp(line, "motionpedometer") == 0) {
            g_motion_pedometer = (value != 0);
        } else if (strcmp(line, "irsensor") == 0) {
            g_ir_sensor = (uint8_t)(value >= 0 && value <= 2 ? value : 1);
        } else if (strcmp(line, "irsens") == 0) {
            g_ir_sensitivity = (uint8_t)(value >= 0 && value <= 2 ? value : 1);
        } else if (strcmp(line, "tapsens") == 0) {
            g_tap_sensitivity = (uint8_t)(value >= 0 && value <= 2 ? value : 1);
        } else if (strcmp(line, "motiontap") == 0) {
            g_motion_tap_sensor = (value != 0);
        } else if (strcmp(line, "pedometersens") == 0) {
            g_pedometer_sensitivity = (uint8_t)(value >= 0 && value <= 2 ? value : 1);
        } else if (strcmp(line, "menusounds") == 0) {
            g_menu_sounds = (value != 0);
        } else if (strcmp(line, "menumusic") == 0) {
            g_menu_music = (uint8_t)(value >= 0 && value <= 10 ? value : 0);
        } else if (strcmp(line, "menumusicvol") == 0) {
            g_menu_music_volume = (uint8_t)(value >= 0 && value <= 100 ? (value / 10) * 10 : 60);
        } else if (strcmp(line, "fastforward") == 0) {
            g_fastforward_button = (uint8_t)(value >= 0 && value <= 4 ? value : 0);
        } else if (strcmp(line, "buttonresponse") == 0) {
            g_button_response = (uint8_t)(value >= 0 && value <= 3 ? value : 2);
        } else if (strcmp(line, "ffspeed") == 0) {
            g_fastforward_speed = (uint8_t)(value >= 0 && value <= 2 ? value : 1);
        } else if (strcmp(line, "rewind") == 0) {
            g_rewind_button = (uint8_t)(value >= 0 && value <= 4 ? value : 0);
        } else if (strcmp(line, "rewindlength") == 0) {
            g_rewind_length = (uint8_t)(value >= 0 && value <= 3 ? value : 2);
        } else if (strcmp(line, "settingshelp") == 0) {
            g_settings_help = (value != 0);
        } else if (strcmp(line, "onscreenkeypad") == 0) {
            g_onscreen_keypad = (value != 0);
        } else if (strcmp(line, "pausecombo") == 0) {
            g_pause_combo = (uint8_t)(value >= 0 && value <= 3 ? value : 0);
        } else if (strcmp(line, "menumusicduck") == 0) {
            g_menu_music_duck = (uint8_t)(value >= 0 && value <= 3 ? value : 2);
        } else if (strcmp(line, "menumusictrack") == 0) {
            std::string t(eq + 1);
            while (!t.empty() && (t.back() == '\n' || t.back() == '\r')) t.pop_back();
            g_menu_music_track = t;
        } else if (strcmp(line, "transition") == 0) {
            g_transition = (uint8_t)(value < 12 ? value : 1);
        } else if (strcmp(line, "screenpalette") == 0) {
            g_screen_palette = (uint8_t)(value < 64 ? value : 0);
        } else if (strcmp(line, "screensize") == 0) {
            g_screen_size = (uint8_t)(value < kNumScreenSizes ? value : 2);
        } else if (strcmp(line, "language") == 0) {
            g_language = (uint8_t)(value <= 1 ? value : 0);
        } else if (strcmp(line, "splash") == 0) {
            g_splash_screen = (value != 0);
        } else if (strcmp(line, "makercolors") == 0) {
            g_maker_colors = (value != 0);
        } else if (strcmp(line, "menutouch") == 0) {
            g_menu_touch = (value != 0);
        } else if (strcmp(line, "linksoundbtn") == 0) {
            g_link_sound_button = (uint8_t)(value >= 0 && value <= 4 ? value : 1);
        } else if (strcmp(line, "dateformat") == 0) {
            g_date_format = (uint8_t)(value >= 0 && value <= 2 ? value : 0);
        } else if (strcmp(line, "records") == 0) {
            g_records_enabled = (value != 0);
        } else if (strcmp(line, "recordhud") == 0) {
            g_record_hud = (uint8_t)(value <= 2 ? value : 1);
        } else if (strcmp(line, "recordslist") == 0) {
            g_records_in_list = (value != 0);
        } else if (strcmp(line, "achnotify") == 0) {
            g_ach_notify = (uint8_t)(value <= 2 ? value : 0);
        } else if (strcmp(line, "alertcolor") == 0) {
            g_alert_color = (uint8_t)(value == 1 ? 1 : 0);
        } else if (strcmp(line, "enable_log") == 0) {
            g_log_file_enabled = (value != 0);
        } else if (strcmp(line, "lastgameplayed") == 0) {
            // A ref string, not a number - reads the raw text after '='
            // instead of the numeric `value` computed above.
            std::string ref(eq + 1);
            while (!ref.empty() && (ref.back() == '\n' || ref.back() == '\r' || ref.back() == ' ')) {
                ref.pop_back();
            }
            g_last_game_played_ref = ref;
        } else if (strcmp(line, "experimentalgames") == 0 || strcmp(line, "betagames") == 0) {
            // "betagames" is the pre-rename key (the old "Tiger / David and
            // John games (Beta)" toggle, which this setting replaced) -
            // still read so an existing settings file keeps its choice.
            g_experimental_games_enabled = (value != 0);
        } else if (strcmp(line, "panelgap") == 0) {
            long v = value;
            if (v < 0) v = 0;
            if (v > 30) v = 30;
            g_panel_gap_percent = (uint8_t)v;
        } else if (strcmp(line, "experimentalfeatures") == 0) {
            g_experimental_features_enabled = (value != 0);
        } else if (strcmp(line, "experimental") == 0) {
            // Pre-rename key: the old "Experimental Features" toggle also
            // showed the Konami/Nelsonic/VTech/etc. games that "Experimental
            // Games" now covers, so someone who had it on had opted into
            // exactly those games - carry that over rather than hiding them.
            g_experimental_features_enabled = (value != 0);
            if (value != 0) g_experimental_games_enabled = true;
        } else if (strcmp(line, "debug") == 0) {
            // Hidden/manual-only - never written back, see switch_settings_save().
            g_debug_mode = (value != 0);
        } else if (strcmp(line, "dosimeterreading") == 0) {
            long v = value;
            if (v < 0) v = 0;
            if (v > 15) v = 15;
            g_dosimeter_reading = (uint8_t)v;
        } else if (strcmp(line, "timesetmode") == 0) {
            g_time_set_mode = (value == 1) ? TimeSetMode::Manual : TimeSetMode::Automatic;
        } else if (strcmp(line, "pettime") == 0) {
            if (value == 1) g_pet_time_mode = PetTimeMode::ClockSync;
            else if (value == 2) g_pet_time_mode = PetTimeMode::Paused;
            else g_pet_time_mode = PetTimeMode::RealTime;
        } else if (strcmp(line, "pettimecap") == 0) {
            g_pet_time_cap_hours = (value < 0) ? 72u : (uint32_t)(value > 9999 ? 9999 : value);
        } else if (strcmp(line, "pettimedisplay") == 0) {
            g_pet_time_display = (value >= 0 && value < kNumPetTimeDisplays) ? (PetTimeDisplay)value : PetTimeDisplay::Progress;
        }
    }
    fclose(file);

    if (!menu_rotation_found) g_menu_rotation = g_screen_rotation;  // older file: keep the shared look
    // Force-enabled under debug mode without touching the stored enable_log
    // value itself - see g_debug_mode's doc comment in switch_settings.h.
    debug_log::set_enabled(g_log_file_enabled || g_debug_mode);
    YOKOI_LOG("switch_settings: loaded from '%s' (background_color=0x%06x, segment_marking_alpha=%u, rotation=%u, "
              "hide_console_art=%d, loadstate=%u, enable_log=%d, lastgameplayed='%s', experimentalgames=%d, panelgap=%u, "
              "clonefilter=%u, experimentalfeatures=%d, debug=%d, dosimeterreading=%u, timesetmode=%u)",
              path.c_str(), (unsigned)g_settings.background_color, (unsigned)g_settings.segment_marking_alpha,
              (unsigned)g_screen_rotation, (int)g_hide_console_art, (unsigned)g_load_state_behavior,
              (int)g_log_file_enabled, g_last_game_played_ref.c_str(), (int)g_experimental_games_enabled,
              (unsigned)g_panel_gap_percent, (unsigned)g_clone_filter_mode, (int)g_experimental_features_enabled, (int)g_debug_mode,
              (unsigned)g_dosimeter_reading, (unsigned)g_time_set_mode);
}

void switch_settings_save() {
    if (g_debug_mode) {
        // Hidden debug mode locks the settings file entirely for the rest of
        // this run - see g_debug_mode's doc comment in switch_settings.h.
        return;
    }

    const std::string path = settings_txt_path();
    FILE* file = fopen(path.c_str(), "w");
    if (!file) {
        YOKOI_LOG("switch_settings: save open failed: '%s'", path.c_str());
        return;
    }
    fprintf(file, "background_color=0x%06x\n", (unsigned)g_settings.background_color);
    fprintf(file, "segment_marking_alpha=%u\n", (unsigned)g_settings.segment_marking_alpha);
    fprintf(file, "rotation=%u\n", (unsigned)g_screen_rotation);
    fprintf(file, "menurotation=%u\n", (unsigned)g_menu_rotation);
    fprintf(file, "hide_console_art=%d\n", g_hide_console_art ? 1 : 0);
    fprintf(file, "hide_background_art=%d\n", g_hide_background_art ? 1 : 0);
    fprintf(file, "loadstate=%u\n", (unsigned)g_load_state_behavior);
    fprintf(file, "clonefilter=%u\n", (unsigned)g_clone_filter_mode);
    fprintf(file, "menustyle=%u\n", (unsigned)g_menu_style);
    fprintf(file, "makerorder=%u\n", (unsigned)g_maker_order);
    fprintf(file, "menutheme=%u\n", (unsigned)g_menu_theme);
    fprintf(file, "menufont=%u\n", (unsigned)g_menu_font);
    fprintf(file, "ghosting=%u\n", (unsigned)g_ghosting);
    fprintf(file, "flickerfilter=%u\n", (unsigned)g_flicker_filter);
    fprintf(file, "pixelstyle=%u\n", (unsigned)g_pixel_style);
    fprintf(file, "screenfilter=%u\n", (unsigned)g_screen_filter);
    fprintf(file, "filterstrength=%u\n", (unsigned)g_filter_strength);
    fprintf(file, "enable_log=%d\n", g_log_file_enabled ? 1 : 0);
    fprintf(file, "splash=%d\n", g_splash_screen ? 1 : 0);
    fprintf(file, "makercolors=%d\n", g_maker_colors ? 1 : 0);
    fprintf(file, "menutouch=%d\n", g_menu_touch ? 1 : 0);
    fprintf(file, "linksoundbtn=%u\n", (unsigned)g_link_sound_button);
    fprintf(file, "dateformat=%u\n", (unsigned)g_date_format);
    fprintf(file, "records=%d\n", g_records_enabled ? 1 : 0);
    fprintf(file, "recordhud=%u\n", (unsigned)g_record_hud);
    fprintf(file, "recordslist=%d\n", g_records_in_list ? 1 : 0);
    fprintf(file, "achnotify=%u\n", (unsigned)g_ach_notify);
    fprintf(file, "alertcolor=%u\n", (unsigned)g_alert_color);
    fprintf(file, "language=%u\n", (unsigned)g_language);
    fprintf(file, "screensize=%u\n", (unsigned)g_screen_size);
    fprintf(file, "screenpalette=%u\n", (unsigned)g_screen_palette);
    fprintf(file, "transition=%u\n", (unsigned)g_transition);
    fprintf(file, "volume=%u\n", (unsigned)g_sound_volume);
    fprintf(file, "mute=%d\n", g_sound_mute ? 1 : 0);
    fprintf(file, "soundstyle=%u\n", (unsigned)g_sound_style);
    fprintf(file, "tgcsound=%u\n", (unsigned)g_tgc_sound);
    fprintf(file, "tgcspeed=%u\n", (unsigned)g_tgc_speed);
    fprintf(file, "tgcnoise=%u\n", (unsigned)g_tgc_noise);
    fprintf(file, "tgcpalette=%u\n", (unsigned)g_tgc_palette);
    fprintf(file, "tgcghost=%u\n", (unsigned)g_tgc_ghost);
    fprintf(file, "hgmspeed=%u\n", (unsigned)g_hgm_speed);
    fprintf(file, "rzstyle=%u\n", (unsigned)g_rz_style);
    fprintf(file, "eavscreen=%u\n", (unsigned)g_eav_screen);
    fprintf(file, "tmgfigure=%u\n", (unsigned)g_tmg_figure);
    fprintf(file, "tmgcontacts=%lu\n", (unsigned long)g_tmg_contacts);
    fprintf(file, "menusounds=%d\n", g_menu_sounds ? 1 : 0);
    fprintf(file, "menumusic=%u\n", (unsigned)g_menu_music);
    fprintf(file, "menumusictrack=%s\n", g_menu_music_track.c_str());
    fprintf(file, "menumusicvol=%u\n", (unsigned)g_menu_music_volume);
    fprintf(file, "menumusicduck=%u\n", (unsigned)g_menu_music_duck);
    fprintf(file, "pausecombo=%u\n", (unsigned)g_pause_combo);
    fprintf(file, "onscreenkeypad=%d\n", g_onscreen_keypad ? 1 : 0);
    fprintf(file, "settingshelp=%d\n", g_settings_help ? 1 : 0);
    fprintf(file, "fastforward=%u\n", (unsigned)g_fastforward_button);
    fprintf(file, "ffspeed=%u\n", (unsigned)g_fastforward_speed);
    fprintf(file, "buttonresponse=%u\n", (unsigned)g_button_response);
    fprintf(file, "rewind=%u\n", (unsigned)g_rewind_button);
    fprintf(file, "rewindlength=%u\n", (unsigned)g_rewind_length);
    fprintf(file, "vibration=%u\n", (unsigned)g_vibration_game);
    fprintf(file, "vibrationpets=%d\n", g_vibration_pets ? 1 : 0);
    fprintf(file, "vibrationmenu=%d\n", g_vibration_menu ? 1 : 0);
    fprintf(file, "vibrationstrength=%u\n", (unsigned)g_vibration_strength);
    fprintf(file, "petalertlight=%d\n", g_pet_alert_light ? 1 : 0);
    fprintf(file, "motionpedometer=%d\n", g_motion_pedometer ? 1 : 0);
    fprintf(file, "pedometersens=%u\n", (unsigned)g_pedometer_sensitivity);
    fprintf(file, "motiontap=%d\n", g_motion_tap_sensor ? 1 : 0);
    fprintf(file, "tapsens=%u\n", (unsigned)g_tap_sensitivity);
    fprintf(file, "irsensor=%u\n", (unsigned)g_ir_sensor);
    fprintf(file, "irsens=%u\n", (unsigned)g_ir_sensitivity);
    if (!g_last_game_played_ref.empty()) {
        fprintf(file, "lastgameplayed=%s\n", g_last_game_played_ref.c_str());
    }
    fprintf(file, "experimentalgames=%d\n", g_experimental_games_enabled ? 1 : 0);
    fprintf(file, "panelgap=%u\n", (unsigned)g_panel_gap_percent);
    if (g_segment_color == kSegmentColorAuto) {
        fprintf(file, "segment_color=auto\n");
    } else {
        fprintf(file, "segment_color=0x%06x\n", (unsigned)(g_segment_color & 0xFFFFFFu));
    }
    fprintf(file, "experimentalfeatures=%d\n", g_experimental_features_enabled ? 1 : 0);
    fprintf(file, "dosimeterreading=%u\n", (unsigned)g_dosimeter_reading);
    fprintf(file, "timesetmode=%u\n", (unsigned)g_time_set_mode);
    fprintf(file, "pettime=%u\n", (unsigned)g_pet_time_mode);
    fprintf(file, "pettimecap=%u\n", (unsigned)g_pet_time_cap_hours);
    fprintf(file, "pettimedisplay=%u\n", (unsigned)g_pet_time_display);
    fclose(file);
}

const ScreenPalette kScreenPalettes[] = {
    {"Original", 0, 0},
    {"Classic LCD", 0xB0BA9E, 0x121612},    {"Game Boy", 0x9BBC0F, 0x0F380F},      {"Pocket", 0xC6C6BE, 0x282828},
    {"Game Boy Light", 0x00B5AD, 0x00343E}, {"Nokia", 0xC7F0D8, 0x43523D},         {"Amber", 0x1E0F00, 0xFFAA28},
    {"Green Phosphor", 0x051405, 0x3CFF5A}, {"Backlit", 0x285AC8, 0xE6F0FF},       {"Virtual Boy", 0x000000, 0xFF1414},
    {"Negative", 0x0F0F0F, 0xE1E1E1},       {"Paper", 0xF0EAD6, 0x3C3228},         {"Sepia", 0xE6D2AA, 0x5A3C1E},
    {"Night", 0x0A0F28, 0x78A0FF},          {"Strawberry", 0xFFD2DC, 0xAA143C},    {"Mint", 0xCDF0DC, 0x145A3C},
    {"Synthwave", 0x280A3C, 0xFF50C8},      {"Ocean", 0xBEE1F0, 0x0A3C6E},         {"Gold", 0x281E0A, 0xFFD250},
    {"Digital Watch", 0x96A08C, 0x0A0A0A},
    {"Game.com", 0xB2C0AA, 0x202226},  // the Tiger Game.com LCD, from photos
    {"Game.com Green", 0xDFFF8F, 0x0F4F2F},  // green-tinted (exact five shades in switch_screen.cpp)
};
const int kNumScreenPalettes = (int)(sizeof(kScreenPalettes) / sizeof(kScreenPalettes[0]));
uint8_t g_screen_palette = 0;

uint32_t effective_background_color() {
    if (g_screen_palette > 0 && g_screen_palette < kNumScreenPalettes) return kScreenPalettes[g_screen_palette].bg;
    return g_settings.background_color;
}

uint32_t effective_segment_color(uint32_t background_rgb) {
    if (g_screen_palette > 0 && g_screen_palette < kNumScreenPalettes) return kScreenPalettes[g_screen_palette].seg;
    if (g_segment_color != kSegmentColorAuto) return g_segment_color & 0xFFFFFFu;
    const float r = ((background_rgb >> 16) & 0xFF) / 255.0f;
    const float g = ((background_rgb >> 8) & 0xFF) / 255.0f;
    const float b = (background_rgb & 0xFF) / 255.0f;
    const float luma = 0.299f * r + 0.587f * g + 0.114f * b;
    // Light backgrounds: 12% of the background colour - the look every game
    // has always had. Dark backgrounds (e.g. the Black preset) would make that
    // invisible, so lighten toward white instead, keeping the hue.
    const float k = 0.12f;
    auto ch = [&](float c) {
        const float out = (luma >= 0.35f) ? c * k : c + (1.0f - c) * (1.0f - k);
        return (uint32_t)(out * 255.0f + 0.5f);
    };
    return (ch(r) << 16) | (ch(g) << 8) | ch(b);
}
