/**
 * @file joystick_handler.cpp
 * @brief 摇杆处理器实现
 */

#include "input/joystick_handler.hpp"
#include "hardware/input/joystick.hpp"
#include "hardware/input/hall_joystick_direct.hpp"
#include <cstdio>
#include <string>
#include <cmath>

namespace input {

JoystickHandler::JoystickHandler() 
    : is_initialized_(false)
    , joystick_(nullptr)
    , hall_joystick_direct_(nullptr)
    , deadzone_(20)
    , direction_ratio_(1.5f)
    , rotation_(JoystickRotation::ROTATION_0)
    , last_direction_(JoystickDirection::CENTER)
    , last_button_state_(false)
    , debug_mode_(false) {
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
    hall_joystick_direct_ = nullptr;
    
    printf("初始化摇杆处理器...\n");
    printf("死区设置: %d\n", deadzone_);
    printf("方向比例: %.2f\n", direction_ratio_);
    printf("旋转角度: %s\n", getRotationString(rotation_).c_str());
    
    is_initialized_ = true;
    printf("摇杆处理器初始化完成\n");
    
    return true;
}

bool JoystickHandler::initialize(HallJoystickDirect* joystick) {
    if (is_initialized_) {
        return true;
    }
    
    if (!joystick) {
        printf("错误: 霍尔摇杆直接连接实例为空\n");
        return false;
    }
    
    hall_joystick_direct_ = joystick;
    joystick_ = nullptr;
    
    printf("初始化霍尔摇杆直接连接处理器...\n");
    printf("死区设置: %d\n", deadzone_);
    printf("方向比例: %.2f\n", direction_ratio_);
    printf("旋转角度: %s\n", getRotationString(rotation_).c_str());
    
    is_initialized_ = true;
    printf("霍尔摇杆直接连接处理器初始化完成\n");
    
    return true;
}

void JoystickHandler::update() {
    if (!is_initialized_) return;
    
    int16_t x, y;
    bool button;
    
    if (joystick_) {
        // 使用I2C摇杆
        x = joystick_->get_joy_adc_12bits_offset_value_x();
        y = joystick_->get_joy_adc_12bits_offset_value_y();
        button = (joystick_->get_button_value() == 0); // 0表示按下，1表示释放
    } else if (hall_joystick_direct_) {
        // 使用霍尔摇杆直接连接
        x = hall_joystick_direct_->get_joy_adc_12bits_offset_value_x();
        y = hall_joystick_direct_->get_joy_adc_12bits_offset_value_y();
        button = (hall_joystick_direct_->get_button_value() == 0); // 0表示按下，1表示释放
        
        // 更新LED状态
        hall_joystick_direct_->update_led_from_joystick();
    } else {
        return;
    }
    
    // 处理方向
    JoystickDirection current_direction = processDirection(x, y);
    
    // 应用旋转
    JoystickDirection rotated_direction = applyRotation(current_direction);
    
    // 调试模式：打印原始ADC值和方向检测过程
    if (debug_mode_ && (current_direction != last_direction_ || button != last_button_state_)) {
        printf("摇杆调试 - X:%d, Y:%d, 原始方向:%s, 旋转后:%s, 按钮:%s\n", 
               x, y, 
               getDirectionString(current_direction).c_str(),
               getDirectionString(rotated_direction).c_str(),
               button ? "按下" : "释放");
    }
    
    // 更新状态
    last_direction_ = rotated_direction;
    last_button_state_ = button;
}

JoystickDirection JoystickHandler::processDirection(int16_t x, int16_t y) {
    // 应用死区
    if (abs(x) < deadzone_ && abs(y) < deadzone_) {
        return JoystickDirection::CENTER;
    }
    
    // 如果按钮按下，使用更大的死区来防止误触发
    if (last_button_state_) {
        int16_t button_deadzone = deadzone_ * 2;  // 按钮按下时使用2倍死区
        if (abs(x) < button_deadzone && abs(y) < button_deadzone) {
            return JoystickDirection::CENTER;
        }
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

void JoystickHandler::setRotation(JoystickRotation rotation) {
    rotation_ = rotation;
    printf("设置摇杆旋转: %s\n", getRotationString(rotation_).c_str());
}

int16_t JoystickHandler::getDeadzone() const {
    return deadzone_;
}

float JoystickHandler::getDirectionRatio() const {
    return direction_ratio_;
}

JoystickRotation JoystickHandler::getRotation() const {
    return rotation_;
}

void JoystickHandler::setDebugMode(bool enable) {
    debug_mode_ = enable;
    printf("摇杆调试模式: %s\n", enable ? "启用" : "禁用");
}

bool JoystickHandler::getDebugMode() const {
    return debug_mode_;
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

std::string JoystickHandler::getRotationString(JoystickRotation rotation) {
    switch (rotation) {
        case JoystickRotation::ROTATION_0:   return "0°";
        case JoystickRotation::ROTATION_90:  return "90°";
        case JoystickRotation::ROTATION_180: return "180°";
        case JoystickRotation::ROTATION_270: return "270°";
        default:                             return "UNKNOWN";
    }
}

JoystickDirection JoystickHandler::applyRotation(JoystickDirection original_direction) const {
    if (rotation_ == JoystickRotation::ROTATION_0) {
        return original_direction;
    }
    
    // 定义旋转映射表
    static const JoystickDirection rotation_maps[4][5] = {
        // ROTATION_0: 原始方向
        {JoystickDirection::CENTER, JoystickDirection::UP, JoystickDirection::DOWN, JoystickDirection::LEFT, JoystickDirection::RIGHT},
        // ROTATION_90: 顺时针旋转90度 (UP->RIGHT, RIGHT->DOWN, DOWN->LEFT, LEFT->UP)
        {JoystickDirection::CENTER, JoystickDirection::RIGHT, JoystickDirection::LEFT, JoystickDirection::UP, JoystickDirection::DOWN},
        // ROTATION_180: 旋转180度 (UP->DOWN, DOWN->UP, LEFT->RIGHT, RIGHT->LEFT)
        {JoystickDirection::CENTER, JoystickDirection::DOWN, JoystickDirection::UP, JoystickDirection::RIGHT, JoystickDirection::LEFT},
        // ROTATION_270: 逆时针旋转90度 (UP->LEFT, LEFT->DOWN, DOWN->RIGHT, RIGHT->UP)
        {JoystickDirection::CENTER, JoystickDirection::LEFT, JoystickDirection::RIGHT, JoystickDirection::DOWN, JoystickDirection::UP}
    };
    
    // 获取原始方向的索引
    int original_index = static_cast<int>(original_direction);
    int rotation_index = static_cast<int>(rotation_);
    
    // 返回旋转后的方向
    return rotation_maps[rotation_index][original_index];
}

} // namespace input 
