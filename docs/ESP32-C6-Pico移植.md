# 2048 接到微雪 ESP32-C6-Pico

查证日期：2026-11-20。目标板是微雪 **ESP32-C6-Pico**（SKU 26866 贴片板 / 26845 排针版 ESP32-C6-Pico-M）。主芯片是 ESP32-C6-MINI-1：RISC-V 单核，最高 160 MHz，320 KB ROM，512 KB HP SRAM，16 KB LP SRAM，**4 MB Flash，没有 PSRAM**。官方说明：<https://docs.waveshare.net/ESP32-C6-Pico/>。原理图：<https://files.waveshare.com/wiki/ESP32-C6-Pico/ESP32-C6-Pico-Sch.pdf>。

排针的物理孔位和 Raspberry Pi Pico / RP2350 一致。丝印上的 `GP` 编号沿用 Pico 的孔名。铜皮接到哪颗脚，以原理图为准，不按丝印上的数字当成 C6 的 GPIO 号。

底座接线保持项目里已经焊好的那一套。Pico 工程和 `esp32/` 里现有的 ESP32-S3 工程都不改行为。C6 只在同一份 `esp32/` 工程里加一份目标配置。

这份说明写的是插上现有底座之后，每根线实际落到哪里。屏幕六根落在板载扩展芯片上，见第 3 节。

## 1. 板载扩展口

官方要求：GPIO22（SDA）和 GPIO23（SCL）只做 I2C，用来驱动板载 **TCA9554PWR**。这两只脚保持 I2C。可以在这条总线上再挂地址不冲突的 I2C 设备。不要把它们配成普通 GPIO、SPI 或 UART，否则 EXIO 整组失效。

TCA9554 把 8 只扩展脚送到排针。官方把它们叫做 EXIO1–EXIO7，芯片的 P7 是 EXIO8，没有引出到排针。要用这些脚，固件在 GPIO22/GPIO23 上按 I2C 时序读写扩展芯片的寄存器。C6 只有一路高速 I2C 控制器，摇杆已经占用 GPIO14/GPIO15 上的这一路。扩展芯片仍只使用 GPIO22/GPIO23，寄存器和下面这张表一致，时序由这两只脚按开漏方式送出：

| 寄存器 | 地址 | 作用 |
|---|---|---|
| 输入 | 0x00 | 读电平 |
| 输出 | 0x01 | 写出电平 |
| 极性 | 0x02 | 保持 0 |
| 方向 | 0x03 | 1 为输入，0 为输出 |

官方示例里的函数对应这些寄存器：`TCA9554PWR_Init`、`Read_REG` / `Write_REG`、`Mode_EXIO`、`Read_EXIO`、`Set_EXIO`、`Set_Toggle`。地址由 A2、A1、A0 决定，范围 `0x20`–`0x27`。原理图上 A1、A2 接地。A0 的电平以实机扫描为准，上电后在 GPIO22/GPIO23 上从 `0x20` 扫到 `0x27`，应答的那个就是它。摇杆地址是 `0x63`，和这个范围错开，而且摇杆不在这条总线上。

扩展芯片和摇杆是两条 I2C：

| 总线 | C6 引脚 | 上面的器件 |
|---|---|---|
| 扩展 | GPIO22 = SDA，GPIO23 = SCL | TCA9554，地址 `0x20`–`0x27` |
| 摇杆 | GPIO14 = SDA，GPIO15 = SCL | 摇杆，地址 `0x63`，100 kHz |

## 2. 项目里已经占用的孔

孔位和 `docs/ESP32-S3迁移手册.md`、`esp32/main/pins.hpp` 相同。屏幕信号来自 `config/ili9488_config.json`：DC = GP20，RST = GP15，CS = GP17，SCK = GP18，MOSI = GP19，背光 = GP16。没有 MISO。SPI 模式 0。

| 底座功能 | Pico 孔 | 物理针 | ESP32-S3 上的 GPIO | 这套 2048 是否使用 |
|---|---|---|---|---|
| 摇杆 SDA | GP6 | 9 | 17 | 用，I2C `0x63` |
| 摇杆 SCL | GP7 | 10 | 18 | 用 |
| 上翻页 | GP8 | 11 | 33 | 用，按下为低 |
| 下翻页 | GP9 | 12 | 34 | 用 |
| 屏幕键 | GP14 | 19 | 39 | 用，长按 600 ms 开关背光 |
| 屏幕 RST | GP15 | 20 | 40 | 用 |
| 背光 | GP16 | 21 | 42 | 用，高电平亮 |
| 屏幕 CS | GP17 | 22 | 41 | 用 |
| 屏幕 SCK | GP18 | 24 | 1 | 用 |
| 屏幕 MOSI | GP19 | 25 | 2 | 用 |
| 屏幕 DC | GP20 | 26 | 4 | 用 |
| MicroSD SCK | GP10 | 14 | 35 | 2048 不用 |
| MicroSD MISO | GP11 | 15 | 36 | 2048 不用 |
| MicroSD MOSI | GP12 | 16 | 37 | 2048 不用 |
| MicroSD CS | GP13 | 17 | 38 | 2048 不用 |

GP10–GP13 按本仓库现有用法保留给 MicroSD。Pico 侧在 `include/hardware/system/pin_config.hpp`：SCK = 10，MISO = 11，MOSI = 12，CS = 13，走 SPI1。ESP32-S3 上这四孔是 GPIO 35、36、37、38。2048 的 ILI9488 路径不占用它们。C6 固件同样把这四孔留空，不拿来改接屏幕。

## 3. 同一批孔插到 C6 上落到哪里

下表是原理图网表。左列是底座上的 Pico 孔，右列是这块 C6 板上那只孔实际连接的网络。

| 底座功能 | Pico 孔 | 物理针 | C6 上的实际网络 | 能不能当原生 GPIO |
|---|---|---|---|---|
| 串口 TX | 板端 TX | 1 | UART0 TX | 能，日志口 |
| 串口 RX | 板端 RX | 2 | UART0 RX | 能 |
| 摇杆 SDA | GP6 | 9 | GPIO14 | 能，摇杆 I2C |
| 摇杆 SCL | GP7 | 10 | GPIO15 | 能。GPIO15 是启动脚，按键平时为高 |
| 上翻页 | GP8 | 11 | GPIO8 | 能。GPIO8 是启动脚，按键平时为高 |
| 下翻页 | GP9 | 12 | GPIO9 | 能。GPIO9 是启动脚，按键平时为高 |
| MicroSD SCK | GP10 | 14 | GPIO18 | 能。2048 留空 |
| MicroSD MISO | GP11 | 15 | GPIO19 | 能。2048 留空 |
| MicroSD MOSI | GP12 | 16 | GPIO20 | 能。2048 留空 |
| MicroSD CS | GP13 | 17 | GPIO21 | 能。2048 留空 |
| 屏幕键 | GP14 | 19 | **EXIO6** | 扩展输入 |
| 屏幕 RST | GP15 | 20 | **EXIO7** | 扩展输出 |
| 背光 | GP16 | 21 | **EXIO5** | 扩展输出 |
| 屏幕 CS | GP17 | 22 | **EXIO4** | 扩展输出 |
| 屏幕 SCK | GP18 | 24 | **EXIO3** | 扩展输出 |
| 屏幕 MOSI | GP19 | 25 | **EXIO2** | 扩展输出 |
| 屏幕 DC | GP20 | 26 | **GPIO22** | 这只脚同时是 TCA9554 的 SDA |
| 扩展 SCL | GP21 | 27 | GPIO23 | TCA9554 的 SCL，保持 I2C |
| （未用） | GP22 | 29 | EXIO1 | 扩展脚，2048 不用 |

EXIO 和 TCA9554 端口的对应：EXIO1 = P0，EXIO2 = P1，EXIO3 = P2，EXIO4 = P3，EXIO5 = P4，EXIO6 = P5，EXIO7 = P6，EXIO8 = P7（未引出）。

屏幕六根里，SCK、MOSI、CS、RST、背光都在扩展芯片上。DC 这只孔是 GPIO22，也就是扩展芯片自己的 SDA。官方要求这只脚保持 I2C。它不能再当成 SPI 的 DC 去翻转。

TCA9554 每次改一只脚，都要在 I2C 上写一次寄存器。按 400 kHz 估算，一个 SPI 时钟沿大约一百微秒。320×480、RGB666 的一整帧大约 46 万字节，走扩展口要十几分钟。ESP32-S3 上同一帧走 SPI2 DMA、20 MHz，是几十毫秒。插在现有底座上时，C6 的屏幕刷新到不了 S3 那一版的速度。

摇杆和上下翻页落在原生 GPIO 上，这两路可以和 S3 一样工作。屏幕键在 EXIO6 上，读扩展芯片的输入寄存器即可，按下为低。

启动脚是 GPIO4、GPIO5、GPIO8、GPIO9、GPIO15。翻页和摇杆 SCL 用了后面三只。按键带内部上拉、平时为高，复位时没有按住就能正常启动。

GPIO12、GPIO13 是芯片自己的 USB D−/D+，接到 Type-C，没有引到排针。GPIO24–GPIO30 在模组内部的 Flash 上，排针上没有。WiFi 也不占用上面任何一只孔，控制方式见第 6 节。

## 4. 烧录口

官方产品页、ESP-IDF 页和这份原理图里都没有 CH343，也没有其他 USB 转串口芯片。Type-C 的 D+、D− 直接接到模组的 USB 脚（原理图网络 `USB_P`、`USB_N`，对应 GPIO12、GPIO13）。

烧录和日志走 C6 自己的 USB Serial/JTAG。Mac 上出现的是 `/dev/cu.usbmodem*`。这和 ESP32-S3-Pico 不同：S3 的下载口是沁恒 CH343，系统里显示为 `USB Single Serial`，见 `docs/ESP32-S3迁移手册.md`。

打开这个口会经 DTR/RTS 复位芯片，和 S3 上打开 CH343 口的现象一样。

Flash 容量按模组标称的 4 MB 配置。关掉 PSRAM。ESP-IDF 用现有的 `~/esp/esp-idf`（v5.5.4），补上 `esp32c6` 工具链后再 `idf.py set-target esp32c6`。

## 5. 固件怎么放

仍用一份 `esp32/` 工程。`sdkconfig.defaults` 继续给 S3。另加 `sdkconfig.defaults.esp32c6`：目标 `esp32c6`，Flash 4 MB，关闭 PSRAM。

引脚按芯片拆开。`pins.hpp` 按 `CONFIG_IDF_TARGET_ESP32C6` 选表。S3 的数字保持现在的 `pins.hpp`。C6 的表按第 3 节：

- 摇杆 SDA/SCL = GPIO14 / GPIO15
- 上翻页 / 下翻页 = GPIO8 / GPIO9
- 屏幕键 = EXIO6（端口 P5）。GPIO22/GPIO23 扫描地址 `0x20`–`0x27`，方向寄存器写成全输入，再读输入寄存器。按下为低
- 屏幕的 SCK、MOSI、CS、DC、RST、背光按第 3 节标成扩展脚，引脚号写成 `-1`。`ili9488_panel` 看到 `-1` 就不调用 `spi_bus_initialize`。游戏循环继续跑，摇杆和翻页的动作打在 USB 日志里

`game2048` 和 `main.cpp` 的画面参数（黑底、分数、68×68、红框、摇杆阈值、长按背光）保持不变。Pico 工程不动。当前这版固件不调用 `esp_wifi_init()`，调制解调器保持关闭。

## 6. WiFi 怎么控制

查的是乐鑫 ESP-IDF v5.5 对 ESP32-C6 的说明，以及本机 `~/esp/esp-idf`（v5.5.4）里的源码。

- 驱动说明：<https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32c6/api-guides/wifi.html>
- API：<https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32c6/api-reference/network/esp_wifi.html>
- 站点示例：`examples/wifi/getting_started/station/main/station_example_main.c`

天线、匹配电路和 40 MHz 晶振都在 ESP32-C6-MINI-1 模组内部。应用程序不翻转任何 GPIO。控制手段是一组函数，Wi-Fi 任务收到调用后写片内 MAC、基带和 PHY 的寄存器，PHY 再驱动模组上的天线。蓝牙 LE 和 802.15.4 共用这套 2.4 GHz 射频。

站点连上路由器的顺序：

1. `nvs_flash_init()` 准备存放射频校准和配置的 Flash 区域。
2. `esp_netif_init()`、`esp_event_loop_create_default()`、`esp_netif_create_default_wifi_sta()` 建好 TCP/IP 接口和事件循环。
3. `esp_wifi_init()` 分配收发缓冲并创建 Wi-Fi 任务。
4. `esp_wifi_set_mode(WIFI_MODE_STA)` 设成站点，`esp_wifi_set_config()` 写入 SSID 和密码。
5. `esp_wifi_start()` 启动射频。驱动发出 `WIFI_EVENT_STA_START`。
6. 回调里调用 `esp_wifi_connect()`。驱动自己扫描和认证。连上后发出 `WIFI_EVENT_STA_CONNECTED`，LwIP 自动做 DHCP。
7. 拿到地址时发出 `IP_EVENT_STA_GOT_IP`。之后套接字收发的数据不再经过排针。

`esp_wifi_init()` 的开源部分在 `components/esp_wifi/src/wifi_init.c`。它给调制解调器准备时钟和电源，调用 `esp_wifi_init_internal()`、`esp_phy_modem_init()`，再启动 WPA 认证。MAC、基带和射频寄存器的实现是按芯片分发的预编译库：

- `components/esp_wifi/lib/esp32c6/libnet80211.a`、`libpp.a`、`libcore.a`
- `components/esp_phy/lib/esp32c6/libphy.a`

时钟门控是开源的。`components/esp_hw_support/periph_ctrl.c` 的 `wifi_module_enable()` 打开 Wi-Fi 模块时钟。`components/esp_hw_support/modem_clock.c` 的 `modem_clock_module_mac_reset()` 复位片内 Wi-Fi MAC。两处都是写芯片内部寄存器。

这块模组的 WiFi 和本仓库现在用的摇杆、翻页、扩展口、屏幕孔没有引脚重叠。真正要留的是内存和 CPU：C6 只有一颗核、512 KB SRAM，没有 PSRAM，协议栈和画屏分这一份资源。发射时 3.3 V 电流会抬高。模组天线那一头下面如果压着大块金属，信号会变弱。
