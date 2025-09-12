#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

/**
 * @brief 简单的UDP客户端测试工具
 * 用于测试UDP Echo服务器的功能
 */
class UdpClient {
public:
    UdpClient(const std::string& server_ip, int server_port) 
        : m_serverIP(server_ip), m_serverPort(server_port), m_socket(INVALID_SOCKET) {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
    }

    ~UdpClient() {
        close();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool connect() {
        m_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_socket == INVALID_SOCKET) {
            std::cerr << "创建socket失败" << std::endl;
            return false;
        }

        // 设置服务器地址
        memset(&m_serverAddr, 0, sizeof(m_serverAddr));
        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port = htons(static_cast<uint16_t>(m_serverPort));
        
        if (inet_pton(AF_INET, m_serverIP.c_str(), &m_serverAddr.sin_addr) <= 0) {
            std::cerr << "无效的服务器地址: " << m_serverIP << std::endl;
            return false;
        }

        std::cout << "UDP客户端连接到 " << m_serverIP << ":" << m_serverPort << std::endl;
        return true;
    }

    void close() {
        if (m_socket != INVALID_SOCKET) {
            CLOSE_SOCKET(m_socket);
            m_socket = INVALID_SOCKET;
        }
    }

    bool sendMessage(const std::string& message) {
        ssize_t sent = sendto(m_socket, message.c_str(), message.size(), 0,
                             reinterpret_cast<const struct sockaddr*>(&m_serverAddr), sizeof(m_serverAddr));
        
        if (sent == SOCKET_ERROR) {
            std::cerr << "发送消息失败" << std::endl;
            return false;
        }

        std::cout << "发送: " << message << " (" << sent << " bytes)" << std::endl;
        return true;
    }

    std::string receiveMessage(int timeout_ms = 5000) {
        char buffer[1024];
        sockaddr_in from;
        socklen_t fromLen = sizeof(from);

        // 设置接收超时
#ifdef _WIN32
        DWORD timeout = timeout_ms;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

        ssize_t received = recvfrom(m_socket, buffer, sizeof(buffer) - 1, 0,
                                   reinterpret_cast<struct sockaddr*>(&from), &fromLen);
        
        if (received == SOCKET_ERROR) {
            std::cerr << "接收消息超时或失败" << std::endl;
            return "";
        }

        buffer[received] = '\0';
        std::string response(buffer, received);
        std::cout << "接收: " << response << " (" << received << " bytes)" << std::endl;
        return response;
    }

    void runInteractiveTest() {
        std::cout << "\n=== UDP Echo客户端交互测试 ===" << std::endl;
        std::cout << "输入消息发送给服务器，输入 'quit' 退出" << std::endl;
        
        std::string input;
        while (true) {
            std::cout << "\n请输入消息: ";
            std::getline(std::cin, input);
            
            if (input == "quit" || input == "exit") {
                std::cout << "退出客户端..." << std::endl;
                break;
            }
            
            if (input.empty()) {
                continue;
            }
            
            if (sendMessage(input)) {
                receiveMessage();
            }
        }
    }

    void runPerformanceTest(int message_count = 1000) {
        std::cout << "\n=== UDP Echo性能测试 ===" << std::endl;
        std::cout << "发送 " << message_count << " 条消息..." << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        int success_count = 0;
        
        for (int i = 0; i < message_count; ++i) {
            std::string message = "Test message " + std::to_string(i + 1);
            
            if (sendMessage(message)) {
                std::string response = receiveMessage(1000); // 1秒超时
                if (!response.empty() && response.find(message) != std::string::npos) {
                    success_count++;
                }
            }
            
            // 每100条消息打印一次进度
            if ((i + 1) % 100 == 0) {
                std::cout << "已处理: " << (i + 1) << "/" << message_count << std::endl;
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "\n=== 性能测试结果 ===" << std::endl;
        std::cout << "总发送数: " << message_count << std::endl;
        std::cout << "成功数: " << success_count << std::endl;
        std::cout << "成功率: " << (double)success_count / message_count * 100 << "%" << std::endl;
        std::cout << "总耗时: " << duration.count() << " ms" << std::endl;
        std::cout << "平均延迟: " << (double)duration.count() / success_count << " ms" << std::endl;
        std::cout << "QPS: " << (double)success_count * 1000 / duration.count() << std::endl;
    }

private:
    std::string m_serverIP;
    int m_serverPort;
    int m_socket;
    sockaddr_in m_serverAddr;
};

int main(int argc, char* argv[]) {
    std::string server_ip = "127.0.0.1";
    int server_port = 8081;
    
    // 解析命令行参数
    if (argc >= 2) {
        server_ip = argv[1];
    }
    if (argc >= 3) {
        server_port = std::stoi(argv[2]);
    }
    
    try {
        UdpClient client(server_ip, server_port);
        
        if (!client.connect()) {
            std::cerr << "连接服务器失败" << std::endl;
            return 1;
        }
        
        std::cout << "UDP客户端测试工具" << std::endl;
        std::cout << "服务器: " << server_ip << ":" << server_port << std::endl;
        std::cout << "\n选择测试模式:" << std::endl;
        std::cout << "1. 交互测试模式" << std::endl;
        std::cout << "2. 性能测试模式" << std::endl;
        std::cout << "请选择 (1-2): ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // 忽略换行符
        
        switch (choice) {
            case 1:
                client.runInteractiveTest();
                break;
            case 2:
                std::cout << "输入测试消息数量 (默认1000): ";
                std::string input;
                std::getline(std::cin, input);
                int count = input.empty() ? 1000 : std::stoi(input);
                client.runPerformanceTest(count);
                break;
            default:
                std::cout << "无效选择，使用交互模式" << std::endl;
                client.runInteractiveTest();
                break;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 