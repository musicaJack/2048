#include "ili9488_panel.hpp"

#include <cstring>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ili9488";

// 一次最多约 20 行。ESP32-S3 上更大的 DMA 块会丢掉末尾几个字节，屏幕上就是一条横纹。
static constexpr int kDmaBytes = 320 * 20 * 3;

static int s_dc_pin = -1;
static int s_cs_pin = -1;
static spi_device_handle_t s_spi = nullptr;
static int s_spi_errors = 0;
static int s_spi_bytes = 0;

static void cs_select(bool selected) {
    if (s_cs_pin < 0) {
        return;
    }
    if (!selected) {
        // DMA 报完成时最后几个时钟还在移位寄存器里，立刻抬片选会被屏丢掉。
        esp_rom_delay_us(5);
    }
    gpio_set_level(static_cast<gpio_num_t>(s_cs_pin), selected ? 0 : 1);
}

static void IRAM_ATTR lcd_dc_cb(spi_transaction_t* t) {
    gpio_set_level(static_cast<gpio_num_t>(s_dc_pin), static_cast<int>(reinterpret_cast<uintptr_t>(t->user)));
}

static void spi_tx(const uint8_t* dma_buf, int len, int dc_level) {
    spi_transaction_t t = {};
    t.length = static_cast<size_t>(len) * 8;
    t.tx_buffer = dma_buf;
    t.user = reinterpret_cast<void*>(static_cast<uintptr_t>(dc_level));
    const esp_err_t err = spi_device_polling_transmit(s_spi, &t);
    if (err != ESP_OK) {
        if (s_spi_errors < 6) {
            ESP_LOGE(TAG, "spi %s, %d bytes, dc %d", esp_err_to_name(err), len, dc_level);
        }
        s_spi_errors++;
        return;
    }
    s_spi_bytes += len;
}

esp_err_t Ili9488Panel::begin(const LcdPins& pins) {
    if (pins.sck < 0 || pins.mosi < 0 || pins.cs < 0 || pins.dc < 0 || pins.rst < 0 || pins.bl < 0) {
        ESP_LOGE(TAG, "LCD is on the expander, SPI2 stays off");
        return ESP_ERR_NOT_SUPPORTED;
    }
    dc_ = pins.dc;
    bl_ = pins.bl;
    s_dc_pin = pins.dc;
    s_cs_pin = pins.cs;

    gpio_config_t out = {};
    out.pin_bit_mask = (1ULL << pins.dc) | (1ULL << pins.rst) | (1ULL << pins.bl) | (1ULL << pins.cs);
    out.mode = GPIO_MODE_OUTPUT;
    out.pull_up_en = GPIO_PULLUP_DISABLE;
    out.pull_down_en = GPIO_PULLDOWN_DISABLE;
    out.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&out));

    gpio_set_level(static_cast<gpio_num_t>(pins.bl), 1);
    backlight_on_ = true;
    gpio_set_level(static_cast<gpio_num_t>(pins.dc), 1);
    gpio_set_level(static_cast<gpio_num_t>(pins.cs), 1);
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
    dev.spics_io_num = -1;
    dev.queue_size = 1;
    dev.flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_NO_DUMMY;
    dev.pre_cb = lcd_dc_cb;
    err = spi_bus_add_device(SPI2_HOST, &dev, &s_spi);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi device add failed: %s", esp_err_to_name(err));
        return err;
    }
    spi_ = s_spi;

    // GPIO1/GPIO2 不是 SPI 专用脚，时钟要绕 GPIO 矩阵。默认驱动电流大约 20mA，
    // 边沿很陡，线一长就会振铃，屏幕把振铃当成多余的时钟，下面的行逐行错开。
    // 驱动电流降到约 5mA，边沿变缓。
    gpio_set_drive_capability(static_cast<gpio_num_t>(pins.sck), GPIO_DRIVE_CAP_0);
    gpio_set_drive_capability(static_cast<gpio_num_t>(pins.mosi), GPIO_DRIVE_CAP_0);

    line_ = static_cast<uint8_t*>(
        heap_caps_aligned_alloc(64, kDmaBytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
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

    // TFT_eSPI 的 ILI9488 初始化。0xB4=0x02 是两点翻转，默认的行翻转会在纯色上画出横纹。
    writeCommand(0xB0);
    const uint8_t ifmode = 0x00;
    writeData(&ifmode, 1);
    writeCommand(0xB1);
    const uint8_t frame = 0xA0;
    writeData(&frame, 1);
    writeCommand(0xB4);
    const uint8_t inversion = 0x02;
    writeData(&inversion, 1);
    writeCommand(0xB6);
    const uint8_t display_fn[] = {0x02, 0x02, 0x3B};
    writeData(display_fn, sizeof(display_fn));
    writeCommand(0xB7);
    const uint8_t entry = 0xC6;
    writeData(&entry, 1);
    writeCommand(0xF7);
    const uint8_t adjust[] = {0xA9, 0x51, 0x2C, 0x82};
    writeData(adjust, sizeof(adjust));

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
    const bool own_cs = !pixel_write_open_;
    if (own_cs) {
        cs_select(true);
    }
    while (len > 0) {
        int n = len > kDmaBytes ? kDmaBytes : len;
        if (len > n) {
            n -= n % 12;
        }
        if (data != line_) {
            std::memcpy(line_, data, static_cast<size_t>(n));
        }
        spi_tx(line_, n, dc_level);
        data += n;
        len -= n;
    }
    if (own_cs) {
        cs_select(false);
    }
}

void Ili9488Panel::beginPixelWrite() {
    if (pixel_write_open_) {
        return;
    }
    cs_select(true);
    pixel_write_open_ = true;
}

void Ili9488Panel::endPixelWrite() {
    if (!pixel_write_open_) {
        return;
    }
    pixel_write_open_ = false;
    cs_select(false);
    // 像素写完后发一个空命令，把屏的字节计数拉回命令边界。
    // 否则下一块会接着上一块的余数，横纹就从第一块下面开始。
    writeCommand(0x00);
}

void Ili9488Panel::writePixelChunk(const uint8_t* data, int len, bool last) {
    if (!pixel_write_open_ || len <= 0 || data == nullptr || line_ == nullptr) {
        return;
    }
    (void)last;
    const uint8_t* tx = data;
    if (data != line_) {
        std::memcpy(line_, data, static_cast<size_t>(len));
        tx = line_;
    }
    spi_tx(tx, len, 1);
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
    const int row_bytes = w * 3;
    for (int i = 0; i < w; ++i) {
        line_[i * 3] = rr;
        line_[i * 3 + 1] = gg;
        line_[i * 3 + 2] = bb;
    }

    int rows_per = kDmaBytes / row_bytes;
    if (rows_per < 1) {
        rows_per = 1;
    }

    setWindow(x, y, x + w - 1, y + h - 1);
    beginPixelWrite();
    for (int row = 0; row < h;) {
        int rows = h - row;
        if (rows > rows_per) {
            rows = rows_per;
        }
        for (int r = 1; r < rows; ++r) {
            std::memcpy(line_ + static_cast<size_t>(r) * row_bytes, line_, static_cast<size_t>(row_bytes));
        }
        writePixelChunk(line_, rows * row_bytes, row + rows >= h);
        row += rows;
    }
    endPixelWrite();
}

void Ili9488Panel::fillScreen(uint8_t r, uint8_t g, uint8_t b) {
    fillRect(0, 0, WIDTH, HEIGHT, r, g, b);
}

void Ili9488Panel::blit(int x, int y, int w, int h, const uint8_t* rgb) {
    if (w <= 0 || h <= 0 || rgb == nullptr || line_ == nullptr) {
        return;
    }
    setWindow(x, y, x + w - 1, y + h - 1);
    const int row_bytes = w * 3;
    int rows_per = kDmaBytes / row_bytes;
    if (rows_per < 1) {
        rows_per = 1;
    }
    beginPixelWrite();
    for (int row = 0; row < h;) {
        int rows = h - row;
        if (rows > rows_per) {
            rows = rows_per;
        }
        const int n = rows * row_bytes;
        const uint8_t* src = rgb + static_cast<size_t>(row) * row_bytes;
        for (int i = 0; i < n; ++i) {
            line_[i] = src[i] & 0xFC;
        }
        const int next = row + rows;
        writePixelChunk(line_, n, next >= h);
        row = next;
    }
    endPixelWrite();
}
