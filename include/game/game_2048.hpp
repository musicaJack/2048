#pragma once

#include "game/game_state.hpp"
#include "game/game_logic.hpp"

namespace game2048 {

/**
 * @brief 2048主游戏类
 * 
 * 整合游戏状态管理和游戏逻辑，提供完整的游戏接口
 */
class Game2048 {
public:
    Game2048();
    ~Game2048();

    // 禁用拷贝构造和赋值
    Game2048(const Game2048&) = delete;
    Game2048& operator=(const Game2048&) = delete;

    // 游戏初始化和控制
    bool initialize();
    void reset();
    
    // 游戏操作
    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();
    
    // 状态查询
    bool isGameOver() const;
    bool hasWon() const;
    bool canMove() const;
    
    // 访问器
    GameState& getGameState();
    const GameState& getGameState() const;
    GameLogic& getGameLogic();
    const GameLogic& getGameLogic() const;

private:
    GameState game_state_;
    GameLogic game_logic_;
    bool is_initialized_;
};

} // namespace game2048 
