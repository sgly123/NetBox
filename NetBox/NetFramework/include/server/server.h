#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <atomic>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <vector>
#include <thread>
#include <mutex>
#include "../IO/IOFactory.h"
#include <unordered_map>



class EchoServer {
public:
    EchoServer(std::string ip, int port, IOMultiplexer::IOType io_type);
    ~EchoServer();

    IOMultiplexer::IOType type() const;
    bool start();
    void run();
    void handleAccept();  // 处理新连接
    void handleRead(int client_fd);  // 处理客户端读事件（接收数据）
    void handleWrite(int client_fd);  // 处理客户端写事件（可选，发送数据）
    void stop();
    void clientHandler(int client_fd);

private:
    int m_socket;
    int m_port;
    std::string m_ip;
    std::atomic<bool> m_running;
    std::unique_ptr<IOMultiplexer> m_io;
    std::unordered_map<int, int> m_clients;   
    const int BUFFER_SIZE = 4096;
    std::mutex m_mutex;
    IOFactory::PerformanceStats m_stats;
    std::atomic<int> m_current_concurrent{0};
};

#endif // SERVER_H