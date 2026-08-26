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

        if (g_cpu && g_input) {
            const uint32_t ctl_mask = yokoi_controller_get_mask();
            yokoi_input_apply_controller_mask(g_input.get(), ctl_mask, 0, g_gamea_pulse_frames, g_gameb_pulse_frames);
        }

        g_sound.play_sample();

        if (g_cpu) {
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
        }

        switch_screen_render();
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
