#include "base/SpdlogLogger.h"
#include <spdlog/spdlog.h>

// 将自定义LogLevel映射为spdlog的日志级别
static spdlog::level::level_enum toSpdlogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return spdlog::level::debug;
        case LogLevel::INFO:  return spdlog::level::info;
        case LogLevel::WARN:  return spdlog::level::warn;
        case LogLevel::ERROR: return spdlog::level::err;
        default:              return spdlog::level::info;
    }
}

// 实现日志输出，调用spdlog
void SpdlogLogger::log(LogLevel level, const std::string& msg) {
    spdlog::log(toSpdlogLevel(level), msg);
} 