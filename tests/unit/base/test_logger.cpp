#include <gtest/gtest.h>
#include "base/Logger.h"
#include "test_utils.h"
#include <sstream>
#include <iostream>
#include <memory>

/**
 * @brief 测试用的Logger实现，用于捕获日志输出（线程安全）
 */
class TestLogger : public Logger {
public:
    void log(LogLevel level, const std::string& msg) override {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.push_back({level, msg});
        last_level_ = level;
        last_message_ = msg;
    }

    struct LogEntry {
        LogLevel level;
        std::string message;
    };

    std::vector<LogEntry> getLogs() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return logs_;
    }

    LogLevel getLastLevel() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_level_;
    }

    std::string getLastMessage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_message_;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        logs_.clear();
    }

private:
    mutable std::mutex mutex_;
    std::vector<LogEntry> logs_;
    LogLevel last_level_ = LogLevel::DEBUG;
    std::string last_message_;
};

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_logger_ = std::make_unique<TestLogger>();
        original_logger_ = Logger::getInstance();
        Logger::setInstance(test_logger_.get());
    }

    void TearDown() override {
        Logger::setInstance(original_logger_);
        test_logger_.reset();
    }

    std::unique_ptr<TestLogger> test_logger_;
    Logger* original_logger_;
};

// 测试Logger基本功能
TEST_F(LoggerTest, BasicLogging) {
    // 测试不同级别的日志
    Logger::debug("Debug message");
    EXPECT_EQ(test_logger_->getLastLevel(), LogLevel::DEBUG);
    EXPECT_EQ(test_logger_->getLastMessage(), "Debug message");

    Logger::info("Info message");
    EXPECT_EQ(test_logger_->getLastLevel(), LogLevel::INFO);
    EXPECT_EQ(test_logger_->getLastMessage(), "Info message");

    Logger::warn("Warning message");
    EXPECT_EQ(test_logger_->getLastLevel(), LogLevel::WARN);
    EXPECT_EQ(test_logger_->getLastMessage(), "Warning message");

    Logger::error("Error message");
    EXPECT_EQ(test_logger_->getLastLevel(), LogLevel::ERROR);
    EXPECT_EQ(test_logger_->getLastMessage(), "Error message");
}

// 测试日志记录顺序
TEST_F(LoggerTest, LoggingOrder) {
    Logger::debug("First");
    Logger::info("Second");
    Logger::warn("Third");
    Logger::error("Fourth");

    const auto& logs = test_logger_->getLogs();
    ASSERT_EQ(logs.size(), 4);
    
    EXPECT_EQ(logs[0].level, LogLevel::DEBUG);
    EXPECT_EQ(logs[0].message, "First");
    
    EXPECT_EQ(logs[1].level, LogLevel::INFO);
    EXPECT_EQ(logs[1].message, "Second");
    
    EXPECT_EQ(logs[2].level, LogLevel::WARN);
    EXPECT_EQ(logs[2].message, "Third");
    
    EXPECT_EQ(logs[3].level, LogLevel::ERROR);
    EXPECT_EQ(logs[3].message, "Fourth");
}

// 测试空消息
TEST_F(LoggerTest, EmptyMessage) {
    Logger::info("");
    EXPECT_EQ(test_logger_->getLastLevel(), LogLevel::INFO);
    EXPECT_EQ(test_logger_->getLastMessage(), "");
}

// 测试长消息
TEST_F(LoggerTest, LongMessage) {
    std::string long_message = TestUtils::generateRandomString(1000);
    Logger::info(long_message);
    EXPECT_EQ(test_logger_->getLastLevel(), LogLevel::INFO);
    EXPECT_EQ(test_logger_->getLastMessage(), long_message);
}

// 测试特殊字符
TEST_F(LoggerTest, SpecialCharacters) {
    std::string special_msg = "Message with\nnewlines\tand\ttabs";
    Logger::info(special_msg);
    EXPECT_EQ(test_logger_->getLastMessage(), special_msg);
}

// 测试Unicode字符
TEST_F(LoggerTest, UnicodeCharacters) {
    std::string unicode_msg = "测试中文消息 🚀 Test Unicode";
    Logger::info(unicode_msg);
    EXPECT_EQ(test_logger_->getLastMessage(), unicode_msg);
}

// 测试Logger单例模式
TEST_F(LoggerTest, SingletonPattern) {
    Logger* instance1 = Logger::getInstance();
    Logger* instance2 = Logger::getInstance();
    EXPECT_EQ(instance1, instance2);
}

// 测试Logger实例切换
TEST_F(LoggerTest, InstanceSwitching) {
    auto another_logger = std::make_unique<TestLogger>();
    Logger::setInstance(another_logger.get());
    
    Logger::info("Test message");
    
    // 原来的logger不应该收到消息
    EXPECT_TRUE(test_logger_->getLogs().empty());
    
    // 新的logger应该收到消息
    EXPECT_EQ(another_logger->getLogs().size(), 1);
    EXPECT_EQ(another_logger->getLastMessage(), "Test message");
}

// 测试多线程日志记录
TEST_F(LoggerTest, MultiThreadLogging) {
    const int thread_count = 10;
    const int messages_per_thread = 100;
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([i, messages_per_thread]() {
            for (int j = 0; j < messages_per_thread; ++j) {
                Logger::info("Thread " + std::to_string(i) + " Message " + std::to_string(j));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有消息都被记录
    const auto& logs = test_logger_->getLogs();
    EXPECT_EQ(logs.size(), thread_count * messages_per_thread);
    
    // 验证所有消息都是INFO级别
    for (const auto& log : logs) {
        EXPECT_EQ(log.level, LogLevel::INFO);
    }
}

// 测试Logger性能
TEST_F(LoggerTest, LoggingPerformance) {
    const int message_count = 10000;
    
    double execution_time = TestUtils::measureExecutionTime([&]() {
        for (int i = 0; i < message_count; ++i) {
            Logger::info("Performance test message " + std::to_string(i));
        }
    });
    
    // 验证日志记录性能（应该在合理时间内完成）
    EXPECT_LT(execution_time, 1000.0); // 应该在1秒内完成
    
    // 验证所有消息都被记录
    EXPECT_EQ(test_logger_->getLogs().size(), message_count);
    
    std::cout << "Logged " << message_count << " messages in " 
              << execution_time << " ms" << std::endl;
    std::cout << "Average time per message: " 
              << execution_time / message_count << " ms" << std::endl;
}

// 测试Logger内存使用
TEST_F(LoggerTest, MemoryUsage) {
    const int message_count = 1000;
    
    // 记录大量消息
    for (int i = 0; i < message_count; ++i) {
        Logger::info("Memory test message " + std::to_string(i));
    }
    
    // 验证消息数量
    EXPECT_EQ(test_logger_->getLogs().size(), message_count);
    
    // 清空日志
    test_logger_->clear();
    EXPECT_EQ(test_logger_->getLogs().size(), 0);
}

// 测试LogLevel枚举
TEST_F(LoggerTest, LogLevelEnum) {
    // 测试LogLevel枚举值
    EXPECT_EQ(static_cast<int>(LogLevel::DEBUG), 0);
    EXPECT_EQ(static_cast<int>(LogLevel::INFO), 1);
    EXPECT_EQ(static_cast<int>(LogLevel::WARN), 2);
    EXPECT_EQ(static_cast<int>(LogLevel::ERROR), 3);
}

// 测试Logger异常处理
TEST_F(LoggerTest, ExceptionHandling) {
    // 测试nullptr Logger
    Logger::setInstance(nullptr);
    
    // 这些调用不应该崩溃
    EXPECT_NO_THROW(Logger::debug("Debug with null logger"));
    EXPECT_NO_THROW(Logger::info("Info with null logger"));
    EXPECT_NO_THROW(Logger::warn("Warn with null logger"));
    EXPECT_NO_THROW(Logger::error("Error with null logger"));
}
