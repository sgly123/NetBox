#pragma once

#include "server/TcpServer.h"
#include "ProtocolRouter.h"
#include "base/ThreadPool.h"
#include "base/Logger.h"
#include <memory>
#include <functional>
#include <vector>
#include <string>

/**
 * @brief 应用层基础服务器类
 * 
 * 继承关系：TcpServer → ApplicationServer → 具体业务Server
 */
class ApplicationServer : public TcpServer {
public:
    ApplicationServer(const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool = nullptr);
    virtual ~ApplicationServer();

    virtual bool start() override;
    virtual void stop() override;

protected:
    std::unique_ptr<ProtocolRouter> m_router;
    IThreadPool* m_pool;
    int m_currentClientFd = 0;  // 当前处理的客户端FD

    // 协议路由器初始化（由子类实现）
    virtual void initializeProtocolRouter() = 0;

    // 业务相关接口（由子类实现）
    virtual std::string handleHttpRequest(const std::string& request, int clientFd) = 0;
    virtual std::string handleBusinessLogic(const std::string& command, const std::vector<std::string>& args) = 0;
    virtual bool parseRequestPath(const std::string& path, std::string& command, std::vector<std::string>& args) = 0;

    // 生成JSON响应
    std::string generateJsonResponse(bool success, const std::string& data, const std::string& message);

    // 网络事件回调
    virtual void onDataReceived(int clientFd, const char* data, size_t len) override;
    virtual void onClientConnected(int clientFd) override;
    virtual void onClientDisconnected(int clientFd) override;

    // 协议回调
    void onProtocolPacket(uint32_t protoId, const std::vector<char>& packet);
}; 