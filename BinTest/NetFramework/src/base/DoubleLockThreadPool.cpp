#include "base/DoubleLockThreadPool.h"
#include "base/Logger.h"
#include <algorithm>

DoubleLockThreadPool::DoubleLockThreadPool(size_t thread_count, size_t max_queue_size) : stop(false), max_queue_size(max_queue_size) {
    workers.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers.emplace_back(&DoubleLockThreadPool::worker, this);
    }
}

DoubleLockThreadPool::~DoubleLockThreadPool() {
    stop = true;
    condition.notify_all();
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

bool DoubleLockThreadPool::enqueue(const std::function<void()>& task) {
    if (stop) {
        Logger::warn("线程池已停止，无法投递任务");
        return false;
    }
    // 仅锁定队尾添加任务
    std::lock_guard<std::mutex> lock(back_mutex);
    if (tasks.size() >= max_queue_size) {
        Logger::warn("任务队列已满，当前大小: " + std::to_string(tasks.size()) + ", 最大容量: " + std::to_string(max_queue_size));
        return false;
    }
    tasks.emplace_back(task);
    condition.notify_one();
    return true;
}

void DoubleLockThreadPool::worker() {
    while (!stop) {
        std::function<void()> task;
        // 仅锁定队头获取任务
        std::unique_lock<std::mutex> lock(front_mutex);
        condition.wait(lock, [this] { 
            return stop || !tasks.empty(); 
        });
        if (stop && tasks.empty()) {
            return;
        }
        if (!tasks.empty()) {
            task = std::move(tasks.front());
            tasks.pop_front();
            lock.unlock();
            try {
                task();
            } catch (const std::exception& e) {
                Logger::error("任务执行异常: " + std::string(e.what()));
            }
        }
    }
}