// #include "../include/Tcp_Server.h"
#include "../../base_tool/include/EpollManger.h"
#include <iostream>
#include <thread>

// 测试服务端：接收数据并打印
void test_server() {
    Tcp_Server server(8888);
    if (!server.start()) {
        std::cerr << "服务端启动失败" << std::endl;
        return;
    }
    std::cout << "服务端运行中，等待客户端连接..." << std::endl;
    int socket_fd = server.get_socket();
    EpollManger(socket_fd, server);

}

int main() {
    // 启动服务端（后台线程）
    test_server();
    return 0;
}