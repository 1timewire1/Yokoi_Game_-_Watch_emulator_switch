#include "KnucklesTreasureHunt.h"

#include <algorithm>

namespace {
constexpr const char* EMERALD_1 = "emerald-1";
constexpr const char* EMERALD_2 = "emerald-2";
constexpr const char* EMERALD_3 = "emerald-3";
constexpr const char* EMERALD_4 = "emerald-4";
constexpr const char* EMERALD_5 = "emerald-5";
constexpr const char* KNUCKLES_21 = "knux-21";
constexpr const char* FALL_UP = "fall-up";
constexpr const char* FALL_DOWN = "fall-down";

bool contains(const std::vector<int>& v, int x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}
void remove_value(std::vector<int>& v, int x) {
    v.erase(std::remove(v.begin(), v.end(), x), v.end());
}
}  // namespace

// Spawnlists gleaned by looking at the real game. Platforms spawn
// following this looping pattern.
const std::vector<bool> KnucklesTreasureHunt::kSpawnmapLeft =
    {true, false, true, false, true, false, true, true, false, true, false, true, false, false};
const std::vector<bool> KnucklesTreasureHunt::kSpawnmapRight =
    {true, false, false, false, true, false, true, false, true, false, false, true};

std::vector<std::string> KnucklesTreasureHunt::get_all_game_elements() const {
    return {FALL_UP,
            "platform-2", "platform-3",
            EMERALD_1, "knux-11", "knux-12", "knux-13", "knux-14", EMERALD_2,
            "platform-12", "platform-13",
            EMERALD_3, "knux-21", "knux-22", "knux-23", "knux-24", EMERALD_4,
            "platform-22", "platform-23",
            "knux-31", "knux-32", "knux-33", "knux-34", EMERALD_5,
            "platform-32", "platform-33",
            FALL_DOWN};
}

void KnucklesTreasureHunt::handle_inputs(const std::vector<std::string>& pressed_inputs) {
    for (const std::string& input : pressed_inputs) {
        has_knuckles_moved_ = true;
        if (input == "Left" && knuckles_position_ % 10 > 1) {
            knuckles_position_--;
            queue_sound("move.ogg");
        } else if (input == "Right" && knuckles_position_ % 10 < 4) {
            knuckles_position_++;
            queue_sound("move.ogg");
        }
    }
}

std::vector<std::string> KnucklesTreasureHunt::get_platform_elements() const {
    std::vector<std::string> elements;
    for (int p : platform_positions_) elements.push_back("platform-" + std::to_string(p));
    return elements;
}

std::string KnucklesTreasureHunt::knuckles_element() const {
    if (knuckles_position_ != -1) return "knux-" + std::to_string(knuckles_position_);
    return "";
}

std::vector<std::string> KnucklesTreasureHunt::get_knuckles_and_platform_elements() const {
    std::vector<std::string> elements = get_platform_elements();
    elements.push_back(knuckles_element());
    return elements;
}

std::vector<std::string> KnucklesTreasureHunt::get_visible_elements() {
    std::vector<std::string> elements = get_knuckles_and_platform_elements();
    for (int e : emerald_positions_) elements.push_back("emerald-" + std::to_string(e));
    return elements;
}

void KnucklesTreasureHunt::initialize_game_state() {
    life_count_ = 3;
    level_ = is_endless_mode_ ? 7 : 1;
    emeralds_collected_ = 0;
    is_first_emerald_ = true;
    knuckles_position_ = 31;
    platforms_to_move_next_update_ = 2;
    current_left_spawn_ = 0;
    current_right_spawn_ = 0;
    platform_positions_.clear();
    emerald_positions_.clear();
    custom_update_speed_ = is_endless_mode_ ? 400 : 1000;
    startup_music("game_start.ogg", 2);
}

void KnucklesTreasureHunt::emerald_spawn_animation() {
    input_blocked_ = true;
    has_knuckles_moved_ = false;

    if (level_ >= 5) {
        emerald_positions_ = {1, 2, 3, 4, 5};
    } else {
        for (int i = 0; i < level_; i++) {
            emerald_positions_.push_back(get_random_emerald_position());
        }
    }

    std::vector<std::string> spawn1 = get_knuckles_and_platform_elements();
    spawn1.push_back(EMERALD_3);
    std::vector<std::string> spawn2 = get_knuckles_and_platform_elements();
    spawn2.push_back(EMERALD_1);
    std::vector<std::string> spawn3 = get_knuckles_and_platform_elements();
    spawn3.push_back(EMERALD_2);
    std::vector<std::string> spawn4 = get_knuckles_and_platform_elements();
    spawn4.push_back(EMERALD_4);
    std::vector<std::string> spawn5 = get_knuckles_and_platform_elements();
    spawn5.push_back(EMERALD_5);
    std::vector<std::string> spawn6 = get_visible_elements();
    std::vector<std::string> spawn7 = get_knuckles_and_platform_elements();

    std::vector<std::vector<std::string>> animation;
    for (auto* f : {&spawn1, &spawn2, &spawn3, &spawn4, &spawn5, &spawn6, &spawn7}) {
        for (int i = 0; i < 6; i++) animation.push_back(*f);
    }
    queue_sound("emerald_spawn.ogg");
    play_animation(animation);

    emerald_time_limit_ = 10 + level_;
    input_blocked_ = false;
}

int KnucklesTreasureHunt::get_random_emerald_position() {
    if (emerald_positions_.size() == 5) return -1;

    if (is_first_emerald_) {
        is_first_emerald_ = false;
        return 4;
    }

    int position = 0;
    while (position == 0 || contains(emerald_positions_, position)) {
        position = rand_range(1, 5);
    }
    return position;
}

bool KnucklesTreasureHunt::is_in_front_of_emerald(int emerald) const {
    return (knuckles_position_ == 11 && emerald == 1) || (knuckles_position_ == 14 && emerald == 2) ||
           (knuckles_position_ == 21 && emerald == 3) || (knuckles_position_ == 24 && emerald == 4) ||
           (knuckles_position_ == 34 && emerald == 5);
}

void KnucklesTreasureHunt::fall_animation() {
    if (is_falling_) return;

    is_falling_ = true;
    input_blocked_ = true;
    emerald_positions_.clear();

    std::vector<std::vector<std::string>> animation;
    std::vector<std::string> fall_frame1;

    if (knuckles_position_ == 2) {
        // Special fallup case, just blink FALL_UP twice.
        fall_frame1 = get_platform_elements();
        fall_frame1.push_back(FALL_UP);
    } else {
        // Center knuckles in case he's in the border zones.
        if (knuckles_position_ % 10 == 1) knuckles_position_++;
        if (knuckles_position_ % 10 == 4) knuckles_position_--;

        // Fall all the way to the bottom, then blink FALL_DOWN twice - the
        // original calls PlayAnimation() once per row here and blocks
        // until each finishes; ported by building the whole sequence up
        // front instead (see class comment).
        while (knuckles_position_ <= 33) {
            queue_sound("fall.ogg");
            const std::vector<std::string> fall_frame = get_knuckles_and_platform_elements();
            for (int i = 0; i < 6; i++) animation.push_back(fall_frame);
            knuckles_position_ += 10;
        }
        fall_frame1 = get_platform_elements();
        fall_frame1.push_back(FALL_DOWN);
    }

    const std::vector<std::string> fall_frame2 = get_platform_elements();

    queue_sound("fall_end.ogg");
    for (int i = 0; i < 2; i++) {
        animation.insert(animation.end(), {fall_frame1, fall_frame1, fall_frame1, fall_frame1,
                                            fall_frame2, fall_frame2, fall_frame2, fall_frame2});
    }

    life_count_--;
    const bool died = (life_count_ == 0);
    if (died) {
        // Original calls GameOver() here, which is only ever reached this
        // way (no other call site) - its own slow FALL_UP/FALL_DOWN blink
        // is appended onto this same animation instead of a separate
        // play_animation() call, which would cut this one off before it's
        // shown.
        queue_sound("game_over.ogg");
        const std::vector<std::string> go1 = {FALL_UP};
        const std::vector<std::string> go2 = {FALL_DOWN};
        for (int i = 0; i < 4; i++) {
            animation.insert(animation.end(), {go1, go1, go1, go1, go1, go1, go1, go1,
                                                go2, go2, go2, go2, go2, go2, go2, go2});
        }
    }

    play_animation(animation);

    knuckles_position_ = died ? -1 : 31;
    emerald_positions_.clear();
    is_falling_ = false;

    if (died) {
        level_ = 0;
        stop();
    }
}

void KnucklesTreasureHunt::update_core() {
    const int digit = knuckles_position_ % 10;
    if ((digit == 2 || digit == 3) && !contains(platform_positions_, knuckles_position_)) fall_animation();

    if (knuckles_position_ == 2 || knuckles_position_ == 43) fall_animation();

    for (int emerald : std::vector<int>(emerald_positions_)) {
        if (is_in_front_of_emerald(emerald) && has_knuckles_moved_) {
            emeralds_collected_++;
            increment_score();
            queue_sound("emerald_get.ogg");
            remove_value(emerald_positions_, emerald);
        }

        if (emerald_positions_.empty()) {
            input_blocked_ = true;
            block_custom_updates(1);
        }
    }
}

void KnucklesTreasureHunt::custom_update() {
    if (emeralds_collected_ == 5 * level_) {
        level_up();
    }

    if (emerald_positions_.empty() && !is_falling_) {
        emerald_spawn_animation();
    }

    if (level_ == 6) {
        victory();
        return;
    }

    emerald_time_limit_--;
    if (emerald_time_limit_ < (-5 - level_)) {
        fall_animation();
    } else if (emerald_time_limit_ < 0) {
        queue_sound("emerald_blink.ogg");
        for (int emerald : emerald_positions_) {
            blink_element("emerald-" + std::to_string(emerald), 1);
        }
    }

    // Remove platforms that scrolled past the end of the screen.
    remove_value(platform_positions_, 2);
    remove_value(platform_positions_, 43);

    // Platforms move the left column first, then the right.
    for (int platform : std::vector<int>(platform_positions_)) {
        if (platform % 10 != platforms_to_move_next_update_) continue;
        remove_value(platform_positions_, platform);
        if (platforms_to_move_next_update_ == 2) {
            platform_positions_.push_back(platform - 10);
            if (knuckles_position_ == platform) knuckles_position_ -= 10;
        }
        if (platforms_to_move_next_update_ == 3) {
            platform_positions_.push_back(platform + 10);
            if (knuckles_position_ == platform) knuckles_position_ += 10;
        }
    }

    if (platforms_to_move_next_update_ == 2) {
        platforms_to_move_next_update_ = 3;
        if (kSpawnmapLeft[current_left_spawn_]) platform_positions_.push_back(32);
        current_left_spawn_++;
        if (current_left_spawn_ == (int)kSpawnmapLeft.size()) current_left_spawn_ = 0;
    } else {
        platforms_to_move_next_update_ = 2;
        if (kSpawnmapRight[current_right_spawn_]) platform_positions_.push_back(3);
        current_right_spawn_++;
        if (current_right_spawn_ == (int)kSpawnmapRight.size()) current_right_spawn_ = 0;
    }
}

void KnucklesTreasureHunt::level_up() {
    input_blocked_ = true;

    std::vector<std::string> f1 = get_knuckles_and_platform_elements();
    f1.insert(f1.end(), {EMERALD_1, EMERALD_2, EMERALD_3, EMERALD_4, EMERALD_5});
    std::vector<std::string> f2 = get_knuckles_and_platform_elements();

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 2; i++) {
        animation.insert(animation.end(), {f1, f1, f1, f1, f1, f1, f2, f2, f2, f2, f2, f2});
    }
    queue_sound("level_up.ogg");
    play_animation(animation);
    level_++;
    knuckles_position_ = 31;
    emeralds_collected_ = 0;
    life_count_ = 3;
    custom_update_speed_ -= 75;
}

void KnucklesTreasureHunt::victory() {
    queue_sound("game_win.ogg");

    const std::vector<std::string> f1 = {KNUCKLES_21};
    std::vector<std::string> f2 = {KNUCKLES_21, EMERALD_1, EMERALD_2, EMERALD_3, EMERALD_4, EMERALD_5};
    if (burger_) f2.push_back("win");

    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 5; i++) {
        animation.insert(animation.end(), {f1, f1, f1, f1, f1, f1, f2, f2, f2, f2, f2, f2});
    }
    play_animation(animation);
    knuckles_position_ = -1;
    stop();
}
