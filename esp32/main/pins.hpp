#pragma once

#include <cstdint>

// 孔位是 Pico 的，数值是 ESP32-S3 的 GPIO。
static constexpr int PIN_JOY_SDA = 17;    // Pico GP6
static constexpr int PIN_JOY_SCL = 18;    // Pico GP7
static constexpr int PIN_PAGE_UP = 33;    // Pico GP8，按下为低
static constexpr int PIN_PAGE_DOWN = 34;  // Pico GP9
static constexpr int PIN_SCREEN = 39;     // Pico GP14
static constexpr int PIN_LCD_RST = 40;    // Pico GP15
static constexpr int PIN_LCD_BL = 42;     // Pico GP16
static constexpr int PIN_LCD_CS = 41;     // Pico GP17
static constexpr int PIN_LCD_SCK = 1;     // Pico GP18
static constexpr int PIN_LCD_MOSI = 2;    // Pico GP19
static constexpr int PIN_LCD_DC = 4;      // Pico GP20

static constexpr uint8_t JOYSTICK_ADDR = 0x63;
static constexpr int I2C_HZ = 100000;
static constexpr int SPI_HZ_BRINGUP = 10 * 1000 * 1000;

static constexpr int JOYSTICK_DEADZONE = 1000;
static constexpr int JOYSTICK_THRESHOLD = 1800;
static constexpr int BUTTON_DEBOUNCE_MS = 50;
static constexpr int BUTTON_LONG_PRESS_MS = 600;
