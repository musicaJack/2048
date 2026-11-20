#include "board_input.hpp"

#include <cstring>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

static const char* TAG = "input";

static constexpr uint8_t REG_BUTTON = 0x20;
static constexpr uint8_t REG_OFFSET_12 = 0x50;
static constexpr uint8_t REG_FW = 0xFE;

esp_err_t BoardInput::begin(const InputPins& pins) {
    uint64_t mask = 0;
    if (pins.page_up >= 0) {
        mask |= 1ULL << pins.page_up;
    }
    if (pins.page_down >= 0) {
        mask |= 1ULL << pins.page_down;
    }
    if (pins.screen >= 0) {
        mask |= 1ULL << pins.screen;
    }
    if (mask != 0) {
        gpio_config_t in = {};
        in.pin_bit_mask = mask;
        in.mode = GPIO_MODE_INPUT;
        in.pull_up_en = GPIO_PULLUP_ENABLE;
        in.pull_down_en = GPIO_PULLDOWN_DISABLE;
        in.intr_type = GPIO_INTR_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&in));
    }

    if (pins.exp_sda >= 0 && pins.exp_scl >= 0 && pins.exp_screen_bit >= 0) {
        exp_sda_ = pins.exp_sda;
        exp_scl_ = pins.exp_scl;
        exp_bit_ = pins.exp_screen_bit;
        gpio_config_t od = {};
        od.pin_bit_mask = (1ULL << pins.exp_sda) | (1ULL << pins.exp_scl);
        od.mode = GPIO_MODE_OUTPUT_OD;
        od.pull_up_en = GPIO_PULLUP_ENABLE;
        od.pull_down_en = GPIO_PULLDOWN_DISABLE;
        od.intr_type = GPIO_INTR_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&od));
        gpio_set_level(static_cast<gpio_num_t>(exp_sda_), 1);
        gpio_set_level(static_cast<gpio_num_t>(exp_scl_), 1);
        for (uint8_t addr = 0x20; addr <= 0x27; ++addr) {
            if (expProbe(addr)) {
                exp_addr_ = addr;
                exp_ready_ = expWrite(0x03, 0xFF);
                ESP_LOGI(TAG, "expander 0x%02x %s", addr, exp_ready_ ? "ready" : "config failed");
                break;
            }
        }
        if (!exp_ready_) {
            ESP_LOGW(TAG, "TCA9554 0x20-0x27 no ack");
        }
    }

    up_.pin = pins.page_up;
    down_.pin = pins.page_down;
    screen_.pin = pins.screen;
    const int64_t now = esp_timer_get_time();
    up_.changed_us = down_.changed_us = screen_.changed_us = now;
    if (exp_ready_) {
        // EXIO 没有上拉。上电时若读到低，先当作已经按住，等松开后再认一次新的长按。
        screen_.raw = true;
        screen_.stable = true;
    }

    i2c_master_bus_config_t bus_cfg = {};
    bus_cfg.i2c_port = I2C_NUM_0;
    bus_cfg.sda_io_num = static_cast<gpio_num_t>(pins.sda);
    bus_cfg.scl_io_num = static_cast<gpio_num_t>(pins.scl);
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.glitch_ignore_cnt = 7;
    bus_cfg.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus = nullptr;
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c bus failed: %s", esp_err_to_name(err));
        return err;
    }

    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = pins.addr;
    dev_cfg.scl_speed_hz = pins.hz;

    i2c_master_dev_handle_t dev = nullptr;
    err = i2c_master_bus_add_device(bus, &dev_cfg, &dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c device failed: %s", esp_err_to_name(err));
        return err;
    }
    dev_ = dev;

    const uint8_t reg = REG_FW;
    uint8_t version = 0;
    err = i2c_master_transmit_receive(dev, &reg, 1, &version, 1, 100);
    if (err == ESP_OK) {
        joy_ready_ = true;
        fw_ = version;
        ESP_LOGI(TAG, "joystick 0x%02x firmware 0x%02x", pins.addr, version);
    } else {
        ESP_LOGW(TAG, "joystick 0x%02x no ack: %s", pins.addr, esp_err_to_name(err));
    }
    return ESP_OK;
}

static esp_err_t read_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t* buf, size_t len) {
    return i2c_master_transmit_receive(dev, &reg, 1, buf, len, 50);
}

void BoardInput::readStick(int16_t& x, int16_t& y) {
    x = 0;
    y = 0;
    if (!joy_ready_) {
        return;
    }
    uint8_t data[4] = {};
    if (read_reg(static_cast<i2c_master_dev_handle_t>(dev_), REG_OFFSET_12, data, 4) == ESP_OK) {
        std::memcpy(&x, data, 2);
        std::memcpy(&y, data + 2, 2);
    }
}

bool BoardInput::stickPressed() {
    if (!joy_ready_) {
        return false;
    }
    uint8_t value = 1;
    if (read_reg(static_cast<i2c_master_dev_handle_t>(dev_), REG_BUTTON, &value, 1) != ESP_OK) {
        return false;
    }
    return value == 0;
}

void BoardInput::pollKey(Key& key, bool raw, int debounce_ms, int long_ms, bool click_on_press,
                         bool& click, bool& long_press) {
    click = false;
    long_press = false;
    const int64_t now = esp_timer_get_time();
    if (raw != key.raw) {
        key.raw = raw;
        key.changed_us = now;
    }
    if ((now - key.changed_us) < static_cast<int64_t>(debounce_ms) * 1000) {
        return;
    }
    if (raw == key.stable) {
        if (!click_on_press && key.stable && key.saw_press && !key.long_sent &&
            (now - key.changed_us) >= static_cast<int64_t>(long_ms) * 1000) {
            key.long_sent = true;
            long_press = true;
        }
        return;
    }

    key.stable = raw;
    if (key.stable) {
        key.saw_press = true;
        key.long_sent = false;
        if (click_on_press) {
            click = true;
        }
    } else {
        if (!click_on_press && key.saw_press && !key.long_sent) {
            click = true;
        }
        key.saw_press = false;
        key.long_sent = false;
    }
}

bool BoardInput::screenPressed() {
    if (exp_ready_) {
        uint8_t value = 0xFF;
        if (!expRead(0x00, value)) {
            return false;
        }
        return (value & (1U << exp_bit_)) == 0;
    }
    if (screen_.pin < 0) {
        return false;
    }
    return gpio_get_level(static_cast<gpio_num_t>(screen_.pin)) == 0;
}

void BoardInput::poll(int debounce_ms, int long_ms, bool& up_click, bool& down_click, bool& screen_long) {
    bool ignored = false;
    const bool up_raw = up_.pin >= 0 && gpio_get_level(static_cast<gpio_num_t>(up_.pin)) == 0;
    const bool down_raw = down_.pin >= 0 && gpio_get_level(static_cast<gpio_num_t>(down_.pin)) == 0;
    pollKey(up_, up_raw, debounce_ms, long_ms, true, up_click, ignored);
    pollKey(down_, down_raw, debounce_ms, long_ms, true, down_click, ignored);
    bool screen_click = false;
    pollKey(screen_, screenPressed(), debounce_ms, long_ms, false, screen_click, screen_long);
}

void BoardInput::expStart() {
    gpio_set_level(static_cast<gpio_num_t>(exp_sda_), 1);
    gpio_set_level(static_cast<gpio_num_t>(exp_scl_), 1);
    esp_rom_delay_us(5);
    gpio_set_level(static_cast<gpio_num_t>(exp_sda_), 0);
    esp_rom_delay_us(5);
    gpio_set_level(static_cast<gpio_num_t>(exp_scl_), 0);
    esp_rom_delay_us(5);
}

void BoardInput::expStop() {
    gpio_set_level(static_cast<gpio_num_t>(exp_sda_), 0);
    esp_rom_delay_us(5);
    gpio_set_level(static_cast<gpio_num_t>(exp_scl_), 1);
    esp_rom_delay_us(5);
    gpio_set_level(static_cast<gpio_num_t>(exp_sda_), 1);
    esp_rom_delay_us(5);
}

void BoardInput::expWriteBit(int bit) {
    gpio_set_level(static_cast<gpio_num_t>(exp_sda_), bit ? 1 : 0);
    esp_rom_delay_us(5);
    gpio_set_level(static_cast<gpio_num_t>(exp_scl_), 1);
    esp_rom_delay_us(5);
    gpio_set_level(static_cast<gpio_num_t>(exp_scl_), 0);
    esp_rom_delay_us(5);
}

int BoardInput::expReadBit() {
    gpio_set_level(static_cast<gpio_num_t>(exp_sda_), 1);
    esp_rom_delay_us(5);
    gpio_set_level(static_cast<gpio_num_t>(exp_scl_), 1);
    esp_rom_delay_us(5);
    const int level = gpio_get_level(static_cast<gpio_num_t>(exp_sda_));
    gpio_set_level(static_cast<gpio_num_t>(exp_scl_), 0);
    esp_rom_delay_us(5);
    return level;
}

bool BoardInput::expWriteByte(uint8_t value) {
    for (int bit = 7; bit >= 0; --bit) {
        expWriteBit((value >> bit) & 1);
    }
    return expReadBit() == 0;
}

uint8_t BoardInput::expReadByte(bool ack) {
    uint8_t value = 0;
    for (int bit = 7; bit >= 0; --bit) {
        if (expReadBit()) {
            value |= static_cast<uint8_t>(1U << bit);
        }
    }
    expWriteBit(ack ? 0 : 1);
    return value;
}

bool BoardInput::expProbe(uint8_t addr) {
    expStart();
    const bool ack = expWriteByte(static_cast<uint8_t>(addr << 1));
    expStop();
    return ack;
}

bool BoardInput::expWrite(uint8_t reg, uint8_t data) {
    expStart();
    const bool ok = expWriteByte(static_cast<uint8_t>(exp_addr_ << 1)) && expWriteByte(reg) &&
                    expWriteByte(data);
    expStop();
    return ok;
}

bool BoardInput::expRead(uint8_t reg, uint8_t& data) {
    expStart();
    if (!expWriteByte(static_cast<uint8_t>(exp_addr_ << 1)) || !expWriteByte(reg)) {
        expStop();
        return false;
    }
    expStart();
    if (!expWriteByte(static_cast<uint8_t>((exp_addr_ << 1) | 1))) {
        expStop();
        return false;
    }
    data = expReadByte(false);
    expStop();
    return true;
}
