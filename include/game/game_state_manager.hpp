#pragma once

#include "game/game_2048.hpp"
#include "game/game_ui.hpp"
#include <cstdint>

namespace game2048 {

// 游戏状态管理器
class GameStateManager {
public:
    GameStateManager(Game2048* game, GameUIRenderer* ui_renderer);
    
    // 检查游戏状态
    void checkGameStatus(GameLogic& logic, GameState& game_state);
    
    // 重置游戏
    void resetGame();
    
    // 状态查询
    LocalGameState getCurrentState() const { return current_state_; }
    bool isPlaying() const { return current_state_ == LocalGameState::PLAYING; }
    bool isWon() const { return current_state_ == LocalGameState::WON; }
    bool isGameOver() const { return current_state_ == LocalGameState::GAME_OVER; }
    
    // 设置状态
    void setState(LocalGameState state) { current_state_ = state; }

private:
    Game2048* game_;
    GameUIRenderer* ui_renderer_;
    LocalGameState current_state_;
    
    // 处理获胜状态
    void handleWonState(GameState& game_state);
    
    // 处理游戏结束状态
    void handleGameOverState(GameState& game_state);
};

} // namespace game2048
