/**
 * @file benchmark_client.cpp
 * @brief 高性能Echo服务器压力测试客户端
 * 
 * 功能特性：
 * 1. 多线程并发连接
 * 2. QPS统计和报告
 * 3. 延迟分析（P50, P95, P99）
 * 4. 连接复用优化
 * 5. 支持不同测试场景
 */

#include "SimpleHeaderProtocol.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <random>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <sstream>

/**
 * @brief 性能统计信息
 */
struct BenchmarkStats {
    std::atomic<uint64_t> totalRequests{0};
    std::atomic<uint64_t> totalResponses{0};
    std::atomic<uint64_t> totalErrors{0};
    std::atomic<uint64_t> totalBytes{0};
    std::vector<double> latencies;
    std::mutex latencyMutex;
    
    void addLatency(double latency_ms) {
        std::lock_guard<std::mutex> lock(latencyMutex);
        latencies.push_back(latency_ms);
    }
    
    void printStats(double duration_seconds) {
        double qps = totalResponses.load() / duration_seconds;
        double throughput_mbps = (totalBytes.load() / (1024.0 * 1024.0)) / duration_seconds;
        
        std::cout << "\n==================== 性能测试报告 ====================" << std::endl;
        std::cout << "测试时长: " << std::fixed << std::setprecision(2) << duration_seconds << " 秒" << std::endl;
        std::cout << "总请求数: " << totalRequests.load() << std::endl;
        std::cout << "成功响应: " << totalResponses.load() << std::endl;
        std::cout << "失败请求: " << totalErrors.load() << std::endl;
        std::cout << "成功率: " << std::fixed << std::setprecision(2) 
                  << (100.0 * totalResponses.load() / totalRequests.load()) << "%" << std::endl;
        std::cout << "QPS: " << std::fixed << std::setprecision(0) << qps << " requests/sec" << std::endl;
        std::cout << "吞吐量: " << std::fixed << std::setprecision(2) << throughput_mbps << " MB/s" << std::endl;
        
        if (!latencies.empty()) {
            std::sort(latencies.begin(), latencies.end());
            size_t count = latencies.size();
            std::cout << "延迟统计:" << std::endl;
            std::cout << "  平均延迟: " << std::fixed << std::setprecision(2) 
                      << std::accumulate(latencies.begin(), latencies.end(), 0.0) / count << " ms" << std::endl;
            std::cout << "  P50延迟: " << std::fixed << std::setprecision(2) 
                      << latencies[count * 50 / 100] << " ms" << std::endl;
            std::cout << "  P95延迟: " << std::fixed << std::setprecision(2) 
                      << latencies[count * 95 / 100] << " ms" << std::endl;
            std::cout << "  P99延迟: " << std::fixed << std::setprecision(2) 
                      << latencies[count * 99 / 100] << " ms" << std::endl;
            std::cout << "  最大延迟: " << std::fixed << std::setprecision(2) 
                      << latencies[count - 1] << " ms" << std::endl;
        }
        std::cout << "=====================================================" << std::endl;
    }
};

/**
 * @brief 测试配置
 */
struct BenchmarkConfig {
    std::string serverIp = "127.0.0.1";
    int serverPort = 8888;
    int threadCount = 4;           // 并发线程数
    int connectionsPerThread = 10; // 每个线程的连接数
    int requestsPerConnection = 100; // 每个连接的请求数
    int messageSize = 64;          // 消息大小（字节）
    int testDuration = 60;         // 测试时长（秒），0表示不限制
    bool keepAlive = true;         // 是否保持连接
    bool randomData = false;       // 是否使用随机数据
    
    void print() {
        std::cout << "测试配置:" << std::endl;
        std::cout << "  服务器: " << serverIp << ":" << serverPort << std::endl;
        std::cout << "  并发线程: " << threadCount << std::endl;
        std::cout << "  每线程连接数: " << connectionsPerThread << std::endl;
        std::cout << "  每连接请求数: " << requestsPerConnection << std::endl;
        std::cout << "  消息大小: " << messageSize << " 字节" << std::endl;
        std::cout << "  测试时长: " << (testDuration > 0 ? std::to_string(testDuration) + "秒" : "无限制") << std::endl;
        std::cout << "  保持连接: " << (keepAlive ? "是" : "否") << std::endl;
        std::cout << "  随机数据: " << (randomData ? "是" : "否") << std::endl;
    }
};

/**
 * @brief 单个客户端连接类
 */
class BenchmarkClient {
private:
    int socket_;
    SimpleHeaderProtocol protocol_;
    BenchmarkStats* stats_;
    BenchmarkConfig config_;
    std::string testMessage_;
    std::atomic<bool>* running_;
    
    bool connectToServer() {
        socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_ < 0) return false;
        
        // 设置为非阻塞模式，但连接时暂时使用阻塞
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(config_.serverPort);
        inet_pton(AF_INET, config_.serverIp.c_str(), &addr.sin_addr);
        
        if (connect(socket_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            close(socket_);
            return false;
        }
        
        return true;
    }
    
    void setupProtocol() {
        protocol_.setPacketCallback([this](const std::vector<char>& packet) {
            stats_->totalResponses++;
            stats_->totalBytes += packet.size();
        });
        
        protocol_.setErrorCallback([this](const std::string& error) {
            stats_->totalErrors++;
        });
    }
    
public:
    BenchmarkClient(BenchmarkStats* stats, const BenchmarkConfig& config, std::atomic<bool>* running)
        : socket_(-1), stats_(stats), config_(config), running_(running) {
        
        // 生成测试消息
        if (config_.randomData) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis('A', 'Z');
            testMessage_.reserve(config_.messageSize);
            for (int i = 0; i < config_.messageSize; ++i) {
                testMessage_ += static_cast<char>(dis(gen));
            }
        } else {
            testMessage_ = std::string(config_.messageSize, 'A');
        }
        
        setupProtocol();
    }
    
    ~BenchmarkClient() {
        if (socket_ >= 0) {
            close(socket_);
        }
    }
    
    bool run() {
        if (!connectToServer()) {
            stats_->totalErrors++;
            return false;
        }
        
        int requestsSent = 0;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        while (running_->load() && 
               (config_.testDuration == 0 || 
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::high_resolution_clock::now() - startTime).count() < config_.testDuration) &&
               requestsSent < config_.requestsPerConnection) {
            
            if (!sendRequest()) {
                stats_->totalErrors++;
                break;
            }
            
            if (!receiveResponse()) {
                stats_->totalErrors++;
                break;
            }
            
            requestsSent++;
            
            if (!config_.keepAlive) {
                close(socket_);
                if (!connectToServer()) {
                    stats_->totalErrors++;
                    break;
                }
            }
        }
        
        return true;
    }
    
private:
    bool sendRequest() {
        auto sendStart = std::chrono::high_resolution_clock::now();
        
        // 使用协议封包
        std::vector<char> packetData;
        if (!protocol_.pack(testMessage_.data(), testMessage_.size(), packetData)) {
            return false;
        }
        
        // 添加协议路由头（4字节协议ID = 1）
        std::vector<char> routedPacket = {0x00, 0x00, 0x00, 0x01};
        routedPacket.insert(routedPacket.end(), packetData.begin(), packetData.end());
        
        ssize_t sent = send(socket_, routedPacket.data(), routedPacket.size(), 0);
        if (sent != static_cast<ssize_t>(routedPacket.size())) {
            return false;
        }
        
        stats_->totalRequests++;
        stats_->totalBytes += sent;
        
        return true;
    }
    
    bool receiveResponse() {
        char buffer[4096];
        ssize_t received = recv(socket_, buffer, sizeof(buffer), 0);
        
        if (received <= 0) return false;
        
        // 跳过协议路由头，处理响应数据
        if (received > 4) {
            protocol_.onDataReceived(buffer + 4, received - 4);
        }
        
        return true;
    }
};

/**
 * @brief 工作线程函数
 */
void workerThread(int threadId, BenchmarkStats* stats, const BenchmarkConfig& config, std::atomic<bool>* running) {
    std::vector<std::unique_ptr<BenchmarkClient>> clients;
    
    // 创建连接
    for (int i = 0; i < config.connectionsPerThread; ++i) {
        auto client = std::make_unique<BenchmarkClient>(stats, config, running);
        clients.push_back(std::move(client));
    }
    
    // 运行测试
    for (auto& client : clients) {
        if (!running->load()) break;
        client->run();
    }
}

void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -h <host>       服务器IP地址 (默认: 127.0.0.1)" << std::endl;
    std::cout << "  -p <port>       服务器端口 (默认: 8888)" << std::endl;
    std::cout << "  -t <threads>    并发线程数 (默认: 4)" << std::endl;
    std::cout << "  -c <conns>      每线程连接数 (默认: 10)" << std::endl;
    std::cout << "  -r <requests>   每连接请求数 (默认: 100)" << std::endl;
    std::cout << "  -s <size>       消息大小 (默认: 64)" << std::endl;
    std::cout << "  -d <duration>   测试时长秒数 (默认: 60, 0=无限制)" << std::endl;
    std::cout << "  --no-keepalive  不保持连接" << std::endl;
    std::cout << "  --random-data   使用随机数据" << std::endl;
    std::cout << "  --help          显示此帮助信息" << std::endl;
}

int main(int argc, char* argv[]) {
    BenchmarkConfig config;
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--no-keepalive") {
            config.keepAlive = false;
        } else if (arg == "--random-data") {
            config.randomData = true;
        } else if (arg == "-h" && i + 1 < argc) {
            config.serverIp = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            config.serverPort = std::atoi(argv[++i]);
        } else if (arg == "-t" && i + 1 < argc) {
            config.threadCount = std::atoi(argv[++i]);
        } else if (arg == "-c" && i + 1 < argc) {
            config.connectionsPerThread = std::atoi(argv[++i]);
        } else if (arg == "-r" && i + 1 < argc) {
            config.requestsPerConnection = std::atoi(argv[++i]);
        } else if (arg == "-s" && i + 1 < argc) {
            config.messageSize = std::atoi(argv[++i]);
        } else if (arg == "-d" && i + 1 < argc) {
            config.testDuration = std::atoi(argv[++i]);
        }
    }
    
    config.print();
    
    BenchmarkStats stats;
    std::atomic<bool> running{true};
    
    std::cout << "\n开始压力测试..." << std::endl;
    auto testStart = std::chrono::high_resolution_clock::now();
    
    // 启动工作线程
    std::vector<std::thread> threads;
    for (int i = 0; i < config.threadCount; ++i) {
        threads.emplace_back(workerThread, i, &stats, config, &running);
    }
    
    // 如果设置了测试时长，启动定时器
    std::thread timerThread;
    if (config.testDuration > 0) {
        timerThread = std::thread([&running, &config]() {
            std::this_thread::sleep_for(std::chrono::seconds(config.testDuration));
            running = false;
        });
    }
    
    // 等待线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    if (timerThread.joinable()) {
        timerThread.join();
    }
    
    auto testEnd = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(testEnd - testStart).count();
    
    // 输出统计信息
    stats.printStats(duration);
    
    return 0;
} 