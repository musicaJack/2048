#pragma once

#include "display/display_driver.hpp"
#include "game/game_config.hpp"
#include "game/game_2048.hpp"
#include <cstdint>

namespace game2048 {

// 游戏状态
enum class LocalGameState {
    INIT,
    PLAYING,
    WON,
    GAME_OVER
};

// 布局参数
struct LayoutParams {
    uint16_t board_size_pixels;
    uint16_t cell_size;
    uint16_t board_start_x;
    uint16_t board_start_y;
};

// UI渲染器
class GameUIRenderer {
public:
    GameUIRenderer(DisplayDriver* display);
    
    // 布局计算
    void calculateLayout();
    const LayoutParams& getLayoutParams() const { return layout_params_; }
    
    // 绘制功能
    void drawTileBackground(uint16_t x, uint16_t y, int value);
    void drawTileText(uint16_t x, uint16_t y, int value);
    void drawBoardOptimized(const GameState& game_state);
    void drawStatusBarOptimized(const GameState& game_state);
    void drawControlHint();
    
    // 缓存管理
    void resetCache();
    void setFirstDraw(bool first_draw) { first_draw_ = first_draw; }
    
    // 颜色方案
    void setColorScheme(const ColorScheme& scheme) { color_scheme_ = scheme; }
    const ColorScheme& getColorScheme() const { return color_scheme_; }

private:
    DisplayDriver* display_;
    ColorScheme color_scheme_;
    LayoutParams layout_params_;
    
    // 缓存变量 - 用于局部刷新
    BoardArray last_board_;
    uint32_t last_score_;
    GameStatus last_game_status_;
    bool first_draw_;
    
    // 辅助函数
    int getColorIndex(int value) const;
    uint8_t getFontSize(int value) const;
    void drawStringWithGame2048Font(uint16_t x, uint16_t y, const std::string& text, 
                                   const Color& color, const Color& bgColor, uint8_t size);
};

} // namespace game2048
