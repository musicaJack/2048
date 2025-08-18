#pragma once

#include <cstdint>

namespace utils {

/**
 * @brief 游戏计时器类
 * 
 * 负责管理游戏时间，包括开始、暂停、恢复等功能
 */
class GameTimer {
public:
    GameTimer();
    ~GameTimer();

    // 禁用拷贝构造和赋值
    GameTimer(const GameTimer&) = delete;
    GameTimer& operator=(const GameTimer&) = delete;

    // 计时器控制
    void start();
    void pause();
    void resume();
    void stop();

    // 时间查询
    uint32_t getElapsedTime() const;
    bool isPaused() const;

private:
    uint32_t start_time_;
    uint32_t pause_time_;
    bool is_paused_;
};

} // namespace utils 
