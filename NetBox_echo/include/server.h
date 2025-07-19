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

class EchoServer {
public:
    EchoServer(std::string ip, int port);
    ~EchoServer();

    bool start();
    void stop();

private:
    int m_socket;
    int m_port;
    std::string m_ip;
    std::atomic<bool> m_running;
    std::vector<std::thread> client_threads;    
    static constexpr size_t BUFFER_SIZE = 4096;

    void run();
    void handle_client(int client_fd);
};

#endif // SERVER_H