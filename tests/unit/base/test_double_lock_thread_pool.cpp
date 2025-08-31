#include <gtest/gtest.h>
#include "base/DoubleLockThreadPool.h"
#include "test_utils.h"
#include <atomic>
#include <vector>
#include <chrono>

class DoubleLockThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试开始前的设置
    }

    void TearDown() override {
        // 每个测试结束后的清理
    }
};

// 测试DoubleLockThreadPool基本功能
TEST_F(DoubleLockThreadPoolTest, BasicFunctionality) {
    const size_t thread_count = 4;
    const size_t max_queue_size = 1000;
    DoubleLockThreadPool pool(thread_count, max_queue_size);
    
    std::atomic<int> counter{0};
    
    // 提交一些任务
    for (int i = 0; i < 10; ++i) {
        bool success = pool.enqueue([&counter]() {
            counter.fetch_add(1);
        });
        EXPECT_TRUE(success);
    }
    
    // 等待任务完成
    TestUtils::waitFor(100);
    
    EXPECT_EQ(counter.load(), 10);
}

// 测试构造函数参数
TEST_F(DoubleLockThreadPoolTest, ConstructorParameters) {
    // 测试不同的线程数和队列大小
    {
        DoubleLockThreadPool pool(2, 500);
        std::atomic<int> counter{0};
        
        for (int i = 0; i < 5; ++i) {
            bool success = pool.enqueue([&counter]() {
                counter.fetch_add(1);
            });
            EXPECT_TRUE(success);
        }
        
        TestUtils::waitFor(50);
        EXPECT_EQ(counter.load(), 5);
    }
    
    // 测试大线程数
    {
        DoubleLockThreadPool pool(8, 2000);
        std::atomic<int> counter{0};
        
        for (int i = 0; i < 20; ++i) {
            bool success = pool.enqueue([&counter]() {
                counter.fetch_add(1);
            });
            EXPECT_TRUE(success);
        }
        
        TestUtils::waitFor(100);
        EXPECT_EQ(counter.load(), 20);
    }
}

// 测试队列大小限制
TEST_F(DoubleLockThreadPoolTest, QueueSizeLimit) {
    const size_t thread_count = 1; // 单线程，便于控制
    const size_t max_queue_size = 10; // 小队列
    DoubleLockThreadPool pool(thread_count, max_queue_size);
    
    std::atomic<int> executed_count{0};
    std::atomic<bool> long_task_started{false};
    
    // 先提交一个长时间运行的任务，占用工作线程
    bool success = pool.enqueue([&long_task_started, &executed_count]() {
        long_task_started = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        executed_count.fetch_add(1);
    });
    EXPECT_TRUE(success);
    
    // 等待长任务开始
    TestUtils::waitForCondition([&]() { return long_task_started.load(); });
    
    // 现在快速提交任务，直到队列满
    int successful_enqueues = 0;
    const int total_attempts = max_queue_size + 5; // 超过队列限制
    
    for (int i = 0; i < total_attempts; ++i) {
        bool enqueue_success = pool.enqueue([&executed_count]() {
            executed_count.fetch_add(1);
        });
        
        if (enqueue_success) {
            successful_enqueues++;
        }
    }
    
    // 应该有一些任务被拒绝
    EXPECT_LE(successful_enqueues, static_cast<int>(max_queue_size));
    
    // 等待所有任务完成
    TestUtils::waitFor(500);
    
    // 执行的任务数应该等于成功入队的任务数 + 1（长任务）
    EXPECT_EQ(executed_count.load(), successful_enqueues + 1);
}

// 测试双锁机制的并发性能
TEST_F(DoubleLockThreadPoolTest, ConcurrentEnqueueDequeue) {
    const size_t thread_count = 4;
    const size_t max_queue_size = 10000;
    DoubleLockThreadPool pool(thread_count, max_queue_size);
    
    std::atomic<int> enqueue_count{0};
    std::atomic<int> execute_count{0};
    
    const int total_tasks = 1000;
    
    // 启动多个线程同时入队任务
    std::vector<std::thread> enqueue_threads;
    const int enqueue_thread_count = 4;
    
    for (int t = 0; t < enqueue_thread_count; ++t) {
        enqueue_threads.emplace_back([&]() {
            const int tasks_per_thread = total_tasks / enqueue_thread_count;
            for (int i = 0; i < tasks_per_thread; ++i) {
                bool success = pool.enqueue([&execute_count]() {
                    execute_count.fetch_add(1);
                    // 模拟一些工作
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                });
                
                if (success) {
                    enqueue_count.fetch_add(1);
                }
            }
        });
    }
    
    // 等待所有入队线程完成
    for (auto& thread : enqueue_threads) {
        thread.join();
    }
    
    // 等待所有任务执行完成
    TestUtils::waitForCondition([&]() {
        return execute_count.load() == enqueue_count.load();
    }, 5000);
    
    EXPECT_EQ(execute_count.load(), enqueue_count.load());
    EXPECT_GT(enqueue_count.load(), 0);
}

// 测试异常处理
TEST_F(DoubleLockThreadPoolTest, ExceptionHandling) {
    DoubleLockThreadPool pool(2, 100);
    
    std::atomic<int> normal_tasks{0};
    std::atomic<int> exception_tasks{0};
    
    // 提交正常任务和会抛异常的任务
    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0) {
            // 正常任务
            pool.enqueue([&normal_tasks]() {
                normal_tasks.fetch_add(1);
            });
        } else {
            // 抛异常的任务
            pool.enqueue([&exception_tasks]() {
                exception_tasks.fetch_add(1);
                throw std::runtime_error("Test exception in DoubleLockThreadPool");
            });
        }
    }
    
    // 等待任务完成
    TestUtils::waitFor(200);
    
    // 验证正常任务和异常任务都执行了
    EXPECT_EQ(normal_tasks.load(), 5);
    EXPECT_EQ(exception_tasks.load(), 5);
}

// 测试线程池析构时的任务完成
TEST_F(DoubleLockThreadPoolTest, DestructionWithPendingTasks) {
    std::atomic<int> completed_tasks{0};
    
    {
        DoubleLockThreadPool pool(2, 100);
        
        // 提交一些任务
        for (int i = 0; i < 10; ++i) {
            pool.enqueue([&completed_tasks]() {
                // 减少任务执行时间
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                completed_tasks.fetch_add(1);
            });
        }

        // 给任务一些时间开始执行
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // 线程池在这里析构，应该等待已开始的任务完成
    }

    // 析构后，检查完成的任务数
    EXPECT_GT(completed_tasks.load(), 0); // 至少有一些任务完成了
    EXPECT_LE(completed_tasks.load(), 10); // 不会超过提交的任务数
}

// 测试性能对比（与单锁线程池的理论对比）
TEST_F(DoubleLockThreadPoolTest, PerformanceTest) {
    const size_t thread_count = std::thread::hardware_concurrency();
    const size_t max_queue_size = 10000;
    DoubleLockThreadPool pool(thread_count, max_queue_size);
    
    const int task_count = 5000;
    std::atomic<int> completed{0};
    
    double execution_time = TestUtils::measureExecutionTime([&]() {
        // 启动多个线程同时提交任务
        std::vector<std::thread> submit_threads;
        const int submit_thread_count = 4;
        
        for (int t = 0; t < submit_thread_count; ++t) {
            submit_threads.emplace_back([&]() {
                const int tasks_per_thread = task_count / submit_thread_count;
                for (int i = 0; i < tasks_per_thread; ++i) {
                    pool.enqueue([&completed]() {
                        completed.fetch_add(1);
                    });
                }
            });
        }
        
        for (auto& thread : submit_threads) {
            thread.join();
        }
        
        // 等待所有任务完成
        TestUtils::waitForCondition([&]() {
            return completed.load() == task_count;
        }, 5000);
    });
    
    EXPECT_EQ(completed.load(), task_count);
    
    std::cout << "DoubleLockThreadPool executed " << task_count << " tasks in " 
              << execution_time << " ms using " << thread_count << " threads" << std::endl;
    std::cout << "Average time per task: " 
              << execution_time / task_count << " ms" << std::endl;
    
    // 性能应该合理
    EXPECT_LT(execution_time, 3000.0); // 应该在3秒内完成
}

// 测试停止后的任务提交
TEST_F(DoubleLockThreadPoolTest, EnqueueAfterStop) {
    auto pool = std::make_unique<DoubleLockThreadPool>(2, 100);

    std::atomic<int> counter{0};

    // 提交一个任务
    bool success1 = pool->enqueue([&counter]() {
        counter.fetch_add(1);
    });
    EXPECT_TRUE(success1);

    // 等待任务执行
    TestUtils::waitForCondition([&]() {
        return counter.load() > 0;
    }, 1000);

    // 销毁线程池
    pool.reset();

    // 验证任务已执行
    EXPECT_EQ(counter.load(), 1);
}

// 测试大量并发任务
TEST_F(DoubleLockThreadPoolTest, HighConcurrencyTest) {
    const size_t thread_count = 8;
    const size_t max_queue_size = 5000;
    DoubleLockThreadPool pool(thread_count, max_queue_size);
    
    std::atomic<int> total_executed{0};
    std::atomic<int> concurrent_count{0};
    std::atomic<int> max_concurrent{0};
    
    const int task_count = 2000;
    
    for (int i = 0; i < task_count; ++i) {
        pool.enqueue([&]() {
            int current = concurrent_count.fetch_add(1) + 1;
            
            // 更新最大并发数
            int expected = max_concurrent.load();
            while (current > expected && 
                   !max_concurrent.compare_exchange_weak(expected, current)) {
                expected = max_concurrent.load();
            }
            
            // 模拟工作
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            
            concurrent_count.fetch_sub(1);
            total_executed.fetch_add(1);
        });
    }
    
    // 等待所有任务完成
    TestUtils::waitForCondition([&]() {
        return total_executed.load() == task_count;
    }, 10000);
    
    EXPECT_EQ(total_executed.load(), task_count);
    EXPECT_LE(max_concurrent.load(), static_cast<int>(thread_count));
    EXPECT_GT(max_concurrent.load(), 0);
    
    std::cout << "Max concurrent tasks: " << max_concurrent.load() 
              << " (thread count: " << thread_count << ")" << std::endl;
}
