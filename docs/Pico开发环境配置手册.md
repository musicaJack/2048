# Raspberry Pi Pico 开发环境配置手册（macOS）

查证日期：2026-11-11。面向本机：Apple Silicon（arm64）、macOS 26.6.2。覆盖 **RP2040**（Pico / Pico W）和 **RP2350**（Pico 2）的 C/C++ 命令行环境。本仓库的 2048 工程目前按 RP2040 编写，同一套工具链可以再编 RP2350。

官方在 2026-07-03 版《Getting started with Raspberry Pi Pico-series》里，把 VS Code 扩展当成图形界面的主路径，命令行安装说明改到附录，并指向 `pico-setup.sh`（那份脚本只适用于 Raspberry Pi OS）。在这台 Mac、以及这个已有 CMake 工程上，按下面的命令行方式安装。

## 1. 采用的版本

| 组件 | 版本 | 来源 |
|---|---|---|
| Pico SDK | **2.3.1**（2026-09-04，标签 `2.3.1`，提交 `079c6f3`） | https://github.com/raspberrypi/pico-sdk/releases/tag/2.3.1 |
| picotool | **2.3.1**（2026-09-05） | `brew install picotool` |
| Arm GNU Toolchain | **15.3.rel2**，主机包 `darwin-arm64-arm-none-eabi` | `brew install --cask gcc-arm-embedded` |
| CMake | 本机已有 **4.4.4**（要求 ≥ 3.13） | 已安装，不必重装 |
| Ninja | 随 Homebrew 安装 | `brew install ninja` |
| 原生编译器 | Xcode 命令行工具，路径 `/Applications/Xcode.app/Contents/Developer` | 本机已有 |

SDK 2.3.1 发行说明写明已支持 **GCC 15.3.1**。Homebrew cask `gcc-arm-embedded` 提供的 15.3.rel2 就是这条 Arm 裸机工具链的 Apple Silicon 原生包，和 SDK 的支持范围一致。

不要安装下面两样，它们已经过时，编 Pico 时会报 `cannot read spec file 'nosys.specs'`：

- `brew tap ArmMbed/homebrew-formulae`
- `brew install arm-none-eabi-gcc`

Apple Silicon 上也不需要为这个工具链装 Rosetta。cask 里的包名带 `darwin-arm64`，是原生 arm64。

`master` 分支等于当前稳定版。本手册固定检出标签 `2.3.1`，避免以后 `master` 向前走导致和文档不一致。

## 2. 本机在配置前缺什么

2026-11-11 检查结果：

- 已有：CMake 4.4.4、Python 3.9.6、Git 2.54.0、Xcode 命令行工具。
- 没有：`PICO_SDK_PATH`、`arm-none-eabi-gcc`、`picotool`、`ninja`。
- `~/Projects/code/MicroSDTextReader_ST7306/pico-sdk` 是另一个仓库里的残缺目录，没有 `pico_sdk_init.cmake`，也没有 `src/rp2350`。不要把它设成 `PICO_SDK_PATH`。

SDK 单独放在 `~/pico/pico-sdk`，不放进任何一个业务仓库。

## 3. 安装

### 3.1 本机已经装上的部分

2026-11-11 在这台 Mac 上实际落到这些位置：

| 组件 | 结果 |
|---|---|
| Ninja 1.13.2、picotool 2.3.1、libusb 1.0.30 | `brew install ninja picotool` 已完成 |
| Arm GNU Toolchain 15.3.Rel2（GCC 15.3.1 20260627） | 解压到 `~/pico/arm-gnu-toolchain/15.3.rel2`，没有装进 `/Applications` |
| Pico SDK 2.3.1（`079c6f3`） | `~/pico/pico-sdk` |
| `~/.zshrc` | 已写入 `PICO_SDK_PATH` 和 `PICO_TOOLCHAIN_PATH` |

Homebrew cask `gcc-arm-embedded` 的安装包下完了，但 `installer` 要用 `sudo` 写到 `/Applications`，这个终端读不到管理员密码，所以安装被中止。工具链改从同一份官方包里解出来，放到用户目录，效果和装到 `/Applications` 一样，只是路径不同。新开一个终端后，`arm-none-eabi-gcc --version` 应显示 `Arm GNU Toolchain 15.3.Rel2`。

如果以后希望改成 Homebrew 托管、并链接进 `/opt/homebrew/bin`，在自己的终端里执行（会要密码）：

```bash
brew install --cask gcc-arm-embedded
```

装成功后可以删掉 `~/.zshrc` 里的 `PICO_TOOLCHAIN_PATH` 和对应的 `PATH` 行，改用 cask 放进 `/opt/homebrew/bin` 的链接。

### 3.2 从零安装时的命令

`gcc-arm-embedded` 是 `.pkg`，会写到 `/Applications/ArmGNUToolchain/`，需要管理员密码。没有密码时用 3.3 节。

```bash
brew install ninja picotool
brew install --cask gcc-arm-embedded

mkdir -p "$HOME/pico"
git clone --branch 2.3.1 --depth 1 https://github.com/raspberrypi/pico-sdk.git "$HOME/pico/pico-sdk"
git -C "$HOME/pico/pico-sdk" submodule update --init
```

`git submodule update --init` 会拉下 TinyUSB、Mbed TLS、cyw43、lwIP 等子模块。只做浅克隆、不初始化子模块时，带 USB 或 Wi-Fi 的工程会在 CMake 阶段失败。

可选：把官方示例放在 SDK 旁边，用来验证工具链。2048 工程本身不依赖它。

```bash
git clone --branch sdk-2.3.1 --depth 1 https://github.com/raspberrypi/pico-examples.git "$HOME/pico/pico-examples"
```

示例仓库的 2.3.1 标签名是 `sdk-2.3.1`，和 SDK 仓库的 `2.3.1` 不一样。

### 3.3 没有管理员密码时安装工具链

cask 装不进 `/Applications` 时，把同一份官方包解到用户目录。包是 xar，里面的 `Payload` 是 gzip 过的 cpio，根目录就是工具链本身：

```bash
mkdir -p "$HOME/pico/toolchain-pkg" "$HOME/pico/arm-gnu-toolchain/15.3.rel2"
cd "$HOME/pico/toolchain-pkg"
xar -xf /path/to/arm-gnu-toolchain-15.3.rel2-darwin-arm64-arm-none-eabi.pkg
gzcat Payload | (cd "$HOME/pico/arm-gnu-toolchain/15.3.rel2" && cpio -idm)
```

解压完成后，编译器在 `~/pico/arm-gnu-toolchain/15.3.rel2/bin/arm-none-eabi-gcc`。按第 4 节设置 `PICO_TOOLCHAIN_PATH` 指向 `~/pico/arm-gnu-toolchain/15.3.rel2`（它的上一级是 `bin`，不是 `arm-none-eabi/bin`）。cask 装到 `/Applications` 时，这一层目录名才是 `arm-none-eabi`。

## 4. 环境变量

把下面这段加进 `~/.zshrc`：

```bash
# Raspberry Pi Pico SDK (RP2040 / RP2350)
export PICO_SDK_PATH="$HOME/pico/pico-sdk"
```

然后执行 `source ~/.zshrc`，或新开一个终端。

用 cask 装到 `/Applications` 时，它会把 `arm-none-eabi-gcc` 链接进 `/opt/homebrew/bin`。Homebrew 的 bin 已经在 PATH 里时，不必再设 `PICO_TOOLCHAIN_PATH`。

本机现在走的是用户目录里的工具链，`~/.zshrc` 里是：

```bash
export PICO_SDK_PATH="$HOME/pico/pico-sdk"
export PICO_TOOLCHAIN_PATH="$HOME/pico/arm-gnu-toolchain/15.3.rel2"
export PATH="$PICO_TOOLCHAIN_PATH/bin:$PATH"
```

如果改用 cask，编译器在 `/Applications/ArmGNUToolchain/15.3.rel2/arm-none-eabi/bin`。CMake 找不到编译器时再设：

```bash
export PICO_TOOLCHAIN_PATH="/Applications/ArmGNUToolchain/15.3.rel2/arm-none-eabi"
export PATH="$PICO_TOOLCHAIN_PATH/bin:$PATH"
```

SDK 从 2.0.0 起用 picotool 做 ELF 到 UF2 的转换。`brew install picotool` 装好的 2.3.1 在 PATH 里，CMake 会直接用它，不会每个工程再下载一份。

## 5. 安装后检查

```bash
arm-none-eabi-gcc --version    # 期望：Arm GNU Toolchain 15.3.rel2，Target: arm-none-eabi
picotool version               # 期望：picotool v2.3.1
cmake --version                # 期望：3.13 以上，本机为 4.4.4
echo "$PICO_SDK_PATH"
test -f "$PICO_SDK_PATH/pico_sdk_init.cmake" && echo "SDK OK"
test -d "$PICO_SDK_PATH/src/rp2040" && echo "RP2040 OK"
test -d "$PICO_SDK_PATH/src/rp2350" && echo "RP2350 OK"
test -d "$PICO_SDK_PATH/lib/tinyusb" && echo "TinyUSB OK"
```

`picotool version` 如果出现 `compiled without USB support`，说明编译 picotool 时没找到 libusb。Homebrew 的 bottle 默认带 USB 支持；若是自己从源码编的，先 `brew install libusb` 再重编。

## 6. RP2040 和 RP2350 怎么选

同一份 SDK、同一套 Arm 工具链。差别只在 CMake 参数。

| 目标板 | 芯片 | CMake 参数 | BOOTSEL 后的卷名 |
|---|---|---|---|
| Pico、Pico W | RP2040，双核 Cortex-M0+ | 默认，或 `-DPICO_BOARD=pico` / `pico_w` | `RPI-RP2` |
| Pico 2 | RP2350，双核 Cortex-M33 | `-DPICO_BOARD=pico2` | `RP2350` |
| Pico 2，RISC-V 核 | RP2350 Hazard3 | `-DPICO_PLATFORM=rp2350-riscv` | `RP2350` |

`-DPICO_BOARD=pico2` 会自动把平台设成 `rp2350`，一般不用再写 `-DPICO_PLATFORM`。

RISC-V 目标需要另一套编译器（`riscv32-unknown-elf-gcc`），Arm 工具链不能编它。本仓库的 2048 只用 Arm，这一步先不做。

改了 `PICO_BOARD` 或 SDK 版本之后，删掉原来的 `build` 目录再配置。SDK 发行说明要求升级后不要复用旧的 CMake 缓存。

## 7. 用官方 blink 验证

```bash
cd "$HOME/pico/pico-examples"
rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DPICO_BOARD=pico
cmake --build build --target blink
```

成功时生成：

```text
build/blink/blink.uf2
```

编 Pico 2 时换成另一个构建目录，避免和 RP2040 的缓存混在一起：

```bash
cmake -S . -B build-pico2 -G Ninja -DCMAKE_BUILD_TYPE=Release -DPICO_BOARD=pico2
cmake --build build-pico2 --target blink
```

配置日志里应能看到类似字样：`PICO_PLATFORM` 为 `rp2040` 或 `rp2350`，C 编译器为 GNU（`arm-none-eabi-gcc`）。中间出现 AppleClang 是正常的：`pioasm`、picotool 这些在 Mac 上跑的小工具用本机编译器，固件本身用 Arm GCC。

## 8. 接到本仓库的 2048 工程

`CMakeLists.txt` 在 `project()` 之前包含：

```cmake
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
```

这和 SDK 2.3.1 的文件布局一致，`external/pico_sdk_import.cmake` 仍在。环境变量设好后，在仓库根目录：

```bash
# ILI9488 彩色屏（CMake 默认也是这个）
cmake -S . -B build-ili9488 -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DUSE_ILI9488_DISPLAY=ON -DUSE_ST7306_DISPLAY=OFF
cmake --build build-ili9488 --target game2048_unified

# ST7306 反射屏。必须单独一个构建目录
cmake -S . -B build-st7306 -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DUSE_ST7306_DISPLAY=ON -DUSE_ILI9488_DISPLAY=OFF
cmake --build build-st7306 --target game2048_unified

# RP2350 + ST7306。再开一个目录，不要复用 build-st7306 的缓存
cmake -S . -B build-st7306-rp2350 -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DPICO_BOARD=pico2 \
  -DUSE_ST7306_DISPLAY=ON -DUSE_ILI9488_DISPLAY=OFF
cmake --build build-st7306-rp2350 --target game2048_unified
```

两个显示选项不能同时为 ON。产物：

```text
build-ili9488/game2048_unified.uf2          # RP2040
build-st7306/game2048_unified.uf2           # RP2040
build-st7306-rp2350/Game2350_st7306.uf2       # RP2350，文件名以 Game2350 开头
```

不写 `PICO_BOARD` 时，SDK 默认是 `pico`（RP2040）。那份 UF2 的家族号是 `0xe48bff56`，RP2350 的引导程序会忽略它：文件拷进去了，板子没有任何变化。RP2350 用上面的 `build-st7306-rp2350`，家族号是 `0xe48bff59`。

ST7306 这块板上的摇杆不是 I2C。X 接 GPIO26（ADC0），Y 接 GPIO27（ADC1），按键接 GPIO22，内部上拉，按下为低。中心大约在 2048，死区 300。ILI9488 仍用 GPIO6、GPIO7 上地址 `0x63` 的 I2C 摇杆。

## 9. 烧录

1. 按住板子上的 BOOTSEL，再插上 USB。
2. 桌面出现 `RPI-RP2`（RP2040）或 `RP2350`（RP2350）。
3. 任选一种：

```bash
cp build-ili9488/game2048_unified.uf2 /Volumes/RPI-RP2/
# RP2350 的卷名是 RP2350。macOS 的 cp 会带上扩展属性，拷贝可能失败。
# 用重定向写进去，卷消失表示引导程序收下了这份固件。
cat build-st7306-rp2350/Game2350_st7306.uf2 > /Volumes/RP2350/Game2350_st7306.uf2
# 或
picotool load -f build-ili9488/game2048_unified.uf2
picotool reboot
```

`picotool info` 可以在 BOOTSEL 模式下看芯片型号。macOS 上如果提示找不到设备，先确认卷已经挂载。

本工程打开了 USB 串口、关闭了 UART 串口（`pico_enable_stdio_usb`）。烧录后用系统自带的串口或 `picotool` 看日志，设备名一般是 `/dev/cu.usbmodem*`。

## 10. 以后升级 SDK

```bash
git -C "$HOME/pico/pico-sdk" fetch --tags
git -C "$HOME/pico/pico-sdk" checkout <新标签>
git -C "$HOME/pico/pico-sdk" submodule update --init
brew upgrade picotool
```

然后删掉各工程的 `build*` 目录，重新 `cmake -S . -B build`。picotool 主版本要和 SDK 对齐，2.3.1 对 2.3.1。

## 11. 常见问题

**CMake 说 `PICO_SDK_PATH` 未设置，或目录里没有 SDK。**  
新开终端后再试。`echo "$PICO_SDK_PATH"` 应是 `/Users/shmusica.mj/pico/pico-sdk`，且该目录下有 `pico_sdk_init.cmake`。

**`cannot read spec file 'nosys.specs'`。**  
PATH 里还留着旧的 `arm-none-eabi-gcc` formula。卸掉它和连带的 binutils，再装 cask：

```bash
brew uninstall arm-none-eabi-gcc
brew autoremove
brew install --cask gcc-arm-embedded
```

**cask 安装报 `/opt/homebrew/bin/arm-none-eabi-as` 已存在。**  
同样是旧 formula 没清干净，按上一条卸掉后再装。

**子模块不完整，`lib/tinyusb` 是空目录。**  
在 SDK 目录执行 `git submodule update --init`。

**想用 VS Code / Cursor 扩展代替命令行。**  
扩展 ID 是 `raspberry-pi.raspberry-pi-pico`。它会把 SDK 和工具链下载到 `~/.pico-sdk/`，和本手册的 `~/pico/pico-sdk` 是两套位置。这个仓库已经有自己的 `CMakeLists.txt`，命令行环境足够，不必两套同时启用，以免 `PICO_SDK_PATH` 指错地方。

## 12. 参考

- Pico SDK 2.3.1：https://github.com/raspberrypi/pico-sdk/releases/tag/2.3.1
- SDK README：https://github.com/raspberrypi/pico-sdk/blob/master/README.md
- 导入脚本（仍位于 `external/pico_sdk_import.cmake`）：https://github.com/raspberrypi/pico-sdk/blob/master/external/pico_sdk_import.cmake
- picotool 2.3.1：https://github.com/raspberrypi/picotool/releases/tag/2.3.1
- picotool 编译说明：https://github.com/raspberrypi/picotool/blob/master/BUILDING.md
- 示例与 `PICO_BOARD` / `PICO_PLATFORM`：https://github.com/raspberrypi/pico-examples/blob/master/README.md
- 《Getting started with Raspberry Pi Pico-series》，2026-07-03 构建：https://pip-assets.raspberrypi.com/categories/610-raspberry-pi-pico/documents/RP-008276-DS-2-getting-started-with-pico.pdf
- Homebrew cask `gcc-arm-embedded`：https://formulae.brew.sh/cask/gcc-arm-embedded
- Homebrew formula `picotool`：https://formulae.brew.sh/formula/picotool
- 废弃的 ArmMbed formula 会导致 `nosys.specs` 错误：https://github.com/raspberrypi/pico-sdk/issues/1529
