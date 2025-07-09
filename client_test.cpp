#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <fcntl.h>
#include "protocol.h"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>

// 非阻塞读取辅助函数
bool inputAvailable() {
    struct pollfd fds = {.fd = STDIN_FILENO, .events = POLLIN};
    return poll(&fds, 1, 0) > 0;
}

// 设置socket非阻塞
void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) return;
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

// 封装消息发送函数
void send_protocol_message(int sock, const std::string& content) {
    ProtocolMessage msg;
    msg.add_string(content);
    auto data = msg.serialize();
        if (send(sock, data.data(), data.size(), 0) == -1) {
            perror("send failed for second message");
        } else {
            std::cout << "Sent message successed\n";
        }
}

// 读取服务器响应
void receive_responses(int sock) {
    static std::vector<char> buffer; // 静态缓冲区保存接收到的数据
    
    // 尝试接收数据
    char temp_buf[1024];
    ProtocolMessage received_msg;
    ssize_t n = recv(sock, temp_buf, sizeof(temp_buf), 0);
    auto msgs = received_msg.get_strings();
    for (const auto& msg : msgs)
    {
        std::cout << "收到客户端消息: " << msg << std::endl;
    }
    if (n > 0) {
        buffer.insert(buffer.end(), temp_buf, temp_buf + n);
        std::cout << "收到数据块: " << n << "字节 (总缓冲: " << buffer.size() << "字节)" << std::endl;
    } else if (n == 0) {
        std::cout << "服务器已关闭连接" << std::endl;
        close(sock);
        exit(0);
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("接收错误");
        close(sock);
        exit(1);
    }
    
    // 尝试解析完整消息
    while (buffer.size() >= sizeof(uint32_t)) {
        uint32_t msg_size;
        memcpy(&msg_size, buffer.data(), sizeof(msg_size));
        msg_size = ntohl(msg_size);
        
        if (buffer.size() < sizeof(uint32_t) + msg_size) {
            // 消息还不完整
            break;
        }
        
        // 提取完整消息
        std::vector<char> msg_data(buffer.begin() + sizeof(uint32_t), 
                                  buffer.begin() + sizeof(uint32_t) + msg_size);
        
        // TODO: 这里应添加ProtocolMessage的反序列化逻辑
        // 示例中直接作为字符串输出
        std::string message(msg_data.begin(), msg_data.end());
        std::cout << "服务器响应: " << message << std::endl;
        
        // 移除已处理的消息
        buffer.erase(buffer.begin(), buffer.begin() + sizeof(uint32_t) + msg_size);
    }
}

int main() {
    // 创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket创建失败");
        return 1;
    }

    // 设置服务器地址
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    
    // 将点分十进制转换为网络字节序的二进制地址
    if (inet_pton(AF_INET, "192.168.88.133", &server_addr.sin_addr) <= 0) {
        perror("地址转换失败");
        close(sock);
        return 1;
    }

    // 连接服务器
    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("连接服务器失败");
        close(sock);
        return 1;
    }
    std::cout << "成功连接到服务器!" << std::endl;
    
    // 设置非阻塞模式
    set_nonblocking(sock);
    
    // 初始化消息
    send_protocol_message(sock, "Hello from client!");
    send_protocol_message(sock, "第二条消息");
    send_protocol_message(sock, "田文镜");
    send_protocol_message(sock, "第三条消息");
    
    std::string input;
    std::cout << "输入消息 (输入'exit'退出): ";

    while (true) {
        // 检查是否有控制台输入
        if (inputAvailable()) {
            std::getline(std::cin, input);
            
            if (input == "exit") {
                std::cout << "退出程序..." << std::endl;
                break;
            }
            
            if (!input.empty()) {
                send_protocol_message(sock, input);
            }
            
            std::cout << "输入消息 (输入'exit'退出): ";
        }
        
        // 接收和处理服务器响应
        receive_responses(sock);
        
        // 短暂休眠避免CPU过载
        usleep(100000); // 100ms
    }
    
    close(sock);
    return 0;
}