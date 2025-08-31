#pragma once

/**
 * @file CrossPlatformNet.h
 * @brief 跨平台网络API统一接口
 * 
 * 设计目标：
 * 1. 提供统一的网络编程接口
 * 2. 隐藏平台差异，简化上层代码
 * 3. 保持高性能，避免不必要的抽象开销
 * 4. 支持所有主流平台的网络特性
 * 
 * 使用方式：
 * ```cpp
 * #include "platform/CrossPlatformNet.h"
 * 
 * // 初始化网络库
 * NetBox::Net::initialize();
 * 
 * // 创建socket
 * int sock = NetBox::Net::socket(AF_INET, SOCK_STREAM, 0);
 * 
 * // 使用统一的API进行网络编程
 * NetBox::Net::bind(sock, addr, addrlen);
 * NetBox::Net::listen(sock, backlog);
 * 
 * // 清理网络库
 * NetBox::Net::cleanup();
 * ```
 */

#include "platform/Platform.h"
#include <string>
#include <vector>

// 平台相关的网络头文件
#ifdef NETBOX_PLATFORM_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
#endif

namespace NetBox {
namespace Net {

/**
 * @brief 网络库初始化
 * @return bool 初始化是否成功
 * 
 * 平台差异处理：
 * - Windows: 调用WSAStartup初始化Winsock
 * - Linux/macOS: 无需特殊初始化，直接返回true
 * - 设置全局错误处理和信号处理
 */
bool initialize();

/**
 * @brief 网络库清理
 * 
 * 平台差异处理：
 * - Windows: 调用WSACleanup清理Winsock
 * - Linux/macOS: 恢复信号处理设置
 */
void cleanup();

/**
 * @brief 创建socket
 * @param family 地址族 (AF_INET, AF_INET6)
 * @param type 套接字类型 (SOCK_STREAM, SOCK_DGRAM)
 * @param protocol 协议 (通常为0)
 * @return int socket描述符，失败返回-1
 * 
 * 跨平台统一：
 * - 所有平台都返回int类型
 * - 失败时统一返回-1
 * - 自动处理平台特定的socket选项
 */
int socket(int family, int type, int protocol);

/**
 * @brief 关闭socket
 * @param sockfd socket描述符
 * @return int 成功返回0，失败返回-1
 * 
 * 平台差异处理：
 * - Windows: 使用closesocket()
 * - Linux/macOS: 使用close()
 */
int close(int sockfd);

/**
 * @brief 绑定socket到地址
 * @param sockfd socket描述符
 * @param addr 地址结构体指针
 * @param addrlen 地址结构体长度
 * @return int 成功返回0，失败返回-1
 */
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

/**
 * @brief 监听连接
 * @param sockfd socket描述符
 * @param backlog 监听队列长度
 * @return int 成功返回0，失败返回-1
 */
int listen(int sockfd, int backlog);

/**
 * @brief 接受连接
 * @param sockfd 监听socket描述符
 * @param addr 客户端地址结构体指针
 * @param addrlen 地址结构体长度指针
 * @return int 新连接的socket描述符，失败返回-1
 */
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);

/**
 * @brief 连接到服务器
 * @param sockfd socket描述符
 * @param addr 服务器地址结构体指针
 * @param addrlen 地址结构体长度
 * @return int 成功返回0，失败返回-1
 */
int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

/**
 * @brief 发送数据
 * @param sockfd socket描述符
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @param flags 发送标志
 * @return int 实际发送的字节数，失败返回-1
 * 
 * 跨平台统一：
 * - 返回值统一为int类型
 * - 处理平台特定的发送标志
 */
int send(int sockfd, const void* buf, size_t len, int flags);

/**
 * @brief 接收数据
 * @param sockfd socket描述符
 * @param buf 接收缓冲区
 * @param len 缓冲区长度
 * @param flags 接收标志
 * @return int 实际接收的字节数，失败返回-1
 */
int recv(int sockfd, void* buf, size_t len, int flags);

/**
 * @brief 设置socket选项
 * @param sockfd socket描述符
 * @param level 选项级别
 * @param optname 选项名称
 * @param optval 选项值指针
 * @param optlen 选项值长度
 * @return int 成功返回0，失败返回-1
 */
int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen);

/**
 * @brief 获取socket选项
 * @param sockfd socket描述符
 * @param level 选项级别
 * @param optname 选项名称
 * @param optval 选项值指针
 * @param optlen 选项值长度指针
 * @return int 成功返回0，失败返回-1
 */
int getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen);

/**
 * @brief 设置socket为非阻塞模式
 * @param sockfd socket描述符
 * @return bool 设置是否成功
 * 
 * 平台差异处理：
 * - Windows: 使用ioctlsocket(FIONBIO)
 * - Linux/macOS: 使用fcntl(O_NONBLOCK)
 */
bool setNonBlocking(int sockfd);

/**
 * @brief 设置socket为阻塞模式
 * @param sockfd socket描述符
 * @return bool 设置是否成功
 */
bool setBlocking(int sockfd);

/**
 * @brief 设置SO_REUSEADDR选项
 * @param sockfd socket描述符
 * @return bool 设置是否成功
 */
bool setReuseAddr(int sockfd);

/**
 * @brief 设置SO_REUSEPORT选项（如果平台支持）
 * @param sockfd socket描述符
 * @return bool 设置是否成功
 * 
 * 平台支持：
 * - Linux: 支持，用于负载均衡
 * - macOS: 支持
 * - Windows: 不支持，返回false
 */
bool setReusePort(int sockfd);

/**
 * @brief 设置TCP_NODELAY选项
 * @param sockfd socket描述符
 * @param enable 是否启用
 * @return bool 设置是否成功
 */
bool setTcpNoDelay(int sockfd, bool enable = true);

/**
 * @brief 设置SO_KEEPALIVE选项
 * @param sockfd socket描述符
 * @param enable 是否启用
 * @return bool 设置是否成功
 */
bool setKeepAlive(int sockfd, bool enable = true);

/**
 * @brief 设置socket发送缓冲区大小
 * @param sockfd socket描述符
 * @param size 缓冲区大小
 * @return bool 设置是否成功
 */
bool setSendBufferSize(int sockfd, int size);

/**
 * @brief 设置socket接收缓冲区大小
 * @param sockfd socket描述符
 * @param size 缓冲区大小
 * @return bool 设置是否成功
 */
bool setRecvBufferSize(int sockfd, int size);

/**
 * @brief 获取最后的网络错误码
 * @return int 错误码
 * 
 * 平台差异处理：
 * - Windows: WSAGetLastError()
 * - Linux/macOS: errno
 */
int getLastError();

/**
 * @brief 将错误码转换为字符串
 * @param errorCode 错误码
 * @return std::string 错误描述
 */
std::string errorToString(int errorCode);

/**
 * @brief 检查错误是否为"会阻塞"类型
 * @param errorCode 错误码
 * @return bool 是否为会阻塞错误
 * 
 * 平台差异处理：
 * - Windows: WSAEWOULDBLOCK
 * - Linux/macOS: EAGAIN或EWOULDBLOCK
 */
bool isWouldBlockError(int errorCode);

/**
 * @brief 检查错误是否为"连接中断"类型
 * @param errorCode 错误码
 * @return bool 是否为连接中断错误
 */
bool isConnectionError(int errorCode);

/**
 * @brief 地址转换：字符串IP转二进制
 * @param family 地址族 (AF_INET, AF_INET6)
 * @param src 源IP字符串
 * @param dst 目标二进制地址
 * @return int 成功返回1，失败返回0或-1
 */
int inet_pton(int family, const char* src, void* dst);

/**
 * @brief 地址转换：二进制转字符串IP
 * @param family 地址族 (AF_INET, AF_INET6)
 * @param src 源二进制地址
 * @param dst 目标字符串缓冲区
 * @param size 缓冲区大小
 * @return const char* 成功返回dst，失败返回nullptr
 */
const char* inet_ntop(int family, const void* src, char* dst, socklen_t size);

/**
 * @brief 获取本机IP地址列表
 * @return std::vector<std::string> IP地址列表
 */
std::vector<std::string> getLocalIPAddresses();

/**
 * @brief 检查端口是否可用
 * @param port 端口号
 * @param family 地址族 (AF_INET, AF_INET6)
 * @return bool 端口是否可用
 */
bool isPortAvailable(int port, int family = AF_INET);

/**
 * @brief 获取socket的本地地址
 * @param sockfd socket描述符
 * @param addr 地址结构体指针
 * @param addrlen 地址结构体长度指针
 * @return int 成功返回0，失败返回-1
 */
int getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen);

/**
 * @brief 获取socket的对端地址
 * @param sockfd socket描述符
 * @param addr 地址结构体指针
 * @param addrlen 地址结构体长度指针
 * @return int 成功返回0，失败返回-1
 */
int getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen);

} // namespace Net
} // namespace NetBox
