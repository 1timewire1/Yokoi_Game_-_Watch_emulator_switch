#pragma once

#include <cstdint>
#include <string>

// Switch-only settings persistence. Writes a small human-readable text file
// (yokoi_settings.txt, next to the .nro in sdmc:/switch/yokoi) instead of the
// binary yokoi_gw_settings.dat that source/std/settings.cpp's
// load_settings()/save_settings() use for 3DS/Android - requested explicitly
// so the file can be opened/edited by hand. Deliberately bypasses those
// shared functions entirely (they're untouched, so 3DS/Android's on-disk
// format and behavior don't change) while still reusing std/settings.h's
// g_settings.background_color and g_settings.segment_marking_alpha as the
// live values - only persistence differs. Screen rotation has no shared-core
// equivalent, so it lives here instead of in AppSettings.

enum class ScreenRotation : uint8_t { Deg0 = 0, Deg90 = 1, Deg270 = 2 };

extern ScreenRotation g_screen_rotation;

// Hides the decorative "console art" strip (the handheld's plastic-case
// artwork, drawn from GW_rom::console_info - see switch_screen.cpp's
// kRenderConsoleOverlay) so the real game screen can use that space instead.
// Only ever applies where that strip exists in the first place: single-
// screen games, and "double_in_one_screen" games (two real screens shown
// side by side within one combined top region, still with a console strip
// below it) - both go through switch_layout.cpp's !g_split_two_screens_to_panels
// branch. True dual-screen "clamshell" games (g_split_two_screens_to_panels)
// have no console strip at all - their "bottom" is a second real screen, not
// artwork - so this setting can't affect and doesn't need to check for them
// specially. David & John games likewise never set console_info, so they're
// naturally unaffected too. Defaults to hidden, per explicit request.
extern bool g_hide_console_art;

// Governs what happens when A is pressed on a game that already has a save
// file - see the load prompt in switch_menu.cpp. Only concerns *loading*;
// saving on leaving a game is always unconditional regardless of this
// (matching 3DS/Android), except under g_debug_mode, which disables both.
// Only ever shown/changeable from the game list's settings screen.
enum class LoadStateBehavior : uint8_t { Ask = 0, AutoLoad = 1, NeverLoad = 2 };
extern LoadStateBehavior g_load_state_behavior;

// Whether debug_log (debug_log.h) writes to yokoi_log.txt at all. Defaults
// off. Applied via debug_log::set_enabled() both at load and live whenever
// this row changes in the settings menu. Only ever shown/changeable from the
// game list's settings screen.
extern bool g_log_file_enabled;

// Ref (GW_rom::ref, stable across pack reordering - same key savestate.cpp
// uses) of the last game a save was actually written for - see
// switch_unload_current_game() in switch_game_loader.cpp, which sets this
// and calls switch_settings_save() right alongside save_game_state(), and
// switch_get_default_game_index(), which prefers this over the older
// per-manufacturer restoration when it resolves to a real game. Empty until
// the first game is ever saved (or forever, under g_debug_mode).
extern std::string g_last_game_played_ref;

// Shows/hides Tiger and David & John games from the game list, including the
// visible game count - see rebuild_sorted_order() in switch_menu.cpp, which
// filters the navigable index list by manufacturer whenever this (or
// g_debug_mode) is false. Defaults off. Enabling requires confirming a
// beta-support warning in the settings menu first (switch_menu.cpp's
// g_betagames_confirm_active); disabling is instant, no confirmation. Save
// files for beta games are never touched by this - only visibility - though
// disabling does reset g_last_game_played_ref away from one if it was
// pointing at a now-hidden beta game (see the settings-row handler in
// switch_menu.cpp). Only ever shown/changeable from the game list's settings
// screen.
extern bool g_betagames_enabled;

// Hidden, advanced-use-only flag: never shown in the settings menu, never
// added to yokoi_settings.txt automatically, and never written back once
// read - the only way to set it is by hand-editing the file with `debug=1`.
// When true:
//  - switch_settings_save() no-ops entirely, so yokoi_settings.txt can't be
//    overwritten for the rest of this run.
//  - Every settings-menu row except Rotation is locked read-only, showing
//    whatever raw value was actually in the file - including a value outside
//    the normal presets, since loading never snapped values to the nearest
//    preset anyway (see e.g. find_bg_preset_index()'s callers in
//    switch_menu.cpp: it's only used to pick a *starting point* to cycle
//    Left/Right from, never to rewrite the loaded value itself).
//  - Rotation still changes freely in the menu, but - like everything else
//    here - never reaches disk, since switch_settings_save() no-ops outright.
//  - Save states are bypassed completely: no autosave on leaving a game
//    (so g_last_game_played_ref never updates either), and no load prompt
//    (starting a game always loads fresh).
//  - Logging is force-enabled regardless of g_log_file_enabled, WITHOUT
//    changing that stored value - see switch_settings_load()'s debug_log::
//    set_enabled() call. Meant for inspecting a run without also having to
//    edit enable_log (which would itself be blocked from saving anyway).
//  - Beta games (Tiger, David & John) are force-visible the same way, and
//    for the same reason - see rebuild_sorted_order()'s betagames-visible
//    check in switch_menu.cpp, which ORs this in right alongside
//    g_betagames_enabled without touching that stored value either.
extern bool g_debug_mode;

// Loads yokoi_settings.txt if present (applying background_color/
// segment_marking_alpha into g_settings, rotation into g_screen_rotation,
// hide_console_art into g_hide_console_art, loadstate into
// g_load_state_behavior, enable_log into g_log_file_enabled, lastgameplayed
// into g_last_game_played_ref, betagames into g_betagames_enabled, and the
// hidden debug flag into g_debug_mode - which also then force-enables
// debug_log regardless of enable_log); otherwise resets to AppSettings'
// compiled-in defaults, ScreenRotation::Deg0, hidden console art, Ask,
// logging off, no last-played game, beta games off, and debug mode off.
void switch_settings_load();

// Writes the current g_settings fields + g_screen_rotation +
// g_hide_console_art + g_load_state_behavior + g_log_file_enabled +
// g_last_game_played_ref + g_betagames_enabled back out. No-op when
// g_debug_mode is set - see its doc comment above.
void switch_settings_save();
