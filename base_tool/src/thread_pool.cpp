#include "../include/thread_pool.h"

thread_pool::thread_pool(size_t threads):stop(false) {
    if (threads == 0) threads = 1;
    workers.reserve(threads);
    for (size_t i = 0; i < threads; i++)
    {
        workers.emplace_back([this] {
            run_task();
        });
    }
}

void thread_pool::run_task() {
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] {
                return stop || !tasks.empty();
            });
            if (stop && tasks.empty())
            {
                return;
            }
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

thread_pool::~thread_pool() {
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();
    for(auto &work:workers) {
        if (work.joinable())
        {
            work.join();
        }
    }
}