#include "game/game_state_manager.hpp"
#include "pico/stdlib.h"
#include <cstdio>

namespace game2048 {

GameStateManager::GameStateManager(Game2048* game, GameUIRenderer* ui_renderer)
    : game_(game), ui_renderer_(ui_renderer), current_state_(LocalGameState::INIT) {
}

void GameStateManager::checkGameStatus(GameLogic& logic, GameState& game_state) {
    // 检查是否获胜
    if (game_state.hasWon() && game_state.getGameStatus() == GameStatus::PLAYING) {
        handleWonState(game_state);
    }
    
    // 检查游戏是否结束
    if (game_state.isGameOver()) {
        handleGameOverState(game_state);
    }
}

void GameStateManager::resetGame() {
    if (game_) {
        game_->reset();
        current_state_ = LocalGameState::PLAYING;
        
        // 重置UI缓存
        if (ui_renderer_) {
            ui_renderer_->resetCache();
            ui_renderer_->setFirstDraw(true);
        }
        
        printf("游戏重置完成!\n");
    }
}

void GameStateManager::handleWonState(GameState& game_state) {
    game_state.setGameStatus(GameStatus::WON);
    current_state_ = LocalGameState::WON;
    printf("🎉 恭喜! 你达到了2048! 🎉\n");
    
    // 更新状态栏显示
    if (ui_renderer_) {
        ui_renderer_->drawStatusBarOptimized(game_state);
    }
}

void GameStateManager::handleGameOverState(GameState& game_state) {
    game_state.setGameStatus(GameStatus::GAME_OVER);
    current_state_ = LocalGameState::GAME_OVER;
    printf("💀 游戏结束! 没有可移动的位置了! 💀\n");
    
    // 更新状态栏显示
    if (ui_renderer_) {
        ui_renderer_->drawStatusBarOptimized(game_state);
    }
}

} // namespace game2048
