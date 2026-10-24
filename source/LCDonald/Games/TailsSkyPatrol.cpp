#include "TailsSkyPatrol.h"

#include <algorithm>

namespace {
constexpr const char* MISS_CENTER = "miss-center";
constexpr const char* MISS_BOTTOM = "miss-bottom";
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* TAILS_TOP = "tails-top";
constexpr const char* TAILS_BOTTOM = "tails-bottom";
constexpr const char* TAILS_CENTER = "tails-center";

void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

std::vector<std::string> TailsSkyPatrol::get_all_game_elements() const {
    return {LEVEL_1,     LEVEL_2,     LEVEL_3,
            "ring-11",   "ring-21",   "ring-31",   "ring-41",   TAILS_TOP,
            MISS_CENTER,
            "ring-12",   "ring-22",   "ring-32",   "ring-42",   TAILS_CENTER,
            MISS_BOTTOM,
            "ring-13",   "ring-23",   "ring-33",   "ring-43",   TAILS_BOTTOM};
}

void TailsSkyPatrol::initialize_game_state() {
    tails_position_ = 2;
    rings_collected_ = 0;
    rings_missed_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    ring_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 400 : 900;
    startup_music();
}

void TailsSkyPatrol::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Up" && tails_position_ > 1) {
            tails_position_--;
        } else if (input == "Down" && tails_position_ < 3) {
            tails_position_++;
        }
    }
}

std::string TailsSkyPatrol::tails_element() const {
    if (tails_position_ == 1) return TAILS_TOP;
    if (tails_position_ == 2) return TAILS_CENTER;
    if (tails_position_ == 3) return TAILS_BOTTOM;
    return "";
}

std::vector<std::string> TailsSkyPatrol::get_visible_elements() {
    std::vector<std::string> elements;
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    elements.push_back(tails_element());
    for (int r : ring_positions_) elements.push_back("ring-" + std::to_string(r));
    return elements;
}

void TailsSkyPatrol::update_core() {
    for (int ring_pos : std::vector<int>(ring_positions_)) {
        const int digit = ring_pos % 10;
        if (digit == tails_position_ && ring_pos > 40) {
            queue_sound("../common/hit.ogg");
            remove_value(ring_positions_, ring_pos);
            rings_collected_++;
            increment_score();
        }
    }
}

void TailsSkyPatrol::custom_update() {
    if (rings_collected_ >= 30 && !is_endless_mode_) {
        queue_sound("../common/level_up.ogg");
        rings_missed_ = 0;
        rings_collected_ = 0;
        level_++;
        custom_update_speed_ -= 175;
    }

    if (level_ == 4) {
        victory();
        return;
    }

    // Move rings forward.
    for (int& r : ring_positions_) r += 10;

    for (int ring_pos : std::vector<int>(ring_positions_)) {
        if (ring_pos > 50) {
            queue_sound("../common/miss.ogg");
            remove_value(ring_positions_, ring_pos);
            rings_missed_++;
            block_custom_updates(1);

            const int vertical_pos = ring_pos % 10;
            if (vertical_pos == 1) {
                blink_element(MISS_CENTER, 1);
            } else if (vertical_pos == 2) {
                blink_element(MISS_CENTER, 1);
                blink_element(MISS_BOTTOM, 1);
            } else if (vertical_pos == 3) {
                blink_element(MISS_BOTTOM, 1);
            }
        }
    }

    if (rings_missed_ == 10) {
        game_over();
        return;
    }

    if (blocked_custom_updates_ > 0) {
        // If rings were missed, cancel the previous forward move.
        for (int& r : ring_positions_) r -= 10;
    } else {
        const int first_ring = rand_range(11, 13);
        ring_positions_.push_back(first_ring);

        if (rand_range(0, 1) == 0) {
            int second_ring = first_ring;
            do {
                second_ring = rand_range(11, 13);
            } while (second_ring == first_ring);
            ring_positions_.push_back(second_ring);
        }
    }
}

void TailsSkyPatrol::game_over() {
    tails_position_ = -1;
    level_ = 0;
    ring_positions_.clear();
    generic_game_over_animation({"ring-11", "ring-12", "ring-13", "ring-21", "ring-22", "ring-23",
                                  "ring-31", "ring-32", "ring-33", "ring-41", "ring-42", "ring-43"});
    stop();
}

void TailsSkyPatrol::victory() {
    tails_position_ = -1;
    level_ = 0;
    ring_positions_.clear();
    generic_victory_animation({LEVEL_1, LEVEL_2, LEVEL_3});
    stop();
}
