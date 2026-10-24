#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/TailsSoccer.cs ("Tails Soccer (2004)") -
// same shape as KnucklesSoccer, different names/thresholds/speed.
class TailsSoccer : public LCDonald_program {
public:
    std::string short_name() const override { return "tsoccer"; }
    std::string name() const override { return "Tails Soccer (2004)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(tails_position_);
        io.io(level_);
        io.io(balls_intercepted_);
        io.io(balls_missed_);
        io.io(ball_pos_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override {}  // Unused in this game.

private:
    std::string tails_element() const;
    void game_over();
    void victory();

    int tails_position_ = 0;
    int level_ = 0;
    int balls_intercepted_ = 0;
    int balls_missed_ = 0;
    int ball_pos_ = -1;
};
