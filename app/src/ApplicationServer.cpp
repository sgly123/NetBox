#include "ApplicationServer.h"
#include <sstream>
#include <iomanip>
#include "HttpProtocol.h"
#include "PureRedisProtocol.h"
#include <sstream>
#include <algorithm>
#include <sys/socket.h>
#include "ProtocolRouter.h"

ApplicationServer::ApplicationServer(const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool)
    : TcpServer(ip, port, io_type), m_pool(pool) {}

ApplicationServer::~ApplicationServer() {}

bool ApplicationServer::start() {
    // 初始化协议路由器
    m_router = std::make_unique<ProtocolRouter>();

    // 设置ProtocolRouter的回调（关键修复！）
    m_router->setPacketCallback([this](uint32_t protoId, const std::vector<char>& packet) {
        if (protoId  != 3) {
            Logger::info("ApplicationServer收到协议" + std::to_string(protoId) + "的数据包，长度: " + std::to_string(packet.size()));
            this->onProtocolPacket(protoId, packet);
        }
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

    m_currentClientFd = clientFd;

    std::ostringstream hexStream;
    hexStream << "原始数据十六进制: ";
    for (size_t i = 0; i < len && i < 50; ++i) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << (unsigned char)data[i] << " ";
    }
    Logger::debug(hexStream.str());

    // ✅ 优先：如果是 RESP 格式（以 '*' 开头），直接走 PureRedisProtocol
    if (len > 0 && data[0] == '*') {
        auto* pureProto = dynamic_cast<PureRedisProtocol*>(m_router->getProtocol(3));
        if (pureProto) {
            size_t processed = pureProto->onClientDataReceived(clientFd, data, len);
            Logger::debug("PureRedisProtocol 直接处理了 " + std::to_string(processed) + " 字节");
            return; // ✅ 处理完就返回，不再走分发器
        }
    }

    // ✅  fallback：非 RESP 再走协议分发器
    if (m_router) {
        size_t processed = m_router->onDataReceived(clientFd, data, len);
        Logger::debug("协议分发器处理了 " + std::to_string(processed) + " 字节");

        if (processed == 0 && len > 0) {
            Logger::warn("协议分发器未识别，仍尝试 PureRedisProtocol");
            auto* pureProto = dynamic_cast<PureRedisProtocol*>(m_router->getProtocol(3));
            if (pureProto) {
                size_t fallbackProcessed = pureProto->onClientDataReceived(clientFd, data, len);
                Logger::debug("PureRedisProtocol 兜底处理了 " + std::to_string(fallbackProcessed) + " 字节");
            } else {
                Logger::error("PureRedisProtocol 未注册");
            }
        }
    } else {
        Logger::error("协议分发器未初始化");
    }
} 

void ApplicationServer::onProtocolPacket(uint32_t protoId, [[maybe_unused]] const std::vector<char>& packet) {
    if (protoId != 3) {  // 跳过 Redis 数据包
        Logger::info("ApplicationServer::onProtocolPacket 被调用，协议ID: " + std::to_string(protoId));
        // 处理其他协议
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