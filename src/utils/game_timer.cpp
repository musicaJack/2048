/**
 * @file game_timer.cpp
 * @brief 游戏计时器实现
 */

#include "utils/game_timer.hpp"
#include "pico/stdlib.h"

namespace utils {

GameTimer::GameTimer() 
    : start_time_(0)
    , pause_time_(0)
    , is_paused_(false) {
}

GameTimer::~GameTimer() = default;

void GameTimer::start() {
    start_time_ = to_ms_since_boot(get_absolute_time());
    pause_time_ = 0;
    is_paused_ = false;
}

void GameTimer::pause() {
    if (!is_paused_) {
        pause_time_ = to_ms_since_boot(get_absolute_time());
        is_paused_ = true;
    }
}

void GameTimer::resume() {
    if (is_paused_) {
        start_time_ += to_ms_since_boot(get_absolute_time()) - pause_time_;
        is_paused_ = false;
    }
}

void GameTimer::stop() {
    start_time_ = 0;
    pause_time_ = 0;
    is_paused_ = false;
}

uint32_t GameTimer::getElapsedTime() const {
    if (start_time_ == 0) return 0;
    
    if (is_paused_) {
        return pause_time_ - start_time_;
    } else {
        return to_ms_since_boot(get_absolute_time()) - start_time_;
    }
}

bool GameTimer::isPaused() const {
    return is_paused_;
}

} // namespace utils 
