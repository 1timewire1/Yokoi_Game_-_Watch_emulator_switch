// Yokoi Switch port - main loop.
//
// Single-threaded, modeled on source/main.cpp (3DS) rather than Android's
// separate-emulation-thread design: poll input -> step CPU the right number
// of times for this frame -> render -> swap. Starts in switch_menu.cpp's
// game/manufacturer select menu; A loads and starts a game, B pauses back
// out to the menu.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <switch.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/glad.h>

#include "std/GW_ROM.h"
#include "std/debug_log.h"
#include "std/gw_pack.h"
#include "std/load_file.h"
#include "std/platform_paths.h"
#include "std/settings.h"

#include "SM5XX/SM5XX.h"

#define get_input_config get_input_config__switch_main_tu
#include "virtual_i_o/virtual_input.h"
#undef get_input_config

#include "audio/switch_sound.h"
#include "core/yokoi_cpu_utils.h"
#include "core/switch_game_loader.h"
#include "core/switch_menu.h"
#include "core/switch_runtime_state.h"
#include "core/switch_segments_state.h"
#include "core/switch_settings.h"
#include "input/switch_input.h"
#include "input/yokoi_controller_state.h"
#include "input/yokoi_input_mapping.h"
#include "input/modifier_layer_input.h"
#include "input/uchitari_clock_setup.h"
#include "input/vesarif_clock_setup.h"
#include "input/auto_confirm_press.h"
#include "input/lcdonald_input_map.h"
#include "input/rzone_input_map.h"
#include "input/e0c6200_input_map.h"
#include "input/e0c6200_clock_setup.h"
#include "core/pet_time.h"
#include "E0C6200/E0C6S46.h"
#include "std/savestate.h"
#include "core/high_scores.h"
#include "virtual_i_o/rzone_io.h"
#include "audio/lcdonald_sound.h"
#include "render/lcdonald_hud.h"
#include "LCDonald/LCDonald_fake_cpu.h"
#include "render/switch_screen.h"

namespace {

EGLDisplay s_display = nullptr;
EGLContext s_context = nullptr;
EGLSurface s_surface = nullptr;

bool initEgl(NWindow* win, int fixed_w, int fixed_h) {
    // Must be set before eglCreateWindowSurface() - see the comment below on
    // why the actual surface size comes from here, not from EGL.
    nwindowSetDimensions(win, fixed_w, fixed_h);

    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!s_display) {
        return false;
    }

    eglInitialize(s_display, nullptr, nullptr);

    if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE) {
        eglTerminate(s_display);
        s_display = nullptr;
        return false;
    }

    static const EGLint framebufferAttributeList[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE,     8,
        EGL_GREEN_SIZE,   8,
        EGL_BLUE_SIZE,    8,
        EGL_ALPHA_SIZE,   8,
        EGL_DEPTH_SIZE,   0,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs = 0;
    eglChooseConfig(s_display, framebufferAttributeList, &config, 1, &numConfigs);
    if (numConfigs == 0) {
        eglTerminate(s_display);
        s_display = nullptr;
        return false;
    }

    s_surface = eglCreateWindowSurface(s_display, config, win, nullptr);
    if (!s_surface) {
        eglTerminate(s_display);
        s_display = nullptr;
        return false;
    }

    static const EGLint contextAttributeList[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, contextAttributeList);
    if (!s_context) {
        eglDestroySurface(s_display, s_surface);
        s_surface = nullptr;
        eglTerminate(s_display);
        s_display = nullptr;
        return false;
    }

    eglMakeCurrent(s_display, s_surface, s_surface, s_context);

    // eglQuerySurface(EGL_WIDTH/EGL_HEIGHT) reports 0x0 on Switch's Mesa/nouveau
    // EGL backend for NWindow-backed surfaces (confirmed on hardware - not just
    // an unlikely edge case). None of devkitPro's own OpenGL examples query it
    // either; they get the size from nwindowSetDimensions() instead (see
    // examples/switch/graphics/opengl/dynamic_resolution), which is what
    // out_w/out_h are set from below, before this function is even called.
    YOKOI_LOG("main: EGL surface created (dimensions come from nwindowSetDimensions, not EGL)");
    return true;
}

void deinitEgl() {
    if (!s_display) {
        return;
    }
    eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (s_context) {
        eglDestroyContext(s_display, s_context);
        s_context = nullptr;
    }
    if (s_surface) {
        eglDestroySurface(s_display, s_surface);
        s_surface = nullptr;
    }
    eglTerminate(s_display);
    s_display = nullptr;
}

void run_console_error_loop(const char* msg) {
    consoleInit(nullptr);
    printf("%s\n\nPress + to exit.\n", msg);
    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    while (appletMainLoop()) {
        padUpdate(&pad);
        if (padGetButtonsDown(&pad) & HidNpadButton_Plus) {
            break;
        }
        consoleUpdate(nullptr);
    }
    consoleExit(nullptr);
}

bool load_pack() {
    // No dedicated "switch" CONVERT_ROM target profile exists yet (see
    // switch/README.md) - the Switch build deliberately targets the same
    // rgds pack Android uses, same filename Android's nativeInit() hardcodes,
    // so a pack generated for Android/RGDS works here unmodified.
    std::string err;
    if (gw_pack::load(storage_path("yokoi_pack_rgds.ykp"), &err)) {
        return true;
    }
    printf("ROM pack load failed: %s\n", err.c_str());
    return false;
}

// NUMMUNCH (VTech Electronic Number Muncher, SM511) - see
// modifier_layer_input.h's doc comment for the general scheme. 26 physical
// buttons, (group,line) targets read straight off MAME's own hh_sm510.cpp
// INPUT_PORTS_START(nummunch) (group = IN.N port index, line = bit
// position - confirmed against an existing shipped SM511 game's
// Virtual_Input class, e.g. KDRIBBLE, rather than assumed).
// Base layer: primary functions + in-game Left/Right, matching the
// console's own top-to-bottom button groupings. ZL layer: digits 0-9 in the
// device's own numpad order. ZR layer: the numeric keypad's own +-x/ operators
// (distinct from the "Choose" +-x/ buttons already on the base layer).
constexpr ModLayerTarget kNummunchLayout[36] = {
    // base: D-Up=Game D-Down=Quiz D-Left=Left D-Right=Right A=Count B="=" X=Calc/Clear Y=OFF L=Choose+ R=Choose- +=Choosex -=Choose/
    {2,0}, {2,2}, {4,2}, {4,3}, {2,1}, {1,2}, {1,1}, {1,0}, {3,0}, {3,1}, {3,2}, {3,3},
    // ZL (digits): D-Up=1 D-Down=2 D-Left=3 D-Right=4 A=5 B=6 X=7 Y=8 L=9 R=0
    {4,0}, {5,3}, {5,2}, {5,1}, {5,0}, {6,3}, {6,2}, {6,1}, {6,0}, {4,1}, {-1,-1}, {-1,-1},
    // ZR (numpad operators): D-Up=+ D-Down=- D-Left=x D-Right=/
    {7,0}, {7,1}, {7,2}, {7,3}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1},
};

// UCHITARI (Elektronika "Uchitel' arifmetiki", SM511). 23 physical buttons -
// three pairs are wired to the SAME line on real hardware (MAME's own
// PORT_CONDITION-gated "FAKE" port bits): "0"/Minute share (2,0), "1"/Hour
// share (3,2), Alarm/Exam Level share (1,2) - each pair placed on two slots
// of the SAME layer below, relying on apply_modifier_layer_input()'s OR
// logic rather than a coincidence.
constexpr ModLayerTarget kUchitariLayout[36] = {
    // base: D-Up=Enter D-Down=StartExam D-Left=Minute D-Right=Hour A=Mode B=Clear X=Alarm Y=ExamLevel L=E
    {0,0}, {0,2}, {2,0}, {3,2}, {1,1}, {2,1}, {1,2}, {1,2}, {1,0}, {-1,-1}, {-1,-1}, {-1,-1},
    // ZL (digits): D-Up=1 D-Down=2 D-Left=3 D-Right=4 A=5 B=6 X=7 Y=8 L=9 R=0
    {3,2}, {4,2}, {5,2}, {6,2}, {2,2}, {3,0}, {4,0}, {5,0}, {6,0}, {2,0}, {-1,-1}, {-1,-1},
    // ZR (operators): D-Up=+ D-Down=- D-Left=x D-Right=/
    {3,1}, {4,1}, {5,1}, {6,1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1},
};

// SHUTTLE_VOYAGE (MG_8) / THIEF_IN_GARDEN (TG_18) - real Tronica calculator
// keypad neither has ever had mapped, gated behind Experimental Features
// (see apply_calculator_modifier_input()'s doc comment). Both share the
// exact same input ports in MAME (TG_18 is confirmed to reuse trshutvoy's
// INPUT_PORTS_START verbatim - same MCU, different graphics), hence one
// table for both refs. ZL = digits 0-9 in the same positions the
// calculator-only games above use; ZR = the remaining 9 keys (decimal
// point, =, the four math operators, ALM, %, C).
constexpr ModLayerTarget kShuttleVoyageCalcLayout[24] = {
    // ZL (digits): D-Up=1 D-Down=2 D-Left=3 D-Right=4 A=5 B=6 X=7 Y=8 L=9 R=0
    {0,1}, {0,2}, {0,3}, {1,0}, {1,1}, {1,2}, {1,3}, {2,0}, {2,1}, {0,0}, {-1,-1}, {-1,-1},
    // ZR: D-Up=. D-Down== D-Left=+ D-Right=- A=x B=/ X=ALM Y=% L=C
    {2,2}, {2,3}, {3,0}, {3,1}, {3,2}, {3,3}, {4,0}, {4,1}, {4,2}, {-1,-1}, {-1,-1}, {-1,-1},
};

// SPACE_ADVENTURE (SA_12) - own driver/ROM, but the exact same calculator
// keypad block at the exact same (group,line) addresses as the table
// above, with one difference: "C" is IN.4 bit3 here instead of bit2 (that
// bit is IPT_UNUSED on this game, vs. "C" on trshutvoy's IN.4 bit2/bit3
// swapped) - confirmed directly against MAME's INPUT_PORTS_START(trspacadv).
constexpr ModLayerTarget kSpaceAdventureCalcLayout[24] = {
    {0,1}, {0,2}, {0,3}, {1,0}, {1,1}, {1,2}, {1,3}, {2,0}, {2,1}, {0,0}, {-1,-1}, {-1,-1},
    {2,2}, {2,3}, {3,0}, {3,1}, {3,2}, {3,3}, {4,0}, {4,1}, {4,3}, {-1,-1}, {-1,-1}, {-1,-1},
};

} // namespace

int main(int argc, char* argv[]) {
    set_storage_root("sdmc:/switch/yokoi");
    switch_settings_load();
    YOKOI_LOG("main: storage_root=%s (log file itself proves this path is writable)", storage_root().c_str());

    // Fixed at the handheld-native resolution; docked output is the console's
    // own upscale of this, same tradeoff the bring-up triangle test relied on
    // implicitly. See switch/README.md for revisiting this per dock/handheld
    // mode later (c.f. devkitPro's dynamic_resolution example).
    const int viewport_w = 1280, viewport_h = 720;
    if (!initEgl(nwindowGetDefault(), viewport_w, viewport_h)) {
        run_console_error_loop("EGL init failed.");
        return EXIT_FAILURE;
    }

    if (!gladLoadGL()) {
        run_console_error_loop("gladLoadGL failed.");
        deinitEgl();
        return EXIT_FAILURE;
    }

    if (!load_pack() || get_nb_name() == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "ROM pack missing or empty.\nPlace a pack at:\n%s",
                 storage_path("yokoi_pack_rgds.ykp").c_str());
        YOKOI_LOG("main: %s", msg);
        deinitEgl();
        run_console_error_loop(msg);
        return EXIT_FAILURE;
    }
    YOKOI_LOG("main: pack loaded, %u games", (unsigned)get_nb_name());

    switch_screen_init(viewport_w, viewport_h);
    switch_input_init();
    g_sound.configure_sound();
    switch_menu_init();

    while (appletMainLoop()) {
        const uint64_t kDown = switch_input_poll();
        const uint64_t kHeld = switch_input_held();

        if (switch_menu_is_active()) {
            // L + R + Plus, game list only: quits to hbmenu.
            if ((kHeld & HidNpadButton_L) && (kHeld & HidNpadButton_R) && (kDown & HidNpadButton_Plus)) {
                // Nothing should ever be loaded while the game list is up
                // (see switch_unload_current_game()'s callers) - this is a
                // no-op in the normal case, kept only as a safety net.
                switch_unload_current_game();
                break;
            }
        } else {
            // ZL + ZR, gameplay only: opens the in-game settings screen,
            // pausing the same way switch_menu_open_ingame_settings() always
            // has (it flips g_menu_active, which is what stops CPU stepping
            // below) - its own "Return to Main Menu" row is the only way
            // back to the game list from here now. Unlike the old L+R+Plus
            // combo, ZL/ZR need no third button to guard against an
            // accidental trigger: neither does anything else during gameplay
            // (L=Time, +=Game A, -=Game B), so this only needs both held
            // together. Triggers on whichever of the two completes the pair
            // this frame, so a near-simultaneous press of both still fires
            // exactly once - and unlike the old Plus-based combo, there's no
            // same-frame double-processing risk to guard against here either,
            // since switch_menu_update()'s settings-close check only ever
            // watches Plus/B, never ZL/ZR.
            const bool zlzr_combo = ((kDown & HidNpadButton_ZL) || (kDown & HidNpadButton_ZR))
                                  && (kHeld & HidNpadButton_ZL) && (kHeld & HidNpadButton_ZR);
            if (zlzr_combo) {
                switch_menu_open_ingame_settings();
            }
        }

        switch_menu_update(kDown, kHeld);

        if (switch_menu_is_active()) {
            switch_menu_render(viewport_w, viewport_h);
            eglSwapBuffers(s_display, s_surface);
            continue;
        }

        // Pet Time (core/pet_time.h): while a virtual pet is being caught up
        // on time spent away, that catch-up owns the whole frame - no input,
        // no normal stepping, no tone output - and draws its own overlay.
        if (pet_time_frame(kDown)) {
            g_sound.play_sample();
            switch_screen_render();
            pet_time_render(viewport_w, viewport_h);
            eglSwapBuffers(s_display, s_surface);
            continue;
        }

        // VESARIF has a normal Virtual_Input class (g_input isn't null for
        // it) but still needs its boot-time clock-setup sequence to own
        // every button while it plays out - see vesarif_clock_setup.h's
        // doc comment for why (Game/Start don't respond at all until the
        // real device's own time-set procedure is completed). Skip the
        // standard call entirely during that window, same reasoning as
        // UCHITARI below despite the different (Virtual_Input-based)
        // normal input path.
        if (g_cpu && g_game && g_game->ref == "VESARIF" && vesarif_clock_setup_active()) {
            vesarif_clock_setup_advance(g_cpu.get());
        }
        // BASSMATE and the Tronica lineup (all but Treasure Island): same
        // idea, but only a single auto-pressed confirm button rather than a
        // full clock-setup sequence - see auto_confirm_press.h.
        else if (g_cpu && auto_confirm_press_active()) {
            auto_confirm_press_advance(g_cpu.get());
        }
        else if (g_cpu && g_input) {
            const uint32_t ctl_mask = yokoi_controller_get_mask();
            yokoi_input_apply_controller_mask(g_input.get(), ctl_mask, 0, g_gamea_pulse_frames, g_gameb_pulse_frames);
        }

        // Player 2: only the three "Vs." games actually have a genuine
        // second player's worth of input in the emulated hardware itself
        // (BX_301, shared by AK_302/HK_303 - see its set_input()'s
        // player-2 case in virtual_input.h, the only class in the codebase
        // that does anything with a player other than 1). Fully automatic,
        // no setting - see switch_input_poll_p2()'s doc comment: always
        // fed regardless of whether a second controller is actually
        // connected, since an empty slot just polls as all-released, same
        // as real Vs. hardware with nothing plugged into that port.
        if (g_cpu && g_input && g_game &&
            (g_game->ref == "BX_301" || g_game->ref == "AK_302" || g_game->ref == "HK_303")) {
            const uint32_t ctl_mask_p2 = switch_input_poll_p2();
            yokoi_input_apply_controller_mask(g_input.get(), ctl_mask_p2, 0, g_gamea_pulse_frames, g_gameb_pulse_frames, 2);
        }

        // Altair's Dosimeter Reading dial (see g_dosimeter_reading's doc
        // comment in switch_settings.h) - a raw 4-bit value, not a button,
        // so it's fed directly here every frame rather than through
        // NALTAIR::set_input(). Independent of g_input (still applies even
        // though NALTAIR has no PART_SETUP/LEFT/RIGHT case for it).
        if (g_cpu && g_game && g_game->ref == "NALTAIR") {
            for (int line = 0; line < 4; line++) {
                g_cpu->input_set(1, line, (g_dosimeter_reading >> line) & 0x01);
            }
        }

        // NUMMUNCH/UCHITARI have far more physical buttons than the normal
        // 12-input layout covers - see modifier_layer_input.h's doc comment.
        // Neither is registered in virtual_input.h's get_input_config(), so
        // g_input stays null for them and the standard call above is
        // already skipped; this replaces it entirely for these two games.
        if (g_cpu && g_game && g_game->ref == "NUMMUNCH") {
            apply_modifier_layer_input(g_cpu.get(), kNummunchLayout);
        }
        else if (g_cpu && g_game && g_game->ref == "UCHITARI") {
            // While the boot-time clock-setup sequence is still playing out
            // (see uchitari_clock_setup.h), it owns every button this game
            // reads - skip the normal modifier-layer call entirely so a
            // stray player press can't get miscounted as one of its
            // scripted taps. Once it finishes, input reverts to normal.
            if (uchitari_clock_setup_active()) {
                uchitari_clock_setup_advance(g_cpu.get());
            } else {
                apply_modifier_layer_input(g_cpu.get(), kUchitariLayout);
            }
        }

        // Tiger R-Zone: buttons go through the cartridge's 74165 shift
        // register (see rzone_io.h), not the col/line keypad - g_input is
        // null for these, same as the special cases above.
        if (g_cpu && g_rzone_io) {
            apply_rzone_input(g_rzone_io.get());
        }

        // E0C6S46/S48 keychain games (Tamagotchi etc.): every button is
        // wired straight to a K pin - see e0c6200_input_map.h. g_input is
        // null for these too. Type id check, not dynamic_cast (-fno-rtti).
        // While the automatic clock-set sequence is pressing buttons (see
        // e0c6200_clock_setup.h), it owns the K pins instead.
        if (g_cpu && g_game && g_cpu->get_cpu_type_id() == CPU_TYPE_E0C6S46) {
            E0C6S46* e0c = static_cast<E0C6S46*>(g_cpu.get());
            if (e0c6200_clock_setup_active()) {
                e0c6200_clock_setup_advance(e0c);
            } else {
                apply_e0c6200_input(e0c, g_game->ref);
            }
        }

        // McDonald's (LCDonald-ported) games read a small named-input
        // vocabulary ("Jump", "Left", ...) rather than the normal col/line
        // keypad, so - same reasoning as NUMMUNCH/UCHITARI above - they're
        // not registered in virtual_input.h's get_input_config() (g_input
        // stays null, the standard call above is already skipped) and get
        // their own dedicated call here instead. static_cast is safe (not
        // dynamic_cast - this build is -fno-rtti): manufacturer already
        // tells us g_cpu is genuinely a LCDonald_fake_cpu.
        if (g_cpu && g_game && GW_rom::is_lcdonald_family(g_game->manufacturer)) {
            LCDonald_fake_cpu* lcd_cpu = static_cast<LCDonald_fake_cpu*>(g_cpu.get());
            apply_lcdonald_input(lcd_cpu);

            // A freshly loaded game waits on the all-lit screen (as does a
            // finished one, once its game_over()/victory() closing
            // animation ends and it stop()s) - Plus starts the normal game
            // ("Game A"), Minus starts Endless mode ("Game B"). Neither
            // button is used during gameplay for these games (they're not
            // registered in virtual_input.h's get_input_config(), so the
            // normal Game A/B bindings never apply here), and both are gated
            // on is_stopped() so a stray press can't reset a run in progress.
            // Endless high score: tracked live, written to disk once the run
            // ends (and on unload - see switch_unload_current_game()).
            if (lcd_cpu->is_endless()) {
                high_score_note(g_game->ref, "endless", (uint32_t)lcd_cpu->endless_score());
            }
            if (lcd_cpu->is_stopped()) {
                high_score_flush();
                if (kDown & HidNpadButton_Plus) {
                    lcd_cpu->start_game(false);
                } else if (kDown & HidNpadButton_Minus) {
                    lcd_cpu->start_game(true);
                }
            }
        }

        // SPACE_ADVENTURE/SHUTTLE_VOYAGE/THIEF_IN_GARDEN: real calculator
        // keypad, layered on TOP of the normal controls above via ZL/ZR
        // rather than replacing them (see apply_calculator_modifier_input()'s
        // doc comment) - gated behind Experimental Features since exposing
        // it changes these games' input behavior beyond how they originally
        // shipped in this codebase, unlike NUMMUNCH/UCHITARI above (which
        // have no normal controls to begin with).
        if (g_cpu && g_game && g_experimental_features_enabled) {
            if (g_game->ref == "SA_12") {
                apply_calculator_modifier_input(g_cpu.get(), kSpaceAdventureCalcLayout);
            } else if (g_game->ref == "MG_8" || g_game->ref == "TG_18") {
                apply_calculator_modifier_input(g_cpu.get(), kShuttleVoyageCalcLayout);
            }
        }

        g_sound.play_sample();

        if (g_cpu) {
            // See reassert_combined_hour()'s doc comment: some SM530 games hard-reset
            // their hour digit to a boot default several real seconds after load, well
            // past kTimeSetGracePeriod's much shorter instruction-counted window above -
            // this replays just the hour/PM cells, once a frame, cheaply enough that
            // it's fine to call unconditionally (no-op for every other game).
            if (g_hour_reassert_grace_frames > 0) {
                g_hour_reassert_grace_frames--;
                g_cpu->reassert_combined_hour();
            }

            g_rate_accu += g_cpu->frequency;
            uint32_t steps = g_rate_accu / (uint32_t)kTargetFps;
            g_rate_accu -= steps * (uint32_t)kTargetFps;

            while (steps > 0) {
                if (g_cpu->step()) {
                    if (g_time_set_grace_counter > 0) {
                        g_time_set_grace_counter--;
                        g_cpu->time_set(false);
                        yokoi_cpu_set_time_if_needed(g_cpu.get());
                    }
                    update_segments_from_cpu(g_cpu.get());
                }
                g_sound.update_sound(g_cpu.get());
                steps--;
            }

            // Drains whatever this frame's tick()/custom_update() calls
            // above just queued (LCDonald_program::queue_sound()) and plays
            // each as a real decoded .ogg sample - see lcdonald_sound.h's
            // doc comment for why this needs to run every frame rather than
            // less often (take_queued_sounds() drains, it doesn't accumulate).
            if (g_game && GW_rom::is_lcdonald_family(g_game->manufacturer)) {
                lcdonald_sound_play_queued(static_cast<LCDonald_fake_cpu*>(g_cpu.get()));
            }
        }

        switch_screen_render();
        if (g_cpu && g_game && GW_rom::is_lcdonald_family(g_game->manufacturer)) {
            lcdonald_hud_render(viewport_w, viewport_h, static_cast<LCDonald_fake_cpu*>(g_cpu.get()),
                                high_score_get(g_game->ref, "endless"));
        }
        eglSwapBuffers(s_display, s_surface);
    }

    // Covers exits appletMainLoop() itself ends, not just the ones this file
    // already handles explicitly (quitting from the game list, or the
    // settings screen's "Return to Main Menu" row) - e.g. the HOME menu
    // force-closing a forwarder mid-game, which never goes through either of
    // those paths. No-op if nothing was loaded (including the normal case,
    // since by the time the game list is showing, nothing ever is - see
    // switch_unload_current_game()'s callers).
    switch_unload_current_game();

    switch_menu_shutdown();
    g_sound.Exit();
    switch_screen_shutdown();
    deinitEgl();
    return EXIT_SUCCESS;
}
