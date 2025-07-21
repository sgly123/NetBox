#include "../include/server.h"


EchoServer::EchoServer(std::string ip, int port) 
    : m_ip(ip), m_port(port), m_running(false), m_socket(-1) {}

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

    std::cout << "服务器启动成功: " << m_ip << ":" << m_port << "\n";
    m_running = true;
    run();
    return true;
}

void EchoServer::run() {
    while (m_running) {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(m_socket, (sockaddr*)&client_addr, &len);
        
        if (client_fd < 0) {
            if (m_running) perror("accept failed");
            continue;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        std::cout << "新客户端: " << ip << ":" << ntohs(client_addr.sin_port) << "\n";

            this->handle_client(client_fd);
            close(client_fd);  
    }
}

void EchoServer::handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    
    while (m_running) {
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received > 0) {
            for(int i=0; i<bytes_received; ++i) {
                std::cout << buffer[i];
            }
            std::cout << "\n";

            int total_sent = 0;
            while(total_sent < bytes_received) {
                int sent = send(client_fd, buffer + total_sent, 
                                bytes_received - total_sent, 0);
                if(sent <= 0) {
                    perror("发送失败");
                    break;
                }
                total_sent += sent;
            }
        } 
        else if (bytes_received == 0) {
            std::cout << "客户端断开连接" << std::endl;
            break;  
        } 
        else {
            if(errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recv错误");
                break;
            }
        }
    }
}

void EchoServer::stop() {
    m_running = false;
    close(m_socket);
    
}