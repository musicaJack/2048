#pragma once

#include "display/display_driver.hpp"
#include "game/game_2048.hpp"
#include "input/joystick_handler.hpp"
#include <memory>

namespace game2048 {

// 统一的颜色主题
struct ColorTheme {
    Color background;      // 背景色
    Color board_bg;        // 棋盘背景
    Color grid_line;       // 网格线
    Color text;           // 文字颜色
    Color score_bg;       // 分数背景
    Color tile_colors[12]; // 数字方块颜色 (2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048)
    
    ColorTheme() {
        // 默认主题 - 深色背景，鲜艳数字
        background = Color(0, 0, 0);
        board_bg = Color(0, 0, 0);
        grid_line = Color(255, 255, 255);
        text = Color(255, 255, 255);
        score_bg = Color(83, 52, 131);
        
        // 数字颜色 - 鲜艳马卡龙色系
        tile_colors[0] = Color(255, 179, 186);  // 2
        tile_colors[1] = Color(255, 179, 217);  // 4
        tile_colors[2] = Color(255, 179, 255);  // 8
        tile_colors[3] = Color(217, 179, 255);  // 16
        tile_colors[4] = Color(179, 179, 255);  // 32
        tile_colors[5] = Color(179, 217, 255);  // 64
        tile_colors[6] = Color(179, 255, 255);  // 128
        tile_colors[7] = Color(179, 255, 217);  // 256
        tile_colors[8] = Color(179, 255, 179);  // 512
        tile_colors[9] = Color(217, 255, 179);  // 1024
        tile_colors[10] = Color(255, 255, 179); // 2048
        tile_colors[11] = Color(255, 217, 179); // 4096+
    }
};

// 游戏布局配置
struct GameLayout {
    uint16_t board_size_pixels;    // 棋盘尺寸（像素）
    uint16_t cell_size;           // 单个格子尺寸
    uint16_t board_start_x;       // 棋盘起始X坐标
    uint16_t board_start_y;       // 棋盘起始Y坐标
    uint16_t status_bar_height;   // 状态栏高度
    uint16_t hint_bar_height;     // 提示栏高度
    uint8_t grid_thickness;       // 网格线粗细
    uint8_t border_thickness;     // 边框粗细
    
    GameLayout() : board_size_pixels(300), cell_size(75), board_start_x(10), 
                   board_start_y(80), status_bar_height(40), hint_bar_height(30),
                   grid_thickness(2), border_thickness(3) {}
};

// 统一的游戏UI管理器
class UnifiedGameUI {
public:
    UnifiedGameUI(DisplayDriver* display, input::JoystickHandler* joystick);
    ~UnifiedGameUI() = default;
    
    // 初始化UI
    bool init();
    
    // 计算自适应布局
    void calculateLayout();
    
    // 绘制游戏界面
    void drawGame(const Game2048& game);
    
    // 绘制状态栏
    void drawStatusBar(const GameState& game_state);
    
    // 绘制棋盘
    void drawBoard(const BoardArray& board);
    
    // 绘制单个方块
    void drawTile(uint16_t x, uint16_t y, int value);
    
    // 绘制提示栏
    void drawHintBar();
    
    // 处理输入
    bool handleInput(GameLogic& logic, GameState& game_state);
    
    // 设置颜色主题
    void setColorTheme(const ColorTheme& theme);
    
    // 获取当前布局
    const GameLayout& getLayout() const { return layout_; }
    
    // 清屏
    void clear();
    
    // 刷新显示
    void refresh();

private:
    DisplayDriver* display_;
    input::JoystickHandler* joystick_;
    ColorTheme theme_;
    GameLayout layout_;
    
    // 缓存上次的棋盘状态，用于局部刷新
    BoardArray last_board_;
    uint32_t last_score_;
    GameStatus last_game_status_;
    bool first_draw_;
    
    // 重置缓存
    void resetCache();
    
    // 获取数字对应的颜色索引
    uint8_t getColorIndex(int value) const;
    
    // 绘制文本（适配不同显示器）
    void drawText(uint16_t x, uint16_t y, const std::string& text, const Color& color, uint8_t size = 1);
    
    // 绘制数字文本
    void drawNumber(uint16_t x, uint16_t y, int number, const Color& color, uint8_t size = 1);
};

} // namespace game2048
