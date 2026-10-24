#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/CreamFlowerCatch2005.cs ("Cream Flower
// Catch (2005)") - upgraded CreamFlowerCatch with hit bubbles and a 5-step
// miss meter instead of left/right miss indicators.
class CreamFlowerCatch2005 : public LCDonald_program {
public:
    std::string short_name() const override { return "cflower2"; }
    std::string name() const override { return "Cream Flower Catch (2005)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::string cream_element() const;
    void game_over();
    void victory();

    int cream_position_ = 0;
    int flowers_collected_ = 0;
    int flowers_missed_ = 0;
    int level_ = 0;
    std::vector<int> flower_positions_;
};
