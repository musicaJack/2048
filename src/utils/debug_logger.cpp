/**
 * @file debug_logger.cpp
 * @brief 调试日志器实现
 */

#include "utils/debug_logger.hpp"
#include <cstdio>
#include <cstdarg>

namespace utils {

DebugLogger::DebugLogger() 
    : log_level_(LogLevel::INFO)
    , is_enabled_(true) {
}

DebugLogger::~DebugLogger() = default;

void DebugLogger::setLogLevel(LogLevel level) {
    log_level_ = level;
}

void DebugLogger::enable(bool enable) {
    is_enabled_ = enable;
}

void DebugLogger::log(LogLevel level, const char* format, ...) {
    if (!is_enabled_ || level < log_level_) {
        return;
    }
    
    // 打印日志级别
    const char* level_str = "";
    switch (level) {
        case LogLevel::DEBUG:   level_str = "DEBUG"; break;
        case LogLevel::INFO:    level_str = "INFO"; break;
        case LogLevel::WARNING: level_str = "WARN"; break;
        case LogLevel::ERROR:   level_str = "ERROR"; break;
    }
    
    printf("[%s] ", level_str);
    
    // 打印消息
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

void DebugLogger::debug(const char* format, ...) {
    if (!is_enabled_ || log_level_ > LogLevel::DEBUG) return;
    
    printf("[DEBUG] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void DebugLogger::info(const char* format, ...) {
    if (!is_enabled_ || log_level_ > LogLevel::INFO) return;
    
    printf("[INFO] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void DebugLogger::warning(const char* format, ...) {
    if (!is_enabled_ || log_level_ > LogLevel::WARNING) return;
    
    printf("[WARN] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

void DebugLogger::error(const char* format, ...) {
    if (!is_enabled_ || log_level_ > LogLevel::ERROR) return;
    
    printf("[ERROR] ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

} // namespace utils 
