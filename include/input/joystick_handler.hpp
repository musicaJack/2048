#pragma once

#include <cstdint>
#include <string>

// 前向声明
class Joystick;

namespace input {

// 摇杆方向枚举
enum class JoystickDirection {
    CENTER,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

/**
 * @brief 摇杆处理器类
 * 
 * 负责处理摇杆输入，包括方向检测、死区处理等
 */
class JoystickHandler {
public:
    JoystickHandler();
    ~JoystickHandler();

    // 禁用拷贝构造和赋值
    JoystickHandler(const JoystickHandler&) = delete;
    JoystickHandler& operator=(const JoystickHandler&) = delete;

    // 初始化和更新
    bool initialize(Joystick* joystick);
    void update();

    // 方向处理
    JoystickDirection processDirection(int16_t x, int16_t y);
    JoystickDirection getCurrentDirection() const;

    // 按钮状态
    bool isButtonPressed() const;
    bool isButtonJustPressed() const;
    bool isButtonJustReleased() const;

    // 配置
    void setDeadzone(int16_t deadzone);
    void setDirectionRatio(float ratio);
    int16_t getDeadzone() const;
    float getDirectionRatio() const;

    // 工具方法
    std::string getDirectionString(JoystickDirection direction);

private:
    bool is_initialized_;
    Joystick* joystick_;
    int16_t deadzone_;
    float direction_ratio_;
    JoystickDirection last_direction_;
    bool last_button_state_;
};

} // namespace input 
