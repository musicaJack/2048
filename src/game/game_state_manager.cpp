#include "game/game_state_manager.hpp"
#include "game/game_config.hpp" // For BOARD_SIZE
#include <algorithm> // For std::max

namespace game2048 {

GameStateManager::GameStateManager(Game2048* game, GameUIRenderer* ui_renderer)
    : game_(game), ui_renderer_(ui_renderer), anim_manager_(nullptr),
      current_state_(LocalGameState::INIT), last_max_value_(0),
      board_initialized_(false) { // 初始化合并追踪
}

void GameStateManager::checkGameStatus(GameLogic& logic, GameState& game_state) {
    // 检查合并和星星奖励
    checkMerges(game_state);
    
    // 检查成就
    checkAchievements(game_state);
    
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
    game_->reset();
    current_state_ = LocalGameState::PLAYING;
    last_max_value_ = 0;
    board_initialized_ = false; // 重置合并追踪
    
    // 重置星星计数
    if (anim_manager_) {
        anim_manager_->resetStars();
    }
    
    // 重置UI缓存
    if (ui_renderer_) {
        ui_renderer_->resetCache();
        ui_renderer_->setFirstDraw(true);
    }
    
    printf("游戏重置完成!\n");
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

void GameStateManager::checkAchievements(const GameState& game_state) {
    if (!anim_manager_) return;
    
    // 获取当前棋盘上的最大值
    const BoardArray& board = game_state.getBoard();
    int current_max = 0;
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board[i][j] > current_max) {
                current_max = board[i][j];
            }
        }
    }
    
    // 检查是否有新的成就（仅ILI9488显示成就动画）
    if (current_max > last_max_value_) {
        // 检查是否达到特定成就等级
        if (current_max >= 32 && last_max_value_ < 32) {
            anim_manager_->startAchievementAnimation(AchievementLevel::ACHIEVEMENT_32);
        } else if (current_max >= 64 && last_max_value_ < 64) {
            anim_manager_->startAchievementAnimation(AchievementLevel::ACHIEVEMENT_64);
        } else if (current_max >= 128 && last_max_value_ < 128) {
            anim_manager_->startAchievementAnimation(AchievementLevel::ACHIEVEMENT_128);
        } else if (current_max >= 256 && last_max_value_ < 256) {
            anim_manager_->startAchievementAnimation(AchievementLevel::ACHIEVEMENT_256);
        } else if (current_max >= 512 && last_max_value_ < 512) {
            anim_manager_->startAchievementAnimation(AchievementLevel::ACHIEVEMENT_512);
        } else if (current_max >= 1024 && last_max_value_ < 1024) {
            anim_manager_->startAchievementAnimation(AchievementLevel::ACHIEVEMENT_1024);
        } else if (current_max >= 2048 && last_max_value_ < 2048) {
            anim_manager_->startAchievementAnimation(AchievementLevel::ACHIEVEMENT_2048);
        }
        
        last_max_value_ = current_max;
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

void GameStateManager::checkMerges(const GameState& game_state) {
    const BoardArray& current_board = game_state.getBoard();
    
    // 第一次初始化
    if (!board_initialized_) {
        // 复制数组内容
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                last_board_state_[i][j] = current_board[i][j];
            }
        }
        board_initialized_ = true;
        return;
    }
    
    // 检测合并事件
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            int current_value = current_board[i][j];
            int last_value = last_board_state_[i][j];
            
            // 如果当前值大于上次值，说明发生了合并
            if (current_value > last_value && current_value > 0) {
                // 计算星星奖励
                uint8_t star_reward = calculateStarReward(current_value);
                
                if (star_reward > 0 && anim_manager_) {
                    printf("检测到合并: %d → %d, 获得 %d 颗星\n", last_value, current_value, star_reward);
                    anim_manager_->startStarGainAnimation(star_reward);
                }
            }
        }
    }
    
    // 更新上次状态
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            last_board_state_[i][j] = current_board[i][j];
        }
    }
}

uint8_t GameStateManager::calculateStarReward(int merged_value) {
    // 根据合并后的数值计算星星奖励
    // 合并后的数值应该是2的幂次方
    int power = 0;
    int temp = merged_value;
    
    while (temp > 1 && temp % 2 == 0) {
        temp /= 2;
        power++;
    }
    
    if (temp != 1) {
        return 0; // 不是2的幂次方，不是有效合并
    }
    
    // 根据幂次计算星星奖励
    if (power >= 2 && power - 2 < static_cast<int>(STAR_REWARDS_COUNT)) {
        return STAR_REWARDS[power - 2];
    }
    
    return 0; // 超出范围
}

} // namespace game2048
