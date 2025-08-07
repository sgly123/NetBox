#pragma once

/**
 * @file KqueueMultiplexer.h
 * @brief macOS/FreeBSD Kqueue 多路复用器实现
 * 
 * 设计说明：
 * 1. 利用BSD kqueue实现高性能事件通知
 * 2. 支持文件、网络、定时器等多种事件类型
 * 3. 与NetBox框架的IOMultiplexer接口兼容
 * 4. 提供macOS/BSD平台最优的网络性能
 * 
 * Kqueue技术特点：
 * - 统一的事件通知接口
 * - 支持边缘触发和水平触发
 * - 内核级别的事件过滤
 * - 高效的事件聚合机制
 * - BSD系统的原生高性能IO模型
 * 
 * 相比其他IO模型的优势：
 * - 比select/poll更高效
 * - 事件类型更丰富
 * - 内存使用更少
 * - 扩展性更好
 */

#if defined(NETBOX_PLATFORM_MACOS) || defined(NETBOX_PLATFORM_FREEBSD) || defined(NETBOX_PLATFORM_OPENBSD)

#include "base/IOMultiplexer.h"
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

namespace NetBox {
namespace IO {

/**
 * @brief Kqueue事件过滤器类型
 * 定义kqueue支持的不同事件类型
 */
enum class KqueueFilter {
    READ = EVFILT_READ,      // 读事件过滤器
    WRITE = EVFILT_WRITE,    // 写事件过滤器
    TIMER = EVFILT_TIMER,    // 定时器过滤器
    SIGNAL = EVFILT_SIGNAL,  // 信号过滤器
    PROC = EVFILT_PROC,      // 进程事件过滤器
    VNODE = EVFILT_VNODE     // 文件系统事件过滤器
};

/**
 * @brief Kqueue事件标志
 * 定义事件的行为特性
 */
enum class KqueueFlags {
    ADD = EV_ADD,           // 添加事件
    DELETE = EV_DELETE,     // 删除事件
    ENABLE = EV_ENABLE,     // 启用事件
    DISABLE = EV_DISABLE,   // 禁用事件
    ONESHOT = EV_ONESHOT,   // 一次性事件
    CLEAR = EV_CLEAR,       // 边缘触发（清除事件）
    EOF_FLAG = EV_EOF,      // 文件结束标志
    ERROR_FLAG = EV_ERROR   // 错误标志
};

/**
 * @brief Kqueue多路复用器实现
 * 
 * 架构设计：
 * 1. 创建kqueue描述符 (kqueue())
 * 2. 注册感兴趣的事件 (kevent() with EV_ADD)
 * 3. 等待事件发生 (kevent() with timeout)
 * 4. 处理触发的事件
 * 5. 管理事件的生命周期
 * 
 * 事件处理流程：
 * - 网络读写事件：EVFILT_READ/EVFILT_WRITE
 * - 连接状态变化：通过EOF标志检测
 * - 错误处理：通过ERROR标志检测
 * - 边缘触发：使用EV_CLEAR标志
 */
class KqueueMultiplexer : public IOMultiplexer {
private:
    int m_kqueueFd;                                             // kqueue文件描述符
    std::vector<struct kevent> m_events;                        // 事件数组
    std::unordered_map<int, IOMultiplexer::EventType> m_fdEvents; // fd到事件类型的映射
    std::mutex m_mutex;                                         // 线程安全锁
    std::atomic<bool> m_initialized{false};                    // 初始化状态
    int m_maxEvents;                                            // 最大事件数量
    
    /**
     * @brief 将IOMultiplexer事件类型转换为kqueue过滤器
     * @param events IOMultiplexer事件类型
     * @return std::vector<KqueueFilter> kqueue过滤器列表
     */
    std::vector<KqueueFilter> convertToKqueueFilters(EventType events);
    
    /**
     * @brief 将kqueue事件转换为IOMultiplexer事件类型
     * @param kevent kqueue事件结构
     * @return EventType IOMultiplexer事件类型
     */
    EventType convertFromKqueueEvent(const struct kevent& kevent);
    
    /**
     * @brief 添加kqueue事件
     * @param fd 文件描述符
     * @param filter 事件过滤器
     * @param flags 事件标志
     * @return bool 添加是否成功
     */
    bool addKqueueEvent(int fd, KqueueFilter filter, uint16_t flags = EV_ADD | EV_ENABLE);
    
    /**
     * @brief 删除kqueue事件
     * @param fd 文件描述符
     * @param filter 事件过滤器
     * @return bool 删除是否成功
     */
    bool deleteKqueueEvent(int fd, KqueueFilter filter);
    
    /**
     * @brief 修改kqueue事件
     * @param fd 文件描述符
     * @param filter 事件过滤器
     * @param flags 新的事件标志
     * @return bool 修改是否成功
     */
    bool modifyKqueueEvent(int fd, KqueueFilter filter, uint16_t flags);

public:
    /**
     * @brief 构造函数
     * @param maxEvents 最大事件数量，默认1024
     */
    explicit KqueueMultiplexer(int maxEvents = 1024);
    
    /**
     * @brief 析构函数
     */
    ~KqueueMultiplexer() override;
    
    /**
     * @brief 初始化kqueue多路复用器
     * @return bool 初始化是否成功
     * 
     * 实现步骤：
     * 1. 创建kqueue描述符
     * 2. 预分配事件数组
     * 3. 设置相关参数
     */
    bool init() override;
    
    /**
     * @brief 获取IO类型
     * @return IOType IO类型标识
     */
    IOType type() const override {
        return IOType::KQUEUE; // 需要在IOMultiplexer.h中添加KQUEUE类型
    }
    
    /**
     * @brief 添加文件描述符到监听集合
     * @param fd 文件描述符
     * @param events 监听的事件类型
     * @return bool 添加是否成功
     * 
     * Kqueue实现：
     * 1. 将IOMultiplexer事件类型转换为kqueue过滤器
     * 2. 为每个过滤器调用kevent()注册事件
     * 3. 记录fd和事件类型的映射关系
     * 4. 支持同时监听读写事件
     */
    bool addfd(int fd, EventType events) override;
    
    /**
     * @brief 从监听集合中移除文件描述符
     * @param fd 文件描述符
     * @return bool 移除是否成功
     * 
     * Kqueue实现：
     * 1. 获取当前监听的事件类型
     * 2. 为每个过滤器调用kevent()删除事件
     * 3. 从映射表中移除记录
     */
    bool removefd(int fd) override;
    
    /**
     * @brief 修改文件描述符的监听事件
     * @param fd 文件描述符
     * @param events 新的事件类型
     * @return bool 修改是否成功
     * 
     * Kqueue实现：
     * 1. 删除旧的事件过滤器
     * 2. 添加新的事件过滤器
     * 3. 更新事件映射表
     */
    bool modifyFd(int fd, EventType events) override;
    
    /**
     * @brief 等待IO事件
     * @param activeEvents 输出参数，存储活跃的事件
     * @param timeout 超时时间（毫秒）
     * @return int 活跃事件的数量，-1表示错误
     * 
     * Kqueue实现：
     * 1. 设置超时时间结构体
     * 2. 调用kevent()等待事件
     * 3. 遍历返回的事件数组
     * 4. 将kqueue事件转换为IOMultiplexer格式
     * 5. 处理特殊标志（EOF、ERROR等）
     */
    int wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) override;
    
    /**
     * @brief 获取kqueue性能统计信息
     * @return KqueueStats 性能统计结构
     */
    struct KqueueStats {
        uint64_t totalEvents;          // 总事件数
        uint64_t readEvents;           // 读事件数
        uint64_t writeEvents;          // 写事件数
        uint64_t errorEvents;          // 错误事件数
        uint64_t eofEvents;            // EOF事件数
        uint32_t activeDescriptors;    // 活跃描述符数
        uint32_t maxDescriptors;       // 最大描述符数
    };
    
    KqueueStats getStats() const;
    
    /**
     * @brief 设置边缘触发模式
     * @param fd 文件描述符
     * @param enable 是否启用边缘触发
     * @return bool 设置是否成功
     * 
     * 边缘触发特点：
     * - 事件只在状态变化时触发一次
     * - 需要一次性读取/写入所有可用数据
     * - 性能更高，但编程复杂度增加
     */
    bool setEdgeTriggered(int fd, bool enable);
    
    /**
     * @brief 添加定时器事件
     * @param timerId 定时器ID
     * @param intervalMs 间隔时间（毫秒）
     * @param oneshot 是否为一次性定时器
     * @return bool 添加是否成功
     * 
     * Kqueue定时器特性：
     * - 高精度定时器
     * - 支持一次性和周期性定时器
     * - 与IO事件统一处理
     */
    bool addTimer(int timerId, int intervalMs, bool oneshot = false);
    
    /**
     * @brief 删除定时器事件
     * @param timerId 定时器ID
     * @return bool 删除是否成功
     */
    bool removeTimer(int timerId);
    
    /**
     * @brief 添加信号监听
     * @param signal 信号编号
     * @return bool 添加是否成功
     * 
     * Kqueue信号处理：
     * - 统一的信号事件处理
     * - 避免传统信号处理的竞态条件
     * - 与IO事件同步处理
     */
    bool addSignal(int signal);
    
    /**
     * @brief 删除信号监听
     * @param signal 信号编号
     * @return bool 删除是否成功
     */
    bool removeSignal(int signal);
    
    /**
     * @brief 获取kqueue文件描述符
     * @return int kqueue文件描述符
     * 
     * 用途：
     * - 高级用户直接操作kqueue
     * - 集成其他kqueue相关库
     * - 调试和监控
     */
    int getKqueueFd() const { return m_kqueueFd; }

private:
    KqueueStats m_stats;            // 性能统计
    
    /**
     * @brief 更新性能统计
     * @param kevent kqueue事件
     */
    void updateStats(const struct kevent& kevent);
    
    /**
     * @brief 检查文件描述符是否有效
     * @param fd 文件描述符
     * @return bool 是否有效
     */
    bool isValidFd(int fd) const;
    
    /**
     * @brief 处理kqueue错误
     * @param operation 操作名称
     * @param fd 相关文件描述符
     * @return bool 是否为可恢复错误
     */
    bool handleKqueueError(const std::string& operation, int fd = -1);
};

} // namespace IO
} // namespace NetBox

#endif // NETBOX_PLATFORM_MACOS || NETBOX_PLATFORM_FREEBSD || NETBOX_PLATFORM_OPENBSD
