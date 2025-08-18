#pragma once

namespace utils {

// 日志级别枚举
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/**
 * @brief 调试日志器类
 * 
 * 提供不同级别的日志输出功能
 */
class DebugLogger {
public:
    DebugLogger();
    ~DebugLogger();

    // 禁用拷贝构造和赋值
    DebugLogger(const DebugLogger&) = delete;
    DebugLogger& operator=(const DebugLogger&) = delete;

    // 配置
    void setLogLevel(LogLevel level);
    void enable(bool enable);

    // 日志输出
    void log(LogLevel level, const char* format, ...);
    void debug(const char* format, ...);
    void info(const char* format, ...);
    void warning(const char* format, ...);
    void error(const char* format, ...);

private:
    LogLevel log_level_;
    bool is_enabled_;
};

} // namespace utils 
