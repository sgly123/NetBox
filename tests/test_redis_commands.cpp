#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class RedisCommandTester {
private:
    int sockfd;
    
public:
    bool connect(const std::string& host, int port) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "创建socket失败" << std::endl;
            return false;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(host.c_str());
        
        if (::connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "连接服务器失败" << std::endl;
            ::close(sockfd);
            return false;
        }
        
        std::cout << "成功连接到Redis服务器 " << host << ":" << port << std::endl;
        return true;
    }
    
    std::string sendCommand(const std::string& command) {
        // 发送命令
        ssize_t sent = send(sockfd, command.c_str(), command.length(), 0);
        if (sent < 0) {
            std::cerr << "发送命令失败" << std::endl;
            return "";
        }
        
        // 接收响应
        char buffer[1024];
        ssize_t received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (received < 0) {
            std::cerr << "接收响应失败" << std::endl;
            return "";
        }
        
        buffer[received] = '\0';
        return std::string(buffer, received);
    }
    
    void testSetCommand() {
        std::cout << "\n=== 测试SET命令 ===" << std::endl;
        
        // 测试1: SET name "test_value"
        std::string command1 = "SET name \"test_value\"\r\n";
        std::cout << "发送: " << command1;
        std::string response1 = sendCommand(command1);
        std::cout << "接收: " << response1;
        
        // 测试2: SET age 25
        std::string command2 = "SET age 25\r\n";
        std::cout << "发送: " << command2;
        std::string response2 = sendCommand(command2);
        std::cout << "接收: " << response2;
    }
    
    void testGetCommand() {
        std::cout << "\n=== 测试GET命令 ===" << std::endl;
        
        // 测试1: GET name
        std::string command1 = "GET name\r\n";
        std::cout << "发送: " << command1;
        std::string response1 = sendCommand(command1);
        std::cout << "接收: " << response1;
        
        // 测试2: GET age
        std::string command2 = "GET age\r\n";
        std::cout << "发送: " << command2;
        std::string response2 = sendCommand(command2);
        std::cout << "接收: " << response2;
        
        // 测试3: GET nonexistent
        std::string command3 = "GET nonexistent\r\n";
        std::cout << "发送: " << command3;
        std::string response3 = sendCommand(command3);
        std::cout << "接收: " << response3;
    }
    
    void testPingCommand() {
        std::cout << "\n=== 测试PING命令 ===" << std::endl;
        
        std::string command = "PING\r\n";
        std::cout << "发送: " << command;
        std::string response = sendCommand(command);
        std::cout << "接收: " << response;
    }
    
    void close() {
        if (sockfd >= 0) {
            ::close(sockfd);
        }
    }
    
    ~RedisCommandTester() {
        close();
    }
};

int main() {
    RedisCommandTester tester;
    
    if (!tester.connect("192.168.88.135", 6379)) {
        std::cerr << "连接失败，退出测试" << std::endl;
        return 1;
    }
    
    // 测试各种命令
    tester.testPingCommand();
    tester.testSetCommand();
    tester.testGetCommand();
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
} 