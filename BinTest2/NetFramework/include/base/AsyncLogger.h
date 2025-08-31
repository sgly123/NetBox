#pragma once
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>

// 使用现有的LogLevel定义，避免重复
#include "base/Logger.h"

class AsyncLogger {
public:
    static AsyncLogger& getInstance();
    
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    
    ~AsyncLogger();
    
    // 禁用拷贝
    AsyncLogger(const AsyncLogger&) = delete;
    AsyncLogger& operator=(const AsyncLogger&) = delete;

private:
    AsyncLogger();
    
    struct LogEntry {
        LogLevel level;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        
        LogEntry(LogLevel l, const std::string& msg) 
            : level(l), message(msg), timestamp(std::chrono::system_clock::now()) {}
    };
    
    void worker();
    std::string getLevelString(LogLevel level);
    std::string getCurrentTime();
    
    std::queue<std::unique_ptr<LogEntry>> log_queue;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::thread worker_thread;
    
    // 日志文件
    std::ofstream log_file;
    bool use_file;
};

// 便捷宏定义
#define LOG_DEBUG(msg) AsyncLogger::getInstance().debug(msg)
#define LOG_INFO(msg) AsyncLogger::getInstance().info(msg)
#define LOG_WARN(msg) AsyncLogger::getInstance().warn(msg)
#define LOG_ERROR(msg) AsyncLogger::getInstance().error(msg) 