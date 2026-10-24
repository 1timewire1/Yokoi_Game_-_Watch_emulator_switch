#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/KnucklesBaseball.cs ("Knuckles Baseball
// (2004)") - vertical (Up/Down) version of KnucklesSoccer/TailsSoccer.
class KnucklesBaseball : public LCDonald_program {
public:
    std::string short_name() const override { return "kbaseball"; }
    std::string name() const override { return "Knuckles Baseball (2004)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Up", "Down"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(knuckles_position_);
        io.io(level_);
        io.io(balls_intercepted_);
        io.io(balls_missed_);
        io.io(ball_pos_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override {}  // Unused in this game.

private:
    std::string knuckles_element() const;
    void game_over();
    void victory();

    int knuckles_position_ = 0;
    int level_ = 0;
    int balls_intercepted_ = 0;
    int balls_missed_ = 0;
    int ball_pos_ = -1;
};
