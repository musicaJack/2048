#pragma once

namespace utils {

// 游戏配置结构
struct GameConfig {
    int display_brightness;
    bool sound_enabled;
    int difficulty_level;
    int auto_save_interval;
};

/**
 * @brief 配置加载器类
 * 
 * 负责加载和保存游戏配置
 */
class ConfigLoader {
public:
    ConfigLoader();
    ~ConfigLoader();

    // 禁用拷贝构造和赋值
    ConfigLoader(const ConfigLoader&) = delete;
    ConfigLoader& operator=(const ConfigLoader&) = delete;

    // 配置操作
    bool loadConfig(const char* filename);
    bool saveConfig(const char* filename);
    const GameConfig& getConfig() const;
    GameConfig& getConfig();
    bool isLoaded() const;

private:
    GameConfig config_;
    bool is_loaded_;
};

} // namespace utils
