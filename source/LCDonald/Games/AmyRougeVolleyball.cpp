#include "AmyRougeVolleyball.h"

namespace {
constexpr const char* SCORE_CENTER = "score-center";
constexpr const char* SCORE_A1 = "score-a1";
constexpr const char* SCORE_A2 = "score-a2";
constexpr const char* SCORE_A3 = "score-a3";
constexpr const char* SCORE_R1 = "score-r1";
constexpr const char* SCORE_R2 = "score-r2";
constexpr const char* SCORE_R3 = "score-r3";
constexpr const char* AMY_1 = "amy-1";
constexpr const char* AMY_2 = "amy-2";
constexpr const char* AMY_3 = "amy-3";
constexpr const char* TENNIS_2 = "tennis-2";
constexpr const char* MISS = "miss";
}  // namespace

std::vector<std::string> AmyRougeVolleyball::get_all_game_elements() const {
    std::vector<std::string> elements = {"score-a3", "score-a2", "score-a1", SCORE_CENTER, SCORE_R1, "score-r2", "score-r3",
            "rouge-1",  "rouge-2",
            "ball-11",  "ball-12",
            "ball-21",  "ball-22",  "ball-23",
            MISS,
            "ball-31",  "ball-32",  "ball-33",
            "ball-41",  "ball-42",  "ball-43",
            "tennis-1", AMY_1, "tennis-2", AMY_2, "tennis-3", AMY_3};
    if (burger_) elements.insert(elements.end(), {"hands-1", "hands-2", "hands-3"});  // Burger's new hands groups.
    return elements;
}


void AmyRougeVolleyball::initialize_game_state() {
    amy_position_ = 2;
    ball_position_ = -1;
    rouge_position_ = -1;
    next_ball_spawn_ = rand_range(1, 2);
    level_ = is_endless_mode_ ? 6 : 0;
    current_score_ = is_endless_mode_ ? -3 : 0;
    custom_update_speed_ = is_endless_mode_ ? 200 : 500;
    startup_music("game_start.ogg");
}

void AmyRougeVolleyball::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && amy_position_ > 1) {
            amy_position_--;
        } else if (input == "Right" && amy_position_ < 3) {
            amy_position_++;
        } else if (input == "Volley") {
            bat_engaged_ = true;
        }
    }
}

std::vector<std::string> AmyRougeVolleyball::get_visible_elements() {
    std::vector<std::string> elements;
    if (current_score_ == 0) elements = {SCORE_CENTER};
    else if (current_score_ == 1) elements = {SCORE_CENTER, SCORE_A1};
    else if (current_score_ == 2) elements = {SCORE_CENTER, SCORE_A1, SCORE_A2};
    else if (current_score_ == 3) elements = {SCORE_CENTER, SCORE_A1, SCORE_A2, SCORE_A3};
    else if (current_score_ == -1) elements = {SCORE_CENTER, SCORE_R1};
    else if (current_score_ == -2) elements = {SCORE_CENTER, SCORE_R1, SCORE_R2};
    else if (current_score_ == -3) elements = {SCORE_CENTER, SCORE_R1, SCORE_R2, SCORE_R3};
    else elements = {SCORE_CENTER, SCORE_A1, SCORE_A2, SCORE_A3, SCORE_R1, SCORE_R2, SCORE_R3};

    elements.push_back(amy_element());
    elements.push_back("rouge-" + std::to_string(rouge_position_));
    elements.push_back("ball-" + std::to_string(ball_position_));

    if (bat_engaged_) {
        elements.push_back("tennis-" + std::to_string(amy_position_));
    } else if (burger_) {
        elements.push_back("hands-" + std::to_string(amy_position_));
    }

    return elements;
}

void AmyRougeVolleyball::update_core() {
    if (ball_position_ > 40 && ball_position_ % 10 == amy_position_ && bat_engaged_ && !ball_direction_) {
        ball_direction_ = true;
        queue_sound("hit_confirm.ogg");
    } else if (bat_engaged_) {
        queue_sound("hit.ogg");
    }
    // Burger disengages the bat later, in custom_update() (see below).
    if (!burger_) bat_engaged_ = false;
}

void AmyRougeVolleyball::custom_update() {
    if (ball_position_ == -1) {
        rouge_position_ = next_ball_spawn_;
        next_ball_spawn_ = (rouge_position_ == 1) ? 2 : 1;
        ball_position_ = rouge_position_ + 10;
        ball_direction_ = false;

        if (level_ == 0) ai_resistance_ = rand_range(1, 1);
        else if (level_ == 1) ai_resistance_ = rand_range(1, 5);
        else if (level_ == 2) ai_resistance_ = rand_range(2, 8);
        else if (level_ == 3) ai_resistance_ = rand_range(3, 9);
        else if (level_ == 4) ai_resistance_ = rand_range(6, 18);
    } else {
        if (ball_direction_) {
            ball_position_ -= 10;
        } else {
            ball_position_ += 10;
        }

        if (ball_position_ > 20 && ball_position_ < 40) {
            ball_position_ += rand_range(-1, 1);
            if (ball_position_ % 10 == 0) ball_position_++;
            if (ball_position_ % 10 == 4) ball_position_--;
        }

        if (ball_position_ == 13) ball_position_ = 12;
    }

    if (ball_position_ > 50) {
        current_score_--;
        ball_position_ = -1;
        blink_element(MISS, 1);
        queue_sound("miss.ogg");
    }

    if (ball_position_ < 20 && ball_position_ != -1 && ai_resistance_ > 0 && ball_direction_) {
        ai_resistance_--;
        increment_score();
        queue_sound("hit_return.ogg");
        ball_direction_ = false;
        rouge_position_ = ball_position_ % 10;
    }

    if (ball_position_ < 10 && ball_position_ != -1 && ai_resistance_ == 0) {
        current_score_++;
        ball_position_ = -1;
        queue_sound("score.ogg");
    }

    // Disengage bat later in the Burger version.
    if (burger_) bat_engaged_ = false;

    if (current_score_ == 4) {
        level_up_animation();
        custom_update_speed_ -= 60;
    }

    if (level_ == 5) {
        victory();
        return;
    }

    if (current_score_ == -4) {
        game_over();
        return;
    }
}

void AmyRougeVolleyball::level_up_animation() {
    input_blocked_ = true;
    current_score_ = 0;

    std::vector<std::string> f0 = get_visible_game_elements();
    f0.push_back(SCORE_CENTER);
    std::vector<std::string> f1 = get_visible_game_elements();
    f1.push_back(SCORE_A1);
    std::vector<std::string> f2 = get_visible_game_elements();
    f2.push_back(SCORE_A2);
    std::vector<std::string> f3 = get_visible_game_elements();
    f3.push_back(SCORE_A3);

    queue_sound("level_up.ogg");
    play_animation({f0, f1, f2, f3});
    level_++;
}

void AmyRougeVolleyball::game_over() {
    amy_position_ = -1;
    level_ = 0;
    rouge_position_ = -1;
    ball_position_ = -1;
    current_score_ = 0;

    queue_sound("game_over.ogg");

    const std::vector<std::string> f1 = {SCORE_CENTER};
    const std::vector<std::string> f2 = {SCORE_R1};
    const std::vector<std::string> f3 = {"score-r2"};
    const std::vector<std::string> f4 = {"score-r3"};
    const std::vector<std::string> f1s = {SCORE_CENTER, MISS};
    const std::vector<std::string> f2s = {SCORE_R1, MISS};
    const std::vector<std::string> f3s = {"score-r2", MISS};
    const std::vector<std::string> f4s = {"score-r3", MISS};

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 6; i++) {
        animation.insert(animation.end(), {f1, f2, f3, f4, f1s, f2s, f3s, f4s});
    }
    play_animation(animation);
    stop();
}

void AmyRougeVolleyball::victory() {
    amy_position_ = -1;
    level_ = 0;
    rouge_position_ = -1;
    ball_position_ = -1;
    current_score_ = 0;

    queue_sound("game_win.ogg");

    const std::vector<std::string> f1 = {SCORE_CENTER};
    const std::vector<std::string> f2 = {SCORE_A1};
    const std::vector<std::string> f3 = {SCORE_A2};
    const std::vector<std::string> f4 = {SCORE_A3};
    std::vector<std::string> f1s = {SCORE_CENTER, AMY_1, AMY_2, AMY_3, TENNIS_2};
    std::vector<std::string> f2s = {SCORE_A1, AMY_1, AMY_2, AMY_3, TENNIS_2};
    std::vector<std::string> f3s = {SCORE_A2, AMY_1, AMY_2, AMY_3, TENNIS_2};
    std::vector<std::string> f4s = {SCORE_A3, AMY_1, AMY_2, AMY_3, TENNIS_2};
    if (burger_) {
        for (std::vector<std::string>* f : {&f1s, &f2s, &f3s, &f4s}) {
            f->insert(f->end() - 1, {"hands-1", "hands-2", "hands-3"});  // ahead of TENNIS_2, as in the C#.
        }
        f1s.push_back("win");
        f3s.push_back("win");
    }

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 11; i++) {
        animation.insert(animation.end(), {f1, f2, f3, f4, f1s, f2s, f3s, f4s});
    }
    play_animation(animation);
    stop();
}
