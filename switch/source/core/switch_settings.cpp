#include "switch_settings.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "std/debug_log.h"
#include "std/platform_paths.h"
#include "std/settings.h"

ScreenRotation g_screen_rotation = ScreenRotation::Deg0;
bool g_hide_console_art = true;
bool g_hide_background_art = false;
LoadStateBehavior g_load_state_behavior = LoadStateBehavior::Ask;
CloneFilterMode g_clone_filter_mode = CloneFilterMode::ShowAll;
bool g_log_file_enabled = false;
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
    return storage_path("yokoi_settings.txt");
}

} // namespace

void switch_settings_load() {
    reset_settings_to_default();
    g_screen_rotation = ScreenRotation::Deg0;
    g_hide_console_art = true;
    g_hide_background_art = false;
    g_load_state_behavior = LoadStateBehavior::Ask;
    g_clone_filter_mode = CloneFilterMode::ShowAll;
    g_log_file_enabled = false;
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

    char line[128];
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
            g_pet_time_display = (value == 1) ? PetTimeDisplay::FastForward : PetTimeDisplay::Progress;
        }
    }
    fclose(file);

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
    fprintf(file, "hide_console_art=%d\n", g_hide_console_art ? 1 : 0);
    fprintf(file, "hide_background_art=%d\n", g_hide_background_art ? 1 : 0);
    fprintf(file, "loadstate=%u\n", (unsigned)g_load_state_behavior);
    fprintf(file, "clonefilter=%u\n", (unsigned)g_clone_filter_mode);
    fprintf(file, "enable_log=%d\n", g_log_file_enabled ? 1 : 0);
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

uint32_t effective_segment_color(uint32_t background_rgb) {
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
