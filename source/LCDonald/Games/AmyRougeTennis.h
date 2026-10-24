#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/AmyRougeTennis.cs ("Amy & Rouge Tennis
// (2005)") - near-identical to AmyRougeVolleyball, simpler UpdateCore
// (no "hit noball" miss branch) and different AI-resistance/speed tuning.
class AmyRougeTennis : public LCDonald_program {
public:
    std::string short_name() const override { return "artennis"; }
    std::string name() const override { return "Amy & Rouge Tennis (2005)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right", "Tennis"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(amy_position_);
        io.io(rouge_position_);
        io.io(ball_position_);
        io.io(level_);
        io.io(current_score_);
        io.io(ai_resistance_);
        io.io(next_ball_spawn_);
        io.io(bat_engaged_);
        io.io(ball_direction_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::string amy_element() const { return "amy-" + std::to_string(amy_position_); }
    void level_up_animation();
    void game_over();
    void victory();

    int amy_position_ = 0;
    int rouge_position_ = 0;
    int ball_position_ = 0;
    int level_ = 0;
    int current_score_ = 0;
    int ai_resistance_ = 2147483647;
    int next_ball_spawn_ = 0;
    bool bat_engaged_ = false;
    bool ball_direction_ = false;
};
