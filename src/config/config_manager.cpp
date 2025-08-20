#include "config/config_manager.hpp"
#include "config/embedded_config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace game2048 {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "无法打开配置文件: " << config_file << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();
    
    return parseJsonConfig(json_content);
}

bool ConfigManager::loadEmbeddedConfig() {
    const char* embedded_config = embedded_config::getEmbeddedConfig();
    if (!embedded_config) {
        std::cerr << "无法获取嵌入式配置" << std::endl;
        return false;
    }
    
    std::cout << "加载嵌入式配置..." << std::endl;
    return parseJsonConfig(embedded_config);
}

bool ConfigManager::parseJsonConfig(const std::string& json_content) {
    // 简单的JSON解析器（针对我们的配置文件格式）
    try {
        // 解析driver
        size_t pos = json_content.find("\"driver\"");
        if (pos != std::string::npos) {
            pos = json_content.find("\"", pos + 8);
            if (pos != std::string::npos) {
                size_t end = json_content.find("\"", pos + 1);
                if (end != std::string::npos) {
                    display_config_.driver = json_content.substr(pos + 1, end - pos - 1);
                }
            }
        }
        
        // 解析rotation
        pos = json_content.find("\"rotation\"");
        if (pos != std::string::npos) {
            pos = json_content.find(":", pos);
            if (pos != std::string::npos) {
                size_t end = json_content.find_first_of(",}", pos);
                if (end != std::string::npos) {
                    std::string value = json_content.substr(pos + 1, end - pos - 1);
                    display_config_.rotation = static_cast<uint8_t>(std::stoi(value));
                }
            }
        }
        
        // 解析brightness
        pos = json_content.find("\"brightness\"");
        if (pos != std::string::npos) {
            pos = json_content.find(":", pos);
            if (pos != std::string::npos) {
                size_t end = json_content.find_first_of(",}", pos);
                if (end != std::string::npos) {
                    std::string value = json_content.substr(pos + 1, end - pos - 1);
                    display_config_.brightness = static_cast<uint8_t>(std::stoi(value));
                }
            }
        }
        
        // 解析SPI配置
        pos = json_content.find("\"spi\"");
        if (pos != std::string::npos) {
            // SPI port
            size_t port_pos = json_content.find("\"port\"", pos);
            if (port_pos != std::string::npos) {
                port_pos = json_content.find(":", port_pos);
                if (port_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", port_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(port_pos + 1, end - port_pos - 1);
                        display_config_.hardware.spi_port = static_cast<uint8_t>(std::stoi(value));
                    }
                }
            }
            
            // SPI frequency
            size_t freq_pos = json_content.find("\"frequency\"", pos);
            if (freq_pos != std::string::npos) {
                freq_pos = json_content.find(":", freq_pos);
                if (freq_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", freq_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(freq_pos + 1, end - freq_pos - 1);
                        display_config_.hardware.spi_frequency = static_cast<uint32_t>(std::stoul(value));
                    }
                }
            }
        }
        
        // 解析引脚配置
        pos = json_content.find("\"pins\"");
        if (pos != std::string::npos) {
            // DC pin
            size_t pin_pos = json_content.find("\"dc\"", pos);
            if (pin_pos != std::string::npos) {
                pin_pos = json_content.find(":", pin_pos);
                if (pin_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", pin_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(pin_pos + 1, end - pin_pos - 1);
                        display_config_.pins.dc = static_cast<uint8_t>(std::stoi(value));
                    }
                }
            }
            
            // RST pin
            pin_pos = json_content.find("\"rst\"", pos);
            if (pin_pos != std::string::npos) {
                pin_pos = json_content.find(":", pin_pos);
                if (pin_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", pin_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(pin_pos + 1, end - pin_pos - 1);
                        display_config_.pins.rst = static_cast<uint8_t>(std::stoi(value));
                    }
                }
            }
            
            // CS pin
            pin_pos = json_content.find("\"cs\"", pos);
            if (pin_pos != std::string::npos) {
                pin_pos = json_content.find(":", pin_pos);
                if (pin_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", pin_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(pin_pos + 1, end - pin_pos - 1);
                        display_config_.pins.cs = static_cast<uint8_t>(std::stoi(value));
                    }
                }
            }
            
            // SCLK pin
            pin_pos = json_content.find("\"sclk\"", pos);
            if (pin_pos != std::string::npos) {
                pin_pos = json_content.find(":", pin_pos);
                if (pin_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", pin_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(pin_pos + 1, end - pin_pos - 1);
                        display_config_.pins.sclk = static_cast<uint8_t>(std::stoi(value));
                    }
                }
            }
            
            // MOSI pin
            pin_pos = json_content.find("\"mosi\"", pos);
            if (pin_pos != std::string::npos) {
                pin_pos = json_content.find(":", pin_pos);
                if (pin_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", pin_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(pin_pos + 1, end - pin_pos - 1);
                        display_config_.pins.mosi = static_cast<uint8_t>(std::stoi(value));
                    }
                }
            }
            
            // BL pin (背光引脚)
            pin_pos = json_content.find("\"bl\"", pos);
            if (pin_pos != std::string::npos) {
                pin_pos = json_content.find(":", pin_pos);
                if (pin_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", pin_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(pin_pos + 1, end - pin_pos - 1);
                        display_config_.pins.bl = static_cast<uint8_t>(std::stoi(value));
                    }
                }
            }
        }
        
        // 解析游戏配置
        pos = json_content.find("\"game\"");
        if (pos != std::string::npos) {
            // UI scale
            size_t scale_pos = json_content.find("\"ui_scale\"", pos);
            if (scale_pos != std::string::npos) {
                scale_pos = json_content.find(":", scale_pos);
                if (scale_pos != std::string::npos) {
                    size_t end = json_content.find_first_of(",}", scale_pos);
                    if (end != std::string::npos) {
                        std::string value = json_content.substr(scale_pos + 1, end - scale_pos - 1);
                        display_config_.game.ui_scale = std::stof(value);
                    }
                }
            }
            
            // Color scheme
            size_t scheme_pos = json_content.find("\"color_scheme\"", pos);
            if (scheme_pos != std::string::npos) {
                scheme_pos = json_content.find("\"", scheme_pos + 14);
                if (scheme_pos != std::string::npos) {
                    size_t end = json_content.find("\"", scheme_pos + 1);
                    if (end != std::string::npos) {
                        display_config_.game.color_scheme = json_content.substr(scheme_pos + 1, end - scheme_pos - 1);
                    }
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "解析配置文件时出错: " << e.what() << std::endl;
        return false;
    }
}

std::string ConfigManager::generateJsonConfig() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"display\": {\n";
    json << "    \"driver\": \"" << display_config_.driver << "\",\n";
    json << "    \"rotation\": " << static_cast<int>(display_config_.rotation) << ",\n";
    json << "    \"brightness\": " << static_cast<int>(display_config_.brightness) << "\n";
    json << "  },\n";
    json << "  \"hardware\": {\n";
    json << "    \"spi\": {\n";
    json << "      \"port\": " << static_cast<int>(display_config_.hardware.spi_port) << ",\n";
    json << "      \"frequency\": " << display_config_.hardware.spi_frequency << "\n";
    json << "    },\n";
    json << "    \"pins\": {\n";
    json << "      \"dc\": " << static_cast<int>(display_config_.pins.dc) << ",\n";
    json << "      \"rst\": " << static_cast<int>(display_config_.pins.rst) << ",\n";
    json << "      \"cs\": " << static_cast<int>(display_config_.pins.cs) << ",\n";
    json << "      \"sclk\": " << static_cast<int>(display_config_.pins.sclk) << ",\n";
    json << "      \"mosi\": " << static_cast<int>(display_config_.pins.mosi) << ",\n";
    json << "      \"bl\": " << static_cast<int>(display_config_.pins.bl) << "\n";
    json << "    }\n";
    json << "  },\n";
    json << "  \"game\": {\n";
    json << "    \"ui_scale\": " << display_config_.game.ui_scale << ",\n";
    json << "    \"color_scheme\": \"" << display_config_.game.color_scheme << "\"\n";
    json << "  }\n";
    json << "}\n";
    return json.str();
}

bool ConfigManager::saveConfig(const std::string& config_file) {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "无法创建配置文件: " << config_file << std::endl;
        return false;
    }
    
    std::string json_content = generateJsonConfig();
    file << json_content;
    return true;
}

bool ConfigManager::validateConfig() const {
    // 验证驱动类型
    if (display_config_.driver != "ili9488" && display_config_.driver != "st7306") {
        return false;
    }
    
    // 验证旋转角度 (0=0°, 1=90°, 2=180°, 3=270°)
    if (display_config_.rotation > 3) {
        return false;
    }
    
    // 验证亮度
    if (display_config_.brightness > 100) {
        return false;
    }
    
    // 验证UI缩放
    if (display_config_.game.ui_scale <= 0.0f || display_config_.game.ui_scale > 5.0f) {
        return false;
    }
    
    return true;
}

} // namespace game2048
