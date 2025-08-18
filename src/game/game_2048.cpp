/**
 * @file game_2048.cpp
 * @brief 2048主游戏类实现
 */

#include "game/game_2048.hpp"
#include <cstdio>

namespace game2048 {

Game2048::Game2048() 
    : game_state_()
    , game_logic_()
    , is_initialized_(false) {
}

Game2048::~Game2048() = default;

bool Game2048::initialize() {
    if (is_initialized_) {
        return true;
    }
    
    printf("初始化2048游戏...\n");
    
    // 重置游戏状态
    reset();
    
    is_initialized_ = true;
    printf("2048游戏初始化完成\n");
    
    return true;
}

void Game2048::reset() {
    game_state_.resetBoard();
    printf("游戏已重置\n");
}

bool Game2048::moveLeft() {
    if (!is_initialized_) return false;
    return game_logic_.moveLeft(game_state_);
}

bool Game2048::moveRight() {
    if (!is_initialized_) return false;
    return game_logic_.moveRight(game_state_);
}

bool Game2048::moveUp() {
    if (!is_initialized_) return false;
    return game_logic_.moveUp(game_state_);
}

bool Game2048::moveDown() {
    if (!is_initialized_) return false;
    return game_logic_.moveDown(game_state_);
}

bool Game2048::isGameOver() const {
    return game_state_.isGameOver();
}

bool Game2048::hasWon() const {
    return game_state_.hasWon();
}

bool Game2048::canMove() const {
    return game_logic_.canMove(game_state_);
}

GameState& Game2048::getGameState() {
    return game_state_;
}

const GameState& Game2048::getGameState() const {
    return game_state_;
}

GameLogic& Game2048::getGameLogic() {
    return game_logic_;
}

const GameLogic& Game2048::getGameLogic() const {
    return game_logic_;
}

} // namespace game2048 
