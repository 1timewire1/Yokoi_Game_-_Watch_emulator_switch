#include "SonicSpeedway.h"

#include <algorithm>

namespace {
constexpr const char* HIT_LEFT = "hit-left";
constexpr const char* HIT_RIGHT = "hit-right";
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* SONIC_CENTER = "sonic-center";
constexpr const char* SONIC_LEFT = "sonic-left";
constexpr const char* SONIC_RIGHT = "sonic-right";

void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

std::vector<std::string> SonicSpeedway::get_all_game_elements() const {
    return {LEVEL_1,      LEVEL_2,      LEVEL_3,
            "car-11",     "car-12",     "car-13",
            "car-21",     "car-22",     "car-23",
            "car-31",     "car-32",     "car-33",
            "car-41",     "car-42",     "car-43",
            HIT_LEFT,     HIT_RIGHT,
            SONIC_LEFT,   SONIC_CENTER, SONIC_RIGHT};
}

void SonicSpeedway::initialize_game_state() {
    sonic_position_ = 2;
    car_rows_dodged_ = 0;
    cars_hit_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    car_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 300 : 800;
    startup_music();
}

void SonicSpeedway::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && sonic_position_ > 1) {
            sonic_position_--;
        } else if (input == "Right" && sonic_position_ < 3) {
            sonic_position_++;
        }
    }
}

std::string SonicSpeedway::sonic_element() const {
    if (sonic_position_ == 1) return SONIC_LEFT;
    if (sonic_position_ == 2) return SONIC_CENTER;
    if (sonic_position_ == 3) return SONIC_RIGHT;
    return "";
}

std::vector<std::string> SonicSpeedway::get_visible_elements() {
    std::vector<std::string> elements;
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    elements.push_back(sonic_element());
    for (int c : car_positions_) elements.push_back("car-" + std::to_string(c));
    return elements;
}

void SonicSpeedway::custom_update() {
    if (car_rows_dodged_ >= 30 && !is_endless_mode_) {
        queue_sound("../common/level_up.ogg");
        cars_hit_ = 0;
        car_rows_dodged_ = 0;
        level_++;
        custom_update_speed_ -= 125;
    }

    if (level_ == 4) {
        victory();
        return;
    }

    for (int& c : car_positions_) c += 10;

    bool has_dodged_car = false;
    bool has_been_hit = false;
    for (int car_pos : std::vector<int>(car_positions_)) {
        if (car_pos > 50) {
            remove_value(car_positions_, car_pos);
            const int horizontal_pos = car_pos % 10;

            if (horizontal_pos == sonic_position_) {
                queue_sound("../common/miss.ogg");
                cars_hit_++;
                has_been_hit = true;

                if (horizontal_pos == 1) {
                    blink_element(HIT_LEFT, 1);
                } else if (horizontal_pos == 2) {
                    blink_element(HIT_LEFT, 1);
                    blink_element(HIT_RIGHT, 1);
                } else if (horizontal_pos == 3) {
                    blink_element(HIT_RIGHT, 1);
                }
            } else {
                has_dodged_car = true;
            }
        }
    }

    if (has_dodged_car && !has_been_hit) {
        queue_sound("../common/hit.ogg");
        car_rows_dodged_++;
        increment_score();
    }

    if (cars_hit_ == 5) {
        game_over();
        return;
    }

    const int first_car = rand_range(11, 13);
    car_positions_.push_back(first_car);

    if (rand_range(0, 1) == 0) {
        int second_car = first_car;
        do {
            second_car = rand_range(11, 13);
        } while (second_car == first_car);
        car_positions_.push_back(second_car);
    }
}

void SonicSpeedway::game_over() {
    sonic_position_ = -1;
    level_ = 0;
    car_positions_.clear();
    generic_game_over_animation({SONIC_CENTER, "car-42", HIT_LEFT, HIT_RIGHT});
    stop();
}

void SonicSpeedway::victory() {
    sonic_position_ = -1;
    level_ = 0;
    car_positions_.clear();
    generic_victory_animation({LEVEL_1, LEVEL_2, LEVEL_3});
    stop();
}
