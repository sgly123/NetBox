#include "../include/Tcp_Server.h"

#include <string.h>


Tcp_Server::Tcp_Server(int port) 
: port_(port),socket_fd(-1) {};

int Tcp_Server::get_socket() {
    return socket_fd;
}

bool Tcp_Server::start() {
    socket_fd = sockets();
        if (socket_fd < 0) {
            std::cerr << "创建套接字失败" << std::endl;
            return false;
        }
        int opt = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt 失败"); // 输出错误原因
            close(socket_fd);
            return false;
    }
    if (binds() < 0) {
            std::cerr << "绑定失败" << std::endl;
            return false;
        }
    if (listens() < 0) {
        std::cerr << "监听失败" << std::endl;
            return false;
    }
    std::cout << "监听成功,端口为:"<<port_<<"\n";
    return true;
}

int Tcp_Server::sockets() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    return socket_fd;
}

int Tcp_Server::binds() {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    int bind_fd = bind(socket_fd,(sockaddr*)&addr,sizeof(addr));
    if (bind_fd < 0) {
         std::cerr << "绑定失败" <<"\n";
    }
    return bind_fd;
}

int Tcp_Server::listens() {
    int listen_fd = listen(socket_fd,SOMAXCONN);
    if (listen_fd < 0)
    {
        std::cerr << "监听失败"<<"\n";
        close(socket_fd);
    }
    return listen_fd;
}

int Tcp_Server::accept_client() {
    sockaddr_in client_addr {};
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(socket_fd, (sockaddr*)&client_addr, &client_addr_len);
    if (client_fd < 0)
    {
        perror("accept 失败");
        return -1;
    }
    return client_fd;
}

void Tcp_Server::handle_client(int client_fd) {
    while (true) {
        // 使用协议接收数据
        ProtocolMessage received_msg;

        if (!receive_message(client_fd, received_msg)) {
            // std::cerr << "接收消息失败或连接关闭" << std::endl;
            continue;
        }

        try {
            // 解析协议消息
            auto messages = received_msg.get_strings();
            for (const auto& msg : messages) {
                std::cout << "收到客户端消息: " << msg << std::endl;
            }
            
            // 创建响应消息
            ProtocolMessage response;
            response.add_string("服务器收到你的消息");
            response.add_string("消息数量: " + std::to_string(messages.size()));
            
            // 使用协议发送响应
            if (!send_message(client_fd, response)) {
                std::cerr << "发送响应失败" << std::endl;
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "协议解析错误: " << e.what() << std::endl;
            break;
        }
    }
    close_client(client_fd);
}

ssize_t Tcp_Server::send_data(int client_fd, char* buf, size_t buf_len) {
     ssize_t  sends = send(client_fd, buf, buf_len, 0);
     return sends;
}


ssize_t Tcp_Server::recv_data(int client_fd, char* buf, size_t buf_len) {
        size_t total_received = 0;
        while (total_received < buf_len) {
            ssize_t n = recv(client_fd, buf + total_received, buf_len - total_received, 0);
            if (n <= 0) {
                return n; // 错误或连接关闭
            }
            total_received += n;
        }
        return total_received;
    }

// 使用协议发送消息
    bool Tcp_Server::send_message(int client_fd, const ProtocolMessage& msg) {
        auto data = msg.serialize();
        return send_data(client_fd, data.data(), data.size()) == static_cast<ssize_t>(data.size());
    }
    
    // 使用协议接收消息
    bool Tcp_Server::receive_message(int client_fd, ProtocolMessage& out_msg) {
        // 第一步：接收消息头
        ProtocolHeader header;
        char header_buf[sizeof(ProtocolHeader)];
        
        ssize_t header_bytes = recv_data(client_fd, header_buf, sizeof(header_buf));
        if (header_bytes != sizeof(header_buf)) {
            return false;
        }
        
        if (!ProtocolHeader::deserialize(header_buf, header_bytes, header)) {
            return false;
        }
        
        // 第二步：接收消息体
        const size_t total_size = sizeof(ProtocolHeader) + header.body_length;
        std::vector<char> full_msg(total_size);
        
        // 拷贝已接收的头部
        std::memcpy(full_msg.data(), header_buf, sizeof(header_buf));
        
        // 接收剩余部分
        char* body_start = full_msg.data() + sizeof(header_buf);
        ssize_t body_bytes = recv_data(client_fd, body_start, header.body_length);
        if (body_bytes != static_cast<ssize_t>(header.body_length)) {
            return false;
        }
        
        // 反序列化完整消息
        return ProtocolMessage::deserialize(full_msg.data(), full_msg.size(), out_msg);
    }



void Tcp_Server::close_client(int client_fd) {
    close(client_fd);
}