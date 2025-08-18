/**
 * @file game_state.cpp
 * @brief 2048游戏状态管理实现
 */

#include "game/game_state.hpp"
#include <cstring>
#include <cstdlib>

namespace game2048 {

GameState::GameState() 
    : current_score_(0)
    , best_score_(0)
    , game_status_(GameStatus::READY)
    , board_()
    , last_move_time_(0)
    , game_start_time_(0) {
    
    // 初始化空棋盘
    resetBoard();
}

GameState::~GameState() = default;

void GameState::resetBoard() {
    // 清空棋盘
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board_[i][j] = 0;
        }
    }
    
    // 添加两个初始数字
    addRandomTile();
    addRandomTile();
    
    current_score_ = 0;
    game_status_ = GameStatus::PLAYING;
    game_start_time_ = to_ms_since_boot(get_absolute_time());
    last_move_time_ = game_start_time_;
}

void GameState::addRandomTile() {
    // 统计空位置
    int empty_count = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board_[i][j] == 0) {
                empty_count++;
            }
        }
    }
    
    if (empty_count == 0) return;
    
    // 随机选择一个空位置
    int target_pos = rand() % empty_count;
    int current_pos = 0;
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board_[i][j] == 0) {
                if (current_pos == target_pos) {
                    // 90%概率生成2，10%概率生成4
                    board_[i][j] = (rand() % 10 < 9) ? 2 : 4;
                    return;
                }
                current_pos++;
            }
        }
    }
}

bool GameState::isGameOver() const {
    // 检查是否有空位置
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board_[i][j] == 0) {
                return false;
            }
        }
    }
    
    // 检查是否可以合并
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            int current = board_[i][j];
            
            // 检查右边
            if (j < BOARD_SIZE - 1 && board_[i][j + 1] == current) {
                return false;
            }
            
            // 检查下边
            if (i < BOARD_SIZE - 1 && board_[i + 1][j] == current) {
                return false;
            }
        }
    }
    
    return true;
}

bool GameState::hasWon() const {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board_[i][j] >= 2048) {
                return true;
            }
        }
    }
    return false;
}

void GameState::updateScore(int points) {
    current_score_ += points;
    if (current_score_ > best_score_) {
        best_score_ = current_score_;
    }
}

void GameState::setGameStatus(GameStatus status) {
    game_status_ = status;
}

GameStatus GameState::getGameStatus() const {
    return game_status_;
}

uint32_t GameState::getCurrentScore() const {
    return current_score_;
}

uint32_t GameState::getBestScore() const {
    return best_score_;
}

uint32_t GameState::getGameTime() const {
    if (game_status_ == GameStatus::PLAYING) {
        return to_ms_since_boot(get_absolute_time()) - game_start_time_;
    }
    return last_move_time_ - game_start_time_;
}

const BoardArray& GameState::getBoard() const {
    return board_;
}

BoardArray& GameState::getBoard() {
    return board_;
}

void GameState::updateLastMoveTime() {
    last_move_time_ = to_ms_since_boot(get_absolute_time());
}

} // namespace game2048 
