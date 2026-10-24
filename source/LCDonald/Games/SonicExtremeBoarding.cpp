#include "SonicExtremeBoarding.h"

#include <algorithm>

namespace {
constexpr const char* HIT_1 = "hit-1";
constexpr const char* HIT_2 = "hit-2";
constexpr const char* HIT_3 = "hit-3";
constexpr const char* HIT_4 = "hit-4";
constexpr const char* HIT_5 = "hit-5";
constexpr const char* HIT_6 = "hit-6";
constexpr const char* RING_0 = "ring-0";
constexpr const char* SONIC_4 = "sonic-4";
constexpr const char* COOLRING = "cool";

bool contains(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}
void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

std::vector<std::string> SonicExtremeBoarding::get_all_game_elements() const {
    return {"sonic-1", "ring-31", COOLRING, "ring-37", "sonic-7",
            "ring-21", "ring-27",
            "ring-11", RING_0, "ring-17",
            "bomb-12", "bomb-14", "bomb-16",
            HIT_1, "bomb-22", "bomb-26", HIT_6,
            "sonic-2", "bomb-32", "bomb-24", "bomb-36", "sonic-6",
            HIT_2, HIT_5,
            "sonic-3", "bomb-34", "sonic-5",
            HIT_3, HIT_4,
            SONIC_4};
}

void SonicExtremeBoarding::initialize_game_state() {
    sonic_position_ = 4;
    rings_collected_ = 0;
    bombs_hit_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    bomb_positions_.clear();
    ring_position_ = -1;
    ring_spawn_ = 11;
    show_ring_spawn_indicator_ = false;
    custom_update_speed_ = is_endless_mode_ ? 400 : 950;
    startup_music("game_start.ogg", 2);
}

void SonicExtremeBoarding::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input == "Left" && sonic_position_ > 1) {
            sonic_position_--;
        } else if (input == "Right" && sonic_position_ < 7) {
            sonic_position_++;
        }
    }
}

std::vector<std::string> SonicExtremeBoarding::get_visible_elements() {
    std::vector<std::string> elements = {sonic_element()};
    if (show_ring_spawn_indicator_) elements.push_back(RING_0);
    elements.push_back("ring-" + std::to_string(ring_position_));
    for (int b : bomb_positions_) elements.push_back("bomb-" + std::to_string(b));
    return elements;
}

void SonicExtremeBoarding::update_core() {
    if (ring_position_ > 30) {
        const int horizontal_pos = ring_position_ % 10;
        if (horizontal_pos == sonic_position_) {
            ring_position_ = 0;
            queue_sound("ring_hit.ogg");
            rings_collected_++;
            increment_score();
            ring_spawn_ = (ring_spawn_ == 11) ? 17 : 11;
        }
    }

    // If sonic is at position 1 or 7, bring him down to 2/6 - on a single
    // frame here in the standard version; Burger does it in the main cycle
    // instead (custom_update() below).
    if (!burger_ && (sonic_position_ == 1 || sonic_position_ == 7)) {
        sonic_position_ = (sonic_position_ == 1) ? 2 : 6;
    }
}

void SonicExtremeBoarding::custom_update() {
    if (ring_position_ < 0) {
        ring_position_ = 0;
        show_ring_spawn_indicator_ = true;
        return;
    }

    if (rings_collected_ >= 20 && !is_endless_mode_) {
        if (level_ == 4) {
            victory();
            return;
        } else {
            level_up();
            custom_update_speed_ -= 125;
        }
    }

    bomb_positions_ = [this] {
        std::vector<int> moved;
        for (int b : bomb_positions_) moved.push_back(b + 10);
        return moved;
    }();

    for (int bomb_pos : std::vector<int>(bomb_positions_)) {
        if (bomb_pos <= 40) continue;
        remove_value(bomb_positions_, bomb_pos);
        const int horizontal_pos = bomb_pos % 10;

        if (horizontal_pos == sonic_position_) {
            queue_sound("bomb_hit.ogg");
            bombs_hit_++;

            blink_element(sonic_element(), 1);
            if (sonic_position_ == 2) {
                blink_element(HIT_1, 1);
                blink_element(HIT_2, 1);
            } else if (sonic_position_ == 4) {
                blink_element(HIT_3, 1);
                blink_element(HIT_4, 1);
            } else if (sonic_position_ == 6) {
                blink_element(HIT_5, 1);
                blink_element(HIT_6, 1);
            }
        }
    }

    if (ring_position_ > 0) ring_position_ += 10;

    if (ring_position_ > 30) {
        show_ring_spawn_indicator_ = true;
    } else {
        show_ring_spawn_indicator_ = false;
    }

    if (ring_position_ > 40 || ring_position_ == 0) {
        // Simulate a weird edge case from the OG circuit - if the ring
        // respawns on the same line due to the player not collecting it,
        // the spawn indicator remains for one extra cycle.
        if (ring_position_ % 10 != ring_spawn_ % 10) show_ring_spawn_indicator_ = false;
        ring_position_ = ring_spawn_;
    }

    if (bombs_hit_ == 5) {
        game_over();
        return;
    }

    // In the Burger Bard version, the hang position is updated in the main
    // cycle instead of on a single frame. If sonic is at position 1 or 7,
    // bring him down to 2/6.
    if (burger_ && (sonic_position_ == 1 || sonic_position_ == 7)) {
        sonic_position_ = (sonic_position_ == 1) ? 2 : 6;
    }

    const int bombs_on_previous_row =
        (int)std::count_if(bomb_positions_.begin(), bomb_positions_.end(), [](int r) { return r > 20 && r < 30; });

    if (bombs_on_previous_row != 3 && rand_range(0, 99) < 80) {
        const int first_bomb = spawn_bomb();
        if (first_bomb != -1) bomb_positions_.push_back(first_bomb);

        // If we're on the same row as a ring, there's only one bomb max.
        if (ring_position_ == 11 || ring_position_ == 17) return;

        if (bombs_on_previous_row < 2 && rand_range(0, 99) < 33) {
            const int second_bomb = spawn_bomb();
            if (second_bomb == -1) return;
            bomb_positions_.push_back(second_bomb);
        }

        if (bombs_on_previous_row == 0 && rand_range(0, 99) < 33) {
            const int third_bomb = spawn_bomb();
            if (third_bomb == -1) return;
            bomb_positions_.push_back(third_bomb);
        }
    }
}

int SonicExtremeBoarding::spawn_bomb() {
    std::vector<int> possible_spawns = {12, 14, 16};

    if (contains(bomb_positions_, 22) || contains(bomb_positions_, 12) || ring_position_ == 11) {
        remove_value(possible_spawns, 12);
    }
    if (contains(bomb_positions_, 24) || contains(bomb_positions_, 14)) {
        remove_value(possible_spawns, 14);
    }
    // Matches the original's own apparent bug: this branch removes 14
    // again instead of 16 (see ShadowHockey-adjacent games for context -
    // ported faithfully, not fixed).
    if (contains(bomb_positions_, 26) || contains(bomb_positions_, 16) || ring_position_ == 17) {
        remove_value(possible_spawns, 14);
    }

    if (possible_spawns.empty()) return -1;

    return possible_spawns[rand_range(0, (int)possible_spawns.size() - 1)];
}

void SonicExtremeBoarding::level_up() {
    input_blocked_ = true;
    bombs_hit_ = 0;
    rings_collected_ = 0;

    std::vector<std::string> f1 = get_visible_game_elements();
    f1.push_back(COOLRING);
    std::vector<std::string> f2 = get_visible_game_elements();

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 3; i++) {
        animation.insert(animation.end(), {f1, f1, f1, f2, f2, f2});
    }
    queue_sound("level_up.ogg");
    play_animation(animation);
    level_++;
}

void SonicExtremeBoarding::game_over() {
    sonic_position_ = -1;
    level_ = 0;
    bomb_positions_.clear();
    ring_position_ = -1;
    generic_game_over_animation({SONIC_4, "bomb-12", "bomb-14", "bomb-16", "bomb-22", "bomb-24", "bomb-26",
                                  "bomb-32", "bomb-34", "bomb-36"},
                                 "game_over.ogg");
    stop();
}

void SonicExtremeBoarding::victory() {
    sonic_position_ = -1;
    level_ = 0;
    bomb_positions_.clear();
    ring_position_ = -1;

    queue_sound("game_win.ogg");

    const std::vector<std::string> f1 = {};
    std::vector<std::string> f2 = {SONIC_4, COOLRING};
    if (burger_) f2.push_back("win");

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 5; i++) {
        animation.insert(animation.end(), {f1, f1, f1, f1, f1, f1, f2, f2, f2, f2, f2, f2});
    }
    play_animation(animation);
    stop();
}
