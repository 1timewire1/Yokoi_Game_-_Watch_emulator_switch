#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/ShadowGrinder.cs ("Shadow Grinder (2003)") -
// a single-button endless-runner: Shadow rides a rail with periodic gaps,
// Jump to hop over one. Chosen (along with KnucklesSoccer) as the first
// LCDonald port precisely because it's the smallest game in the set (221
// lines of C#) - a good test of how involved the logic-port side of this
// is before committing to the rest of the roster.
class ShadowGrinder : public LCDonald_program {
public:
    std::string short_name() const override { return "sgrinder"; }
    std::string name() const override { return "Shadow Grinder (2003)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Jump"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(shadow_position_);
        io.io(gaps_dodged_);
        io.io(misses_);
        io.io(level_);
        io.io(spawn_first_gap_);
        io.io(rail_positions_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override {}  // Unused in this game.

private:
    void game_over();
    void victory();

    int shadow_position_ = 0;
    int gaps_dodged_ = 0;
    int misses_ = 0;
    int level_ = 0;
    bool spawn_first_gap_ = false;
    std::vector<int> rail_positions_;
};
