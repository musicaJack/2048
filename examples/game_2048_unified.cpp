/**
 * @file game_2048_unified.cpp
 * @brief 2048游戏 - 统一版本，支持编译时切换显示驱动 (ST7306/ILI9488)
 * @author 集成项目团队
 * @date 2024
 */

#include "pico/stdlib.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include "display/display_manager.hpp"
#include "display/adapters/st7306_adapter.hpp"
#include "display/adapters/ili9488_adapter.hpp"
#include "input/joystick_handler.hpp"
#include "hardware/input/joystick.hpp"
#include "game/game_2048.hpp"
#include "config/display_config_compile_time.hpp"
#include "hardware/gpio.h"

using namespace game2048;

// 全局变量
Game2048* g_game = nullptr;
input::JoystickHandler* g_joystick_handler = nullptr;
Joystick* g_joystick = nullptr;
DisplayDriver* g_display = nullptr;

// 游戏配置
#define GAME_UPDATE_DELAY_MS 50
#define JOYSTICK_DIRECTION_RATIO 0.7f
#define INPUT_DEBOUNCE_MS 200

// 棋盘布局参数 - 自适应不同分辨率
uint16_t BOARD_SIZE_PIXELS = 300;  // 棋盘尺寸
uint16_t CELL_SIZE = 75;           // 格子尺寸
uint16_t BOARD_START_X = 10;       // 棋盘起始X
uint16_t BOARD_START_Y = 50;       // 棋盘起始Y

// 颜色定义 - 根据显示驱动选择不同的配色方案
#ifdef DISPLAY_DRIVER_ST7306
    // ST7306 4级灰度配色方案
    #define COLOR_BACKGROUND Color(255, 255, 255)  // 白色背景（灰度0 - 最亮）
    #define COLOR_BOARD_BG Color(170, 170, 170)    // 棋盘背景（灰度1）
    #define COLOR_GRID Color(85, 85, 85)           // 网格线（灰度2）
    #define COLOR_BORDER Color(0, 0, 0)            // 外框线（灰度3 - 最暗）
    #define COLOR_TEXT Color(255, 255, 255)        // 文字颜色（白色）
    #define COLOR_SCORE_BG Color(0, 0, 0)          // 状态栏背景（黑色）
    #define COLOR_TILE_BORDER Color(85, 85, 85)    // 方块边框（灰度2）

    // 数字方块颜色 - ST7306 4级灰度优化设计
    const Color TILE_COLORS[] = {
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
#else
    // ILI9488 深色主题配色方案
    #define COLOR_BACKGROUND Color(0, 0, 0)        // 纯黑色背景
    #define COLOR_BOARD_BG Color(40, 40, 40)       // 深灰色棋盘背景
    #define COLOR_GRID Color(60, 60, 60)           // 中灰色网格线
    #define COLOR_BORDER Color(80, 80, 80)         // 浅灰色外框线
    #define COLOR_TEXT Color(255, 255, 255)        // 白色文字
    #define COLOR_SCORE_BG Color(20, 30, 50)       // 蓝黑色状态栏背景
    #define COLOR_TILE_BORDER Color(100, 100, 100) // 浅灰色方块边框

    // 数字方块颜色 - ILI9488 深色主题优化设计
    const Color TILE_COLORS[] = {
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
#endif

// 游戏状态
enum class LocalGameState {
    INIT,
    PLAYING,
    WON,
    GAME_OVER
};

LocalGameState g_game_state = LocalGameState::INIT;
uint32_t g_last_input_time = 0;
bool g_input_enabled = true;

// 缓存变量 - 用于局部刷新
BoardArray g_last_board = {{0}};
uint32_t g_last_score = 0;
GameStatus g_last_game_status = GameStatus::READY;
bool g_first_draw = true;

// 重置缓存状态
void resetCache() {
    memset(g_last_board, 0, sizeof(BoardArray));
    g_last_score = 0;
    g_last_game_status = GameStatus::READY;
    g_first_draw = true;
}

// 控制台输出函数
void logPrint(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// 自适应布局计算
void calculateLayout() {
    uint16_t display_width = g_display->getWidth();
    uint16_t display_height = g_display->getHeight();
    
    // 计算最大可用棋盘尺寸
    uint16_t available_width = display_width - 40;  // 留20像素边距
    uint16_t available_height = display_height - 120; // 留出状态栏和提示栏空间
    
    // 选择较小的尺寸作为棋盘尺寸，确保正方形
    BOARD_SIZE_PIXELS = (available_width < available_height) ? available_width : available_height;
    
    // 确保棋盘尺寸能被4整除
    BOARD_SIZE_PIXELS = (BOARD_SIZE_PIXELS / 4) * 4;
    
    // 计算格子尺寸
    CELL_SIZE = BOARD_SIZE_PIXELS / 4;
    
    // 计算棋盘居中位置
    BOARD_START_X = (display_width - BOARD_SIZE_PIXELS) / 2;
    BOARD_START_Y = 60; // 留出状态栏空间
    
    logPrint("自适应布局计算完成:");
    logPrint("  显示分辨率: %dx%d", display_width, display_height);
    logPrint("  棋盘尺寸: %dx%d", BOARD_SIZE_PIXELS, BOARD_SIZE_PIXELS);
    logPrint("  格子尺寸: %d", CELL_SIZE);
    logPrint("  棋盘位置: (%d, %d)", BOARD_START_X, BOARD_START_Y);
}

// 绘制数字方块背景（只绘制背景，不绘制文字）
void drawTileBackground(uint16_t x, uint16_t y, int value) {
    if (value == 0) {
        // 空白格子
        g_display->fillRect(x, y, CELL_SIZE - 2, CELL_SIZE - 2, COLOR_BOARD_BG);
        return;
    }
    
    // 计算颜色索引
    int colorIndex = 0;
    int tempValue = value;
    while (tempValue > 2) {
        tempValue /= 2;
        colorIndex++;
    }
    if (colorIndex >= sizeof(TILE_COLORS)/sizeof(TILE_COLORS[0])) {
        colorIndex = sizeof(TILE_COLORS)/sizeof(TILE_COLORS[0]) - 1;
    }
    
    // 绘制方块背景
    g_display->fillRect(x, y, CELL_SIZE - 2, CELL_SIZE - 2, TILE_COLORS[colorIndex]);
    
    // 为浅色背景的数字添加边框，增强视觉边界感
    if (colorIndex <= 1) { // 数字2、4（白色和浅灰背景）
        // 绘制中灰边框
        g_display->drawRect(x, y, CELL_SIZE - 2, CELL_SIZE - 2, COLOR_TILE_BORDER);
    }
}

// 绘制数字方块文字（只绘制文字，不绘制背景）
void drawTileText(uint16_t x, uint16_t y, int value) {
    if (value == 0) {
        return; // 空白格子不需要绘制文字
    }
    
    // 计算颜色索引
    int colorIndex = 0;
    int tempValue = value;
    while (tempValue > 2) {
        tempValue /= 2;
        colorIndex++;
    }
    if (colorIndex >= sizeof(TILE_COLORS)/sizeof(TILE_COLORS[0])) {
        colorIndex = sizeof(TILE_COLORS)/sizeof(TILE_COLORS[0]) - 1;
    }
    
    // 绘制数字
    char numStr[8];
    snprintf(numStr, sizeof(numStr), "%d", value);
    
    // 根据数字大小调整字体大小 - 最终优化版本
    uint8_t fontSize = 1;
    if (value >= 1000) {
        fontSize = 1; // 4位数用1倍字体 (1024, 2048, 4096) - 确保完整显示
    } else if (value >= 100) {
        fontSize = 1; // 3位数用1倍字体 (128, 256, 512) - 降低到1倍，确保一致性
    } else if (value >= 10) {
        fontSize = 2; // 2位数用2倍字体 (16, 32, 64) - 保持较大显示
    } else {
        fontSize = 2; // 1位数用2倍字体 (2, 4, 8) - 保持较大显示
    }
    
    // 计算文字位置（居中）- 使用Game2048字体（12x16）
    uint16_t textWidth = strlen(numStr) * 12 * fontSize; // Game2048字体宽度
    uint16_t textX = x + (CELL_SIZE - textWidth) / 2;
    uint16_t textY = y + (CELL_SIZE - 16 * fontSize) / 2; // Game2048字体高度
    
    // 根据背景色选择文字颜色，确保对比度
    Color textColor;
    #ifdef DISPLAY_DRIVER_ST7306
        if (colorIndex <= 1) {
            // 白色和浅灰背景用黑色文字
            textColor = Color(0, 0, 0);
        } else {
            // 中灰和黑色背景用白色文字
            textColor = Color(255, 255, 255);
        }
    #else
        // ILI9488深色主题版本：根据背景亮度选择文字颜色
        const Color& bgColor = TILE_COLORS[colorIndex];
        uint8_t brightness = (bgColor.r * 299 + bgColor.g * 587 + bgColor.b * 114) / 1000;
        if (brightness > 180) {
            // 浅色背景用深色文字
            textColor = Color(119, 110, 101); // 深棕色
        } else {
            // 深色背景用白色文字
            textColor = Color(255, 255, 255); // 白色
        }
    #endif
    
    // 使用Game2048优化字体绘制 - 统一支持两种驱动
    #ifdef DISPLAY_DRIVER_ST7306
        if (auto* st7306_adapter = static_cast<ST7306Adapter*>(g_display)) {
            st7306_adapter->drawStringGame2048(textX, textY, numStr, textColor, TILE_COLORS[colorIndex], fontSize);
        } else {
            g_display->drawString(textX, textY, numStr, textColor, TILE_COLORS[colorIndex], fontSize);
        }
    #elif defined(DISPLAY_DRIVER_ILI9488)
        if (auto* ili9488_adapter = static_cast<ILI9488Adapter*>(g_display)) {
            ili9488_adapter->drawStringGame2048(textX, textY, numStr, textColor, TILE_COLORS[colorIndex], fontSize);
        } else {
            g_display->drawString(textX, textY, numStr, textColor, TILE_COLORS[colorIndex], fontSize);
        }
    #else
        g_display->drawString(textX, textY, numStr, textColor, TILE_COLORS[colorIndex], fontSize);
    #endif
}

// 绘制游戏棋盘（局部刷新版本）
void drawBoardOptimized(const GameState& game_state) {
    const BoardArray& board = game_state.getBoard();
    
    // 如果是首次绘制，绘制完整棋盘
    if (g_first_draw) {
        // 绘制棋盘背景
        g_display->fillRect(BOARD_START_X - 10, BOARD_START_Y - 10, 
                           BOARD_SIZE_PIXELS + 20, BOARD_SIZE_PIXELS + 20, COLOR_BOARD_BG);
        
        // 绘制白色外框线
        // 上边框
        g_display->fillRect(BOARD_START_X - 10, BOARD_START_Y - 10, 
                           BOARD_SIZE_PIXELS + 20, 2, COLOR_BORDER);
        // 下边框
        g_display->fillRect(BOARD_START_X - 10, BOARD_START_Y + BOARD_SIZE_PIXELS + 8, 
                           BOARD_SIZE_PIXELS + 20, 2, COLOR_BORDER);
        // 左边框
        g_display->fillRect(BOARD_START_X - 10, BOARD_START_Y - 10, 
                           2, BOARD_SIZE_PIXELS + 20, COLOR_BORDER);
        // 右边框
        g_display->fillRect(BOARD_START_X + BOARD_SIZE_PIXELS + 8, BOARD_START_Y - 10, 
                           2, BOARD_SIZE_PIXELS + 20, COLOR_BORDER);
        
        // 第一步：绘制所有方块背景
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                uint16_t x = BOARD_START_X + j * CELL_SIZE + 1;
                uint16_t y = BOARD_START_Y + i * CELL_SIZE + 1;
                drawTileBackground(x, y, board[i][j]);
            }
        }
        
        // 第二步：绘制所有方块文字
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                uint16_t x = BOARD_START_X + j * CELL_SIZE + 1;
                uint16_t y = BOARD_START_Y + i * CELL_SIZE + 1;
                drawTileText(x, y, board[i][j]);
            }
        }
        
        // 复制当前状态到缓存
        memcpy(g_last_board, board, sizeof(BoardArray));
        g_first_draw = false;
        return;
    }
    
    // 局部刷新：只更新变化的方块
    // 第一步：绘制变化的方块背景
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (g_last_board[i][j] != board[i][j]) {
                uint16_t x = BOARD_START_X + j * CELL_SIZE + 1;
                uint16_t y = BOARD_START_Y + i * CELL_SIZE + 1;
                drawTileBackground(x, y, board[i][j]);
            }
        }
    }
    
    // 第二步：绘制变化的方块文字
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (g_last_board[i][j] != board[i][j]) {
                uint16_t x = BOARD_START_X + j * CELL_SIZE + 1;
                uint16_t y = BOARD_START_Y + i * CELL_SIZE + 1;
                drawTileText(x, y, board[i][j]);
            }
        }
    }
    
    // 更新缓存
    memcpy(g_last_board, board, sizeof(BoardArray));
}

// 绘制状态栏（局部刷新版本）
void drawStatusBarOptimized(const GameState& game_state) {
    uint32_t current_score = game_state.getCurrentScore();
    GameStatus current_game_status = game_state.getGameStatus();
    
    // 检查是否需要更新状态栏
    bool need_update = g_first_draw || 
                      current_score != g_last_score ||
                      current_game_status != g_last_game_status;
    
    if (!need_update) {
        return;
    }
    
    // 重绘状态栏背景
    g_display->fillRect(0, 0, g_display->getWidth(), 40, COLOR_SCORE_BG);
    
    // 分数信息 - 只显示当前分数
    char scoreStr[64];
    snprintf(scoreStr, sizeof(scoreStr), "Score: %lu", current_score);
    g_display->drawString(10, 12, scoreStr, COLOR_TEXT, 1);
    
    // 游戏状态
    const char* statusStr = "Playing";
    switch (current_game_status) {
        case GameStatus::WON: statusStr = "WON! 🎉"; break;
        case GameStatus::GAME_OVER: statusStr = "Game Over"; break;
        default: break;
    }
    
    uint16_t statusX = g_display->getWidth() - strlen(statusStr) * 12 - 10;
    g_display->drawString(statusX, 12, statusStr, COLOR_TEXT, 1);
    
    // 更新缓存
    g_last_score = current_score;
    g_last_game_status = current_game_status;
}

// 绘制控制提示
void drawControlHint() {
    // 底部控制提示背景
    g_display->fillRect(0, g_display->getHeight() - 30, g_display->getWidth(), 30, COLOR_SCORE_BG);
    g_display->drawString(10, g_display->getHeight() - 25, "Joystick: Move | Button: Reset", COLOR_TEXT, 1);
}

// 处理摇杆输入（优化版本）
void handleJoystickInput(GameLogic& logic, GameState& game_state) {
    if (!g_joystick_handler || !g_input_enabled) {
        return;
    }
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // 更新摇杆状态
    g_joystick_handler->update();
    
    // 获取当前方向
    input::JoystickDirection direction = g_joystick_handler->getCurrentDirection();
    
    // 检查按钮状态
    bool button_pressed = g_joystick_handler->isButtonPressed();
    static bool last_button = false;
    
    // 处理方向输入（防抖）
    if (current_time - g_last_input_time > INPUT_DEBOUNCE_MS) {
        bool moved = false;
        
        switch (direction) {
            case input::JoystickDirection::UP:
                moved = logic.moveUp(game_state);
                break;
            case input::JoystickDirection::DOWN:
                moved = logic.moveDown(game_state);
                break;
            case input::JoystickDirection::LEFT:
                moved = logic.moveLeft(game_state);
                break;
            case input::JoystickDirection::RIGHT:
                moved = logic.moveRight(game_state);
                break;
            default:
                break;
        }
        
        if (moved) {
            g_last_input_time = current_time;
            
            // 重绘棋盘和状态栏
            drawBoardOptimized(game_state);
            drawStatusBarOptimized(game_state);
            g_display->display(); // 刷新显示
        }
    }
    
    // 处理按钮输入（重置游戏）
    if (button_pressed && !last_button) {
        logPrint("重置游戏...");
        g_game->reset();
        g_game_state = LocalGameState::PLAYING;
        g_last_input_time = to_ms_since_boot(get_absolute_time());
        
        // 重置缓存状态
        resetCache();
        
        // 重绘整个界面
        g_display->clear(COLOR_BACKGROUND);
        drawBoardOptimized(game_state);
        drawStatusBarOptimized(game_state);
        drawControlHint();
        g_display->display(); // 刷新显示
        
        logPrint("游戏重置完成!");
    }
    last_button = button_pressed;
}

// 检查游戏状态
void checkGameStatus(GameLogic& logic, GameState& game_state) {
    // 检查是否获胜
    if (game_state.hasWon() && game_state.getGameStatus() == GameStatus::PLAYING) {
        game_state.setGameStatus(GameStatus::WON);
        g_game_state = LocalGameState::WON;
        logPrint("🎉 恭喜! 你达到了2048! 🎉");
        drawStatusBarOptimized(game_state);
        g_display->display(); // 刷新显示
    }
    
    // 检查游戏是否结束
    if (game_state.isGameOver()) {
        game_state.setGameStatus(GameStatus::GAME_OVER);
        g_game_state = LocalGameState::GAME_OVER;
        logPrint("💀 游戏结束! 没有可移动的位置了! 💀");
        drawStatusBarOptimized(game_state);
        g_display->display(); // 刷新显示
    }
}

int main() {
    // 初始化Pico SDK
    stdio_init_all();
    
    // 初始化板载LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    
    // 等待串口稳定
    sleep_ms(2000);
    
    logPrint("========================================");
    logPrint("=== 2048游戏 - 统一版本 ===");
    logPrint("========================================");
    logPrint("编译时选择的显示驱动: %s", CURRENT_DISPLAY_DRIVER);
    logPrint("显示分辨率: %dx%d", DISPLAY_WIDTH, DISPLAY_HEIGHT);
    logPrint("颜色深度: %d", DISPLAY_COLOR_DEPTH);
    logPrint("是否彩色: %s", DISPLAY_IS_COLOR ? "是" : "否");
    logPrint("是否灰度: %s", DISPLAY_IS_GRAYSCALE ? "是" : "否");
    logPrint("========================================");
    
    // 初始化显示管理器
    logPrint("正在初始化显示管理器...");
    DisplayManager& display_mgr = DisplayManager::getInstance();
    
    if (!display_mgr.init()) {
        logPrint("错误: 显示管理器初始化失败!");
        return -1;
    }
    
    // 获取显示驱动
    g_display = display_mgr.getDriver();
    if (!g_display) {
        logPrint("错误: 无法获取显示驱动!");
        return -1;
    }
    
    logPrint("显示管理器初始化成功!");
    
    // 计算自适应布局
    calculateLayout();
    
    // 初始化摇杆硬件
    logPrint("正在初始化摇杆...");
    g_joystick = new Joystick();
    if (!g_joystick->begin(JOYSTICK_I2C_INST)) {
        logPrint("错误: 摇杆硬件初始化失败!");
        return -1;
    }
    
    // 初始化摇杆处理器
    g_joystick_handler = new input::JoystickHandler();
    g_joystick_handler->setDeadzone(300);  // 设置死区
    g_joystick_handler->setDirectionRatio(JOYSTICK_DIRECTION_RATIO);  // 设置方向比例
    
    if (!g_joystick_handler->initialize(g_joystick)) {
        logPrint("错误: 摇杆处理器初始化失败!");
        return -1;
    }
    
    logPrint("摇杆初始化成功!");
    
    // 创建游戏实例
    logPrint("正在初始化游戏...");
    g_game = new Game2048();
    
    if (!g_game->initialize()) {
        logPrint("错误: 游戏初始化失败!");
        return -1;
    }
    
    GameLogic& logic = g_game->getGameLogic();
    GameState& game_state = g_game->getGameState();
    
    logPrint("游戏初始化成功!");
    
    // 绘制初始界面
    logPrint("绘制游戏界面...");
    g_display->clear(COLOR_BACKGROUND);
    drawBoardOptimized(game_state);
    drawStatusBarOptimized(game_state);
    drawControlHint();
    g_display->display(); // 刷新显示
    
    logPrint("游戏已就绪! 使用摇杆控制游戏...");
    
    g_game_state = LocalGameState::PLAYING;
    g_last_input_time = to_ms_since_boot(get_absolute_time());
    
    // 主游戏循环
    while (true) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        switch (g_game_state) {
            case LocalGameState::PLAYING:
                // 处理摇杆输入
                handleJoystickInput(logic, game_state);
                
                // 检查游戏状态
                checkGameStatus(logic, game_state);
                
                // 板载LED闪烁指示程序运行
                gpio_put(PICO_DEFAULT_LED_PIN, (current_time / 500) % 2);
                break;
                
            case LocalGameState::WON:
            case LocalGameState::GAME_OVER:
                // 等待按钮输入重新开始
                handleJoystickInput(logic, game_state);
                break;
                
            default:
                break;
        }
        
        sleep_ms(GAME_UPDATE_DELAY_MS);
    }
    
    return 0;
}
