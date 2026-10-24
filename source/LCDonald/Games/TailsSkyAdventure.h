#pragma once

#include "../LCDonald_program.h"

// Port of LCDonald.Core/Games/TailsSkyAdventure.cs ("Tails' Sky Adventure
// (2005)") - Tails shoots a single projectile at an enemy weaving toward
// him across 3 lanes. Life-loss moments in the original spin-wait on a
// second thread (`while (IsBlinking(...)) { }`, relying on the separate
// per-frame Update() thread to keep decaying the blink counter concurrently)
// - ported as an explicit `while (is_blinking()) tick();` loop instead,
// which reproduces the same "keep everything else ticking while we wait"
// behavior deterministically in a single thread (see cpp for the exact
// spot) rather than relying on OS scheduling.
class TailsSkyAdventure : public LCDonald_program {
public:
    std::string short_name() const override { return "tskyadventure"; }
    std::string name() const override { return "Tails' Sky Adventure (2005)"; }

    std::vector<std::string> get_all_game_elements() const override;
    std::vector<std::string> get_available_inputs() const override { return {"Left", "Right", "Fire"}; }

    void initialize_game_state() override;
    void handle_inputs(const std::vector<std::string>& pressed_inputs) override;
    void custom_update() override;

protected:
    std::vector<std::string> get_visible_elements() override;
    void update_core() override;

private:
    std::string tails_element() const;
    void game_over();
    void victory();

    int tails_position_ = 0;
    int life_count_ = 0;
    int level_ = 0;
    int enemies_hit_ = 0;
    int enemies_missed_ = 0;
    int projectile_pos_ = -1;
    int enemy_pos_ = -1;
    bool spawn_enemy_next_update_ = false;
};
