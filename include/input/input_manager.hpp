#pragma once

#include <cstdint>

namespace input {

// 输入类型枚举
enum class InputType {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    BUTTON,
    RESET
};

/**
 * @brief 输入管理器类
 * 
 * 负责管理所有输入设备的状态和事件
 */
class InputManager {
public:
    InputManager();
    ~InputManager();

    // 禁用拷贝构造和赋值
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // 初始化和更新
    bool initialize();
    void update();

    // 输入状态查询
    InputType getCurrentInput() const;
    InputType getLastInput() const;
    bool isInputPressed(InputType input) const;
    bool isInputJustPressed(InputType input) const;
    bool isInputReleased(InputType input) const;

    // 时间戳
    uint32_t getInputTimestamp() const;

    // 手动设置输入（用于测试）
    void setInput(InputType input);
    void clearInput();

private:
    bool is_initialized_;
    InputType current_input_;
    InputType last_input_;
    uint32_t input_timestamp_;
};

} // namespace input 
