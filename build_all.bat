@echo off
echo ========================================
echo 编译所有版本的 2048 游戏
echo ========================================

REM 确保在正确的目录中
if not exist "CMakeLists.txt" (
    echo 错误: 请在项目根目录中运行此脚本!
    echo 当前目录: %CD%
    pause
    exit /b 1
)

REM 创建输出目录
if not exist "firmware" mkdir firmware

echo.
echo ========================================
echo 1. 编译 ST7306 版本
echo ========================================

REM 创建并进入 build 目录
if not exist "build" mkdir build
cd build

REM 配置 ST7306 版本
echo 正在配置 ST7306 版本...
cmake -DUSE_ST7306_DISPLAY=ON -DUSE_ILI9488_DISPLAY=OFF ..

if %ERRORLEVEL% neq 0 (
    echo 错误: ST7306 版本 CMake 配置失败!
    pause
    exit /b 1
)

REM 编译 ST7306 版本
echo 正在编译 ST7306 版本...
cmake --build . --target game2048_unified

if %ERRORLEVEL% neq 0 (
    echo 错误: ST7306 版本编译失败!
    pause
    exit /b 1
)

REM 复制 ST7306 固件
if exist "game2048_unified.uf2" (
    copy "game2048_unified.uf2" "..\firmware\game2048_unified_st7306.uf2"
    echo ST7306 版本编译完成: firmware\game2048_unified_st7306.uf2
) else (
    echo 警告: 未找到 ST7306 固件文件
)

echo.
echo ========================================
echo 2. 编译 ILI9488 版本
echo ========================================

REM 配置 ILI9488 版本
echo 正在配置 ILI9488 版本...
cmake -DUSE_ST7306_DISPLAY=OFF -DUSE_ILI9488_DISPLAY=ON ..

if %ERRORLEVEL% neq 0 (
    echo 错误: ILI9488 版本 CMake 配置失败!
    pause
    exit /b 1
)

REM 编译 ILI9488 版本
echo 正在编译 ILI9488 版本...
cmake --build . --target game2048_unified

if %ERRORLEVEL% neq 0 (
    echo 错误: ILI9488 版本编译失败!
    pause
    exit /b 1
)

REM 复制 ILI9488 固件
if exist "game2048_unified.uf2" (
    copy "game2048_unified.uf2" "..\firmware\game2048_unified_ili9488.uf2"
    echo ILI9488 版本编译完成: firmware\game2048_unified_ili9488.uf2
) else (
    echo 警告: 未找到 ILI9488 固件文件
)

REM 返回上级目录
cd ..

echo.
echo ========================================
echo 编译完成!
echo ========================================
echo 生成的固件文件:
if exist "firmware\game2048_unified_st7306.uf2" (
    echo   - firmware\game2048_unified_st7306.uf2 (ST7306版本)
)
if exist "firmware\game2048_unified_ili9488.uf2" (
    echo   - firmware\game2048_unified_ili9488.uf2 (ILI9488版本)
)
echo.
echo 请将对应的固件文件复制到 Pico 的 BOOTSEL 模式中
echo ========================================

