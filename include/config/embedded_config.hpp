#pragma once

#include <string>

namespace game2048 {
namespace embedded_config {

// 编译时嵌入的配置文件内容
#ifdef DISPLAY_DRIVER_ILI9488
    static constexpr const char* ILI9488_CONFIG_JSON = R"({
  "display": {
    "driver": "ili9488",
    "rotation": 0,
    "brightness": 100
  },
  "hardware": {
    "spi": {
      "port": 0,
      "frequency": 40000000
    },
    "pins": {
      "dc": 20,
      "rst": 15,
      "cs": 17,
      "sclk": 18,
      "mosi": 19,
      "bl": 16
    }
  },
  "game": {
    "ui_scale": 1.0,
    "color_scheme": "default"
  }
})";
#endif

#ifdef DISPLAY_DRIVER_ST7306
    static constexpr const char* ST7306_CONFIG_JSON = R"({
  "display": {
    "driver": "st7306",
    "rotation": 0,
    "brightness": 100
  },
  "hardware": {
    "spi": {
      "port": 0,
      "frequency": 40000000
    },
    "pins": {
      "dc": 20,
      "rst": 15,
      "cs": 17,
      "sclk": 18,
      "mosi": 19,
      "bl": 255
    }
  },
  "game": {
    "ui_scale": 1.0,
    "color_scheme": "default"
  }
})";
#endif

// 获取当前编译的配置文件内容
inline const char* getEmbeddedConfig() {
#ifdef DISPLAY_DRIVER_ILI9488
    return ILI9488_CONFIG_JSON;
#elif defined(DISPLAY_DRIVER_ST7306)
    return ST7306_CONFIG_JSON;
#else
    return nullptr;
#endif
}

} // namespace embedded_config
} // namespace game2048
