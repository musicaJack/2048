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
#include "hardware/display/ili9488_driver.hpp"

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

// 横屏尺寸 (Landscape_90)
#define LANDSCAPE_WIDTH 480
#define LANDSCAPE_HEIGHT 320

// 棋盘布局参数
#define BOARD_SIZE_PIXELS 200
#define CELL_SIZE (BOARD_SIZE_PIXELS / 4)
#define BOARD_START_X ((LANDSCAPE_WIDTH - BOARD_SIZE_PIXELS) / 2)
#define BOARD_START_Y ((LANDSCAPE_HEIGHT - BOARD_SIZE_PIXELS) / 2)

// 颜色定义 (RGB666)
#define COLOR_BACKGROUND 0x000000  // 纯黑色背景
#define COLOR_BOARD_BG 0x16213E    // 棋盘背景
#define COLOR_GRID 0x0F3460        // 网格线
#define COLOR_TEXT 0xE94560        // 文字颜色
#define COLOR_SCORE_BG 0x533483    // 分数背景

// 数字颜色 (2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048)
const uint32_t TILE_COLORS[] = {
    0xEEE4DA, 0xEDE0C8, 0xF2B179, 0xF59563, 0xF67C5F,
    0xF65E3B, 0xEDCF72, 0xEDCC61, 0xEDC850, 0xEDC53F, 0xEDC22E
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

// 控制台输出函数
void logPrint(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// 绘制数字方块
void drawTile(uint16_t x, uint16_t y, int value, uint32_t color) {
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
    
    // 绘制数字
    char numStr[8];
    snprintf(numStr, sizeof(numStr), "%d", value);
    
    // 计算文字位置（居中）
    uint16_t textWidth = strlen(numStr) * 12; // 估算文字宽度
    uint16_t textX = x + (CELL_SIZE - textWidth) / 2;
    uint16_t textY = y + (CELL_SIZE - 16) / 2; // 16是字体高度
    
    g_display->drawString(textX, textY, numStr, COLOR_TEXT, TILE_COLORS[colorIndex]);
}

// 绘制游戏棋盘
void drawBoard(const game2048::GameState& game_state) {
    const BoardArray& board = game_state.getBoard();
    
    // 绘制棋盘背景
    g_display->fillAreaRGB666(BOARD_START_X - 10, BOARD_START_Y - 10, 
                              BOARD_START_X + BOARD_SIZE_PIXELS + 10, 
                              BOARD_START_Y + BOARD_SIZE_PIXELS + 10, COLOR_BOARD_BG);
    
    // 绘制网格和数字
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            uint16_t x = BOARD_START_X + j * CELL_SIZE;
            uint16_t y = BOARD_START_Y + i * CELL_SIZE;
            
            // 绘制数字方块
            drawTile(x + 1, y + 1, board[i][j], COLOR_TEXT);
        }
    }
}

// 绘制状态栏
void drawStatusBar(const game2048::GameState& game_state) {
    // 顶部状态栏背景
    g_display->fillAreaRGB666(0, 0, LANDSCAPE_WIDTH - 1, 40, COLOR_SCORE_BG);
    
    // 分数信息
    char scoreStr[64];
    snprintf(scoreStr, sizeof(scoreStr), "Score: %lu | Best: %lu", 
             game_state.getCurrentScore(), game_state.getBestScore());
    g_display->drawString(10, 12, scoreStr, COLOR_TEXT, COLOR_SCORE_BG);
    
    // 游戏状态
    const char* statusStr = "Playing";
    switch (game_state.getGameStatus()) {
        case GameStatus::WON: statusStr = "WON! 🎉"; break;
        case GameStatus::GAME_OVER: statusStr = "Game Over"; break;
        default: break;
    }
    
    uint16_t statusX = LANDSCAPE_WIDTH - strlen(statusStr) * 12 - 10;
    g_display->drawString(statusX, 12, statusStr, COLOR_TEXT, COLOR_SCORE_BG);
}

// 绘制控制提示
void drawControlHint() {
    // 底部控制提示背景
    g_display->fillAreaRGB666(0, LANDSCAPE_HEIGHT - 30, LANDSCAPE_WIDTH - 1, LANDSCAPE_HEIGHT - 1, COLOR_SCORE_BG);
    
    g_display->drawString(10, LANDSCAPE_HEIGHT - 25, "Joystick: Move | Button: Reset", COLOR_TEXT, COLOR_SCORE_BG);
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
            drawBoard(game_state);
            drawStatusBar(game_state);
        }
    }
    
    // 处理按钮输入（重置游戏）
    if (button_pressed && !last_button) {
        logPrint("Button pressed - Resetting game...");
        g_game->reset();
        g_game_state = LocalGameState::PLAYING;
        g_last_input_time = to_ms_since_boot(get_absolute_time());
        
        // 重绘整个界面
        g_display->fillScreenRGB666(COLOR_BACKGROUND);
        drawBoard(game_state);
        drawStatusBar(game_state);
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
        drawStatusBar(game_state);
    }
    
    // 检查游戏是否结束
    if (game_state.isGameOver()) {
        game_state.setGameStatus(GameStatus::GAME_OVER);
        g_game_state = LocalGameState::GAME_OVER;
        logPrint("💀 GAME OVER! No more moves possible! 💀");
        drawStatusBar(game_state);
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
    logPrint("=== 2048游戏 - 横屏版本 ===");
    logPrint("========================================");
    logPrint("屏幕尺寸: %dx%d", LANDSCAPE_WIDTH, LANDSCAPE_HEIGHT);
    logPrint("棋盘尺寸: %dx%d", BOARD_SIZE_PIXELS, BOARD_SIZE_PIXELS);
    logPrint("========================================");
    
    // 初始化显示屏
    logPrint("正在初始化显示屏...");
    g_display = new ILI9488Driver(ILI9488_GET_SPI_CONFIG());
    
    if (!g_display->initialize()) {
        logPrint("错误: 显示屏初始化失败!");
        return -1;
    }
    
    // 设置横屏模式
    logPrint("设置横屏模式...");
    g_display->setRotation(Rotation::Landscape_90);
    g_display->fillScreenRGB666(COLOR_BACKGROUND);
    
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
    drawBoard(game_state);
    drawStatusBar(game_state);
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
