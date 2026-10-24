#include "AiAiBananaCatch.h"

#include <algorithm>

namespace {
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* AIAI_LEFT = "aiai-left";
constexpr const char* AIAI_RIGHT = "aiai-right";
constexpr const char* AIAI_CENTER = "aiai-center";
constexpr const char* MISS_LEFT = "miss-left";
constexpr const char* MISS_RIGHT = "miss-right";

void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

std::vector<std::string> AiAiBananaCatch::get_all_game_elements() const {
    return {LEVEL_1,      LEVEL_2,      LEVEL_3,
            "banana-11",  "banana-12",  "banana-13",
            "banana-21",  "banana-22",  "banana-23",
            "banana-31",  "banana-32",  "banana-33",
            "banana-41",  "banana-42",  "banana-43",
            MISS_LEFT,    MISS_RIGHT,
            AIAI_LEFT,    AIAI_CENTER,  AIAI_RIGHT};
}

void AiAiBananaCatch::initialize_game_state() {
    aiai_position_ = 2;
    bananas_collected_ = 0;
    bananas_missed_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    banana_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 400 : 900;
    startup_music();
}

void AiAiBananaCatch::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && aiai_position_ > 1) {
            aiai_position_--;
        } else if (input == "Right" && aiai_position_ < 3) {
            aiai_position_++;
        }
    }
}

std::string AiAiBananaCatch::aiai_element() const {
    if (aiai_position_ == 1) return AIAI_LEFT;
    if (aiai_position_ == 2) return AIAI_CENTER;
    if (aiai_position_ == 3) return AIAI_RIGHT;
    return "";
}

std::vector<std::string> AiAiBananaCatch::get_visible_elements() {
    std::vector<std::string> elements;
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    elements.push_back(aiai_element());
    for (int r : banana_positions_) elements.push_back("banana-" + std::to_string(r));
    return elements;
}

void AiAiBananaCatch::update_core() {
    for (int ban_pos : std::vector<int>(banana_positions_)) {
        const int digit = ban_pos % 10;
        if (digit == aiai_position_ && ban_pos > 40) {
            queue_sound("../common/hit.ogg");
            remove_value(banana_positions_, ban_pos);
            bananas_collected_++;
            increment_score();
        }
    }
}

void AiAiBananaCatch::custom_update() {
    if (bananas_collected_ >= 30 && !is_endless_mode_) {
        queue_sound("../common/level_up.ogg");
        bananas_missed_ = 0;
        bananas_collected_ = 0;
        level_++;
        custom_update_speed_ -= 175;
    }

    if (level_ == 4) {
        victory();
        return;
    }

    for (int& r : banana_positions_) r += 10;

    for (int ring_pos : std::vector<int>(banana_positions_)) {
        if (ring_pos > 50) {
            queue_sound("../common/miss.ogg");
            remove_value(banana_positions_, ring_pos);
            bananas_missed_++;
            block_custom_updates(1);

            const int vertical_pos = ring_pos % 10;
            if (vertical_pos == 1) {
                blink_element(MISS_LEFT, 1);
            } else if (vertical_pos == 2) {
                blink_element(MISS_LEFT, 1);
                blink_element(MISS_RIGHT, 1);
            } else if (vertical_pos == 3) {
                blink_element(MISS_RIGHT, 1);
            }
        }
    }

    if (bananas_missed_ == 10) {
        game_over();
        return;
    }

    if (blocked_custom_updates_ > 0) {
        for (int& r : banana_positions_) r -= 10;
    } else {
        const int first_ring = rand_range(11, 13);
        banana_positions_.push_back(first_ring);

        if (rand_range(0, 1) == 0) {
            int second_ring = first_ring;
            do {
                second_ring = rand_range(11, 13);
            } while (second_ring == first_ring);
            banana_positions_.push_back(second_ring);
        }
    }
}

void AiAiBananaCatch::game_over() {
    aiai_position_ = -1;
    level_ = 0;
    banana_positions_.clear();
    generic_game_over_animation({"banana-11", "banana-12", "banana-13", "banana-21", "banana-22", "banana-23",
                                  "banana-31", "banana-32", "banana-33", "banana-41", "banana-42", "banana-43"});
    stop();
}

void AiAiBananaCatch::victory() {
    aiai_position_ = -1;
    level_ = 0;
    banana_positions_.clear();
    generic_victory_animation({LEVEL_1, LEVEL_2, LEVEL_3});
    stop();
}
