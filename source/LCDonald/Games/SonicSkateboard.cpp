#include "SonicSkateboard.h"

#include <algorithm>

namespace {
constexpr const char* LIFE_1 = "life-1";
constexpr const char* LIFE_2 = "life-2";
constexpr const char* LIFE_3 = "life-3";
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* BIGRING_1 = "bigring-1";
constexpr const char* BIGRING_2 = "bigring-2";
constexpr const char* BIGRING_3 = "bigring-3";
constexpr const char* SONIC_CENTER = "sonic-center";
constexpr const char* SONIC_LEFT = "sonic-left";
constexpr const char* SONIC_RIGHT = "sonic-right";

bool contains(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}
void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

std::vector<std::string> SonicSkateboard::get_all_game_elements() const {
    return {LIFE_1,       LIFE_2,       LIFE_3,       LEVEL_1,      LEVEL_2,      LEVEL_3,
            BIGRING_1,
            "ring-11",    BIGRING_2,    "ring-13",
            "bomb-11",    "bomb-12",    "bomb-13",
            "ring-21",    BIGRING_3,    "ring-23",
            "bomb-21",    "bomb-22",    "bomb-23",
            "ring-31",    "ring-33",
            "bomb-31",    "bomb-32",    "bomb-33",
            SONIC_LEFT,   SONIC_CENTER, SONIC_RIGHT};
}

void SonicSkateboard::initialize_game_state() {
    sonic_position_ = 2;
    rings_collected_ = 0;
    bombs_hit_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    lives_ = 3;
    bomb_positions_.clear();
    ring_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 400 : 950;
    startup_music();
}

void SonicSkateboard::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && sonic_position_ > 1) {
            sonic_position_--;
        } else if (input == "Right" && sonic_position_ < 3) {
            sonic_position_++;
        }
    }
}

std::string SonicSkateboard::sonic_element() const {
    if (sonic_position_ == 1) return SONIC_LEFT;
    if (sonic_position_ == 2) return SONIC_CENTER;
    if (sonic_position_ == 3) return SONIC_RIGHT;
    return "";
}

std::vector<std::string> SonicSkateboard::get_lives_and_level_elements() const {
    std::vector<std::string> elements;
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    if (lives_ >= 1) elements.push_back(LIFE_1);
    if (lives_ >= 2) elements.push_back(LIFE_2);
    if (lives_ >= 3) elements.push_back(LIFE_3);
    return elements;
}

std::vector<std::string> SonicSkateboard::get_visible_elements() {
    std::vector<std::string> elements = get_lives_and_level_elements();
    elements.push_back(sonic_element());
    for (int b : bomb_positions_) elements.push_back("bomb-" + std::to_string(b));
    for (int r : ring_positions_) elements.push_back("ring-" + std::to_string(r));
    return elements;
}

void SonicSkateboard::update_core() {
    for (int ring_pos : std::vector<int>(ring_positions_)) {
        if (ring_pos <= 30) continue;
        const int horizontal_pos = ring_pos % 10;
        if (horizontal_pos == sonic_position_) {
            remove_value(ring_positions_, ring_pos);
            queue_sound("../common/hit.ogg");
            rings_collected_++;
            increment_score();
        }
    }
}

void SonicSkateboard::spawn_rings() {
    int ring_pos = (rand_range(0, 1) == 0) ? 11 : 13;
    if (!contains(bomb_positions_, ring_pos)) ring_positions_.push_back(ring_pos);

    if (rand_range(0, 99) < 33) {
        const int second_ring = (ring_pos == 11) ? 13 : 11;
        if (!contains(bomb_positions_, second_ring)) ring_positions_.push_back(second_ring);
    }
}

void SonicSkateboard::custom_update() {
    if (rings_collected_ >= 30 && !is_endless_mode_) {
        if (level_ == 3) {
            victory();
            return;
        } else {
            level_up();
            custom_update_speed_ -= 150;
        }
    }

    for (int& b : bomb_positions_) b += 10;
    for (int& r : ring_positions_) r += 10;

    for (int bomb_pos : std::vector<int>(bomb_positions_)) {
        if (bomb_pos <= 40) continue;
        remove_value(bomb_positions_, bomb_pos);
        const int horizontal_pos = bomb_pos % 10;
        if (horizontal_pos == sonic_position_) {
            queue_sound("../common/miss.ogg");
            bombs_hit_++;
            blink_element(sonic_element(), 1);
        }
    }

    for (int ring_pos : std::vector<int>(ring_positions_)) {
        if (ring_pos > 40) remove_value(ring_positions_, ring_pos);
    }

    if (bombs_hit_ == 5) {
        lives_--;
        bombs_hit_ = 0;
        if (lives_ == 0) game_over();
        return;
    }

    if (rand_range(0, 99) < 90) {
        const int first_bomb = rand_range(11, 13);
        bomb_positions_.push_back(first_bomb);

        if (rand_range(0, 99) < 33) {
            int second_bomb = first_bomb;
            do {
                second_bomb = rand_range(11, 13);
            } while (second_bomb == first_bomb);
            bomb_positions_.push_back(second_bomb);
        }

        if (rand_range(0, 99) < 33) spawn_rings();
    } else {
        spawn_rings();
    }
}

void SonicSkateboard::level_up() {
    input_blocked_ = true;
    ring_positions_.clear();
    bomb_positions_.clear();
    bombs_hit_ = 0;
    rings_collected_ = 0;
    sonic_position_ = 2;

    std::vector<std::string> f1 = get_lives_and_level_elements();
    f1.push_back(SONIC_CENTER);
    f1.push_back(BIGRING_1);
    std::vector<std::string> f2 = get_lives_and_level_elements();
    f2.push_back(SONIC_CENTER);
    f2.push_back(BIGRING_2);
    std::vector<std::string> f3 = get_lives_and_level_elements();
    f3.push_back(SONIC_CENTER);
    f3.push_back(BIGRING_3);

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 7; i++) animation.push_back(f1);
    for (int i = 0; i < 7; i++) animation.push_back(f2);
    for (int i = 0; i < 7; i++) animation.push_back(f3);
    play_animation(animation);
    queue_sound("../common/level_up.ogg");
    level_++;
}

void SonicSkateboard::game_over() {
    sonic_position_ = -1;
    level_ = 0;
    bomb_positions_.clear();
    ring_positions_.clear();
    generic_game_over_animation({SONIC_CENTER, "bomb-11", "bomb-12", "bomb-13", "bomb-21", "bomb-22", "bomb-23",
                                  "bomb-31", "bomb-32", "bomb-33"});
    stop();
}

void SonicSkateboard::victory() {
    sonic_position_ = -1;
    lives_ = 0;
    level_ = 0;
    bomb_positions_.clear();
    ring_positions_.clear();
    queue_sound("../common/game_win_short.ogg");

    const std::vector<std::string> f1 = {BIGRING_1};
    const std::vector<std::string> f2 = {BIGRING_2};
    const std::vector<std::string> f3 = {BIGRING_3};
    std::vector<std::string> f1s = {BIGRING_1, SONIC_CENTER};
    const std::vector<std::string> f2s = {BIGRING_2, SONIC_CENTER};
    std::vector<std::string> f3s = {BIGRING_3, SONIC_CENTER};
    if (burger_) {
        f1s.push_back("win");
        f3s.push_back("win");
    }

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 6; i++) {
        animation.insert(animation.end(), {f1, f1, f1, f2s, f2s, f2s, f3, f3, f3,
                                            f1s, f1s, f1s, f2, f2, f2, f3s, f3s, f3s});
    }
    play_animation(animation);
    stop();
}
