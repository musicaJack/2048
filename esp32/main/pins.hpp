#pragma once

#include <cstdint>

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32C6
// 孔位仍是 Pico 的。数值是 ESP32-C6-Pico 原理图上该孔实际接到的 GPIO。
// 屏幕六根在 TCA9554 上，DC 还和扩展芯片的 SDA 是同一只脚，不能送进 SPI。
static constexpr int PIN_JOY_SDA = 14;    // Pico GP6
static constexpr int PIN_JOY_SCL = 15;    // Pico GP7，启动脚，平时为高
static constexpr int PIN_PAGE_UP = 8;     // Pico GP8，启动脚，按下为低
static constexpr int PIN_PAGE_DOWN = 9;   // Pico GP9，启动脚
static constexpr int PIN_SCREEN = -1;     // Pico GP14，在 EXIO6 上
static constexpr int PIN_LCD_RST = -1;    // Pico GP15 = EXIO7
static constexpr int PIN_LCD_BL = -1;     // Pico GP16 = EXIO5
static constexpr int PIN_LCD_CS = -1;     // Pico GP17 = EXIO4
static constexpr int PIN_LCD_SCK = -1;    // Pico GP18 = EXIO3
static constexpr int PIN_LCD_MOSI = -1;   // Pico GP19 = EXIO2
static constexpr int PIN_LCD_DC = -1;     // Pico GP20 = GPIO22，扩展芯片 SDA

static constexpr int PIN_EXP_SDA = 22;
static constexpr int PIN_EXP_SCL = 23;
static constexpr int PIN_EXP_SCREEN_BIT = 5;  // EXIO6 = P5
#else
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

static constexpr int PIN_EXP_SDA = -1;
static constexpr int PIN_EXP_SCL = -1;
static constexpr int PIN_EXP_SCREEN_BIT = -1;
#endif

static constexpr uint8_t JOYSTICK_ADDR = 0x63;
static constexpr int I2C_HZ = 100000;
static constexpr int SPI_HZ_BRINGUP = 20 * 1000 * 1000;

static constexpr int JOYSTICK_DEADZONE = 1000;
static constexpr int JOYSTICK_THRESHOLD = 1800;
static constexpr int BUTTON_DEBOUNCE_MS = 50;
static constexpr int BUTTON_LONG_PRESS_MS = 600;
