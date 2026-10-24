#include "TailsSkyAdventure.h"

namespace {
constexpr const char* LIFE_1 = "life-1";
constexpr const char* LIFE_2 = "life-2";
constexpr const char* LIFE_3 = "life-3";
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* TAILS_LEFT = "tails-left";
constexpr const char* TAILS_RIGHT = "tails-right";
constexpr const char* TAILS_CENTER = "tails-center";
}  // namespace

std::vector<std::string> TailsSkyAdventure::get_all_game_elements() const {
    return {LIFE_1,          LIFE_2,          LIFE_3,          LEVEL_1,         LEVEL_2,         LEVEL_3,
            "enemy-11",      "enemy-12",      "enemy-13",
            "projectile-11", "projectile-12", "projectile-13",
            "enemy-21",      "enemy-22",      "enemy-23",
            "projectile-21", "projectile-22", "projectile-23",
            "enemy-31",      "enemy-32",      "enemy-33",
            "projectile-31", "projectile-32", "projectile-33",
            TAILS_LEFT,      TAILS_CENTER,    TAILS_RIGHT};
}

void TailsSkyAdventure::initialize_game_state() {
    tails_position_ = 2;
    life_count_ = 3;
    level_ = is_endless_mode_ ? 5 : 0;
    enemies_hit_ = 0;
    enemies_missed_ = 0;
    projectile_pos_ = -1;
    enemy_pos_ = -1;
    custom_update_speed_ = is_endless_mode_ ? 400 : 900;
    startup_music("game_start.ogg", 0);
}

void TailsSkyAdventure::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && tails_position_ > 1) {
            tails_position_--;
        } else if (input == "Right" && tails_position_ < 3) {
            tails_position_++;
        } else if (input == "Fire" && projectile_pos_ == -1) {
            queue_sound("fire.ogg");
            projectile_pos_ = tails_position_ + 30;
        }
    }
}

std::string TailsSkyAdventure::tails_element() const {
    if (tails_position_ == 1) return TAILS_LEFT;
    if (tails_position_ == 2) return TAILS_CENTER;
    if (tails_position_ == 3) return TAILS_RIGHT;
    return "";
}

std::vector<std::string> TailsSkyAdventure::get_visible_elements() {
    std::vector<std::string> elements;
    if (life_count_ >= 1) elements.push_back(LIFE_1);
    if (life_count_ >= 2) elements.push_back(LIFE_2);
    if (life_count_ == 3) elements.push_back(LIFE_3);
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    elements.push_back(tails_element());
    if (projectile_pos_ != -1) elements.push_back("projectile-" + std::to_string(projectile_pos_));
    if (enemy_pos_ != -1) elements.push_back("enemy-" + std::to_string(enemy_pos_));
    return elements;
}

void TailsSkyAdventure::update_core() {
    // +10 tolerance in case the projectile passed while the enemy moved forward.
    if (projectile_pos_ != -1 && (projectile_pos_ == enemy_pos_ || projectile_pos_ + 10 == enemy_pos_)) {
        blink_element("enemy-" + std::to_string(enemy_pos_), 3);
        queue_sound("hit.ogg");
        enemy_pos_ = projectile_pos_ = -1;
        enemies_hit_++;
        increment_score();
    }

    if (projectile_pos_ > 10) {
        projectile_pos_ -= 10;
    } else {
        projectile_pos_ = -1;
    }
}

void TailsSkyAdventure::custom_update() {
    if (enemies_hit_ == 15 && !is_endless_mode_) {
        queue_sound("level_up.ogg");
        enemies_hit_ = 0;
        level_++;
        blink_element("level-" + std::to_string(level_), 2);
        custom_update_speed_ -= 175;
    }

    if (level_ == 4) {
        victory();
        return;
    }

    if (enemy_pos_ != -1) {
        enemy_pos_ += 10;

        if (enemy_pos_ > 40 && projectile_pos_ != enemy_pos_ - 10) {
            const int digit = enemy_pos_ % 10;
            enemy_pos_ = -1;
            if (digit == tails_position_) {
                life_count_--;
                queue_sound("life_loss.ogg");

                input_blocked_ = true;
                // Life loss, both character and enemy blink.
                const std::string tails_spr = tails_element();
                blink_element(tails_spr, 2);
                const std::string enemy_spr = "enemy-3" + std::to_string(digit);
                blink_element(enemy_spr, 3);
                // Wait for blinking to end - see the class doc comment for why
                // this explicit tick() loop replaces the original's raw
                // thread spin-wait.
                while (is_blinking(enemy_spr)) tick();
                input_blocked_ = false;
            } else {
                enemies_missed_++;
                if (enemies_missed_ == 5) {
                    life_count_--;
                    queue_sound("life_loss.ogg");
                    input_blocked_ = true;
                    const std::string spr = tails_element();
                    blink_element(spr, 2);
                    while (is_blinking(spr)) tick();
                    input_blocked_ = false;
                } else {
                    queue_sound("miss.ogg");
                }
            }

            if (life_count_ == 0) game_over();
        } else {
            enemy_pos_ += rand_range(-1, 1);
            if (enemy_pos_ % 10 == 0) enemy_pos_++;
            if (enemy_pos_ % 10 == 4) enemy_pos_--;
        }
    } else if (!spawn_enemy_next_update_) {
        spawn_enemy_next_update_ = true;
    } else {
        spawn_enemy_next_update_ = false;
        enemy_pos_ = rand_range(11, 13);
    }
}

void TailsSkyAdventure::game_over() {
    queue_sound("game_over.ogg");

    const std::vector<std::string> frame1 = {TAILS_CENTER, "enemy-31", "enemy-32", "enemy-33"};
    const std::vector<std::string> frame2 = {TAILS_CENTER};

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 5; i++) {
        animation.insert(animation.end(), {frame1, frame1, frame1, frame1, frame2, frame2, frame2, frame2});
    }

    tails_position_ = -1;
    // Hand-built animation (matching the original's own frame1/frame2)
    // rather than generic_game_over_animation(), since this one uses two
    // custom frames instead of a single blink target list.
    play_animation(animation);
    stop();
}

void TailsSkyAdventure::victory() {
    queue_sound("game_win.ogg");

    std::vector<std::string> frame1 = {TAILS_CENTER, "projectile-31", "projectile-32", "projectile-33"};
    const std::vector<std::string> frame2 = {TAILS_CENTER, "projectile-21", "projectile-22", "projectile-23"};
    std::vector<std::string> frame3 = {TAILS_CENTER, "projectile-11", "projectile-12", "projectile-13"};
    if (burger_) {
        frame1.push_back("win");
        frame3.push_back("win");
    }

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 5; i++) {
        animation.insert(animation.end(), {frame1, frame1, frame1, frame1, frame2, frame2, frame2, frame2,
                                            frame3, frame3, frame3, frame3});
    }

    tails_position_ = -1;
    play_animation(animation);
    stop();
}
