#pragma once

/**
 * @file WindowsPlatform.h
 * @brief Windows平台特定实现
 * 
 * 设计说明：
 * 1. 封装Windows Socket API (Winsock2)
 * 2. 提供与POSIX兼容的接口
 * 3. 处理Windows特有的错误码和行为
 * 4. 支持IOCP高性能IO模型
 * 
 * Windows网络编程特点：
 * - 需要初始化Winsock库 (WSAStartup/WSACleanup)
 * - 使用closesocket()而不是close()
 * - 错误码通过WSAGetLastError()获取
 * - 支持IOCP(完成端口)高性能模型
 * - 文件描述符实际是SOCKET类型
 */

#ifdef NETBOX_PLATFORM_WINDOWS

// Windows系统头文件
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  // 减少Windows.h包含的内容，加快编译
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <io.h>
#include <process.h>

// 链接必要的库
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#include <string>
#include <vector>
#include <memory>

namespace NetBox {
namespace Platform {

/**
 * @brief Windows平台网络API封装
 * 提供与POSIX兼容的网络编程接口
 */
class WindowsNetAPI {
public:
    /**
     * @brief 初始化Windows网络库
     * @return bool 初始化是否成功
     * 
     * 实现细节：
     * - 调用WSAStartup初始化Winsock
     * - 请求Winsock 2.2版本
     * - 检查版本兼容性
     */
    static bool initialize();
    
    /**
     * @brief 清理Windows网络库
     * 
     * 实现细节：
     * - 调用WSACleanup清理资源
     * - 应在程序退出前调用
     */
    static void cleanup();
    
    /**
     * @brief 创建socket
     * @param family 地址族 (AF_INET, AF_INET6)
     * @param type 套接字类型 (SOCK_STREAM, SOCK_DGRAM)
     * @param protocol 协议 (通常为0)
     * @return int socket描述符，失败返回-1
     */
    static int socket_create(int family, int type, int protocol);
    
    /**
     * @brief 关闭socket
     * @param sockfd socket描述符
     * @return int 成功返回0，失败返回-1
     * 
     * Windows特殊处理：
     * - 使用closesocket()而不是close()
     * - 统一返回值格式
     */
    static int socket_close(int sockfd);
    
    /**
     * @brief 绑定socket到地址
     * @param sockfd socket描述符
     * @param addr 地址结构体指针
     * @param addrlen 地址结构体长度
     * @return int 成功返回0，失败返回-1
     */
    static int socket_bind(int sockfd, const struct sockaddr* addr, int addrlen);
    
    /**
     * @brief 监听连接
     * @param sockfd socket描述符
     * @param backlog 监听队列长度
     * @return int 成功返回0，失败返回-1
     */
    static int socket_listen(int sockfd, int backlog);
    
    /**
     * @brief 接受连接
     * @param sockfd 监听socket描述符
     * @param addr 客户端地址结构体指针
     * @param addrlen 地址结构体长度指针
     * @return int 新连接的socket描述符，失败返回-1
     */
    static int socket_accept(int sockfd, struct sockaddr* addr, int* addrlen);
    
    /**
     * @brief 连接到服务器
     * @param sockfd socket描述符
     * @param addr 服务器地址结构体指针
     * @param addrlen 地址结构体长度
     * @return int 成功返回0，失败返回-1
     */
    static int socket_connect(int sockfd, const struct sockaddr* addr, int addrlen);
    
    /**
     * @brief 发送数据
     * @param sockfd socket描述符
     * @param buf 数据缓冲区
     * @param len 数据长度
     * @param flags 发送标志
     * @return int 实际发送的字节数，失败返回-1
     */
    static int socket_send(int sockfd, const void* buf, int len, int flags);
    
    /**
     * @brief 接收数据
     * @param sockfd socket描述符
     * @param buf 接收缓冲区
     * @param len 缓冲区长度
     * @param flags 接收标志
     * @return int 实际接收的字节数，失败返回-1
     */
    static int socket_recv(int sockfd, void* buf, int len, int flags);
    
    /**
     * @brief 设置socket选项
     * @param sockfd socket描述符
     * @param level 选项级别
     * @param optname 选项名称
     * @param optval 选项值指针
     * @param optlen 选项值长度
     * @return int 成功返回0，失败返回-1
     */
    static int socket_setsockopt(int sockfd, int level, int optname, const void* optval, int optlen);
    
    /**
     * @brief 设置socket为非阻塞模式
     * @param sockfd socket描述符
     * @return bool 设置是否成功
     * 
     * Windows实现：
     * - 使用ioctlsocket(sockfd, FIONBIO, &mode)
     * - mode=1表示非阻塞，mode=0表示阻塞
     */
    static bool set_nonblocking(int sockfd);
    
    /**
     * @brief 获取最后的错误码
     * @return int 错误码
     * 
     * Windows实现：
     * - 调用WSAGetLastError()
     * - 转换为标准错误码格式
     */
    static int get_last_error();
    
    /**
     * @brief 将错误码转换为字符串
     * @param error_code 错误码
     * @return std::string 错误描述
     */
    static std::string error_to_string(int error_code);
};

/**
 * @brief Windows平台系统信息获取
 */
class WindowsSystemInfo {
public:
    /**
     * @brief 获取CPU核心数
     * @return int CPU核心数
     */
    static int getCPUCores();
    
    /**
     * @brief 获取总内存大小
     * @return size_t 总内存字节数
     */
    static size_t getTotalMemory();
    
    /**
     * @brief 获取Windows版本信息
     * @return std::string 版本字符串
     */
    static std::string getOSVersion();
    
    /**
     * @brief 获取系统架构
     * @return PlatformInfo::Architecture 系统架构
     */
    static PlatformInfo::Architecture getArchitecture();
};

/**
 * @brief Windows平台文件系统操作
 */
class WindowsFileSystem {
public:
    /**
     * @brief 获取当前工作目录
     * @return std::string 当前目录路径
     */
    static std::string getCurrentDirectory();
    
    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return bool 文件是否存在
     */
    static bool fileExists(const std::string& path);
    
    /**
     * @brief 创建目录
     * @param path 目录路径
     * @return bool 创建是否成功
     */
    static bool createDirectory(const std::string& path);
    
    /**
     * @brief 规范化路径分隔符
     * @param path 原始路径
     * @return std::string 规范化后的路径
     */
    static std::string normalizePath(const std::string& path);
};

/**
 * @brief Windows平台线程操作
 */
class WindowsThread {
public:
    /**
     * @brief 获取当前线程ID
     * @return unsigned long 线程ID
     */
    static unsigned long getCurrentThreadId();
    
    /**
     * @brief 设置线程名称（用于调试）
     * @param name 线程名称
     */
    static void setThreadName(const std::string& name);
    
    /**
     * @brief 线程休眠
     * @param milliseconds 休眠毫秒数
     */
    static void sleep(int milliseconds);
};

} // namespace Platform
} // namespace NetBox

// 为了兼容性，定义一些POSIX风格的宏
#define netbox_close(fd) NetBox::Platform::WindowsNetAPI::socket_close(fd)
#define netbox_socket(family, type, protocol) NetBox::Platform::WindowsNetAPI::socket_create(family, type, protocol)
#define netbox_bind(fd, addr, len) NetBox::Platform::WindowsNetAPI::socket_bind(fd, addr, len)
#define netbox_listen(fd, backlog) NetBox::Platform::WindowsNetAPI::socket_listen(fd, backlog)
#define netbox_accept(fd, addr, len) NetBox::Platform::WindowsNetAPI::socket_accept(fd, addr, len)
#define netbox_connect(fd, addr, len) NetBox::Platform::WindowsNetAPI::socket_connect(fd, addr, len)
#define netbox_send(fd, buf, len, flags) NetBox::Platform::WindowsNetAPI::socket_send(fd, buf, len, flags)
#define netbox_recv(fd, buf, len, flags) NetBox::Platform::WindowsNetAPI::socket_recv(fd, buf, len, flags)

#endif // NETBOX_PLATFORM_WINDOWS
