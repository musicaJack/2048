#pragma once

#include <cstdint>

namespace utils {

/**
 * @brief 分数管理器类
 * 
 * 负责管理游戏分数，包括当前分数和最高分数
 */
class ScoreManager {
public:
    ScoreManager();
    ~ScoreManager();

    // 禁用拷贝构造和赋值
    ScoreManager(const ScoreManager&) = delete;
    ScoreManager& operator=(const ScoreManager&) = delete;

    // 分数操作
    void addScore(uint32_t points);
    void resetScore();
    uint32_t getCurrentScore() const;
    uint32_t getBestScore() const;
    void setBestScore(uint32_t score);

private:
    uint32_t current_score_;
    uint32_t best_score_;
};

} // namespace utils 
