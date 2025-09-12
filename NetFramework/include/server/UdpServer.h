#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <string>
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include <queue>
#include <vector>
#include "../IO/IOFactory.h"
#include "../base/Logger.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

/**
 * @brief UDP服务器类，提供无连接的数据报服务
 *        支持事件回调机制，可用于各种UDP应用场景
 */
class UdpServer {
public:
    // 数据报接收回调：参数为客户端地址和收到的数据
    using OnDatagramCallback = std::function<void(const sockaddr_in&, const std::string&)>;
    // 错误处理回调：参数为错误码和错误信息
    using OnErrorCallback = std::function<void(int, const std::string&)>;

    /**
     * @brief UDP错误类型枚举
     */
    enum UdpErrorType {
        BIND_FAILED = 1,
        SEND_FAILED = 2,
        RECV_FAILED = 3,
        PACKET_TOO_LARGE = 4,
        INVALID_ADDRESS = 5,
        UDP_SOCKET_ERROR = 6
    };

    /**
     * @brief 构造函数
     * @param ip      监听IP地址
     * @param port    监听端口
     * @param io_type IO多路复用类型
     */
    UdpServer(const std::string& ip, int port, IOMultiplexer::IOType io_type);
    virtual ~UdpServer();

    // 设置数据报接收回调
    void setOnDatagram(OnDatagramCallback cb);
    // 设置错误处理回调
    void setOnError(OnErrorCallback cb);

    // 启动服务器
    virtual bool start();
    // 停止服务器
    virtual void stop();

    // 发送数据到指定地址
    virtual bool sendTo(const sockaddr_in& addr, const std::string& data);
    virtual bool sendTo(const std::string& ip, int port, const std::string& data);

    // 数据包接收处理（应用层可重写）
    virtual void onDatagramReceived(const sockaddr_in& from, const char* data, size_t len);
    // 错误处理（应用层可重写）
    virtual void onError(int error_code, const std::string& message);

    // 获取当前IO类型
    IOMultiplexer::IOType type() const;

    // 获取统计信息
    struct UdpStats {
        std::atomic<uint64_t> packets_received{0};
        std::atomic<uint64_t> packets_sent{0};
        std::atomic<uint64_t> bytes_received{0};
        std::atomic<uint64_t> bytes_sent{0};
        std::atomic<uint64_t> send_errors{0};
        std::atomic<uint64_t> recv_errors{0};
    };
    
    const UdpStats& getStats() const { return m_stats; }

protected:
    // 处理UDP读事件
    virtual void handleRead();
    // 处理UDP写事件
    virtual void handleWrite();
    // 主运行循环
    void run();

    // 工具函数
    sockaddr_in createAddress(const std::string& ip, int port);
    std::string addressToString(const sockaddr_in& addr);

private:
    // 发送队列项
    struct SendItem {
        sockaddr_in addr;
        std::vector<char> data;
        
        SendItem(const sockaddr_in& a, const std::string& d) 
            : addr(a), data(d.begin(), d.end()) {}
    };

    // 成员变量
    int m_socket;                                    // UDP socket
    int m_port;                                      // 监听端口
    std::string m_ip;                               // 监听IP
    std::atomic<bool> m_running;                    // 运行状态
    std::unique_ptr<IOMultiplexer> m_io;           // IO多路复用器
    
    // 接收缓冲区
    char m_recvBuffer[IOMultiplexer::MAX_UDP_PACKET_SIZE];
    
    // 发送队列和锁
    std::queue<SendItem> m_sendQueue;
    std::mutex m_sendMutex;
    
    // 事件回调
    OnDatagramCallback m_onDatagram;
    OnErrorCallback m_onError;
    
    // 统计信息
    UdpStats m_stats;
    
    // 内部方法
    bool createSocket();
    bool bindSocket();
    void closeSocket();
    bool processRecv();
    bool processSend();
    void notifyError(UdpErrorType type, const std::string& detail = "");
};

#endif // UDP_SERVER_H 