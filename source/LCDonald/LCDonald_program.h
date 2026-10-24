#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "std/timer.h"
#include "LCDonald_state_io.h"

// Base class for porting LCDonald (https://github.com/Difegue/LCDonald) game
// logic - McDonald's Sonic-branded LCD games, simulated in C# there rather
// than emulated from a real ROM dump (no real MCU exists to dump: these were
// simple ASIC-driven toys, not SM5XX-family Game & Watch hardware). LCDonald
// already targets MAME's .lay layout format for its artwork, identical to
// what this project's own CONVERT_ROM pipeline already extracts segment art
// from - the porting gap is purely the game-logic layer, ported here as a
// direct architectural analogue to David_And_John_program/David_And_John_
// fake_cpu (a hand-simulated "program" driving a fake CPU class rather than
// real SM5XX opcode execution), just against LCDonald's own simpler contract
// (named SVG group visibility per tick) instead of David & John's projectile/
// life-system template.
//
// Ticking model differs slightly from the original C# (LCDGameBase.cs):
// there, a 100ms background timer calls CustomUpdate() while a separate,
// faster per-frame Update() call advances animations/blink state and can
// block the calling thread until an animation finishes. This engine has no
// such second thread - everything is driven by one fixed-rate tick, so
// animations here are modeled as pure state (current_animation_/animation_
// index_) that advances one frame per update() call instead of blocking the
// caller. From every game's own perspective (which never reads animation
// state back, only starts one and returns) this is behaviorally identical,
// just non-blocking.
class LCDonald_program {
public:
    virtual ~LCDonald_program() = default;

    virtual std::string short_name() const = 0;
    virtual std::string name() const = 0;

    // Every SVG group name this game's layout defines - used by
    // generic_victory_animation()'s "flash everything" frame, and (once
    // wired into the real asset pipeline) to validate a game's ported
    // element names against its actual .lay/SVG group list.
    virtual std::vector<std::string> get_all_game_elements() const = 0;

    // Input names this game reads in handle_inputs() (e.g. "Jump", "Left",
    // "Right") - purely descriptive here, unlike LCDGameInput.cs's KeyCode
    // (a desktop keybinding concept with no equivalent on this project's
    // platforms).
    virtual std::vector<std::string> get_available_inputs() const = 0;

    virtual void initialize_game_state() = 0;
    virtual void handle_inputs(const std::vector<std::string>& pressed_inputs) = 0;

    // Called once per custom_update_speed_ms() tick (default 100ms, some
    // games change it) via step_custom_update() below. Advances gameplay
    // state. Required override even when unused (several ported games
    // leave this empty, matching their C# source's "// Unused in this
    // game" UpdateCore() overrides) - kept pure virtual rather than
    // defaulted so every port makes that choice explicitly, same as the
    // original abstract method did.
    virtual void custom_update() = 0;

    // Public API mirroring ILCDGame.cs.
    std::vector<std::string> get_visible_game_elements();
    void start(bool is_endless = false);
    void pause();
    void resume();
    void stop();
    bool is_input_blocked() const { return input_blocked_; }
    bool is_stopped() const { return is_stopped_; }
    bool is_animating() const { return has_animation_; }
    int custom_update_speed_ms() const { return custom_update_speed_; }
    std::vector<std::string> take_queued_sounds();
    // increment_score() calls made since the last call (the original's Scored
    // event) - drained and reset; the host sums them for Endless mode's score.
    int take_score_events();

    // Burger Bard white-label variant (LCDonald's `#if BURGER` build): the
    // same game logic re-skinned as a fictional brand, with a handful of
    // per-game tweaks and a few extra elements ("win" on victory frames, a
    // rare hidden "score-1" group). `folder` is the Burger asset folder /
    // short name ("platform", "seashell", ...); select_game() calls this
    // right after constructing the program. Sound lookups use the base
    // game's own files (identical in the Burger assets) with the folder name
    // available for per-Burger overrides - see lcdonald_sound.cpp.
    void set_burger(const std::string& folder) { burger_ = true; burger_folder_ = folder; }
    bool is_burger() const { return burger_; }
    const std::string& burger_folder() const { return burger_folder_; }

    // True while the Burger hidden "score-1" group is showing (a ~50ms
    // flash - 5 display polls at the original's 10ms pace). The host uses it
    // to know when the visible-element snapshot needs refreshing.
    bool hidden_group_visible() const;

    // Called every tick regardless of custom_update_speed_ms() pacing -
    // matches Update()'s per-frame role (blink decay + animation advance).
    // Callers own their own pacing for step_custom_update() (this engine
    // has no separate background timer to do it for them) - see
    // custom_update_speed_ms() above.
    void tick();

    // Call on the custom_update_speed_ms() cadence - wraps custom_update()
    // with the same blocked-cycle countdown and input-unblock timing as
    // the original's UpdateGameState() (the internal 100ms timer handler):
    // while blocked_custom_updates_ > 0 (set by startup_music()), this
    // just counts down and skips custom_update() entirely; the tick that
    // finally reaches 0 is the one that clears input_blocked_ and calls
    // custom_update() itself, not the tick after.
    void step_custom_update();

    // Save states: writes/reads the shared base-class state, then the game's
    // own fields via visit_state(). Returns false on a short/corrupt file.
    // Not saved: queued sounds (transient) and the RNG (reseeded from the
    // clock at construction - a restored game just continues with fresh
    // randomness).
    bool serialize_state(LCDonaldStateIO& io);

protected:
    // Each game lists its own state fields here, in a fixed order; the same
    // function both saves and loads them (see LCDonaldStateIO).
    virtual void visit_state(LCDonaldStateIO& io) = 0;

    // Returns this frame's visible elements, before blink/animation
    // overrides are applied - equivalent to GetVisibleElements() (C#).
    virtual std::vector<std::string> get_visible_elements() = 0;

    // Called from tick() (the per-frame call), NOT step_custom_update() -
    // matches UpdateCore()'s original role of running every frame rather
    // than on the slower custom-update cadence. Neither game ported so far
    // uses it (both leave it a one-line empty override, matching their C#
    // source), but it's kept virtual rather than removed so a future port
    // that does need per-frame logic doesn't need base-class surgery.
    virtual void update_core() {}

    void queue_sound(const std::string& file);
    bool is_blinking(const std::string& element) const;
    void blink_element(const std::string& element, int times);

    // Starts a non-blocking animation - see the class comment for how this
    // differs from the original's blocking behavior. Sets input_blocked_
    // until the animation finishes.
    void play_animation(const std::vector<std::vector<std::string>>& animation);

    void generic_game_over_animation(const std::vector<std::string>& elements_to_blink,
                                      const std::string& animation_sound = "../common/game_over.ogg");
    void generic_victory_animation(const std::vector<std::string>& elements_to_blink);

    void startup_music(const std::string& music_file = "../common/game_start.ogg", int block_input_cycles = 1);
    void block_custom_updates(int blocked_updates) { blocked_custom_updates_ = blocked_updates; }

    void increment_score() { ++score_events_; }

    int custom_update_speed_ = 100;
    int blocked_custom_updates_ = 0;
    bool input_blocked_ = false;
    bool is_endless_mode_ = false;
    bool burger_ = false;
    std::string burger_folder_;
    // Seeded from the real clock (time_us_64_p(), std/timer.h), NOT
    // std::random_device - devkitA64/libnx's random_device support goes
    // through a hardware RNG service that isn't guaranteed to be quick or
    // even available the moment a game is selected (see select_game() in
    // LCDonald_fake_cpu.cpp, called from load_rom() during boot), and with
    // -fno-exceptions (see switch/Makefile) any failure path libstdc++
    // would normally throw on instead calls std::terminate(). That matches
    // exactly what was seen in testing: games booting inconsistently, some
    // hanging with no segments ever shown - i.e. failing inside this very
    // member initializer, before the game ever got a chance to run.
    // time_us_64_p() has no failure path and is already used elsewhere in
    // this engine's own timing code.
    std::minstd_rand rng_{static_cast<std::minstd_rand::result_type>(time_us_64_p())};

    // Inclusive [lo, hi] random int, matching C#'s Random.Next(lo, hi+1)
    // callsites in the ported games (System.Random.Next(min, max) is
    // exclusive of max, so every call site below adds 1 back at the C++
    // call site to stay inclusive-inclusive instead).
    int rand_range(int lo, int hi);

private:
    void finish_stop();

    bool is_paused_ = false;
    bool is_stopped_ = true;
    std::vector<std::string> queued_sounds_;
    std::unordered_map<std::string, float> blinking_elements_;
    std::vector<std::vector<std::string>> current_animation_;
    size_t animation_index_ = 0;
    bool has_animation_ = false;
    // stop() called while an animation is still playing - see stop().
    bool stop_pending_ = false;
    int score_events_ = 0;
    // Burger only: LCDGameBase.cs gives every update a 1-in-9 chance (once
    // per game) to show the hidden "score-1" group for 5 display polls.
    bool has_triggered_hidden_group_ = false;
    uint64_t hidden_group_until_us_ = 0;
};
