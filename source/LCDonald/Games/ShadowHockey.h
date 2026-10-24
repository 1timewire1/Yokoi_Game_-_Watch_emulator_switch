#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/ShadowHockey.cs ("Shadow Hockey (2004)") -
// move Shadow left/right, shoot pucks past AI defenders ("omega") for
// goals shown on an 8-segment display.
class ShadowHockey : public LCDonald_program {
public:
    std::string short_name() const override { return "shockey"; }
    std::string name() const override { return "Shadow Hockey (2004)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right", "Hockey"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(shadow_position_);
        io.io(omega_positions_);
        io.io(ball_position_);
        io.io(hand_positions_);
        io.io(goals_scored_);
        io.io(balls_intercepted_);
        io.io(level_);
        io.io(ball_half_step_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    void set_ball_slowdown(bool slowdown);
    void add_omega();
    void game_over();
    void victory();

    int shadow_position_ = 0;
    std::vector<int> omega_positions_;
    int ball_position_ = 0;
    std::vector<int> hand_positions_;
    int goals_scored_ = 0;
    int balls_intercepted_ = 0;
    int level_ = 0;
    bool ball_half_step_ = false;
};
