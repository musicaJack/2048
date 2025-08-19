#include "game/game_input.hpp"
#include "pico/stdlib.h"
#include <cstdio>

namespace game2048 {

GameInputHandler::GameInputHandler(input::JoystickHandler* joystick_handler)
    : joystick_handler_(joystick_handler), input_enabled_(true), 
      last_input_time_(0), last_button_state_(false), button_reset_requested_(false) {
}

void GameInputHandler::handleInput(GameLogic& logic, GameState& game_state) {
    if (!joystick_handler_ || !input_enabled_) {
        return;
    }
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // 更新摇杆状态
    joystick_handler_->update();
    
    // 获取当前方向
    input::JoystickDirection direction = joystick_handler_->getCurrentDirection();
    
    // 检查按钮状态
    bool button_pressed = joystick_handler_->isButtonPressed();
    
    // 处理方向输入（防抖）
    if (current_time - last_input_time_ > GameConfig::INPUT_DEBOUNCE_MS) {
        bool moved = handleDirectionInput(logic, game_state, direction);
        if (moved) {
            last_input_time_ = current_time;
        }
    }
    
    // 处理按钮输入（重置游戏）
    if (button_pressed && !last_button_state_) {
        printf("检测到重置按钮按下\n");
        button_reset_requested_ = true; // 设置重置请求标志
        last_input_time_ = current_time;
    }
    last_button_state_ = button_pressed;
}

bool GameInputHandler::handleDirectionInput(GameLogic& logic, GameState& game_state, 
                                          input::JoystickDirection direction) {
    switch (direction) {
        case input::JoystickDirection::UP:
            return logic.moveUp(game_state);
        case input::JoystickDirection::DOWN:
            return logic.moveDown(game_state);
        case input::JoystickDirection::LEFT:
            return logic.moveLeft(game_state);
        case input::JoystickDirection::RIGHT:
            return logic.moveRight(game_state);
        default:
            return false;
    }
}

void GameInputHandler::resetGame(Game2048* game) {
    if (game) {
        game->reset();
        printf("游戏重置完成!\n");
    }
}

} // namespace game2048
