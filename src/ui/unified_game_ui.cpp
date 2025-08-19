#include "ui/unified_game_ui.hpp"
#include <cstring>
#include <cstdio>

namespace game2048 {

UnifiedGameUI::UnifiedGameUI(DisplayDriver* display, input::JoystickHandler* joystick)
    : display_(display), joystick_(joystick), first_draw_(true) {
    resetCache();
}

bool UnifiedGameUI::init() {
    if (!display_ || !joystick_) {
        return false;
    }
    
    // 计算自适应布局
    calculateLayout();
    
    // 清屏并绘制初始界面
    clear();
    
    return true;
}

void UnifiedGameUI::calculateLayout() {
    uint16_t width = display_->getWidth();
    uint16_t height = display_->getHeight();
    
    // 根据屏幕尺寸计算布局
    layout_.status_bar_height = 40;
    layout_.hint_bar_height = 30;
    
    // 计算棋盘尺寸 - 取屏幕宽度和高度的较小值，留出边距
    uint16_t available_width = width - 20;  // 左右各留10像素边距
    uint16_t available_height = height - layout_.status_bar_height - layout_.hint_bar_height - 20; // 上下留边距
    
    // 棋盘尺寸取可用空间的最小值
    layout_.board_size_pixels = (available_width < available_height) ? available_width : available_height;
    
    // 确保棋盘尺寸是4的倍数（4x4网格）
    layout_.board_size_pixels = (layout_.board_size_pixels / 4) * 4;
    
    // 计算单个格子尺寸
    layout_.cell_size = layout_.board_size_pixels / 4;
    
    // 计算棋盘起始位置（居中）
    layout_.board_start_x = (width - layout_.board_size_pixels) / 2;
    layout_.board_start_y = layout_.status_bar_height + 10;
    
    // 根据屏幕尺寸调整网格线粗细
    if (width < 320) {
        layout_.grid_thickness = 1;
        layout_.border_thickness = 2;
    } else {
        layout_.grid_thickness = 2;
        layout_.border_thickness = 3;
    }
}

void UnifiedGameUI::drawGame(const Game2048& game) {
    const GameState& game_state = game.getGameState();
    const BoardArray& board = game_state.getBoard();
    
    // 检查是否需要完整重绘
    bool need_full_redraw = first_draw_;
    
    if (!need_full_redraw) {
        // 检查状态栏是否需要更新
        need_full_redraw = (last_score_ != game_state.getCurrentScore() ||
                           last_game_status_ != game_state.getGameStatus());
    }
    
    if (need_full_redraw) {
        // 完整重绘
        clear();
        drawStatusBar(game_state);
        drawBoard(board);
        drawHintBar();
        
        // 更新缓存
        memcpy(last_board_, board, sizeof(BoardArray));
        last_score_ = game_state.getCurrentScore();
        last_game_status_ = game_state.getGameStatus();
        first_draw_ = false;
    } else {
        // 局部刷新 - 只更新变化的方块
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (last_board_[i][j] != board[i][j]) {
                    uint16_t x = layout_.board_start_x + j * layout_.cell_size + layout_.grid_thickness;
                    uint16_t y = layout_.board_start_y + i * layout_.cell_size + layout_.grid_thickness;
                    drawTile(x, y, board[i][j]);
                }
            }
        }
        
        // 更新缓存
        memcpy(last_board_, board, sizeof(BoardArray));
    }
    
    // 刷新显示
    refresh();
}

void UnifiedGameUI::drawStatusBar(const GameState& game_state) {
    uint16_t width = display_->getWidth();
    
    // 绘制状态栏背景
    display_->fillRect(0, 0, width, layout_.status_bar_height, theme_.score_bg);
    
    // 绘制分数
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "Score: %lu", game_state.getCurrentScore());
    drawText(10, 12, score_str, theme_.text, 1);
    
    // 绘制游戏状态
    const char* status_str = "Playing";
    switch (game_state.getGameStatus()) {
        case GameStatus::WON: status_str = "WON! 🎉"; break;
        case GameStatus::GAME_OVER: status_str = "Game Over"; break;
        default: break;
    }
    
    // 计算状态文本位置（右对齐）
    uint16_t status_width = strlen(status_str) * 8; // 估算文本宽度
    uint16_t status_x = width - status_width - 10;
    drawText(status_x, 12, status_str, theme_.text, 1);
}

void UnifiedGameUI::drawBoard(const BoardArray& board) {
    uint16_t x = layout_.board_start_x;
    uint16_t y = layout_.board_start_y;
    uint16_t size = layout_.board_size_pixels;
    
    // 绘制棋盘背景
    display_->fillRect(x, y, size, size, theme_.board_bg);
    
    // 绘制网格线
    for (int i = 0; i <= 4; ++i) {
        uint16_t pos = x + i * layout_.cell_size;
        
        // 垂直线
        display_->fillRect(pos, y, layout_.grid_thickness, size, theme_.grid_line);
        
        // 水平线
        display_->fillRect(x, pos, size, layout_.grid_thickness, theme_.grid_line);
    }
    
    // 绘制数字方块
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            uint16_t tile_x = x + j * layout_.cell_size + layout_.grid_thickness;
            uint16_t tile_y = y + i * layout_.cell_size + layout_.grid_thickness;
            drawTile(tile_x, tile_y, board[i][j]);
        }
    }
}

void UnifiedGameUI::drawTile(uint16_t x, uint16_t y, int value) {
    uint16_t tile_size = layout_.cell_size - layout_.grid_thickness;
    
    if (value == 0) {
        // 空白格子
        display_->fillRect(x, y, tile_size, tile_size, theme_.board_bg);
        return;
    }
    
    // 获取颜色
    uint8_t color_index = getColorIndex(value);
    Color tile_color = theme_.tile_colors[color_index];
    
    // 绘制方块背景
    display_->fillRect(x, y, tile_size, tile_size, tile_color);
    
    // 绘制数字
    char num_str[8];
    snprintf(num_str, sizeof(num_str), "%d", value);
    
    // 计算文本位置（居中）
    uint16_t text_width = strlen(num_str) * 8; // 估算文本宽度
    uint16_t text_height = 16; // 估算文本高度
    uint16_t text_x = x + (tile_size - text_width) / 2;
    uint16_t text_y = y + (tile_size - text_height) / 2;
    
    // 根据数字大小调整字体大小
    uint8_t font_size = 1;
    if (value >= 1000) font_size = 1;
    else if (value >= 100) font_size = 1;
    else font_size = 1;
    
    drawText(text_x, text_y, num_str, theme_.text, font_size);
}

void UnifiedGameUI::drawHintBar() {
    uint16_t width = display_->getWidth();
    uint16_t height = display_->getHeight();
    uint16_t y = height - layout_.hint_bar_height;
    
    // 绘制提示栏背景
    display_->fillRect(0, y, width, layout_.hint_bar_height, theme_.score_bg);
    
    // 绘制控制提示
    drawText(10, y + 5, "Joystick: Move | Button: Reset", theme_.text, 1);
}

bool UnifiedGameUI::handleInput(GameLogic& logic, GameState& game_state) {
    if (!joystick_) {
        return false;
    }
    
    // 更新摇杆状态
    joystick_->update();
    
    // 获取当前方向
    input::JoystickDirection direction = joystick_->getCurrentDirection();
    
    // 检查按钮状态
    bool button_pressed = joystick_->isButtonPressed();
    
    bool input_processed = false;
    
    // 处理方向输入
    switch (direction) {
        case input::JoystickDirection::UP:
            logic.moveUp(game_state);
            input_processed = true;
            break;
        case input::JoystickDirection::DOWN:
            logic.moveDown(game_state);
            input_processed = true;
            break;
        case input::JoystickDirection::LEFT:
            logic.moveLeft(game_state);
            input_processed = true;
            break;
        case input::JoystickDirection::RIGHT:
            logic.moveRight(game_state);
            input_processed = true;
            break;
        default:
            break;
    }
    
    // 处理按钮输入（重置游戏）
    if (button_pressed) {
        game_state.resetBoard();
        game_state.addRandomTile();
        game_state.addRandomTile();
        resetCache();
        input_processed = true;
    }
    
    return input_processed;
}

void UnifiedGameUI::setColorTheme(const ColorTheme& theme) {
    theme_ = theme;
}

void UnifiedGameUI::clear() {
    if (display_) {
        display_->clear(theme_.background);
    }
}

void UnifiedGameUI::refresh() {
    if (display_) {
        display_->display();
    }
}

void UnifiedGameUI::resetCache() {
    memset(last_board_, 0, sizeof(BoardArray));
    last_score_ = 0;
    last_game_status_ = GameStatus::READY;
    first_draw_ = true;
}

uint8_t UnifiedGameUI::getColorIndex(int value) const {
    // 计算颜色索引
    int index = 0;
    int temp = value;
    while (temp > 2 && index < 11) {
        temp /= 2;
        index++;
    }
    return (index < 12) ? index : 11;
}

void UnifiedGameUI::drawText(uint16_t x, uint16_t y, const std::string& text, const Color& color, uint8_t size) {
    if (display_) {
        display_->drawString(x, y, text, color, size);
    }
}

void UnifiedGameUI::drawNumber(uint16_t x, uint16_t y, int number, const Color& color, uint8_t size) {
    char num_str[16];
    snprintf(num_str, sizeof(num_str), "%d", number);
    drawText(x, y, num_str, color, size);
}

} // namespace game2048
