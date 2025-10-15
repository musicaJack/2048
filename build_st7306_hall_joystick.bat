@echo off
chcp 65001 >nul
echo ========================================
echo 编译 ST7306 + 霍尔摇杆版本的 2048 游戏
echo ========================================

REM 确保在正确的目录中
if not exist "CMakeLists.txt" (
    echo 错误: 请在项目根目录中运行此脚本!
    echo 当前目录: %CD%
    pause
    exit /b 1
)

REM 创建并进入 build 目录
if not exist "build" mkdir build
cd build

REM 设置编译选项
set CMAKE_OPTIONS=-DUSE_ST7306_DISPLAY=ON -DUSE_ILI9488_DISPLAY=OFF

REM 配置项目
echo 正在配置 ST7306 + 霍尔摇杆版本...
cmake %CMAKE_OPTIONS% ..

if %ERRORLEVEL% neq 0 (
    echo 错误: CMake 配置失败!
    pause
    exit /b 1
)

REM 编译霍尔摇杆版本
echo 正在编译 ST7306 + 霍尔摇杆版本...
cmake --build . --target game2048_hall_joystick

if %ERRORLEVEL% neq 0 (
    echo 错误: 编译失败!
    pause
    exit /b 1
)

echo.
echo ========================================
echo ST7306 + 霍尔摇杆版本编译完成!
echo ========================================
echo 生成的固件文件: build\game2048_hall_joystick.uf2
echo 请将此文件复制到 Pico 的 BOOTSEL 模式中
echo.
echo 硬件连接:
echo   X轴: GP26 (ADC0)
echo   Y轴: GP27 (ADC1)
echo   开关: GP22
echo   LED: GP25 (PICO主板自带LED)
echo ========================================
cd ..
