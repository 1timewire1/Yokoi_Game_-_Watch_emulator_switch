#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/SonicExtremeBoarding.cs ("Sonic Extreme
// Boarding (2004)") - Sonic moves across a 7-position rail collecting a
// single traveling ring while dodging bombs. The original's `lock
// (_bombPositions)` calls are C#'s thread-safety primitive for its
// separate Update()/CustomUpdate() threads - dropped entirely here since
// this port is single-threaded (see LCDonald_program.h's class comment).
class SonicExtremeBoarding : public LCDonald_program {
public:
    std::string short_name() const override { return "sxtremeboard"; }
    std::string name() const override { return "Sonic Extreme Boarding (2004)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(sonic_position_);
        io.io(rings_collected_);
        io.io(bombs_hit_);
        io.io(level_);
        io.io(ring_spawn_);
        io.io(ring_position_);
        io.io(show_ring_spawn_indicator_);
        io.io(bomb_positions_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::string sonic_element() const { return "sonic-" + std::to_string(sonic_position_); }
    int spawn_bomb();
    void level_up();
    void game_over();
    void victory();

    int sonic_position_ = 0;
    int rings_collected_ = 0;
    int bombs_hit_ = 0;
    int level_ = 0;

    int ring_spawn_ = 0;
    int ring_position_ = 0;
    bool show_ring_spawn_indicator_ = false;

    std::vector<int> bomb_positions_;
};
