#pragma once
#include <string>

// 日志级别枚举
// DEBUG: 调试信息，INFO: 普通信息，WARN: 警告，ERROR: 错误
enum class LogLevel { DEBUG, INFO, WARN, ERROR };

/**
 * @brief 日志抽象基类，定义日志接口，便于后续扩展和替换不同日志实现
 */
class Logger {
public:
    virtual ~Logger() = default;
    // 记录一条日志
    virtual void log(LogLevel level, const std::string& msg) = 0;

    // 获取全局Logger实例（单例模式）
    static Logger* getInstance();
    // 设置全局Logger实例（可切换为其他实现）
    static void setInstance(Logger* logger);

    // 便捷静态接口，直接调用即可输出日志
    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
};

/**
 * @brief 简单的控制台日志实现，输出到终端
 */
class ConsoleLogger : public Logger {
public:
    // 实现日志输出到控制台
    void log(LogLevel level, const std::string& msg) override;
}; 