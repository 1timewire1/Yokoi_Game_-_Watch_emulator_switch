#include "ShadowBasketball.h"

#include <algorithm>

namespace {
constexpr const char* SEGMENT_1 = "segment-1";
constexpr const char* SEGMENT_21 = "segment-21";
constexpr const char* SEGMENT_22 = "segment-22";
constexpr const char* SEGMENT_23 = "segment-23";
constexpr const char* SEGMENT_24 = "segment-24";
constexpr const char* SEGMENT_25 = "segment-25";
constexpr const char* SEGMENT_26 = "segment-26";
constexpr const char* SEGMENT_27 = "segment-27";
constexpr const char* SEGMENT_28 = "segment-28";
constexpr const char* SHADOW_1 = "shadow-1";
constexpr const char* SHADOW_2 = "shadow-2";
constexpr const char* SHADOW_3 = "shadow-3";
constexpr const char* OMEGA_2 = "omega-2";
constexpr const char* HANDS_22 = "hands-22";

bool contains(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}
}  // namespace

std::vector<std::string> ShadowBasketball::get_all_game_elements() const {
    return {SEGMENT_21,
            SEGMENT_26, SEGMENT_22,
            SEGMENT_1, SEGMENT_27, SEGMENT_28,
            SEGMENT_25, SEGMENT_23,
            "ball-32", SEGMENT_24,
            "ball-21", "ball-22", "ball-23",
            "hands-21", "hands-22", "hands-23",
            "omega-1", "omega-2", "omega-3",
            "hands-1", "hands-2", "hands-3",
            "ball-11", "ball-12", "ball-13",
            SHADOW_1, SHADOW_2, SHADOW_3};
}

std::vector<std::string> ShadowBasketball::get_visible_elements() {
    std::vector<std::string> elements = {"ball-" + std::to_string(ball_position_),
                                          "shadow-" + std::to_string(shadow_position_)};

    for (int pos : omega_positions_) elements.push_back("omega-" + std::to_string(pos));
    for (int pos : hand_positions_) elements.push_back("hands-" + std::to_string(pos));

    switch (goals_scored_ % 10) {
        case 0:
            elements.insert(elements.end(), {SEGMENT_21, SEGMENT_22, SEGMENT_23, SEGMENT_24, SEGMENT_25, SEGMENT_26});
            break;
        case 2:
            elements.insert(elements.end(), {SEGMENT_21, SEGMENT_22, SEGMENT_27, SEGMENT_28, SEGMENT_25, SEGMENT_24});
            break;
        case 4:
            elements.insert(elements.end(), {SEGMENT_26, SEGMENT_22, SEGMENT_27, SEGMENT_28, SEGMENT_23});
            break;
        case 6:
            elements.insert(elements.end(), {SEGMENT_21, SEGMENT_26, SEGMENT_27, SEGMENT_28, SEGMENT_23, SEGMENT_25, SEGMENT_24});
            break;
        case 8:
            elements.insert(elements.end(), {SEGMENT_21, SEGMENT_22, SEGMENT_23, SEGMENT_24, SEGMENT_25, SEGMENT_26, SEGMENT_27, SEGMENT_28});
            break;
    }

    if (goals_scored_ > 8) elements.push_back(SEGMENT_1);

    return elements;
}

void ShadowBasketball::initialize_game_state() {
    shadow_position_ = 2;
    omega_positions_ = {2};
    ball_position_ = 0;
    hand_positions_ = {2};
    goals_scored_ = 0;
    balls_intercepted_ = 0;
    level_ = is_endless_mode_ ? 6 : 0;
    custom_update_speed_ = is_endless_mode_ ? 200 : 800;

    if (is_endless_mode_) {
        add_omega();
        add_omega();
    }

    startup_music();
}

void ShadowBasketball::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && shadow_position_ > 1) {
            shadow_position_--;
        } else if (input == "Right" && shadow_position_ < 3) {
            shadow_position_++;
        } else if (input == "Basket" && ball_position_ == 0) {
            ball_position_ = shadow_position_ + 10;
            set_ball_slowdown(true);
        }
    }
}

void ShadowBasketball::set_ball_slowdown(bool slowdown) {
    if (slowdown) {
        custom_update_speed_ += 150;
    } else {
        custom_update_speed_ -= 150;
    }
}

void ShadowBasketball::update_core() {
    if (ball_position_ == 0) return;

    if (ball_half_step_) {
        ball_half_step_ = false;
        ball_position_ += 10;
    } else {
        ball_half_step_ = true;
    }

    if (ball_position_ > 40) {
        queue_sound("../common/hit.ogg");
        goals_scored_ += 2;
        increment_score();
        ball_position_ = -10;
        ball_half_step_ = true;
        set_ball_slowdown(false);
        return;
    }

    if (ball_position_ > 23) ball_position_ = 32;

    if (contains(hand_positions_, ball_position_)) {
        balls_intercepted_++;
        input_blocked_ = true;
        queue_sound("../common/miss.ogg");

        blink_element("omega-" + std::to_string(ball_position_ % 10), 2);
        blink_element("hands-" + std::to_string(ball_position_), 2);
        blink_element("ball-" + std::to_string(ball_position_), 2);

        ball_position_ = 0;
        set_ball_slowdown(false);
    }
}

void ShadowBasketball::custom_update() {
    input_blocked_ = false;

    if (goals_scored_ >= 18 && !is_endless_mode_) {
        input_blocked_ = true;
        queue_sound("../common/level_up.ogg");
        balls_intercepted_ = 0;
        goals_scored_ = 0;
        level_++;
        custom_update_speed_ -= 100;

        omega_positions_.clear();
        hand_positions_.clear();

        const int pos = rand_range(1, 3);
        omega_positions_.push_back(pos);
        hand_positions_.push_back(pos);

        if (level_ >= 2) add_omega();
        if (level_ == 4) add_omega();
        return;
    }

    if (level_ == 5) {
        victory();
        return;
    }

    if (omega_positions_.size() < 3) {
        for (size_t i = 0; i < omega_positions_.size(); i++) {
            const int pos = omega_positions_[i];
            int new_pos = pos + rand_range(-1, 1);
            if (new_pos == 0) new_pos = 3;
            if (new_pos == 4) new_pos = 1;

            if (new_pos != pos && contains(omega_positions_, new_pos)) {
                std::vector<int> l = {1, 2, 3};
                l.erase(std::remove(l.begin(), l.end(), pos), l.end());
                l.erase(std::remove(l.begin(), l.end(), new_pos), l.end());
                new_pos = l.front();
            }

            omega_positions_[i] = new_pos;
            hand_positions_[i] = new_pos;
        }
    }

    if (ball_position_ == 0) {
        for (size_t i = 0; i < hand_positions_.size(); i++) {
            hand_positions_[i] = (rand_range(0, 2) == 0) ? (hand_positions_[i] % 10 + 20) : (hand_positions_[i] % 10);
        }
    }

    if (balls_intercepted_ >= 5) {
        game_over();
        return;
    }
}

void ShadowBasketball::add_omega() {
    int pos = 1;
    while (contains(omega_positions_, pos)) pos = rand_range(1, 3);
    omega_positions_.push_back(pos);
    hand_positions_.push_back(pos);
}

void ShadowBasketball::game_over() {
    shadow_position_ = -1;
    omega_positions_.clear();
    hand_positions_.clear();
    goals_scored_ = -1;
    ball_position_ = 0;
    level_ = 0;
    generic_game_over_animation({SHADOW_2, OMEGA_2, HANDS_22});
    stop();
}

void ShadowBasketball::victory() {
    shadow_position_ = -1;
    omega_positions_.clear();
    hand_positions_.clear();
    goals_scored_ = -1;
    ball_position_ = 0;
    level_ = 0;

    queue_sound("../common/game_win_short.ogg");

    const std::vector<std::string> f1 = {SHADOW_1, "ball-11"};
    std::vector<std::string> f2 = {SHADOW_1, "ball-21"};
    const std::vector<std::string> f3 = {SHADOW_1, "ball-32"};
    std::vector<std::string> f4 = {SHADOW_2, "ball-12"};
    const std::vector<std::string> f5 = {SHADOW_2, "ball-22"};
    std::vector<std::string> f6 = {SHADOW_2, "ball-32"};
    const std::vector<std::string> f7 = {SHADOW_3, "ball-13"};
    std::vector<std::string> f8 = {SHADOW_3, "ball-23"};
    const std::vector<std::string> f9 = {SHADOW_3, "ball-32"};
    if (burger_) {
        for (std::vector<std::string>* f : {&f2, &f4, &f6, &f8}) f->push_back("win");
    }

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 2; i++) {
        animation.insert(animation.end(), {f1, f1, f1, f1, f2, f2, f2, f2, f3, f3, f3, f3, f4, f4, f4, f4,
                                            f5, f5, f5, f5, f6, f6, f6, f6, f7, f7, f7, f7, f8, f8, f8, f8,
                                            f9, f9, f9, f9});
    }
    play_animation(animation);
    stop();
}
