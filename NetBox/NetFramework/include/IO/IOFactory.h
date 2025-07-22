#pragma once

#include "base/IOMultiplexer.h"
#include "EpollMultiplexer.h"
#include <memory>
#include <iostream>
class IOFactory
{
private:
public:
    
    static std::unique_ptr<IOMultiplexer> createIO(IOMultiplexer::IOType type) {
        switch (type)
        {
        case IOMultiplexer::IOType::EPOLL : return std::make_unique<EpollMultiplexer>();
        default: return nullptr;
        }
    }
    struct PerformanceStats {
    int64_t total_requests = 0;  // 总请求数
    int64_t total_time_us = 0;   // 总耗时（微秒）
    int max_concurrent = 0;      // 最大并发数
    // ... 其他指标：平均延迟、QPS等

    void update(int64_t duration_us, int current_concurrent) {
        total_requests++;
        total_time_us += duration_us;
        if (current_concurrent > max_concurrent) {
            max_concurrent = current_concurrent;
        }
    }
    void print() {
        double qps = total_requests * 1.0 / (total_time_us / 1000000.0);
        printf("总请求: %ld, 总耗时: %ldus, QPS: %.2f, 最大并发: %d\n",
               total_requests, total_time_us, qps, max_concurrent);
    }
};
};
