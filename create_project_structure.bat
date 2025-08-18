@echo off
chcp 65001 >nul
echo ========================================
echo 2048游戏项目目录结构生成脚本
echo ========================================
echo.

:: 检查ILI9488_Pico目录是否存在
if not exist "ILI9488_Pico" (
    echo 错误：找不到ILI9488_Pico目录！
    echo 请确保ILI9488_Pico目录在当前目录下。
    pause
    exit /b 1
)

echo 开始创建项目目录结构...

:: 创建主目录结构
echo 创建主目录...
mkdir "include" 2>nul
mkdir "src" 2>nul
mkdir "examples" 2>nul
mkdir "tests" 2>nul
mkdir "assets" 2>nul
mkdir "docs" 2>nul
mkdir "scripts" 2>nul
mkdir "build" 2>nul

:: 创建include子目录
echo 创建include子目录...
mkdir "include\game" 2>nul
mkdir "include\ui" 2>nul
mkdir "include\input" 2>nul
mkdir "include\utils" 2>nul
mkdir "include\hardware" 2>nul
mkdir "include\hardware\display" 2>nul
mkdir "include\hardware\input" 2>nul
mkdir "include\hardware\system" 2>nul
mkdir "include\hardware\common" 2>nul
mkdir "include\common" 2>nul

:: 创建src子目录
echo 创建src子目录...
mkdir "src\game" 2>nul
mkdir "src\ui" 2>nul
mkdir "src\input" 2>nul
mkdir "src\utils" 2>nul
mkdir "src\hardware" 2>nul
mkdir "src\hardware\display" 2>nul
mkdir "src\hardware\input" 2>nul
mkdir "src\hardware\system" 2>nul
mkdir "src\hardware\common" 2>nul

:: 创建assets子目录
echo 创建assets子目录...
mkdir "assets\fonts" 2>nul
mkdir "assets\images" 2>nul
mkdir "assets\configs" 2>nul

:: 创建build子目录
echo 创建build子目录...
mkdir "build\debug" 2>nul
mkdir "build\release" 2>nul
mkdir "build\test" 2>nul

echo 目录结构创建完成！
echo.

echo 开始复制ILI9488_Pico源代码...

:: 复制ILI9488_Pico的include文件到hardware/display
echo 复制显示屏驱动头文件...
copy "ILI9488_Pico\include\ili9488_driver.hpp" "include\hardware\display\" >nul
copy "ILI9488_Pico\include\ili9488_hal.hpp" "include\hardware\display\" >nul
copy "ILI9488_Pico\include\ili9488_ui.hpp" "include\hardware\display\" >nul
copy "ILI9488_Pico\include\ili9488_colors.hpp" "include\hardware\display\" >nul
copy "ILI9488_Pico\include\ili9488_font.hpp" "include\hardware\display\" >nul
copy "ILI9488_Pico\include\pico_ili9488_gfx.hpp" "include\hardware\display\" >nul
copy "ILI9488_Pico\include\pico_ili9488_gfx.inl" "include\hardware\display\" >nul

:: 复制字体相关文件
echo 复制字体系统文件...
copy "ILI9488_Pico\include\fonts\*.*" "include\hardware\display\" >nul 2>nul

:: 复制摇杆相关文件
echo 复制摇杆驱动文件...
copy "ILI9488_Pico\include\joystick\joystick.hpp" "include\hardware\input\" >nul
copy "ILI9488_Pico\include\pin_config.hpp" "include\hardware\system\" >nul

:: 复制ILI9488_Pico的src文件
echo 复制显示屏驱动源文件...
copy "ILI9488_Pico\src\ili9488_driver.cpp" "src\hardware\display\" >nul
copy "ILI9488_Pico\src\ili9488_ui.cpp" "src\hardware\display\" >nul
copy "ILI9488_Pico\src\hal\ili9488_hal.cpp" "src\hardware\display\" >nul
copy "ILI9488_Pico\src\fonts\ili9488_font.cpp" "src\hardware\display\" >nul
copy "ILI9488_Pico\src\fonts\flash_font_cache.cpp" "src\hardware\display\" >nul
copy "ILI9488_Pico\src\fonts\hybrid_font_system.cpp" "src\hardware\display\" >nul

:: 复制摇杆源文件
echo 复制摇杆驱动源文件...
copy "ILI9488_Pico\src\joystick\joystick.cpp" "src\hardware\input\" >nul

echo 源代码复制完成！
echo.

echo 开始创建空文件...

:: 创建游戏核心模块头文件
echo 创建游戏核心模块头文件...
echo. > "include\game\game_2048.hpp"
echo. > "include\game\game_board.hpp"
echo. > "include\game\game_logic.hpp"
echo. > "include\game\game_state.hpp"
echo. > "include\game\game_config.hpp"

:: 创建UI模块头文件
echo 创建UI模块头文件...
echo. > "include\ui\game_ui.hpp"
echo. > "include\ui\tile_renderer.hpp"
echo. > "include\ui\animation_manager.hpp"
echo. > "include\ui\ui_layout.hpp"
echo. > "include\ui\ui_colors.hpp"

:: 创建输入模块头文件
echo 创建输入模块头文件...
echo. > "include\input\input_manager.hpp"
echo. > "include\input\joystick_handler.hpp"
echo. > "include\input\input_config.hpp"

:: 创建工具模块头文件
echo 创建工具模块头文件...
echo. > "include\utils\random_generator.hpp"
echo. > "include\utils\score_manager.hpp"
echo. > "include\utils\game_timer.hpp"
echo. > "include\utils\debug_logger.hpp"

:: 创建硬件模块头文件
echo 创建硬件模块头文件...
echo. > "include\hardware\input\joystick_hal.hpp"
echo. > "include\hardware\input\input_device.hpp"
echo. > "include\hardware\system\spi_manager.hpp"
echo. > "include\hardware\system\i2c_manager.hpp"
echo. > "include\hardware\system\system_clock.hpp"
echo. > "include\hardware\common\hardware_types.hpp"
echo. > "include\hardware\common\dma_manager.hpp"
echo. > "include\hardware\common\interrupt_handler.hpp"

:: 创建公共模块头文件
echo 创建公共模块头文件...
echo. > "include\common\types.hpp"
echo. > "include\common\constants.hpp"
echo. > "include\common\macros.hpp"

:: 创建源文件
echo 创建源文件...
echo. > "src\game\game_2048.cpp"
echo. > "src\game\game_board.cpp"
echo. > "src\game\game_logic.cpp"
echo. > "src\game\game_state.cpp"
echo. > "src\ui\game_ui.cpp"
echo. > "src\ui\tile_renderer.cpp"
echo. > "src\ui\animation_manager.cpp"
echo. > "src\ui\ui_layout.cpp"
echo. > "src\input\input_manager.cpp"
echo. > "src\input\joystick_handler.cpp"
echo. > "src\utils\random_generator.cpp"
echo. > "src\utils\score_manager.cpp"
echo. > "src\utils\game_timer.cpp"
echo. > "src\utils\debug_logger.cpp"
echo. > "src\hardware\display\pico_ili9488_gfx.cpp"
echo. > "src\hardware\input\joystick_hal.cpp"
echo. > "src\hardware\system\spi_manager.cpp"
echo. > "src\hardware\system\i2c_manager.cpp"
echo. > "src\hardware\system\system_clock.cpp"
echo. > "src\hardware\common\dma_manager.cpp"
echo. > "src\hardware\common\interrupt_handler.cpp"
echo. > "src\main.cpp"

:: 创建示例文件
echo 创建示例文件...
echo. > "examples\game_2048_demo.cpp"
echo. > "examples\display_test.cpp"
echo. > "examples\joystick_test.cpp"
echo. > "examples\hardware_test.cpp"

:: 创建测试文件
echo 创建测试文件...
echo. > "tests\game_logic_test.cpp"
echo. > "tests\ui_render_test.cpp"
echo. > "tests\input_test.cpp"
echo. > "tests\hardware_test.cpp"

:: 创建构建文件
echo 创建构建文件...
echo. > "CMakeLists.txt"
echo. > "build_pico.bat"

:: 创建文档文件
echo 创建文档文件...
echo. > "README.md"
echo. > "README.zh.md"
echo. > "docs\architecture.md"
echo. > "docs\hardware_guide.md"
echo. > "docs\api_reference.md"
echo. > "docs\development_guide.md"

:: 创建脚本文件
echo 创建脚本文件...
echo. > "scripts\build.sh"
echo. > "scripts\flash.sh"
echo. > "scripts\test.sh"

:: 创建资源文件
echo 创建资源文件...
echo. > "assets\fonts\font_8x16.bin"
echo. > "assets\fonts\font_12x24.bin"
echo. > "assets\images\game_icons.bin"
echo. > "assets\images\ui_elements.bin"
echo. > "assets\configs\hardware_config.json"
echo. > "assets\configs\game_config.json"

echo 空文件创建完成！
echo.

echo 开始修改复制的头文件包含路径...

:: 修改ili9488_driver.hpp的包含路径
echo 修改ili9488_driver.hpp...
powershell -Command "(Get-Content 'include\hardware\display\ili9488_driver.hpp') -replace '#include \"ili9488_hal.hpp\"', '#include \"ili9488_hal.hpp\"' | Set-Content 'include\hardware\display\ili9488_driver.hpp'"

:: 修改pico_ili9488_gfx.hpp的包含路径
echo 修改pico_ili9488_gfx.hpp...
powershell -Command "(Get-Content 'include\hardware\display\pico_ili9488_gfx.hpp') -replace '#include \"ili9488_ui.hpp\"', '#include \"ili9488_ui.hpp\"' | Set-Content 'include\hardware\display\pico_ili9488_gfx.hpp'"

echo 包含路径修改完成！
echo.

echo ========================================
echo 项目结构创建完成！
echo ========================================
echo.
echo 已创建以下内容：
echo - 完整的目录结构
echo - 所有必要的空文件
echo - 复制的ILI9488_Pico源代码
echo - 修改的包含路径
echo.
echo 下一步：
echo 1. 检查复制的文件是否正确
echo 2. 根据需要调整包含路径
echo 3. 开始编写游戏代码
echo.
pause
