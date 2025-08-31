#include "base/AsyncLogger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

AsyncLogger::AsyncLogger() : stop(false), use_file(false) {
    // 启动工作线程
    worker_thread = std::thread(&AsyncLogger::worker, this);
}

AsyncLogger::~AsyncLogger() {
    stop = true;
    condition.notify_one();
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
    if (log_file.is_open()) {
        log_file.close();
    }
}

AsyncLogger& AsyncLogger::getInstance() {
    static AsyncLogger instance;
    return instance;
}

void AsyncLogger::log(LogLevel level, const std::string& message) {
    auto entry = std::make_unique<LogEntry>(level, message);
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        log_queue.push(std::move(entry));
    }
    
    condition.notify_one();
}

void AsyncLogger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void AsyncLogger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void AsyncLogger::warn(const std::string& message) {
    log(LogLevel::WARN, message);
}

void AsyncLogger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void AsyncLogger::worker() {
    while (!stop) {
        std::unique_ptr<LogEntry> entry;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !log_queue.empty(); });
            
            if (stop && log_queue.empty()) {
                break;
            }
            
            if (!log_queue.empty()) {
                entry = std::move(log_queue.front());
                log_queue.pop();
            }
        }
        
        if (entry) {
            // 格式化日志消息
            std::string formatted_msg = getCurrentTime() + " [" + 
                                      getLevelString(entry->level) + "] " + 
                                      entry->message + "\n";
            
            // 输出到控制台
            std::cout << formatted_msg;
            std::cout.flush();
            
            // 如果启用文件日志，也写入文件
            if (use_file && log_file.is_open()) {
                log_file << formatted_msg;
                log_file.flush();
            }
        }
    }
    
    // 处理剩余的日志条目
    std::unique_ptr<LogEntry> entry;
    while (true) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (log_queue.empty()) {
                break;
            }
            entry = std::move(log_queue.front());
            log_queue.pop();
        }
        
        if (entry) {
            std::string formatted_msg = getCurrentTime() + " [" + 
                                      getLevelString(entry->level) + "] " + 
                                      entry->message + "\n";
            std::cout << formatted_msg;
            std::cout.flush();
            
            if (use_file && log_file.is_open()) {
                log_file << formatted_msg;
                log_file.flush();
            }
        }
    }
}

std::string AsyncLogger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string AsyncLogger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
} 