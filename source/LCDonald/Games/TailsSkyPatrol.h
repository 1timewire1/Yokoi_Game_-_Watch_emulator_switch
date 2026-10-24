#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/TailsSkyPatrol.cs ("Tails' Sky Patrol
// (2003)") - Tails moves between 3 rows to collect rings converging from
// two columns.
class TailsSkyPatrol : public LCDonald_program {
public:
    std::string short_name() const override { return "tskypatrol"; }
    std::string name() const override { return "Tails' Sky Patrol (2003)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Up", "Down"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(tails_position_);
        io.io(rings_collected_);
        io.io(rings_missed_);
        io.io(level_);
        io.io(ring_positions_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::string tails_element() const;
    void game_over();
    void victory();

    int tails_position_ = 0;
    int rings_collected_ = 0;
    int rings_missed_ = 0;
    int level_ = 0;
    std::vector<int> ring_positions_;
};
