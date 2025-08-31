#include "base/AsyncConsoleLogger.h"

void AsyncConsoleLogger::log(LogLevel level, const std::string& msg) {
    // 将日志委托给AsyncLogger处理
    switch (level) {
        case LogLevel::DEBUG:
            AsyncLogger::getInstance().debug(msg);
            break;
        case LogLevel::INFO:
            AsyncLogger::getInstance().info(msg);
            break;
        case LogLevel::WARN:
            AsyncLogger::getInstance().warn(msg);
            break;
        case LogLevel::ERROR:
            AsyncLogger::getInstance().error(msg);
            break;
    }
} 