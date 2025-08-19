#pragma once

#include "game/game_2048.hpp"
#include "game/game_ui.hpp"
#include "game/game_animation.hpp"
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
    
    // 成就检测
    void checkAchievements(const GameState& game_state);
    void setAnimationManager(GameAnimationManager* anim_manager) { anim_manager_ = anim_manager; }
    
    // 合并检测和星星奖励
    void checkMerges(const GameState& game_state);
    uint8_t calculateStarReward(int merged_value);
    
    // 获取当前星星数量
    uint32_t getTotalStars() const { return anim_manager_ ? anim_manager_->getTotalStars() : 0; }

private:
    Game2048* game_;
    GameUIRenderer* ui_renderer_;
    GameAnimationManager* anim_manager_;
    LocalGameState current_state_;
    
    // 成就追踪
    int last_max_value_;
    
    // 合并追踪
    BoardArray last_board_state_;
    bool board_initialized_;
    
    // 星星奖励规则
    static constexpr uint8_t STAR_REWARDS[] = {
        1,  // 2+2=4   → 1颗星
        1,  // 4+4=8   → 1颗星
        2,  // 8+8=16  → 2颗星
        3,  // 16+16=32 → 3颗星
        4,  // 32+32=64 → 4颗星
        5,  // 64+64=128 → 5颗星
        6,  // 128+128=256 → 6颗星
        7,  // 256+256=512 → 7颗星
        8,  // 512+512=1024 → 8颗星
        10  // 1024+1024=2048 → 10颗星
    };
    static constexpr size_t STAR_REWARDS_COUNT = sizeof(STAR_REWARDS) / sizeof(STAR_REWARDS[0]);
    
    // 处理获胜状态
    void handleWonState(GameState& game_state);
    
    // 处理游戏结束状态
    void handleGameOverState(GameState& game_state);
};

} // namespace game2048
