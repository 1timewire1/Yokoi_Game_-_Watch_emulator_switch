#include "ShadowGrinder.h"

#include <algorithm>

namespace {
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* MISS = "miss";
constexpr const char* SHADOW_JUMP = "shadow-2";
constexpr const char* SHADOW = "shadow-1";
constexpr const char* SHADOW_FALL = "shadow-0";

bool contains(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}
}  // namespace

std::vector<std::string> ShadowGrinder::get_all_game_elements() const {
    return {SHADOW_JUMP, LEVEL_1, LEVEL_2, LEVEL_3,
            SHADOW,       "rail-0", "rail-1",
            "rail-2",     "rail-3",
            SHADOW_FALL,  "rail-4", "rail-5",
            MISS,         "rail-6", "rail-7"};
}

void ShadowGrinder::initialize_game_state() {
    shadow_position_ = 1;
    gaps_dodged_ = 0;
    misses_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    spawn_first_gap_ = true;

    rail_positions_ = {0, 1, 2, 3, 4, 5, 6, 7};
    custom_update_speed_ = is_endless_mode_ ? 200 : 800;

    startup_music();
}

void ShadowGrinder::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Jump" && shadow_position_ == 1) {
            shadow_position_ = 2;
        }
    }
}

std::vector<std::string> ShadowGrinder::get_visible_elements() {
    std::vector<std::string> elements;

    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);

    elements.push_back(std::string("shadow-") + std::to_string(shadow_position_));

    if (shadow_position_ == 0) elements.push_back(MISS);

    for (int rail : rail_positions_) {
        elements.push_back("rail-" + std::to_string(rail));
    }

    return elements;
}

void ShadowGrinder::custom_update() {
    if (gaps_dodged_ >= 15 && !is_endless_mode_) {
        queue_sound("../common/level_up.ogg");
        misses_ = 0;
        gaps_dodged_ = 0;
        level_++;
        custom_update_speed_ -= 125;  // Speed up.
    }

    if (level_ == 4) {
        victory();
        return;
    }

    // Move rails forward.
    for (int& rail : rail_positions_) rail -= 1;

    if (!contains(rail_positions_, 0)) {
        if (shadow_position_ == 1) {
            shadow_position_ = 0;
            queue_sound("../common/miss.ogg");
            misses_++;
        } else if (shadow_position_ == 2) {
            queue_sound("../common/hit.ogg");
            gaps_dodged_++;
            increment_score();
        }
    }

    // Apply gravity to shadow - unless he's currently jumping over a gap.
    if (shadow_position_ != 1 && contains(rail_positions_, 0)) {
        shadow_position_ = 1;
    }

    if (misses_ == 10) {
        game_over();
        return;
    }

    // Always spawn a gap on first cycle.
    if (spawn_first_gap_) {
        spawn_first_gap_ = false;
    } else {
        // 50% chance of not spawning a new rail-7, creating a 1 rail-wide
        // gap (see ShadowGrinder.cs's own comment: the original game likely
        // loops through predefined gap patterns instead, but rng is close
        // enough and much simpler to port faithfully).
        if (rand_range(1, 2) != 1) {
            rail_positions_.push_back(7);
        } else if (!contains(rail_positions_, 6)) {
            rail_positions_.push_back(7);  // Prevent 2-rail-wide gaps.
        }
    }
}

void ShadowGrinder::game_over() {
    shadow_position_ = -1;
    level_ = 0;
    rail_positions_.clear();

    generic_game_over_animation({MISS, SHADOW_FALL, "rail-1", "rail-2", "rail-3", "rail-4", "rail-5", "rail-6", "rail-7"});
    stop();
}

void ShadowGrinder::victory() {
    shadow_position_ = -1;
    level_ = 0;
    rail_positions_.clear();

    generic_victory_animation({LEVEL_1, LEVEL_2, LEVEL_3});
    stop();
}
