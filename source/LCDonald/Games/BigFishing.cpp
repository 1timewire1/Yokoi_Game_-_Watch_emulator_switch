#include "BigFishing.h"

namespace {
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* LEVEL_UP = "level-up";
constexpr const char* FISH_CATCH = "fish-catch";
constexpr const char* ROD_1 = "rod-1";
constexpr const char* ROD_2 = "rod-2";
constexpr const char* ROD_3 = "rod-3";
constexpr const char* BIG_HANDS = "big-hands";
constexpr const char* BIG_CATCH = "big-catch";
constexpr const char* HIT = "hit";
constexpr const char* MISS = "miss";
}  // namespace

std::vector<std::string> BigFishing::get_all_game_elements() const {
    return {LEVEL_1,    LEVEL_2,    LEVEL_3,    HIT,        BIG_CATCH,
            LEVEL_UP,   BIG_HANDS,
            FISH_CATCH,
            ROD_1,
            "fish-11",  "fish-21",  "fish-31",  "lure-1",   "fish-41",
            ROD_2,
            "fish-12",  "fish-22",  "fish-32",  "lure-2",   "fish-42",  MISS,
            ROD_3,
            "fish-13",  "fish-23",  "fish-33",  "lure-3",   "fish-43"};
}

void BigFishing::initialize_game_state() {
    rod_position_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    fish_caught_ = 0;
    fish_missed_ = 0;
    has_lure_ = true;
    fish_pos_ = -1;
    custom_update_speed_ = is_endless_mode_ ? 450 : 850;
    startup_music("game_start.ogg");
}

void BigFishing::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Up" && rod_position_ > 0) {
            is_fish_caught_ = false;
            rod_position_--;
            queue_sound("rod_up.ogg");
        } else if (input == "Down" && rod_position_ < 3) {
            is_fish_caught_ = false;
            rod_position_++;
            queue_sound("rod_down.ogg");
        } else if (input == "Reel" && is_fish_caught_) {
            trigger_reel_animation_ = true;
        }
    }
}

std::vector<std::string> BigFishing::get_levels() const {
    std::vector<std::string> elements;
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    return elements;
}

std::vector<std::string> BigFishing::get_visible_elements() {
    std::vector<std::string> elements = get_levels();
    elements.push_back(BIG_HANDS);
    if (is_fish_caught_) elements.push_back(HIT);
    if (rod_position_ >= 1) elements.push_back(ROD_1);
    if (rod_position_ >= 2) elements.push_back(ROD_2);
    if (rod_position_ >= 3) elements.push_back(ROD_3);
    if (has_lure_) elements.push_back("lure-" + std::to_string(rod_position_));
    if (fish_pos_ != -1) elements.push_back("fish-" + std::to_string(fish_pos_));
    return elements;
}

void BigFishing::update_core() {
    if (fish_pos_ > 30 && fish_pos_ < 40) {
        const int digit = fish_pos_ % 10;
        if (digit == rod_position_ && !is_fish_caught_ && !is_input_blocked()) {
            is_fish_caught_ = true;
            has_lure_ = false;
            queue_sound("hit.ogg");
        }
    }

    if (trigger_reel_animation_) {
        is_fish_caught_ = false;
        trigger_reel_animation_ = false;
        reel_up_animation();
    }
}

void BigFishing::custom_update() {
    if (fish_caught_ == 10 && !is_endless_mode_) {
        queue_sound("level_up.ogg");
        fish_caught_ = 0;
        fish_missed_ = 0;
        level_++;
        blink_element(LEVEL_UP, 2);
        custom_update_speed_ -= 75;
    }

    if (level_ == 4) {
        victory();
        return;
    }

    if (fish_pos_ != -1) {
        fish_pos_ += 10;
        is_fish_caught_ = false;

        if (fish_pos_ < 30) {
            fish_pos_ += rand_range(-1, 1);
            if (fish_pos_ % 10 == 0) fish_pos_++;
            if (fish_pos_ % 10 == 4) fish_pos_--;
        }

        if (fish_pos_ > 40 && !has_lure_) {
            rod_position_ = 0;
            has_lure_ = true;
        }

        if (fish_pos_ > 50) {
            fish_pos_ = -1;
            fish_missed_++;
            blink_element(MISS, 2);
            queue_sound("miss.ogg");
            if (fish_missed_ == 5) game_over();
        }
    } else {
        fish_pos_ = rand_range(11, 13);
    }
}

void BigFishing::reel_up_animation() {
    queue_sound("fish_caught.ogg");

    std::vector<std::vector<std::string>> animation;

    // Dynamic amount of frames depending on the current fish position -
    // the original calls PlayAnimation() once per iteration here and
    // blocks until each finishes before recomputing the next frame; ported
    // as building the whole sequence up front instead (see class comment).
    while (fish_pos_ > 30) {
        const std::vector<std::string> fish_frame = get_visible_elements();
        animation.insert(animation.end(), {fish_frame, fish_frame, fish_frame});
        fish_pos_--;
        rod_position_--;
    }

    const std::vector<std::string> reel_frame1 = [this] {
        std::vector<std::string> f = get_levels();
        f.push_back(FISH_CATCH);
        f.push_back(BIG_HANDS);
        return f;
    }();
    const std::vector<std::string> reel_frame2 = [this] {
        std::vector<std::string> f = get_levels();
        f.push_back(BIG_CATCH);
        return f;
    }();
    const std::vector<std::string> reel_frame3 = get_levels();

    animation.insert(animation.end(), {reel_frame1, reel_frame1, reel_frame1, reel_frame2, reel_frame3,
                                        reel_frame2, reel_frame3, reel_frame2, reel_frame3});
    play_animation(animation);

    fish_caught_++;
    increment_score();
    has_lure_ = true;
    rod_position_ = 0;
    fish_pos_ = -1;
}

void BigFishing::game_over() {
    rod_position_ = -1;
    fish_pos_ = -1;
    generic_game_over_animation({MISS, "fish-11", "fish-12", "fish-13", "fish-21", "fish-22", "fish-23",
                                  "fish-31", "fish-32", "fish-33", "fish-41", "fish-42", "fish-43"},
                                 "game_over.ogg");
    stop();
}

void BigFishing::victory() {
    rod_position_ = -1;
    fish_pos_ = -1;
    queue_sound("game_win.ogg");

    const std::vector<std::string> frame1 = {BIG_HANDS};
    const std::vector<std::string> frame2 = {BIG_CATCH};
    std::vector<std::string> frame3 = {LEVEL_UP};
    if (burger_) frame3.push_back("win");
    const std::vector<std::string> frame4 = {};

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 2; i++) {
        animation.insert(animation.end(), {frame1, frame1, frame1, frame1, frame1, frame1,
                                            frame4, frame4, frame4, frame4, frame4, frame4,
                                            frame2, frame2, frame2, frame2, frame2, frame2,
                                            frame4, frame4, frame4, frame4, frame4, frame4,
                                            frame3, frame3, frame3, frame3, frame3, frame3,
                                            frame4, frame4, frame4, frame4, frame4, frame4});
    }
    play_animation(animation);
    stop();
}
