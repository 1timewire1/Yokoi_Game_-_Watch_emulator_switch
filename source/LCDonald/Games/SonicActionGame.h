#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/SonicActionGame.cs ("Sonic Action Game
// (2003)") - Sonic jumps between 2 platform rows dodging/riding blocks
// approaching from the right, with an optional double-jump onto a 3rd row.
class SonicActionGame : public LCDonald_program {
public:
    std::string short_name() const override { return "sagame"; }
    std::string name() const override { return "Sonic Action Game (2003)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Jump"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(sonic_position_);
        io.io(platforms_dodged_);
        io.io(platforms_hit_);
        io.io(level_);
        io.io(block_spawn_counter_);
        io.io(platform_positions_);
        io.io(ring_positions_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override {}  // Unused in this game.

private:
    std::string sonic_element() const { return "sonic-" + std::to_string(sonic_position_); }
    void game_over();
    void victory();

    int sonic_position_ = 0;
    int platforms_dodged_ = 0;
    int platforms_hit_ = 0;
    int level_ = 0;
    int block_spawn_counter_ = 0;
    std::vector<int> platform_positions_;
    std::vector<int> ring_positions_;
};
