#pragma once

// 编译时显示驱动配置
// 这个文件定义了在编译时选择的显示驱动类型

#ifdef DISPLAY_DRIVER_ST7306
    #define CURRENT_DISPLAY_DRIVER "st7306"
    #define DISPLAY_WIDTH 300
    #define DISPLAY_HEIGHT 400
    #define DISPLAY_COLOR_DEPTH 4  // 4级灰度
    #define DISPLAY_IS_COLOR false
    #define DISPLAY_IS_GRAYSCALE true
#elif defined(DISPLAY_DRIVER_ILI9488)
    #define CURRENT_DISPLAY_DRIVER "ili9488"
    #define DISPLAY_WIDTH 320
    #define DISPLAY_HEIGHT 480
    #define DISPLAY_COLOR_DEPTH 65536  // RGB565
    #define DISPLAY_IS_COLOR true
    #define DISPLAY_IS_GRAYSCALE false
#else
    #error "未选择显示驱动！请在CMakeLists.txt中设置USE_ST7306_DISPLAY或USE_ILI9488_DISPLAY"
#endif

// 硬件引脚配置
namespace display_config {
    // 默认SPI引脚配置（可根据需要修改）
    constexpr int DEFAULT_DC_PIN = 20;
    constexpr int DEFAULT_RST_PIN = 15;
    constexpr int DEFAULT_CS_PIN = 17;
    constexpr int DEFAULT_SCLK_PIN = 18;
    constexpr int DEFAULT_MOSI_PIN = 19;
    constexpr int DEFAULT_BL_PIN = 16;  // 背光引脚 - 修复：使用GPIO 16
    constexpr int DEFAULT_SPI_PORT = 0;
    constexpr int DEFAULT_SPI_FREQUENCY = 40000000;  // 40MHz
}
