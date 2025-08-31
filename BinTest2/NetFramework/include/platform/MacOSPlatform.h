#pragma once

/**
 * @file MacOSPlatform.h
 * @brief macOS平台特定实现
 * 
 * 设计说明：
 * 1. 基于BSD/Darwin内核的POSIX实现
 * 2. 支持kqueue高性能IO多路复用
 * 3. 利用macOS特有的系统API
 * 4. 处理macOS特定的行为差异
 * 
 * macOS网络编程特点：
 * - 基于BSD socket API
 * - 支持kqueue事件通知机制
 * - 使用sysctl获取系统信息
 * - 支持Grand Central Dispatch (GCD)
 * - 文件系统大小写不敏感（默认）
 */

#ifdef NETBOX_PLATFORM_MACOS

// macOS/Darwin系统头文件
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/event.h>      // kqueue
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>     // macOS系统信息接口
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
#include <mach/mach.h>      // Mach内核接口
#include <mach/mach_host.h>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace NetBox {
namespace Platform {

/**
 * @brief macOS平台网络API封装
 * 基于BSD socket API，与Linux类似但有细微差异
 */
class MacOSNetAPI {
public:
    /**
     * @brief 初始化网络库（macOS无需特殊初始化）
     * @return bool 始终返回true
     */
    static bool initialize() { return true; }
    
    /**
     * @brief 清理网络库（macOS无需特殊清理）
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
     * macOS实现：
     * - 与Linux相同，使用fcntl + O_NONBLOCK
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
     * @brief 设置SO_REUSEPORT选项（macOS支持）
     * @param sockfd socket描述符
     * @return bool 设置是否成功
     * 
     * macOS的SO_REUSEPORT：
     * - 从macOS 10.9开始支持
     * - 行为与Linux类似
     */
    static bool set_reuseport(int sockfd) {
        int opt = 1;
        return socket_setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == 0;
    }
    
    /**
     * @brief 设置TCP_NODELAY选项
     * @param sockfd socket描述符
     * @return bool 设置是否成功
     */
    static bool set_tcp_nodelay(int sockfd) {
        int opt = 1;
        return socket_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == 0;
    }
};

/**
 * @brief macOS平台系统信息获取
 * 使用sysctl和Mach内核接口获取系统信息
 */
class MacOSSystemInfo {
public:
    /**
     * @brief 获取CPU核心数
     * @return int CPU核心数
     * 
     * macOS实现：
     * 1. 使用sysctl("hw.ncpu")获取逻辑核心数
     * 2. 使用sysctl("hw.physicalcpu")获取物理核心数
     */
    static int getCPUCores() {
        int cores = 0;
        size_t size = sizeof(cores);
        if (sysctlbyname("hw.ncpu", &cores, &size, nullptr, 0) == 0) {
            return cores;
        }
        
        // 备用方案
        long cores_long = sysconf(_SC_NPROCESSORS_ONLN);
        return cores_long > 0 ? static_cast<int>(cores_long) : 1;
    }
    
    /**
     * @brief 获取物理CPU核心数
     * @return int 物理CPU核心数
     */
    static int getPhysicalCPUCores() {
        int cores = 0;
        size_t size = sizeof(cores);
        if (sysctlbyname("hw.physicalcpu", &cores, &size, nullptr, 0) == 0) {
            return cores;
        }
        return getCPUCores();
    }
    
    /**
     * @brief 获取总内存大小
     * @return size_t 总内存字节数
     * 
     * macOS实现：
     * - 使用sysctl("hw.memsize")获取物理内存大小
     */
    static size_t getTotalMemory() {
        uint64_t memsize = 0;
        size_t size = sizeof(memsize);
        if (sysctlbyname("hw.memsize", &memsize, &size, nullptr, 0) == 0) {
            return static_cast<size_t>(memsize);
        }
        return 0;
    }
    
    /**
     * @brief 获取macOS版本信息
     * @return std::string 版本字符串
     * 
     * macOS实现：
     * 1. 使用sysctl("kern.version")获取内核版本
     * 2. 解析版本号获取macOS版本
     */
    static std::string getOSVersion() {
        // 获取macOS版本
        char version[256];
        size_t size = sizeof(version);
        if (sysctlbyname("kern.osproductversion", version, &size, nullptr, 0) == 0) {
            return "macOS " + std::string(version);
        }
        
        // 备用方案：使用uname
        struct utsname info;
        if (uname(&info) == 0) {
            return std::string(info.sysname) + " " + info.release;
        }
        
        return "macOS";
    }
    
    /**
     * @brief 获取Darwin内核版本
     * @return std::string 内核版本字符串
     */
    static std::string getKernelVersion() {
        char version[256];
        size_t size = sizeof(version);
        if (sysctlbyname("kern.version", version, &size, nullptr, 0) == 0) {
            std::string ver(version);
            // 提取版本号（第一行）
            size_t pos = ver.find('\n');
            if (pos != std::string::npos) {
                ver = ver.substr(0, pos);
            }
            return ver;
        }
        
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
        char machine[64];
        size_t size = sizeof(machine);
        if (sysctlbyname("hw.machine", machine, &size, nullptr, 0) == 0) {
            std::string arch(machine);
            if (arch == "x86_64") {
                return PlatformInfo::Architecture::X64;
            } else if (arch == "i386") {
                return PlatformInfo::Architecture::X86;
            } else if (arch == "arm64") {
                return PlatformInfo::Architecture::ARM64;
            } else if (arch.find("arm") != std::string::npos) {
                return PlatformInfo::Architecture::ARM;
            }
        }
        return PlatformInfo::Architecture::Unknown;
    }
    
    /**
     * @brief 获取CPU品牌信息
     * @return std::string CPU品牌字符串
     */
    static std::string getCPUBrand() {
        char brand[128];
        size_t size = sizeof(brand);
        if (sysctlbyname("machdep.cpu.brand_string", brand, &size, nullptr, 0) == 0) {
            return std::string(brand);
        }
        return "Unknown CPU";
    }
};

/**
 * @brief macOS平台文件系统操作
 */
class MacOSFileSystem {
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
     * @brief 规范化路径分隔符（macOS使用/）
     * @param path 原始路径
     * @return std::string 规范化后的路径
     */
    static std::string normalizePath(const std::string& path) {
        return path; // macOS路径已经是标准格式
    }
    
    /**
     * @brief 检查文件系统是否大小写敏感
     * @param path 测试路径
     * @return bool 是否大小写敏感
     * 
     * macOS特殊性：
     * - 默认文件系统(APFS/HFS+)大小写不敏感
     * - 可以格式化为大小写敏感
     */
    static bool isCaseSensitive(const std::string& path = "/") {
        // 简单测试：创建临时文件检查
        std::string testFile = path + "/NetBox_Case_Test";
        std::string testFileUpper = path + "/NETBOX_CASE_TEST";
        
        // 创建小写文件
        int fd = open(testFile.c_str(), O_CREAT | O_EXCL, 0644);
        if (fd >= 0) {
            close(fd);
            
            // 检查大写文件是否存在
            bool sensitive = !fileExists(testFileUpper);
            
            // 清理
            unlink(testFile.c_str());
            return sensitive;
        }
        
        return false; // 默认假设不敏感
    }
};

/**
 * @brief macOS平台线程操作
 */
class MacOSThread {
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
        pthread_setname_np(name.c_str()); // macOS版本不需要线程参数
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
#define netbox_close(fd) NetBox::Platform::MacOSNetAPI::socket_close(fd)
#define netbox_socket(family, type, protocol) NetBox::Platform::MacOSNetAPI::socket_create(family, type, protocol)
#define netbox_bind(fd, addr, len) NetBox::Platform::MacOSNetAPI::socket_bind(fd, addr, len)
#define netbox_listen(fd, backlog) NetBox::Platform::MacOSNetAPI::socket_listen(fd, backlog)
#define netbox_accept(fd, addr, len) NetBox::Platform::MacOSNetAPI::socket_accept(fd, addr, len)
#define netbox_connect(fd, addr, len) NetBox::Platform::MacOSNetAPI::socket_connect(fd, addr, len)
#define netbox_send(fd, buf, len, flags) NetBox::Platform::MacOSNetAPI::socket_send(fd, buf, len, flags)
#define netbox_recv(fd, buf, len, flags) NetBox::Platform::MacOSNetAPI::socket_recv(fd, buf, len, flags)

#endif // NETBOX_PLATFORM_MACOS
