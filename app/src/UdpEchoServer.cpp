#include "UdpEchoServer.h"
#include "base/Logger.h"
#include <sstream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

UdpEchoServer::UdpEchoServer(const std::string& ip, int port, IOMultiplexer::IOType io_type)
    : UdpServer(ip, port, io_type)
{
    Logger::info("UDP Echo服务器创建 " + ip + ":" + std::to_string(port));
}

UdpEchoServer::~UdpEchoServer() {
    Logger::info("UDP Echo服务器销毁");
}

bool UdpEchoServer::startEchoServer() {
    // 设置数据包处理回调
    setOnDatagram([this](const sockaddr_in& from, const std::string& data) {
        // 数据包处理将在重写的onDatagramReceived中进行
        // 这里的lambda主要是为了与基类的回调机制兼容
        (void)from; (void)data; // 避免未使用参数警告
    });

    // 设置错误处理回调
    setOnError([this](int error_code, const std::string& message) {
        // 错误处理将在重写的onError中进行
        (void)error_code; (void)message; // 避免未使用参数警告
    });

    // 启动服务器
    bool success = start();
    if (success) {
        Logger::info("UDP Echo服务器启动成功");
    } else {
        Logger::error("UDP Echo服务器启动失败");
    }
    
    return success;
}

std::unordered_map<std::string, UdpEchoServer::ClientStats> UdpEchoServer::getClientStats() const {
    std::lock_guard<std::mutex> lock(m_clientStatsMutex);
    return m_clientStats;
}

void UdpEchoServer::cleanupInactiveClients(int timeout_seconds) {
    std::lock_guard<std::mutex> lock(m_clientStatsMutex);
    
    auto now = std::chrono::steady_clock::now();
    auto timeout_duration = std::chrono::seconds(timeout_seconds);
    
    auto it = m_clientStats.begin();
    while (it != m_clientStats.end()) {
        if (now - it->second.last_activity > timeout_duration) {
            Logger::debug("清理不活跃客户端: " + it->first);
            it = m_clientStats.erase(it);
        } else {
            ++it;
        }
    }
}

void UdpEchoServer::printStats() const {
    std::lock_guard<std::mutex> lock(m_clientStatsMutex);
    
    // 打印服务器总体统计
    const auto& serverStats = getStats();
    std::ostringstream oss;
    oss << "\n=== UDP Echo服务器统计信息 ===\n";
    oss << "总接收数据包: " << serverStats.packets_received.load() << "\n";
    oss << "总发送数据包: " << serverStats.packets_sent.load() << "\n";
    oss << "总接收字节数: " << serverStats.bytes_received.load() << "\n";
    oss << "总发送字节数: " << serverStats.bytes_sent.load() << "\n";
    oss << "接收错误次数: " << serverStats.recv_errors.load() << "\n";
    oss << "发送错误次数: " << serverStats.send_errors.load() << "\n";
    oss << "活跃客户端数: " << m_clientStats.size() << "\n";
    
    // 打印客户端统计
    if (!m_clientStats.empty()) {
        oss << "\n=== 客户端统计信息 ===\n";
        for (const auto& [client, stats] : m_clientStats) {
            oss << "客户端 " << client << ":\n";
            oss << "  接收数据包: " << stats.packets_received << "\n";
            oss << "  发送数据包: " << stats.packets_sent << "\n";
            oss << "  接收字节数: " << stats.bytes_received << "\n";
            oss << "  发送字节数: " << stats.bytes_sent << "\n";
            
            auto elapsed = std::chrono::steady_clock::now() - stats.last_activity;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
            oss << "  最后活跃: " << seconds << "秒前\n";
        }
    }
    oss << "==============================";
    
    Logger::info(oss.str());
}

void UdpEchoServer::onDatagramReceived(const sockaddr_in& from, const char* data, size_t len) {
    std::string client_key = getClientKey(from);
    std::string message(data, len);
    
    Logger::debug("收到来自 " + client_key + " 的数据: " + message);
    
    // 更新客户端统计信息
    updateClientStats(client_key, len);
    
    // 回显数据
    std::string echo_message = "[ECHO] " + message;
    bool sent = sendTo(from, echo_message);
    
    if (sent) {
        // 更新发送统计
        updateClientStats(client_key, 0, echo_message.size());
        Logger::debug("回显给 " + client_key + ": " + echo_message);
    } else {
        Logger::warn("回显失败 to " + client_key);
    }
}

void UdpEchoServer::onError(int error_code, const std::string& message) {
    Logger::error("UDP Echo服务器错误 [" + std::to_string(error_code) + "]: " + message);
    
    // 如果是严重错误，可以考虑重启或其他恢复措施
    switch (error_code) {
        case UdpErrorType::BIND_FAILED:
        case UdpErrorType::UDP_SOCKET_ERROR:
            Logger::error("严重错误，服务器可能需要重启");
            break;
        case UdpErrorType::SEND_FAILED:
        case UdpErrorType::RECV_FAILED:
            Logger::warn("网络IO错误，继续运行");
            break;
        default:
            Logger::warn("其他错误: " + message);
            break;
    }
}

void UdpEchoServer::updateClientStats(const std::string& client_key, uint64_t bytes_received, uint64_t bytes_sent) {
    std::lock_guard<std::mutex> lock(m_clientStatsMutex);
    
    auto& stats = m_clientStats[client_key];
    if (bytes_received > 0) {
        stats.packets_received++;
        stats.bytes_received += bytes_received;
    }
    if (bytes_sent > 0) {
        stats.packets_sent++;
        stats.bytes_sent += bytes_sent;
    }
    stats.last_activity = std::chrono::steady_clock::now();
}

std::string UdpEchoServer::getClientKey(const sockaddr_in& addr) const {
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
    return std::string(ip_str) + ":" + std::to_string(ntohs(addr.sin_port));
} 