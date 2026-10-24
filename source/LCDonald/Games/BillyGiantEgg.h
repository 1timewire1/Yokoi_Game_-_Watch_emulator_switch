#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/BillyGiantEgg.cs ("Billy's Giant Egg
// (2005)") - collect fruit while dodging bombs; every 5 fruit lights an
// egg stage, 2 bomb hits knocks the egg stage back down.
class BillyGiantEgg : public LCDonald_program {
public:
    std::string short_name() const override { return "bgiantegg"; }
    std::string name() const override { return "Billy's Giant Egg (2005)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::string billy_element() const;
    void spawn_fruits();
    void level_up();
    void game_over();
    void victory();

    int billy_position_ = 0;
    int fruits_collected_ = 0;
    int bombs_hit_ = 0;
    int level_ = 0;
    std::vector<int> bomb_positions_;
    std::vector<int> fruit_positions_;
};
