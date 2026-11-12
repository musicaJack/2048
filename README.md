# 2048 Game Project - Dual Display Support

A 2048 game project based on Raspberry Pi Pico with support for both ST7306 reflective LCD and ILI9488 color TFT displays.

## Latest Updates

### 2026-11-12 - ESP32-S3-Pico

The ILI9488 build now runs on the Waveshare ESP32-S3-Pico. The IDF project lives in `esp32/` and is separate from the Pico CMake build. Pins, the flash port, and the layout are in `docs/ESP32-S3迁移手册.md`. ST7306 is still Pico-only.

Scoring follows classic 2048: the game starts at 0, and a merge adds the value of the new tile.

### 2024-12-19 - v2.0 Major Refactoring
- **Dual Display Support**: Added support for both ST7306 (4-level grayscale) and ILI9488 (color) displays
- **Unified Game Architecture**: Created modular game architecture with separate UI, input, state, and animation managers
- **Advanced Animation System**: Implemented game start animations, achievement notifications, and star reward system for ILI9488
- **Code Reorganization**: Separated configuration and functional implementation into appropriate include and src directories
- **Build System Optimization**: Cleaned up CMakeLists.txt and removed test programs
- **Compilation Scripts**: Added batch scripts for easy compilation of both display versions

## Hardware Requirements

### Option 1: ST7306 Reflective LCD Setup
- **Raspberry Pi Pico (RP2040)**
- **ST7306 Reflective LCD Display** (128×64 resolution, 4-level grayscale)
- **I2C Joystick Controller** (Address: 0x63)

### Option 2: ILI9488 Color TFT Setup
- **Raspberry Pi Pico (RP2040)**
- **ILI9488 3.5" TFT LCD Display** (320×480 resolution, RGB565 color)
- **I2C Joystick Controller** (Address: 0x63)

## Hardware Connections

### ST7306 Display Connections
```
Pico GPIO18 -> Display SCK
Pico GPIO19 -> Display MOSI
Pico GPIO17 -> Display CS
Pico GPIO20 -> Display DC
Pico GPIO15 -> Display RST
Pico 3.3V  -> Display VCC
Pico GND   -> Display GND
```

### ILI9488 Display Connections
```
Pico GPIO18 -> Display SCK
Pico GPIO19 -> Display MOSI
Pico GPIO17 -> Display CS
Pico GPIO20 -> Display DC
Pico GPIO15 -> Display RST
Pico GPIO16 -> Display BL (Backlight)
Pico 3.3V  -> Display VCC
Pico GND   -> Display GND
```

### I2C Joystick Connections (Both Setups)
```
Pico GPIO6  -> Joystick SDA
Pico GPIO7  -> Joystick SCL
Pico 3.3V   -> Joystick VCC
Pico GND    -> Joystick GND
```

## Build and Flash

### Environment Setup

1. **Install Pico SDK**
   ```bash
   git clone https://github.com/raspberrypi/pico-sdk
   cd pico-sdk
   git submodule update --init
   ```

2. **Set Environment Variables**
   ```bash
   # Windows
   set PICO_SDK_PATH=C:\path\to\pico-sdk
   
   # Linux/macOS
   export PICO_SDK_PATH=/path/to/pico-sdk
   ```

3. **Install Build Tools**
   - CMake 3.13+
   - ARM GCC Compiler
   - Visual Studio Code (recommended)

### Build Project

#### Windows Users
```bash
# Build ST7306 version
build_st7306.bat

# Build ILI9488 version
build_ili9488.bat

# Build both versions
build_all.bat

# Clean build directories
clean.bat
```

#### Linux/macOS Users
```bash
# Create build directory
mkdir build
cd build

# Configure for ST7306
cmake .. -DUSE_ST7306_DISPLAY=ON -DCMAKE_BUILD_TYPE=Release

# Configure for ILI9488
cmake .. -DUSE_ILI9488_DISPLAY=ON -DCMAKE_BUILD_TYPE=Release

# Build unified game
make game2048_unified -j4
```

### Flash Firmware

1. **Hold BOOTSEL button and insert Pico**
2. **Drag the generated firmware file to RPI-RP2 drive**
   - `game2048_unified.uf2` - Unified 2048 game program (supports both displays)

## Program Description

### Unified 2048 Game Program

The `game2048_unified` program is the main game application that automatically adapts to the selected display driver.

#### ST7306 Version Features
- **4-level grayscale rendering**: Optimized for reflective LCD characteristics
- **Game2048 optimized font**: Custom font designed for small display
- **Minimal animations**: No animations to maintain stability
- **Efficient rendering**: Partial refresh and optimized drawing routines

#### ILI9488 Version Features
- **Full color support**: RGB565 color palette with classic 2048 colors
- **Dark theme UI**: Pure black background with blue-black status bar
- **Advanced animations**: Game start flashing, achievement heartbeat effects
- **Star achievement system**: Earn stars for tile merges (16→32, 32→64, etc.)
- **Enhanced visual feedback**: Dynamic color schemes and visual rewards

#### Common Features
- **Complete 2048 game logic**: Full game implementation with score tracking
- **Joystick control**: Directional movement and center button reset
- **Adaptive layout**: Automatically adjusts to display resolution
- **Modular architecture**: Separated UI, input, state, and animation systems

## Game Controls

- **Joystick Directions**: Move tiles in corresponding direction
- **Center Button**: Reset game (works in all game states)
- **Automatic Detection**: Game automatically detects valid moves

## Achievement System (ILI9488 Only)

### Star Rewards
- **2+2=4**: 1 star
- **4+4=8**: 1 star
- **8+8=16**: 2 stars
- **16+16=32**: 3 stars
- **32+32=64**: 4 stars
- **64+64=128**: 5 stars
- **128+128=256**: 6 stars
- **256+256=512**: 7 stars
- **512+512=1024**: 8 stars
- **1024+1024=2048**: 10 stars

### Achievement Notifications
- **Tile Value Achievements**: Heartbeat animation for reaching 32, 64, 128, 256, 512, 1024, 2048
- **Visual Feedback**: Color-coded achievement notifications
- **Star Display**: Total stars shown in status bar

## Project Structure

```
2048/
├── include/                 # Header files
│   ├── config/             # Configuration management
│   ├── display/            # Display driver interfaces
│   │   └── adapters/       # Display adapters
│   ├── game/               # Game core modules
│   ├── hardware/           # Hardware drivers
│   │   ├── display/        # Display drivers
│   │   │   ├── st7306/     # ST7306 driver
│   │   │   └── ili9488/    # ILI9488 driver
│   │   └── input/          # Input device drivers
│   ├── input/              # Input processing
│   ├── ui/                 # User interface
│   └── utils/              # Utility modules
├── src/                    # Source code
│   ├── config/             # Configuration implementation
│   ├── display/            # Display management
│   ├── game/               # Game implementation
│   ├── hardware/           # Hardware implementations
│   ├── input/              # Input processing
│   ├── ui/                 # UI implementation
│   └── utils/              # Utility implementations
├── examples/               # Example programs
│   └── game_2048_unified.cpp # Main unified game program
├── build/                  # Build output
├── firmware/               # Generated firmware files
├── CMakeLists.txt          # Build configuration
├── build_st7306.bat        # ST7306 build script
├── build_ili9488.bat       # ILI9488 build script
├── build_all.bat           # Build both versions
├── clean.bat               # Clean build directories
└── README.md               # Project documentation
```

## Troubleshooting

### Display Issues

1. **ST7306 Display Problems**
   - Check SPI connections (GPIO18, 19, 17, 20, 15)
   - Verify power connections (3.3V, GND)
   - Ensure proper initialization sequence

2. **ILI9488 Display Problems**
   - Check all SPI and control pins
   - Verify backlight connection (GPIO16)
   - Confirm display rotation settings

### Joystick Issues

1. **No Response**
   - Check I2C connections (GPIO6, GPIO7)
   - Verify joystick address (0x63)
   - Check power connections

2. **Incorrect Input**
   - Calibrate joystick deadzone
   - Check I2C clock frequency
   - Verify joystick firmware

### Build Issues

1. **CMake Configuration Fails**
   - Verify PICO_SDK_PATH environment variable
   - Check Pico SDK version (requires 1.5.1+)
   - Confirm CMake version (requires 3.13+)

2. **Compilation Errors**
   - Check C++ compiler version (requires C++17 support)
   - Verify all dependencies are installed
   - Check for syntax errors in source code

## Next Steps

The project is feature-complete with the following potential enhancements:

1. **Additional Game Modes**: Time challenges, target scores, endless mode
2. **Enhanced Animations**: More sophisticated visual effects
3. **Performance Optimization**: Further rendering optimizations
4. **Feature Extensions**: Sound effects, haptic feedback, save/load functionality

## License

MIT License - see LICENSE file for details

## Contributing

Welcome to submit Issues and Pull Requests to improve the project! 
