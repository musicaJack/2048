# Commit log

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
