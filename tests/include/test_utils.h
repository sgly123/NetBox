#pragma once

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <fstream>
#include <random>
#include <functional>

/**
 * @brief 测试工具类，提供通用的测试辅助功能
 */
class TestUtils {
public:
    /**
     * @brief 创建临时测试文件
     * @param filename 文件名
     * @param content 文件内容
     * @return 是否创建成功
     */
    static bool createTempFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        file << content;
        file.close();
        return true;
    }

    /**
     * @brief 删除临时测试文件
     * @param filename 文件名
     * @return 是否删除成功
     */
    static bool removeTempFile(const std::string& filename) {
        return std::remove(filename.c_str()) == 0;
    }

    /**
     * @brief 生成随机字符串
     * @param length 字符串长度
     * @return 随机字符串
     */
    static std::string generateRandomString(size_t length) {
        const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, chars.size() - 1);
        
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            result += chars[dis(gen)];
        }
        return result;
    }

    /**
     * @brief 生成随机整数
     * @param min 最小值
     * @param max 最大值
     * @return 随机整数
     */
    static int generateRandomInt(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    /**
     * @brief 等待指定时间
     * @param milliseconds 等待时间（毫秒）
     */
    static void waitFor(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    /**
     * @brief 等待条件满足或超时
     * @param condition 条件函数
     * @param timeout_ms 超时时间（毫秒）
     * @param check_interval_ms 检查间隔（毫秒）
     * @return 条件是否在超时前满足
     */
    static bool waitForCondition(std::function<bool()> condition, 
                                int timeout_ms = 5000, 
                                int check_interval_ms = 10) {
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start).count() < timeout_ms) {
            if (condition()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(check_interval_ms));
        }
        return false;
    }

    /**
     * @brief 测量函数执行时间
     * @param func 要测量的函数
     * @return 执行时间（毫秒）
     */
    template<typename Func>
    static double measureExecutionTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return duration.count() / 1000.0; // 返回毫秒
    }

    /**
     * @brief 创建测试配置文件
     * @param filename 配置文件名
     * @param format 配置文件格式（"txt" 或 "yaml"）
     * @return 是否创建成功
     */
    static bool createTestConfigFile(const std::string& filename, const std::string& format = "txt") {
        std::string content;
        if (format == "yaml") {
            content = R"(
application:
  type: echo
  name: test_server

network:
  ip: 127.0.0.1
  port: 8888
  io_type: epoll

thread_pool:
  size: 4
  max_queue_size: 1000

logging:
  level: info
  file: test.log
  async: true
)";
        } else {
            content = R"(
# Test configuration file
host=127.0.0.1
port=8888
io_type=epoll
thread_count=4
log_level=info
log_file=test.log
)";
        }
        return createTempFile(filename, content);
    }

    /**
     * @brief 获取测试数据目录路径
     * @return 测试数据目录路径
     */
    static std::string getTestDataDir() {
        return "tests/data/";
    }

    /**
     * @brief 创建测试数据目录
     * @return 是否创建成功
     */
    static bool createTestDataDir() {
        return system(("mkdir -p " + getTestDataDir()).c_str()) == 0;
    }
};

/**
 * @brief 性能测试基类
 */
class PerformanceTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    void TearDown() override {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time_);
        std::cout << "Test execution time: " << duration.count() << " ms" << std::endl;
    }

    /**
     * @brief 获取测试执行时间
     * @return 执行时间（毫秒）
     */
    double getExecutionTime() const {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - start_time_);
        return duration.count() / 1000.0;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

/**
 * @brief 多线程测试基类
 */
class MultiThreadTestBase : public ::testing::Test {
protected:
    /**
     * @brief 运行多线程测试
     * @param thread_count 线程数量
     * @param task 每个线程执行的任务
     * @param iterations 每个线程的迭代次数
     */
    void runMultiThreadTest(int thread_count, 
                           std::function<void(int thread_id, int iteration)> task,
                           int iterations = 100) {
        std::vector<std::thread> threads;
        std::vector<std::exception_ptr> exceptions(thread_count);
        
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([i, iterations, task, &exceptions]() {
                try {
                    for (int j = 0; j < iterations; ++j) {
                        task(i, j);
                    }
                } catch (...) {
                    exceptions[i] = std::current_exception();
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        // 检查是否有异常
        for (int i = 0; i < thread_count; ++i) {
            if (exceptions[i]) {
                std::rethrow_exception(exceptions[i]);
            }
        }
    }
};

// 常用的测试宏定义
#define EXPECT_TIMEOUT(condition, timeout_ms) \
    EXPECT_TRUE(TestUtils::waitForCondition([&]() { return condition; }, timeout_ms))

#define ASSERT_TIMEOUT(condition, timeout_ms) \
    ASSERT_TRUE(TestUtils::waitForCondition([&]() { return condition; }, timeout_ms))

#define EXPECT_EXECUTION_TIME_LESS_THAN(func, max_time_ms) \
    do { \
        double time = TestUtils::measureExecutionTime([&]() { func; }); \
        EXPECT_LT(time, max_time_ms) << "Execution time: " << time << " ms"; \
    } while(0)

#define ASSERT_EXECUTION_TIME_LESS_THAN(func, max_time_ms) \
    do { \
        double time = TestUtils::measureExecutionTime([&]() { func; }); \
        ASSERT_LT(time, max_time_ms) << "Execution time: " << time << " ms"; \
    } while(0)
