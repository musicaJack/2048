cmake_minimum_required(VERSION 3.13)

# Pull in Raspberry Pi Pico SDK (must be defined before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# 设置项目信息
project(Game2048 VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 显示驱动选择选项
option(USE_ST7306_DISPLAY "使用ST7306反射式显示器" OFF)
option(USE_ILI9488_DISPLAY "使用ILI9488彩色显示器" ON)

# 确保只选择一个显示驱动
if(USE_ST7306_DISPLAY AND USE_ILI9488_DISPLAY)
    message(FATAL_ERROR "不能同时选择两个显示驱动！请只选择一个。")
endif()

if(NOT USE_ST7306_DISPLAY AND NOT USE_ILI9488_DISPLAY)
    message(STATUS "未选择显示驱动，默认使用ST7306")
    set(USE_ST7306_DISPLAY ON)
endif()

# 设置编译时宏定义
if(USE_ST7306_DISPLAY)
    add_compile_definitions(DISPLAY_DRIVER_ST7306=1)
    message(STATUS "选择的显示驱动: ST7306 (反射式显示器)")
elseif(USE_ILI9488_DISPLAY)
    add_compile_definitions(DISPLAY_DRIVER_ILI9488=1)
    message(STATUS "选择的显示驱动: ILI9488 (彩色显示器)")
endif()

# Initialize the Pico SDK
pico_sdk_init()

# 创建硬件驱动库 - 更新为新的目录结构
add_library(hardware_drivers
    # ILI9488驱动文件
    src/hardware/display/ili9488/ili9488_driver.cpp
    src/hardware/display/ili9488/ili9488_hal.cpp
    src/hardware/display/ili9488/ili9488_ui.cpp
    src/hardware/display/ili9488/ili9488_font.cpp
    src/hardware/display/ili9488/flash_font_cache.cpp
    src/hardware/display/ili9488/hybrid_font_system.cpp
    # 摇杆驱动文件
    src/hardware/input/joystick.cpp
)

# 设置硬件驱动库包含目录
target_include_directories(hardware_drivers PUBLIC
    include
)

# 链接Pico SDK库
target_link_libraries(hardware_drivers
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_adc
    hardware_dma
    hardware_pwm
)

# 创建配置管理库
add_library(config_manager
    src/config/config_manager.cpp
)

# 设置配置管理库包含目录
target_include_directories(config_manager PUBLIC
    include
)

# 创建显示管理库
add_library(display_manager
    src/display/display_manager.cpp
    src/display/adapters/st7306_adapter.cpp
    src/display/adapters/ili9488_adapter.cpp
    src/hardware/display/st7306/st7306_driver.cpp
    src/hardware/display/st7306/st73xx_ui.cpp
    src/hardware/display/st7306/st73xx_font.cpp
    src/hardware/display/ili9488/ili9488_driver.cpp
    src/hardware/display/ili9488/ili9488_hal.cpp
    src/hardware/display/ili9488/ili9488_ui.cpp
    src/hardware/display/ili9488/ili9488_font.cpp
    src/hardware/display/ili9488/flash_font_cache.cpp
    src/hardware/display/ili9488/hybrid_font_system.cpp
)

# 设置显示管理库包含目录
target_include_directories(display_manager PUBLIC
    include
)

# 链接配置管理库和Pico SDK库
target_link_libraries(display_manager
    config_manager
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_pwm
)

# 创建游戏核心库
add_library(game2048_core
    src/game/game_2048.cpp
    src/game/game_config.cpp
    src/game/game_ui.cpp
    src/game/game_input.cpp
    src/game/game_state_manager.cpp
    src/game/game_board.cpp
    src/game/game_logic.cpp
    src/game/game_state.cpp
    src/input/input_manager.cpp
    src/input/joystick_handler.cpp
    src/utils/random_generator.cpp
    src/utils/score_manager.cpp
    src/utils/game_timer.cpp
    src/utils/debug_logger.cpp
    src/utils/config_loader.cpp
    src/ui/unified_game_ui.cpp
)

# 设置游戏核心库包含目录
target_include_directories(game2048_core PUBLIC
    include
)

# 链接硬件驱动库和显示管理库
target_link_libraries(game2048_core
    hardware_drivers
    display_manager
    config_manager
)



# 创建游戏主程序 (暂时注释掉)
# add_executable(game2048_main src/main.cpp)
# target_link_libraries(game2048_main 
#     game2048_core
#     hardware_drivers
# )

# 创建游戏演示程序 (暂时注释掉，文件不存在)
# add_executable(game2048_demo examples/game_2048_demo.cpp)
# target_compile_options(game2048_demo PRIVATE -fexceptions)
# pico_enable_stdio_usb(game2048_demo 1)
# pico_enable_stdio_uart(game2048_demo 0)

# target_link_libraries(game2048_demo 
#     game2048_core
#     hardware_drivers
#     pico_stdlib
# )

# Generate UF2 output
# pico_add_extra_outputs(game2048_demo)

# 创建竖屏2048游戏程序
add_executable(game2048_portrait examples/game_2048_portrait.cpp)

# 启用C++异常处理
target_compile_options(game2048_portrait PRIVATE -fexceptions)

# 启用串口输出和调试功能
pico_enable_stdio_usb(game2048_portrait 1)
pico_enable_stdio_uart(game2048_portrait 0)

target_link_libraries(game2048_portrait 
    game2048_core
    hardware_drivers
    pico_stdlib
)

# Generate UF2 output
pico_add_extra_outputs(game2048_portrait)

# add_executable(display_test examples/display_test.cpp)
# target_link_libraries(display_test hardware_drivers)

# add_executable(joystick_test examples/joystick_test.cpp)
# target_link_libraries(joystick_test hardware_drivers)

# add_executable(hardware_test_example examples/hardware_test.cpp)
# target_link_libraries(hardware_test_example hardware_drivers)

# 创建测试程序 (暂时注释掉)
# add_executable(game_logic_test tests/game_logic_test.cpp)
# target_link_libraries(game_logic_test game2048_core)

# add_executable(ui_render_test tests/ui_render_test.cpp)
# target_link_libraries(ui_render_test game2048_core)

# add_executable(input_test tests/input_test.cpp)
# target_link_libraries(input_test game2048_core)

# add_executable(hardware_unit_test tests/hardware_test.cpp)
# target_link_libraries(hardware_unit_test hardware_drivers)







# 创建统一2048游戏程序（最终版本）
add_executable(game2048_unified examples/game_2048_unified.cpp)
target_compile_options(game2048_unified PRIVATE -fexceptions)
pico_enable_stdio_usb(game2048_unified 1)
pico_enable_stdio_uart(game2048_unified 0)
target_link_libraries(game2048_unified
    game2048_core
    config_manager
    display_manager
    pico_stdlib
)
pico_add_extra_outputs(game2048_unified)



# 打印构建信息
message(STATUS "Game2048 Project Configuration:")
message(STATUS "  - Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  - Pico SDK Path: $ENV{PICO_SDK_PATH}")
message(STATUS "  - Output Directory: ${CMAKE_BINARY_DIR}")
message(STATUS "")
message(STATUS "Available Targets:")
message(STATUS "  - game2048_unified: 统一2048游戏程序（推荐使用）")
message(STATUS "  - game2048_portrait: 原版竖屏2048游戏程序（仅ILI9488）")
message(STATUS "")
message(STATUS "Build Commands:")
message(STATUS "  - cmake --build . --target game2048_unified")
message(STATUS "  - cmake --build . --target game2048_portrait") 
