#include "../include/server/TcpServer.h"
#include "base/Logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <thread>
#include "base/HeartbeatThreadPool.h"
#include <chrono>
const uint32_t HEARTBEAT_MAGIC = 0x12345678;


TcpServer::TcpServer(const std::string& ip, int port, IOMultiplexer::IOType io_type)
    : m_port(port), m_ip(ip), m_running(false), m_io(IOFactory::createIO(io_type)) {
    m_socket = -1;
}

TcpServer::~TcpServer() {
    stop();
}

void TcpServer::setOnConnect(OnConnectCallback cb) {
    m_onConnect = cb;
}

void TcpServer::setOnMessage(OnMessageCallback cb) {
    m_onMessage = cb;
}

void TcpServer::setOnClose(OnCloseCallback cb) {
    m_onClose = cb;
}

IOMultiplexer::IOType TcpServer::type() const {
    return m_io ? m_io->type() : IOMultiplexer::IOType::SELECT;
}

bool TcpServer::start() {
    // 创建socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
        Logger::error("socket creation failed");
        return false;
    }
    // 设置SO_REUSEADDR
    int opt = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        Logger::error("setsockopt failed");
        close(m_socket);
        return false;
    }
    // 绑定地址
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    if (inet_pton(AF_INET, m_ip.c_str(), &addr.sin_addr) <= 0) {
        Logger::error("invalid address");
        close(m_socket);
        return false;
    }
    if (bind(m_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        Logger::error("bind failed");
        close(m_socket);
        return false;
    }
    // 监听
    if (listen(m_socket, 20000) < 0) {
        Logger::error("listen failed");
        close(m_socket);
        return false;
    }
    // 初始化IO多路复用器
    if (!m_io->init()) {
        Logger::error("IO多路复用器初始化失败");
        close(m_socket);
        return false;
    }
    // 注册监听socket到IO多路复用器
    m_io->addfd(m_socket, IOMultiplexer::EventType::READ);
    Logger::info("[TcpServer] 服务器启动成功: " + m_ip + ":" + std::to_string(m_port));
    m_running = true;
    // 启动主循环（可用线程实现）
    std::thread([this]() { this->run(); }).detach();
    // 初始化心跳线程池（1线程，10秒检测一次）
    m_heartbeatPool = std::make_unique<HeartbeatThreadPool>(1, 10000);
    m_heartbeatPool->registerTask([this]() { this->checkHeartbeats(); });
    return true;
}

void TcpServer::run() {
    while (m_running) {
        std::vector<std::pair<int, IOMultiplexer::EventType>> activeEvents;
        int n = m_io->wait(activeEvents, 1000); // 1秒超时
        if (n < 0) {
            Logger::error("等待事件失败");
            continue;
        }
        for (auto& [fd, event] : activeEvents) {
            if (fd == m_socket) {
                // 新连接
                handleAccept();
            } else if (event & IOMultiplexer::EventType::READ) {
                // 客户端读事件
                handleRead(fd);
            } else if (event & IOMultiplexer::EventType::ERROR) {
                // 错误事件，关闭连接
                handleClose(fd);
            }
        }
    }
}

void TcpServer::stop() {
    m_running = false;
    close(m_socket);
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto &[fd, _] : m_clients) {
        m_io->removefd(fd);
        close(fd);
    }
    m_clients.clear();
    Logger::info("[TcpServer] 服务器已停止");
}

void TcpServer::handleAccept() {
    sockaddr_in client_addr{};
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(m_socket, (sockaddr*)&client_addr, &len);
    if (client_fd < 0) {
        if (m_running) Logger::error("accept failed");
        return;
    }
    // 设置非阻塞
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_clients[client_fd] = 1;
        m_lastActive[client_fd] = std::chrono::steady_clock::now();
    }
    m_io->addfd(client_fd, IOMultiplexer::EventType::READ);
    if (m_onConnect) m_onConnect(client_fd);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
    Logger::info(std::string("[TcpServer] 客户端") + std::to_string(client_fd) + "连接成功（IP:" + ip + "）");
}

void TcpServer::sendHeartbeat(int client_fd) {
    // 智能心跳控制：只有启用心跳时才发送
    if (m_heartbeatEnabled) {
        uint32_t magic = htonl(HEARTBEAT_MAGIC); // 网络字节序
        send(client_fd, &magic, sizeof(magic), 0);
    }
}

void TcpServer::checkHeartbeats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::steady_clock::now();
    for (auto it = m_clients.begin(); it != m_clients.end(); ) {
        int fd = it->first;
        auto last = m_lastActive.find(fd);
        if (last == m_lastActive.end() || std::chrono::duration_cast<std::chrono::seconds>(now - last->second).count() > m_heartbeatTimeout) {
            Logger::info("[Heartbeat] 客户端" + std::to_string(fd) + "心跳超时，关闭连接");
            m_io->removefd(fd);
            close(fd);
            it = m_clients.erase(it);
            m_lastActive.erase(fd);
            if (m_onClose) m_onClose(fd);
        } else {
            sendHeartbeat(fd);
            ++it;
        }
    }
}

void TcpServer::handleRead(int client_fd) {
    std::vector<char> buffer(BUFFER_SIZE);
    int bytes_received = recv(client_fd, buffer.data(), buffer.size(), 0);
        if (bytes_received > 0) {
        m_lastActive[client_fd] = std::chrono::steady_clock::now();
        if (bytes_received == sizeof(uint32_t)) {
            uint32_t magic_recv = 0;
            memcpy(&magic_recv, buffer.data(), sizeof(magic_recv));
            magic_recv = ntohl(magic_recv);
            if (magic_recv == HEARTBEAT_MAGIC) {
                // 收到心跳包，不处理业务
                return;
            }
        }
        std::string data(buffer.data(), bytes_received);
        if (m_onMessage) m_onMessage(client_fd, data);
        } else if (bytes_received == 0) {
            handleClose(client_fd);
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                handleClose(client_fd);
            }
        }
}

void TcpServer::handleClose(int client_fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_clients.find(client_fd) != m_clients.end()) {
        m_io->removefd(client_fd);
        close(client_fd);
        m_clients.erase(client_fd);
        m_lastActive.erase(client_fd);
        if (m_onClose) m_onClose(client_fd);
        Logger::info(std::string("[TcpServer] 客户端") + std::to_string(client_fd) + "断开连接");
    }
}

void TcpServer::onDataReceived(int, const char*, size_t) {
    // 默认空实现
}

void TcpServer::onClientConnected(int) {
    // 默认空实现
}

void TcpServer::onClientDisconnected(int) {
    // 默认空实现
} 