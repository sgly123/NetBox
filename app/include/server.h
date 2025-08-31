#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <atomic>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include "IO/IOFactory.h"
#include "server/TcpServer.h"
#include "base/IThreadPool.h"
#include "SimpleHeaderProtocol.h"
#include "HttpProtocol.h"
#include "ProtocolRouter.h"
#include "ApplicationServer.h"

/**
 * @brief 客户端连接信息结构体
 */
struct ClientInfo {
    uint32_t protocolId;                                    // 使用的协议ID
    std::chrono::steady_clock::time_point lastActiveTime;   // 最后活跃时间
    std::vector<char> buffer;                               // 客户端缓冲区
};

/**
 * @brief EchoServer 应用层服务器实现
 * 
 * 设计思路：
 * 1. 继承自 TcpServer，复用网络层功能
 * 2. 集成 ProtocolRouter，实现协议分发
 * 3. 实现业务逻辑（回显功能）
 * 4. 支持线程池注入，实现异步处理
 * 
 * 架构层次：
 * - 应用层：EchoServer（业务逻辑）
 * - 协议层：ProtocolRouter + SimpleHeaderProtocol（协议处理）
 * - 网络层：TcpServer（网络IO）
 * 
 * 数据流向：
 * 客户端数据 → TcpServer::handleRead → ProtocolRouter → SimpleHeaderProtocol → EchoServer::onPacketReceived → 回显响应
 */
class EchoServer : public ApplicationServer {
public:
    /**
     * @brief 构造函数，支持依赖注入设计
     * @param ip 监听IP地址
     * @param port 监听端口号
     * @param io_type IO多路复用类型（SELECT/POLL/EPOLL）
     * @param pool 线程池指针，可为nullptr（主线程处理）
     * 
     * 设计优势：
     * 1. 支持线程池注入，便于不同业务场景的线程管理
     * 2. 支持多种IO模型，可根据性能需求选择
     * 3. 配置灵活，支持不同部署环境
     */
    EchoServer(const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool = nullptr);
    
    /**
     * @brief 析构函数，确保资源正确释放
     */
    ~EchoServer();

    /**
     * @brief 重写网络层读事件处理，实现应用层逻辑
     * @param client_fd 客户端文件描述符
     * 
     * 处理流程：
     * 1. 从 client_fd 读取原始数据
     * 2. 记录当前处理的客户端fd（用于回显）
     * 3. 调用协议分发器处理数据
     * 4. 清理临时状态
     * 
     * 设计说明：
     * - 重写基类方法，实现应用层特定的数据处理逻辑
     * - 通过 m_currentClientFd 记录当前客户端，支持回显功能
     * - 调用协议分发器，实现协议层和应用层的解耦
     */
    void handleRead(int client_fd) override;
    
    /**
     * @brief 应用层回调：处理完整的协议数据包
     * @param packet 完整的业务数据包（已去除协议头）
     * 
     * 业务逻辑：
     * 1. 解析业务数据（字符串消息）
     * 2. 构造回显消息（"Echo: " + 原消息）
     * 3. 使用协议层封包
     * 4. 添加协议路由头
     * 5. 发送回客户端
     * 
     * 设计优势：
     * - 只处理完整的业务数据，不关心协议细节
     * - 通过协议层封包，确保数据格式正确
     * - 支持多种协议，只需修改协议分发器配置
     */
    void onPacketReceived(const std::vector<char>& packet);
    
    /**
     * @brief HTTP协议回调：处理完整的HTTP数据包
     * @param packet 完整的HTTP数据包
     * 
     * 业务逻辑：
     * 1. 解析HTTP请求内容
     * 2. 构造HTTP响应
     * 3. 使用HTTP协议封包
     * 4. 发送回客户端
     * 
     * 设计优势：
     * - 专门处理HTTP协议，提供Web服务功能
     * - 支持HTTP请求/响应格式
     * - 可扩展为完整的Web服务器
     */
    void onHttpPacketReceived(const std::vector<char>& packet);
    
    // ApplicationServer纯虚函数实现
    void initializeProtocolRouter() override;
    std::string handleHttpRequest(const std::string& request, int clientFd) override;
    std::string handleBusinessLogic(const std::string& command, const std::vector<std::string>& args) override;
    bool parseRequestPath(const std::string& path, std::string& command, std::vector<std::string>& args) override;
    
private:
    ProtocolRouter m_router;                // 协议分发器，负责协议识别和数据分发
    int m_currentClientFd = -1;             // 当前处理的客户端fd，用于回显功能
    
    // 客户端连接管理
    std::unordered_map<int, ClientInfo> m_clientInfo; // 客户端连接信息
    mutable std::mutex m_clientInfoMutex;             // 客户端信息访问互斥锁
};

#endif // SERVER_H