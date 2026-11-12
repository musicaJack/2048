# 2048 迁到微雪 ESP32-S3-Pico

查证日期：2026-11-11。目标板是微雪 **ESP32-S3-Pico**（主芯片 ESP32-S3R2，16 MB Flash，2 MB Quad PSRAM，USB 口经过沁恒 CH343）。排针的**物理孔位**和 Raspberry Pi Pico 一致，丝印上的 `GP` 编号是 ESP32-S3 自己的 GPIO，不是 Pico 的 GP 号。

底座已经焊死，线不动。迁移只改固件里的引脚号，以及把 Pico SDK 的 SPI、I2C、GPIO 换成 ESP-IDF。`firmware/*.uf2` 仍只给 RP2040，不能拖到这块板。

## 0. 2026-11-12 已落地

ILI9488 这一路已经在微雪 ESP32-S3-Pico 上跑通。ST7306 没有移植。Pico 工程仍用根目录的 `CMakeLists.txt`，不要和 `esp32/` 合成一个构建。

实际目录：

```text
esp32/
├── CMakeLists.txt
├── sdkconfig.defaults          # esp32s3、16MB、QIO、Quad PSRAM
├── main/
│   ├── main.cpp                # 画面、局部刷新、摇杆和按键
│   └── pins.hpp
└── components/
    ├── game2048/               # 编译仓库里的 src/game/game_logic.cpp、game_state.cpp、game_2048.cpp
    ├── ili9488_panel/          # SPI2 + DMA，RGB666
    └── board_input/            # I2C 摇杆、上下翻页、屏幕键
```

编译和烧录（先 `. "$HOME/esp/esp-idf/export.sh"`，或用 `get_idf`）：

```bash
cd esp32
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/cu.usbmodemXXXXXXXX flash
```

这块板直连 Mac 的 Type-C 会出两个口：

- `/dev/cu.usbmodem5AE70645591` 一类：沁恒 CH343，`USB Single Serial`。`idf.py flash` 和日志用这个。
- `/dev/cu.usbmodem11201` 一类：乐鑫 USB JTAG/serial。不要拿它烧录。

macOS 已把 CH343 绑成 `usbmodem`，不必先装沁恒的 CH34x 驱动。装了驱动口名可能变成 `wchusbserial`，现有烧录命令会对不上。

屏幕是 ILI9488，320×480，竖屏。初始化沿用 Pico 驱动：`MADCTL 0x36 = 0x48`，像素格式 `0x3A = 0x66`（RGB666），`INVON`。SPI 用 `SPI2_HOST`，必须 `SPI_DMA_CH_AUTO`。DMA 关掉时主机一次最多 64 字节，整行像素会被丢掉，屏保持出厂白底。时钟先用 `pins.hpp` 里的 10 MHz。背光是 GPIO42 高电平，还没有 PWM。

画面：左上角「2048」，右上角分数，下面是 68×68 的方格，棋盘靠屏幕下沿。空格是棕色，有数字的格子用经典 2048 配色。摇杆只重画变过的格子和分数。摇杆死区 1000、阈值 1800；按住约每 200 ms 走一步。按下摇杆中间重新开局。上翻、下翻键等于向上、向下。屏幕键长按 600 ms 开关背光。

计分按网上经典 2048：开局 0 分。两张相同的牌合成一张时，把新牌的数值加进分数。开局的两张牌和后来随机补上的牌不计分。旧规则把数字平方再加角落奖励，所以两张 2、其中一张在角上会显示 28，已经删掉。

日志在 CH343 口，115200。打开这个口会经 DTR/RTS 复位芯片。

## 1. 先定边界

留下、原样编译进新工程的部分：

- `src/game/game_logic.cpp`、`include/game/game_logic.hpp`：棋盘滑动、合并、胜负。
- `src/game/game_state.cpp` 里的棋盘数据。迁的时候把头文件里的 `#include "pico/stdlib.h"` 换成 `<cstdint>` 和 `<array>` 这一类标准头。
- 方块滑动、合并、胜负。计分已改成经典规则：只累加本次合并出的新牌数值，开局为 0。画面布局在 `esp32/main/main.cpp`，不在这几份共享源文件里。

必须重写的部分：

| 现有文件 | 原因 |
|---|---|
| `src/hardware/display/ili9488/ili9488_driver.cpp`、`ili9488_hal.cpp` | 直接调用 `spi_init`、`gpio_set_function(..., GPIO_FUNC_SPI)` |
| `src/hardware/display/st7306/st7306_driver.cpp`、`st7305_driver.cpp` | 同样绑在 `spi0` 上 |
| `src/hardware/input/joystick.cpp` | 使用 `hardware/i2c.h` 和 `i2c_inst_t` |
| `examples/game_2048_unified.cpp` | `pico/stdlib.h`、`PICO_DEFAULT_LED_PIN` |
| `CMakeLists.txt` | `pico_sdk_init()`、`pico_add_extra_outputs()` |

`include/display/display_driver.hpp` 里的 `DisplayDriver` 接口（`fillRect`、`drawString`、`display`）不依赖 Pico，新驱动继续实现这组接口，游戏 UI 就不用重画。

不要把现有的 `CMakeLists.txt` 指到 ESP-IDF 上硬编。`pico/stdlib.h` 在 ESP-IDF 里不存在，修不完。新开一个 `esp32/` 工程，把能用的源文件加进去。

## 2. 引脚对照

微雪把 Pico 的孔叫 `D0`–`D28`，每个孔接到一颗 ESP32-S3 GPIO。下表是官方 MicroPython 固件里的对应关系。固件里只写右列的数字。

底座上 2048 和文本阅读器已经占用的孔：

| 底座上的功能 | Pico 孔位 | ESP32-S3 GPIO | 电气要求 |
|---|---|---|---|
| 摇杆 SDA | GP6 | **17** | I2C，开内部上拉，100 kHz，器件地址 `0x63` |
| 摇杆 SCL | GP7 | **18** | 同上 |
| 上翻页 | GP8 | **33** | 输入，上拉，按下为低 |
| 下翻页 | GP9 | **34** | 同上 |
| 屏幕开关 | GP14 | **39** | 同上，长按 600 ms 开关屏 |
| 屏幕 RST | GP15 | **40** | 推挽输出 |
| 屏幕背光 | GP16 | **42** | 先做 GPIO 常亮，稳定后再改 LEDC PWM |
| 屏幕 CS | GP17 | **41** | SPI 片选，空闲为高 |
| 屏幕 SCK | GP18 | **1** | SPI 时钟 |
| 屏幕 MOSI | GP19 | **2** | SPI 数据 |
| 屏幕 DC | GP20 | **4** | 数据/命令，不进 SPI 总线 |

阅读器工程里的 MicroSD（若底座上也焊了）占用 Pico GP10–GP13，对应 ESP32 GPIO **35、36、37、38**。2048 当前不用 SD 卡，这四只脚先空着。

不要占用：

- **GPIO0**：BOOT 键，启动时拉低会进入下载模式。丝印靠近 USB 的 `GP0` 就是它。
- **GPIO21**：板载 WS2812。丝印上的 `GP21` 是这颗灯，不是 Pico 的 GP21。
- **GPIO19、GPIO20**：芯片 USB D−/D+，这块板没有把它们引到排针上。
- **GPIO26–GPIO32**：模组内部的 Flash / Quad PSRAM，排针上没有。

这张图上的 SCK、MOSI、DC（GPIO1、GPIO2、GPIO4）不是 ESP32-S3 SPI 控制器的默认脚。ESP-IDF 用 GPIO 矩阵仍能把 SPI 接到这三只脚上。第一次点亮屏幕时把时钟放在 **10 MHz**，花屏再降到 4 MHz，稳定后再往 20 MHz、40 MHz 试。Pico 工程里写的 40 MHz 不要一上来就用。

按键电平和阅读器一致：释放为高，按下为低，内部上拉，消抖 50 ms，长按 600 ms。

## 3. 在这台 Mac 上装 ESP-IDF

用 ESP-IDF **v5.5.4**，目标芯片 `esp32s3`。官方说明：<https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/get-started/linux-macos-setup.html>

本机已经有 CMake、Ninja、Git、Python 3.9。还缺下载模式用的工具：

```bash
brew install dfu-util ccache
```

IDF 和 Pico SDK 分开目录，不要克隆进 `~/pico`。这台机器在国内，源码走 Gitee 上的乐鑫镜像，编译器和 Python 包走乐鑫国内下载站。不要运行 `esp-gitee-tools/jihu-mirror.sh set`，那个脚本会改全局 Git 配置。

```bash
mkdir -p "$HOME/esp"
git clone -b v5.5.4 --depth 1 https://gitee.com/EspressifSystems/esp-idf.git "$HOME/esp/esp-idf"
git clone --depth 1 https://gitee.com/EspressifSystems/esp-gitee-tools.git "$HOME/esp/esp-gitee-tools"
"$HOME/esp/esp-gitee-tools/submodule-update.sh" "$HOME/esp/esp-idf"
cd "$HOME/esp/esp-idf"
env -u http_proxy -u https_proxy -u HTTP_PROXY -u HTTPS_PROXY \
  -u all_proxy -u ALL_PROXY \
  IDF_GITHUB_ASSETS="dl.espressif.cn/github_assets" \
  IDF_PIP_WHEELS_URL="https://dl.espressif.cn/pypi" \
  ./install.sh esp32s3
```

`IDF_GITHUB_ASSETS` 只影响这次工具下载，把 GitHub Release 附件改到 `dl.espressif.cn`。子模块地址由 `submodule-update.sh` 写进这个仓库自己的 `.git/config`，不改用户全局配置。

Gitee 和 `dl.espressif.cn` 在国内，这两步不要走本机代理。终端里如果已经设置了 `http_proxy` / `https_proxy`，先对这一次命令去掉：

```bash
env -u http_proxy -u https_proxy -u HTTP_PROXY -u HTTPS_PROXY \
  -u all_proxy -u ALL_PROXY \
  GIT_HTTP_VERSION=HTTP/1.1 \
  "$HOME/esp/esp-gitee-tools/submodule-update.sh" "$HOME/esp/esp-idf"
```

`components/esp_wifi/lib`（`esp32-wifi-lib`）在 Gitee 上会返回 423，仓库被封。这一份单独从 GitHub 拉，走代理即可。只改本仓库的子模块地址：

```bash
cd "$HOME/esp/esp-idf"
git config --local submodule.components/esp_wifi/lib.url https://github.com/espressif/esp32-wifi-lib.git
git submodule update --init --depth 1 components/esp_wifi/lib
```

`--depth 1` 只取 v5.5.4 记录的那一次提交。不要省掉它，否则会把这个二进制仓库的全部历史拉下来，代理会很慢。

GitHub 直连可用时，等价的国际线路是：

```bash
git clone -b v5.5.4 --recursive https://github.com/espressif/esp-idf.git "$HOME/esp/esp-idf"
cd "$HOME/esp/esp-idf"
./install.sh esp32s3
```

每次新开终端、准备编 ESP32 时再加载工具链。不要把下面这行写进 `~/.zshrc`。这台机器的 `~/.zshrc` 里已经有 Pico 的 `PICO_SDK_PATH` 和 `arm-none-eabi-gcc`，两条工具链的 `PATH` 叠在一起会互相盖住编译器。

```bash
. "$HOME/esp/esp-idf/export.sh"
```

可以只加一个别名：

```bash
alias get_idf='. "$HOME/esp/esp-idf/export.sh"'
```

加载之后确认：

```bash
echo "$IDF_PATH"          # /Users/shmusica.mj/esp/esp-idf
idf.py --version
xtensa-esp32s3-elf-gcc --version
```

这块板下载走 CH343 的 UART，板上有自动下载电路，正常情况不用按 BOOT。2026-11-12 在这台 Mac 上直连 Type-C 即可枚举，不必经过 Hub，也不必先装沁恒 CH34x 驱动。系统把它认成 `usbmodem`：

```text
/dev/cu.usbmodem*     # 沁恒 USB Single Serial，烧录和日志用这个
/dev/cu.usbmodem*     # 乐鑫 USB JTAG/serial，不要用于 idf.py flash
```

用 `ioreg -p IOUSB` 区分：产品名是 `USB Single Serial` 的才是 CH343。端口一直不出现时，按住 BOOT 再插 USB-C。装了 CH34x 驱动之后，口名可能变成 `/dev/cu.wchusbserial*`。

## 4. 用 hello_world 确认板子

工具链装完后，先不要动 2048 的源码。

```bash
. "$HOME/esp/esp-idf/export.sh"
cp -r "$IDF_PATH/examples/get-started/hello_world" "$HOME/esp/hello_world"
cd "$HOME/esp/hello_world"
idf.py set-target esp32s3
idf.py menuconfig
```

在 menuconfig 里改两项，否则 16 MB Flash 的板子会起不来：

- `Serial flasher config` → `Flash size` → **16 MB**
- `Component config` → `ESP PSRAM` → 打开 SPI RAM，模式选 **Quad**

然后：

```bash
idf.py -p /dev/cu.usbmodemXXXXXXXX flash monitor
```

串口打出 `Hello world!` 就说明编译器、下载和复位都通了。退出监视器用 `Ctrl+]`。

第二步再确认孔位映射，不要先搬整个游戏。写一个只做三件事的小工程：

1. GPIO42 拉高，ILI9488 背光应亮（这个孔在 Pico 上是 GP16）。
2. GPIO40 对屏幕做一次复位脉冲：拉低 10 ms，再拉高。
3. GPIO17、GPIO18 作为 I2C，向地址 `0x63` 读寄存器 `0xFE`（摇杆固件版本）。有 ACK 就说明摇杆在总线上。

这三步过了，再迁显示驱动。背光不亮或 I2C 没有 ACK，先查映射，不要开始搬 UI。

## 5. 新工程怎么放

在本仓库里单独放，不替换 Pico 的 `CMakeLists.txt`。已经写好的目录见第 0 节，组件名是 `game2048`、`ili9488_panel`、`board_input`。没有去实现 Pico 的 `DisplayDriver`。

`sdkconfig.defaults` 至少包含：

```text
CONFIG_IDF_TARGET="esp32s3"
CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y
CONFIG_ESPTOOLPY_FLASHMODE_QIO=y
CONFIG_SPIRAM=y
CONFIG_SPIRAM_MODE_QUAD=y
```

`main/pins.hpp` 用 ESP32 的编号，并在注释里保留 Pico 孔位，避免以后按丝印上的 GP 号去改：

```cpp
#pragma once

// 孔位是 Pico 的，数值是 ESP32-S3 的 GPIO。
static constexpr int PIN_JOY_SDA = 17;   // Pico GP6
static constexpr int PIN_JOY_SCL = 18;   // Pico GP7
static constexpr int PIN_PAGE_UP = 33;   // Pico GP8，按下为低
static constexpr int PIN_PAGE_DOWN = 34; // Pico GP9
static constexpr int PIN_SCREEN = 39;    // Pico GP14
static constexpr int PIN_LCD_RST = 40;   // Pico GP15
static constexpr int PIN_LCD_BL  = 42;   // Pico GP16
static constexpr int PIN_LCD_CS  = 41;   // Pico GP17
static constexpr int PIN_LCD_SCK = 1;    // Pico GP18
static constexpr int PIN_LCD_MOSI = 2;   // Pico GP19
static constexpr int PIN_LCD_DC  = 4;    // Pico GP20

static constexpr uint8_t JOYSTICK_ADDR = 0x63;
static constexpr int I2C_HZ = 100000;
static constexpr int SPI_HZ_BRINGUP = 10 * 1000 * 1000;
```

## 6. 迁移顺序

按这个顺序做。上一层没亮，不要开始下一层。

### 6.1 游戏逻辑脱开 Pico 头文件

`include/game/game_state.hpp` 现在包含 `pico/stdlib.h`。改成标准库头之后，`game_logic.cpp` 和 `game_state.cpp` 应能被宿主编译器单独编译，不链接 `hardware_spi`、`hardware_i2c`。这一步不过，说明逻辑层还藏着 Pico API，先清掉再谈显示。

### 6.2 屏幕

ILI9488 初始化命令序列从 `ili9488_driver.cpp` 原样抄过来，只换掉收发实现：

- `spi_bus_initialize(SPI2_HOST, ...)`，`mosi_io_num = 2`，`sclk_io_num = 1`，`miso_io_num = -1`，`quadwp_io_num = -1`，`quadhd_io_num = -1`。
- CS 用 `spics_io_num = 41`，也可以自己拉 GPIO41。DC 用 GPIO4 在每次传输前手动切换，不要把它放进 SPI 总线。
- 复位脚 GPIO40：低 10 ms，高之后等 120 ms，再送初始化命令。
- 背光 GPIO42 先输出高电平。Pico 工程里 `ILI9488_BACKLIGHT_USE_PWM` 为 1，那是第二步，用 LEDC 即可。
- 颜色是 RGB666（`PIXFMT 0x66`），每个像素 3 字节，低 2 位清掉。不要改成 RGB565。
- `spi_bus_initialize` 的第三个参数用 `SPI_DMA_CH_AUTO`。关掉 DMA 时，大于 64 字节的传输会被驱动拒绝，屏幕写不进显存。
- 发给 DMA 的缓冲区放在 `MALLOC_CAP_DMA` 内存里。源数据如果已经是这块 DMA 缓冲，不要再往自己身上 `memcpy`。

三色能铺满之后再画棋盘。画面在 `esp32/main/main.cpp`，没有接 Pico 的 `GameUIRenderer`。

ST7306 用同一组 SPI 脚，另做一个驱动，实现同一个 `DisplayDriver`。灰度打包格式留在现有 `st7306_driver.cpp` 里，只替换 `spi0` 那一层。两块屏仍然分成两个烧录目标，不要在一个固件里同时初始化两块控制器。

### 6.3 摇杆

寄存器定义继续用 `include/hardware/input/joystick.hpp` 里的 `0x00`、`0x10`、`0x20`、`0x30`、`0xFE`。把 `begin(i2c_inst_t*, ...)` 换成不带 `i2c_inst_t` 的接口。总线用 `i2c_new_master_bus`，SDA 17、SCL 18，地址 `0x63`，内部上拉打开。

方向阈值继续用现在的死区：`JOYSTICK_DEADZONE` 是 1000，`JOYSTICK_THRESHOLD` 是 1800。中心键仍是复位一局，和 `examples/game_2048_unified.cpp` 里的行为一致。

### 6.4 三颗底座按键

2048 的 Pico 固件没有读这三颗键，底座上它们是存在的。显示和摇杆稳定之后再加：

- GPIO33、GPIO34、GPIO39，`GPIO_MODE_INPUT`，上拉，下降沿算按下。
- 阅读器里的分工：短按上、下是翻页或移动；长按 600 ms，上键进菜单，下键返回；GPIO39 长按开关屏幕。2048 可以先做成上、下等价于摇杆的上、下，屏幕键拉低 GPIO42 关背光。
- 消抖 50 ms。不要用 GPIO0 做这颗键。

### 6.5 主循环

`examples/game_2048_unified.cpp` 的流程留着：初始化显示、初始化输入、循环读方向、调用 `Game2048::move*`、再画 UI。把 `sleep_ms` 换成 `vTaskDelay`。板载 LED 不是 Pico 的 GP25；这块板的灯是 GPIO21 上的 WS2812，游戏循环里先不要驱它。

日志用 `ESP_LOGI`，监视器波特率 115200。`printf` 也会进监视器，但要先 `idf.py monitor`，它不会变成 Pico 那种 USB CDC 盘符。

## 7. 编译和烧录

在加载过 `export.sh` 的终端里：

```bash
cd /Users/shmusica.mj/Projects/code/2048/esp32
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/cu.wchusbserialXXXX flash monitor
```

产物是 `build/*.bin`，加上 bootloader 和分区表，由 `idf.py flash` 一次写完。没有 `.uf2`，也不要再拖到 `RPI-RP2`。

改过 `sdkconfig` 里的 Flash 大小或 PSRAM 之后，若启动停在 `invalid header`，清掉再烧：

```bash
idf.py -p /dev/cu.usbmodemXXXXXXXX erase-flash
idf.py -p /dev/cu.usbmodemXXXXXXXX flash monitor
```

Pico 的两份 UF2 留在 `firmware/`，继续给 RP2040 底座用。ESP32 的 bin 不要覆盖那两个文件。

## 8. 和 Pico 环境怎么共存

| | Pico（已经装好） | ESP32-S3 |
|---|---|---|
| SDK | `~/pico/pico-sdk` 2.3.1 | `~/esp/esp-idf` v5.5.4 |
| 编译器 | `arm-none-eabi-gcc` 15.3.Rel2 | `xtensa-esp32s3-elf-gcc` |
| 进入环境 | 新终端自动有 `PICO_SDK_PATH` | 需要时执行 `. ~/esp/esp-idf/export.sh` |
| 烧录 | BOOTSEL 后拷贝 UF2 | `idf.py flash`，串口是 CH343 |
| 本仓库入口 | 根目录 `CMakeLists.txt` | `esp32/` |

编 Pico 的终端里不要先执行 `export.sh`。编 ESP32 的终端里不要指望 `arm-none-eabi-gcc` 还在 `PATH` 最前面。

## 9. 参考

- ESP-IDF v5.5.4，ESP32-S3，macOS 安装：<https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/get-started/linux-macos-setup.html>
- 微雪 ESP32-S3-Pico 引脚与 `D0`–`D28` 对照：<https://www.waveshare.com/wiki/ESP32-S3-Pico>
- ESP32-S3 GPIO 限制（GPIO0 启动脚、GPIO19/20 USB、GPIO26–32 Flash/PSRAM）：<https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html>
- 沁恒 CH343 Mac 驱动：微雪该页 Resources → CH343 → MAC driver
