#pragma once

/**
 * @file pin_config.hpp
 * @brief 统一的引脚配置管理 - ILI9488项目
 * @description 这个文件包含了所有硬件引脚的定义，包括SPI屏幕、joystick和microsd的引脚配置
 */

#include <string>
#include <cstdint>
#include "pico/stdlib.h"

// 前向声明，避免头文件循环依赖
typedef struct spi_inst spi_inst_t;
typedef struct i2c_inst i2c_inst_t;

// =============================================================================
// ILI9488 SPI 显示屏引脚配置
// =============================================================================

// =============================================================================
// 显示驱动配置 - 已移除硬编码，现在从配置文件读取
// =============================================================================

// 注意：所有显示驱动的引脚配置现在都从配置文件中读取
// 不再使用硬编码的引脚定义

// 背光控制方式
// 1 = 使用PWM（可调亮度，可能引入纹波）；0 = 直流常亮/开关（无PWM纹波）
#define ILI9488_BACKLIGHT_USE_PWM    1

// 背光亮度配置 (0-255) - 默认值，实际从配置文件读取
#define ILI9488_BACKLIGHT_BRIGHTNESS 128    // 50%亮度 (255 * 0.5 = 128)

// SPI 接口配置 - 默认值，实际从配置文件读取
#define ILI9488_SPI_INST        spi0        // SPI接口实例，使用SPI0
#define ILI9488_SPI_SPEED_HZ    40000000    // SPI速度40MHz

// =============================================================================
// 霍尔摇杆直接连接配置 (RP2040内置ADC)
// =============================================================================

// 霍尔摇杆引脚定义
#define HALL_JOYSTICK_PIN_X     26          // X轴ADC引脚 (GP26, ADC0)
#define HALL_JOYSTICK_PIN_Y     27          // Y轴ADC引脚 (GP27, ADC1)  
#define HALL_JOYSTICK_PIN_SW    22          // 开关引脚 (GP22)

// ADC配置
#define HALL_JOYSTICK_ADC_X_CHANNEL  0      // ADC通道0 (GP26)
#define HALL_JOYSTICK_ADC_Y_CHANNEL  1      // ADC通道1 (GP27)

// 霍尔摇杆操作参数配置
#define HALL_JOYSTICK_THRESHOLD      1800   // 操作检测阈值
#define HALL_JOYSTICK_LOOP_DELAY_MS  20     // 循环延迟时间（毫秒）
#define HALL_JOYSTICK_DEADZONE       1000   // 摇杆死区阈值

// LED配置
#define HALL_JOYSTICK_LED_PIN        25      // PICO主板LED引脚 (GP25)

// =============================================================================
// Joystick 手柄 I2C 配置 (保留用于ADS1115方案)
// =============================================================================

// I2C 接口配置
#define JOYSTICK_I2C_INST       i2c1        // I2C接口实例
#define JOYSTICK_I2C_ADDR       0x63        // I2C设备地址
#define JOYSTICK_I2C_SPEED      100000      // I2C速度（100kHz）

// I2C 信号引脚
#define JOYSTICK_PIN_SDA        6           // I2C数据引脚
#define JOYSTICK_PIN_SCL        7           // I2C时钟引脚

// Joystick 操作参数配置
#define JOYSTICK_THRESHOLD      1800        // 操作检测阈值
#define JOYSTICK_LOOP_DELAY_MS  20          // 循环延迟时间（毫秒）
#define JOYSTICK_DEADZONE       1000        // 摇杆死区阈值

// Joystick LED 颜色定义 (24位RGB格式)
#define JOYSTICK_LED_OFF        0x000000    // 黑色（关闭）
#define JOYSTICK_LED_RED        0xFF0000    // 红色
#define JOYSTICK_LED_GREEN      0x00FF00    // 绿色
#define JOYSTICK_LED_BLUE       0x0000FF    // 蓝色

// =============================================================================
// MicroSD 卡 SPI 配置
// =============================================================================

// MicroSD SPI 端口定义 - 使用nullptr，在运行时设置
#define SPI_PORT_MICROSD        nullptr     // MicroSD SPI端口，将在运行时设置为spi1

// MicroSD 引脚定义 (SPI1 默认引脚映射)
#define MICROSD_PIN_MISO        11          // MISO引脚 (GPIO11) - 主入从出
#define MICROSD_PIN_CS          13          // CS引脚 (GPIO13)   - 片选
#define MICROSD_PIN_SCK         10          // SCK引脚 (GPIO10)  - 时钟
#define MICROSD_PIN_MOSI        12          // MOSI引脚 (GPIO12) - 主出从入

// MicroSD 频率定义
#define MICROSD_SPI_FREQ_SLOW_DEFAULT    (400 * 1000)       // 慢速时钟频率 (400KHz用于初始化)
#define MICROSD_SPI_FREQ_FAST_DEFAULT    (40 * 1000 * 1000) // 快速时钟频率 (40MHz用于正常操作)
#define MICROSD_SPI_FREQ_SLOW_COMPAT     (200 * 1000)       // 兼容性慢速频率
#define MICROSD_SPI_FREQ_FAST_COMPAT     (20 * 1000 * 1000) // 兼容性快速频率
#define MICROSD_SPI_FREQ_FAST_HIGH       (50 * 1000 * 1000) // 高速频率

// MicroSD 配置标志
#define MICROSD_USE_INTERNAL_PULLUP     true    // 默认使用内部上拉电阻

// =============================================================================
// 兼容性宏定义（保持向后兼容）
// =============================================================================

// 显示驱动兼容性定义 - 已移除硬编码引用
// 现在这些定义不再使用，因为引脚配置从配置文件读取

// Joystick 兼容性定义（保持向后兼容）
#define JOYSTICK_I2C_PORT       JOYSTICK_I2C_INST
#define JOYSTICK_I2C_SDA_PIN    JOYSTICK_PIN_SDA
#define JOYSTICK_I2C_SCL_PIN    JOYSTICK_PIN_SCL
#define JOYSTICK_I2C_FREQUENCY  JOYSTICK_I2C_SPEED

// 向后兼容性定义
#ifndef JOYSTICK_ADDR
#define JOYSTICK_ADDR           JOYSTICK_I2C_ADDR
#endif
#define PIN_SDA                 JOYSTICK_PIN_SDA
#define PIN_SCL                 JOYSTICK_PIN_SCL
#define I2C_FREQUENCY           JOYSTICK_I2C_SPEED
#define JOY_DEADZONE            JOYSTICK_DEADZONE

// =============================================================================
// 硬件配置验证宏
// =============================================================================

// 编译时验证引脚配置的合理性 - 已移除显示驱动硬编码引用
// 现在这些检查在运行时通过配置文件进行

#if MICROSD_PIN_SCK == JOYSTICK_PIN_SDA || MICROSD_PIN_SCK == JOYSTICK_PIN_SCL
    #warning "MicroSD SCK pin conflicts with I2C pins"
#endif

#if MICROSD_PIN_MOSI == JOYSTICK_PIN_SDA || MICROSD_PIN_MOSI == JOYSTICK_PIN_SCL
    #warning "MicroSD MOSI pin conflicts with I2C pins"
#endif

// =============================================================================
// 辅助宏定义
// =============================================================================

// 获取完整的SPI配置 - 已移除硬编码引用
// 现在这些配置从配置文件中读取

// 获取完整的Joystick配置
#define JOYSTICK_GET_I2C_CONFIG() JOYSTICK_I2C_INST, JOYSTICK_I2C_ADDR, JOYSTICK_PIN_SDA, JOYSTICK_PIN_SCL, JOYSTICK_I2C_SPEED

// =============================================================================
// MicroSD 配置结构体
// =============================================================================

namespace MicroSD {

/**
 * @brief MicroSD 引脚配置结构体
 */
struct PinConfig {
    uint pin_miso = MICROSD_PIN_MISO;        // MISO引脚
    uint pin_cs = MICROSD_PIN_CS;            // CS引脚
    uint pin_sck = MICROSD_PIN_SCK;          // SCK引脚
    uint pin_mosi = MICROSD_PIN_MOSI;        // MOSI引脚
    bool use_internal_pullup = MICROSD_USE_INTERNAL_PULLUP;  // 使用内部上拉电阻
    
    // 引脚功能验证
    bool is_valid() const {
        return pin_miso <= 29 && pin_cs <= 29 && 
               pin_sck <= 29 && pin_mosi <= 29;
    }
    
    // 获取引脚描述
    std::string get_description() const {
        return "MISO:" + std::to_string(pin_miso) + 
               " CS:" + std::to_string(pin_cs) + 
               " SCK:" + std::to_string(pin_sck) + 
               " MOSI:" + std::to_string(pin_mosi);
    }
};

/**
 * @brief MicroSD SPI配置结构体
 */
struct SPIConfig {
    spi_inst_t* spi_port = SPI_PORT_MICROSD;          // SPI端口 (使用 SPI1)
    uint32_t clk_slow = MICROSD_SPI_FREQ_SLOW_DEFAULT;       // 慢速时钟频率
    uint32_t clk_fast = MICROSD_SPI_FREQ_FAST_DEFAULT;       // 快速时钟频率
    PinConfig pins;                       // 引脚配置
    
    // 构造函数，设置正确的SPI端口
    SPIConfig() {
        // 在运行时设置SPI端口
        #ifdef __PICO__
        extern spi_inst_t* spi1;
        spi_port = spi1;
        #endif
    }
    
    // 验证配置
    bool is_valid() const {
        return spi_port != nullptr && pins.is_valid();
    }
    
    // 获取配置描述
    std::string get_description() const {
        std::string spi_name = "SPI";
        if (spi_port != nullptr) {
            // 通过指针地址判断是哪个SPI实例
            spi_name += "1"; // 默认使用SPI1
        } else {
            spi_name += "?";
        }
        return spi_name +
               " Slow:" + std::to_string(clk_slow/1000) + "KHz" +
               " Fast:" + std::to_string(clk_fast/1000000) + "MHz" +
               " Pins:" + pins.get_description();
    }
};

/**
 * @brief MicroSD 预定义配置
 */
namespace Config {
    // 默认配置
    inline const SPIConfig DEFAULT = SPIConfig();
    
    // 高速配置
    inline const SPIConfig HIGH_SPEED = []() {
        SPIConfig config;
        config.clk_fast = MICROSD_SPI_FREQ_FAST_HIGH;
        return config;
    }();
    
    // 兼容性配置 (较低频率)
    inline const SPIConfig COMPATIBLE = []() {
        SPIConfig config;
        config.clk_slow = MICROSD_SPI_FREQ_SLOW_COMPAT;
        config.clk_fast = MICROSD_SPI_FREQ_FAST_COMPAT;
        return config;
    }();
}

} // namespace MicroSD 

// ================= SPI 通用配置结构体（合并自spi_config.hpp） =================

namespace spi_config {

struct SPIConfig {
    spi_inst_t* spi_inst;
    uint32_t clk_slow;
    uint32_t clk_fast;
    uint pin_miso;
    uint pin_cs;
    uint pin_sck;
    uint pin_mosi;
    bool pullup;
    SPIConfig() : spi_inst(nullptr), clk_slow(400000), clk_fast(40000000),
                  pin_miso(0), pin_cs(0), pin_sck(0), pin_mosi(0), pullup(true) {}
    bool is_valid() const {
        return spi_inst != nullptr && 
               pin_miso <= 29 && pin_cs <= 29 && 
               pin_sck <= 29 && pin_mosi <= 29;
    }
    std::string get_description() const {
        if (!spi_inst) return "Invalid SPI Config";
        std::string spi_name = "SPI";
        if (spi_inst != nullptr) {
            spi_name += "1";
        } else {
            spi_name += "?";
        }
        return spi_name +
               " Slow:" + std::to_string(clk_slow/1000) + "KHz" +
               " Fast:" + std::to_string(clk_fast/1000000) + "MHz" +
               " Pins:" + std::to_string(pin_miso) + "," + 
               std::to_string(pin_cs) + "," + 
               std::to_string(pin_sck) + "," + 
               std::to_string(pin_mosi);
    }
};

inline SPIConfig get_default_config() {
    SPIConfig config;
    config.spi_inst = nullptr;
    config.clk_slow = 400000;
    config.clk_fast = 40000000;
    config.pin_miso = 11;
    config.pin_cs = 13;
    config.pin_sck = 10;
    config.pin_mosi = 12;
    config.pullup = true;
    return config;
}

inline SPIConfig get_compat_config() {
    SPIConfig config;
    config.spi_inst = nullptr;
    config.clk_slow = 200000;
    config.clk_fast = 20000000;
    config.pin_miso = 11;
    config.pin_cs = 13;
    config.pin_sck = 10;
    config.pin_mosi = 12;
    config.pullup = true;
    return config;
}

} // namespace spi_config 