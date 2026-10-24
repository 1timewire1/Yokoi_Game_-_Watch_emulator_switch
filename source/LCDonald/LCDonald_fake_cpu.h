#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../SM5XX/SM5XX.h"
#include "LCDonald_program.h"

// One row of the per-game segment map CONVERT_ROM emits alongside the
// extracted segment PNGs (see extract_lcdonald_segs2.py) - the synthetic
// (col,line,word) triple assigned to a given named SVG segment during
// extraction, in the same order the pack's Segment array was built in, so
// this table and the pack agree on what each id means without either side
// hand-maintaining it.
struct LCDonaldSegmentMapEntry {
    uint8_t col, line, word;
    const char* name;
};

// Fake CPU for LCDonald-derived games (McDonald's Sonic LCD games,
// simulated in C# by the LCDonald project rather than emulated from a real
// ROM dump - see LCDonald_program.h). Architecturally the same idea as
// David_And_John_fake_cpu: subclasses SM5XX purely so the rest of the
// engine's rendering/save-state/menu code can keep treating every loaded
// game as "some SM5XX", with every real-CPU-specific virtual (opcode
// execution, ROM/RAM access, RAM-based segment decoding) stubbed out and
// segment state instead read directly from the wrapped LCDonald_program's
// current visible-elements list via segment_map.
//
// Timing: SM5XX::step() (the engine's actual per-cycle driver, called some
// number of times per rendered frame based on `frequency` - see main.cpp's
// `g_rate_accu += g_cpu->frequency` pacing) is NOT virtual, so unlike what
// a first pass at this class assumed, it can't be overridden - every real
// CPU (David_And_John_fake_cpu included) gets its actual logic to run by
// putting it in execute_curr_opcode() instead, which step() already calls
// once every `cycle_curr_opcode` cycles (reset each time via the also-
// non-virtual calculate_cycle(), using is_on_double_octet()'s return value
// - true here, matching David_And_John_fake_cpu, giving a fixed 4-cycle
// step between calls). execute_curr_opcode() therefore fires far more often
// than program()->tick() (10Hz) or step_custom_update() (~10Hz default)
// actually need - it paces both against real wall-clock time
// (time_us_64_p(), std/timer.h), exactly like David_And_John_program.h's
// own WAIT_BEFORE_STEP-gated update_step()/blink_player()/etc, rather than
// assuming execute_curr_opcode() itself lands at a fixed rate derived from
// `frequency`. That assumption doesn't hold in practice (main.cpp's
// g_rate_accu/kTargetFps pacing isn't exactly 60Hz in real time), and an
// earlier version of this class that counted quanta instead of reading the
// clock ran every game and read input noticeably too fast as a result.
// get_segments_state() just reads a cached visible-elements snapshot
// refreshed whenever either real-time check fires.
class LCDonald_fake_cpu : public SM5XX {
public:
    LCDonald_fake_cpu();

    void init() override;

    // Selects which of the 39 games this instance runs (20 McDonald's + 19
    // Burger Bard), keyed by the dummy
    // ROM's last byte (a 1-39 game id - see GW_ROM_RGDS/lcd_*.cpp, the same
    // "no real ROM, last byte picks the program" scheme David_And_John_fake_
    // cpu uses via its own 5-game switch in load_rom()). Unrecognized ids
    // leave program_ null, matching David_And_John_fake_cpu's behavior for
    // an out-of-range switch (no case matches -> curr_program stays null).
    void load_rom(const uint8_t* file_hex, size_t size_hex) override;

    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override;
    bool screen_is_on() override { return true; }

    bool save_state(FILE* file) override;
    bool load_state(FILE* file) override;
    uint8_t get_cpu_type_id() override { return 255; }  // CPU_TYPE_FAKE_CPU, matches David_And_John_fake_cpu.

    // Marks a named input (matching program()->get_available_inputs(),
    // e.g. "Jump", "Left") held or released for the current frame - the
    // Switch input layer calls this from whatever physical buttons it
    // maps onto that game's declared inputs.
    //
    // Edge-triggered, NOT "currently held": the original C# (see
    // AvaloniaLCDView.axaml.cs's HandleInput()/GetPressedInputs()) adds an
    // input to its pending queue on KeyDown, explicitly guards against
    // "holding the key down from spamming the input buffer" via a separate
    // _bufferedKeyCodes latch, and only clears that latch on KeyUp - so
    // HandleInputs() sees a given input's name exactly once per physical
    // press, no matter how many polls happen while it's held or how long
    // the hold lasts. A first version of this class instead forwarded
    // "currently held" state to handle_inputs() every tick() (~60Hz), so
    // holding a direction for even a handful of frames replayed that game's
    // one-tap-equals-one-move logic several times per press (e.g. rolling
    // through two lane changes instead of one). buffered_inputs_/
    // pending_inputs_ below reproduce the same press/latch/drain shape.
    void set_input_pressed(const std::string& input_name, bool pressed);

    // True while the game is stopped: right after loading (nothing started
    // yet - the all-lit screen) and again once a run ends (game over / win,
    // after its closing animation - the game's own game_over()/victory()
    // call stop() following that animation; see LCDonald_program::stop()'s
    // deferral). Used by the Switch input layer (main.cpp) to decide when
    // + (Normal) / - (Endless) may start a game, so a stray press mid-run
    // can never reset it.
    bool is_stopped() const { return program_ && program_->is_stopped(); }

    // Starts (or restarts) the loaded game from scratch as if freshly booted -
    // program()->start(endless) re-runs initialize_game_state() on the same
    // instance, which is also how the original app's own menu re-selects a
    // game it already loaded (LCDonald keeps one long-lived instance per
    // game, not a fresh object per play). Only meant to be called while
    // is_stopped() is true - see main.cpp's call site.
    void start_game(bool endless);

    // Endless mode bookkeeping (LCDonald.Core's Endless: same game, no
    // levels/win - see each game's is_endless_mode_ branches). The running
    // score is the count of increment_score() calls the game made, exactly
    // like the original's Scored event; not persisted anywhere yet.
    // is_endless() stays true after an Endless run ends so the final score
    // remains on screen until the next start_game() call replaces it.
    bool is_endless() const { return endless_; }
    int endless_score() const { return endless_score_; }

    LCDonald_program* program() { return program_.get(); }

private:
    // Constructs the right LCDonald_program subclass + points segment_map_/
    // segment_map_size_ at the right static table for game_id (1-39, see
    // load_rom()'s switch). Split out of load_rom() purely for readability.
    void select_game(uint8_t game_id);


    // Real logic lives here - see the class comment on why (step() itself
    // isn't virtual).
    void execute_curr_opcode() override;

    void update_segment() override {}
    bool no_pc_increase(uint8_t) override { return false; }
    bool is_on_double_octet(uint8_t) override { return true; }  // Fixes cycle_curr_opcode at 4 - see class comment.
    bool condition_to_update_segment() override { return false; }
    void wake_up() override {}
    uint8_t read_rom_value() override { return 0; }
    uint8_t read_ram_value() override { return 0; }
    void write_ram_value(uint8_t) override {}
    void set_ram_value(uint8_t, uint8_t, uint8_t) override {}
    bool get_active_sound() override { return false; }
    void end_of_cpu() override {}

    void refresh_visible_cache();

    std::unique_ptr<LCDonald_program> program_;
    // Defaulted (not left indeterminate) so get_segments_state() is a safe
    // no-op if it's ever reached before select_game() runs - see
    // LCDonald_fake_cpu::init()'s cycle_curr_opcode comment for the class
    // of bug this same pattern already caused once.
    const LCDonaldSegmentMapEntry* segment_map_ = nullptr;
    size_t segment_map_size_ = 0;

    std::vector<std::string> visible_cache_;
    // Mirrors AvaloniaLCDView's _bufferedKeyCodes - inputs currently latched
    // "already reported this press", cleared on release (set_input_pressed
    // with pressed=false), not on drain.
    std::vector<std::string> buffered_inputs_;
    // Mirrors its _inputBuffer - newly-pressed-since-last-drain inputs,
    // drained by execute_curr_opcode()'s 10ms input pass (handed to
    // program_->handle_inputs() only if the game isn't input-blocked).
    std::vector<std::string> pending_inputs_;

    // Real wall-clock pacing (time_us_64_p(), std/timer.h) - NOT derived
    // from counting execute_curr_opcode() calls. An earlier version paced
    // tick()/step_custom_update() off a synthetic microsecond accumulator
    // that assumed execute_curr_opcode() fires at exactly frequency/4 Hz in
    // real time; in practice that undercounts how often step() actually
    // runs (main.cpp's g_rate_accu/kTargetFps pacing isn't perfectly 60Hz
    // in real time), so every LCDonald game ran and read input noticeably
    // faster than intended. David_And_John_program.h's update_step()/
    // blink_player() etc. hit the same class of problem and already solved
    // it this way (last_time_step + WAIT_BEFORE_STEP < curr_time via
    // time_us_64_p()) - this mirrors that fix instead of the synthetic
    // accumulator. 0 means "not started yet" (set on the first call).
    uint64_t last_tick_time_us_ = 0;
    uint64_t last_custom_update_time_us_ = 0;
    uint64_t last_input_poll_time_us_ = 0;

    bool endless_ = false;
    int endless_score_ = 0;
    bool hidden_group_shown_ = false;
    // The dummy ROM's game id (see load_rom()) - kept so a failed state load
    // can rebuild a fresh program for the same game.
    uint8_t game_id_ = 0;
};
