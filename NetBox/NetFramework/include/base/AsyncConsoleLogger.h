#pragma once
#include "base/Logger.h"
#include "base/AsyncLogger.h"

/**
 * @brief 基于异步日志的控制台日志实现
 * 继承自Logger基类，使用AsyncLogger进行异步输出
 */
class AsyncConsoleLogger : public Logger {
public:
    void log(LogLevel level, const std::string& msg) override;
    
private:
    // 将LogLevel转换为AsyncLogger的LogLevel
    LogLevel convertLevel(LogLevel level);
}; 