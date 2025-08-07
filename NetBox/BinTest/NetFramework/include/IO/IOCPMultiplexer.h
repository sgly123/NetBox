#pragma once

/**
 * @file IOCPMultiplexer.h
 * @brief Windows IOCP (I/O Completion Port) 多路复用器实现
 * 
 * 设计说明：
 * 1. 利用Windows IOCP实现高性能异步IO
 * 2. 支持大量并发连接（理论上无限制）
 * 3. 与NetBox框架的IOMultiplexer接口兼容
 * 4. 提供Windows平台最优的网络性能
 * 
 * IOCP技术特点：
 * - 异步IO模型，避免线程阻塞
 * - 内核级别的事件通知
 * - 支持海量并发连接
 * - CPU利用率高，扩展性好
 * - Windows服务器应用的首选IO模型
 * 
 * 实现挑战：
 * - 需要适配同步的IOMultiplexer接口
 * - 处理异步IO的复杂状态管理
 * - 内存管理和错误处理
 */

#ifdef NETBOX_PLATFORM_WINDOWS

#include "base/IOMultiplexer.h"
#include "platform/WindowsPlatform.h"
#include <windows.h>
#include <winsock2.h>
#include <mswsock.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

namespace NetBox {
namespace IO {

/**
 * @brief IOCP操作类型枚举
 * 定义不同的异步IO操作类型
 */
enum class IOCPOperationType {
    ACCEPT,     // 接受连接操作
    RECV,       // 接收数据操作
    SEND,       // 发送数据操作
    DISCONNECT  // 断开连接操作
};

/**
 * @brief IOCP操作上下文结构
 * 每个异步IO操作都需要一个上下文结构
 */
struct IOCPContext {
    OVERLAPPED overlapped;          // Windows异步IO必需的重叠结构
    IOCPOperationType operation;    // 操作类型
    SOCKET socket;                  // 关联的socket
    WSABUF wsaBuf;                 // Windows socket缓冲区
    char buffer[8192];             // 数据缓冲区
    DWORD bytesTransferred;        // 传输的字节数
    DWORD flags;                   // 操作标志
    
    /**
     * @brief 构造函数
     * @param op 操作类型
     * @param sock socket描述符
     */
    IOCPContext(IOCPOperationType op, SOCKET sock) 
        : operation(op), socket(sock), bytesTransferred(0), flags(0) {
        ZeroMemory(&overlapped, sizeof(overlapped));
        wsaBuf.buf = buffer;
        wsaBuf.len = sizeof(buffer);
    }
    
    /**
     * @brief 重置上下文以便重用
     */
    void reset() {
        ZeroMemory(&overlapped, sizeof(overlapped));
        bytesTransferred = 0;
        flags = 0;
    }
};

/**
 * @brief Windows IOCP多路复用器实现
 * 
 * 架构设计：
 * 1. 创建完成端口 (CreateIoCompletionPort)
 * 2. 将socket关联到完成端口
 * 3. 投递异步IO操作 (WSARecv, WSASend等)
 * 4. 等待完成通知 (GetQueuedCompletionStatus)
 * 5. 处理完成的IO操作
 * 
 * 线程模型：
 * - 主线程：处理连接管理和事件分发
 * - 工作线程：处理IOCP完成通知（可选）
 * - 支持单线程和多线程模式
 */
class IOCPMultiplexer : public IOMultiplexer {
private:
    HANDLE m_completionPort;                                    // 完成端口句柄
    std::unordered_map<int, SOCKET> m_sockets;                 // socket映射表
    std::unordered_map<int, IOMultiplexer::EventType> m_events; // 事件类型映射
    std::vector<std::unique_ptr<IOCPContext>> m_contexts;      // 操作上下文池
    std::mutex m_mutex;                                         // 线程安全锁
    std::atomic<bool> m_initialized{false};                    // 初始化状态
    
    // IOCP相关函数指针（动态加载）
    LPFN_ACCEPTEX m_acceptEx;
    LPFN_GETACCEPTEXSOCKADDRS m_getAcceptExSockaddrs;
    LPFN_DISCONNECTEX m_disconnectEx;
    
    /**
     * @brief 加载IOCP扩展函数
     * @param socket 用于获取函数指针的socket
     * @return bool 加载是否成功
     */
    bool loadIOCPExtensions(SOCKET socket);
    
    /**
     * @brief 投递接收操作
     * @param socket 目标socket
     * @return bool 投递是否成功
     */
    bool postReceive(SOCKET socket);
    
    /**
     * @brief 投递接受连接操作
     * @param listenSocket 监听socket
     * @return bool 投递是否成功
     */
    bool postAccept(SOCKET listenSocket);
    
    /**
     * @brief 处理完成的IO操作
     * @param context 操作上下文
     * @param bytesTransferred 传输字节数
     * @param error 错误码
     */
    void handleCompletedIO(IOCPContext* context, DWORD bytesTransferred, DWORD error);
    
    /**
     * @brief 创建操作上下文
     * @param operation 操作类型
     * @param socket socket描述符
     * @return IOCPContext* 上下文指针
     */
    IOCPContext* createContext(IOCPOperationType operation, SOCKET socket);
    
    /**
     * @brief 回收操作上下文
     * @param context 上下文指针
     */
    void recycleContext(IOCPContext* context);

public:
    /**
     * @brief 构造函数
     */
    IOCPMultiplexer();
    
    /**
     * @brief 析构函数
     */
    ~IOCPMultiplexer() override;
    
    /**
     * @brief 初始化IOCP多路复用器
     * @return bool 初始化是否成功
     * 
     * 实现步骤：
     * 1. 初始化Winsock库
     * 2. 创建IO完成端口
     * 3. 加载IOCP扩展函数
     * 4. 设置工作线程数量
     */
    bool init() override;
    
    /**
     * @brief 获取IO类型
     * @return IOType IO类型标识
     */
    IOType type() const override {
        return IOType::IOCP; // 需要在IOMultiplexer.h中添加IOCP类型
    }
    
    /**
     * @brief 添加文件描述符到监听集合
     * @param fd 文件描述符
     * @param events 监听的事件类型
     * @return bool 添加是否成功
     * 
     * IOCP实现：
     * 1. 将socket关联到完成端口
     * 2. 根据事件类型投递相应的异步操作
     * 3. 记录socket和事件类型的映射关系
     */
    bool addfd(int fd, EventType events) override;
    
    /**
     * @brief 从监听集合中移除文件描述符
     * @param fd 文件描述符
     * @return bool 移除是否成功
     * 
     * IOCP实现：
     * 1. 取消所有未完成的IO操作
     * 2. 从映射表中移除记录
     * 3. 清理相关资源
     */
    bool removefd(int fd) override;
    
    /**
     * @brief 修改文件描述符的监听事件
     * @param fd 文件描述符
     * @param events 新的事件类型
     * @return bool 修改是否成功
     * 
     * IOCP实现：
     * 1. 取消当前的IO操作
     * 2. 根据新事件类型投递新操作
     * 3. 更新事件映射表
     */
    bool modifyFd(int fd, EventType events) override;
    
    /**
     * @brief 等待IO事件
     * @param activeEvents 输出参数，存储活跃的事件
     * @param timeout 超时时间（毫秒）
     * @return int 活跃事件的数量，-1表示错误
     * 
     * IOCP实现：
     * 1. 调用GetQueuedCompletionStatus等待完成通知
     * 2. 处理完成的IO操作
     * 3. 将结果转换为IOMultiplexer标准格式
     * 4. 重新投递新的异步操作以保持监听状态
     */
    int wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) override;
    
    /**
     * @brief 获取性能统计信息
     * @return IOCPStats 性能统计结构
     */
    struct IOCPStats {
        uint64_t totalOperations;      // 总操作数
        uint64_t completedOperations;  // 完成操作数
        uint64_t failedOperations;     // 失败操作数
        uint64_t bytesTransferred;     // 传输字节数
        uint32_t activeConnections;    // 活跃连接数
        uint32_t pendingOperations;    // 待完成操作数
    };
    
    IOCPStats getStats() const;
    
    /**
     * @brief 设置工作线程数量
     * @param threadCount 线程数量，0表示使用CPU核心数
     * 
     * 说明：
     * - IOCP可以使用多个工作线程处理完成通知
     * - 通常设置为CPU核心数的1-2倍
     * - 过多线程可能导致上下文切换开销
     */
    void setWorkerThreadCount(int threadCount);
    
    /**
     * @brief 启用/禁用Nagle算法
     * @param fd socket描述符
     * @param enable 是否启用
     * @return bool 设置是否成功
     */
    bool setTcpNoDelay(int fd, bool enable);
    
    /**
     * @brief 设置socket缓冲区大小
     * @param fd socket描述符
     * @param sendBufSize 发送缓冲区大小
     * @param recvBufSize 接收缓冲区大小
     * @return bool 设置是否成功
     */
    bool setSocketBufferSize(int fd, int sendBufSize, int recvBufSize);

private:
    IOCPStats m_stats;              // 性能统计
    int m_workerThreadCount;        // 工作线程数量
    std::vector<HANDLE> m_workerThreads; // 工作线程句柄
    std::atomic<bool> m_shutdown{false};  // 关闭标志
    
    /**
     * @brief 工作线程函数
     * @param param 线程参数（IOCPMultiplexer指针）
     * @return DWORD 线程返回值
     */
    static DWORD WINAPI workerThreadProc(LPVOID param);
    
    /**
     * @brief 工作线程主循环
     */
    void workerThreadLoop();
};

} // namespace IO
} // namespace NetBox

#endif // NETBOX_PLATFORM_WINDOWS
