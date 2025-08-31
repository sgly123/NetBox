#include "SimpleHeaderProtocol.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstring>

/**
 * @brief Echo客户端主程序
 * 
 * 功能说明：
 * 1. 连接到Echo服务器
 * 2. 发送用户输入的消息
 * 3. 接收服务器的回显响应
 * 4. 显示回显内容
 * 
 * 协议流程：
 * 1. 建立TCP连接
 * 2. 使用SimpleHeaderProtocol封包用户数据
 * 3. 添加协议路由头（4字节协议ID）
 * 4. 发送数据到服务器
 * 5. 接收服务器响应
 * 6. 解析并显示回显内容
 * 
 * 调试功能：
 * - 显示发送数据长度
 * - 显示接收数据长度
 * - 显示原始数据（十六进制）
 * - 显示协议处理过程
 */
int main() {
    // 创建TCP套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "创建套接字失败" << std::endl;
        return 1;
    }

    // 配置服务器地址
    sockaddr_in addr{};
    addr.sin_family = AF_INET;                    // IPv4协议族
    addr.sin_port = htons(8888);                  // 服务器端口（网络字节序）
    inet_pton(AF_INET, "192.168.88.135", &addr.sin_addr);  // 服务器IP地址

    // 连接到服务器
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "连接服务器失败" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "已连接到服务器: 192.168.88.135:8888" << std::endl;

    // 创建协议处理器实例
    SimpleHeaderProtocol proto;
    
    // 设置协议层回调：处理服务器回显数据
    proto.setPacketCallback([&](const std::vector<char>& packet) {
        std::string msg(packet.begin(), packet.end());
        std::cout << "收到回显: " << msg << std::endl;
    });
    
    // 设置协议层错误回调：处理协议错误
    proto.setErrorCallback([](const std::string& error) {
        std::cout << "协议错误: " << error << std::endl;
    });

    // 交互式消息发送循环
    std::string input;
    while (true) {
        std::cout << "请输入要发送的内容（exit退出）: ";
        std::getline(std::cin, input);
        
        // 检查退出命令
        if (input == "exit") break;

        // 使用协议层封包用户数据
        // 协议层会自动添加4字节包头（包体长度）
        std::vector<char> sendbuf;
        if (!proto.pack(input.data(), input.size(), sendbuf)) {
            std::cout << "协议封包失败" << std::endl;
            continue;
        }
        
        // 添加协议路由头（4字节协议ID，大端序）
        // 协议ID = 1，对应SimpleHeaderProtocol
        std::vector<char> routedPacket = {0x00, 0x00, 0x00, 0x01};
        routedPacket.insert(routedPacket.end(), sendbuf.begin(), sendbuf.end());
        
        // 发送数据到服务器
        send(sock, routedPacket.data(), routedPacket.size(), 0);
        std::cout << "已发送数据，长度: " << routedPacket.size() << std::endl;

        // 接收服务器响应
        char buf[4096];
        ssize_t n = recv(sock, buf, sizeof(buf), 0);
        std::cout << "接收到服务器响应，长度: " << n << std::endl;
        
        if (n <= 0) {
            std::cout << "服务器连接已断开" << std::endl;
            break;
        }
        
        // 处理回显数据
        // 跳过前4字节协议头，将剩余数据交给协议层处理
        if (n > 4) {
            std::cout << "处理回显数据，跳过4字节协议头" << std::endl;
            proto.onDataReceived(buf + 4, n - 4);
        } else {
            std::cout << "接收到的数据太短，无法处理" << std::endl;
        }
    }

    // 关闭连接
    close(sock);
    std::cout << "客户端已退出" << std::endl;
    return 0;
}