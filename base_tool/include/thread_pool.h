#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <queue>
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <atomic>

class thread_pool
{
private:
    std::queue<std::function<void()>> tasks;
    std::vector<std::thread> workers;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop;

public:
    thread_pool(size_t threads = std::thread::hardware_concurrency());
    ~thread_pool();
    void run_task();
    template<typename T>
auto queue(T&& task) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        tasks.emplace(std::forward<T>(task));
    }
    cv.notify_one();
}

};
#endif // THREADPOOL_H