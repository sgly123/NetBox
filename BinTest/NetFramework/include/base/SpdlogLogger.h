#pragma once
#include "Logger.h"

/**
 * @brief 基于spdlog的Logger实现，适配第三方高性能日志库
 *        使用方法：Logger::setInstance(new SpdlogLogger());
 */
class SpdlogLogger : public Logger {
public:
    // 调用spdlog输出日志
    void log(LogLevel level, const std::string& msg) override;
}; 