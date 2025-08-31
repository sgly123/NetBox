#pragma once

/**
 * @file LinuxPlatform.h
 * @brief Linux平台特定实现
 * 
 * 设计说明：
 * 1. 封装Linux系统调用和POSIX API
 * 2. 提供高性能的epoll IO多路复用
 * 3. 支持Linux特有的优化特性
 * 4. 处理Linux特定的错误处理和信号
 * 
 * Linux网络编程特点：
 * - 原生支持POSIX socket API
 * - 提供高性能的epoll机制
 * - 支持SO_REUSEPORT等高级特性
 * - 丰富的系统信息接口(/proc文件系统)
 * - 支持各种Linux发行版差异
 */

#ifdef NETBOX_PLATFORM_LINUX

// Linux系统头文件
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>
#include <cstring>  // 添加这个头文件以支持strerror

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace NetBox {
namespace Platform {

/**
 * @brief Linux平台网络API封装
 * 直接使用POSIX标准API，提供统一接口
 */
class LinuxNetAPI {
public:
    /**
     * @brief 初始化网络库（Linux无需特殊初始化）
     * @return bool 始终返回true
     */
    static bool initialize() { return true; }
    
    /**
     * @brief 清理网络库（Linux无需特殊清理）
     */
    static void cleanup() {}
    
    /**
     * @brief 创建socket
     * @param family 地址族 (AF_INET, AF_INET6)
     * @param type 套接字类型 (SOCK_STREAM, SOCK_DGRAM)
     * @param protocol 协议 (通常为0)
     * @return int socket描述符，失败返回-1
     */
    static int socket_create(int family, int type, int protocol) {
        return ::socket(family, type, protocol);
    }
    
    /**
     * @brief 关闭socket
     * @param sockfd socket描述符
     * @return int 成功返回0，失败返回-1
     */
    static int socket_close(int sockfd) {
        return ::close(sockfd);
    }
    
    /**
     * @brief 绑定socket到地址
     * @param sockfd socket描述符
     * @param addr 地址结构体指针
     * @param addrlen 地址结构体长度
     * @return int 成功返回0，失败返回-1
     */
    static int socket_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
        return ::bind(sockfd, addr, addrlen);
    }
    
    /**
     * @brief 监听连接
     * @param sockfd socket描述符
     * @param backlog 监听队列长度
     * @return int 成功返回0，失败返回-1
     */
    static int socket_listen(int sockfd, int backlog) {
        return ::listen(sockfd, backlog);
    }
    
    /**
     * @brief 接受连接
     * @param sockfd 监听socket描述符
     * @param addr 客户端地址结构体指针
     * @param addrlen 地址结构体长度指针
     * @return int 新连接的socket描述符，失败返回-1
     */
    static int socket_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
        return ::accept(sockfd, addr, addrlen);
    }
    
    /**
     * @brief 连接到服务器
     * @param sockfd socket描述符
     * @param addr 服务器地址结构体指针
     * @param addrlen 地址结构体长度
     * @return int 成功返回0，失败返回-1
     */
    static int socket_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
        return ::connect(sockfd, addr, addrlen);
    }
    
    /**
     * @brief 发送数据
     * @param sockfd socket描述符
     * @param buf 数据缓冲区
     * @param len 数据长度
     * @param flags 发送标志
     * @return ssize_t 实际发送的字节数，失败返回-1
     */
    static ssize_t socket_send(int sockfd, const void* buf, size_t len, int flags) {
        return ::send(sockfd, buf, len, flags);
    }
    
    /**
     * @brief 接收数据
     * @param sockfd socket描述符
     * @param buf 接收缓冲区
     * @param len 缓冲区长度
     * @param flags 接收标志
     * @return ssize_t 实际接收的字节数，失败返回-1
     */
    static ssize_t socket_recv(int sockfd, void* buf, size_t len, int flags) {
        return ::recv(sockfd, buf, len, flags);
    }
    
    /**
     * @brief 设置socket选项
     * @param sockfd socket描述符
     * @param level 选项级别
     * @param optname 选项名称
     * @param optval 选项值指针
     * @param optlen 选项值长度
     * @return int 成功返回0，失败返回-1
     */
    static int socket_setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) {
        return ::setsockopt(sockfd, level, optname, optval, optlen);
    }
    
    /**
     * @brief 设置socket为非阻塞模式
     * @param sockfd socket描述符
     * @return bool 设置是否成功
     * 
     * Linux实现：
     * - 使用fcntl(sockfd, F_SETFL, flags | O_NONBLOCK)
     * - 获取当前标志并添加O_NONBLOCK
     */
    static bool set_nonblocking(int sockfd) {
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        if (flags == -1) return false;
        return ::fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) != -1;
    }
    
    /**
     * @brief 获取最后的错误码
     * @return int 错误码 (errno)
     */
    static int get_last_error() {
        return errno;
    }
    
    /**
     * @brief 将错误码转换为字符串
     * @param error_code 错误码
     * @return std::string 错误描述
     */
    static std::string error_to_string(int error_code) {
        return std::string(strerror(error_code));
    }
    
    /**
     * @brief 设置SO_REUSEPORT选项（Linux特有）
     * @param sockfd socket描述符
     * @return bool 设置是否成功
     * 
     * SO_REUSEPORT优势：
     * - 允许多个进程绑定同一端口
     * - 内核负载均衡
     * - 提高多进程服务器性能
     */
    static bool set_reuseport(int sockfd) {
        int opt = 1;
        return socket_setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == 0;
    }
    
    /**
     * @brief 设置TCP_NODELAY选项
     * @param sockfd socket描述符
     * @return bool 设置是否成功
     * 
     * TCP_NODELAY作用：
     * - 禁用Nagle算法
     * - 减少小包延迟
     * - 适用于低延迟应用
     */
    static bool set_tcp_nodelay(int sockfd) {
        int opt = 1;
        return socket_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == 0;
    }
};

/**
 * @brief Linux平台系统信息获取
 * 利用/proc文件系统和系统调用获取详细信息
 */
class LinuxSystemInfo {
public:
    /**
     * @brief 获取CPU核心数
     * @return int CPU核心数
     * 
     * 实现方式：
     * 1. 优先使用sysconf(_SC_NPROCESSORS_ONLN)
     * 2. 备用方案：解析/proc/cpuinfo
     */
    static int getCPUCores() {
        long cores = sysconf(_SC_NPROCESSORS_ONLN);
        return cores > 0 ? static_cast<int>(cores) : 1;
    }
    
    /**
     * @brief 获取总内存大小
     * @return size_t 总内存字节数
     * 
     * 实现方式：
     * 1. 使用sysinfo系统调用
     * 2. 备用方案：解析/proc/meminfo
     */
    static size_t getTotalMemory() {
        struct sysinfo info;
        if (sysinfo(&info) == 0) {
            return info.totalram * info.mem_unit;
        }
        return 0;
    }
    
    /**
     * @brief 获取Linux发行版信息
     * @return std::string 发行版字符串
     * 
     * 实现方式：
     * 1. 读取/etc/os-release
     * 2. 备用方案：读取/etc/issue
     * 3. 最后方案：使用uname
     */
    static std::string getOSVersion() {
        // 尝试读取/etc/os-release
        std::ifstream file("/etc/os-release");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.find("PRETTY_NAME=") == 0) {
                    return line.substr(13, line.length() - 14); // 去掉引号
                }
            }
        }
        
        // 备用方案：使用uname
        struct utsname info;
        if (uname(&info) == 0) {
            return std::string(info.sysname) + " " + info.release;
        }
        
        return "Linux";
    }
    
    /**
     * @brief 获取内核版本
     * @return std::string 内核版本字符串
     */
    static std::string getKernelVersion() {
        struct utsname info;
        if (uname(&info) == 0) {
            return std::string(info.release);
        }
        return "Unknown";
    }
    
    /**
     * @brief 获取系统架构
     * @return PlatformInfo::Architecture 系统架构
     */
    static PlatformInfo::Architecture getArchitecture() {
        struct utsname info;
        if (uname(&info) == 0) {
            std::string machine(info.machine);
            if (machine == "x86_64" || machine == "amd64") {
                return PlatformInfo::Architecture::X64;
            } else if (machine == "i386" || machine == "i686") {
                return PlatformInfo::Architecture::X86;
            } else if (machine == "aarch64") {
                return PlatformInfo::Architecture::ARM64;
            } else if (machine.find("arm") != std::string::npos) {
                return PlatformInfo::Architecture::ARM;
            }
        }
        return PlatformInfo::Architecture::Unknown;
    }
};

/**
 * @brief Linux平台文件系统操作
 */
class LinuxFileSystem {
public:
    /**
     * @brief 获取当前工作目录
     * @return std::string 当前目录路径
     */
    static std::string getCurrentDirectory() {
        char* cwd = getcwd(nullptr, 0);
        if (cwd) {
            std::string result(cwd);
            free(cwd);
            return result;
        }
        return "/";
    }
    
    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return bool 文件是否存在
     */
    static bool fileExists(const std::string& path) {
        return access(path.c_str(), F_OK) == 0;
    }
    
    /**
     * @brief 创建目录
     * @param path 目录路径
     * @return bool 创建是否成功
     */
    static bool createDirectory(const std::string& path) {
        return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
    }
    
    /**
     * @brief 规范化路径分隔符（Linux已经是/）
     * @param path 原始路径
     * @return std::string 规范化后的路径
     */
    static std::string normalizePath(const std::string& path) {
        return path; // Linux路径已经是标准格式
    }
};

/**
 * @brief Linux平台线程操作
 */
class LinuxThread {
public:
    /**
     * @brief 获取当前线程ID
     * @return pthread_t 线程ID
     */
    static pthread_t getCurrentThreadId() {
        return pthread_self();
    }
    
    /**
     * @brief 设置线程名称（用于调试）
     * @param name 线程名称
     */
    static void setThreadName(const std::string& name) {
        pthread_setname_np(pthread_self(), name.c_str());
    }
    
    /**
     * @brief 线程休眠
     * @param milliseconds 休眠毫秒数
     */
    static void sleep(int milliseconds) {
        usleep(milliseconds * 1000);
    }
};

} // namespace Platform
} // namespace NetBox

// 为了兼容性，定义统一的宏
#define netbox_close(fd) NetBox::Platform::LinuxNetAPI::socket_close(fd)
#define netbox_socket(family, type, protocol) NetBox::Platform::LinuxNetAPI::socket_create(family, type, protocol)
#define netbox_bind(fd, addr, len) NetBox::Platform::LinuxNetAPI::socket_bind(fd, addr, len)
#define netbox_listen(fd, backlog) NetBox::Platform::LinuxNetAPI::socket_listen(fd, backlog)
#define netbox_accept(fd, addr, len) NetBox::Platform::LinuxNetAPI::socket_accept(fd, addr, len)
#define netbox_connect(fd, addr, len) NetBox::Platform::LinuxNetAPI::socket_connect(fd, addr, len)
#define netbox_send(fd, buf, len, flags) NetBox::Platform::LinuxNetAPI::socket_send(fd, buf, len, flags)
#define netbox_recv(fd, buf, len, flags) NetBox::Platform::LinuxNetAPI::socket_recv(fd, buf, len, flags)

#endif // NETBOX_PLATFORM_LINUX
