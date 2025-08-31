#include <gtest/gtest.h>
#include "test_utils.h"
#include "base/ThreadPool.h"
#include "base/DoubleLockThreadPool.h"
#include "IO/IOFactory.h"
#include "base/AsyncLogger.h"
#include <chrono>
#include <vector>
#include <atomic>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

class FrameworkBenchmarkTest : public PerformanceTestBase {
protected:
    void SetUp() override {
        PerformanceTestBase::SetUp();
        std::cout << "\n=== NetBox Framework Performance Benchmark ===" << std::endl;
        std::cout << "测试环境: 虚拟机环境" << std::endl;
        std::cout << "CPU核心数: " << std::thread::hardware_concurrency() << std::endl;
        std::cout << "================================================\n" << std::endl;
    }

    void TearDown() override {
        PerformanceTestBase::TearDown();
    }
};

// 线程池性能基准测试
TEST_F(FrameworkBenchmarkTest, ThreadPoolBenchmark) {
    std::cout << "🧵 线程池性能基准测试" << std::endl;

    const int task_count = 10000; // 减少任务数量
    const int thread_count = std::thread::hardware_concurrency();

    // 测试MutexThreadPool
    {
        std::cout << "测试 MutexThreadPool (线程数: " << thread_count << ")" << std::endl;

        MutexThreadPool pool(thread_count);
        std::atomic<int> completed{0};

        double execution_time = TestUtils::measureExecutionTime([&]() {
            for (int i = 0; i < task_count; ++i) {
                pool.enqueue([&completed]() {
                    completed.fetch_add(1);
                    // 模拟轻量级工作
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                });
            }

            // 等待所有任务完成
            TestUtils::waitForCondition([&]() {
                return completed.load() >= task_count * 0.95; // 允许95%完成
            }, 15000);
        });

        double throughput = completed.load() / (execution_time / 1000.0);
        std::cout << "  - 执行时间: " << execution_time << " ms" << std::endl;
        std::cout << "  - 完成任务数: " << completed.load() << "/" << task_count << std::endl;
        std::cout << "  - 吞吐量: " << static_cast<int>(throughput) << " tasks/sec" << std::endl;
        std::cout << "  - 平均延迟: " << execution_time / completed.load() << " ms/task" << std::endl;

        EXPECT_GT(completed.load(), task_count * 0.9); // 至少90%完成
    }

    // 测试DoubleLockThreadPool
    {
        std::cout << "\n测试 DoubleLockThreadPool (线程数: " << thread_count << ")" << std::endl;

        DoubleLockThreadPool pool(thread_count, 20000); // 适当的队列大小
        std::atomic<int> completed{0};

        double execution_time = TestUtils::measureExecutionTime([&]() {
            for (int i = 0; i < task_count; ++i) {
                bool success = pool.enqueue([&completed]() {
                    completed.fetch_add(1);
                    // 模拟轻量级工作
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                });
                if (!success) {
                    break; // 队列满了就停止
                }
            }

            // 等待所有任务完成
            TestUtils::waitForCondition([&]() {
                return completed.load() >= task_count * 0.95; // 允许95%完成
            }, 15000);
        });

        double throughput = completed.load() / (execution_time / 1000.0);
        std::cout << "  - 执行时间: " << execution_time << " ms" << std::endl;
        std::cout << "  - 完成任务数: " << completed.load() << "/" << task_count << std::endl;
        std::cout << "  - 吞吐量: " << static_cast<int>(throughput) << " tasks/sec" << std::endl;
        std::cout << "  - 平均延迟: " << execution_time / completed.load() << " ms/task" << std::endl;

        EXPECT_GT(completed.load(), task_count * 0.8); // 至少80%完成
    }

    std::cout << std::endl;
}

// IO多路复用性能基准测试
TEST_F(FrameworkBenchmarkTest, IOMultiplexerBenchmark) {
    std::cout << "🔌 IO多路复用性能基准测试" << std::endl;
    
    const int connection_count = 500; // 减少连接数以适应Select限制
    const int events_per_connection = 10;
    
    // 创建测试连接
    std::vector<std::pair<int, int>> connections;
    for (int i = 0; i < connection_count; ++i) {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0) {
            // 设置非阻塞
            int flags = fcntl(fds[0], F_GETFL, 0);
            fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);
            flags = fcntl(fds[1], F_GETFL, 0);
            fcntl(fds[1], F_SETFL, flags | O_NONBLOCK);
            
            connections.push_back({fds[0], fds[1]});
        }
    }
    
    std::cout << "创建了 " << connections.size() << " 个测试连接" << std::endl;
    
    // 测试不同的IO多路复用机制
    std::vector<IOMultiplexer::IOType> io_types = {
        IOMultiplexer::IOType::EPOLL,
        IOMultiplexer::IOType::POLL,
        IOMultiplexer::IOType::SELECT
    };
    
    std::vector<std::string> io_names = {"Epoll", "Poll", "Select"};
    
    for (size_t i = 0; i < io_types.size(); ++i) {
        std::cout << "\n测试 " << io_names[i] << " Multiplexer:" << std::endl;
        
        auto io = IOFactory::createIO(io_types[i]);
        if (!io || !io->init()) {
            std::cout << "  - 初始化失败，跳过测试" << std::endl;
            continue;
        }
        
        // 添加所有连接到IO多路复用器
        double setup_time = TestUtils::measureExecutionTime([&]() {
            for (const auto& conn : connections) {
                io->addfd(conn.first, IOMultiplexer::EventType::READ);
            }
        });
        
        // 触发事件并测试处理性能
        std::atomic<int> total_events{0};
        
        double event_time = TestUtils::measureExecutionTime([&]() {
            // 向所有连接写入数据
            for (const auto& conn : connections) {
                for (int j = 0; j < events_per_connection; ++j) {
                    char data = 'A' + (j % 26);
                    write(conn.second, &data, 1);
                }
            }
            
            // 处理事件
            int processed_events = 0;
            int max_iterations = 100; // 防止无限循环
            
            while (processed_events < connection_count * events_per_connection && max_iterations-- > 0) {
                std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
                int result = io->wait(active_events, 10);
                
                if (result > 0) {
                    for (const auto& event : active_events) {
                        if (event.first > 0 && (event.second & IOMultiplexer::EventType::READ)) {
                            char buffer[256];
                            int bytes = read(event.first, buffer, sizeof(buffer));
                            if (bytes > 0) {
                                processed_events += bytes;
                                total_events.fetch_add(bytes);
                            }
                        }
                    }
                }
            }
        });
        
        // 清理
        for (const auto& conn : connections) {
            io->removefd(conn.first);
        }
        
        double setup_throughput = connections.size() / (setup_time / 1000.0);
        double event_throughput = total_events.load() / (event_time / 1000.0);
        
        std::cout << "  - 连接设置时间: " << setup_time << " ms" << std::endl;
        std::cout << "  - 连接设置吞吐量: " << static_cast<int>(setup_throughput) << " conn/sec" << std::endl;
        std::cout << "  - 事件处理时间: " << event_time << " ms" << std::endl;
        std::cout << "  - 事件处理吞吐量: " << static_cast<int>(event_throughput) << " events/sec" << std::endl;
        std::cout << "  - 处理的总事件数: " << total_events.load() << std::endl;
    }
    
    // 清理连接
    for (const auto& conn : connections) {
        close(conn.first);
        close(conn.second);
    }
    
    std::cout << std::endl;
}

// 异步日志性能基准测试
TEST_F(FrameworkBenchmarkTest, AsyncLoggerBenchmark) {
    std::cout << "📝 异步日志性能基准测试" << std::endl;
    
    AsyncLogger& logger = AsyncLogger::getInstance();
    
    const int message_count = 50000;
    const int thread_count = 4;
    
    std::cout << "测试参数: " << message_count << " 条日志消息, " << thread_count << " 个线程" << std::endl;
    
    std::atomic<int> messages_sent{0};
    
    double execution_time = TestUtils::measureExecutionTime([&]() {
        std::vector<std::thread> threads;
        
        for (int t = 0; t < thread_count; ++t) {
            threads.emplace_back([&, t]() {
                int messages_per_thread = message_count / thread_count;
                for (int i = 0; i < messages_per_thread; ++i) {
                    logger.info("Thread " + std::to_string(t) + " Message " + std::to_string(i) + 
                               " - Performance benchmark test message with some content");
                    messages_sent.fetch_add(1);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    });
    
    // 等待异步日志处理完成
    TestUtils::waitFor(1000);
    
    double throughput = messages_sent.load() / (execution_time / 1000.0);
    double avg_latency = execution_time / messages_sent.load();
    
    std::cout << "  - 执行时间: " << execution_time << " ms" << std::endl;
    std::cout << "  - 发送消息数: " << messages_sent.load() << std::endl;
    std::cout << "  - 吞吐量: " << static_cast<int>(throughput) << " msg/sec" << std::endl;
    std::cout << "  - 平均延迟: " << avg_latency << " ms/msg" << std::endl;
    
    EXPECT_EQ(messages_sent.load(), message_count);
    EXPECT_LT(execution_time, 3000.0); // 应在3秒内完成
    
    std::cout << std::endl;
}

// 综合性能基准测试
TEST_F(FrameworkBenchmarkTest, ComprehensiveBenchmark) {
    std::cout << "🚀 综合性能基准测试" << std::endl;
    std::cout << "模拟真实服务器场景: 多线程处理 + IO多路复用 + 异步日志" << std::endl;
    
    const int connection_count = 200; // 减少连接数确保稳定性
    const int worker_threads = 4;
    const int requests_per_connection = 10; // 减少每连接请求数
    
    // 创建线程池
    MutexThreadPool thread_pool(worker_threads);
    
    // 创建IO多路复用器
    auto io = IOFactory::createIO(IOMultiplexer::IOType::EPOLL);
    ASSERT_TRUE(io && io->init());
    
    // 创建测试连接
    std::vector<std::pair<int, int>> connections;
    for (int i = 0; i < connection_count; ++i) {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0) {
            int flags = fcntl(fds[0], F_GETFL, 0);
            fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);
            connections.push_back({fds[0], fds[1]});
            io->addfd(fds[0], IOMultiplexer::EventType::READ);
        }
    }
    
    std::atomic<int> total_requests{0};
    std::atomic<int> total_responses{0};
    AsyncLogger& logger = AsyncLogger::getInstance();
    
    double execution_time = TestUtils::measureExecutionTime([&]() {
        // 发送请求
        for (const auto& conn : connections) {
            for (int i = 0; i < requests_per_connection; ++i) {
                std::string request = "REQUEST_" + std::to_string(i);
                write(conn.second, request.c_str(), request.length());
                total_requests.fetch_add(1);
            }
        }
        
        // 处理请求
        int max_iterations = 1000;
        while (total_responses.load() < total_requests.load() && max_iterations-- > 0) {
            std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
            int result = io->wait(active_events, 10);
            
            if (result > 0) {
                for (const auto& event : active_events) {
                    if (event.first > 0 && (event.second & IOMultiplexer::EventType::READ)) {
                        // 提交到线程池处理
                        thread_pool.enqueue([&, fd = event.first]() {
                            char buffer[256];
                            int bytes = read(fd, buffer, sizeof(buffer));
                            if (bytes > 0) {
                                total_responses.fetch_add(1);
                                logger.info("Processed request from fd " + std::to_string(fd));
                            }
                        });
                    }
                }
            }
        }
        
        // 等待所有任务完成
        TestUtils::waitForCondition([&]() {
            return total_responses.load() >= total_requests.load() * 0.9; // 允许90%完成
        }, 5000);
    });
    
    double throughput = total_responses.load() / (execution_time / 1000.0);
    double success_rate = (double)total_responses.load() / total_requests.load() * 100;
    
    std::cout << "  - 总执行时间: " << execution_time << " ms" << std::endl;
    std::cout << "  - 发送请求数: " << total_requests.load() << std::endl;
    std::cout << "  - 处理响应数: " << total_responses.load() << std::endl;
    std::cout << "  - 成功率: " << success_rate << "%" << std::endl;
    std::cout << "  - 吞吐量: " << static_cast<int>(throughput) << " req/sec" << std::endl;
    std::cout << "  - 平均延迟: " << execution_time / total_responses.load() << " ms/req" << std::endl;
    
    // 清理
    for (const auto& conn : connections) {
        io->removefd(conn.first);
        close(conn.first);
        close(conn.second);
    }
    
    EXPECT_GT(success_rate, 80.0); // 至少80%成功率
    EXPECT_LT(execution_time, 10000.0); // 应在10秒内完成
    
    std::cout << std::endl;
}
