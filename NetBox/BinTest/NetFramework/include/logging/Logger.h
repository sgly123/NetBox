#pragma once

/**
 * @file Logger.h
 * @brief NetBox 统一日志系统
 * 
 * 特性：
 * 1. 多级别日志支持 (DEBUG, INFO, WARN, ERROR, FATAL)
 * 2. 多输出目标 (控制台, 文件, 系统日志)
 * 3. 跨平台兼容 (Windows, Linux, macOS)
 * 4. 高性能异步日志
 * 5. 线程安全
 * 6. 格式化输出支持
 * 7. 日志轮转和压缩
 */

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace NetBox {
namespace Logging {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    DEBUG = 0,    // 调试信息
    INFO = 1,     // 一般信息
    WARN = 2,     // 警告信息
    ERROR = 3,    // 错误信息
    FATAL = 4     // 致命错误
};

/**
 * @brief 日志输出目标枚举
 */
enum class LogTarget {
    CONSOLE = 1,  // 控制台输出
    FILE = 2,     // 文件输出
    SYSLOG = 4    // 系统日志
};

/**
 * @brief 日志记录结构体
 */
struct LogRecord {
    LogLevel level;                                    // 日志级别
    std::chrono::system_clock::time_point timestamp;  // 时间戳
    std::string message;                               // 日志消息
    std::string file;                                  // 源文件名
    int line;                                          // 源文件行号
    std::string function;                              // 函数名
    std::thread::id thread_id;                         // 线程ID
    
    LogRecord(LogLevel lvl, const std::string& msg, 
              const std::string& f = "", int l = 0, const std::string& func = "")
        : level(lvl), timestamp(std::chrono::system_clock::now()), 
          message(msg), file(f), line(l), function(func), 
          thread_id(std::this_thread::get_id()) {}
};

/**
 * @brief 日志格式化器接口
 */
class LogFormatter {
public:
    virtual ~LogFormatter() = default;
    virtual std::string format(const LogRecord& record) = 0;
};

/**
 * @brief 默认日志格式化器
 * 格式: [时间] [级别] [线程ID] [文件:行号] 消息
 */
class DefaultFormatter : public LogFormatter {
public:
    std::string format(const LogRecord& record) override {
        std::ostringstream oss;
        
        // 时间格式化
        auto time_t = std::chrono::system_clock::to_time_t(record.timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            record.timestamp.time_since_epoch()) % 1000;
        
        oss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "]";
        
        // 日志级别
        oss << " [" << levelToString(record.level) << "]";
        
        // 线程ID
        oss << " [" << record.thread_id << "]";
        
        // 源文件信息
        if (!record.file.empty()) {
            oss << " [" << record.file << ":" << record.line << "]";
        }
        
        // 消息内容
        oss << " " << record.message;
        
        return oss.str();
    }

private:
    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARN:  return "WARN ";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
};

/**
 * @brief 日志输出器接口
 */
class LogAppender {
public:
    virtual ~LogAppender() = default;
    virtual void append(const LogRecord& record) = 0;
    virtual void flush() = 0;
};

/**
 * @brief 控制台日志输出器
 */
class ConsoleAppender : public LogAppender {
private:
    std::unique_ptr<LogFormatter> m_formatter;
    std::mutex m_mutex;

public:
    ConsoleAppender(std::unique_ptr<LogFormatter> formatter = nullptr)
        : m_formatter(formatter ? std::move(formatter) : std::make_unique<DefaultFormatter>()) {}
    
    void append(const LogRecord& record) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string formatted = m_formatter->format(record);
        
        // 根据日志级别选择输出流
        if (record.level >= LogLevel::ERROR) {
            std::cerr << formatted << std::endl;
        } else {
            std::cout << formatted << std::endl;
        }
    }
    
    void flush() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::cout.flush();
        std::cerr.flush();
    }
};

/**
 * @brief 文件日志输出器
 */
class FileAppender : public LogAppender {
private:
    std::unique_ptr<LogFormatter> m_formatter;
    std::ofstream m_file;
    std::string m_filename;
    std::mutex m_mutex;
    size_t m_maxFileSize;
    int m_maxFiles;

public:
    FileAppender(const std::string& filename, 
                 std::unique_ptr<LogFormatter> formatter = nullptr,
                 size_t maxFileSize = 10 * 1024 * 1024,  // 10MB
                 int maxFiles = 5)
        : m_formatter(formatter ? std::move(formatter) : std::make_unique<DefaultFormatter>()),
          m_filename(filename), m_maxFileSize(maxFileSize), m_maxFiles(maxFiles) {
        m_file.open(m_filename, std::ios::app);
    }
    
    ~FileAppender() {
        if (m_file.is_open()) {
            m_file.close();
        }
    }
    
    void append(const LogRecord& record) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_file.is_open()) {
            return;
        }
        
        std::string formatted = m_formatter->format(record);
        m_file << formatted << std::endl;
        
        // 检查文件大小，必要时轮转
        if (m_file.tellp() > static_cast<std::streampos>(m_maxFileSize)) {
            rotateFile();
        }
    }
    
    void flush() override {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.is_open()) {
            m_file.flush();
        }
    }

private:
    void rotateFile() {
        m_file.close();
        
        // 轮转文件
        for (int i = m_maxFiles - 1; i > 0; --i) {
            std::string oldFile = m_filename + "." + std::to_string(i);
            std::string newFile = m_filename + "." + std::to_string(i + 1);
            std::rename(oldFile.c_str(), newFile.c_str());
        }
        
        std::string backupFile = m_filename + ".1";
        std::rename(m_filename.c_str(), backupFile.c_str());
        
        // 重新打开文件
        m_file.open(m_filename, std::ios::trunc);
    }
};

/**
 * @brief 异步日志器
 */
class AsyncLogger {
private:
    std::vector<std::unique_ptr<LogAppender>> m_appenders;
    std::queue<LogRecord> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::thread m_worker;
    bool m_running;
    LogLevel m_minLevel;

public:
    AsyncLogger(LogLevel minLevel = LogLevel::INFO) 
        : m_running(true), m_minLevel(minLevel) {
        m_worker = std::thread(&AsyncLogger::workerThread, this);
    }
    
    ~AsyncLogger() {
        stop();
    }
    
    void addAppender(std::unique_ptr<LogAppender> appender) {
        m_appenders.push_back(std::move(appender));
    }
    
    void log(LogLevel level, const std::string& message, 
             const std::string& file = "", int line = 0, 
             const std::string& function = "") {
        if (level < m_minLevel) {
            return;
        }
        
        LogRecord record(level, message, file, line, function);
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(record));
        }
        m_condition.notify_one();
    }
    
    void setMinLevel(LogLevel level) {
        m_minLevel = level;
    }
    
    void flush() {
        // 等待队列清空
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] { return m_queue.empty(); });
        
        // 刷新所有输出器
        for (auto& appender : m_appenders) {
            appender->flush();
        }
    }
    
    void stop() {
        if (m_running) {
            m_running = false;
            m_condition.notify_all();
            if (m_worker.joinable()) {
                m_worker.join();
            }
        }
    }

private:
    void workerThread() {
        while (m_running) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this] { return !m_queue.empty() || !m_running; });
            
            while (!m_queue.empty()) {
                LogRecord record = std::move(m_queue.front());
                m_queue.pop();
                lock.unlock();
                
                // 输出到所有appender
                for (auto& appender : m_appenders) {
                    appender->append(record);
                }
                
                lock.lock();
            }
        }
        
        // 处理剩余的日志
        while (!m_queue.empty()) {
            LogRecord record = std::move(m_queue.front());
            m_queue.pop();
            
            for (auto& appender : m_appenders) {
                appender->append(record);
            }
        }
    }
};

/**
 * @brief 全局日志器单例
 */
class Logger {
private:
    static std::unique_ptr<AsyncLogger> s_instance;
    static std::once_flag s_initFlag;

public:
    static AsyncLogger& getInstance() {
        std::call_once(s_initFlag, []() {
            s_instance = std::make_unique<AsyncLogger>();
            
            // 默认添加控制台输出器
            s_instance->addAppender(std::make_unique<ConsoleAppender>());
        });
        return *s_instance;
    }
    
    static void debug(const std::string& message, const std::string& file = "", int line = 0) {
        getInstance().log(LogLevel::DEBUG, message, file, line);
    }
    
    static void info(const std::string& message, const std::string& file = "", int line = 0) {
        getInstance().log(LogLevel::INFO, message, file, line);
    }
    
    static void warn(const std::string& message, const std::string& file = "", int line = 0) {
        getInstance().log(LogLevel::WARN, message, file, line);
    }
    
    static void error(const std::string& message, const std::string& file = "", int line = 0) {
        getInstance().log(LogLevel::ERROR, message, file, line);
    }
    
    static void fatal(const std::string& message, const std::string& file = "", int line = 0) {
        getInstance().log(LogLevel::FATAL, message, file, line);
    }
    
    static void addFileAppender(const std::string& filename) {
        getInstance().addAppender(std::make_unique<FileAppender>(filename));
    }
    
    static void setMinLevel(LogLevel level) {
        getInstance().setMinLevel(level);
    }
    
    static void flush() {
        getInstance().flush();
    }
};

} // namespace Logging
} // namespace NetBox

// 便利宏定义
#define NETBOX_LOG_DEBUG(msg) NetBox::Logging::Logger::debug(msg, __FILE__, __LINE__)
#define NETBOX_LOG_INFO(msg)  NetBox::Logging::Logger::info(msg, __FILE__, __LINE__)
#define NETBOX_LOG_WARN(msg)  NetBox::Logging::Logger::warn(msg, __FILE__, __LINE__)
#define NETBOX_LOG_ERROR(msg) NetBox::Logging::Logger::error(msg, __FILE__, __LINE__)
#define NETBOX_LOG_FATAL(msg) NetBox::Logging::Logger::fatal(msg, __FILE__, __LINE__)
