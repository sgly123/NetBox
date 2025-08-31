/**
 * @file stress_test_echo.cpp
 * @brief Echo服务器压力测试启动程序
 * 
 * 用于压力测试的简化echo服务器
 * 专门为性能测试设计，移除不必要的日志和功能
 */

#include "server.h"
#include "base/Logger.h"
#include "base/ThreadPool.h"
#include "IO/epoll_multiplexer.h"
#include <iostream>
#include <signal.h>
#include <memory>

static std::unique_ptr<EchoServer> g_server = nullptr;

void signalHandler(int signal) {
    std::cout << "\n收到信号: " << signal << ", 正在停止服务器..." << std::endl;
    if (g_server) {
        g_server->stop();
        g_server.reset();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // 解析命令行参数
    std::string ip = "127.0.0.1";
    int port = 8888;
    int thread_count = 4;
    
    if (argc >= 2) ip = argv[1];
    if (argc >= 3) port = std::atoi(argv[2]);
    if (argc >= 4) thread_count = std::atoi(argv[3]);
    
    std::cout << "启动Echo服务器用于压力测试" << std::endl;
    std::cout << "监听地址: " << ip << ":" << port << std::endl;
    std::cout << "工作线程: " << thread_count << std::endl;
    
    // 配置日志级别为ERROR，减少性能影响
    Logger::setLevel(Logger::Level::ERROR);
    
    try {
        // 创建线程池
        auto threadPool = std::make_unique<ThreadPool>(thread_count);
        
        // 创建Echo服务器（使用EPOLL获得最佳性能）
        g_server = std::make_unique<EchoServer>(ip, port, IOMultiplexer::IOType::EPOLL, threadPool.get());
        
        // 启动服务器
        if (!g_server->start()) {
            std::cerr << "服务器启动失败!" << std::endl;
            return 1;
        }
        
        std::cout << "✅ Echo服务器启动成功，等待客户端连接..." << std::endl;
        std::cout << "按 Ctrl+C 停止服务器" << std::endl;
        
        // 保持主线程运行
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "服务器异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 