#include "ili9488_panel.hpp"

#include <cstring>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ili9488";

static constexpr int kDmaBytes = Ili9488Panel::WIDTH * 3 * 4;

static int s_dc_pin = -1;
static int s_spi_errors = 0;
static int s_spi_bytes = 0;

static void IRAM_ATTR lcd_dc_cb(spi_transaction_t* t) {
    gpio_set_level(static_cast<gpio_num_t>(s_dc_pin), static_cast<int>(reinterpret_cast<uintptr_t>(t->user)));
}

esp_err_t Ili9488Panel::begin(const LcdPins& pins) {
    dc_ = pins.dc;
    bl_ = pins.bl;
    s_dc_pin = pins.dc;

    gpio_config_t out = {};
    out.pin_bit_mask = (1ULL << pins.dc) | (1ULL << pins.rst) | (1ULL << pins.bl);
    out.mode = GPIO_MODE_OUTPUT;
    out.pull_up_en = GPIO_PULLUP_DISABLE;
    out.pull_down_en = GPIO_PULLDOWN_DISABLE;
    out.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&out));

    gpio_set_level(static_cast<gpio_num_t>(pins.bl), 1);
    backlight_on_ = true;
    gpio_set_level(static_cast<gpio_num_t>(pins.dc), 1);
    gpio_set_level(static_cast<gpio_num_t>(pins.rst), 1);

    spi_bus_config_t bus = {};
    bus.mosi_io_num = pins.mosi;
    bus.miso_io_num = -1;
    bus.sclk_io_num = pins.sck;
    bus.quadwp_io_num = -1;
    bus.quadhd_io_num = -1;
    bus.max_transfer_sz = kDmaBytes;

    esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi bus init failed: %s", esp_err_to_name(err));
        return err;
    }

    spi_device_interface_config_t dev = {};
    dev.clock_speed_hz = pins.hz;
    dev.mode = 0;
    dev.spics_io_num = pins.cs;
    dev.queue_size = 1;
    dev.cs_ena_pretrans = 1;
    dev.cs_ena_posttrans = 1;
    dev.pre_cb = lcd_dc_cb;

    spi_device_handle_t handle = nullptr;
    err = spi_bus_add_device(SPI2_HOST, &dev, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi device add failed: %s", esp_err_to_name(err));
        return err;
    }
    spi_ = handle;

    line_ = static_cast<uint8_t*>(heap_caps_malloc(kDmaBytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (line_ == nullptr) {
        return ESP_ERR_NO_MEM;
    }

    gpio_set_level(static_cast<gpio_num_t>(pins.rst), 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(static_cast<gpio_num_t>(pins.rst), 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(static_cast<gpio_num_t>(pins.rst), 1);
    vTaskDelay(pdMS_TO_TICKS(150));

    // Same command sequence as the Pico ILI9488 driver, including 18-bit RGB666.
    writeCommand(0x01);
    vTaskDelay(pdMS_TO_TICKS(200));
    writeCommand(0x11);
    vTaskDelay(pdMS_TO_TICKS(200));

    writeCommand(0x36);
    const uint8_t madctl = 0x48;
    writeData(&madctl, 1);

    writeCommand(0x3A);
    const uint8_t pixfmt = 0x66;
    writeData(&pixfmt, 1);

    writeCommand(0xC5);
    const uint8_t vcom[] = {0x00, 0x36, 0x80};
    writeData(vcom, sizeof(vcom));

    writeCommand(0xC2);
    const uint8_t power = 0xA7;
    writeData(&power, 1);

    writeCommand(0xE0);
    const uint8_t gamma_pos[] = {
        0xF0, 0x01, 0x06, 0x0F, 0x12, 0x1D, 0x36, 0x54,
        0x44, 0x0C, 0x18, 0x16, 0x13, 0x15};
    writeData(gamma_pos, sizeof(gamma_pos));

    writeCommand(0xE1);
    const uint8_t gamma_neg[] = {
        0xF0, 0x01, 0x05, 0x0A, 0x0B, 0x07, 0x32, 0x44,
        0x44, 0x0C, 0x18, 0x17, 0x13, 0x16};
    writeData(gamma_neg, sizeof(gamma_neg));

    writeCommand(0x21);
    writeCommand(0x29);
    vTaskDelay(pdMS_TO_TICKS(50));

    ESP_LOGI(TAG, "panel ready, spi errors %d", s_spi_errors);
    return ESP_OK;
}

void Ili9488Panel::setBacklight(bool on) {
    if (bl_ < 0) {
        return;
    }
    backlight_on_ = on;
    gpio_set_level(static_cast<gpio_num_t>(bl_), on ? 1 : 0);
}

void Ili9488Panel::writeCommand(uint8_t cmd) {
    writeData(&cmd, 1, 0);
}

void Ili9488Panel::writeData(const uint8_t* data, int len) {
    writeData(data, len, 1);
}

void Ili9488Panel::writeData(const uint8_t* data, int len, int dc_level) {
    if (len <= 0 || data == nullptr || line_ == nullptr) {
        return;
    }
    while (len > 0) {
        int n = len > kDmaBytes ? kDmaBytes : len;
        const uint8_t* tx = data;
        if (data != line_) {
            std::memcpy(line_, data, n);
            tx = line_;
        }
        spi_transaction_t t = {};
        t.length = n * 8;
        t.tx_buffer = tx;
        t.user = reinterpret_cast<void*>(static_cast<uintptr_t>(dc_level));
        const esp_err_t err = spi_device_polling_transmit(static_cast<spi_device_handle_t>(spi_), &t);
        if (err != ESP_OK) {
            if (s_spi_errors < 6) {
                ESP_LOGE(TAG, "spi %s, %d bytes, dc %d", esp_err_to_name(err), n, dc_level);
            }
            s_spi_errors++;
        } else {
            s_spi_bytes += n;
        }
        data += n;
        len -= n;
    }
}

void Ili9488Panel::setWindow(int x0, int y0, int x1, int y1) {
    writeCommand(0x2A);
    const uint8_t caset[] = {
        static_cast<uint8_t>(x0 >> 8), static_cast<uint8_t>(x0),
        static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1)};
    writeData(caset, sizeof(caset));

    writeCommand(0x2B);
    const uint8_t paset[] = {
        static_cast<uint8_t>(y0 >> 8), static_cast<uint8_t>(y0),
        static_cast<uint8_t>(y1 >> 8), static_cast<uint8_t>(y1)};
    writeData(paset, sizeof(paset));

    writeCommand(0x2C);
}

void Ili9488Panel::fillRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    if (w <= 0 || h <= 0 || line_ == nullptr) {
        return;
    }
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x >= WIDTH || y >= HEIGHT) {
        return;
    }
    if (x + w > WIDTH) {
        w = WIDTH - x;
    }
    if (y + h > HEIGHT) {
        h = HEIGHT - y;
    }
    if (w <= 0 || h <= 0) {
        return;
    }

    const uint8_t rr = r & 0xFC;
    const uint8_t gg = g & 0xFC;
    const uint8_t bb = b & 0xFC;
    for (int i = 0; i < w; ++i) {
        line_[i * 3] = rr;
        line_[i * 3 + 1] = gg;
        line_[i * 3 + 2] = bb;
    }

    setWindow(x, y, x + w - 1, y + h - 1);
    for (int row = 0; row < h; ++row) {
        writeData(line_, w * 3);
    }
}

void Ili9488Panel::fillScreen(uint8_t r, uint8_t g, uint8_t b) {
    fillRect(0, 0, WIDTH, HEIGHT, r, g, b);
}

void Ili9488Panel::blit(int x, int y, int w, int h, const uint8_t* rgb) {
    if (w <= 0 || h <= 0 || rgb == nullptr || line_ == nullptr) {
        return;
    }
    setWindow(x, y, x + w - 1, y + h - 1);
    const int total = w * h * 3;
    const int chunk = WIDTH * 3;
    for (int off = 0; off < total; off += chunk) {
        int n = total - off;
        if (n > chunk) {
            n = chunk;
        }
        // 不把一个像素拆到两次传输里。
        n -= n % 3;
        for (int i = 0; i < n; ++i) {
            line_[i] = rgb[off + i] & 0xFC;
        }
        writeData(line_, n);
    }
}
