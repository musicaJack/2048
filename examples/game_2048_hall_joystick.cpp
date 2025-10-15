/**
 * @file game_2048_hall_joystick.cpp
 * @brief 2048游戏 - 使用霍尔摇杆直接连接版本
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
#include "hardware/input/hall_joystick_direct.hpp"
#include "game/game_2048.hpp"
#include "game/game_config.hpp"
#include "game/game_ui.hpp"
#include "game/game_input.hpp"
#include "game/game_state_manager.hpp"
#include "game/game_animation.hpp"
#include "config/display_config_compile_time.hpp"
#include "hardware/gpio.h"

using namespace game2048;

// 全局变量
Game2048* g_game = nullptr;
input::JoystickHandler* g_joystick_handler = nullptr;
HallJoystickDirect* g_hall_joystick = nullptr;
DisplayDriver* g_display = nullptr;
GameUIRenderer* g_ui_renderer = nullptr;
GameInputHandler* g_input_handler = nullptr;
GameStateManager* g_state_manager = nullptr;
GameAnimationManager* g_animation_manager = nullptr;

// 控制台输出函数
void logPrint(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

int main() {
    // 初始化Pico SDK
    stdio_init_all();
    
    // 初始化板载LED（关闭，由霍尔摇杆控制）
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);  // 关闭，避免与霍尔摇杆LED冲突
    
    // 等待串口稳定
    sleep_ms(2000);
    
    logPrint("========================================");
    logPrint("=== 2048游戏 - 霍尔摇杆版本 ===");
    logPrint("========================================");
    logPrint("编译时选择的显示驱动: %s", CURRENT_DISPLAY_DRIVER);
    logPrint("显示分辨率: %dx%d", DISPLAY_WIDTH, DISPLAY_HEIGHT);
    logPrint("颜色深度: %d", DISPLAY_COLOR_DEPTH);
    logPrint("是否彩色: %s", DISPLAY_IS_COLOR ? "是" : "否");
    logPrint("是否灰度: %s", DISPLAY_IS_GRAYSCALE ? "是" : "否");
    logPrint("霍尔摇杆连接: X轴=GP26, Y轴=GP27, 开关=GP22");
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
    
    // 创建UI渲染器
    g_ui_renderer = new GameUIRenderer(g_display);
    
    // 设置颜色方案
    ColorScheme color_scheme = ColorSchemeManager::getCurrentColorScheme();
    g_ui_renderer->setColorScheme(color_scheme);
    
    // 计算自适应布局
    g_ui_renderer->calculateLayout();
    
    const LayoutParams& layout = g_ui_renderer->getLayoutParams();
    logPrint("自适应布局计算完成:");
    logPrint("  显示分辨率: %dx%d", g_display->getWidth(), g_display->getHeight());
    logPrint("  棋盘尺寸: %dx%d", layout.board_size_pixels, layout.board_size_pixels);
    logPrint("  格子尺寸: %d", layout.cell_size);
    logPrint("  棋盘位置: (%d, %d)", layout.board_start_x, layout.board_start_y);
    
    // 初始化霍尔摇杆硬件
    logPrint("正在初始化霍尔摇杆...");
    g_hall_joystick = new HallJoystickDirect();
    if (!g_hall_joystick->begin()) {
        logPrint("错误: 霍尔摇杆硬件初始化失败!");
        return -1;
    }
    
    // 初始化摇杆处理器
    g_joystick_handler = new input::JoystickHandler();
    g_joystick_handler->setDeadzone(200);  // 霍尔摇杆适中的死区设置
    g_joystick_handler->setDirectionRatio(1.0f);  // 调整方向比例为1.0，提高响应性
    
    // 设置旋转角度（无旋转，原始方向）
    g_joystick_handler->setRotation(input::JoystickRotation::ROTATION_180);
    
    // 启用调试模式（可选）
    g_joystick_handler->setDebugMode(false);
    
    if (!g_joystick_handler->initialize(g_hall_joystick)) {
        logPrint("错误: 摇杆处理器初始化失败!");
        return -1;
    }
    
    logPrint("霍尔摇杆初始化成功!");
    logPrint("死区设置: %d", g_joystick_handler->getDeadzone());
    logPrint("方向比例: %.2f", g_joystick_handler->getDirectionRatio());
    logPrint("旋转角度: %s (原始方向)", g_joystick_handler->getRotationString(g_joystick_handler->getRotation()).c_str());
    
    // 创建输入处理器
    g_input_handler = new GameInputHandler(g_joystick_handler);
    
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
    
    // 创建动画管理器
    g_animation_manager = new GameAnimationManager(g_display);
    
    // 根据显示驱动设置显示类型
#ifdef DISPLAY_DRIVER_ST7306
    g_animation_manager->setDisplayType(DisplayType::ST7306);
#else
    g_animation_manager->setDisplayType(DisplayType::ILI9488);
#endif
    
    // 设置动画区域（底部提示区域上方）
    uint16_t anim_x = 10;
    uint16_t anim_y = g_display->getHeight() - 70; // 底部提示区域上方，向上提高10像素
    uint16_t anim_width = g_display->getWidth() - 20;
    uint16_t anim_height = 20; // 动画区域高度
    g_animation_manager->setAnimationArea(anim_x, anim_y, anim_width, anim_height);
    
    // 连接动画管理器到UI渲染器
    g_ui_renderer->setAnimationManager(g_animation_manager);
    
    // 创建状态管理器
    g_state_manager = new GameStateManager(g_game, g_ui_renderer);
    g_state_manager->setState(LocalGameState::PLAYING);
    g_state_manager->setAnimationManager(g_animation_manager);
    
    // 绘制初始界面
    logPrint("绘制游戏界面...");
    g_display->clear(color_scheme.background);
    g_ui_renderer->drawBoardOptimized(game_state);
    g_ui_renderer->drawStatusBarOptimized(game_state);
    g_ui_renderer->drawControlHint();
    g_display->display(); // 刷新显示
    
    // 开始游戏开始动画
    g_animation_manager->startGameStartAnimation();
    
    logPrint("游戏已就绪! 使用霍尔摇杆控制游戏...");
    logPrint("操作说明:");
    logPrint("  - 移动摇杆: 控制方块移动方向 (原始方向，无旋转)");
    logPrint("  - 按下摇杆: 重置游戏");
    logPrint("  - LED指示: 摇杆操作时亮起");
    
    // 主游戏循环
    while (true) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        switch (g_state_manager->getCurrentState()) {
            case LocalGameState::PLAYING:
                // 处理摇杆输入
                g_input_handler->handleInput(logic, game_state);
                
                // 检查是否有重置请求
                if (g_input_handler->isResetRequested()) {
                    g_state_manager->resetGame();
                    
                    // 重绘整个界面
                    g_display->clear(color_scheme.background);
                    g_ui_renderer->drawBoardOptimized(game_state);
                    g_ui_renderer->drawStatusBarOptimized(game_state);
                    g_ui_renderer->drawControlHint();
                    g_display->display(); // 刷新显示
                }
                
                // 检查是否有移动
                if (g_input_handler->getLastInputTime() > 0) {
                    // 重绘棋盘和状态栏
                    g_ui_renderer->drawBoardOptimized(game_state);
                    g_ui_renderer->drawStatusBarOptimized(game_state);
                    g_display->display(); // 刷新显示
                }
                
                // 检查游戏状态
                g_state_manager->checkGameStatus(logic, game_state);
                
                // 更新动画
                if (g_animation_manager) {
                    g_animation_manager->update();
                    if (g_animation_manager->isPlaying()) {
                        g_display->display(); // 动画播放时刷新显示
                    }
                }
                
                // LED由霍尔摇杆控制，无需手动控制
                break;
                
            case LocalGameState::WON:
            case LocalGameState::GAME_OVER:
                // 等待按钮输入重新开始
                g_input_handler->handleInput(logic, game_state);
                
                // 检查是否按下重置按钮
                if (g_joystick_handler && g_joystick_handler->isButtonPressed()) {
                    g_state_manager->resetGame();
                    
                    // 重绘整个界面
                    g_display->clear(color_scheme.background);
                    g_ui_renderer->drawBoardOptimized(game_state);
                    g_ui_renderer->drawStatusBarOptimized(game_state);
                    g_ui_renderer->drawControlHint();
                    g_display->display(); // 刷新显示
                }
                break;
                
            default:
                break;
        }
        
        sleep_ms(GameConfig::GAME_UPDATE_DELAY_MS);
    }
    
    return 0;
}
