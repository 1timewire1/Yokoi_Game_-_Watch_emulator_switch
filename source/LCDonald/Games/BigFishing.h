#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/BigFishing.cs ("Big's Fishing (2004)") - move
// a rod up/down to hook a fish, then Reel it in. The original's ReelUp
// Animation() calls PlayAnimation() in a blocking while-loop, once per
// step of the reel-in, recomputing the frame from the just-changed state
// each time - its own comment calls this out as the one place the engine's
// blocking-animation assumption shows through. Ported by precomputing the
// whole frame sequence up front (looping the state changes without
// blocking) and calling play_animation() once at the end - see the .cpp.
class BigFishing : public LCDonald_program {
public:
    std::string short_name() const override { return "bfishing"; }
    std::string name() const override { return "Big's Fishing (2004)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Up", "Down", "Reel"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(rod_position_);
        io.io(level_);
        io.io(fish_caught_);
        io.io(fish_missed_);
        io.io(fish_pos_);
        io.io(is_fish_caught_);
        io.io(has_lure_);
        io.io(trigger_reel_animation_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::vector<std::string> get_levels() const;
    void reel_up_animation();
    void game_over();
    void victory();

    int rod_position_ = 0;
    int level_ = 0;
    int fish_caught_ = 0;
    int fish_missed_ = 0;
    int fish_pos_ = -1;
    bool is_fish_caught_ = false;
    bool has_lure_ = false;
    bool trigger_reel_animation_ = false;
};
