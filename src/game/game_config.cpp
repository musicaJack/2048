#include "game/game_config.hpp"
#include "config/display_config_compile_time.hpp"

namespace game2048 {

// ST7306 4级灰度配色方案
ColorScheme ColorSchemeManager::getST7306ColorScheme() {
    static const Color tile_colors[] = {
        Color(255, 255, 255), // 2: 白色背景 (灰度0 - 最亮)
        Color(170, 170, 170), // 4: 浅灰背景 (灰度1)
        Color(85, 85, 85),    // 8: 中灰背景 (灰度2)
        Color(0, 0, 0),       // 16: 黑色背景 (灰度3 - 最暗)
        Color(0, 0, 0),       // 32: 黑色背景
        Color(0, 0, 0),       // 64: 黑色背景
        Color(0, 0, 0),       // 128: 黑色背景
        Color(0, 0, 0),       // 256: 黑色背景
        Color(0, 0, 0),       // 512: 黑色背景
        Color(0, 0, 0),       // 1024: 黑色背景
        Color(0, 0, 0),       // 2048: 黑色背景
        Color(0, 0, 0)        // 4096+: 黑色背景
    };
    
    return ColorScheme{
        Color(255, 255, 255),  // 白色背景（灰度0 - 最亮）
        Color(170, 170, 170),  // 棋盘背景（灰度1）
        Color(85, 85, 85),     // 网格线（灰度2）
        Color(0, 0, 0),        // 外框线（灰度3 - 最暗）
        Color(255, 255, 255),  // 文字颜色（白色）
        Color(0, 0, 0),        // 状态栏背景（黑色）
        Color(85, 85, 85),     // 方块边框（灰度2）
        tile_colors,
        sizeof(tile_colors) / sizeof(tile_colors[0])
    };
}

// ILI9488 深色主题配色方案
ColorScheme ColorSchemeManager::getILI9488ColorScheme() {
    static const Color tile_colors[] = {
        Color(238, 228, 218), // 2: 浅米色背景
        Color(237, 224, 200), // 4: 浅橙色背景
        Color(242, 177, 121), // 8: 橙色背景
        Color(245, 149, 99),  // 16: 深橙色背景
        Color(246, 124, 95),  // 32: 红橙色背景
        Color(246, 94, 59),   // 64: 红色背景
        Color(237, 207, 114), // 128: 黄色背景
        Color(237, 204, 97),  // 256: 金黄色背景
        Color(237, 200, 80),  // 512: 深金黄色背景
        Color(237, 197, 63),  // 1024: 更深的金黄色背景
        Color(237, 194, 46),  // 2048: 最深的金黄色背景
        Color(60, 58, 50)     // 4096+: 深灰色背景
    };
    
    return ColorScheme{
        Color(0, 0, 0),        // 纯黑色背景
        Color(40, 40, 40),     // 深灰色棋盘背景
        Color(60, 60, 60),     // 中灰色网格线
        Color(80, 80, 80),     // 浅灰色外框线
        Color(255, 255, 255),  // 白色文字
        Color(20, 30, 50),     // 蓝黑色状态栏背景
        Color(100, 100, 100),  // 浅灰色方块边框
        tile_colors,
        sizeof(tile_colors) / sizeof(tile_colors[0])
    };
}

// 根据显示驱动获取对应的颜色方案
ColorScheme ColorSchemeManager::getCurrentColorScheme() {
#ifdef DISPLAY_DRIVER_ST7306
    return getST7306ColorScheme();
#else
    return getILI9488ColorScheme();
#endif
}

// 根据背景色选择文字颜色，确保对比度
Color ColorSchemeManager::getTextColorForBackground(const Color& bgColor, bool isST7306) {
    if (isST7306) {
        // ST7306版本：简单的灰度判断
        uint8_t brightness = (bgColor.r * 299 + bgColor.g * 587 + bgColor.b * 114) / 1000;
        if (brightness > 128) {
            return Color(0, 0, 0);      // 浅色背景用黑色文字
        } else {
            return Color(255, 255, 255); // 深色背景用白色文字
        }
    } else {
        // ILI9488深色主题版本：根据背景亮度选择文字颜色
        uint8_t brightness = (bgColor.r * 299 + bgColor.g * 587 + bgColor.b * 114) / 1000;
        if (brightness > 180) {
            return Color(119, 110, 101); // 浅色背景用深色文字
        } else {
            return Color(255, 255, 255); // 深色背景用白色文字
        }
    }
}

} // namespace game2048
