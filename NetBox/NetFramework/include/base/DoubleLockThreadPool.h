#pragma once
#include "IThreadPool.h"
#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>

/**
 * @brief 双锁线程池实现
 * 使用前后端双锁分离入队和出队操作，减少锁竞争
 */
class DoubleLockThreadPool : public IThreadPool {
public:
    explicit DoubleLockThreadPool(size_t thread_count, size_t max_queue_size = 10000);
    ~DoubleLockThreadPool() override;
    bool enqueue(const std::function<void()>& task) override;

private:
    void worker();
    std::vector<std::thread> workers;       // 工作线程集合
    std::deque<std::function<void()>> tasks; // 任务队列
    std::mutex front_mutex;                 // 队头锁(出队操作)
    std::mutex back_mutex;                  // 队尾锁(入队操作)
    std::condition_variable condition;      // 任务通知条件变量
    std::atomic<bool> stop;                 // 停止标志(原子变量)
    size_t max_queue_size;                  // 任务队列最大容量
};