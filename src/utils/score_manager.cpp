/**
 * @file score_manager.cpp
 * @brief 分数管理器实现
 */

#include "utils/score_manager.hpp"

namespace utils {

ScoreManager::ScoreManager() 
    : current_score_(0)
    , best_score_(0) {
}

ScoreManager::~ScoreManager() = default;

void ScoreManager::addScore(uint32_t points) {
    current_score_ += points;
    if (current_score_ > best_score_) {
        best_score_ = current_score_;
    }
}

void ScoreManager::resetScore() {
    current_score_ = 0;
}

uint32_t ScoreManager::getCurrentScore() const {
    return current_score_;
}

uint32_t ScoreManager::getBestScore() const {
    return best_score_;
}

void ScoreManager::setBestScore(uint32_t score) {
    if (score > best_score_) {
        best_score_ = score;
    }
}

} // namespace utils 
