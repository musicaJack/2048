/**
 * @file input_manager.cpp
 * @brief 输入管理器实现
 */

#include "input/input_manager.hpp"
#include "pico/stdlib.h"
#include <cstdio>

namespace input {

InputManager::InputManager() 
    : is_initialized_(false)
    , current_input_(InputType::NONE)
    , last_input_(InputType::NONE)
    , input_timestamp_(0) {
}

InputManager::~InputManager() = default;

bool InputManager::initialize() {
    if (is_initialized_) {
        return true;
    }
    
    printf("初始化输入管理器...\n");
    
    // 这里可以初始化各种输入设备
    // 目前只是简单的实现
    
    is_initialized_ = true;
    printf("输入管理器初始化完成\n");
    
    return true;
}

void InputManager::update() {
    if (!is_initialized_) return;
    
    // 更新输入状态
    last_input_ = current_input_;
    current_input_ = InputType::NONE;
    
    // 这里应该读取实际的输入设备
    // 目前只是占位实现
}

InputType InputManager::getCurrentInput() const {
    return current_input_;
}

InputType InputManager::getLastInput() const {
    return last_input_;
}

bool InputManager::isInputPressed(InputType input) const {
    return current_input_ == input;
}

bool InputManager::isInputJustPressed(InputType input) const {
    return current_input_ == input && last_input_ != input;
}

bool InputManager::isInputReleased(InputType input) const {
    return last_input_ == input && current_input_ != input;
}

uint32_t InputManager::getInputTimestamp() const {
    return input_timestamp_;
}

void InputManager::setInput(InputType input) {
    current_input_ = input;
    input_timestamp_ = to_ms_since_boot(get_absolute_time());
}

void InputManager::clearInput() {
    current_input_ = InputType::NONE;
    last_input_ = InputType::NONE;
}

} // namespace input 
