#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/SonicSkateboard.cs ("Sonic Skateboard
// (2005)") - dodge bombs and collect rings across 3 lanes, 3 lives.
class SonicSkateboard : public LCDonald_program {
public:
    std::string short_name() const override { return "sskateboard"; }
    std::string name() const override { return "Sonic Skateboard (2005)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::string sonic_element() const;
    std::vector<std::string> get_lives_and_level_elements() const;
    void spawn_rings();
    void level_up();
    void game_over();
    void victory();

    int sonic_position_ = 0;
    int rings_collected_ = 0;
    int bombs_hit_ = 0;
    int lives_ = 0;
    int level_ = 0;
    std::vector<int> bomb_positions_;
    std::vector<int> ring_positions_;
};
