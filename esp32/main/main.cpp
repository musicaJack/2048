#include "pins.hpp"
#include "ili9488_panel.hpp"
#include "board_input.hpp"
#include "game/game_2048.hpp"

#include <cstdlib>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "game2048";

namespace {

struct Rgb {
    uint8_t r, g, b;
};

constexpr Rgb BG{0, 0, 0};
constexpr Rgb INK{0, 0, 0};
constexpr Rgb LIGHT{255, 255, 255};
constexpr Rgb FRAME{255, 0, 0};
constexpr Rgb CELL{60, 58, 50};
constexpr int kBorder = 3;

Rgb tileColor(int value) {
    switch (value) {
        case 2: return {238, 228, 218};
        case 4: return {237, 224, 200};
        case 8: return {242, 177, 121};
        case 16: return {245, 149, 99};
        case 32: return {246, 124, 95};
        case 64: return {246, 94, 59};
        case 128: return {237, 207, 114};
        case 256: return {237, 204, 97};
        case 512: return {237, 200, 80};
        case 1024: return {237, 197, 63};
        default: return value >= 2048 ? Rgb{237, 194, 46} : CELL;
    }
}

// 3x5 数字点阵，bit2 是左列。
constexpr uint8_t DIGIT[10][5] = {
    {0x7, 0x5, 0x5, 0x5, 0x7},
    {0x2, 0x6, 0x2, 0x2, 0x7},
    {0x7, 0x1, 0x7, 0x4, 0x7},
    {0x7, 0x1, 0x7, 0x1, 0x7},
    {0x5, 0x5, 0x7, 0x1, 0x1},
    {0x7, 0x4, 0x7, 0x1, 0x7},
    {0x7, 0x4, 0x7, 0x5, 0x7},
    {0x7, 0x1, 0x2, 0x2, 0x2},
    {0x7, 0x5, 0x7, 0x5, 0x7},
    {0x7, 0x5, 0x7, 0x1, 0x7},
};

void drawDigit(Ili9488Panel& lcd, int x, int y, int digit, int scale, Rgb fg, Rgb bg) {
    const int gw = 3 * scale;
    const int gh = 5 * scale;
    static uint8_t buf[3 * 8 * 5 * 8 * 3];
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            const bool on = (DIGIT[digit][row] & (1 << (2 - col))) != 0;
            const Rgb c = on ? fg : bg;
            for (int dy = 0; dy < scale; ++dy) {
                for (int dx = 0; dx < scale; ++dx) {
                    const int px = (row * scale + dy) * gw + col * scale + dx;
                    buf[px * 3] = c.r;
                    buf[px * 3 + 1] = c.g;
                    buf[px * 3 + 2] = c.b;
                }
            }
        }
    }
    lcd.blit(x, y, gw, gh, buf);
}

int numberWidth(int value, int scale) {
    int digits = 1;
    for (int v = value; v >= 10; v /= 10) {
        ++digits;
    }
    return digits * (3 * scale + scale) - scale;
}

int drawNumber(Ili9488Panel& lcd, int x, int y, int value, int scale, Rgb fg, Rgb bg) {
    char text[12];
    int n = 0;
    int v = value;
    if (v == 0) {
        text[n++] = '0';
    } else {
        char rev[12];
        int r = 0;
        while (v > 0 && r < 11) {
            rev[r++] = static_cast<char>('0' + (v % 10));
            v /= 10;
        }
        while (r > 0) {
            text[n++] = rev[--r];
        }
    }
    const int advance = 3 * scale + scale;
    for (int i = 0; i < n; ++i) {
        drawDigit(lcd, x + i * advance, y, text[i] - '0', scale, fg, bg);
    }
    return n * advance - scale;
}

constexpr int kCell = 68;
constexpr int kGap = 8;
constexpr int kBoardPx = kCell * 4 + kGap * 3;
constexpr int kBottomMargin = 12;
constexpr int kBoardY = Ili9488Panel::HEIGHT - kBottomMargin - kGap - kBoardPx;

int boardOriginX() {
    return (Ili9488Panel::WIDTH - kBoardPx) / 2;
}

constexpr int kFrame = kGap;
constexpr int kImageW = kBoardPx + kFrame * 2;
constexpr int kImageH = kImageW;

void fillImage(uint8_t* img, int x, int y, int w, int h, Rgb c) {
    for (int row = 0; row < h; ++row) {
        uint8_t* dst = img + (static_cast<size_t>(y + row) * kImageW + x) * 3;
        for (int col = 0; col < w; ++col) {
            dst[col * 3] = c.r;
            dst[col * 3 + 1] = c.g;
            dst[col * 3 + 2] = c.b;
        }
    }
}

void drawImageDigit(uint8_t* img, int x, int y, int digit, int scale, Rgb fg, Rgb bg) {
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            const bool on = (DIGIT[digit][row] & (1 << (2 - col))) != 0;
            fillImage(img, x + col * scale, y + row * scale, scale, scale, on ? fg : bg);
        }
    }
}

void drawImageNumber(uint8_t* img, int x, int y, int value, int scale, Rgb fg, Rgb bg) {
    char text[12];
    int n = 0;
    int v = value;
    if (v == 0) {
        text[n++] = '0';
    } else {
        char rev[12];
        int r = 0;
        while (v > 0 && r < 11) {
            rev[r++] = static_cast<char>('0' + (v % 10));
            v /= 10;
        }
        while (r > 0) {
            text[n++] = rev[--r];
        }
    }
    const int advance = 3 * scale + scale;
    for (int i = 0; i < n; ++i) {
        drawImageDigit(img, x + i * advance, y, text[i] - '0', scale, fg, bg);
    }
}

uint8_t* boardImage() {
    static uint8_t* img = nullptr;
    if (img != nullptr) {
        return img;
    }
    const size_t bytes = static_cast<size_t>(kImageW) * kImageH * 3;
    img = static_cast<uint8_t*>(heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (img == nullptr) {
        img = static_cast<uint8_t*>(heap_caps_malloc(bytes, MALLOC_CAP_8BIT));
    }
    return img;
}

void paintBoard(Ili9488Panel& lcd, const int (&cells)[4][4]) {
    uint8_t* img = boardImage();
    if (img == nullptr) {
        return;
    }
    fillImage(img, 0, 0, kImageW, kImageH, BG);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            const int value = cells[row][col];
            const int x = kFrame + col * (kCell + kGap);
            const int y = kFrame + row * (kCell + kGap);
            fillImage(img, x, y, kCell, kBorder, FRAME);
            fillImage(img, x, y + kCell - kBorder, kCell, kBorder, FRAME);
            fillImage(img, x, y, kBorder, kCell, FRAME);
            fillImage(img, x + kCell - kBorder, y, kBorder, kCell, FRAME);
            fillImage(img, x + kBorder, y + kBorder, kCell - kBorder * 2, kCell - kBorder * 2, INK);
            if (value <= 0) {
                continue;
            }
            const int scale = value >= 1000 ? 4 : 6;
            const int text_w = numberWidth(value, scale);
            const int text_h = 5 * scale;
            drawImageNumber(img, x + (kCell - text_w) / 2, y + (kCell - text_h) / 2, value, scale,
                            tileColor(value), INK);
        }
    }
    lcd.blit(boardOriginX() - kFrame, kBoardY - kFrame, kImageW, kImageH, img);
}

void drawScore(Ili9488Panel& lcd, int score) {
    lcd.fillRect(140, 28, Ili9488Panel::WIDTH - 148, 28, BG.r, BG.g, BG.b);
    const int score_width = numberWidth(score, 4);
    drawNumber(lcd, Ili9488Panel::WIDTH - 16 - score_width, 32, score, 4, LIGHT, BG);
}

struct Frame {
    int cells[4][4] = {};
    int score = -1;
    bool ready = false;
};

Frame g_frame;

void drawBoard(Ili9488Panel& lcd, const game2048::GameState& state, bool full) {
    const auto& cells = state.getBoard();
    const int score = static_cast<int>(state.getCurrentScore());
    if (full || !g_frame.ready) {
        lcd.fillScreen(BG.r, BG.g, BG.b);
        drawNumber(lcd, 16, 28, 2048, 6, LIGHT, BG);
        drawScore(lcd, score);
        paintBoard(lcd, cells);
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                g_frame.cells[row][col] = cells[row][col];
            }
        }
        g_frame.score = score;
        g_frame.ready = true;
        return;
    }

    int changed = 0;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (g_frame.cells[row][col] != cells[row][col]) {
                ++changed;
            }
            g_frame.cells[row][col] = cells[row][col];
        }
    }
    if (changed > 0) {
        paintBoard(lcd, cells);
    }
    if (score != g_frame.score) {
        drawScore(lcd, score);
        g_frame.score = score;
    }
    ESP_LOGI(TAG, "partial %d cells, score %d", changed, score);
}

bool applyMove(game2048::Game2048& game, MoveDir dir) {
    if (game.isGameOver() || dir == MoveDir::None) {
        return false;
    }
    switch (dir) {
        case MoveDir::Left: return game.moveLeft();
        case MoveDir::Right: return game.moveRight();
        case MoveDir::Up: return game.moveUp();
        case MoveDir::Down: return game.moveDown();
        default: return false;
    }
}

MoveDir stickDirection(int16_t x, int16_t y) {
    const int ax = x < 0 ? -x : x;
    const int ay = y < 0 ? -y : y;
    if (ax < JOYSTICK_DEADZONE && ay < JOYSTICK_DEADZONE) {
        return MoveDir::None;
    }
    if (ax < JOYSTICK_THRESHOLD && ay < JOYSTICK_THRESHOLD) {
        return MoveDir::None;
    }
    if (ax >= ay) {
        return x > 0 ? MoveDir::Right : MoveDir::Left;
    }
    return y > 0 ? MoveDir::Down : MoveDir::Up;
}

}  // namespace

extern "C" void app_main(void) {
    std::srand(static_cast<unsigned>(esp_timer_get_time()));

    Ili9488Panel lcd;
    const LcdPins lcd_pins{PIN_LCD_SCK, PIN_LCD_MOSI, PIN_LCD_CS, PIN_LCD_DC,
                           PIN_LCD_RST, PIN_LCD_BL, SPI_HZ_BRINGUP};
    const esp_err_t lcd_err = lcd.begin(lcd_pins);
    if (lcd_err != ESP_OK) {
        ESP_LOGE(TAG, "panel idle: %s", esp_err_to_name(lcd_err));
    }

    BoardInput input;
    const InputPins input_pins{PIN_JOY_SDA, PIN_JOY_SCL, PIN_PAGE_UP, PIN_PAGE_DOWN,
                               PIN_SCREEN, JOYSTICK_ADDR, I2C_HZ,
                               PIN_EXP_SDA, PIN_EXP_SCL, PIN_EXP_SCREEN_BIT};
    ESP_ERROR_CHECK(input.begin(input_pins));
    ESP_LOGI(TAG, "joystick %s, firmware 0x%02x",
             input.joystickReady() ? "ready" : "missing", input.firmwareVersion());

    game2048::Game2048 game;
    game.initialize();
    ESP_LOGI(TAG, "draw board");
    drawBoard(lcd, game.getGameState(), true);
    ESP_LOGI(TAG, "board drawn, score %u", game.getGameState().getCurrentScore());

    MoveDir held = MoveDir::None;
    int64_t last_move_us = 0;
    bool stick_was_down = false;

    while (true) {
        int16_t x = 0;
        int16_t y = 0;
        input.readStick(x, y);
        const MoveDir dir = stickDirection(x, y);
        const int64_t now = esp_timer_get_time();
        const bool repeat = dir != MoveDir::None && dir == held && (now - last_move_us) >= 200000;
        const bool fresh = dir != MoveDir::None && dir != held;
        if (fresh || repeat) {
            if (applyMove(game, dir)) {
                drawBoard(lcd, game.getGameState(), false);
            }
            held = dir;
            last_move_us = now;
        } else if (dir == MoveDir::None) {
            held = MoveDir::None;
        }

        const bool stick_down = input.stickPressed();
        if (stick_down && !stick_was_down) {
            game.reset();
            drawBoard(lcd, game.getGameState(), true);
            ESP_LOGI(TAG, "reset");
        }
        stick_was_down = stick_down;

        bool up = false;
        bool down = false;
        bool screen = false;
        input.poll(BUTTON_DEBOUNCE_MS, BUTTON_LONG_PRESS_MS, up, down, screen);
        if (up && applyMove(game, MoveDir::Up)) {
            drawBoard(lcd, game.getGameState(), false);
        }
        if (down && applyMove(game, MoveDir::Down)) {
            drawBoard(lcd, game.getGameState(), false);
        }
        if (screen) {
            lcd.setBacklight(!lcd.backlightOn());
            ESP_LOGI(TAG, "backlight %s", lcd.backlightOn() ? "on" : "off");
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
