#include "SimpleHeaderProtocol.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstring>

/**
 * @brief 修复版Echo客户端
 * 
 * 修复内容：
 * 1. 简化协议处理逻辑
 * 2. 更好的错误处理
 * 3. 清晰的数据流程
 * 4. 支持命令行参数
 */

class EchoClient {
private:
    int m_socket;
    std::string m_host;
    int m_port;
    bool m_connected;
    
public:
    EchoClient(const std::string& host = "192.168.88.135", int port = 8888) 
        : m_socket(-1), m_host(host), m_port(port), m_connected(false) {
    }
    
    ~EchoClient() {
        disconnect();
    }
    
    bool connect() {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0) {
            std::cerr << "❌ 创建socket失败" << std::endl;
            return false;
        }
        
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(m_port);
        
        if (inet_pton(AF_INET, m_host.c_str(), &addr.sin_addr) <= 0) {
            std::cerr << "❌ 无效的IP地址: " << m_host << std::endl;
            close(m_socket);
            return false;
        }
        
        if (::connect(m_socket, (sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "❌ 连接服务器失败: " << m_host << ":" << m_port << std::endl;
            close(m_socket);
            return false;
        }
        
        m_connected = true;
        std::cout << "✅ 已连接到Echo服务器: " << m_host << ":" << m_port << std::endl;
        return true;
    }
    
    void disconnect() {
        if (m_socket >= 0) {
            close(m_socket);
            m_socket = -1;
        }
        m_connected = false;
    }
    
    bool sendMessage(const std::string& message) {
        if (!m_connected) {
            std::cerr << "❌ 未连接到服务器" << std::endl;
            return false;
        }
        
        // 使用SimpleHeaderProtocol封包
        SimpleHeaderProtocol protocol;
        std::vector<char> packet;
        
        if (!protocol.pack(message.c_str(), message.length(), packet)) {
            std::cerr << "❌ 协议封包失败" << std::endl;
            return false;
        }
        
        // 添加协议路由头（4字节协议ID，大端序）
        std::vector<char> routedPacket = {0x00, 0x00, 0x00, 0x01}; // 协议ID=1
        routedPacket.insert(routedPacket.end(), packet.begin(), packet.end());
        
        // 发送数据
        ssize_t sent = send(m_socket, routedPacket.data(), routedPacket.size(), 0);
        if (sent < 0) {
            std::cerr << "❌ 发送数据失败" << std::endl;
            return false;
        }
        
        std::cout << "📤 已发送: " << message << " (" << sent << " 字节)" << std::endl;
        return true;
    }
    
    std::string receiveMessage() {
        if (!m_connected) {
            return "❌ 未连接到服务器";
        }
        
        char buffer[4096];
        ssize_t received = recv(m_socket, buffer, sizeof(buffer), 0);
        
        if (received <= 0) {
            if (received == 0) {
                std::cout << "🔌 服务器关闭连接" << std::endl;
            } else {
                std::cerr << "❌ 接收数据失败" << std::endl;
            }
            m_connected = false;
            return "";
        }
        
        std::cout << "📥 接收到: " << received << " 字节" << std::endl;
        
        // 跳过协议路由头（4字节）
        if (received <= 4) {
            std::cerr << "❌ 数据包太短" << std::endl;
            return "";
        }
        
        // 解析协议包
        SimpleHeaderProtocol protocol;
        std::string result;
        
        // 设置回调来接收解析结果
        protocol.setPacketCallback([&result](const std::vector<char>& packet) {
            result = std::string(packet.begin(), packet.end());
        });
        
        // 处理协议数据（跳过4字节路由头）
        size_t processed = protocol.onDataReceived(buffer + 4, received - 4);
        
        if (processed > 0 && !result.empty()) {
            std::cout << "📋 解析成功: " << result << std::endl;
            return result;
        } else {
            std::cerr << "❌ 协议解析失败" << std::endl;
            return "";
        }
    }
    
    void runInteractive() {
        std::cout << "\n🎮 Echo客户端交互模式" << std::endl;
        std::cout << "输入消息发送到服务器，输入'quit'退出" << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        std::string input;
        while (true) {
            std::cout << "\necho> ";
            std::getline(std::cin, input);
            
            if (input == "quit" || input == "exit") {
                std::cout << "👋 再见！" << std::endl;
                break;
            }
            
            if (input.empty()) {
                continue;
            }
            
            // 发送消息
            if (sendMessage(input)) {
                // 接收回显
                std::string echo = receiveMessage();
                if (!echo.empty()) {
                    std::cout << "🔄 回显: " << echo << std::endl;
                }
            }
            
            if (!m_connected) {
                std::cout << "❌ 连接已断开，退出程序" << std::endl;
                break;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    std::string host = "192.168.88.135";
    int port = 8888;
    
    // 解析命令行参数
    if (argc >= 2) {
        host = argv[1];
    }
    if (argc >= 3) {
        port = std::atoi(argv[2]);
    }
    
    std::cout << "🚀 Echo客户端启动" << std::endl;
    std::cout << "目标服务器: " << host << ":" << port << std::endl;
    
    EchoClient client(host, port);
    
    if (!client.connect()) {
        return 1;
    }
    
    client.runInteractive();
    
    return 0;
}
