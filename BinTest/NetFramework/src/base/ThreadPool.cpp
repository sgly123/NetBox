#include "base/ThreadPool.h"
#include "base/HeartbeatThreadPool.h"

/**
 * @brief 构造函数，创建指定数量的工作线程
 */
MutexThreadPool::MutexThreadPool(size_t thread_count) : stop(false) {
    for (size_t i = 0; i < thread_count; ++i) {
        // 每个线程都运行worker()主循环
        workers.emplace_back([this] { this->worker(); });
    }
}

/**
 * @brief 工作线程主函数，不断从队列取任务执行
 */
void MutexThreadPool::worker() {
    while (!stop) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            // 等待任务到来或线程池停止
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) return; // 退出条件
            task = std::move(tasks.front());
            tasks.pop();
        }
        // 执行任务，添加异常处理
        try {
            task();
        } catch (const std::exception& e) {
            // 记录异常但不中断线程
            // Logger::error("线程池任务执行异常: " + std::string(e.what()));
        } catch (...) {
            // Logger::error("线程池任务执行未知异常");
        }
    }
}

/**
 * @brief 投递任务到线程池
 */
bool MutexThreadPool::enqueue(const std::function<void()>& task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        // 限制队列大小，避免内存溢出
        if (tasks.size() >= 10000) {
            // Logger::error("线程池任务队列已满，丢弃任务");
            return false;
        }
        tasks.push(task);
    }
    condition.notify_one(); // 唤醒一个等待线程
    return true;
}

/**
 * @brief 析构函数，安全停止所有线程
 */
MutexThreadPool::~MutexThreadPool() {
    stop = true;
    condition.notify_all(); // 唤醒所有线程准备退出
    for (auto& t : workers) {
        if (t.joinable()) t.join(); // 等待线程结束
    }
}

HeartbeatThreadPool::HeartbeatThreadPool(size_t thread_count, int interval_ms)
    : stop(false), interval_ms(interval_ms) {
    for (size_t i = 0; i < thread_count; ++i) {
        workers.emplace_back([this] { this->worker(); });
    }
}

HeartbeatThreadPool::~HeartbeatThreadPool() {
    stop = true;
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }
}

void HeartbeatThreadPool::registerTask(const std::function<void()>& task) {
    std::lock_guard<std::mutex> lock(task_mutex);
    periodic_tasks.push_back(task);
}

bool HeartbeatThreadPool::enqueue(const std::function<void()>& task) {
    // 兼容接口：立即执行一次
    std::thread(task).detach();
    return true;
}

void HeartbeatThreadPool::worker() {
    while (!stop) {
        {
            std::lock_guard<std::mutex> lock(task_mutex);
            for (auto& task : periodic_tasks) {
                try {
                    task();
                } catch (...) {
                    // 忽略心跳任务异常
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
} 