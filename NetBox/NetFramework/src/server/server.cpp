#include "server/server.h"


EchoServer::EchoServer(std::string ip, int port, IOMultiplexer::IOType io_type) 
    : m_ip(ip), m_port(port), m_running(false), m_socket(-1) {
        m_io = IOFactory::createIO(io_type);
        if (!m_io) {
        std::cerr << "创建IO多路复用器失败\n";
        exit(1);
    }
    }

EchoServer::~EchoServer() {
    stop();
}

bool EchoServer::start() {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
        perror("socket creation failed");
        return false;
    }

    int opt = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(m_socket);
        return false;
    }


    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    if (inet_pton(AF_INET, m_ip.c_str(), &addr.sin_addr) <= 0) {
        perror("invalid address");
        close(m_socket);
        return false;
    }

    if (bind(m_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        close(m_socket);
        return false;
    }

    // 监听连接
    if (listen(m_socket, 5) < 0) {
        perror("listen failed");
        close(m_socket);
        return false;
    }

    if (!m_io->init())
    {
        perror("IO多路复用器初始化失败");
        close(m_socket);
        return false;
    }
    m_io->addfd(m_socket,IOMultiplexer::EventType::READ);
    

    std::cout << "服务器启动成功: " << m_ip << " " <<"端口为:"<< m_port << 
    (m_io->type() == IOMultiplexer::IOType::EPOLL ? "epoll" : "select")<<"\n";
    m_running = true;
    run();
    return true;
}

void EchoServer::run() {
    while (m_running) {
        std::vector<std::pair<int, IOMultiplexer::EventType>> activeEvents;
        int n = m_io->wait(activeEvents, 1000);
        if (n < 0) {
            perror("等待事件失败");
            continue;
        }
        for (auto& [fd, event] : activeEvents) {
            if (fd == m_socket) {  // 监听socket的事件（新连接）
                handleAccept();
            } else if (event & IOMultiplexer::EventType::READ) {  // 客户端读事件（有数据）
                handleRead(fd);
            } else if (event & IOMultiplexer::EventType::WRITE) {  // 客户端写事件（可发送）
                handleWrite(fd);  // 简化处理，实际可缓存未发送数据
            }
        }
    } 
        
}

void EchoServer::handleAccept() {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(m_socket, (sockaddr*)&client_addr, &len);
        
        if (client_fd < 0) {
            if (m_running) perror("accept failed");
            return;
        }
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        std::lock_guard<std::mutex> lock(m_mutex);

        m_clients[client_fd] = 1;  // 记录客户端fd
        m_io->addfd(client_fd, IOMultiplexer::EventType::READ);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        std::cout << "[日志] 客户端" << client_fd << "连接成功（IP:" << ip << "），已添加到IO多路复用器\n";
    }

void EchoServer::handleRead(int client_fd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_clients.find(client_fd) == m_clients.end()) {
        return;  // 客户端已断开，忽略事件
    }
    // 创建新线程处理该客户端的读写（ detach() 分离线程，主线程继续处理其他事件）
    std::thread(&EchoServer::clientHandler, this, client_fd).detach();
}

void EchoServer::handleWrite(int client_fd) {
    std::cout << "客户端" << client_fd << "可发送数据\n";
}

void EchoServer::clientHandler(int client_fd) {
    m_current_concurrent++;
    auto start = std::chrono::high_resolution_clock::now();

    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {  // 收到数据，直接回显
        std::cout << "客户端" << client_fd << "发送: " << std::string(buffer, bytes_received) << "\n";
        std::cout << "[线程" << std::this_thread::get_id() << "] 客户端" << client_fd << "收到数据：" << std::string(buffer, bytes_received) << "\n";
        // 回显数据（直接发送，简化处理）
        int total_sent = 0;
        while (total_sent < bytes_received) {
            int sent = send(client_fd, buffer + total_sent, bytes_received - total_sent, 0);
            if (sent <= 0) {
                perror("发送数据失败");
                break;
            }
            total_sent += sent;
        }
    } else if (bytes_received == 0) {  // 客户端断开连接
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::cout << "客户端" << client_fd << "断开连接\n";
        m_io->removefd(client_fd);  // 从IO多路复用器中移除
        close(client_fd);
        m_clients.erase(client_fd);  // 从管理集合中删除
    } else {  // 读错误
        if (errno != EAGAIN && errno != EWOULDBLOCK) {  // 非阻塞正常错误
            perror("接收数据错误");
            std::lock_guard<std::mutex> lock(m_mutex);
            m_io->removefd(client_fd);
            close(client_fd);
            m_clients.erase(client_fd);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    int64_t duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    m_current_concurrent--;

    std::lock_guard<std::mutex> lock(m_mutex);  
    m_stats.update(duration_us, m_current_concurrent.load());  
}

void EchoServer::stop() {
    m_running = false;
    close(m_socket);
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto &[fd, _] :m_clients)
    {
        m_io->removefd(fd);
        close(fd);
        m_clients.clear();
        std::cout << "服务器已停止\n";
    }
    std::cout << "\n===== 服务器性能统计 =====" << std::endl;
    m_stats.print();

    std::cout << "服务器已停止" << std::endl;
}

