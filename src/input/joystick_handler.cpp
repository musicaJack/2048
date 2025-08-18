/**
 * @file joystick_handler.cpp
 * @brief 摇杆处理器实现
 */

#include "input/joystick_handler.hpp"
#include "hardware/input/joystick.hpp"
#include <cstdio>
#include <string>
#include <cmath>

namespace input {

JoystickHandler::JoystickHandler() 
    : is_initialized_(false)
    , joystick_(nullptr)
    , deadzone_(20)
    , direction_ratio_(1.5f)
    , last_direction_(JoystickDirection::CENTER)
    , last_button_state_(false) {
}

JoystickHandler::~JoystickHandler() = default;

bool JoystickHandler::initialize(Joystick* joystick) {
    if (is_initialized_) {
        return true;
    }
    
    if (!joystick) {
        printf("错误: 摇杆实例为空\n");
        return false;
    }
    
    joystick_ = joystick;
    
    printf("初始化摇杆处理器...\n");
    printf("死区设置: %d\n", deadzone_);
    printf("方向比例: %.2f\n", direction_ratio_);
    
    is_initialized_ = true;
    printf("摇杆处理器初始化完成\n");
    
    return true;
}

void JoystickHandler::update() {
    if (!is_initialized_ || !joystick_) return;
    
    // 读取摇杆数据
    int16_t x = joystick_->get_joy_adc_12bits_offset_value_x();
    int16_t y = joystick_->get_joy_adc_12bits_offset_value_y();
    bool button = (joystick_->get_button_value() == 0); // 0表示按下，1表示释放
    
    // 处理方向
    JoystickDirection current_direction = processDirection(x, y);
    
    // 检查方向变化
    if (current_direction != last_direction_) {
        printf("摇杆方向变化: %s -> %s\n", 
               getDirectionString(last_direction_).c_str(),
               getDirectionString(current_direction).c_str());
        last_direction_ = current_direction;
    }
    
    // 检查按钮状态变化
    if (button != last_button_state_) {
        printf("摇杆按钮: %s\n", button ? "按下" : "释放");
        last_button_state_ = button;
    }
    
    // 只在数据变化时打印详细信息
    static int16_t last_x = 0, last_y = 0;
    static bool last_button = false;
    
    if (x != last_x || y != last_y || button != last_button) {
        printf("Joystick - X: %d, Y: %d, Button: %s\n", 
               x, y, button ? "PRESSED" : "RELEASED");
        last_x = x;
        last_y = y;
        last_button = button;
    }
}

JoystickDirection JoystickHandler::processDirection(int16_t x, int16_t y) {
    // 应用死区
    if (abs(x) < deadzone_ && abs(y) < deadzone_) {
        return JoystickDirection::CENTER;
    }
    
    // 计算方向角度
    float angle = atan2(y, x) * 180.0f / 3.14159f;
    if (angle < 0) angle += 360.0f;
    
    // 确定主要方向
    if (angle >= 315.0f || angle < 45.0f) {
        return JoystickDirection::RIGHT;
    } else if (angle >= 45.0f && angle < 135.0f) {
        return JoystickDirection::DOWN;
    } else if (angle >= 135.0f && angle < 225.0f) {
        return JoystickDirection::LEFT;
    } else {
        return JoystickDirection::UP;
    }
}

JoystickDirection JoystickHandler::getCurrentDirection() const {
    return last_direction_;
}

bool JoystickHandler::isButtonPressed() const {
    return last_button_state_;
}

bool JoystickHandler::isButtonJustPressed() const {
    // 这里需要跟踪按钮状态变化
    return last_button_state_;
}

bool JoystickHandler::isButtonJustReleased() const {
    // 这里需要跟踪按钮状态变化
    return !last_button_state_;
}

void JoystickHandler::setDeadzone(int16_t deadzone) {
    deadzone_ = deadzone;
    printf("设置摇杆死区: %d\n", deadzone_);
}

void JoystickHandler::setDirectionRatio(float ratio) {
    direction_ratio_ = ratio;
    printf("设置方向比例: %.2f\n", direction_ratio_);
}

int16_t JoystickHandler::getDeadzone() const {
    return deadzone_;
}

float JoystickHandler::getDirectionRatio() const {
    return direction_ratio_;
}

std::string JoystickHandler::getDirectionString(JoystickDirection direction) {
    switch (direction) {
        case JoystickDirection::UP:     return "UP";
        case JoystickDirection::DOWN:   return "DOWN";
        case JoystickDirection::LEFT:   return "LEFT";
        case JoystickDirection::RIGHT:  return "RIGHT";
        case JoystickDirection::CENTER: return "CENTER";
        default:                        return "UNKNOWN";
    }
}

} // namespace input 
