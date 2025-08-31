#include "ApplicationServer.h"
#include <sstream>
#include <iomanip>
#include "HttpProtocol.h"
#include <sstream>
#include <algorithm>
#include <sys/socket.h>

ApplicationServer::ApplicationServer(const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool)
    : TcpServer(ip, port, io_type), m_pool(pool) {}

ApplicationServer::~ApplicationServer() {}

bool ApplicationServer::start() {
    // 初始化协议路由器
    m_router = std::make_unique<ProtocolRouter>();

    // 设置ProtocolRouter的回调（关键修复！）
    m_router->setPacketCallback([this](uint32_t protoId, const std::vector<char>& packet) {
        Logger::info("ApplicationServer收到协议" + std::to_string(protoId) + "的数据包，长度: " + std::to_string(packet.size()));
        this->onProtocolPacket(protoId, packet);
    });

    initializeProtocolRouter();

    // 设置TcpServer的消息回调
    setOnMessage([this](int clientFd, const std::string& data) {
        Logger::info("ApplicationServer通过回调收到客户端" + std::to_string(clientFd) + "的数据，长度: " + std::to_string(data.length()));
        this->onDataReceived(clientFd, data.c_str(), data.length());
    });

    // 启动TCP服务器
    return TcpServer::start();
}

void ApplicationServer::stop() {
    TcpServer::stop();
}

void ApplicationServer::onDataReceived(int clientFd, const char* data, size_t len) {
    Logger::info("ApplicationServer收到客户端" + std::to_string(clientFd) + "的数据，长度: " + std::to_string(len));

    // 保存当前客户端FD，供协议回调使用
    m_currentClientFd = clientFd;

    // 调试：打印原始数据的十六进制
    std::ostringstream hexStream;
    hexStream << "原始数据十六进制: ";
    for (size_t i = 0; i < len && i < 50; ++i) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (unsigned char)data[i] << " ";
    }
    Logger::debug(hexStream.str());

    // 通过协议分发器处理数据
    if (m_router) {
        size_t processed = m_router->onDataReceived(clientFd, data, len);
        Logger::debug("协议分发器处理了 " + std::to_string(processed) + " 字节");

        if (processed == 0 && len > 0) {
            Logger::warn("协议分发器未处理任何数据，可能数据不完整");
        }
    } else {
        Logger::error("协议分发器未初始化");
    }
}

void ApplicationServer::onProtocolPacket(uint32_t protoId, const std::vector<char>& packet) {
    Logger::info("ApplicationServer::onProtocolPacket 被调用，协议ID: " + std::to_string(protoId));

    // 这里可以根据协议ID进行不同的处理
    // 对于PureRedisProtocol (ID=3)，直接发送响应
    if (protoId == 3) {  // PureRedisProtocol
        std::string response(packet.begin(), packet.end());
        Logger::info("收到PureRedisProtocol响应: " + response.substr(0, 20) + "...");

        // 直接发送RESP响应
        if (m_currentClientFd > 0) {
            ssize_t sent = send(m_currentClientFd, response.c_str(), response.length(), 0);
            if (sent > 0) {
                Logger::info("Pure Redis响应已发送到客户端" + std::to_string(m_currentClientFd) + "，长度: " + std::to_string(sent));
            } else {
                Logger::error("发送Pure Redis响应失败，错误码: " + std::to_string(errno));
            }
        } else {
            Logger::error("无效的客户端FD，无法发送响应");
        }
    } else {
        Logger::warn("未处理的协议ID: " + std::to_string(protoId));
    }
}

void ApplicationServer::onClientConnected(int clientFd) {
    (void)clientFd; // 避免未使用参数警告
    // 基类默认实现，子类可以重写
}

void ApplicationServer::onClientDisconnected(int clientFd) {
    (void)clientFd; // 避免未使用参数警告
    // 基类默认实现，子类可以重写
}

std::string ApplicationServer::generateJsonResponse(bool success, const std::string& data, const std::string& message) {
    std::string json = "{";
    json += "\"success\":" + std::string(success ? "true" : "false") + ",";
    json += "\"data\":\"" + data + "\",";
    json += "\"message\":\"" + message + "\"";
    json += "}";
    return json;
} 