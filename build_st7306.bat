@echo off
echo ========================================
echo 构建ST7306版本的2048游戏
echo ========================================

REM 清理构建目录
if exist build rmdir /s /q build
mkdir build
cd build

REM 配置CMake - 使用ST7306显示驱动
echo 配置CMake (ST7306)...
cmake -DUSE_ST7306_DISPLAY=ON -DUSE_ILI9488_DISPLAY=OFF ..

REM 构建游戏程序
echo 构建游戏程序...
cmake --build . --config Debug --target game2048_unified

echo.
echo ========================================
echo 构建完成！
echo 生成的文件:
echo   - game2048_unified.uf2 (ST7306版本)
echo ========================================
echo.
echo 请将 game2048_unified.uf2 复制到Pico进行测试
echo.

pause
