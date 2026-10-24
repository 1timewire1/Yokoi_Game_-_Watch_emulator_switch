#include "BillyGiantEgg.h"

#include <algorithm>

namespace {
constexpr const char* EGG_1 = "egg-1";
constexpr const char* EGG_2 = "egg-2";
constexpr const char* EGG_3 = "egg-3";
constexpr const char* EGG_4 = "egg-4";
constexpr const char* GET_LEFT = "get-left";
constexpr const char* GET_RIGHT = "get-right";
constexpr const char* BILLYCENTER = "billy-center";
constexpr const char* BILLYLEFT = "billy-left";
constexpr const char* BILLYRIGHT = "billy-right";

bool contains(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}
}  // namespace

std::vector<std::string> BillyGiantEgg::get_all_game_elements() const {
    std::vector<std::string> elements = {EGG_1, EGG_2, EGG_3, EGG_4,
            "fruit-11", "fruit-12", "fruit-13",
            "bomb-11", "bomb-12", "bomb-13",
            "fruit-21", "fruit-22", "fruit-23",
            "bomb-21", "bomb-22", "bomb-23",
            "fruit-31", "fruit-32", "fruit-33",
            "bomb-31", "bomb-32", "bomb-33",
            GET_LEFT, GET_RIGHT,
            BILLYLEFT, BILLYCENTER, BILLYRIGHT};
    if (burger_) {
        // Burger's additional "common" group.
        elements.insert(elements.end(), {"common-11", "common-12", "common-13",
                                          "common-21", "common-22", "common-23",
                                          "common-31", "common-32", "common-33"});
    }
    return elements;
}

void BillyGiantEgg::initialize_game_state() {
    billy_position_ = 2;
    fruits_collected_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    bomb_positions_.clear();
    fruit_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 250 : 950;
    startup_music("game_start.ogg");
}

void BillyGiantEgg::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && billy_position_ > 1) {
            billy_position_--;
        } else if (input == "Right" && billy_position_ < 3) {
            billy_position_++;
        }
    }
}

std::string BillyGiantEgg::billy_element() const {
    if (billy_position_ == 1) return BILLYLEFT;
    if (billy_position_ == 2) return BILLYCENTER;
    if (billy_position_ == 3) return BILLYRIGHT;
    return "";
}

std::vector<std::string> BillyGiantEgg::get_visible_elements() {
    std::vector<std::string> elements;
    elements.push_back(billy_element());
    for (int b : bomb_positions_) elements.push_back("bomb-" + std::to_string(b));
    for (int f : fruit_positions_) elements.push_back("fruit-" + std::to_string(f));

    if (fruits_collected_ >= 5) elements.push_back(EGG_1);
    if (fruits_collected_ >= 10) elements.push_back(EGG_2);
    if (fruits_collected_ >= 15) elements.push_back(EGG_3);

    if (burger_) {
        // Every bomb and fruit also lights a "common" group at its position,
        // so they can share some design features.
        for (int b : bomb_positions_) elements.push_back("common-" + std::to_string(b));
        for (int f : fruit_positions_) elements.push_back("common-" + std::to_string(f));
    }

    return elements;
}

void BillyGiantEgg::update_core() {
    for (int fruit_pos : std::vector<int>(fruit_positions_)) {
        if (fruit_pos <= 30) continue;
        const int horizontal_pos = fruit_pos % 10;

        if (horizontal_pos == billy_position_) {
            fruit_positions_.erase(std::remove(fruit_positions_.begin(), fruit_positions_.end(), fruit_pos),
                                    fruit_positions_.end());
            increment_score();

            if (horizontal_pos == 1 || horizontal_pos == 2) blink_element(GET_LEFT, 1);
            if (horizontal_pos == 3 || horizontal_pos == 2) blink_element(GET_RIGHT, 1);

            queue_sound("hit.ogg");
            if (fruits_collected_ < 0) {
                fruits_collected_ = 1;
            } else {
                fruits_collected_++;
            }

            if (fruits_collected_ % 5 == 0 && fruits_collected_ != 20) queue_sound("egg_up.ogg");
        }
    }
}

void BillyGiantEgg::custom_update() {
    if (fruits_collected_ >= 20 && !is_endless_mode_) {
        if (level_ == 3) {
            victory();
            return;
        } else {
            level_up();
            custom_update_speed_ -= 150;
        }
    }

    for (int& b : bomb_positions_) b += 10;
    for (int& f : fruit_positions_) f += 10;

    for (int bomb_pos : std::vector<int>(bomb_positions_)) {
        if (bomb_pos <= 40) continue;
        bomb_positions_.erase(std::remove(bomb_positions_.begin(), bomb_positions_.end(), bomb_pos),
                               bomb_positions_.end());
        const int horizontal_pos = bomb_pos % 10;

        if (horizontal_pos == billy_position_) {
            queue_sound("miss.ogg");
            bombs_hit_++;

            if (bombs_hit_ >= 2) {
                bombs_hit_ = 0;
                if (fruits_collected_ < 5) {
                    game_over();
                } else {
                    fruits_collected_ = (fruits_collected_ >= 15) ? 10 : (fruits_collected_ >= 10) ? 5 : 0;
                }
            }
        }
    }

    for (int fruit_pos : std::vector<int>(fruit_positions_)) {
        if (fruit_pos > 40) {
            fruit_positions_.erase(std::remove(fruit_positions_.begin(), fruit_positions_.end(), fruit_pos),
                                    fruit_positions_.end());
        }
    }

    if (fruits_collected_ == -2) {
        game_over();
        return;
    }

    if (rand_range(0, 99) < 60) {
        const int first_bomb = rand_range(11, 13);
        bomb_positions_.push_back(first_bomb);

        if (rand_range(0, 99) < 33) {
            int second_bomb = first_bomb;
            do {
                second_bomb = rand_range(11, 13);
            } while (second_bomb == first_bomb);
            bomb_positions_.push_back(second_bomb);
        }

        if (rand_range(0, 99) < 50) spawn_fruits();
    } else {
        spawn_fruits();
    }
}

void BillyGiantEgg::spawn_fruits() {
    const int fruit_pos = rand_range(1, 3) + 10;
    if (!contains(bomb_positions_, fruit_pos)) fruit_positions_.push_back(fruit_pos);

    if (rand_range(0, 99) < 33) {
        const int second_fruit = (fruit_pos == 11) ? 13 : 11;
        if (!contains(bomb_positions_, second_fruit)) fruit_positions_.push_back(second_fruit);
    }
}

void BillyGiantEgg::level_up() {
    input_blocked_ = true;
    fruit_positions_.clear();
    bomb_positions_.clear();
    fruits_collected_ = 0;
    billy_position_ = 2;

    if (burger_) {
        // Eggs 1-3 should still be onscreen when egg-4 is present.
        blink_element(EGG_1, 6);
        blink_element(EGG_2, 6);
        blink_element(EGG_3, 6);
    }
    blink_element(EGG_4, 6);
    queue_sound("egg_hatch.ogg");

    const std::vector<std::string> f1 = {BILLYCENTER, EGG_1, EGG_2, EGG_3};
    const std::vector<std::string> f2 = {BILLYCENTER, EGG_4};
    const std::vector<std::string> f3 = {BILLYCENTER};

    std::vector<std::vector<std::string>> animation = {f1, f1};
    for (int i = 0; i < 7; i++) animation.insert(animation.end(), {f2, f2, f3, f3});
    play_animation(animation);
    level_++;
}

void BillyGiantEgg::game_over() {
    billy_position_ = -1;
    level_ = 0;
    bomb_positions_.clear();
    fruit_positions_.clear();
    generic_game_over_animation({BILLYCENTER, "bomb-31", "bomb-32", "bomb-33"}, "game_over.ogg");
    stop();
}

void BillyGiantEgg::victory() {
    billy_position_ = -1;
    level_ = 0;
    bomb_positions_.clear();
    fruit_positions_.clear();

    queue_sound("game_win.ogg");

    const std::vector<std::string> f1 = {BILLYCENTER, EGG_1};
    std::vector<std::string> f2 = {BILLYCENTER, EGG_2};
    const std::vector<std::string> f3 = {BILLYCENTER, EGG_3};
    std::vector<std::string> f4 = {BILLYCENTER, EGG_4};
    if (burger_) {
        f2.push_back("win");
        f4.push_back("win");
    }

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 7; i++) {
        animation.insert(animation.end(), {f1, f1, f2, f2, f3, f3, f4, f4});
    }
    play_animation(animation);
    stop();
}
