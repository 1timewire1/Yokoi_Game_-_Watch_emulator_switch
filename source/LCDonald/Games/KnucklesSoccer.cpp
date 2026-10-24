#include "KnucklesSoccer.h"

namespace {
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* KNUCKLES_LEFT = "knux-left";
constexpr const char* KNUCKLES_RIGHT = "knux-right";
constexpr const char* KNUCKLES_CENTER = "knux-center";
constexpr const char* BALL_MISS = "ball-miss";
}  // namespace

std::vector<std::string> KnucklesSoccer::get_all_game_elements() const {
    return {LEVEL_1,          LEVEL_2,          LEVEL_3,
            "ball-11",        "ball-12",        "ball-13",
            "ball-21",        "ball-22",        "ball-23",
            "ball-31",        "ball-32",        "ball-33",
            "ball-41",        "ball-42",        "ball-43",
            KNUCKLES_LEFT,    KNUCKLES_CENTER,  KNUCKLES_RIGHT,
            BALL_MISS};
}

void KnucklesSoccer::initialize_game_state() {
    knuckles_position_ = 2;
    level_ = is_endless_mode_ ? 5 : 0;

    balls_intercepted_ = 0;
    balls_missed_ = 0;

    ball_pos_ = -1;

    custom_update_speed_ = is_endless_mode_ ? 200 : 500;
    startup_music();
}

void KnucklesSoccer::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && knuckles_position_ > 1) {
            knuckles_position_--;
        } else if (input == "Right" && knuckles_position_ < 3) {
            knuckles_position_++;
        }
    }
}

std::string KnucklesSoccer::knuckles_element() const {
    if (knuckles_position_ == 1) return KNUCKLES_LEFT;
    if (knuckles_position_ == 2) return KNUCKLES_CENTER;
    if (knuckles_position_ == 3) return KNUCKLES_RIGHT;
    return "";
}

std::vector<std::string> KnucklesSoccer::get_visible_elements() {
    std::vector<std::string> elements;

    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);

    elements.push_back(knuckles_element());

    if (ball_pos_ != -1) {
        elements.push_back("ball-" + std::to_string(ball_pos_));
    }

    return elements;
}

void KnucklesSoccer::custom_update() {
    if (balls_intercepted_ == 10 && !is_endless_mode_) {
        queue_sound("../common/level_up.ogg");
        balls_intercepted_ = 0;
        balls_missed_ = 0;
        level_++;
        custom_update_speed_ -= 75;  // Speed up.
    }

    if (level_ == 4) {
        victory();
        return;
    }

    if (ball_pos_ != -1) {
        ball_pos_ += 10;

        if (ball_pos_ > 50) {
            const int digit = ball_pos_ % 10;
            ball_pos_ = -1;
            if (digit == knuckles_position_) {
                balls_intercepted_++;
                increment_score();
                queue_sound("../common/hit.ogg");
            } else {
                balls_missed_++;
                blink_element(BALL_MISS, 1);
                queue_sound("../common/miss.ogg");
            }

            if (balls_missed_ == 5) game_over();
        } else if (ball_pos_ < 40) {
            // Randomly shift ball left or right.
            ball_pos_ += rand_range(-1, 1);

            // Adjust out of bounds (valid columns are 1-3).
            if (ball_pos_ % 10 == 0) ball_pos_++;
            if (ball_pos_ % 10 == 4) ball_pos_--;
        }
    } else {
        // Randomly spawn ball at 11, 12 or 13.
        ball_pos_ = rand_range(11, 13);
    }
}

void KnucklesSoccer::game_over() {
    knuckles_position_ = -1;
    ball_pos_ = -1;

    generic_game_over_animation({BALL_MISS, KNUCKLES_CENTER});
    stop();
}

void KnucklesSoccer::victory() {
    knuckles_position_ = -1;
    ball_pos_ = -1;

    generic_victory_animation({LEVEL_1, LEVEL_2, LEVEL_3});
    stop();
}
