#include "display/adapters/st7306_adapter.hpp"
#include "hardware/display/st7306/st7306_driver.hpp"
#include "hardware/display/st7306/st73xx_font.hpp"
#include <cstring>

namespace game2048 {

ST7306Adapter::ST7306Adapter(const DisplayConfig& config) 
    : config_(config), driver_(nullptr) {
}

ST7306Adapter::~ST7306Adapter() {
    deinit();
}

bool ST7306Adapter::init() {
    // 创建ST7306驱动实例
    driver_ = new st7306::ST7306Driver(
        config_.pins.dc, 
        config_.pins.rst, 
        config_.pins.cs, 
        config_.pins.sclk, 
        config_.pins.mosi
    );
    if (!driver_) {
        return false;
    }
    
    // 初始化驱动
    driver_->initialize();
    
    // 设置旋转
    setRotation(config_.rotation);
    
    // 清屏并显示
    driver_->clearDisplay();
    driver_->display();
    
    return true;
}

void ST7306Adapter::deinit() {
    if (driver_) {
        delete driver_;
        driver_ = nullptr;
    }
}

void ST7306Adapter::clear() {
    if (driver_) {
        driver_->clearDisplay();
        driver_->display();
    }
}

void ST7306Adapter::clear(const Color& color) {
    if (driver_) {
        uint8_t gray = rgbToGrayscale(color);
        // 使用fill方法填充整个屏幕
        driver_->fill(gray);
        driver_->display();
    }
}

void ST7306Adapter::display() {
    if (driver_) {
        driver_->display();
    }
}

void ST7306Adapter::drawPixel(int16_t x, int16_t y, const Color& color) {
    if (!driver_) return;
    
    transformCoordinates(x, y);
    if (x >= 0 && x < getWidth() && y >= 0 && y < getHeight()) {
        uint8_t gray = rgbToGrayscale(color);
        driver_->drawPixelGray(x, y, gray);
    }
}

void ST7306Adapter::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const Color& color) {
    if (!driver_) return;
    
    uint8_t gray = rgbToGrayscale(color);
    
    // 简单的Bresenham直线算法
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx - dy;
    
    while (true) {
        drawPixel(x0, y0, color);
        
        if (x0 == x1 && y0 == y1) break;
        
        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ST7306Adapter::drawRect(int16_t x, int16_t y, int16_t width, int16_t height, const Color& color) {
    if (!driver_) return;
    
    drawLine(x, y, x + width - 1, y, color);
    drawLine(x + width - 1, y, x + width - 1, y + height - 1, color);
    drawLine(x + width - 1, y + height - 1, x, y + height - 1, color);
    drawLine(x, y + height - 1, x, y, color);
}

void ST7306Adapter::fillRect(int16_t x, int16_t y, int16_t width, int16_t height, const Color& color) {
    if (!driver_) return;
    
    uint8_t gray = rgbToGrayscale(color);
    for (int16_t i = 0; i < height; i++) {
        for (int16_t j = 0; j < width; j++) {
            int16_t px = x + j;
            int16_t py = y + i;
            transformCoordinates(px, py);
            if (px >= 0 && px < getWidth() && py >= 0 && py < getHeight()) {
                driver_->drawPixelGray(px, py, gray);
            }
        }
    }
}

void ST7306Adapter::drawCircle(int16_t x, int16_t y, int16_t radius, const Color& color) {
    if (!driver_) return;
    
    // 简单的圆形绘制算法
    for (int16_t i = -radius; i <= radius; i++) {
        for (int16_t j = -radius; j <= radius; j++) {
            if (i * i + j * j <= radius * radius && i * i + j * j > (radius - 1) * (radius - 1)) {
                drawPixel(x + i, y + j, color);
            }
        }
    }
}

void ST7306Adapter::fillCircle(int16_t x, int16_t y, int16_t radius, const Color& color) {
    if (!driver_) return;
    
    for (int16_t i = -radius; i <= radius; i++) {
        for (int16_t j = -radius; j <= radius; j++) {
            if (i * i + j * j <= radius * radius) {
                drawPixel(x + i, y + j, color);
            }
        }
    }
}

void ST7306Adapter::drawChar(int16_t x, int16_t y, char c, const Color& color, uint8_t size) {
    if (!driver_) return;
    
    // 简单的字符绘制
    uint8_t gray = rgbToGrayscale(color);
    bool color_bool = (gray > 1); // 转换为布尔值
    driver_->drawChar(x, y, c, color_bool);
}

void ST7306Adapter::drawString(int16_t x, int16_t y, const std::string& text, const Color& color, uint8_t size) {
    if (!driver_) return;
    
    int16_t current_x = x;
    for (char c : text) {
        drawChar(current_x, y, c, color, size);
        current_x += 8 * size; // 8像素宽度
    }
}

void ST7306Adapter::drawString(int16_t x, int16_t y, const std::string& text, const Color& color, const Color& bgColor, uint8_t size) {
    if (!driver_) return;
    
    // 对于ST7306，背景色在灰度显示中不太重要，使用默认实现
    drawString(x, y, text, color, size);
}

uint16_t ST7306Adapter::getWidth() const {
    // ST7306的分辨率是300x400
    return st7306::ST7306Driver::LCD_WIDTH;
}

uint16_t ST7306Adapter::getHeight() const {
    return st7306::ST7306Driver::LCD_HEIGHT;
}

void ST7306Adapter::setRotation(uint8_t rotation) {
    config_.rotation = rotation;
    if (driver_) {
        driver_->setRotation(rotation);
    }
}

uint8_t ST7306Adapter::getRotation() const {
    return config_.rotation;
}

uint16_t ST7306Adapter::colorToNative(const Color& color) {
    return rgbToGrayscale(color);
}

Color ST7306Adapter::nativeToColor(uint16_t native_color) {
    uint8_t gray = static_cast<uint8_t>(native_color);
    uint8_t rgb = gray * 255 / 3; // 转换回RGB
    return Color(rgb, rgb, rgb);
}

void ST7306Adapter::setBrightness(uint8_t brightness) {
    config_.brightness = brightness;
    // ST7306不支持亮度调节，这里可以留空或实现其他控制
}

uint8_t ST7306Adapter::getBrightness() const {
    return config_.brightness;
}

void* ST7306Adapter::getBuffer() {
    // ST7306驱动没有直接的getBuffer方法，返回nullptr
    return nullptr;
}

// Game2048 optimized font drawing methods
void ST7306Adapter::drawStringGame2048(int16_t x, int16_t y, const std::string& text, const Color& color, uint8_t size) {
    if (!driver_) return;
    
    uint8_t gray = rgbToGrayscale(color);
    int16_t currentX = x;
    
    for (char c : text) {
        const uint8_t* fontData = font::get_game2048_char_data(c);
        if (!fontData) continue;
        
        // Draw character using Game2048 font (12x16)
        for (int row = 0; row < font::GAME2048_FONT_HEIGHT; ++row) {
            uint8_t rowData = fontData[row];
            
            for (int col = 0; col < font::GAME2048_FONT_WIDTH; ++col) {
                if (rowData & (0x80 >> col)) {
                    // Draw pixel with scaling
                    for (int sx = 0; sx < size; ++sx) {
                        for (int sy = 0; sy < size; ++sy) {
                            int16_t px = currentX + col * size + sx;
                            int16_t py = y + row * size + sy;
                            transformCoordinates(px, py);
                            if (px >= 0 && px < getWidth() && py >= 0 && py < getHeight()) {
                                driver_->drawPixelGray(px, py, gray);
                            }
                        }
                    }
                }
            }
        }
        
        currentX += font::GAME2048_FONT_WIDTH * size;
    }
    
    // 刷新屏幕显示
    driver_->display();
}

void ST7306Adapter::drawStringGame2048(int16_t x, int16_t y, const std::string& text, const Color& color, const Color& bgColor, uint8_t size) {
    if (!driver_) return;
    
    uint8_t textGray = rgbToGrayscale(color);
    uint8_t bgGray = rgbToGrayscale(bgColor);
    int16_t currentX = x;
    
    for (char c : text) {
        const uint8_t* fontData = font::get_game2048_char_data(c);
        if (!fontData) continue;
        
        // Draw character using Game2048 font (12x16) with background
        for (int row = 0; row < font::GAME2048_FONT_HEIGHT; ++row) {
            uint8_t rowData = fontData[row];
            
            for (int col = 0; col < font::GAME2048_FONT_WIDTH; ++col) {
                uint8_t pixelGray = (rowData & (0x80 >> col)) ? textGray : bgGray;
                
                // Draw pixel with scaling
                for (int sx = 0; sx < size; ++sx) {
                    for (int sy = 0; sy < size; ++sy) {
                        int16_t px = currentX + col * size + sx;
                        int16_t py = y + row * size + sy;
                        transformCoordinates(px, py);
                        if (px >= 0 && px < getWidth() && py >= 0 && py < getHeight()) {
                            driver_->drawPixelGray(px, py, pixelGray);
                        }
                    }
                }
            }
        }
        
        currentX += font::GAME2048_FONT_WIDTH * size;
    }
    
    // 刷新屏幕显示
    driver_->display();
}

size_t ST7306Adapter::getBufferSize() const {
    // ST7306缓冲区大小
    return st7306::ST7306Driver::DISPLAY_BUFFER_LENGTH;
}

uint8_t ST7306Adapter::rgbToGrayscale(const Color& color) const {
    // 将RGB转换为灰度值 (0-3)
    // 使用加权平均：0.299*R + 0.587*G + 0.114*B
    uint16_t gray = (color.r * 299 + color.g * 587 + color.b * 114) / 1000;
    
    // 映射到ST7306的4级灰度 (0-3)
    // 注意：ST7306中 0=白色，3=黑色
    if (gray < 64) return 3;      // 黑色 (COLOR_BLACK = 0x03)
    if (gray < 128) return 2;     // 深灰 (COLOR_GRAY2 = 0x02)
    if (gray < 192) return 1;     // 浅灰 (COLOR_GRAY1 = 0x01)
    return 0;                     // 白色 (COLOR_WHITE = 0x00)
}

void ST7306Adapter::transformCoordinates(int16_t& x, int16_t& y) const {
    // 根据旋转角度转换坐标
    uint16_t width = getWidth();
    uint16_t height = getHeight();
    
    switch (config_.rotation) {
        case 1: // 90度
            {
                int16_t temp = x;
                x = y;
                y = width - 1 - temp;
            }
            break;
        case 2: // 180度
            x = width - 1 - x;
            y = height - 1 - y;
            break;
        case 3: // 270度
            {
                int16_t temp = x;
                x = height - 1 - y;
                y = temp;
            }
            break;
        default: // 0度，不需要转换
            break;
    }
}

} // namespace game2048
