#pragma once

#include <cstdint>
#include "pico/stdlib.h"

namespace game2048 {

// 游戏棋盘大小
constexpr int BOARD_SIZE = 4;

// 游戏状态枚举
enum class GameStatus {
    READY,      // 准备开始
    PLAYING,    // 游戏中
    WON,        // 获胜
    GAME_OVER   // 游戏结束
};

// 棋盘数组类型
using BoardArray = int[BOARD_SIZE][BOARD_SIZE];

/**
 * @brief 游戏状态管理类
 * 
 * 负责管理2048游戏的状态，包括：
 * - 棋盘数据
 * - 分数记录
 * - 游戏状态
 * - 时间记录
 */
class GameState {
public:
    GameState();
    ~GameState();

    // 禁用拷贝构造和赋值
    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;

    // 棋盘管理
    void resetBoard();
    void addRandomTile();
    
    // 游戏状态检查
    bool isGameOver() const;
    bool hasWon() const;
    
    // 分数管理
    void updateScore(int points);
    uint32_t getCurrentScore() const;
    uint32_t getBestScore() const;
    
    // 游戏状态管理
    void setGameStatus(GameStatus status);
    GameStatus getGameStatus() const;
    
    // 时间管理
    uint32_t getGameTime() const;
    void updateLastMoveTime();
    
    // 棋盘访问
    const BoardArray& getBoard() const;
    BoardArray& getBoard();

private:
    uint32_t current_score_;      // 当前分数
    uint32_t best_score_;         // 最佳分数
    GameStatus game_status_;      // 游戏状态
    BoardArray board_;            // 游戏棋盘
    uint32_t last_move_time_;     // 最后移动时间
    uint32_t game_start_time_;    // 游戏开始时间
};

} // namespace game2048 
