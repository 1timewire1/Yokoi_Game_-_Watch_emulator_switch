#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/SonicSpeedway.cs ("Sonic Speedway (2003)") -
// dodge rows of cars converging from 3 lanes.
class SonicSpeedway : public LCDonald_program {
public:
    std::string short_name() const override { return "sspeedway"; }
    std::string name() const override { return "Sonic Speedway (2003)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    std::vector<std::string> get_visible_elements() override;
    void update_core() override {}  // Unused in this game.

private:
    std::string sonic_element() const;
    void game_over();
    void victory();

    int sonic_position_ = 0;
    int car_rows_dodged_ = 0;
    int cars_hit_ = 0;
    int level_ = 0;
    std::vector<int> car_positions_;
};
