#ifndef UDP_ECHO_SERVER_H
#define UDP_ECHO_SERVER_H

#include "server/UdpServer.h"
#include <unordered_map>
#include <chrono>

/**
 * @brief UDP Echo服务器
 *        实现简单的UDP回显功能，将收到的数据包原样发送回客户端
 *        支持统计信息和客户端管理
 */
class UdpEchoServer : public UdpServer {
public:
    /**
     * @brief 构造函数
     * @param ip      监听IP地址
     * @param port    监听端口
     * @param io_type IO多路复用类型
     */
    UdpEchoServer(const std::string& ip, int port, IOMultiplexer::IOType io_type);
    
    /**
     * @brief 析构函数
     */
    virtual ~UdpEchoServer();

    /**
     * @brief 启动服务器并设置回调
     */
    bool startEchoServer();

    /**
     * @brief 获取客户端统计信息
     */
    struct ClientStats {
        uint64_t packets_received = 0;
        uint64_t packets_sent = 0;
        uint64_t bytes_received = 0;
        uint64_t bytes_sent = 0;
        std::chrono::steady_clock::time_point last_activity;
        
        ClientStats() : last_activity(std::chrono::steady_clock::now()) {}
    };

    /**
     * @brief 获取所有客户端统计信息
     */
    std::unordered_map<std::string, ClientStats> getClientStats() const;

    /**
     * @brief 清理不活跃的客户端统计信息
     * @param timeout_seconds 超时时间（秒）
     */
    void cleanupInactiveClients(int timeout_seconds = 300);

    /**
     * @brief 打印服务器统计信息
     */
    void printStats() const;

protected:
    /**
     * @brief 处理接收到的数据包
     * @param from 发送方地址
     * @param data 数据内容
     * @param len  数据长度
     */
    void onDatagramReceived(const sockaddr_in& from, const char* data, size_t len) override;

    /**
     * @brief 处理错误
     * @param error_code 错误码
     * @param message    错误信息
     */
    void onError(int error_code, const std::string& message) override;

private:
    // 客户端统计信息（按IP:PORT键值存储）
    mutable std::mutex m_clientStatsMutex;
    std::unordered_map<std::string, ClientStats> m_clientStats;

    /**
     * @brief 更新客户端统计信息
     * @param client_key 客户端键值（IP:PORT）
     * @param bytes_received 接收字节数
     * @param bytes_sent 发送字节数
     */
    void updateClientStats(const std::string& client_key, uint64_t bytes_received, uint64_t bytes_sent = 0);

    /**
     * @brief 获取客户端键值
     * @param addr 客户端地址
     * @return 客户端键值字符串
     */
    std::string getClientKey(const sockaddr_in& addr) const;
};

#endif // UDP_ECHO_SERVER_H 