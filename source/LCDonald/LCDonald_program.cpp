#include "LCDonald_program.h"

#include <algorithm>
#include <cmath>

bool LCDonald_program::hidden_group_visible() const {
    return burger_ && hidden_group_until_us_ != 0 && time_us_64_p() < hidden_group_until_us_;
}

std::vector<std::string> LCDonald_program::get_visible_game_elements() {
    if (is_stopped_) {
        return get_all_game_elements();
    }

    if (has_animation_) {
        return current_animation_[animation_index_];
    }

    std::vector<std::string> visible = get_visible_elements();

    // Burger's hidden group is added before blink handling, same as the C#.
    if (hidden_group_visible()) visible.push_back("score-1");

    for (auto it = blinking_elements_.begin(); it != blinking_elements_.end(); ++it) {
        const std::string& element = it->first;
        const float value = it->second;
        // Matches C#'s `value % 1 != 0.5` visibility check (two frames per
        // blink cycle: a whole-number value is visible, a .5 value isn't).
        const float frac = value - std::floor(value);
        const bool is_visible = std::abs(frac - 0.5f) > 0.001f;

        auto found = std::find(visible.begin(), visible.end(), element);
        if (is_visible) {
            if (found == visible.end()) visible.push_back(element);
        } else {
            if (found != visible.end()) visible.erase(found);
        }
    }

    return visible;
}

void LCDonald_program::start(bool is_endless) {
    input_blocked_ = false;
    stop_pending_ = false;
    has_animation_ = false;
    current_animation_.clear();
    animation_index_ = 0;
    has_triggered_hidden_group_ = false;
    hidden_group_until_us_ = 0;
    is_endless_mode_ = is_endless;
    initialize_game_state();
    is_stopped_ = false;
}

void LCDonald_program::pause() {
    if (!is_paused_ && !is_stopped_) {
        is_paused_ = true;
    }
}

void LCDonald_program::resume() {
    if (is_paused_ && !is_stopped_) {
        is_paused_ = false;
    }
}

void LCDonald_program::stop() {
    // Every game's game_over()/victory() plays its animation and THEN calls
    // stop(). In the original PlayAnimation() blocks its thread until the
    // animation has finished, so that Stop() only takes effect afterwards;
    // here play_animation() returns immediately, so stopping right away
    // would wipe the animation before a single frame showed (and, since
    // custom_update() kept running on a "stopped" game, re-trigger
    // game_over() - and its jingle - on every custom-update tick).
    // Deferring until tick() has played out the last frame restores the
    // original ordering.
    if (has_animation_) {
        stop_pending_ = true;
        return;
    }
    finish_stop();
}

void LCDonald_program::finish_stop() {
    stop_pending_ = false;
    is_stopped_ = true;
    is_paused_ = false;
    has_animation_ = false;
    current_animation_.clear();
    animation_index_ = 0;
}

std::vector<std::string> LCDonald_program::take_queued_sounds() {
    std::vector<std::string> out;
    out.swap(queued_sounds_);
    return out;
}

int LCDonald_program::take_score_events() {
    const int out = score_events_;
    score_events_ = 0;
    return out;
}

void LCDonald_program::tick() {
    // A stopped game is finished: Stop() halts the original's custom timer,
    // and nothing left should move or re-queue sounds.
    if (is_stopped_) return;

    update_core();

    for (auto it = blinking_elements_.begin(); it != blinking_elements_.end();) {
        it->second -= 0.5f;
        if (it->second < 0.0f) {
            it = blinking_elements_.erase(it);
        } else {
            ++it;
        }
    }

    if (has_animation_) {
        ++animation_index_;
        if (animation_index_ >= current_animation_.size()) {
            has_animation_ = false;
            current_animation_.clear();
            animation_index_ = 0;
            input_blocked_ = false;
            if (stop_pending_) finish_stop();
        }
    }
}

void LCDonald_program::step_custom_update() {
    if (is_stopped_) return;
    if (blocked_custom_updates_ > 0) {
        --blocked_custom_updates_;
    } else {
        input_blocked_ = false;
        custom_update();
    }

    // Burger only: a 1-in-9 chance on every update, once per game, to flash
    // the hidden group (LCDGameBase.UpdateGameState()'s `_rng.Next(1, 10) == 1`).
    if (burger_ && !has_triggered_hidden_group_ && rand_range(1, 9) == 1) {
        has_triggered_hidden_group_ = true;
        hidden_group_until_us_ = time_us_64_p() + 50000;
    }
}

void LCDonald_program::queue_sound(const std::string& file) {
    queued_sounds_.push_back(file);
}

bool LCDonald_program::is_blinking(const std::string& element) const {
    return blinking_elements_.find(element) != blinking_elements_.end();
}

void LCDonald_program::blink_element(const std::string& element, int times) {
    blinking_elements_[element] = (float)times;
}

void LCDonald_program::play_animation(const std::vector<std::vector<std::string>>& animation) {
    input_blocked_ = true;
    current_animation_ = animation;
    animation_index_ = 0;
    has_animation_ = !current_animation_.empty();
    if (!has_animation_) {
        input_blocked_ = false;
    }
    // Unlike the C# original, this does NOT block the calling thread until
    // the animation finishes - see LCDonald_program.h's class comment.
}

void LCDonald_program::generic_game_over_animation(const std::vector<std::string>& elements_to_blink,
                                                     const std::string& animation_sound) {
    queue_sound(animation_sound);

    const std::vector<std::string> frame1 = elements_to_blink;
    const std::vector<std::string> frame2;

    // Slow 4x blink, same shape as the C# original.
    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 4; i++) {
        animation.insert(animation.end(), {frame1, frame1, frame1, frame1, frame2, frame2, frame2, frame2});
    }
    play_animation(animation);
}

void LCDonald_program::generic_victory_animation(const std::vector<std::string>& elements_to_blink) {
    queue_sound("../common/game_win.ogg");

    const std::vector<std::string> frame1 = elements_to_blink;
    const std::vector<std::string> frame2;
    std::vector<std::string> frame3 = get_all_game_elements();
    if (burger_) frame3.push_back("win");  // Burger's extra "WINNER" image.

    // 3x blink of elements_to_blink, then 10x blink of everything - same
    // shape as the C# original.
    std::vector<std::vector<std::string>> animation;
    for (int i = 0; i < 3; i++) {
        animation.insert(animation.end(), {frame1, frame1, frame1, frame1, frame1, frame1, frame2, frame2, frame2, frame2, frame2, frame2});
    }
    for (int i = 0; i < 10; i++) {
        animation.insert(animation.end(), {frame3, frame3, frame3, frame3, frame3, frame3, frame2, frame2, frame2, frame2, frame2, frame2});
    }
    play_animation(animation);
}

void LCDonald_program::startup_music(const std::string& music_file, int block_input_cycles) {
    queue_sound(music_file);
    blocked_custom_updates_ = block_input_cycles;
    input_blocked_ = true;
}

int LCDonald_program::rand_range(int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng_);
}
