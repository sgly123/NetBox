#include "base/Logger.h"
#include <iostream>
#include <mutex>

// 全局Logger指针和互斥锁，保证线程安全
static Logger* g_logger = nullptr;
static std::mutex g_logger_mutex;

// 获取全局Logger实例，默认使用ConsoleLogger
Logger* Logger::getInstance() {
    std::lock_guard<std::mutex> lock(g_logger_mutex);
    if (!g_logger) g_logger = new ConsoleLogger();
    return g_logger;
}

// 设置全局Logger实例（可切换为其他实现）
void Logger::setInstance(Logger* logger) {
    std::lock_guard<std::mutex> lock(g_logger_mutex);
    g_logger = logger;
}

// 便捷静态接口，直接输出不同级别日志
void Logger::debug(const std::string& msg) { getInstance()->log(LogLevel::DEBUG, msg); }
void Logger::info(const std::string& msg)  { getInstance()->log(LogLevel::INFO, msg); }
void Logger::warn(const std::string& msg)  { getInstance()->log(LogLevel::WARN, msg); }
void Logger::error(const std::string& msg) { getInstance()->log(LogLevel::ERROR, msg); }

// 控制台日志实现，输出到终端
void ConsoleLogger::log(LogLevel level, const std::string& msg) {
    // 日志级别字符串
    static const char* levelStr[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    std::cout << "[" << levelStr[(int)level] << "] " << msg << std::endl;
} 