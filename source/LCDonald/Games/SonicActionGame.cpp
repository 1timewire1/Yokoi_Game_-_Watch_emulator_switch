#include "SonicActionGame.h"

#include <algorithm>

namespace {
constexpr const char* LEVEL_1 = "level-1";
constexpr const char* LEVEL_2 = "level-2";
constexpr const char* LEVEL_3 = "level-3";
constexpr const char* HIT_1 = "hit-1";
constexpr const char* HIT_2 = "hit-2";
constexpr const char* PLATFORM_12 = "platform-12";

bool contains(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}
void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

std::vector<std::string> SonicActionGame::get_all_game_elements() const {
    return {LEVEL_1,       LEVEL_2,       LEVEL_3,
            "sonic-3",     "ring-1",      "ring-2",      "ring-3",
            "platform-21", "platform-22", "platform-23", "platform-24",
            "sonic-2",     HIT_2,
            "platform-11", "platform-12", "platform-13", "platform-14",
            "sonic-1",     HIT_1};
}

void SonicActionGame::initialize_game_state() {
    sonic_position_ = 1;
    platforms_dodged_ = 0;
    platforms_hit_ = 0;
    level_ = is_endless_mode_ ? 5 : 0;
    platform_positions_.clear();
    ring_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 200 : 800;
    startup_music();
}

void SonicActionGame::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        if (input != "Jump") continue;
        // Can only double jump if there's a bottom platform in front of sonic
        // (meaning platform-12 is on).
        if (sonic_position_ == 2 && contains(platform_positions_, 12)) {
            sonic_position_ = 3;
        } else if (sonic_position_ == 1) {
            sonic_position_ = 2;
        }
    }
}

std::vector<std::string> SonicActionGame::get_visible_elements() {
    std::vector<std::string> elements;
    if (level_ >= 1) elements.push_back(LEVEL_1);
    if (level_ >= 2) elements.push_back(LEVEL_2);
    if (level_ >= 3) elements.push_back(LEVEL_3);
    elements.push_back(sonic_element());
    for (int p : platform_positions_) elements.push_back("platform-" + std::to_string(p));
    for (int r : ring_positions_) elements.push_back("ring-" + std::to_string(r));
    return elements;
}

void SonicActionGame::custom_update() {
    if (platforms_dodged_ >= 20 && !is_endless_mode_) {
        queue_sound("../common/level_up.ogg");
        platforms_hit_ = 0;
        platforms_dodged_ = 0;
        level_++;
        custom_update_speed_ -= 125;
    }

    if (level_ == 4) {
        victory();
        return;
    }

    bool has_dodged_platform = false;
    bool has_been_hit = false;
    for (int& p : platform_positions_) p -= 1;
    for (int& r : ring_positions_) r -= 1;

    if (contains(ring_positions_, 0) && sonic_position_ == 3) {
        remove_value(ring_positions_, 0);
        has_dodged_platform = true;
    }

    std::vector<int> platform_snapshot = platform_positions_;
    for (int platform : platform_snapshot) {
        if (platform == 10 || platform == 20) remove_value(platform_positions_, platform);

        if (platform == 11 && sonic_position_ == 1) {
            has_been_hit = true;
            blink_element(HIT_1, 1);
        }

        if (platform == 21 && sonic_position_ == 2) {
            has_been_hit = true;
            blink_element(HIT_2, 1);
        }

        if (platform == 11 && sonic_position_ > 1) {
            // If this is not a L-block platform, count this as a dodged platform.
            if (!contains(platform_snapshot, 22)) has_dodged_platform = true;
        }
    }

    // Apply gravity to sonic - unless there's a top platform he's standing on.
    if (sonic_position_ > 1 && !contains(platform_positions_, 21)) {
        if (contains(platform_positions_, 11)) {
            sonic_position_ = 2;
        } else {
            sonic_position_ = 1;
        }
    }

    if (has_been_hit) {
        queue_sound("../common/miss.ogg");
        remove_value(platform_positions_, 11);
        remove_value(platform_positions_, 12);
        remove_value(platform_positions_, 21);
        remove_value(platform_positions_, 22);
        remove_value(ring_positions_, 1);
        platforms_hit_++;
    } else if (has_dodged_platform) {
        queue_sound("../common/hit.ogg");
        platforms_dodged_++;
        increment_score();
    }

    if (platforms_hit_ == 5) {
        game_over();
        return;
    }

    if (block_spawn_counter_ > 0) {
        block_spawn_counter_--;
    } else {
        // Spawn platform: 3 random possibilities.
        switch (rand_range(1, 3)) {
            case 1:  // Single platform.
                platform_positions_.push_back(14);
                block_spawn_counter_ = 1;
                break;
            case 2:  // L-block platform set.
                platform_positions_.push_back(14);
                platform_positions_.push_back(15);
                platform_positions_.push_back(25);
                ring_positions_.push_back(4);
                block_spawn_counter_ = 2;
                break;
            case 3:  // No platform.
                block_spawn_counter_ = 1;
                break;
        }
    }
}

void SonicActionGame::game_over() {
    sonic_position_ = -1;
    level_ = 0;
    platform_positions_.clear();
    ring_positions_.clear();
    generic_game_over_animation({"sonic-1", HIT_1, PLATFORM_12});
    stop();
}

void SonicActionGame::victory() {
    sonic_position_ = -1;
    level_ = 0;
    platform_positions_.clear();
    ring_positions_.clear();
    generic_victory_animation({LEVEL_1, LEVEL_2, LEVEL_3});
    stop();
}
