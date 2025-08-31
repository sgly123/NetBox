#pragma once
#include "IThreadPool.h"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <chrono>

/**
 * @brief 心跳线程池，定时执行心跳检测等任务
 */
class HeartbeatThreadPool : public IThreadPool {
public:
    /**
     * @brief 构造函数，创建指定数量的线程
     * @param thread_count 线程数量，默认1
     * @param interval_ms  心跳检测间隔（毫秒）
     */
    explicit HeartbeatThreadPool(size_t thread_count = 1, int interval_ms = 10000);
    ~HeartbeatThreadPool();

    /**
     * @brief 注册定时任务（如心跳检测）
     * @param task 任务函数
     */
    void registerTask(const std::function<void()>& task);

    /**
     * @brief 投递一次性任务（接口兼容）
     */
    bool enqueue(const std::function<void()>& task) override;

private:
    void worker();
    std::vector<std::thread> workers;
    std::vector<std::function<void()>> periodic_tasks;
    std::mutex task_mutex;
    std::atomic<bool> stop;
    int interval_ms;
}; 