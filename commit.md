# Commit log

## 2026-11-17 — RP2350 上的 ST7306 摇杆

### Summary

RP2350 配 ST7306 时，摇杆改成和阅读器同一套模拟输入。X 是 GPIO26，Y 是 GPIO27，按键是 GPIO22。ILI9488 仍用 GPIO6、GPIO7 上的 I2C 摇杆。RP2040 的 UF2 不能拿去刷 RP2350。

### 主要变更

- `src/hardware/input/joystick.cpp`：ST7306 编译读 ADC，其它目标保持硬件 I2C
- `include/hardware/system/pin_config.hpp`：写下模拟摇杆的三根脚
- `examples/game_2048_unified.cpp`：摇杆初始化失败时仍画出棋盘
- `docs/Pico开发环境配置手册.md`、两份 README：补上 RP2350 的构建目录和接线

建议说明：

```
feat: RP2350 的 ST7306 改用模拟摇杆

这块板上的摇杆接在 GPIO26、GPIO27 和 GPIO22，不是 I2C。
RP2350 需要单独用 pico2 构建，RP2040 的 UF2 会被引导程序忽略。
```

## 2026-11-12 — 黑底红框

### Summary

ESP32 上的棋盘改成黑底。每个格子一圈红框，框里是黑色，数字用经典方块颜色，没有方块底色。灰色底上的横纹还在，黑底上看不见。最左边一列会自己亮成红色，背景仍用黑色。

### 主要变更

- `esp32/main/main.cpp`：整屏黑底，68×68 红框，彩色数字；有变化时重画整幅棋盘
- `esp32/components/ili9488_panel/`：GPIO 按住片选，20 行一块 DMA，时钟和数据线用最低驱动电流，补上 `0xB4 = 0x02`
- `esp32/main/pins.hpp`：SPI 时钟 20 MHz
- `docs/ESP32-S3迁移手册.md`、两份 README：改成现在这版画面

建议说明：

```
feat: ESP32 棋盘改为黑底红框

灰色底上的横纹盖不住，画面改成黑底、红框和彩色数字。
片选改由 GPIO 按住整块像素。
```

## 2026-11-12 — ESP32-S3-Pico 上的 ILI9488 2048

### Summary

微雪 ESP32-S3-Pico 上的 ILI9488 2048 可以玩了。`esp32/` 是单独的 ESP-IDF 工程，不并进 Pico 的 CMake。棋盘是 68×68 方格，只重画变过的格子。计分改成经典规则：开局 0，合并时加上新牌数值。

### 主要变更

- `esp32/`：SPI DMA 点亮 ILI9488（RGB666），摇杆、上下翻页、屏幕键长按关背光
- `src/game/game_logic.cpp`、`game_state.cpp`：去掉按平方和角落加分；共享逻辑去掉 `pico/stdlib.h`
- `firmware/game2048_ili9488.uf2`、`firmware/game2048_st7306.uf2`：替换旧的 unified UF2 文件名
- `docs/ESP32-S3迁移手册.md`、两份 README：写成已经落地的引脚、烧录口和计分

### 不要提交

- `build-ili9488/`、`build-st7306/`、`esp32/build/`、`esp32/sdkconfig`
- `.DS_Store`

建议说明：

```
feat: 在 ESP32-S3-Pico 上跑通 ILI9488 2048

单独的 esp32/ 工程用 SPI DMA 驱动屏幕，棋盘局部刷新。
计分改为经典合并加分，开局为 0。ST7306 仍只保留 Pico 固件。
```
