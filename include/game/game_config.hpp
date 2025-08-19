#pragma once

#include "display/display_driver.hpp"
#include <cstdint>

namespace game2048 {

// 游戏基础配置
struct GameConfig {
    static constexpr uint32_t GAME_UPDATE_DELAY_MS = 50;
    static constexpr float JOYSTICK_DIRECTION_RATIO = 0.7f;
    static constexpr uint32_t INPUT_DEBOUNCE_MS = 200;
    
    // 棋盘布局参数 - 自适应不同分辨率
    static constexpr uint16_t DEFAULT_BOARD_SIZE_PIXELS = 300;
    static constexpr uint16_t DEFAULT_CELL_SIZE = 75;
    static constexpr uint16_t DEFAULT_BOARD_START_X = 10;
    static constexpr uint16_t DEFAULT_BOARD_START_Y = 50;
};

// 颜色方案配置
struct ColorScheme {
    Color background;
    Color board_bg;
    Color grid;
    Color border;
    Color text;
    Color score_bg;
    Color tile_border;
    const Color* tile_colors;
    size_t tile_colors_count;
};

// 颜色方案管理器
class ColorSchemeManager {
public:
    static ColorScheme getST7306ColorScheme();
    static ColorScheme getILI9488ColorScheme();
    
    // 根据显示驱动获取对应的颜色方案
    static ColorScheme getCurrentColorScheme();
    
    // 根据背景色选择文字颜色，确保对比度
    static Color getTextColorForBackground(const Color& bgColor, bool isST7306 = false);
};

} // namespace game2048 
