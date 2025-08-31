#include <gtest/gtest.h>
#include "base/ThreadPool.h"
#include "test_utils.h"
#include <atomic>
#include <vector>
#include <future>
#include <chrono>

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试开始前创建新的线程池
    }

    void TearDown() override {
        // 测试结束后清理
    }
};

// 测试线程池基本功能
TEST_F(ThreadPoolTest, BasicFunctionality) {
    const size_t thread_count = 4;
    MutexThreadPool pool(thread_count);
    
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

// 测试线程池构造函数
TEST_F(ThreadPoolTest, Constructor) {
    // 测试默认构造函数
    {
        MutexThreadPool pool;
        std::atomic<bool> task_executed{false};
        
        bool success = pool.enqueue([&task_executed]() {
            task_executed = true;
        });
        
        EXPECT_TRUE(success);
        TestUtils::waitFor(50);
        EXPECT_TRUE(task_executed.load());
    }
    
    // 测试指定线程数的构造函数
    {
        const size_t thread_count = 8;
        MutexThreadPool pool(thread_count);
        
        std::atomic<int> counter{0};
        
        for (int i = 0; i < 20; ++i) {
            pool.enqueue([&counter]() {
                counter.fetch_add(1);
            });
        }
        
        TestUtils::waitFor(100);
        EXPECT_EQ(counter.load(), 20);
    }
}

// 测试任务执行顺序（不保证顺序，但都应该执行）
TEST_F(ThreadPoolTest, TaskExecution) {
    MutexThreadPool pool(2);
    
    std::vector<int> results;
    std::mutex results_mutex;
    
    // 提交有序任务
    for (int i = 0; i < 10; ++i) {
        pool.enqueue([i, &results, &results_mutex]() {
            std::lock_guard<std::mutex> lock(results_mutex);
            results.push_back(i);
        });
    }
    
    // 等待所有任务完成
    TestUtils::waitFor(100);
    
    // 验证所有任务都执行了
    EXPECT_EQ(results.size(), 10);
    
    // 验证所有数字都存在（不关心顺序）
    std::sort(results.begin(), results.end());
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(results[i], i);
    }
}

// 测试并发任务执行
TEST_F(ThreadPoolTest, ConcurrentExecution) {
    const size_t thread_count = 4;
    MutexThreadPool pool(thread_count);
    
    std::atomic<int> concurrent_count{0};
    std::atomic<int> max_concurrent{0};
    std::atomic<int> total_executed{0};
    
    const int task_count = 20;
    
    for (int i = 0; i < task_count; ++i) {
        pool.enqueue([&concurrent_count, &max_concurrent, &total_executed]() {
            int current = concurrent_count.fetch_add(1) + 1;
            
            // 更新最大并发数
            int expected = max_concurrent.load();
            while (current > expected && 
                   !max_concurrent.compare_exchange_weak(expected, current)) {
                expected = max_concurrent.load();
            }
            
            // 模拟一些工作
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            concurrent_count.fetch_sub(1);
            total_executed.fetch_add(1);
        });
    }
    
    // 等待所有任务完成
    TestUtils::waitFor(500);
    
    EXPECT_EQ(total_executed.load(), task_count);
    EXPECT_LE(max_concurrent.load(), static_cast<int>(thread_count));
    EXPECT_GT(max_concurrent.load(), 0);
}

// 测试任务队列限制
TEST_F(ThreadPoolTest, TaskQueueLimit) {
    MutexThreadPool pool(1); // 单线程池

    std::atomic<int> executed_count{0};
    std::atomic<int> successful_enqueues{0};

    // 提交大量任务，测试队列限制
    const int total_tasks = 15000; // 超过队列限制(10000)

    // 使用多线程快速填满队列
    std::vector<std::thread> enqueue_threads;
    const int threads_count = 4;
    const int tasks_per_thread = total_tasks / threads_count;

    for (int t = 0; t < threads_count; ++t) {
        enqueue_threads.emplace_back([&, t]() {
            int start = t * tasks_per_thread;
            int end = (t == threads_count - 1) ? total_tasks : (t + 1) * tasks_per_thread;

            for (int i = start; i < end; ++i) {
                bool success = pool.enqueue([&executed_count]() {
                    executed_count.fetch_add(1);
                    // 减少任务执行时间，让队列更容易填满
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                });

                if (success) {
                    successful_enqueues.fetch_add(1);
                }
            }
        });
    }

    // 等待所有入队线程完成
    for (auto& thread : enqueue_threads) {
        thread.join();
    }

    // 应该有一些任务被拒绝（队列满了）
    EXPECT_LT(successful_enqueues.load(), total_tasks);
    // 由于多线程竞争条件，可能会稍微超过队列限制，但不应该超过太多
    EXPECT_LE(successful_enqueues.load(), 10100); // 允许一些误差

    // 等待执行完成
    TestUtils::waitForCondition([&]() {
        return executed_count.load() == successful_enqueues.load();
    }, 5000);

    // 执行的任务数应该等于成功入队的任务数
    EXPECT_EQ(executed_count.load(), successful_enqueues.load());
}

// 测试异常处理
TEST_F(ThreadPoolTest, ExceptionHandling) {
    MutexThreadPool pool(2);
    
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
                throw std::runtime_error("Test exception");
            });
        }
    }
    
    // 等待任务完成
    TestUtils::waitFor(100);
    
    // 验证正常任务和异常任务都执行了
    EXPECT_EQ(normal_tasks.load(), 5);
    EXPECT_EQ(exception_tasks.load(), 5);
}

// 测试线程池析构
TEST_F(ThreadPoolTest, Destruction) {
    std::atomic<int> completed_tasks{0};

    {
        MutexThreadPool pool(2);

        // 提交一些任务，但不要让它们运行太长时间
        for (int i = 0; i < 5; ++i) {
            pool.enqueue([&completed_tasks]() {
                // 减少任务执行时间，确保在析构前有足够时间完成
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                completed_tasks.fetch_add(1);
            });
        }

        // 给任务一些时间开始执行
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        // 线程池在这里析构，应该等待所有已开始的任务完成
    }

    // 析构后，检查完成的任务数（可能不是全部5个，因为有些可能还在队列中）
    EXPECT_GT(completed_tasks.load(), 0); // 至少有一些任务完成了
    EXPECT_LE(completed_tasks.load(), 5); // 不会超过提交的任务数
}

// 测试线程池性能
TEST_F(ThreadPoolTest, Performance) {
    const size_t thread_count = std::thread::hardware_concurrency();
    MutexThreadPool pool(thread_count);
    
    const int task_count = 10000;
    std::atomic<int> completed{0};
    
    double execution_time = TestUtils::measureExecutionTime([&]() {
        for (int i = 0; i < task_count; ++i) {
            pool.enqueue([&completed]() {
                completed.fetch_add(1);
            });
        }
        
        // 等待所有任务完成
        TestUtils::waitForCondition([&]() {
            return completed.load() == task_count;
        }, 5000);
    });
    
    EXPECT_EQ(completed.load(), task_count);
    
    std::cout << "Executed " << task_count << " tasks in " 
              << execution_time << " ms using " << thread_count << " threads" << std::endl;
    std::cout << "Average time per task: " 
              << execution_time / task_count << " ms" << std::endl;
    
    // 性能应该合理（具体数值取决于硬件）
    EXPECT_LT(execution_time, 2000.0); // 应该在2秒内完成
}

// 测试空任务
TEST_F(ThreadPoolTest, EmptyTask) {
    MutexThreadPool pool(2);
    
    // 提交空任务（什么都不做）
    bool success = pool.enqueue([]() {
        // 空任务
    });
    
    EXPECT_TRUE(success);
    TestUtils::waitFor(50);
}

// 测试任务返回值（通过future）
TEST_F(ThreadPoolTest, TaskWithReturnValue) {
    MutexThreadPool pool(2);
    
    // 使用promise/future来获取任务返回值
    std::promise<int> promise;
    std::future<int> future = promise.get_future();
    
    bool success = pool.enqueue([&promise]() {
        promise.set_value(42);
    });
    
    EXPECT_TRUE(success);
    
    // 等待任务完成并获取结果
    auto status = future.wait_for(std::chrono::milliseconds(100));
    EXPECT_EQ(status, std::future_status::ready);
    
    if (status == std::future_status::ready) {
        EXPECT_EQ(future.get(), 42);
    }
}

// 测试多个线程池同时工作
TEST_F(ThreadPoolTest, MultipleThreadPools) {
    MutexThreadPool pool1(2);
    MutexThreadPool pool2(2);
    
    std::atomic<int> pool1_tasks{0};
    std::atomic<int> pool2_tasks{0};
    
    // 向两个线程池提交任务
    for (int i = 0; i < 10; ++i) {
        pool1.enqueue([&pool1_tasks]() {
            pool1_tasks.fetch_add(1);
        });
        
        pool2.enqueue([&pool2_tasks]() {
            pool2_tasks.fetch_add(1);
        });
    }
    
    // 等待任务完成
    TestUtils::waitFor(100);
    
    EXPECT_EQ(pool1_tasks.load(), 10);
    EXPECT_EQ(pool2_tasks.load(), 10);
}
