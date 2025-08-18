 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// 包含硬件驱动头文件
#include "hardware/display/ili9488_driver.hpp"
#include "hardware/display/ili9488_colors.hpp"
#include "hardware/input/joystick.hpp"
#include "hardware/system/pin_config.hpp"

using namespace ili9488;
using namespace ili9488_colors;

// 测试配置
#define JOYSTICK_TEST_DELAY_MS 50  // 降低更新频率，减少闪烁

// 全局变量
ILI9488Driver* g_display = nullptr;
Joystick* g_joystick = nullptr;

// 显示区域定义
#define STATUS_AREA_Y 0
#define STATUS_AREA_HEIGHT 40
#define JOYSTICK_AREA_Y 50
#define JOYSTICK_AREA_HEIGHT 200
#define INFO_AREA_Y 260
#define INFO_AREA_HEIGHT 200

// 上一次显示的数据，用于局部刷新
struct LastDisplayData {
    int16_t last_x = 0;
    int16_t last_y = 0;
    bool last_button = false;
    uint32_t last_update_time = 0;
} g_last_data;

// 函数声明
void drawJoystickVisualization(int16_t x, int16_t y, bool button);
void drawStatusBar(const char* status);
void drawInfoPanel(int16_t x, int16_t y, bool button);
void clearArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void initJoystickBackground();

int main() {
    // 初始化标准库
    stdio_init_all();
    
    // 初始化板载LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    
    // 等待串口稳定
    sleep_ms(2000);
    
    printf("\n");
    printf("========================================\n");
    printf("=== 2048游戏摇杆测试程序 ===\n");
    printf("========================================\n");
    printf("硬件配置:\n");
    printf("- 显示屏: ILI9488 (320x480)\n");
    printf("- 摇杆: I2C Joystick (地址: 0x%02X)\n", JOYSTICK_I2C_ADDR);
    printf("- 平台: Raspberry Pi Pico (RP2040)\n");
    printf("========================================\n\n");
    
    // 测试阶段1: 显示屏初始化
    printf("阶段1: 显示屏初始化...\n");
    
    // 创建显示屏实例
    g_display = new ili9488::ILI9488Driver(
        ILI9488_SPI_INST,                // SPI实例
        ILI9488_PIN_DC,                  // DC引脚
        ILI9488_PIN_RST,                 // RST引脚
        ILI9488_PIN_CS,                  // CS引脚
        ILI9488_PIN_SCK,                 // SCK引脚
        ILI9488_PIN_MOSI,                // MOSI引脚
        ILI9488_PIN_BL,                  // 背光引脚
        ILI9488_SPI_SPEED_HZ             // SPI速度
    );
    
    // 初始化显示屏
    if (!g_display->initialize()) {
        printf("错误: 显示屏初始化失败!\n");
        return -1;
    }
    
    printf("显示屏初始化成功!\n");
    
    // 设置显示屏参数
    g_display->setRotation(ili9488::Rotation::Portrait_0);
    g_display->setBacklight(true);
    g_display->setBacklightBrightness(255);
    
    // 清屏并绘制初始界面
    g_display->fillScreenRGB666(rgb666::BLACK);
    drawStatusBar("Joystick Test Ready");
    printf("显示屏配置完成\n");
    
    // 测试阶段2: 摇杆初始化
    printf("阶段2: 摇杆初始化...\n");
    
    // 创建摇杆实例
    g_joystick = new Joystick();
    
    // 初始化摇杆
    if (!g_joystick->begin(
        JOYSTICK_I2C_INST,               // I2C实例
        JOYSTICK_I2C_ADDR,               // 设备地址
        JOYSTICK_PIN_SDA,                // SDA引脚
        JOYSTICK_PIN_SCL,                // SCL引脚
        JOYSTICK_I2C_SPEED               // I2C速度
    )) {
        printf("错误: 摇杆初始化失败!\n");
        return -1;
    }
    
    printf("摇杆初始化成功!\n");
    
    // 获取摇杆信息
    uint8_t firmware_version = g_joystick->get_firmware_version();
    uint8_t bootloader_version = g_joystick->get_bootloader_version();
    uint8_t i2c_address = g_joystick->get_i2c_address();
    
    printf("摇杆信息:\n");
    printf("- 固件版本: %d\n", firmware_version);
    printf("- 引导程序版本: %d\n", bootloader_version);
    printf("- I2C地址: 0x%02X\n", i2c_address);
    
    // 设置摇杆LED颜色为绿色
    g_joystick->set_rgb_color(JOYSTICK_LED_GREEN);
    
    printf("\n=== 硬件初始化完成 ===\n");
    printf("开始摇杆测试...\n");
    printf("按任意键退出\n\n");
    
    // 绘制初始界面
    drawStatusBar("Joystick Test Running");
    initJoystickBackground();  // 初始化摇杆背景
    drawInfoPanel(0, 0, false);
    
    // 主测试循环
    while (true) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        // 读取摇杆数据
        int16_t x = g_joystick->get_joy_adc_8bits_offset_value_x();
        int16_t y = g_joystick->get_joy_adc_8bits_offset_value_y();
        bool button = (g_joystick->get_button_value() == 0);  // 0表示按下
        
        // 检查数据是否有变化，避免不必要的刷新
        if (x != g_last_data.last_x || y != g_last_data.last_y || 
            button != g_last_data.last_button || 
            (current_time - g_last_data.last_update_time) > 1000) {
            
            // 更新摇杆可视化（局部刷新）
            drawJoystickVisualization(x, y, button);
            
            // 更新信息面板（局部刷新）
            drawInfoPanel(x, y, button);
            
            // 根据摇杆位置设置LED颜色
            if (abs(x) > 50 || abs(y) > 50) {
                g_joystick->set_rgb_color(JOYSTICK_LED_BLUE);
            } else if (button) {
                g_joystick->set_rgb_color(JOYSTICK_LED_RED);
            } else {
                g_joystick->set_rgb_color(JOYSTICK_LED_GREEN);
            }
            
            // 更新上一次的数据
            g_last_data.last_x = x;
            g_last_data.last_y = y;
            g_last_data.last_button = button;
            g_last_data.last_update_time = current_time;
            
            // 每2秒打印一次调试信息
            static uint32_t last_debug = 0;
            if (current_time - last_debug > 2000) {
                last_debug = current_time;
                printf("Joystick - X: %d, Y: %d, Button: %s\n", 
                       x, y, button ? "PRESSED" : "RELEASED");
            }
        }
        
        // 板载LED闪烁指示程序运行
        gpio_put(PICO_DEFAULT_LED_PIN, (current_time / 1000) % 2);
        
        sleep_ms(JOYSTICK_TEST_DELAY_MS);
    }
    
    return 0;
}

void drawJoystickVisualization(int16_t x, int16_t y, bool button) {
    static int16_t last_joy_x = 160;
    static int16_t last_joy_y = 160;
    static bool last_button_state = false;
    
    // 计算摇杆在屏幕上的位置
    uint16_t joy_x = 160 + (x * 100 / 128);
    uint16_t joy_y = 160 + (y * 100 / 128);
    
    // 限制在有效范围内
    if (joy_x < 100) joy_x = 100;
    if (joy_x > 220) joy_x = 220;
    if (joy_y < 100) joy_y = 100;
    if (joy_y > 220) joy_y = 220;
    
    // 只在摇杆位置或按钮状态变化时更新
    if (joy_x != last_joy_x || joy_y != last_joy_y || button != last_button_state) {
        // 清除上一次的摇杆位置点
        g_display->fillAreaRGB666(last_joy_x - 6, last_joy_y - 6, 
                                  last_joy_x + 6, last_joy_y + 6, rgb666::WHITE);
        
        // 绘制新的摇杆位置点
        g_display->fillAreaRGB666(joy_x - 6, joy_y - 6, joy_x + 6, joy_y + 6, 
                                  button ? rgb666::RED : rgb666::GREEN);
        
        // 更新上一次的位置
        last_joy_x = joy_x;
        last_joy_y = joy_y;
        last_button_state = button;
    }
}

void drawStatusBar(const char* status) {
    static char last_status[64] = "";
    
    // 只在状态变化时更新
    if (strcmp(status, last_status) != 0) {
        g_display->fillAreaRGB666(0, STATUS_AREA_Y, 319, STATUS_AREA_Y + STATUS_AREA_HEIGHT - 1, rgb666::BLUE);
        g_display->drawString(10, STATUS_AREA_Y + 10, status, rgb666::WHITE, rgb666::BLUE);
        strcpy(last_status, status);
    }
}

void drawInfoPanel(int16_t x, int16_t y, bool button) {
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    static bool last_button = false;
    
    // 只在数据变化时更新
    if (x != last_x || y != last_y || button != last_button) {
        char info_str[64];
        
        // 清除信息区域
        g_display->fillAreaRGB666(0, INFO_AREA_Y, 319, INFO_AREA_Y + INFO_AREA_HEIGHT - 1, rgb666::BLACK);
        
        // X轴信息
        snprintf(info_str, sizeof(info_str), "X Axis: %d", x);
        g_display->drawString(10, INFO_AREA_Y + 10, info_str, rgb666::WHITE, rgb666::BLACK);
        
        // Y轴信息
        snprintf(info_str, sizeof(info_str), "Y Axis: %d", y);
        g_display->drawString(10, INFO_AREA_Y + 40, info_str, rgb666::WHITE, rgb666::BLACK);
        
        // 按钮状态
        snprintf(info_str, sizeof(info_str), "Button: %s", button ? "PRESSED" : "RELEASED");
        g_display->drawString(10, INFO_AREA_Y + 70, info_str, button ? rgb666::RED : rgb666::GREEN, rgb666::BLACK);
        
        // 方向判断
        const char* direction = "CENTER";
        if (abs(x) > abs(y)) {
            direction = (x > 0) ? "RIGHT" : "LEFT";
        } else if (abs(y) > 0) {
            direction = (y > 0) ? "DOWN" : "UP";
        }
        
        snprintf(info_str, sizeof(info_str), "Direction: %s", direction);
        g_display->drawString(10, INFO_AREA_Y + 100, info_str, rgb666::YELLOW, rgb666::BLACK);
        
        // 更新上一次的数据
        last_x = x;
        last_y = y;
        last_button = button;
    }
}

void clearArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    g_display->fillAreaRGB666(x, y, x + width - 1, y + height - 1, rgb666::BLACK);
}

void initJoystickBackground() {
    // 在显示屏上绘制一个大的白色圆圈作为摇杆的背景
    g_display->fillAreaRGB666(100, 100, 220, 220, rgb666::WHITE);
}
