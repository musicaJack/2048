#pragma once

#include "input/joystick_handler.hpp"
#include "hardware/input/joystick.hpp"
#include "game/game_2048.hpp"
#include "game/game_config.hpp"
#include <cstdint>

namespace game2048 {

// 输入处理器
class GameInputHandler {
public:
    GameInputHandler(input::JoystickHandler* joystick_handler);
    
    // 处理输入
    void handleInput(GameLogic& logic, GameState& game_state);
    
    // 重置游戏
    void resetGame(Game2048* game);
    
    // 设置输入使能
    void setInputEnabled(bool enabled) { input_enabled_ = enabled; }
    bool isInputEnabled() const { return input_enabled_; }
    
    // 获取最后输入时间
    uint32_t getLastInputTime() const { return last_input_time_; }
    
    // 清除最后输入时间
    void clearLastInputTime() { last_input_time_ = 0; }
    
    // 检查是否有重置请求
    bool isResetRequested() { 
        bool requested = button_reset_requested_; 
        button_reset_requested_ = false; // 清除标志
        return requested; 
    }

private:
    input::JoystickHandler* joystick_handler_;
    bool input_enabled_;
    uint32_t last_input_time_;
    bool last_button_state_;
    bool button_reset_requested_; // 重置请求标志
    
    // 处理方向输入
    bool handleDirectionInput(GameLogic& logic, GameState& game_state, 
                            input::JoystickDirection direction);
    
    // 处理按钮输入
    void handleButtonInput(Game2048* game);
};

} // namespace game2048
