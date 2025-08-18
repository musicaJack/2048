/**
 * @file config_loader.cpp
 * @brief 配置加载器实现
 */

#include "utils/config_loader.hpp"
#include <cstdio>

namespace utils {

ConfigLoader::ConfigLoader() 
    : is_loaded_(false) {
}

ConfigLoader::~ConfigLoader() = default;

bool ConfigLoader::loadConfig(const char* filename) {
    // 简单实现，实际项目中可能需要解析JSON或其他格式
    printf("加载配置文件: %s\n", filename);
    
    // 设置默认配置
    config_.display_brightness = 100;
    config_.sound_enabled = true;
    config_.difficulty_level = 1;
    config_.auto_save_interval = 300; // 5分钟
    
    is_loaded_ = true;
    printf("配置文件加载完成\n");
    
    return true;
}

bool ConfigLoader::saveConfig(const char* filename) {
    printf("保存配置文件: %s\n", filename);
    printf("配置内容:\n");
    printf("- 显示亮度: %d%%\n", config_.display_brightness);
    printf("- 声音启用: %s\n", config_.sound_enabled ? "是" : "否");
    printf("- 难度等级: %d\n", config_.difficulty_level);
    printf("- 自动保存间隔: %d秒\n", config_.auto_save_interval);
    
    return true;
}

const GameConfig& ConfigLoader::getConfig() const {
    return config_;
}

GameConfig& ConfigLoader::getConfig() {
    return config_;
}

bool ConfigLoader::isLoaded() const {
    return is_loaded_;
}

} // namespace utils
