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

// 硬件引脚配置 - 已移除硬编码，现在完全依赖配置文件
namespace display_config {
    // 注意：所有引脚配置现在都从配置文件中读取
    // 不再使用硬编码的默认值
}
