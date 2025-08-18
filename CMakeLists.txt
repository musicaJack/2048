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

# Initialize the Pico SDK
pico_sdk_init()

# 创建硬件驱动库
add_library(hardware_drivers
    src/hardware/display/ili9488_driver.cpp
    src/hardware/display/ili9488_hal.cpp
    src/hardware/display/ili9488_ui.cpp
    src/hardware/display/ili9488_font.cpp
    src/hardware/display/flash_font_cache.cpp
    src/hardware/display/hybrid_font_system.cpp
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

# 创建游戏核心库
add_library(game2048_core
    src/game/game_2048.cpp
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
)

# 设置游戏核心库包含目录
target_include_directories(game2048_core PUBLIC
    include
)

# 链接硬件驱动库
target_link_libraries(game2048_core
    hardware_drivers
)

# 创建硬件测试程序
add_executable(hardware_test examples/hardware_test.cpp)

# 启用C++异常处理
target_compile_options(hardware_test PRIVATE -fexceptions)

# 启用串口输出和调试功能
pico_enable_stdio_usb(hardware_test 1)
pico_enable_stdio_uart(hardware_test 0)

target_link_libraries(hardware_test 
    hardware_drivers
    pico_stdlib
)

# 创建游戏主程序 (暂时注释掉)
# add_executable(game2048_main src/main.cpp)
# target_link_libraries(game2048_main 
#     game2048_core
#     hardware_drivers
# )

# 创建游戏演示程序
add_executable(game2048_demo examples/game_2048_demo.cpp)

# 启用C++异常处理
target_compile_options(game2048_demo PRIVATE -fexceptions)

# 启用串口输出和调试功能
pico_enable_stdio_usb(game2048_demo 1)
pico_enable_stdio_uart(game2048_demo 0)

target_link_libraries(game2048_demo 
    game2048_core
    hardware_drivers
    pico_stdlib
)

# Generate UF2 output
pico_add_extra_outputs(game2048_demo)

# 创建横屏2048游戏程序
add_executable(game2048_landscape examples/game_2048_landscape.cpp)

# 启用C++异常处理
target_compile_options(game2048_landscape PRIVATE -fexceptions)

# 启用串口输出和调试功能
pico_enable_stdio_usb(game2048_landscape 1)
pico_enable_stdio_uart(game2048_landscape 0)

target_link_libraries(game2048_landscape 
    game2048_core
    hardware_drivers
    pico_stdlib
)

# Generate UF2 output
pico_add_extra_outputs(game2048_landscape)

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

# 设置输出目录
set_target_properties(hardware_test PROPERTIES
    OUTPUT_NAME "hardware_test"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# 添加自定义目标
add_custom_target(flash_hardware_test
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hardware_test> ${CMAKE_BINARY_DIR}/hardware_test.uf2
    DEPENDS hardware_test
    COMMENT "Copying hardware_test.uf2 for flashing"
)

# Generate UF2 output
pico_add_extra_outputs(hardware_test)

# 打印构建信息
message(STATUS "Game2048 Project Configuration:")
message(STATUS "  - Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  - Pico SDK Path: $ENV{PICO_SDK_PATH}")
message(STATUS "  - Output Directory: ${CMAKE_BINARY_DIR}")
message(STATUS "")
message(STATUS "Available Targets:")
message(STATUS "  - hardware_test: 硬件测试程序")
message(STATUS "  - flash_hardware_test: 复制硬件测试固件") 
