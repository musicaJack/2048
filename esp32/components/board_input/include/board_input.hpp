#pragma once

#include <cstdint>
#include "esp_err.h"

enum class MoveDir { None, Left, Right, Up, Down };

struct InputPins {
    int sda;
    int scl;
    int page_up;
    int page_down;
    int screen;
    uint8_t addr;
    int hz;
};

class BoardInput {
public:
    esp_err_t begin(const InputPins& pins);

    bool joystickReady() const { return joy_ready_; }
    uint8_t firmwareVersion() const { return fw_; }

    // 摇杆偏移量，中心约为 0。读失败时两个值都是 0。
    void readStick(int16_t& x, int16_t& y);
    bool stickPressed();

    // 按下为 true。page_up / page_down 在释放时给出一次短按。
    // screen_long 在按住超过 long_ms 时给出一次。
    void poll(int debounce_ms, int long_ms, bool& up_click, bool& down_click, bool& screen_long);

private:
    struct Key {
        int pin = -1;
        bool raw = false;
        bool stable = false;
        int64_t changed_us = 0;
        bool saw_press = false;
        bool long_sent = false;
    };

    static void pollKey(Key& key, int debounce_ms, int long_ms, bool click_on_press,
                        bool& click, bool& long_press);

    bool joy_ready_ = false;
    uint8_t fw_ = 0;
    void* dev_ = nullptr;
    Key up_;
    Key down_;
    Key screen_;
};
