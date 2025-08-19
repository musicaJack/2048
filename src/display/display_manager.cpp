#include "display/display_manager.hpp"
#include "config/display_config_compile_time.hpp"
#ifdef DISPLAY_DRIVER_ST7306
#include "display/adapters/st7306_adapter.hpp"
#endif
#ifdef DISPLAY_DRIVER_ILI9488
#include "display/adapters/ili9488_adapter.hpp"
#endif
#include <iostream>

namespace game2048 {

DisplayManager& DisplayManager::getInstance() {
    static DisplayManager instance;
    return instance;
}

bool DisplayManager::initFromFile(const std::string& config_file) {
    // 加载配置
    if (!config_manager_.loadConfig(config_file)) {
        std::cerr << "无法加载配置文件" << std::endl;
        return false;
    }
    
    // 验证配置
    if (!config_manager_.validateConfig()) {
        std::cerr << "配置文件验证失败" << std::endl;
        return false;
    }
    
    // 创建显示驱动
    std::string driver_type = config_manager_.getDriverType();
    driver_ = createDriver(driver_type);
    
    if (!driver_) {
        std::cerr << "无法创建显示驱动: " << driver_type << std::endl;
        return false;
    }
    
    // 应用配置
    if (!applyConfig(driver_.get())) {
        std::cerr << "无法应用配置到显示驱动" << std::endl;
        return false;
    }
    
    // 初始化驱动
    if (!driver_->init()) {
        std::cerr << "显示驱动初始化失败" << std::endl;
        return false;
    }
    
    std::cout << "显示管理器初始化成功，使用驱动: " << driver_type << std::endl;
    return true;
}

bool DisplayManager::init() {
    // 使用编译时配置，无需加载配置文件
    std::cout << "使用编译时配置初始化显示管理器" << std::endl;
    std::cout << "当前显示驱动: " << CURRENT_DISPLAY_DRIVER << std::endl;
    std::cout << "显示分辨率: " << DISPLAY_WIDTH << "x" << DISPLAY_HEIGHT << std::endl;
    
    // 创建默认配置
    DisplayConfig config;
    config.driver = CURRENT_DISPLAY_DRIVER;
    config.rotation = 0;
    config.brightness = 100;
               config.pins.dc = display_config::DEFAULT_DC_PIN;
           config.pins.rst = display_config::DEFAULT_RST_PIN;
           config.pins.cs = display_config::DEFAULT_CS_PIN;
           config.pins.sclk = display_config::DEFAULT_SCLK_PIN;
           config.pins.mosi = display_config::DEFAULT_MOSI_PIN;
           config.pins.bl = display_config::DEFAULT_BL_PIN;  // 添加背光引脚
    config.hardware.spi_port = display_config::DEFAULT_SPI_PORT;
    config.hardware.spi_frequency = display_config::DEFAULT_SPI_FREQUENCY;
    
    // 更新配置管理器
    config_manager_.setDriverType(config.driver);
    config_manager_.setRotation(config.rotation);
    config_manager_.setBrightness(config.brightness);
    
    // 创建显示驱动
    driver_ = createDriver(config.driver);
    
    if (!driver_) {
        std::cerr << "无法创建显示驱动: " << config.driver << std::endl;
        return false;
    }
    
    // 初始化显示驱动
    if (!driver_->init()) {
        std::cerr << "显示驱动初始化失败" << std::endl;
        return false;
    }
    
    std::cout << "显示管理器初始化成功" << std::endl;
    return true;
}

std::unique_ptr<DisplayDriver> DisplayManager::createDriver(const std::string& driver_type) {
    const DisplayConfig& config = config_manager_.getDisplayConfig();
    
    // 使用编译时宏确定要创建的驱动
#ifdef DISPLAY_DRIVER_ST7306
    if (driver_type == "st7306") {
        std::cout << "创建ST7306显示驱动适配器" << std::endl;
        return std::make_unique<ST7306Adapter>(config);
    }
#endif

#ifdef DISPLAY_DRIVER_ILI9488
    if (driver_type == "ili9488") {
        std::cout << "创建ILI9488显示驱动适配器" << std::endl;
        return std::make_unique<ILI9488Adapter>(config);
    }
#endif

    std::cout << "未知的显示驱动类型或未在编译时启用: " << driver_type << std::endl;
    return nullptr;
}

bool DisplayManager::applyConfig(DisplayDriver* driver) {
    if (!driver) {
        return false;
    }
    
    const DisplayConfig& config = config_manager_.getDisplayConfig();
    
    // 设置旋转
    driver->setRotation(config.rotation);
    
    // 设置亮度
    driver->setBrightness(config.brightness);
    
    return true;
}

bool DisplayManager::switchDriver(const std::string& driver_type) {
    if (driver_type == getCurrentDriverType()) {
        return true; // 已经是当前驱动
    }
    
    // 清理当前驱动
    if (driver_) {
        driver_->deinit();
        driver_.reset();
    }
    
    // 创建新驱动
    driver_ = createDriver(driver_type);
    if (!driver_) {
        std::cerr << "无法切换到驱动: " << driver_type << std::endl;
        return false;
    }
    
    // 应用配置
    if (!applyConfig(driver_.get())) {
        std::cerr << "无法应用配置到新驱动" << std::endl;
        return false;
    }
    
    // 初始化新驱动
    if (!driver_->init()) {
        std::cerr << "新驱动初始化失败" << std::endl;
        return false;
    }
    
    // 更新配置
    config_manager_.setDriverType(driver_type);
    
    std::cout << "成功切换到驱动: " << driver_type << std::endl;
    return true;
}

std::string DisplayManager::getCurrentDriverType() const {
    if (!driver_) {
        return "";
    }
    return config_manager_.getDriverType();
}

const DisplayConfig& DisplayManager::getConfig() const {
    return config_manager_.getDisplayConfig();
}

bool DisplayManager::reloadConfig() {
    if (!config_manager_.loadConfig()) {
        return false;
    }
    
    if (!config_manager_.validateConfig()) {
        return false;
    }
    
    // 如果驱动类型改变了，需要切换驱动
    std::string new_driver = config_manager_.getDriverType();
    if (new_driver != getCurrentDriverType()) {
        return switchDriver(new_driver);
    }
    
    // 否则只应用新配置
    return applyConfig(driver_.get());
}

void DisplayManager::cleanup() {
    if (driver_) {
        driver_->deinit();
        driver_.reset();
    }
}

} // namespace game2048
