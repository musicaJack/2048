# 2048游戏项目 - 双显示驱动支持

这是一个基于Raspberry Pi Pico的2048游戏项目，支持ST7306反射式LCD和ILI9488彩色TFT两种显示屏。

## 最新更新

### 2026-11-17 - RP2350 上的 ST7306

ST7306 版可以在 RP2350 上玩。这块板上的摇杆是模拟量：X 接 GPIO26，Y 接 GPIO27，按键接 GPIO22，按下为低电平。ILI9488 和 ESP32 仍用 I2C 摇杆（GPIO6、GPIO7，地址 0x63）。

RP2350 要单独建一个目录，并加上 `-DPICO_BOARD=pico2`。`build-st7306/` 里的 UF2 是 RP2040 的，拷到 RP2350 上引导程序会直接忽略。编译和烧录见 `docs/Pico开发环境配置手册.md`。

### 2026-11-12 - ESP32-S3-Pico

ILI9488 版已跑在微雪 ESP32-S3-Pico 上。工程在 `esp32/`，和 Pico 的 CMake 分开。画面是黑底、红色格子框，数字用经典方块颜色，没有方块底色。引脚、烧录口和画面见 `docs/ESP32-S3迁移手册.md`。ST7306 仍只有 Pico 的 UF2。

计分改为经典 2048：开局 0 分，只在合并时加上新牌的数值。

### 2024-12-19 - v2.0 重大重构
- **双显示驱动支持**: 新增对ST7306（4级灰度）和ILI9488（彩色）显示屏的支持
- **统一游戏架构**: 创建模块化游戏架构，分离UI、输入、状态和动画管理器
- **高级动画系统**: 为ILI9488实现游戏开始动画、成就通知和星星奖励系统
- **代码重组**: 将配置和功能实现分离到适当的include和src目录
- **构建系统优化**: 清理CMakeLists.txt并移除测试程序
- **编译脚本**: 添加批处理脚本，方便编译两种显示版本

## 硬件要求

### 方案一：ST7306反射式LCD配置
- **Raspberry Pi Pico（RP2040）或 Pico 2（RP2350）**
- **ST7306反射式LCD显示屏**（300×400，4级灰度）
- **模拟摇杆**：X 为 GPIO26，Y 为 GPIO27，按键为 GPIO22

### 方案二：ILI9488彩色TFT配置
- **Raspberry Pi Pico (RP2040)**
- **ILI9488 3.5寸TFT LCD显示屏** (320×480分辨率，RGB565彩色)
- **I2C摇杆控制器** (地址: 0x63)

## 硬件连接

### ST7306显示屏连接
```
Pico GPIO18 -> 显示屏 SCK
Pico GPIO19 -> 显示屏 MOSI
Pico GPIO17 -> 显示屏 CS
Pico GPIO20 -> 显示屏 DC
Pico GPIO15 -> 显示屏 RST
Pico 3.3V  -> 显示屏 VCC
Pico GND   -> 显示屏 GND
```

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

### ST7306 模拟摇杆
```
Pico GPIO26 -> 摇杆 X（ADC0）
Pico GPIO27 -> 摇杆 Y（ADC1）
Pico GPIO22 -> 摇杆按键（内部上拉，按下为低）
Pico 3.3V   -> 摇杆 VCC
Pico GND    -> 摇杆 GND
```

### I2C摇杆连接（ILI9488 和 ESP32）
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
# 构建ST7306版本
build_st7306.bat

# 构建ILI9488版本
build_ili9488.bat

# 构建两个版本
build_all.bat

# 清理构建目录
clean.bat
```

#### Linux/macOS用户
```bash
# 创建构建目录
mkdir build
cd build

# 配置ST7306版本（RP2040）
cmake .. -DUSE_ST7306_DISPLAY=ON -DCMAKE_BUILD_TYPE=Release

# 回到仓库根目录再配 RP2350，不要复用 build/ 里的缓存
cd ..
cmake -S . -B build-st7306-rp2350 -DUSE_ST7306_DISPLAY=ON -DUSE_ILI9488_DISPLAY=OFF -DPICO_BOARD=pico2
cmake --build build-st7306-rp2350 --target game2048_unified
# 产物是 build-st7306-rp2350/Game2350_st7306.uf2

# 配置ILI9488版本
cmake .. -DUSE_ILI9488_DISPLAY=ON -DCMAKE_BUILD_TYPE=Release

# 构建统一游戏程序
make game2048_unified -j4
```

### 烧录固件

1. **按住BOOTSEL按钮并插入Pico**
2. **将生成的固件文件拖拽到RPI-RP2驱动器**
   - `game2048_unified.uf2` - 统一2048游戏程序（支持两种显示屏）

## 程序说明

### 统一2048游戏程序

`game2048_unified`程序是主游戏应用程序，会根据选择的显示驱动自动适配。

#### ST7306版本特性
- **4级灰度渲染**: 针对反射式LCD特性优化
- **Game2048优化字体**: 为小显示屏设计的自定义字体
- **最小化动画**: 无动画以保持稳定性
- **高效渲染**: 部分刷新和优化的绘制例程

#### ILI9488版本特性
- **全彩色支持**: RGB565彩色调色板，经典2048配色
- **深色主题UI**: 纯黑色背景配蓝黑色状态栏
- **高级动画**: 游戏开始闪烁、成就心跳效果
- **星星成就系统**: 合并方块获得星星（16→32、32→64等）
- **增强视觉反馈**: 动态配色方案和视觉奖励

#### 通用特性
- **完整2048游戏逻辑**: 包含分数跟踪的完整游戏实现
- **摇杆控制**: 方向移动和中心按钮重置
- **自适应布局**: 自动调整到显示屏分辨率
- **模块化架构**: 分离的UI、输入、状态和动画系统

## 游戏控制

- **摇杆方向**: 向对应方向移动方块
- **中心按钮**: 重置游戏（在所有游戏状态下都有效）
- **自动检测**: 游戏自动检测有效移动

## 成就系统（仅ILI9488）

### 星星奖励
- **2+2=4**: 1颗星
- **4+4=8**: 1颗星
- **8+8=16**: 2颗星
- **16+16=32**: 3颗星
- **32+32=64**: 4颗星
- **64+64=128**: 5颗星
- **128+128=256**: 6颗星
- **256+256=512**: 7颗星
- **512+512=1024**: 8颗星
- **1024+1024=2048**: 10颗星

### 成就通知
- **方块值成就**: 达到32、64、128、256、512、1024、2048时的心跳动画
- **视觉反馈**: 颜色编码的成就通知
- **星星显示**: 状态栏显示总星星数

## 项目结构

```
2048/
├── include/                 # 头文件
│   ├── config/             # 配置管理
│   ├── display/            # 显示驱动接口
│   │   └── adapters/       # 显示适配器
│   ├── game/               # 游戏核心模块
│   ├── hardware/           # 硬件驱动
│   │   ├── display/        # 显示驱动
│   │   │   ├── st7306/     # ST7306驱动
│   │   │   └── ili9488/    # ILI9488驱动
│   │   └── input/          # 输入设备驱动
│   ├── input/              # 输入处理
│   ├── ui/                 # 用户界面
│   └── utils/              # 工具模块
├── src/                    # 源代码
│   ├── config/             # 配置实现
│   ├── display/            # 显示管理
│   ├── game/               # 游戏实现
│   ├── hardware/           # 硬件实现
│   ├── input/              # 输入处理
│   ├── ui/                 # UI实现
│   └── utils/              # 工具实现
├── examples/               # 示例程序
│   └── game_2048_unified.cpp # 主统一游戏程序
├── build/                  # 构建输出
├── firmware/               # 生成的固件文件
├── CMakeLists.txt          # 构建配置
├── build_st7306.bat        # ST7306构建脚本
├── build_ili9488.bat       # ILI9488构建脚本
├── build_all.bat           # 构建两个版本
├── clean.bat               # 清理构建目录
└── README.zh.md            # 项目文档
```

## 故障排除

### 显示屏问题

1. **ST7306显示屏问题**
   - 检查SPI连接（GPIO18、19、17、20、15）
   - 验证电源连接（3.3V、GND）
   - 确保正确的初始化序列

2. **ILI9488显示屏问题**
   - 检查所有SPI和控制引脚
   - 验证背光连接（GPIO16）
   - 确认显示屏旋转设置

### 摇杆问题

1. **无响应**
   - ST7306：检查 GPIO26、GPIO27、GPIO22。这块板不走 I2C
   - ILI9488：检查 I2C 连接（GPIO6、GPIO7）和地址 0x63
   - RP2350 上如果画面正常但摇杆不动，先确认固件是用 `-DPICO_BOARD=pico2` 编出来的 ST7306 版本

2. **输入异常**
   - 校准摇杆死区
   - 检查I2C时钟频率
   - 验证摇杆固件

### 编译问题

1. **CMake配置失败**
   - 验证PICO_SDK_PATH环境变量
   - 检查Pico SDK版本（需要1.5.1+）
   - 确认CMake版本（需要3.13+）

2. **编译错误**
   - 检查C++编译器版本（需要C++17支持）
   - 验证所有依赖库已安装
   - 检查源代码语法错误

## 下一步

项目功能完整，可考虑以下增强：

1. **额外游戏模式**: 时间挑战、目标分数、无尽模式
2. **增强动画**: 更复杂的视觉效果
3. **性能优化**: 进一步渲染优化
4. **功能扩展**: 音效、触觉反馈、保存/加载功能

## 许可证

MIT License - 详见LICENSE文件

## 贡献

欢迎提交Issue和Pull Request来改进项目！ 
