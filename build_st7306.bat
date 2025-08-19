@echo off
echo ========================================
echo 编译 ST7306 版本的 2048 游戏
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
echo 正在配置 ST7306 版本...
cmake %CMAKE_OPTIONS% ..

if %ERRORLEVEL% neq 0 (
    echo 错误: CMake 配置失败!
    pause
    exit /b 1
)

REM 编译项目
echo 正在编译 ST7306 版本...
cmake --build . --target game2048_unified

if %ERRORLEVEL% neq 0 (
    echo 错误: 编译失败!
    pause
    exit /b 1
)

echo.
echo ========================================
echo ST7306 版本编译完成!
echo ========================================
echo 生成的固件文件: build\game2048_unified.uf2
echo 请将此文件复制到 Pico 的 BOOTSEL 模式中
echo ========================================
cd ..


