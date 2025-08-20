#pragma once

#include <string>
#include <memory>
#include <map>

namespace game2048 {

// 显示配置结构体
struct DisplayConfig {
    std::string driver;           // "ili9488" 或 "st7306"
    uint8_t rotation;             // 旋转角度 (0, 90, 180, 270)
    uint8_t brightness;           // 亮度 (0-100)
    
    // 硬件配置
    struct {
        uint8_t spi_port;         // SPI端口
        uint32_t spi_frequency;   // SPI频率
    } hardware;
    
    // 引脚配置
    struct {
        uint8_t dc;               // 数据/命令引脚
        uint8_t rst;              // 复位引脚
        uint8_t cs;               // 片选引脚
        uint8_t sclk;             // 时钟引脚
        uint8_t mosi;             // 数据引脚
        uint8_t bl;               // 背光引脚
    } pins;
    
    // 游戏UI配置
    struct {
        float ui_scale;           // UI缩放比例
        std::string color_scheme; // 颜色主题
    } game;
    
    DisplayConfig() : driver("ili9488"), rotation(0), brightness(100),
                     hardware{0, 40000000}, pins{20, 15, 17, 18, 19, 16},
                     game{1.0f, "default"} {}
};

// 配置管理器类
class ConfigManager {
public:
    static ConfigManager& getInstance();
    
    // 加载配置文件
    bool loadConfig(const std::string& config_file);
    
    // 加载嵌入式配置（编译时嵌入的配置）
    bool loadEmbeddedConfig();
    
    // 获取显示配置
    const DisplayConfig& getDisplayConfig() const { return display_config_; }
    
    // 获取特定配置项
    std::string getDriverType() const { return display_config_.driver; }
    uint8_t getRotation() const { return display_config_.rotation; }
    uint8_t getBrightness() const { return display_config_.brightness; }
    
    // 设置配置项
    void setDriverType(const std::string& driver) { display_config_.driver = driver; }
    void setRotation(uint8_t rotation) { display_config_.rotation = rotation; }
    void setBrightness(uint8_t brightness) { display_config_.brightness = brightness; }
    
    // 保存配置到文件
    bool saveConfig(const std::string& config_file);
    
    // 生成JSON配置字符串（用于测试）
    std::string generateJsonConfig() const;
    
    // 验证配置
    bool validateConfig() const;
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    DisplayConfig display_config_;
    
    // 解析JSON配置
    bool parseJsonConfig(const std::string& json_content);
};

} // namespace game2048
