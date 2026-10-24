#include "CreamFlowerCatch2005.h"

#include <algorithm>

namespace {
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* HIT_LEFT = "hit-left";
constexpr const char* HIT_CENTER = "hit-center";
constexpr const char* HIT_RIGHT = "hit-right";
constexpr const char* CREAM_LEFT = "cream-left";
constexpr const char* CREAM_RIGHT = "cream-right";
constexpr const char* CREAM_CENTER = "cream-center";
constexpr const char* MISS_LEFT = "miss-left";
constexpr const char* MISS_RIGHT = "miss-right";
constexpr const char* MISS_1 = "miss-1";
constexpr const char* MISS_2 = "miss-2";
constexpr const char* MISS_3 = "miss-3";
constexpr const char* MISS_4 = "miss-4";
constexpr const char* MISS_5 = "miss-5";

void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

std::vector<std::string> CreamFlowerCatch2005::get_all_game_elements() const {
    return {LEVEL_1,     LEVEL_2,     LEVEL_3,
            "flower-11", "flower-12", "flower-13",
            "flower-21", "flower-22", "flower-23",
            "flower-31", "flower-32", "flower-33",
            "flower-41", HIT_LEFT, HIT_CENTER, "flower-42", HIT_RIGHT, "flower-43",
            CREAM_LEFT,  CREAM_CENTER, CREAM_RIGHT,
            MISS_LEFT,   MISS_1, MISS_2, MISS_3, MISS_4, MISS_5, MISS_RIGHT};
}

void CreamFlowerCatch2005::initialize_game_state() {
    cream_position_ = 2;
    flowers_collected_ = 0;
    flowers_missed_ = is_endless_mode_ ? 4 : 0;
    level_ = is_endless_mode_ ? 5 : 0;
    flower_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 600 : 900;
    startup_music("game_start.ogg");
}

void CreamFlowerCatch2005::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && cream_position_ > 1) {
            cream_position_--;
        } else if (input == "Right" && cream_position_ < 3) {
            cream_position_++;
        }
    }
}

std::string CreamFlowerCatch2005::cream_element() const {
    if (cream_position_ == 1) return CREAM_LEFT;
    if (cream_position_ == 2) return CREAM_CENTER;
    if (cream_position_ == 3) return CREAM_RIGHT;
    return "";
}

std::vector<std::string> CreamFlowerCatch2005::get_visible_elements() {
    std::vector<std::string> elements;
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    elements.push_back(cream_element());
    for (int r : flower_positions_) elements.push_back("flower-" + std::to_string(r));

    if (flowers_missed_ >= 1) elements.push_back(MISS_1);
    if (flowers_missed_ >= 2) elements.push_back(MISS_2);
    if (flowers_missed_ >= 3) elements.push_back(MISS_3);
    if (flowers_missed_ >= 4) elements.push_back(MISS_4);
    if (flowers_missed_ >= 5) elements.push_back(MISS_5);

    return elements;
}

void CreamFlowerCatch2005::update_core() {
    for (int f_pos : std::vector<int>(flower_positions_)) {
        const int digit = f_pos % 10;
        if (digit == cream_position_ && f_pos > 40) {
            queue_sound("hit.ogg");
            remove_value(flower_positions_, f_pos);
            flowers_collected_++;
            increment_score();

            if (digit == 1) blink_element(HIT_LEFT, 1);
            else if (digit == 2) blink_element(HIT_CENTER, 1);
            else if (digit == 3) blink_element(HIT_RIGHT, 1);
        }
    }
}

void CreamFlowerCatch2005::custom_update() {
    if (flowers_collected_ >= 30 && !is_endless_mode_) {
        queue_sound("level_up.ogg");
        flowers_missed_ = 0;
        flowers_collected_ = 0;
        level_++;

        if (level_ >= 1) blink_element(LEVEL_1, 3);
        if (level_ >= 2) blink_element(LEVEL_2, 3);
        if (level_ >= 3) blink_element(LEVEL_3, 3);

        custom_update_speed_ -= 175;
        block_custom_updates(1);
        return;
    }

    if (level_ == 4) {
        victory();
        return;
    }

    for (int& r : flower_positions_) r += 10;

    for (int flower_pos : std::vector<int>(flower_positions_)) {
        if (flower_pos > 50) {
            queue_sound("miss.ogg");
            remove_value(flower_positions_, flower_pos);
            flowers_missed_++;
            blink_element(MISS_LEFT, 1);
            blink_element(MISS_RIGHT, 1);
        }
    }

    if (flowers_missed_ == 5) {
        game_over();
        return;
    }

    const int first_flower = rand_range(11, 13);
    flower_positions_.push_back(first_flower);

    if (rand_range(0, 1) == 0 && level_ >= 2) {
        int second_flower = first_flower;
        do {
            second_flower = rand_range(11, 13);
        } while (second_flower == first_flower);
        flower_positions_.push_back(second_flower);
    }
}

void CreamFlowerCatch2005::game_over() {
    cream_position_ = -1;
    level_ = 0;
    flower_positions_.clear();
    generic_game_over_animation({MISS_LEFT, MISS_1, MISS_2, MISS_3, MISS_4, MISS_5, MISS_RIGHT}, "game_over.ogg");
    stop();
}

void CreamFlowerCatch2005::victory() {
    cream_position_ = -1;
    level_ = 0;
    flower_positions_.clear();

    queue_sound("game_win.ogg");

    const std::vector<std::string> f1l = {"flower-41", "flower-42", "flower-43", CREAM_LEFT};
    std::vector<std::string> f2l = {"flower-31", "flower-32", "flower-33", CREAM_LEFT};
    const std::vector<std::string> f3l = {"flower-21", "flower-22", "flower-23", CREAM_LEFT};
    std::vector<std::string> f4l = {"flower-11", "flower-12", "flower-13", CREAM_LEFT};
    const std::vector<std::string> f1c = {"flower-41", "flower-42", "flower-43", CREAM_CENTER};
    std::vector<std::string> f2c = {"flower-31", "flower-32", "flower-33", CREAM_CENTER};
    const std::vector<std::string> f3c = {"flower-21", "flower-22", "flower-23", CREAM_CENTER};
    std::vector<std::string> f4c = {"flower-11", "flower-12", "flower-13", CREAM_CENTER};
    const std::vector<std::string> f1r = {"flower-41", "flower-42", "flower-43", CREAM_RIGHT};
    std::vector<std::string> f2r = {"flower-31", "flower-32", "flower-33", CREAM_RIGHT};
    const std::vector<std::string> f3r = {"flower-21", "flower-22", "flower-23", CREAM_RIGHT};
    std::vector<std::string> f4r = {"flower-11", "flower-12", "flower-13", CREAM_RIGHT};
    if (burger_) {
        for (std::vector<std::string>* f : {&f2l, &f2c, &f2r, &f4l, &f4c, &f4r}) f->push_back("win");
    }

    std::vector<std::vector<std::string>> animation = {
        f1l, f1l, f1c, f1c, f2r, f2r, f2l, f2l,
        f3c, f3c, f3r, f3r, f4l, f4l, f4c, f4c,
        f1r, f1r, f1l, f1l, f2c, f2c, f2r, f2r,
        f3l, f3l, f3c, f3c, f4r, f4r, f4l, f4l,
        f1c, f1c, f1r, f1r, f2l, f2l, f2c, f2c,
        f3r, f3r, f3l, f3l, f4c, f4c, f4r, f4r};
    play_animation(animation);
    stop();
}
