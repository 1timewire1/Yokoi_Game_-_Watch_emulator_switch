#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/KnucklesTreasureHunt.cs ("Knuckles Treasure
// Hunt (2005)") - Knuckles moves across scrolling platform rows collecting
// emeralds against a time limit, falling if the platform under him
// disappears. FallAnimation() has the same "blocking PlayAnimation() in a
// while-loop" pattern as BigFishing's ReelUpAnimation() - ported the same
// way (build the whole sequence up front, one non-blocking play_animation()
// call at the end). Biggest game in the roster (503 lines of C#).
class KnucklesTreasureHunt : public LCDonald_program {
public:
    std::string short_name() const override { return "ktreasure"; }
    std::string name() const override { return "Knuckles Treasure Hunt (2005)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right"}; }

    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void initialize_game_state() override;
    void custom_update() override;

protected:
    // Save-state fields, in a fixed order (see LCDonald_program::visit_state()).
    void visit_state(LCDonaldStateIO& io) override {
        io.io(knuckles_position_);
        io.io(life_count_);
        io.io(level_);
        io.io(emeralds_collected_);
        io.io(emerald_time_limit_);
        io.io(platform_positions_);
        io.io(emerald_positions_);
        io.io(platforms_to_move_next_update_);
        io.io(current_left_spawn_);
        io.io(current_right_spawn_);
        io.io(is_falling_);
        io.io(is_first_emerald_);
        io.io(has_knuckles_moved_);
    }

    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::vector<std::string> get_platform_elements() const;
    std::vector<std::string> get_knuckles_and_platform_elements() const;
    std::string knuckles_element() const;
    void emerald_spawn_animation();
    int get_random_emerald_position();
    bool is_in_front_of_emerald(int emerald) const;
    // fall_animation() also covers GameOver() - the original only ever
    // triggers it from inside FallAnimation() once life_count_ reaches 0
    // (checked via grep - no other call site exists), so its animation is
    // appended onto the same non-blocking play_animation() call instead of
    // a separate one, which would otherwise cut the fall animation off
    // before it's shown (see class comment).
    void fall_animation();
    void level_up();
    void victory();

    static const std::vector<bool> kSpawnmapLeft;
    static const std::vector<bool> kSpawnmapRight;

    int knuckles_position_ = 0;
    int life_count_ = 0;
    int level_ = 0;

    int emeralds_collected_ = 0;
    int emerald_time_limit_ = 0;
    std::vector<int> platform_positions_;
    std::vector<int> emerald_positions_;

    int platforms_to_move_next_update_ = 0;
    int current_left_spawn_ = 0;
    int current_right_spawn_ = 0;
    bool is_falling_ = false;
    bool is_first_emerald_ = false;
    bool has_knuckles_moved_ = false;
};
