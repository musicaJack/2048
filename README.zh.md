# 2048游戏项目 - 硬件测试程序

这是一个基于Raspberry Pi Pico的2048游戏项目，包含完整的硬件测试程序和2048游戏实现。

## 最新更新

### 2024-08-18 - 编译错误修复
- **修复了名称冲突问题**: 解决了 `game_2048_landscape.cpp` 中 `GameState` 枚举与 `game2048::GameState` 类的名称冲突
- **项目编译成功**: 所有示例程序现在可以正常编译
- **新增横屏版本**: 添加了 `game_2048_landscape.cpp` 横屏游戏示例

## 硬件要求

- **Raspberry Pi Pico (RP2040)**
- **ILI9488 3.5寸TFT LCD显示屏** (320×480分辨率)
- **I2C摇杆控制器** (地址: 0x63)

## 硬件连接

### ILI9488显示屏连接
```
Pico GPIO18 -> 显示屏 SCK
Pico GPIO19 -> 显示屏 MOSI
Pico GPIO17 -> 显示屏 CS
Pico GPIO20 -> 显示屏 DC
Pico GPIO15 -> 显示屏 RST
Pico GPIO16 -> 显示屏 BL (背光)
Pico 3.3V  -> 显示屏 VCC
Pico GND   -> 显示屏 GND
```

### I2C摇杆连接
```
Pico GPIO6  -> 摇杆 SDA
Pico GPIO7  -> 摇杆 SCL
Pico 3.3V   -> 摇杆 VCC
Pico GND    -> 摇杆 GND
```

## 构建和烧录

### 环境准备

1. **安装Pico SDK**
   ```bash
   git clone https://github.com/raspberrypi/pico-sdk
   cd pico-sdk
   git submodule update --init
   ```

2. **设置环境变量**
   ```bash
   # Windows
   set PICO_SDK_PATH=C:\path\to\pico-sdk
   
   # Linux/macOS
   export PICO_SDK_PATH=/path/to/pico-sdk
   ```

3. **安装构建工具**
   - CMake 3.13+
   - ARM GCC编译器
   - Visual Studio Code (推荐)

### 构建项目

#### Windows用户
```bash
# 运行构建脚本
build_pico.bat
```

#### Linux/macOS用户
```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 构建硬件测试程序
make hardware_test -j4

# 构建游戏示例程序
make game2048_demo -j4
make game2048_landscape -j4
```

### 烧录固件

1. **按住BOOTSEL按钮并插入Pico**
2. **将生成的固件文件拖拽到RPI-RP2驱动器**
   - `hardware_test.uf2` - 硬件测试程序
   - `game2048_demo.uf2` - 2048游戏演示程序
   - `game2048_landscape.uf2` - 2048游戏横屏版本

## 程序说明

### 硬件测试程序

硬件测试程序用于验证硬件连接和功能是否正常。

#### 测试内容

硬件测试程序 (`hardware_test`) 包含以下测试阶段：

#### 1. 显示屏测试 (0-5秒)
- **颜色填充测试**: 红、绿、蓝、黄、洋红、青色全屏填充
- **文字渲染测试**: 显示显示屏规格信息
- **图形绘制测试**: 矩形、圆形绘制
- **测试图案**: 棋盘格和渐变图案
- **性能测试**: 1000个随机矩形的绘制性能

#### 2. 摇杆测试 (5-15秒)
- **实时数据显示**: X轴、Y轴数值和按钮状态
- **方向判断**: 自动识别摇杆方向
- **可视化显示**: 摇杆位置的可视化表示
- **LED反馈**: 根据操作状态改变摇杆LED颜色

#### 3. 集成测试 (15-25秒)
- **实时交互**: 摇杆操作实时反映在显示屏上
- **可视化摇杆**: 圆形摇杆界面，实时显示位置
- **状态反馈**: 数值、方向、按钮状态的实时显示

#### 4. 测试完成 (25秒后)
- **完成画面**: 绿色背景显示测试完成信息
- **LED指示**: 板载LED闪烁表示测试完成

### 串口输出

通过串口监视器可以查看详细的测试信息：

```
=== 2048游戏硬件测试程序 ===
硬件配置:
- 显示屏: ILI9488 (320x480)
- 摇杆: I2C Joystick (地址: 0x63)
- 平台: Raspberry Pi Pico (RP2040)

阶段1: 显示屏初始化测试
正在初始化ILI9488显示屏...
显示屏初始化成功!
显示屏旋转设置为180度
背光已开启
清屏测试完成

阶段2: 摇杆初始化测试
正在初始化I2C摇杆...
摇杆初始化成功!
摇杆信息:
- 固件版本: 1
- 引导程序版本: 1
- I2C地址: 0x63

=== 硬件初始化完成 ===
开始功能测试...

开始显示屏功能测试...
测试状态: Display Test: Color Fill
测试状态: Display Test: Text Rendering
测试状态: Display Test: Graphics
测试状态: Display Test: Pattern
测试状态: Display Test: Performance

开始摇杆功能测试...
Joystick - X: 0, Y: 0, Button: RELEASED
Joystick - X: 15, Y: -8, Button: RELEASED
Joystick - X: -23, Y: 12, Button: PRESSED

开始集成功能测试...

=== 硬件测试完成 ===
所有硬件功能正常!
可以开始2048游戏开发!
```

### 串口设置

- **波特率**: 115200
- **数据位**: 8
- **停止位**: 1
- **校验位**: 无

### 2048游戏程序

#### game2048_demo
- **描述**: 2048游戏演示程序，竖屏模式
- **屏幕方向**: 竖屏 (320×480)
- **控制方式**: I2C摇杆
- **功能**: 完整的2048游戏逻辑，包含分数记录和游戏状态管理

#### game2048_landscape
- **描述**: 2048游戏横屏版本
- **屏幕方向**: 横屏 (480×320)
- **控制方式**: I2C摇杆
- **功能**: 优化的横屏界面布局，更好的游戏体验
- **特色**: 深蓝色主题，现代化的UI设计

## 故障排除

### 显示屏问题

1. **显示屏不亮**
   - 检查电源连接 (3.3V, GND)
   - 检查背光引脚连接 (GPIO16)
   - 确认SPI引脚连接正确

2. **显示屏显示异常**
   - 检查SPI引脚连接
   - 确认显示屏旋转设置
   - 检查SPI时钟频率

3. **显示屏无响应**
   - 检查复位引脚连接 (GPIO15)
   - 确认CS引脚连接 (GPIO17)
   - 检查DC引脚连接 (GPIO20)

### 摇杆问题

1. **摇杆无响应**
   - 检查I2C引脚连接 (GPIO6, GPIO7)
   - 确认摇杆地址设置 (0x63)
   - 检查电源连接

2. **摇杆数据异常**
   - 检查I2C时钟频率设置
   - 确认摇杆固件版本
   - 检查连接线质量

3. **摇杆LED不亮**
   - 检查摇杆LED连接
   - 确认I2C通信正常
   - 检查摇杆固件

### 编译问题

1. **CMake配置失败**
   - 确认PICO_SDK_PATH环境变量设置
   - 检查Pico SDK版本 (需要1.5.1+)
   - 确认CMake版本 (需要3.13+)

2. **编译错误**
   - 检查C++编译器版本 (需要支持C++17)
   - 确认所有依赖库已安装
   - 检查代码语法错误

## 项目结构

```
2048/
├── include/                 # 头文件
│   ├── hardware/           # 硬件驱动
│   │   ├── display/        # 显示屏驱动
│   │   ├── input/          # 输入设备驱动
│   │   └── system/         # 系统配置
│   ├── game/              # 游戏核心
│   ├── ui/                # 用户界面
│   ├── input/             # 输入处理
│   └── utils/             # 工具模块
├── src/                   # 源代码
├── examples/              # 示例程序
│   ├── game_2048_demo.cpp     # 2048游戏演示
│   ├── game_2048_landscape.cpp # 2048游戏横屏版本
│   └── hardware_test.cpp      # 硬件测试程序
├── tests/                 # 测试程序
├── assets/                # 资源文件
├── build/                 # 构建输出
├── CMakeLists.txt         # 构建配置
├── build_pico.bat         # Windows构建脚本
└── README.md              # 项目说明
```

## 下一步

项目已完成基础功能开发，可以继续优化：

1. **游戏功能增强**: 添加更多游戏模式（如时间挑战、目标分数等）
2. **界面优化**: 改进动画效果和视觉设计
3. **性能优化**: 优化显示刷新率和游戏响应速度
4. **功能扩展**: 添加音效、震动反馈等

## 许可证

MIT License - 详见LICENSE文件

## 贡献

欢迎提交Issue和Pull Request来改进项目！ 
