#pragma once

#include "game/game_state.hpp"

namespace game2048 {

/**
 * @brief 2048游戏逻辑类
 * 
 * 负责实现2048游戏的核心逻辑，包括：
 * - 四个方向的移动逻辑
 * - 数字合并规则
 * - 游戏状态检查
 * - 分数计算
 */
class GameLogic {
public:
    GameLogic();
    ~GameLogic();

    // 禁用拷贝构造和赋值
    GameLogic(const GameLogic&) = delete;
    GameLogic& operator=(const GameLogic&) = delete;

    // 主要移动方法
    bool moveLeft(GameState& game_state);
    bool moveRight(GameState& game_state);
    bool moveUp(GameState& game_state);
    bool moveDown(GameState& game_state);

    // 辅助方法
    bool canMove(const GameState& game_state) const;

private:
    // 行移动辅助方法。gained 累加本次合并得到的新方块数值。
    bool moveRowLeft(int row[BOARD_SIZE], int& gained);
    bool moveRowRight(int row[BOARD_SIZE], int& gained);
    
    // 列移动辅助方法
    bool moveColumnUp(BoardArray& board, int col, int& gained);
    bool moveColumnDown(BoardArray& board, int col, int& gained);
};

} // namespace game2048 
