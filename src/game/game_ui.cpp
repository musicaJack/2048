#include "game/game_ui.hpp"
#include "game/game_animation.hpp" // 包含完整的头文件以使用成员函数
#include "display/adapters/st7306_adapter.hpp"
#include "display/adapters/ili9488_adapter.hpp"
#include "config/display_config_compile_time.hpp"
#include <cstring>
#include <cstdio>

namespace game2048 {

GameUIRenderer::GameUIRenderer(DisplayDriver* display)
    : display_(display), color_scheme_(ColorSchemeManager::getCurrentColorScheme()),
      layout_params_(), last_game_status_(), first_draw_(true), anim_manager_(nullptr) {
    resetCache();
}

void GameUIRenderer::calculateLayout() {
    uint16_t display_width = display_->getWidth();
    uint16_t display_height = display_->getHeight();
    
    // 计算最大可用棋盘尺寸
    uint16_t available_width = display_width - 40;  // 留20像素边距
    uint16_t available_height = display_height - 120; // 留出状态栏和提示栏空间
    
    // 选择较小的尺寸作为棋盘尺寸，确保正方形
    layout_params_.board_size_pixels = (available_width < available_height) ? available_width : available_height;
    
    // 确保棋盘尺寸能被4整除
    layout_params_.board_size_pixels = (layout_params_.board_size_pixels / 4) * 4;
    
    // 计算格子尺寸
    layout_params_.cell_size = layout_params_.board_size_pixels / 4;
    
    // 计算棋盘居中位置
    layout_params_.board_start_x = (display_width - layout_params_.board_size_pixels) / 2;
    layout_params_.board_start_y = 60; // 留出状态栏空间
}

void GameUIRenderer::resetCache() {
    memset(last_board_, 0, sizeof(BoardArray));
    last_score_ = 0;
    last_game_status_ = GameStatus::READY;
    first_draw_ = true;
}

int GameUIRenderer::getColorIndex(int value) const {
    int colorIndex = 0;
    int tempValue = value;
    while (tempValue > 2) {
        tempValue /= 2;
        colorIndex++;
    }
    if (colorIndex >= color_scheme_.tile_colors_count) {
        colorIndex = color_scheme_.tile_colors_count - 1;
    }
    return colorIndex;
}

uint8_t GameUIRenderer::getFontSize(int value) const {
    if (value >= 1000) {
        return 1; // 4位数用1倍字体 (1024, 2048, 4096) - 确保完整显示
    } else if (value >= 100) {
        return 1; // 3位数用1倍字体 (128, 256, 512) - 降低到1倍，确保一致性
    } else if (value >= 10) {
        return 2; // 2位数用2倍字体 (16, 32, 64) - 保持较大显示
    } else {
        return 2; // 1位数用2倍字体 (2, 4, 8) - 保持较大显示
    }
}

void GameUIRenderer::drawStringWithGame2048Font(uint16_t x, uint16_t y, const std::string& text, 
                                               const Color& color, const Color& bgColor, uint8_t size) {
#ifdef DISPLAY_DRIVER_ST7306
    if (auto* st7306_adapter = static_cast<ST7306Adapter*>(display_)) {
        st7306_adapter->drawStringGame2048(x, y, text, color, bgColor, size);
    } else {
        display_->drawString(x, y, text, color, bgColor, size);
    }
#elif defined(DISPLAY_DRIVER_ILI9488)
    if (auto* ili9488_adapter = static_cast<ILI9488Adapter*>(display_)) {
        ili9488_adapter->drawStringGame2048(x, y, text, color, bgColor, size);
    } else {
        display_->drawString(x, y, text, color, bgColor, size);
    }
#else
    display_->drawString(x, y, text, color, bgColor, size);
#endif
}

void GameUIRenderer::drawTileBackground(uint16_t x, uint16_t y, int value) {
    if (value == 0) {
        // 空白格子
        display_->fillRect(x, y, layout_params_.cell_size - 2, layout_params_.cell_size - 2, color_scheme_.board_bg);
        return;
    }
    
    // 计算颜色索引
    int colorIndex = getColorIndex(value);
    
    // 绘制方块背景
    display_->fillRect(x, y, layout_params_.cell_size - 2, layout_params_.cell_size - 2, 
                      color_scheme_.tile_colors[colorIndex]);
    
    // 为浅色背景的数字添加边框，增强视觉边界感
#ifdef DISPLAY_DRIVER_ST7306
    if (colorIndex <= 1) { // 数字2、4（白色和浅灰背景）
        display_->drawRect(x, y, layout_params_.cell_size - 2, layout_params_.cell_size - 2, color_scheme_.tile_border);
    }
#else
    if (colorIndex <= 1) { // 数字2、4（白色和浅灰背景）
        display_->drawRect(x, y, layout_params_.cell_size - 2, layout_params_.cell_size - 2, color_scheme_.tile_border);
    }
#endif
}

void GameUIRenderer::drawTileText(uint16_t x, uint16_t y, int value) {
    if (value == 0) {
        return; // 空白格子不需要绘制文字
    }
    
    // 计算颜色索引
    int colorIndex = getColorIndex(value);
    
    // 绘制数字
    char numStr[8];
    snprintf(numStr, sizeof(numStr), "%d", value);
    
    // 根据数字大小调整字体大小
    uint8_t fontSize = getFontSize(value);
    
    // 计算文字位置（居中）- 使用Game2048字体（12x16）
    uint16_t textWidth = strlen(numStr) * 12 * fontSize; // Game2048字体宽度
    uint16_t textX = x + (layout_params_.cell_size - textWidth) / 2;
    uint16_t textY = y + (layout_params_.cell_size - 16 * fontSize) / 2; // Game2048字体高度
    
    // 根据背景色选择文字颜色，确保对比度
    const Color& bgColor = color_scheme_.tile_colors[colorIndex];
    Color textColor = ColorSchemeManager::getTextColorForBackground(bgColor, 
#ifdef DISPLAY_DRIVER_ST7306
        true
#else
        false
#endif
    );
    
    // 使用Game2048优化字体绘制
    drawStringWithGame2048Font(textX, textY, numStr, textColor, bgColor, fontSize);
}

void GameUIRenderer::drawBoardOptimized(const GameState& game_state) {
    const BoardArray& board = game_state.getBoard();
    
    // 如果是首次绘制，绘制完整棋盘
    if (first_draw_) {
        // 绘制棋盘背景
        display_->fillRect(layout_params_.board_start_x - 10, layout_params_.board_start_y - 10, 
                          layout_params_.board_size_pixels + 20, layout_params_.board_size_pixels + 20, 
                          color_scheme_.board_bg);
        
        // 绘制外框线
        // 上边框
        display_->fillRect(layout_params_.board_start_x - 10, layout_params_.board_start_y - 10, 
                          layout_params_.board_size_pixels + 20, 2, color_scheme_.border);
        // 下边框
        display_->fillRect(layout_params_.board_start_x - 10, layout_params_.board_start_y + layout_params_.board_size_pixels + 8, 
                          layout_params_.board_size_pixels + 20, 2, color_scheme_.border);
        // 左边框
        display_->fillRect(layout_params_.board_start_x - 10, layout_params_.board_start_y - 10, 
                          2, layout_params_.board_size_pixels + 20, color_scheme_.border);
        // 右边框
        display_->fillRect(layout_params_.board_start_x + layout_params_.board_size_pixels + 8, layout_params_.board_start_y - 10, 
                          2, layout_params_.board_size_pixels + 20, color_scheme_.border);
        
        // 第一步：绘制所有方块背景
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                uint16_t x = layout_params_.board_start_x + j * layout_params_.cell_size + 1;
                uint16_t y = layout_params_.board_start_y + i * layout_params_.cell_size + 1;
                drawTileBackground(x, y, board[i][j]);
            }
        }
        
        // 第二步：绘制所有方块文字
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                uint16_t x = layout_params_.board_start_x + j * layout_params_.cell_size + 1;
                uint16_t y = layout_params_.board_start_y + i * layout_params_.cell_size + 1;
                drawTileText(x, y, board[i][j]);
            }
        }
        
        // 复制当前状态到缓存
        memcpy(last_board_, board, sizeof(BoardArray));
        first_draw_ = false;
        return;
    }
    
    // 局部刷新：只更新变化的方块
    // 第一步：绘制变化的方块背景
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (last_board_[i][j] != board[i][j]) {
                uint16_t x = layout_params_.board_start_x + j * layout_params_.cell_size + 1;
                uint16_t y = layout_params_.board_start_y + i * layout_params_.cell_size + 1;
                drawTileBackground(x, y, board[i][j]);
            }
        }
    }
    
    // 第二步：绘制变化的方块文字
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (last_board_[i][j] != board[i][j]) {
                uint16_t x = layout_params_.board_start_x + j * layout_params_.cell_size + 1;
                uint16_t y = layout_params_.board_start_y + i * layout_params_.cell_size + 1;
                drawTileText(x, y, board[i][j]);
            }
        }
    }
    
    // 更新缓存
    memcpy(last_board_, board, sizeof(BoardArray));
}

void GameUIRenderer::drawStatusBarOptimized(const GameState& game_state) {
    uint32_t current_score = game_state.getCurrentScore();
    GameStatus current_game_status = game_state.getGameStatus();
    
    // 检查是否需要更新状态栏
    bool need_update = first_draw_ || 
                      current_score != last_score_ ||
                      current_game_status != last_game_status_;
    
    if (!need_update) {
        return;
    }
    
    // 重绘状态栏背景
    display_->fillRect(0, 0, display_->getWidth(), 40, color_scheme_.score_bg);
    
    // 分数信息 - 只显示当前分数
    char scoreStr[64];
    snprintf(scoreStr, sizeof(scoreStr), "Score: %lu", current_score);
    display_->drawString(10, 12, scoreStr, color_scheme_.text, 1);
    
    // 在状态栏右侧显示星星数量（优先显示）
    if (anim_manager_) {
        uint32_t star_count = anim_manager_->getTotalStars();
        drawStarsInStatusBar(star_count);
    }
    
    // 游戏状态（在星星左侧显示）
    const char* statusStr = "Playing";
    switch (current_game_status) {
        case GameStatus::WON: statusStr = "WON! 🎉"; break;
        case GameStatus::GAME_OVER: statusStr = "Game Over"; break;
        default: break;
    }
    
    // 计算状态文字位置，确保不与星星重叠
    uint16_t statusX = display_->getWidth() - strlen(statusStr) * 12 - 90; // 在星星左侧留出空间
    display_->drawString(statusX, 12, statusStr, color_scheme_.text, 1);
    
    // 更新缓存
    last_score_ = current_score;
    last_game_status_ = current_game_status;
}

void GameUIRenderer::drawControlHint() {
    // 底部控制提示背景
    display_->fillRect(0, display_->getHeight() - 30, display_->getWidth(), 30, color_scheme_.score_bg);
    display_->drawString(10, display_->getHeight() - 25, "Joystick: Move | Button: Reset", color_scheme_.text, 1);
}

void GameUIRenderer::drawStarCount(uint16_t x, uint16_t y, uint32_t star_count) {
    if (!display_) return;
    
    char count_str[16];
    snprintf(count_str, sizeof(count_str), "%lu", star_count);
    
    // 绘制星星图标
    drawStar(x, y, 8, Color(255, 215, 0)); // 金色星星
    
    // 绘制数量文字
    display_->drawString(x + 12, y - 4, count_str, Color(255, 255, 255), 1);
}

void GameUIRenderer::drawStarsInStatusBar(uint32_t star_count) {
    if (!display_) return;
    
    // 在状态栏最右侧显示星星
    uint16_t star_x = display_->getWidth() - 50; // 最右侧位置
    uint16_t star_y = 15; // 状态栏垂直居中
    
    drawStarCount(star_x, star_y, star_count);
}

void GameUIRenderer::drawStar(int16_t x, int16_t y, uint8_t size, const Color& color) {
    if (!display_) return;
    
    // 简单的五角星绘制（使用5个点）
    int16_t points[5][2] = {
        {x, static_cast<int16_t>(y - size)},           // 顶部
        {static_cast<int16_t>(x + size/2), static_cast<int16_t>(y - size/3)}, // 右上
        {static_cast<int16_t>(x + size/3), static_cast<int16_t>(y + size/2)}, // 右下
        {static_cast<int16_t>(x - size/3), static_cast<int16_t>(y + size/2)}, // 左下
        {static_cast<int16_t>(x - size/2), static_cast<int16_t>(y - size/3)}  // 左上
    };
    
    // 绘制五角星轮廓
    for (int i = 0; i < 5; ++i) {
        int next = (i + 2) % 5; // 跳过相邻点，形成五角星
        display_->drawLine(points[i][0], points[i][1], points[next][0], points[next][1], color);
    }
    
    // 填充星星中心
    display_->fillRect(x - 1, y - 1, 3, 3, color);
}

} // namespace game2048
