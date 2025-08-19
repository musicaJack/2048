/**
 * @file game_2048_landscape.cpp
 * @brief 2048游戏 - 横屏版本 (480×320)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// 包含游戏核心头文件
#include "game/game_2048.hpp"
#include "game/game_state.hpp"
#include "game/game_logic.hpp"
#include "input/joystick_handler.hpp"
#include "utils/random_generator.hpp"
#include "hardware/input/joystick.hpp"
#include "hardware/system/pin_config.hpp"
#include "hardware/display/ili9488/ili9488_driver.hpp"

using namespace game2048;
using namespace input;
using namespace ili9488;

// 全局变量
Game2048* g_game = nullptr;
JoystickHandler* g_joystick_handler = nullptr;
Joystick* g_joystick = nullptr;
ILI9488Driver* g_display = nullptr;

// 游戏配置
#define GAME_UPDATE_DELAY_MS 50
#define JOYSTICK_DIRECTION_RATIO 0.7f

// 竖屏尺寸 (Portrait_0)
#define PORTRAIT_WIDTH 320
#define PORTRAIT_HEIGHT 480

// 棋盘布局参数 - 竖屏优化
#define BOARD_SIZE_PIXELS 300  // 最大棋盘尺寸
#define CELL_SIZE (BOARD_SIZE_PIXELS / 4)
#define BOARD_START_X ((PORTRAIT_WIDTH - BOARD_SIZE_PIXELS) / 2)  // 水平居中
#define BOARD_START_Y 80  // 留出顶部空间给状态栏

// 颜色定义 (RGB666) - 纯黑色背景 + 白色线框
#define COLOR_BACKGROUND 0x000000  // 纯黑色背景
#define COLOR_BOARD_BG 0x000000    // 棋盘背景（纯黑色）
#define COLOR_GRID 0xFFFFFF        // 网格线（白色）
#define COLOR_BORDER 0xFFFFFF      // 外框线（白色）
#define COLOR_TEXT 0xFFFFFF        // 文字颜色（白色）
#define COLOR_SCORE_BG 0x533483    // 分数背景

// 数字颜色 - 鲜艳马卡龙色系 (2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048)
const uint32_t TILE_COLORS[] = {
    0xFFB3BA, 0xFFB3D9, 0xFFB3FF, 0xD9B3FF, 0xB3B3FF,  // 粉红→粉紫→粉蓝
    0xB3D9FF, 0xB3FFFF, 0xB3FFD9, 0xB3FFB3, 0xD9FFB3, 0xFFFFB3  // 粉青→粉绿→粉黄
};

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
BoardArray g_last_board = {{0}};  // 上次的棋盘状态
uint32_t g_last_score = 0;        // 上次的分数
GameStatus g_last_game_status = GameStatus::READY;  // 上次的游戏状态
bool g_first_draw = true;         // 是否是首次绘制

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

// 绘制数字方块背景（只绘制背景，不绘制文字）
void drawTileBackground(uint16_t x, uint16_t y, int value) {
    if (value == 0) {
        // 空白格子
        g_display->fillAreaRGB666(x, y, x + CELL_SIZE - 2, y + CELL_SIZE - 2, COLOR_BOARD_BG);
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
    g_display->fillAreaRGB666(x, y, x + CELL_SIZE - 2, y + CELL_SIZE - 2, TILE_COLORS[colorIndex]);
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
    
    // 计算文字位置（居中）
    uint16_t textWidth = strlen(numStr) * 12; // 估算文字宽度
    uint16_t textX = x + (CELL_SIZE - textWidth) / 2;
    uint16_t textY = y + (CELL_SIZE - 16) / 2; // 16是字体高度
    
    g_display->drawString(textX, textY, numStr, COLOR_TEXT, TILE_COLORS[colorIndex]);
}

// 绘制数字方块（完整版本，保持兼容性）
void drawTile(uint16_t x, uint16_t y, int value, uint32_t color) {
    drawTileBackground(x, y, value);
    drawTileText(x, y, value);
}

// 绘制单个方块（局部刷新用）
void drawSingleTile(uint16_t x, uint16_t y, int value) {
    drawTile(x, y, value, COLOR_TEXT);
}

// 绘制游戏棋盘（局部刷新版本）
void drawBoardOptimized(const game2048::GameState& game_state) {
    const BoardArray& board = game_state.getBoard();
    
    // 如果是首次绘制，绘制完整棋盘
    if (g_first_draw) {
        // 绘制棋盘背景
        g_display->fillAreaRGB666(BOARD_START_X - 10, BOARD_START_Y - 10, 
                                  BOARD_START_X + BOARD_SIZE_PIXELS + 10, 
                                  BOARD_START_Y + BOARD_SIZE_PIXELS + 10, COLOR_BOARD_BG);
        
        // 绘制白色外框线
        // 上边框
        g_display->fillAreaRGB666(BOARD_START_X - 10, BOARD_START_Y - 10, 
                                  BOARD_START_X + BOARD_SIZE_PIXELS + 10, 
                                  BOARD_START_Y - 8, COLOR_BORDER);
        // 下边框
        g_display->fillAreaRGB666(BOARD_START_X - 10, BOARD_START_Y + BOARD_SIZE_PIXELS + 8, 
                                  BOARD_START_X + BOARD_SIZE_PIXELS + 10, 
                                  BOARD_START_Y + BOARD_SIZE_PIXELS + 10, COLOR_BORDER);
        // 左边框
        g_display->fillAreaRGB666(BOARD_START_X - 10, BOARD_START_Y - 10, 
                                  BOARD_START_X - 8, 
                                  BOARD_START_Y + BOARD_SIZE_PIXELS + 10, COLOR_BORDER);
        // 右边框
        g_display->fillAreaRGB666(BOARD_START_X + BOARD_SIZE_PIXELS + 8, BOARD_START_Y - 10, 
                                  BOARD_START_X + BOARD_SIZE_PIXELS + 10, 
                                  BOARD_START_Y + BOARD_SIZE_PIXELS + 10, COLOR_BORDER);
        
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
void drawStatusBarOptimized(const game2048::GameState& game_state) {
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
    g_display->fillAreaRGB666(0, 0, PORTRAIT_WIDTH - 1, 40, COLOR_SCORE_BG);
    
    // 分数信息 - 只显示当前分数
    char scoreStr[64];
    snprintf(scoreStr, sizeof(scoreStr), "Score: %lu", current_score);
    g_display->drawString(10, 12, scoreStr, COLOR_TEXT, COLOR_SCORE_BG);
    
    // 游戏状态
    const char* statusStr = "Playing";
    switch (current_game_status) {
        case GameStatus::WON: statusStr = "WON! 🎉"; break;
        case GameStatus::GAME_OVER: statusStr = "Game Over"; break;
        default: break;
    }
    
    uint16_t statusX = PORTRAIT_WIDTH - strlen(statusStr) * 12 - 10;
    g_display->drawString(statusX, 12, statusStr, COLOR_TEXT, COLOR_SCORE_BG);
    
    // 更新缓存
    g_last_score = current_score;
    g_last_game_status = current_game_status;
}

// 绘制控制提示
void drawControlHint() {
    // 底部控制提示背景
    g_display->fillAreaRGB666(0, PORTRAIT_HEIGHT - 30, PORTRAIT_WIDTH - 1, PORTRAIT_HEIGHT - 1, COLOR_SCORE_BG);
    
    g_display->drawString(10, PORTRAIT_HEIGHT - 25, "Joystick: Move | Button: Reset", COLOR_TEXT, COLOR_SCORE_BG);
}

// 处理摇杆输入
void handleJoystickInput(GameLogic& logic, game2048::GameState& game_state) {
    if (!g_joystick_handler || !g_input_enabled) {
        return;
    }
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // 更新摇杆状态
    g_joystick_handler->update();
    
    // 获取当前方向
    JoystickDirection direction = g_joystick_handler->getCurrentDirection();
    
    // 检查按钮状态
    bool button_pressed = g_joystick_handler->isButtonPressed();
    static bool last_button = false;
    
    // 处理方向输入（防抖）
    if (current_time - g_last_input_time > 200) { // 200ms防抖
        bool moved = false;
        
        switch (direction) {
            case JoystickDirection::UP:
                moved = logic.moveUp(game_state);
                break;
            case JoystickDirection::DOWN:
                moved = logic.moveDown(game_state);
                break;
            case JoystickDirection::LEFT:
                moved = logic.moveLeft(game_state);
                break;
            case JoystickDirection::RIGHT:
                moved = logic.moveRight(game_state);
                break;
            default:
                break;
        }
        
        if (moved) {
            logPrint("Move successful!");
            g_last_input_time = current_time;
            
            // 重绘棋盘
            drawBoardOptimized(game_state);
            drawStatusBarOptimized(game_state);
        }
    }
    
    // 处理按钮输入（重置游戏）
    if (button_pressed && !last_button) {
        logPrint("Button pressed - Resetting game...");
        g_game->reset();
        g_game_state = LocalGameState::PLAYING;
        g_last_input_time = to_ms_since_boot(get_absolute_time());
        
        // 重置缓存状态
        resetCache();
        
        // 重绘整个界面
        g_display->fillScreenRGB666(COLOR_BACKGROUND);
        drawBoardOptimized(game_state);
        drawStatusBarOptimized(game_state);
        drawControlHint();
        
        logPrint("Game reset complete!");
    }
    last_button = button_pressed;
}

// 检查游戏状态
void checkGameStatus(GameLogic& logic, game2048::GameState& game_state) {
    // 检查是否获胜
    if (game_state.hasWon() && game_state.getGameStatus() == GameStatus::PLAYING) {
        game_state.setGameStatus(GameStatus::WON);
        g_game_state = LocalGameState::WON;
        logPrint("🎉 CONGRATULATIONS! You reached 2048! 🎉");
        drawStatusBarOptimized(game_state);
    }
    
    // 检查游戏是否结束
    if (game_state.isGameOver()) {
        game_state.setGameStatus(GameStatus::GAME_OVER);
        g_game_state = LocalGameState::GAME_OVER;
        logPrint("💀 GAME OVER! No more moves possible! 💀");
        drawStatusBarOptimized(game_state);
    }
}

int main() {
    // 初始化标准库
    stdio_init_all();
    
    // 初始化板载LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    
    // 等待串口稳定
    sleep_ms(2000);
    
    logPrint("========================================");
    logPrint("=== 2048游戏 - 竖屏版本 ===");
    logPrint("========================================");
    logPrint("屏幕尺寸: %dx%d", PORTRAIT_WIDTH, PORTRAIT_HEIGHT);
    logPrint("棋盘尺寸: %dx%d", BOARD_SIZE_PIXELS, BOARD_SIZE_PIXELS);
    logPrint("========================================");
    
    // 初始化显示屏
    logPrint("正在初始化显示屏...");
    g_display = new ILI9488Driver(ILI9488_GET_SPI_CONFIG());
    
    if (!g_display->initialize()) {
        logPrint("错误: 显示屏初始化失败!");
        return -1;
    }
    
    // 设置竖屏模式（0度旋转）
    logPrint("设置竖屏模式...");
    g_display->setRotation(Rotation::Portrait_0);
    g_display->fillScreenRGB666(COLOR_BACKGROUND);
    
    // 设置背光亮度
    g_display->setBacklightBrightness(ILI9488_BACKLIGHT_BRIGHTNESS);
    logPrint("背光亮度设置为: %d (%.1f%%)", ILI9488_BACKLIGHT_BRIGHTNESS, (ILI9488_BACKLIGHT_BRIGHTNESS * 100.0f) / 255.0f);
    
    logPrint("显示屏初始化成功!");
    
    // 初始化摇杆
    logPrint("正在初始化摇杆...");
    g_joystick = new Joystick();
    if (!g_joystick->begin(JOYSTICK_I2C_INST, JOYSTICK_ADDR, JOYSTICK_PIN_SDA, JOYSTICK_PIN_SCL, JOYSTICK_I2C_SPEED)) {
        logPrint("错误: 摇杆初始化失败!");
        return -1;
    }
    
    // 初始化摇杆处理器
    g_joystick_handler = new JoystickHandler();
    g_joystick_handler->setDeadzone(300);
    g_joystick_handler->setDirectionRatio(JOYSTICK_DIRECTION_RATIO);
    
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
    
    logPrint("游戏初始化成功!");
    
    // 获取游戏组件
    game2048::GameState& game_state = g_game->getGameState();
    GameLogic& logic = g_game->getGameLogic();
    
    // 绘制初始界面
    logPrint("绘制游戏界面...");
    drawBoardOptimized(game_state);
    drawStatusBarOptimized(game_state);
    drawControlHint();
    
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
