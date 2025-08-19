#pragma once

#include "display/display_driver.hpp"
#include "game/game_config.hpp"
#include <cstdint>
#include <vector>

namespace game2048 {

// 动画类型
enum class AnimationType {
    GAME_START,
    ACHIEVEMENT
};

// 成就等级
enum class AchievementLevel {
    ACHIEVEMENT_32 = 32,
    ACHIEVEMENT_64 = 64,
    ACHIEVEMENT_128 = 128,
    ACHIEVEMENT_256 = 256,
    ACHIEVEMENT_512 = 512,
    ACHIEVEMENT_1024 = 1024,
    ACHIEVEMENT_2048 = 2048
};

// 动画状态
enum class AnimationState {
    IDLE,
    PLAYING,
    FINISHED
};

// 心跳动画参数
struct HeartbeatParams {
    uint8_t base_intensity;    // 基础强度
    uint8_t max_intensity;     // 最大强度
    uint16_t duration_ms;      // 总时长(ms)
    uint16_t frame_delay_ms;   // 帧延迟(ms)
    Color color;               // 心跳颜色
};

// 动画管理器
class GameAnimationManager {
public:
    GameAnimationManager(DisplayDriver* display);
    
    // 动画控制
    void startGameStartAnimation();
    void startAchievementAnimation(AchievementLevel level);
    void update();
    void stop();
    
    // 设置当前成就等级
    void setCurrentAchievementLevel(AchievementLevel level) { current_achievement_level_ = level; }
    
    // 状态查询
    bool isPlaying() const { return state_ == AnimationState::PLAYING; }
    AnimationType getCurrentType() const { return current_type_; }
    
    // 设置动画区域
    void setAnimationArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    
    // 设置心跳参数
    void setHeartbeatParams(const HeartbeatParams& params) { heartbeat_params_ = params; }

private:
    DisplayDriver* display_;
    AnimationState state_;
    AnimationType current_type_;
    
    // 动画区域
    uint16_t anim_x_, anim_y_, anim_width_, anim_height_;
    
    // 心跳动画参数
    HeartbeatParams heartbeat_params_;
    
    // 动画计时器
    uint32_t start_time_;
    uint32_t frame_count_;
    
    // 当前成就等级
    AchievementLevel current_achievement_level_;
    
    // 游戏开始动画参数
    static constexpr uint8_t GAME_START_FLASH_COUNT = 3;
    static constexpr uint16_t GAME_START_FLASH_DURATION = 250; // ms
    static constexpr uint16_t GAME_START_TOTAL_DURATION = 1500; // ms
    
    // 心跳动画参数
    static constexpr uint8_t HEARTBEAT_BASE_INTENSITY = 50;
    static constexpr uint8_t HEARTBEAT_MAX_INTENSITY = 255;
    static constexpr uint16_t HEARTBEAT_DURATION = 2000; // ms
    static constexpr uint16_t HEARTBEAT_FRAME_DELAY = 50; // ms
    
    // 动画方法
    void updateGameStartAnimation();
    void updateAchievementAnimation();
    
    // 绘制方法
    void drawGameStartText(bool visible);
    void drawHeartbeatFrame(uint8_t intensity);
    void drawAchievementText(AchievementLevel level, uint8_t intensity);
    
    // 辅助方法
    uint8_t calculateHeartbeatIntensity(uint32_t elapsed_time);
    Color getAchievementColor(AchievementLevel level);
    const char* getAchievementText(AchievementLevel level);
    void clearAnimationArea();
};

} // namespace game2048
