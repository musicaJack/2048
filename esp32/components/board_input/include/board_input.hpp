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
    // 屏幕键在扩展芯片上时，screen 填 -1，下面三只给出那条 I2C 和端口位。
    int exp_sda = -1;
    int exp_scl = -1;
    int exp_screen_bit = -1;
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

    static void pollKey(Key& key, bool raw, int debounce_ms, int long_ms, bool click_on_press,
                        bool& click, bool& long_press);

    bool screenPressed();
    bool expProbe(uint8_t addr);
    bool expWrite(uint8_t reg, uint8_t data);
    bool expRead(uint8_t reg, uint8_t& data);
    void expStart();
    void expStop();
    bool expWriteByte(uint8_t value);
    uint8_t expReadByte(bool ack);
    void expWriteBit(int bit);
    int expReadBit();

    bool joy_ready_ = false;
    uint8_t fw_ = 0;
    void* dev_ = nullptr;
    int exp_sda_ = -1;
    int exp_scl_ = -1;
    int exp_bit_ = -1;
    uint8_t exp_addr_ = 0;
    bool exp_ready_ = false;
    Key up_;
    Key down_;
    Key screen_;
};
