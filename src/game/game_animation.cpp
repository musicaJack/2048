#include "game/game_animation.hpp"
#include "pico/stdlib.h"
#include <cstdio>
#include <cstring>

namespace game2048 {

GameAnimationManager::GameAnimationManager(DisplayDriver* display)
    : display_(display), state_(AnimationState::IDLE), current_type_(AnimationType::GAME_START),
      anim_x_(0), anim_y_(0), anim_width_(0), anim_height_(0), start_time_(0), frame_count_(0),
      current_achievement_level_(AchievementLevel::ACHIEVEMENT_32) {
    
    // 设置默认心跳参数
    heartbeat_params_ = {
        HEARTBEAT_BASE_INTENSITY,
        HEARTBEAT_MAX_INTENSITY,
        HEARTBEAT_DURATION,
        HEARTBEAT_FRAME_DELAY,
        Color(255, 0, 0)  // 红色
    };
}

void GameAnimationManager::startGameStartAnimation() {
    if (state_ == AnimationState::PLAYING) return;
    
    state_ = AnimationState::PLAYING;
    current_type_ = AnimationType::GAME_START;
    start_time_ = to_ms_since_boot(get_absolute_time());
    frame_count_ = 0;
    
    printf("开始游戏开始动画\n");
}

void GameAnimationManager::startAchievementAnimation(AchievementLevel level) {
    if (state_ == AnimationState::PLAYING) return;
    
    state_ = AnimationState::PLAYING;
    current_type_ = AnimationType::ACHIEVEMENT;
    current_achievement_level_ = level;
    start_time_ = to_ms_since_boot(get_absolute_time());
    frame_count_ = 0;
    
    printf("开始成就动画: %d\n", static_cast<int>(level));
}

void GameAnimationManager::update() {
    if (state_ != AnimationState::PLAYING) return;
    
    switch (current_type_) {
        case AnimationType::GAME_START:
            updateGameStartAnimation();
            break;
        case AnimationType::ACHIEVEMENT:
            updateAchievementAnimation();
            break;
    }
}

void GameAnimationManager::stop() {
    if (state_ == AnimationState::PLAYING) {
        clearAnimationArea();
        state_ = AnimationState::IDLE;
        printf("动画停止\n");
    }
}

void GameAnimationManager::setAnimationArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    anim_x_ = x;
    anim_y_ = y;
    anim_width_ = width;
    anim_height_ = height;
}

void GameAnimationManager::updateGameStartAnimation() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    uint32_t elapsed_time = current_time - start_time_;
    
    if (elapsed_time >= GAME_START_TOTAL_DURATION) {
        stop();
        return;
    }
    
    // 计算当前应该显示还是隐藏
    uint32_t flash_cycle = elapsed_time / (GAME_START_FLASH_DURATION * 2);
    uint32_t cycle_position = elapsed_time % (GAME_START_FLASH_DURATION * 2);
    bool visible = cycle_position < GAME_START_FLASH_DURATION;
    
    if (flash_cycle < GAME_START_FLASH_COUNT) {
        drawGameStartText(visible);
    }
}

void GameAnimationManager::updateAchievementAnimation() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    uint32_t elapsed_time = current_time - start_time_;
    
    if (elapsed_time >= heartbeat_params_.duration_ms) {
        stop();
        return;
    }
    
    // 计算心跳强度
    uint8_t intensity = calculateHeartbeatIntensity(elapsed_time);
    
    // 绘制心跳帧
    drawHeartbeatFrame(intensity);
    
    // 绘制成就文字
    drawAchievementText(current_achievement_level_, intensity);
}

void GameAnimationManager::drawGameStartText(bool visible) {
    if (!display_) return;
    
    const char* text = "Game Start!";
    uint16_t text_width = strlen(text) * 12; // 假设12像素宽度
    uint16_t text_x = anim_x_ + (anim_width_ - text_width) / 2;
    uint16_t text_y = anim_y_ + (anim_height_ - 16) / 2; // 假设16像素高度
    
    if (visible) {
        display_->drawString(text_x, text_y, text, Color(255, 255, 255), 2); // 白色，2倍大小
    } else {
        // 清除文字区域
        display_->fillRect(text_x, text_y, text_width * 2, 16 * 2, Color(0, 0, 0));
    }
}

void GameAnimationManager::drawHeartbeatFrame(uint8_t intensity) {
    if (!display_) return;
    
    // 计算心跳区域
    uint16_t center_x = anim_x_ + anim_width_ / 2;
    uint16_t center_y = anim_y_ + anim_height_ / 2;
    uint16_t radius = (intensity * 20) / 255; // 根据强度计算半径
    
    // 绘制心跳效果（简单的圆形渐变）
    for (int r = 0; r <= radius; r++) {
        uint8_t alpha = (intensity * (radius - r)) / radius;
        Color color = Color(
            (heartbeat_params_.color.r * alpha) / 255,
            (heartbeat_params_.color.g * alpha) / 255,
            (heartbeat_params_.color.b * alpha) / 255
        );
        
        // 绘制圆形（简化为矩形，实际可以优化为真正的圆形）
        display_->fillRect(center_x - r, center_y - r, r * 2, r * 2, color);
    }
}

void GameAnimationManager::drawAchievementText(AchievementLevel level, uint8_t intensity) {
    if (!display_) return;
    
    const char* text = getAchievementText(level);
    Color color = getAchievementColor(level);
    
    // 根据强度调整颜色亮度
    uint8_t brightness = (intensity * 255) / heartbeat_params_.max_intensity;
    Color adjusted_color = Color(
        (color.r * brightness) / 255,
        (color.g * brightness) / 255,
        (color.b * brightness) / 255
    );
    
    uint16_t text_width = strlen(text) * 12;
    uint16_t text_x = anim_x_ + (anim_width_ - text_width) / 2;
    uint16_t text_y = anim_y_ + (anim_height_ - 16) / 2;
    
    display_->drawString(text_x, text_y, text, adjusted_color, 2);
}

uint8_t GameAnimationManager::calculateHeartbeatIntensity(uint32_t elapsed_time) {
    // 心跳模式：快-慢-快
    uint32_t cycle_time = elapsed_time % 800; // 800ms一个心跳周期
    
    if (cycle_time < 200) {
        // 第一个心跳峰值
        return heartbeat_params_.base_intensity + 
               (heartbeat_params_.max_intensity - heartbeat_params_.base_intensity) * 
               (cycle_time * 255) / (200 * 255);
    } else if (cycle_time < 400) {
        // 下降
        return heartbeat_params_.max_intensity - 
               (heartbeat_params_.max_intensity - heartbeat_params_.base_intensity) * 
               ((cycle_time - 200) * 255) / (200 * 255);
    } else if (cycle_time < 600) {
        // 第二个心跳峰值
        return heartbeat_params_.base_intensity + 
               (heartbeat_params_.max_intensity - heartbeat_params_.base_intensity) * 
               ((cycle_time - 400) * 255) / (200 * 255);
    } else {
        // 最终下降
        return heartbeat_params_.max_intensity - 
               (heartbeat_params_.max_intensity - heartbeat_params_.base_intensity) * 
               ((cycle_time - 600) * 255) / (200 * 255);
    }
}

Color GameAnimationManager::getAchievementColor(AchievementLevel level) {
    switch (level) {
        case AchievementLevel::ACHIEVEMENT_32:
            return Color(255, 165, 0); // 橙色
        case AchievementLevel::ACHIEVEMENT_64:
            return Color(255, 140, 0); // 深橙色
        case AchievementLevel::ACHIEVEMENT_128:
            return Color(255, 69, 0);  // 红橙色
        case AchievementLevel::ACHIEVEMENT_256:
            return Color(255, 0, 0);   // 红色
        case AchievementLevel::ACHIEVEMENT_512:
            return Color(255, 215, 0); // 金色
        case AchievementLevel::ACHIEVEMENT_1024:
            return Color(255, 255, 0); // 黄色
        case AchievementLevel::ACHIEVEMENT_2048:
            return Color(255, 255, 255); // 白色
        default:
            return Color(255, 0, 0); // 默认红色
    }
}

const char* GameAnimationManager::getAchievementText(AchievementLevel level) {
    switch (level) {
        case AchievementLevel::ACHIEVEMENT_32:
            return "32!";
        case AchievementLevel::ACHIEVEMENT_64:
            return "64!";
        case AchievementLevel::ACHIEVEMENT_128:
            return "128!";
        case AchievementLevel::ACHIEVEMENT_256:
            return "256!";
        case AchievementLevel::ACHIEVEMENT_512:
            return "512!";
        case AchievementLevel::ACHIEVEMENT_1024:
            return "1024!";
        case AchievementLevel::ACHIEVEMENT_2048:
            return "2048!";
        default:
            return "!";
    }
}

void GameAnimationManager::clearAnimationArea() {
    if (!display_) return;
    
    display_->fillRect(anim_x_, anim_y_, anim_width_, anim_height_, Color(0, 0, 0));
}

} // namespace game2048
