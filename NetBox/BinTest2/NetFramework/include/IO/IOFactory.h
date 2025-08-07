#pragma once

#include "base/IOMultiplexer.h"
#include "platform/Platform.h"
#include <memory>
#include <iostream>
#include "base/Logger.h"

// 根据平台包含相应的IO多路复用器头文件
#ifdef NETBOX_PLATFORM_WINDOWS
    #include "IO/IOCPMultiplexer.h"
    #include "IO/SelectMultiplexer.h"  // Windows也支持select
#elif defined(NETBOX_PLATFORM_LINUX)
    #include "IO/EpollMultiplexer.h"
    #include "IO/PollMultiplexer.h"
    #include "IO/SelectMultiplexer.h"
#elif defined(NETBOX_PLATFORM_MACOS) || defined(NETBOX_PLATFORM_FREEBSD) || defined(NETBOX_PLATFORM_OPENBSD)
    #include "IO/KqueueMultiplexer.h"
    #include "IO/SelectMultiplexer.h"  // BSD也支持select
#else
    #include "IO/SelectMultiplexer.h"  // 默认使用select作为后备方案
#endif
class IOFactory
{
private:
public:
    
    /**
     * @brief 创建IO多路复用器实例
     * @param type IO多路复用器类型
     * @return std::unique_ptr<IOMultiplexer> IO多路复用器智能指针
     *
     * 跨平台支持说明：
     * - Windows: 支持IOCP(推荐)和SELECT
     * - Linux: 支持EPOLL(推荐)、POLL和SELECT
     * - macOS/BSD: 支持KQUEUE(推荐)和SELECT
     * - 其他平台: 支持SELECT作为通用后备方案
     *
     * 性能建议：
     * - 高并发服务器: Windows用IOCP，Linux用EPOLL，macOS用KQUEUE
     * - 中等并发: 可以使用POLL(Linux)或SELECT(跨平台)
     * - 简单应用: SELECT提供最好的兼容性
     */
    static std::unique_ptr<IOMultiplexer> createIO(IOMultiplexer::IOType type) {
        switch (type) {
#ifdef NETBOX_PLATFORM_WINDOWS
            case IOMultiplexer::IOType::IOCP:
                return std::make_unique<NetBox::IO::IOCPMultiplexer>();
            case IOMultiplexer::IOType::SELECT:
                return std::make_unique<SelectMultiplexer>();
            case IOMultiplexer::IOType::EPOLL:
            case IOMultiplexer::IOType::POLL:
            case IOMultiplexer::IOType::KQUEUE:
                Logger::warn("IO类型 " + std::to_string(static_cast<int>(type)) + " 在Windows平台不支持，回退到SELECT");
                return std::make_unique<SelectMultiplexer>();

#elif defined(NETBOX_PLATFORM_LINUX)
            case IOMultiplexer::IOType::EPOLL:
                return std::make_unique<EpollMultiplexer>();
            case IOMultiplexer::IOType::POLL:
                return std::make_unique<PollMultiplexer>();
            case IOMultiplexer::IOType::SELECT:
                return std::make_unique<SelectMultiplexer>();
            case IOMultiplexer::IOType::IOCP:
            case IOMultiplexer::IOType::KQUEUE:
                Logger::warn("IO类型 " + std::to_string(static_cast<int>(type)) + " 在Linux平台不支持，回退到EPOLL");
                return std::make_unique<EpollMultiplexer>();

#elif defined(NETBOX_PLATFORM_MACOS) || defined(NETBOX_PLATFORM_FREEBSD) || defined(NETBOX_PLATFORM_OPENBSD)
            case IOMultiplexer::IOType::KQUEUE:
                return std::make_unique<NetBox::IO::KqueueMultiplexer>();
            case IOMultiplexer::IOType::SELECT:
                return std::make_unique<SelectMultiplexer>();
            case IOMultiplexer::IOType::EPOLL:
            case IOMultiplexer::IOType::POLL:
            case IOMultiplexer::IOType::IOCP:
                Logger::warn("IO类型 " + std::to_string(static_cast<int>(type)) + " 在macOS/BSD平台不支持，回退到KQUEUE");
                return std::make_unique<NetBox::IO::KqueueMultiplexer>();

#else
            // 未知平台，只支持SELECT
            case IOMultiplexer::IOType::SELECT:
                return std::make_unique<SelectMultiplexer>();
            default:
                Logger::warn("未知平台，只支持SELECT IO多路复用");
                return std::make_unique<SelectMultiplexer>();
#endif
            default:
                Logger::error("不支持的IO多路复用类型: " + std::to_string(static_cast<int>(type)));
                return nullptr;
        }
    }

    /**
     * @brief 获取当前平台推荐的IO多路复用器类型
     * @return IOMultiplexer::IOType 推荐的IO类型
     *
     * 根据平台特性返回性能最优的IO多路复用器：
     * - Windows: IOCP (I/O Completion Port)
     * - Linux: EPOLL (高性能事件通知)
     * - macOS/BSD: KQUEUE (统一事件接口)
     * - 其他: SELECT (通用兼容)
     */
    static IOMultiplexer::IOType getRecommendedIOType() {
#ifdef NETBOX_PLATFORM_WINDOWS
        return IOMultiplexer::IOType::IOCP;
#elif defined(NETBOX_PLATFORM_LINUX)
        return IOMultiplexer::IOType::EPOLL;
#elif defined(NETBOX_PLATFORM_MACOS) || defined(NETBOX_PLATFORM_FREEBSD) || defined(NETBOX_PLATFORM_OPENBSD)
        return IOMultiplexer::IOType::KQUEUE;
#else
        return IOMultiplexer::IOType::SELECT;
#endif
    }

    /**
     * @brief 获取当前平台支持的IO多路复用器类型列表
     * @return std::vector<IOMultiplexer::IOType> 支持的IO类型列表
     */
    static std::vector<IOMultiplexer::IOType> getSupportedIOTypes() {
        std::vector<IOMultiplexer::IOType> types;

#ifdef NETBOX_PLATFORM_WINDOWS
        types.push_back(IOMultiplexer::IOType::IOCP);
        types.push_back(IOMultiplexer::IOType::SELECT);
#elif defined(NETBOX_PLATFORM_LINUX)
        types.push_back(IOMultiplexer::IOType::EPOLL);
        types.push_back(IOMultiplexer::IOType::POLL);
        types.push_back(IOMultiplexer::IOType::SELECT);
#elif defined(NETBOX_PLATFORM_MACOS) || defined(NETBOX_PLATFORM_FREEBSD) || defined(NETBOX_PLATFORM_OPENBSD)
        types.push_back(IOMultiplexer::IOType::KQUEUE);
        types.push_back(IOMultiplexer::IOType::SELECT);
#else
        types.push_back(IOMultiplexer::IOType::SELECT);
#endif

        return types;
    }

    /**
     * @brief 获取IO多路复用器类型的名称
     * @param type IO类型
     * @return std::string 类型名称
     */
    static std::string getIOTypeName(IOMultiplexer::IOType type) {
        switch (type) {
            case IOMultiplexer::IOType::SELECT: return "SELECT";
            case IOMultiplexer::IOType::POLL: return "POLL";
            case IOMultiplexer::IOType::EPOLL: return "EPOLL";
            case IOMultiplexer::IOType::KQUEUE: return "KQUEUE";
            case IOMultiplexer::IOType::IOCP: return "IOCP";
            default: return "UNKNOWN";
        }
    }
    struct PerformanceStats {
    int64_t total_requests = 0;  // 总请求数
    int64_t total_time_us = 0;   // 总耗时（微秒）
    int max_concurrent = 0;      // 最大并发数

    void update(int64_t duration_us, int current_concurrent) {
        total_requests++;
        total_time_us += duration_us;
        if (current_concurrent > max_concurrent) {
            max_concurrent = current_concurrent;
        }
    }
    void print() {
        double qps = total_requests * 1.0 / (total_time_us / 1000000.0);
        Logger::info("总请求: " + std::to_string(total_requests) + ", 总耗时: " + std::to_string(total_time_us) + "us, QPS: " + std::to_string(qps) + ", 最大并发: " + std::to_string(max_concurrent));
    }
};
};
