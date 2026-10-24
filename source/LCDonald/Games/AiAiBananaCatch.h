#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/AiAiBananaCatch.cs ("AiAi Banana Catch
// (2003)") - per the original's own comment, "basically a horizontal
// version of tskypatrol".
class AiAiBananaCatch : public LCDonald_program {
public:
    std::string short_name() const override { return "abanana"; }
    std::string name() const override { return "AiAi Banana Catch (2003)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(aiai_position_);
        io.io(bananas_collected_);
        io.io(bananas_missed_);
        io.io(level_);
        io.io(banana_positions_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::string aiai_element() const;
    void game_over();
    void victory();

    int aiai_position_ = 0;
    int bananas_collected_ = 0;
    int bananas_missed_ = 0;
    int level_ = 0;
    std::vector<int> banana_positions_;
};
