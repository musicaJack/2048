#pragma once

#include <cstdint>
#include "esp_err.h"

struct LcdPins {
    int sck;
    int mosi;
    int cs;
    int dc;
    int rst;
    int bl;
    int hz;
};

class Ili9488Panel {
public:
    static constexpr int WIDTH = 320;
    static constexpr int HEIGHT = 480;

    esp_err_t begin(const LcdPins& pins);
    void setBacklight(bool on);
    bool backlightOn() const { return backlight_on_; }

    void fillRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
    void fillScreen(uint8_t r, uint8_t g, uint8_t b);
    // rgb 是紧排的 R,G,B，长度 w*h*3。一次开窗写完，避免每个点单独传输。
    void blit(int x, int y, int w, int h, const uint8_t* rgb);

private:
    void writeCommand(uint8_t cmd);
    void writeData(const uint8_t* data, int len);
    void writeData(const uint8_t* data, int len, int dc_level);
    void setWindow(int x0, int y0, int x1, int y1);

    int dc_ = -1;
    int bl_ = -1;
    bool backlight_on_ = false;
    void* spi_ = nullptr;
    uint8_t* line_ = nullptr;
};
