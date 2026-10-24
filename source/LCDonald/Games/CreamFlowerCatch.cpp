#include "CreamFlowerCatch.h"

#include <algorithm>

namespace {
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* CREAM_LEFT = "cream-left";
constexpr const char* CREAM_RIGHT = "cream-right";
constexpr const char* CREAM_CENTER = "cream-center";
constexpr const char* MISS_LEFT = "miss-left";
constexpr const char* MISS_RIGHT = "miss-right";

void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

std::vector<std::string> CreamFlowerCatch::get_all_game_elements() const {
    return {LEVEL_1,      LEVEL_2,      LEVEL_3,
            "flower-11",  "flower-12",  "flower-13",
            "flower-21",  "flower-22",  "flower-23",
            "flower-31",  "flower-32",  "flower-33",
            "flower-41",  "flower-42",  "flower-43",
            MISS_LEFT,    MISS_RIGHT,
            CREAM_LEFT,   CREAM_CENTER, CREAM_RIGHT};
}

void CreamFlowerCatch::initialize_game_state() {
    cream_position_ = 2;
    flowers_collected_ = 0;
    flowers_missed_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    flower_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 600 : 900;
    startup_music();
}

void CreamFlowerCatch::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && cream_position_ > 1) {
            cream_position_--;
        } else if (input == "Right" && cream_position_ < 3) {
            cream_position_++;
        }
    }
}

std::string CreamFlowerCatch::cream_element() const {
    if (cream_position_ == 1) return CREAM_LEFT;
    if (cream_position_ == 2) return CREAM_CENTER;
    if (cream_position_ == 3) return CREAM_RIGHT;
    return "";
}

std::vector<std::string> CreamFlowerCatch::get_visible_elements() {
    std::vector<std::string> elements;
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    elements.push_back(cream_element());
    for (int r : flower_positions_) elements.push_back("flower-" + std::to_string(r));
    return elements;
}

void CreamFlowerCatch::update_core() {
    for (int f_pos : std::vector<int>(flower_positions_)) {
        const int digit = f_pos % 10;
        if (digit == cream_position_ && f_pos > 40) {
            queue_sound("../common/hit.ogg");
            remove_value(flower_positions_, f_pos);
            flowers_collected_++;
            increment_score();
        }
    }
}

void CreamFlowerCatch::custom_update() {
    if (flowers_collected_ >= 30 && !is_endless_mode_) {
        queue_sound("../common/level_up.ogg");
        flowers_missed_ = 0;
        flowers_collected_ = 0;
        level_++;
        custom_update_speed_ -= 175;
    }

    if (level_ == 4) {
        victory();
        return;
    }

    for (int& r : flower_positions_) r += 10;

    for (int ring_pos : std::vector<int>(flower_positions_)) {
        if (ring_pos > 50) {
            queue_sound("../common/miss.ogg");
            remove_value(flower_positions_, ring_pos);
            flowers_missed_++;

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

    if (flowers_missed_ == 10) {
        game_over();
        return;
    }

    const int first_flower = rand_range(11, 13);
    flower_positions_.push_back(first_flower);

    if (rand_range(0, 1) == 0) {
        int second_flower = first_flower;
        do {
            second_flower = rand_range(11, 13);
        } while (second_flower == first_flower);
        flower_positions_.push_back(second_flower);
    }
}

void CreamFlowerCatch::game_over() {
    cream_position_ = -1;
    level_ = 0;
    flower_positions_.clear();
    generic_game_over_animation({"flower-11", "flower-12", "flower-13", "flower-21", "flower-22", "flower-23",
                                  "flower-31", "flower-32", "flower-33", "flower-41", "flower-42", "flower-43"});
    stop();
}

void CreamFlowerCatch::victory() {
    cream_position_ = -1;
    level_ = 0;
    flower_positions_.clear();
    generic_victory_animation({LEVEL_1, LEVEL_2, LEVEL_3});
    stop();
}
