@echo off
chcp 65001 >nul
echo ========================================
echo 2048游戏项目构建脚本
echo ========================================
echo.

rem 设置Pico SDK路径
set PICO_SDK_PATH=C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\pico-sdk
echo 使用Pico SDK路径: %PICO_SDK_PATH%

rem 检查Pico SDK路径是否存在
if not exist "%PICO_SDK_PATH%" (
    echo 错误: Pico SDK路径不存在!
    echo 请检查路径: %PICO_SDK_PATH%
    pause
    exit /b 1
)

rem 检查pico_sdk_import.cmake文件
if not exist "%PICO_SDK_PATH%\external\pico_sdk_import.cmake" (
    echo 错误: 找不到pico_sdk_import.cmake文件!
    echo 请检查Pico SDK安装是否完整
    pause
    exit /b 1
)

echo Pico SDK配置检查通过
echo.

rem 清理旧的构建文件
if exist build (
    echo 清理旧的构建文件...
    rd /s /q build
)

rem 创建构建目录
echo 创建构建目录...
mkdir build
cd build

rem 配置项目
echo 配置项目...
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
if %ERRORLEVEL% neq 0 (
    echo 错误: CMake配置失败!
    cd ..
    pause
    exit /b 1
)

rem 构建项目
echo.
echo 构建项目...
ninja -j4
if %ERRORLEVEL% neq 0 (
    echo 错误: 构建失败!
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo 构建成功!
echo ========================================
echo.

rem 检查生成的文件
if exist "hardware_test.uf2" (
    echo ✓ hardware_test.uf2 - 硬件测试固件
) else (
    echo ✗ hardware_test.uf2 - 未生成
)

if exist "game2048_main.uf2" (
    echo ✓ game2048_main.uf2 - 游戏主程序固件
) else (
    echo ✗ game2048_main.uf2 - 未生成
)

echo.
echo 烧录说明:
echo 1. 按住BOOTSEL按钮并插入Pico
echo 2. 将hardware_test.uf2拖拽到RPI-RP2驱动器
echo 3. 等待烧录完成，Pico会自动重启
echo 4. 通过串口监视器查看测试输出 (115200波特率)
echo.

cd ..
echo 构建脚本执行完成!
