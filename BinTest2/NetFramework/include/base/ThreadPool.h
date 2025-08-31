#pragma once
#include "IThreadPool.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
 * @brief 单锁线程池实现，继承自IThreadPool，适合大多数并发场景
 */
class MutexThreadPool : public IThreadPool {
public:
    /**
     * @brief 构造函数，创建指定数量的工作线程
     * @param thread_count 线程数量，默认等于CPU核心数
     */
    explicit MutexThreadPool(size_t thread_count = std::thread::hardware_concurrency());

    /**
     * @brief 析构函数，自动回收所有线程
     */
    ~MutexThreadPool();

    /**
     * @brief 投递任务到线程池，任务为任意可调用对象
     * @param task 任务函数（如lambda、std::bind等）
     * @return 是否成功投递任务
     */
    bool enqueue(const std::function<void()>& task) override;

private:
    std::vector<std::thread> workers;                // 工作线程集合
    std::queue<std::function<void()>> tasks;         // 任务队列
    std::mutex queue_mutex;                          // 任务队列互斥锁
    std::condition_variable condition;               // 条件变量，用于线程等待/唤醒
    std::atomic<bool> stop;                          // 停止标志

    /**
     * @brief 工作线程主函数，不断从队列取任务执行
     */
    void worker();
}; 