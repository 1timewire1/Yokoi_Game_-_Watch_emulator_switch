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

// Filters near-duplicate / clone entries out of the game list. Three tiers:
//  - ShowAll: no filtering (default - matches existing behavior).
//  - HideDuplicates: hides only the small, explicitly-curated set of entries
//    that are genuinely the *same* release re-cataloged twice (see
//    kKnownDuplicateRefs in switch_menu.cpp) - e.g. CN-07 vs CN-17 Helmet,
//    where GNW_LIST.md's own comments explain both are kept only because of
//    a minor behavioral difference between the two hardware revisions.
//  - HideAllClones: hides every entry GNW_LIST.md's "Clone Of" column marks
//    as a clone of another entry (see kKnownCloneRefs in switch_menu.cpp) -
//    a much larger set that also includes plenty of legitimately distinct,
//    differently-titled games (e.g. the Elektronika bootlegs) that just
//    happen to reuse another game's hardware/ports in this build pipeline -
//    not just the handful of true duplicates HideDuplicates targets.
// Both ref lists are hand-derived from GNW_LIST.md's "Clone Of" column as of
// when this was written and are NOT regenerated automatically - if that
// column changes meaningfully, the lists in switch_menu.cpp need a manual
// pass to stay accurate. Only ever shown/changeable from the game list's
// settings screen (this filters that same list - meaningless mid-gameplay).
enum class CloneFilterMode : uint8_t { ShowAll = 0, HideDuplicates = 1, HideAllClones = 2 };
extern CloneFilterMode g_clone_filter_mode;

// Governs whether games with a real-time clock that blocks Game A/B (or, for
// UCHITARI/VESARIF, the whole UI) until a time is set and confirmed get that
// confirmation automated on load - see auto_confirm_press.h and
// uchitari_clock_setup.h/vesarif_clock_setup.h. Automatic (the default)
// auto-presses the needed confirm button (or, for UCHITARI/VESARIF, plays
// out their full scripted set-time sequence) so the game is immediately
// playable; Manual leaves every affected game exactly as un-automated
// hardware behaves - clock still set correctly via time_addresses.h where
// that applies, but nothing pressed on the player's behalf, so UCHITARI/
// VESARIF show their default 00:00 boot clock and every game normally
// blocked on a confirm press stays on that screen until the player presses
// it themselves. Read once per game load (switch_game_loader.cpp) - like
// Console Art, changing this mid-game has no effect until that game is next
// (re)loaded. Ungated (no Experimental Features gate) since it's proven
// behavior, not experimental - but only ever shown on the game list's
// settings screen, never the in-game one (g_settings_from_gameplay), since
// it's meaningless once a game has already finished loading.
enum class TimeSetMode : uint8_t { Automatic = 0, Manual = 1 };
extern TimeSetMode g_time_set_mode;

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

// "Experimental Games" - shows/hides every manufacturer except Nintendo,
// Tronica and Elektronika (Tiger, Konami, Nelsonic, V-Tech, David & John,
// McDonald's, Burger Bard) from the game list, including the visible game
// count - see rebuild_sorted_order() and is_experimental_games_manufacturer()
// in switch_menu.cpp, which filter the navigable index list by manufacturer
// whenever this (or g_debug_mode) is false. Defaults off. Enabling requires
// confirming an "experimental and largely untested" warning in the settings
// menu first (switch_menu.cpp's g_experimental_games_confirm_active);
// disabling is instant, no confirmation. Save files for these games are never
// touched by this - only visibility - though disabling does reset
// g_last_game_played_ref away from one if it was pointing at a now-hidden
// game (see the settings-row handler in switch_menu.cpp). Only ever
// shown/changeable from the game list's settings screen. Stored as
// "experimentalgames" (older files used "betagames" for the setting this
// replaced - still read).
extern bool g_experimental_games_enabled;

// Percentage (of the top panel's own height) of empty space inserted below
// it - between a true dual-screen game's two real panels, or between a
// single-screen game and its console-art strip (only when Console Art is
// Shown - see g_hide_console_art above). Real Game & Watch hardware almost
// always had a visible physical gap here; the renderer used to always butt
// the two flush together with zero gap. Applied in
// rebuild_layout_from_game() (switch_layout.cpp), which skips the gap
// entirely whenever there's nothing below to space away from (e.g. a
// single-screen game with Console Art hidden) - no separate check needed
// elsewhere for that case. Unlike Console Art (which only takes effect on a
// game's next (re)load, since nothing re-derives its layout otherwise), this
// row's Left/Right handler and its custom-entry keyboard both explicitly
// call rebuild_layout_from_game() again right after changing the value, so
// it applies immediately - shown and adjustable from both the game list's
// settings screen and the in-game one, same as Rotation/Background/Segments.
// Defaults to 0 (matches the old always-touching behavior) so existing
// installs see no visual change until this is explicitly set. The settings
// menu only ever writes 0-30 here (3 presets + a custom-entry keyboard
// clamped to that range).
extern uint8_t g_panel_gap_percent;

// "Segment Color" - the colour lit LCD segments are drawn in (0xRRGGBB), or
// kSegmentColorAuto (the default) for the long-standing look: a dark tint of
// the game background colour (lightened instead on dark backgrounds, so they
// never vanish). Lets e.g. Tiger R-Zone games be shown red-on-black like the
// real HeadGear screen. Switch-only (stored as "segment_color" in
// yokoi_settings.txt, "auto" or 0xRRGGBB). Has no effect on the panorama/mask
// games, whose segments carry their own printed colours, or on the two games
// with per-segment colours.
constexpr uint32_t kSegmentColorAuto = 0xFFFFFFFFu;
extern uint32_t g_segment_color;

// The segment colour to actually draw with for a given background colour -
// resolves kSegmentColorAuto. Shared by the renderer and the settings menu.
uint32_t effective_segment_color(uint32_t background_rgb);

// "Experimental Features" - gates FEATURES only, never games (games are
// g_experimental_games_enabled above): today the Altair dosimeter row (see
// g_dosimeter_reading) and the ZL/ZR calculator-keypad layer for Space
// Adventure / Shuttle Voyage / Thief in Garden (main.cpp,
// input/modifier_layer_input.h). Same confirm-before-enabling flow, with
// its own warning text (switch_menu.cpp's
// g_experimental_features_confirm_active); disabling is instant. Stored as
// "experimentalfeatures" (older files used "experimental", which also
// covered games - when read from an old file with a value of 1 it turns
// g_experimental_games_enabled on as well, see switch_settings_load()).
extern bool g_experimental_features_enabled;

// Simulated reading (0-15) fed continuously into Altair's (GW_rom::ref
// "naltair") 4-bit "Dosimeter Reading" dial input every frame that game is
// loaded - see the NALTAIR Virtual_Input class in virtual_input.h and its
// feeding logic in switch/source/main.cpp. Real hardware read this from an
// attached Geiger tube (SBM-20-1, per Nauchpribor's mod of the Elektronika
// IM-22 board); MAME itself doesn't simulate one either, exposing the same
// raw 4-bit value as a manual dial (IPT_DIAL) instead - this is that same
// idea, adjustable from the in-game settings screen while Altair is loaded.
// Only ever shown/changeable there (g_settings_from_gameplay), and only
// while Altair itself is the loaded game and Experimental Features is on -
// see build_settings_rows() in switch_menu.cpp. Defaults to 0.
//
// Known gap (2026-09-16): no confirmed way to see this value do anything
// in-game yet. Per MAME's driver comment, "Dosimeter Mode" is entered by
// pressing the same physical line as P1 Right Stick Up while the unit is
// showing Time - tried exactly that (dial=15, Time, then that button) in
// real MAME itself (not just this port) and got no dosimeter display at
// all; holding the button just resumed normal gameplay instead. This
// driver is flagged MACHINE_IMPERFECT_CONTROLS in MAME, so the real
// trigger condition may not even be fully known upstream. The engine-level
// wiring here is still correct (verified byte-for-byte against MAME's own
// raw dial input) - what's missing is the ROM-side trigger condition and
// reading-format logic, which would need an actual disassembly of
// dbgb-06i.bin to pin down. Not being chased for now; the setting stays
// since it's harmless and gated behind Experimental Features regardless.
extern uint8_t g_dosimeter_reading;

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
//  - Experimental games are force-visible the same way, and for the same
//    reason - see rebuild_sorted_order()'s experimental_games_visible()
//    check in switch_menu.cpp, which ORs this in right alongside
//    g_experimental_games_enabled without touching the stored value.
extern bool g_debug_mode;

// Loads yokoi_settings.txt if present (applying background_color/
// segment_marking_alpha into g_settings, rotation into g_screen_rotation,
// hide_console_art into g_hide_console_art, loadstate into
// g_load_state_behavior, enable_log into g_log_file_enabled, lastgameplayed
// into g_last_game_played_ref, experimentalgames into g_experimental_games_enabled, panelgap
// into g_panel_gap_percent, clonefilter into g_clone_filter_mode, timesetmode
// into g_time_set_mode, and the hidden debug flag into g_debug_mode - which
// also then force-enables debug_log regardless of enable_log); otherwise
// resets to AppSettings' compiled-in defaults, ScreenRotation::Deg0, hidden
// console art, Ask, logging off, no last-played game, experimental games off, no
// panel gap, clone filter off (show all), time set mode Automatic, and
// debug mode off.
void switch_settings_load();

// Writes the current g_settings fields + g_screen_rotation +
// g_hide_console_art + g_load_state_behavior + g_log_file_enabled +
// g_last_game_played_ref + g_experimental_games_enabled + g_dosimeter_reading +
// g_time_set_mode back out. No-op when g_debug_mode is set - see its doc
// comment above.
void switch_settings_save();
