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

extern ScreenRotation g_screen_rotation;  // the game screen (Settings > Display > Game Rotation)
// The menus (game list, settings, pause menu, splash) - their own setting
// since the game may be played upright while the menu stays landscape, or
// the other way round. Stored as "menurotation"; a settings file without it
// (from before the split) gives the menu the game's rotation.
extern ScreenRotation g_menu_rotation;

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

// Background Art: Shown (default) / Hidden. Hidden drops each game's printed
// artwork - its Background image and its label Overlay - leaving only the
// segments over the plain, Background Color-tinted fill. Masked games
// (Panorama/Table Top: their art IS the lit layer behind dark segments)
// always keep theirs. Purely a rendering switch (render/switch_screen.cpp),
// shown in both settings screens since it applies live. Stored as
// hide_background_art.
extern bool g_hide_background_art;

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

// Game list style (settings "Menu Style"): the manufacturer grid, or the
// original one-game-at-a-time screen. Either way the other is one button
// away (Y / B) - this just picks which one the app opens on.
enum class MenuStyle : uint8_t { Makers = 0, Classic = 1, Carousel = 2, List = 3 };
extern MenuStyle g_menu_style;

// Order of the manufacturer grid (settings "Maker Order"): alphabetical,
// most games first, or by the year of each maker's oldest game.
enum class MakerOrder : uint8_t { AZ = 0, MostGames = 1, Era = 2 };
extern MakerOrder g_maker_order;

// Colour theme of the menus (settings "Themes"): an index into the theme
// table in switch_menu.cpp (0 = Dark, the original Hekate-style look).
extern uint8_t g_menu_theme;

// Menu font (settings 'Menu Font'): 0 = system font, 1.. = bundled fonts
// (switch_ui_set_font).
extern uint8_t g_menu_font;

// LCD ghosting (settings "LCD Ghosting"): 0 = Off (segments switch
// instantly, the original look), 1-3 = Low/Medium/High - segments fade out
// (and, more gently, in) over a few frames like a real LCD's slow response.
// See switch_screen.cpp's segment pass.
extern uint8_t g_ghosting;

// Anti-flicker filter (core/switch_segments_state.cpp), Settings > Screen and
// the pause menu: 0 Auto (on for the Game & Watch CPUs, off for the
// E0C6S46/S48 - Tamagotchi, Digimon), 1 On, 2 Off. A dot is only shown once
// it's lit on two pictures in a row: hides multiplexing flicker, but erases
// 1-dot strokes on screens that scroll a dot at a time. Stored as
// flickerfilter.
extern uint8_t g_flicker_filter;

// Settings > Screen > Pixel Style: how the square dots of dot-matrix screens
// (Brick Game, Tetris handhelds...) are drawn - 0 filled squares (as the
// art has them), 1 a frame with a square in the middle, like the real LCD.
// Applies to any game with a grid of same-size square segments
// (switch_screen.cpp). Stored as "pixelstyle".
extern uint8_t g_pixel_style;

// Screen filter (settings "Filters"): drawn over each game screen after the
// segments (never over the console art). 0 = None; see kScreenFilterNames
// in switch_screen.cpp for the list. g_filter_strength: 0-2 = Low/Medium/High.
extern uint8_t g_screen_filter;
extern uint8_t g_filter_strength;

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

// "Pet Time" - what happens to a virtual pet (the E0C6S46/S48 ones whose
// clock layout is known - core/e0c6200_clock.h - and the BrickEmuPy-core
// ones marked "clock.pet", like the Apollo 2 in 1) for the time that
// passed while it wasn't running: the app closed (measured from its save
// state's timestamp) or the game paused/suspended mid-play (HOME, console
// sleep, the in-game settings screen). See core/pet_time.h.
//  - RealTime (default): fast-forward the pet through that time (up to
//    g_pet_time_cap_hours), so it really aged/got hungry, then set its clock.
//  - ClockSync: only set its clock to the real time - it doesn't age.
//  - Paused: neither - it picks up exactly where it left off.
enum class PetTimeMode : uint8_t { RealTime = 0, ClockSync = 1, Paused = 2 };
extern PetTimeMode g_pet_time_mode;

// Real Time's catch-up limit in hours (0 = no limit). Any time beyond it is
// skipped like Clock Sync (the clock jumps, the pet doesn't age through it).
// Presets 8/24/72/168 or Unlimited, or a custom 1-9999 (switch_menu.cpp).
extern uint32_t g_pet_time_cap_hours;

// How Real Time's catch-up looks: a progress screen, or the pet's own LCD
// running at high speed with a small remaining-time readout.
// Also Clock (a clock face spinning through the time), Day & Night (the sun
// and moon crossing the sky) and Screen & Bar (the pet's own LCD running,
// with the bar over it) - core/pet_time.cpp's pet_time_render().
enum class PetTimeDisplay : uint8_t { Progress = 0, FastForward = 1, Clock = 2, DayNight = 3, ScreenBar = 4 };
constexpr int kNumPetTimeDisplays = 5;
extern PetTimeDisplay g_pet_time_display;

// Whether debug_log (debug_log.h) writes to yokoi_log.txt at all. Defaults
// off. Applied via debug_log::set_enabled() both at load and live whenever
// this row changes in the settings menu. Only ever shown/changeable from the
// game list's settings screen.
extern bool g_log_file_enabled;

// Startup splash animation (render/switch_splash.h), Settings > Advanced >
// Splash Screen. Defaults on. Stored as "splash".
extern bool g_splash_screen;

// A colour per maker in the maker menu (grid: a stripe on the left of each
// tile; carousel: a band on top of each card), Settings > Menu > Maker
// Colors. Defaults on. Stored as "makercolors".
extern bool g_maker_colors;

// Touch control of the menus (input/menu_touch.h), Settings > Menu > Touch
// Controls. Handheld only. Defaults on. Stored as "menutouch".
extern bool g_menu_touch;

// Two linked devices (core/link_session.h): the button that switches which
// one you hear, pause menu > Game Session > Sound Switch Button. Same
// choices as the fast-forward/rewind buttons: 0 Off, 1 ZL, 2 ZR, 3 L3,
// 4 R3. Defaults to ZL. Stored as "linksoundbtn".
extern uint8_t g_link_sound_button;

// How the menus write a game's date, Settings > Menu > Date Format:
// 0 YYYY-MM-DD (as stored), 1 DD/MM/YYYY, 2 MM/DD/YYYY. Stored as
// "dateformat".
extern uint8_t g_date_format;

// High scores (core/score_tracker.h), Settings > Game List: on/off for all
// of it ("records", on by default); what shows over the game - 0 nothing,
// 1 the "new record" banner, 2 the record too ("recordhud", 1); and the
// records box in the game list ("recordslist", on).
extern bool g_records_enabled;
extern uint8_t g_record_hud;
extern bool g_records_in_list;
// Achievements (core/score_tracker.h): when their banner shows - 0 during
// the game, 1 when the game ends, 2 never ("achnotify", 0).
extern uint8_t g_ach_notify;
extern uint8_t g_alert_color;  // record / achievement alerts: 0 Gold, 1 the menu theme's colours

// App language (render/switch_i18n.h): 0 English, 1 Spanish. Settings >
// Display > Language. Stored as "language".
extern uint8_t g_language;

// Screen Size (Settings > Display, also the pause menu's Screen section):
// how big the game is drawn. 0 Small (70%), 1 Medium (85%), 2 Normal (100%,
// the largest undistorted fit - the default), 3 Large (115%, the edges of
// the art go off screen), 4 Stretched (fills 16:9, distorts), 5 Pixel
// Exact (a whole multiple - or whole fraction - of the art's own size).
// Stored as "screensize".
constexpr uint8_t kNumScreenSizes = 6;
extern uint8_t g_screen_size;

// Settings > Game List > Transition (render/switch_transition.h): how the
// one-at-a-time game view and the Carousel move from one item to the next.
// Stored as "transition".
extern uint8_t g_transition;

// Settings > Sound (also Volume/Mute in the mid-game list). Volume is a
// percentage (0-100, steps of 10) applied to every voice - the game tone,
// McDonald's samples, step sounds, menu beeps and the splash; Mute silences
// all of them without losing the level. Sound Style (switch_sound.cpp's
// push_output_sample()) only shapes the emulated buzzer: 0 Clean, 1 Piezo,
// 2 Octave Down, 3 Flute, 4 Radio. Menu Sounds = menu_sound.h's UI beeps.
// Stored as volume / mute / soundstyle / menusounds.
constexpr uint8_t kNumSoundStyles = 5;
extern uint8_t g_sound_volume;
extern bool g_sound_mute;
extern uint8_t g_sound_style;
// Tiger Game.com / Hartung Game Master options (pause menu > Sound, only in
// those games). 0 = as MAME, the default.
extern uint8_t g_tgc_sound;    // 0 Full (DAC + tones), 1 Basic (one-bit buzzer)
extern uint8_t g_tgc_speed;    // 0 Normal, 1 +20 %
extern uint8_t g_tgc_noise;    // 0 Off, 1 On (approximate noise channel)
extern uint8_t g_tgc_palette;  // 0 Real (greys from photos), 1 MAME (green)
extern uint8_t g_tgc_ghost;    // LCD trails: 0 Off, 1 Light, 2 Strong (as the real slow LCD)
extern uint8_t g_tmg_figure;
// Its contacts, per figure: 2 bits each (bit 0 CD1, bit 1 CD2), figure n at bits 2(n-1); default all CD1+CD2.
extern uint32_t g_tmg_contacts;
inline uint8_t tmg_contacts_of(int n) { return n < 1 ? 0 : (uint8_t)((g_tmg_contacts >> (2 * (n - 1))) & 3); }   // TamaTown Tama-Go: the Gotchi figure inserted, 0 none, 1.. the pack's figures
extern uint8_t g_eav_screen;   // Adventure Vision screen: 0 LEDs (rows apart, as the real one), 1 Smooth
extern uint8_t g_rz_style;     // Tiger R-Zone screen: 0 White (crimson, near-white lit), 1 Red (dark, red lit), 2 LCD (no colours)
extern uint8_t g_hgm_speed;    // 0 Normal (MAME), 1 Fast (x2, as a real unit on video)
extern bool g_menu_sounds;

// Settings > Menu > Music (audio/menu_music.h). Background Music: 0 Off,
// 1-8 the built-in tunes, 9 Random, 10 Custom (files from the SD's
// yokoi_music/ folder). Track (Custom only): a file name, or empty for
// "All (random)". Volume 0-100 in steps of 10. Lower With Beeps: 0 Not at
// all, 1 A little, 2 Medium, 3 A lot. Stored as menumusic / menumusictrack
// / menumusicvol / menumusicduck.
extern uint8_t g_menu_music;
extern std::string g_menu_music_track;
extern uint8_t g_menu_music_volume;
extern uint8_t g_menu_music_duck;

// Settings > Menu > Open Pause Menu (input/control_remap.h): 0 ZL + ZR,
// 1 L3 + R3, 2 L3, 3 R3. Those buttons can't be given to a game's
// functions in pause menu > Controls. Stored as "pausecombo".
extern uint8_t g_pause_combo;

// Pause menu > Screen > On-Screen Keypad (Experimental Features only, and
// only for the keypad games - see input/onscreen_keypad.h). Stored as
// "onscreenkeypad".
extern bool g_onscreen_keypad;

// The explanation box under the settings rows: Y shows / hides it in any
// settings or pause screen (not Controls, where Y resets the buttons).
// Stored as "settingshelp".
extern bool g_settings_help;

// Fast-forward / rewind (input/time_control.h), Settings > Saves & Clock:
// buttons 0 Off, 1 ZL, 2 ZR, 3 L3, 4 R3; speed 0 2x, 1 4x, 2 8x; rewind
// length 0 5 s, 1 10 s, 2 30 s, 3 60 s. Stored as fastforward / ffspeed / rewind /
// rewindlength.
extern uint8_t g_fastforward_button;
extern uint8_t g_fastforward_speed;
extern uint8_t g_rewind_button;
extern uint8_t g_rewind_length;

// Button Response (pause menu > Game, only on games with the M37520 chip -
// Pocket Pikachu): 0 Original (x1), 1 Fast (x2), 2 Very Fast (x4), 3 Instant
// (x8) - M37520::sub_clock_boost. Defaults to Very Fast. Stored as
// buttonresponse.
extern uint8_t g_button_response;

// Vibration (input/switch_rumble.h), Settings > Sound. Game: 0 Off, 1 With
// Sound, 2 Strong Events. Strength: 0 Soft, 1 Medium, 2 Strong. Stored as
// vibration / vibrationpets / vibrationmenu / vibrationstrength.
extern uint8_t g_vibration_game;
extern bool g_vibration_pets;
extern bool g_vibration_menu;
extern uint8_t g_vibration_strength;

// Settings > Controller Features (input/pet_led.h, input/motion_steps.h):
// Pet Alert Light (the HOME light pulses when a pet calls), Pedometer:
// Shake the Controller, and its sensitivity (0 Low, 1 Medium, 2 High).
// Stored as "petalertlight" / "motionpedometer" / "pedometersens".
extern bool g_pet_alert_light;
extern bool g_motion_pedometer;
extern uint8_t g_pedometer_sensitivity;
// ...and Tap Sensor (input/motion_steps.h), stored as "motiontap".
extern bool g_motion_tap_sensor;
// Its sensitivity: 0 Low, 1 Medium, 2 High. Stored as "tapsens".
extern uint8_t g_tap_sensitivity;
// ...and IR Sensor (input/ir_sensor.h): 0 Off, 1 Auto (the camera runs only
// while the game listens to its sensor, up to a minute without a press),
// 2 Always (while the game is played); with its sensitivity (0 Low, 1
// Medium, 2 High). Stored as "irsensor" / "irsens".
extern uint8_t g_ir_sensor;
extern uint8_t g_ir_sensitivity;

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

// "Screen Palette" (Settings > Screen, also the pause menu): a ready-made
// background + lit-segment colour pair for the game screen. 0 = Original
// (Background Color / Segment Color as set). Any other palette takes over
// both - and tints the printed background art with its background colour.
// Panorama/mask games and the few with per-segment colours keep theirs.
// Stored as "screenpalette".
struct ScreenPalette {
    const char* name;
    uint32_t bg, seg;
};
extern const ScreenPalette kScreenPalettes[];
extern const int kNumScreenPalettes;
extern uint8_t g_screen_palette;
// The game background colour to draw with (the palette's, or Background Color).
uint32_t effective_background_color();

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
