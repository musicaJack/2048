#include <cstdio>
#include <cstring>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/spi.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

#include "hardware/display/ili9488/ili9488_driver.hpp"
#include "hardware/display/ili9488/ili9488_colors.hpp"
#include "hardware/display/ili9488/ili9488_font.hpp"

#ifndef WIFI_SSID
#error WIFI_SSID is not set
#endif
#ifndef WIFI_PASSWORD
#error WIFI_PASSWORD is not set
#endif

namespace {
constexpr uint8_t PIN_DC = 20;
constexpr uint8_t PIN_RST = 15;
constexpr uint8_t PIN_CS = 17;
constexpr uint8_t PIN_SCK = 18;
constexpr uint8_t PIN_MOSI = 19;
constexpr uint8_t PIN_BL = 16;
constexpr uint32_t SPI_HZ = 40000000;

constexpr uint32_t RGB_WHITE = 0xFCFCFC;
constexpr uint32_t RGB_BLACK = 0x000000;
constexpr uint32_t RGB_GREEN = 0x00FC00;
constexpr uint32_t RGB_YELLOW = 0xFCFC00;
constexpr uint32_t RGB_RED = 0xFC0000;
constexpr uint16_t RGB565_WHITE = ili9488_colors::rgb565::WHITE;
constexpr uint16_t RGB565_BLACK = ili9488_colors::rgb565::BLACK;

ili9488::ILI9488Driver* g_lcd = nullptr;
bool g_found_target = false;

void draw_string_scaled(uint16_t x, uint16_t y, const char* str, uint8_t scale,
                        uint16_t fg, uint16_t bg) {
    using namespace font;
    if (!g_lcd || scale == 0) {
        return;
    }
    for (const char* p = str; *p; ++p) {
        const uint8_t* data = get_char_data(*p);
        for (uint8_t row = 0; row < FONT_HEIGHT; ++row) {
            uint8_t byte = data[row];
            for (uint8_t col = 0; col < FONT_WIDTH; ++col) {
                bool on = (byte >> (7 - col)) & 0x01;
                uint16_t x0 = x + col * scale;
                uint16_t y0 = y + row * scale;
                g_lcd->fillArea(x0, y0, x0 + scale - 1, y0 + scale - 1, on ? fg : bg);
            }
        }
        x += FONT_WIDTH * scale;
    }
}

void lcd_line(uint16_t x, uint16_t y, const char* text, uint32_t color) {
    if (!g_lcd) {
        return;
    }
    g_lcd->drawString(x, y, text, color, RGB_BLACK);
}

void lcd_status(const char* status, uint32_t color) {
    if (!g_lcd) {
        return;
    }
    g_lcd->fillArea(16, 120, 319, 136, RGB565_BLACK);
    lcd_line(16, 120, status, color);
    printf("%s\n", status);
}

bool init_lcd() {
    static ili9488::ILI9488Driver lcd(spi0, PIN_DC, PIN_RST, PIN_CS, PIN_SCK, PIN_MOSI, PIN_BL, SPI_HZ);
    if (!lcd.initialize()) {
        printf("ILI9488 init failed\n");
        return false;
    }
    lcd.setBacklight(true);
    lcd.setBacklightBrightness(180);
    lcd.fillScreen(RGB565_BLACK);
    g_lcd = &lcd;
    draw_string_scaled(16, 32, "Wi-Fi", 3, RGB565_WHITE, RGB565_BLACK);
    lcd_line(16, 96, "RP2350B-Plus-W", RGB_WHITE);
    lcd_line(16, 160, "SSID:", RGB_WHITE);
    lcd_line(64, 160, WIFI_SSID, RGB_YELLOW);
    lcd_status("Connecting...", RGB_YELLOW);
    return true;
}

int scan_result(void* env, const cyw43_ev_scan_result_t* result) {
    (void)env;
    if (!result) {
        return 0;
    }
    printf("ssid: %-32s rssi: %4d chan: %3d auth: %u\n",
           result->ssid, result->rssi, result->channel, result->auth_mode);
    if (strncmp(reinterpret_cast<const char*>(result->ssid), WIFI_SSID, sizeof(result->ssid)) == 0) {
        g_found_target = true;
    }
    return 0;
}

const char* auth_name(uint32_t auth) {
    switch (auth) {
        case CYW43_AUTH_WPA2_AES_PSK: return "WPA2-AES";
        case CYW43_AUTH_WPA2_MIXED_PSK: return "WPA2-MIXED";
        case CYW43_AUTH_WPA3_WPA2_AES_PSK: return "WPA3/WPA2";
        default: return "unknown";
    }
}

void show_ip(const char* ip) {
    if (!g_lcd) {
        return;
    }
    lcd_status("Connected", RGB_GREEN);
    lcd_line(16, 220, "IP:", RGB_WHITE);
    g_lcd->fillArea(0, 248, 319, 312, RGB565_BLACK);
    uint16_t width = static_cast<uint16_t>(strlen(ip) * font::FONT_WIDTH * 2);
    uint16_t x = width < 320 ? static_cast<uint16_t>((320 - width) / 2) : 8;
    draw_string_scaled(x, 250, ip, 2, RGB565_WHITE, RGB565_BLACK);
}

}  // namespace

int main() {
    stdio_init_all();
    init_lcd();

    printf("\nRP2350B-Plus-W Wi-Fi station test\n");
    printf("SSID: %s\n", WIFI_SSID);

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_CHINA)) {
        lcd_status("cyw43 init failed", RGB_RED);
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    lcd_status("Scanning...", RGB_YELLOW);
    cyw43_wifi_scan_options_t scan_options = {};
    if (cyw43_wifi_scan(&cyw43_state, &scan_options, nullptr, scan_result) == 0) {
        while (cyw43_wifi_scan_active(&cyw43_state)) {
            sleep_ms(200);
        }
    } else {
        lcd_status("Scan failed, connecting", RGB_YELLOW);
    }

    if (g_found_target) {
        lcd_status("SSID found", RGB_GREEN);
    } else {
        lcd_status("SSID not in scan", RGB_YELLOW);
    }

    const uint32_t auths[] = {
        CYW43_AUTH_WPA2_AES_PSK,
        CYW43_AUTH_WPA2_MIXED_PSK,
        CYW43_AUTH_WPA3_WPA2_AES_PSK,
    };

    bool connected = false;
    for (uint32_t auth : auths) {
        char line[40];
        snprintf(line, sizeof(line), "Auth %s", auth_name(auth));
        lcd_status(line, RGB_YELLOW);
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, auth, 20000) == 0) {
            connected = true;
            break;
        }
    }

    if (!connected) {
        lcd_status("Connect failed", RGB_RED);
        while (true) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(150);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(150);
        }
    }

    char ip[16] = "0.0.0.0";
    for (int i = 0; i < 50; ++i) {
        cyw43_arch_lwip_begin();
        const ip4_addr_t* addr = netif_ip4_addr(netif_default);
        bool have_ip = addr && addr->addr != 0;
        if (have_ip) {
            strncpy(ip, ip4addr_ntoa(addr), sizeof(ip) - 1);
        }
        cyw43_arch_lwip_end();
        if (have_ip) {
            break;
        }
        sleep_ms(100);
    }

    printf("Connected. IP %s\n", ip);
    show_ip(ip);

    char last_ip[16];
    strncpy(last_ip, ip, sizeof(last_ip));
    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(800);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(800);

        cyw43_arch_lwip_begin();
        const char* now = ip4addr_ntoa(netif_ip4_addr(netif_default));
        int link = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
        printf("link=%d ip=%s\n", link, now);
        if (strcmp(now, last_ip) != 0) {
            strncpy(last_ip, now, sizeof(last_ip) - 1);
            last_ip[sizeof(last_ip) - 1] = '\0';
            show_ip(last_ip);
        }
        cyw43_arch_lwip_end();
    }
}
