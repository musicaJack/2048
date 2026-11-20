# 2048 接到微雪 RP2350B-Plus-W

查证日期：2026-11-20。目标板是微雪 **RP2350B-Plus-W**（Wiki：<https://www.waveshare.net/wiki/RP2350B-Plus-W>）。主控是 **RP2350B**，16 MB Flash，USB Type-C。无线是板载 Raspberry Pi Radio Module 2（**CYW43439**，Wi-Fi 4 + 蓝牙 5.2）。

排针物理孔位和 Raspberry Pi Pico / 原来的 RP2350 核心板一致，可以直接插进现有 ILI9488 底座。GPIO0–GPIO22 与 Pico 同号，屏幕、摇杆、翻页不用改线。无线走芯片自己的 GPIO36–GPIO39，没有引到 40 针，不占底座。

官方示例包放在 `docs/rp2350-plus-w/RP2350B-Plus-W.zip`。Pico SDK 自带的板定义里没有 `waveshare_rp2350b_plus_w`，不要用 `pico2_w` 或官方 Pico W 的 23/24/25 脚表。

C6 那条线先放下。这块板继续用根目录的 Pico SDK 工程，产物是 UF2。

## 0. 2026-11-20 已落地

两件事都在实机上跑通：

1. **ILI9488 版 2048**。按 `PICO_BOARD=pico2` 编译现有工程，`picotool` 烧进这块 Plus-W。画面和操作与原来的 RP2350 一致。
2. **Wi-Fi 站点**。用微雪板级头文件初始化 CYW43，连上办公室 2.4 GHz WPA2 热点，DHCP 拿到 `192.168.201.247`，`link=1`。同一份测试固件把 SSID 和 IP 画在 ILI9488 上。

密码不要写进仓库。SSID、密码只通过 CMake 的 `-DWIFI_SSID=` / `-DWIFI_PASSWORD=` 传进测试固件。

## 1. 这块板和原来 RP2350 差在哪

| 项目 | 原来的 RP2350 / Pico 2 | RP2350B-Plus-W |
|---|---|---|
| 封装 | RP2350A（30 只 GPIO）或同等 Pico 孔 | **RP2350B**（更多 GPIO，`PICO_RP2350A 0`） |
| Flash | 常见 4 MB | **16 MB** |
| 无线 | 无，或 Pico 2 W 的另一套脚 | CYW43439，接 GPIO36–GPIO39 |
| 40 针 GPIO0–22 | 同号 | 同号，ILI9488 底座可直插 |
| 物理针 31 / 32 / 34 | Pico 的 GP26 / GP27 / GP28 | **GPIO40 / GPIO41 / GPIO42** |
| 板载灯 | 视板而定 | 模组 `CYW43_WL_GPIO_LED_PIN`（0）；另有 LED2 = GPIO23 |
| 烧录 | Type-C，BOOT 进 U 盘，或 picotool | 相同。Mac 上是 `/dev/cu.usbmodem*`，没有 CH343 |

ILI9488 这套只用到 GP6–GP9、GP14–GP20，落在同号 GPIO 上，所以游戏固件按 `pico2` 编也能亮。ST7306 模拟摇杆写的是 GPIO26、GPIO27，插在这块板上对不上那两只孔，本仓库 ILI9488 路径没有用它们。

无线以后要开，就不能再当普通 `pico2` 编。必须把 `PICO_BOARD_HEADER_DIRS` 指到板级头文件，`PICO_BOARD` 设成 `waveshare_rp2350b_plus_w`，并保证头文件里有：

```c
pico_board_cmake_set(PICO_CYW43_SUPPORTED, 1)
```

官方 zip 里的头文件缺这一行。仓库里可用的副本在 `wifi_test/boards/waveshare_rp2350b_plus_w.h`，已经补上。

## 2. 底座上跑通的引脚

孔位与 `config/ili9488_config.json`、`include/hardware/system/pin_config.hpp` 相同。屏幕 SPI 模式 0，无 MISO，时钟 40 MHz，走 `spi0`。

| 底座功能 | Pico 孔 | 物理针 | 这块板上的 GPIO | 2048 | Wi-Fi 测试 |
|---|---|---|---|---|---|
| 摇杆 SDA | GP6 | 9 | GPIO6 | 用，I2C `0x63`，100 kHz，`i2c1` | 未用 |
| 摇杆 SCL | GP7 | 10 | GPIO7 | 用 | 未用 |
| 上翻页 | GP8 | 11 | GPIO8 | 用，按下为低 | 未用 |
| 下翻页 | GP9 | 12 | GPIO9 | 用 | 未用 |
| 屏幕键 | GP14 | 19 | GPIO14 | 用，长按约 600 ms 开关背光 | 未用 |
| 屏幕 RST | GP15 | 20 | GPIO15 | 用 | 用 |
| 背光 | GP16 | 21 | GPIO16 | 用，PWM，高电平方向亮 | 用 |
| 屏幕 CS | GP17 | 22 | GPIO17 | 用 | 用 |
| 屏幕 SCK | GP18 | 24 | GPIO18 | 用 | 用 |
| 屏幕 MOSI | GP19 | 25 | GPIO19 | 用 | 用 |
| 屏幕 DC | GP20 | 26 | GPIO20 | 用 | 用 |
| MicroSD SCK / MISO / MOSI / CS | GP10–GP13 | 14–17 | GPIO10–13 | 2048 不用 | 不用 |

无线不经过排针：

| CYW43 信号 | GPIO | 说明 |
|---|---|---|
| `WL_REG_ON` | 36 | 给模组上电 |
| DATA / IRQ | 37 | 半双工，DIN、DOUT、HOST_WAKE 共用 |
| `CS` | 38 | 片选 |
| `CLK` | 39 | 时钟，PIO 模拟 SPI，不是 `spi0` |
| 模组 LED | CYW43 GPIO0 | `cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, …)` |
| LED2 | 23 | 板载第二颗灯，不在 40 针常用孔上 |

`PICO_VSYS_PIN` 是 GPIO43。VBUS 经模组 GPIO2 读。这些脚也不在底座排针上。

板级头文件里的默认 SPI（SCK 18、MOSI 19、RX 16、CS 17）和屏幕孔重合，那是 Pico 的默认编号。屏幕由 `ILI9488Driver` 自己初始化 `spi0`。CYW43 用 PIO，两路不抢同一套控制器。

## 3. 游戏固件怎么跑通

根目录 `CMakeLists.txt` 不变。ILI9488 路径仍是：

- 驱动：`src/hardware/display/ili9488/`
- 摇杆：`src/hardware/input/joystick.cpp`，地址 `0x63`
- 入口：`examples/game_2048_unified.cpp`
- 引脚：`config/ili9488_config.json`

编译（工具链和 SDK 见 `docs/Pico开发环境配置手册.md`）：

```bash
export PICO_SDK_PATH="$HOME/pico/pico-sdk"
export PATH="$HOME/pico/arm-gnu-toolchain/15.3.rel2/bin:$PATH"
cmake -S . -B build-ili9488-rp2350 -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DPICO_BOARD=pico2 \
  -DUSE_ILI9488_DISPLAY=ON -DUSE_ST7306_DISPLAY=OFF
cmake --build build-ili9488-rp2350 --target game2048_unified
```

产物：`build-ili9488-rp2350/game2048_unified.uf2`，family 是 RP2350，不能拿 RP2040 的 UF2 去刷。

烧录：板子以应用模式挂在 USB 上时：

```bash
picotool load -f -x build-ili9488-rp2350/game2048_unified.uf2
```

也可以按住 BOOT 再插 Type-C，拷进出现的 U 盘。Mac 上口名是 `/dev/cu.usbmodem*`。

这份固件没有初始化 CYW43。模组保持关闭，所以画屏速度和原来的 RP2350 一样。要把 2048 和 Wi-Fi 合成一份固件时，再改成 Plus-W 板定义，并链接 `pico_cyw43_arch_lwip_threadsafe_background`。无线任务走后台中断，避免堵住 SPI 刷屏。

## 4. Wi-Fi 怎么工作

天线、匹配和 2.4 GHz PHY 都在 Radio Module 2 里。应用程序不翻转底座上的任何 GPIO。控制路径是 Pico SDK 的 `cyw43`：

1. 板级头文件给出 GPIO36–39 和 `PICO_CYW43_SUPPORTED`。
2. 链接 `pico_cyw43_arch_lwip_threadsafe_background`（后台跑驱动和 lwIP）或 `pico_cyw43_arch_lwip_poll`（主循环自己 `cyw43_arch_poll()`）。只亮灯、不上网时用 `pico_cyw43_arch_none`。
3. 工程带一份 `lwipopts.h`（DHCP、TCP、DNS）。测试工程用的是官方示例那份，在 `wifi_test/lwipopts.h`。
4. 代码顺序：

```c
cyw43_arch_init_with_country(CYW43_COUNTRY_CHINA);
cyw43_arch_enable_sta_mode();
cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 20000);
ip4addr_ntoa(netif_ip4_addr(netif_default));
```

国家码用 `CYW43_COUNTRY_CHINA`，2.4 GHz 信道按国内来。模组是 Wi-Fi 4，只扫、只连 2.4 GHz。纯 5 GHz 热点连不上。

认证按顺序试 `CYW43_AUTH_WPA2_AES_PSK`、`CYW43_AUTH_WPA2_MIXED_PSK`、`CYW43_AUTH_WPA3_WPA2_AES_PSK`。办公室那次站点连上后 `cyw43_wifi_link_status(..., CYW43_ITF_STA)` 为 1，地址 `192.168.201.247`。

SSID 可以带空格。CMake 这样传：

```bash
cmake -S wifi_test -B build-wifi-rp2350 -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DWIFI_SSID="Your SSID" \
  -DWIFI_PASSWORD="Your Password"
cmake --build build-wifi-rp2350 --target wifi_sta
picotool load -f -x build-wifi-rp2350/wifi_sta.uf2
```

测试程序在 `wifi_test/`：

```text
wifi_test/
├── CMakeLists.txt
├── wifi_sta.cpp
├── lwipopts.h
└── boards/waveshare_rp2350b_plus_w.h
```

逻辑：

1. 先初始化 ILI9488（RST 15、BL 16、CS 17、SCK 18、MOSI 19、DC 20），黑底写出 `Wi-Fi` 和 SSID，状态行显示 `Connecting...`。
2. 初始化 CYW43，开站点，扫描周围热点。
3. 按上面三种认证连接。失败则状态行变红，模组灯快闪。
4. 等到 DHCP 非 0 地址，状态行 `Connected`，屏中央用 2 倍 8×16 字体画出 IP。USB 串口同步打 `link=1 ip=…`。
5. 之后每约 1.6 秒刷新链路；IP 变了就重画。

这份测试会覆盖 2048 固件。要回去玩游戏，再烧 `build-ili9488-rp2350/game2048_unified.uf2`。

官方 zip 里还有：

| 示例 | 作用 |
|---|---|
| `C/01-LED` | `pico_cyw43_arch_none`，闪模组灯和 GPIO23 |
| `C/02-WIFI` | 只扫描，不关联 |
| `C/03-BLE` | BLE 温度；`server_with_wifi.c` 是连路由器 + iperf |
| `Python/` | 先刷 MicroPython UF2，会覆盖现在的应用 |

后续把联网做进 2048 时，从 `wifi_test` 的板定义、`lwipopts.h` 和 `cyw43_arch_init_with_country` 这条路径接进根目录工程即可。不要用 `pico2_w`。
