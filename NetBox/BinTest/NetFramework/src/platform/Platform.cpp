/**
 * @file Platform.cpp
 * @brief NetBox框架跨平台抽象层实现
 * 
 * 实现说明：
 * 1. 提供运行时平台检测功能
 * 2. 统一不同平台的系统信息获取接口
 * 3. 处理平台特定的初始化和清理工作
 * 4. 为上层应用提供一致的平台抽象
 */

#include "platform/Platform.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <signal.h>

namespace NetBox {
namespace Platform {

/**
 * @brief 获取当前平台信息
 * @return PlatformInfo 平台信息结构体
 * 
 * 实现策略：
 * - 编译时确定基本平台类型
 * - 运行时获取详细系统信息
 * - 缓存结果以提高性能
 */
PlatformInfo PlatformInfo::getCurrent() {
    static PlatformInfo info; // 静态变量，只初始化一次
    static bool initialized = false;
    
    if (!initialized) {
        // 确定操作系统类型
#ifdef NETBOX_PLATFORM_WINDOWS
        info.os = OS::Windows;
        #ifdef NETBOX_PLATFORM_WINDOWS
            info.osVersion = NetBox::Platform::WindowsSystemInfo::getOSVersion();
            info.cpuCores = NetBox::Platform::WindowsSystemInfo::getCPUCores();
            info.totalMemory = NetBox::Platform::WindowsSystemInfo::getTotalMemory();
            info.arch = NetBox::Platform::WindowsSystemInfo::getArchitecture();
        #endif
        info.kernelVersion = "NT Kernel";
        
#elif defined(NETBOX_PLATFORM_LINUX)
        info.os = OS::Linux;
        #ifdef NETBOX_PLATFORM_LINUX
            info.osVersion = NetBox::Platform::LinuxSystemInfo::getOSVersion();
            info.kernelVersion = NetBox::Platform::LinuxSystemInfo::getKernelVersion();
            info.cpuCores = NetBox::Platform::LinuxSystemInfo::getCPUCores();
            info.totalMemory = NetBox::Platform::LinuxSystemInfo::getTotalMemory();
            info.arch = NetBox::Platform::LinuxSystemInfo::getArchitecture();
        #endif
        
#elif defined(NETBOX_PLATFORM_MACOS)
        info.os = OS::MacOS;
        #ifdef NETBOX_PLATFORM_MACOS
            info.osVersion = NetBox::Platform::MacOSSystemInfo::getOSVersion();
            info.kernelVersion = NetBox::Platform::MacOSSystemInfo::getKernelVersion();
            info.cpuCores = NetBox::Platform::MacOSSystemInfo::getCPUCores();
            info.totalMemory = NetBox::Platform::MacOSSystemInfo::getTotalMemory();
            info.arch = NetBox::Platform::MacOSSystemInfo::getArchitecture();
        #endif
        
#elif defined(NETBOX_PLATFORM_FREEBSD)
        info.os = OS::FreeBSD;
        info.osVersion = "FreeBSD";
        info.kernelVersion = "FreeBSD Kernel";
        info.cpuCores = 1;
        info.totalMemory = 0;
        info.arch = Architecture::Unknown;
        
#elif defined(NETBOX_PLATFORM_OPENBSD)
        info.os = OS::OpenBSD;
        info.osVersion = "OpenBSD";
        info.kernelVersion = "OpenBSD Kernel";
        info.cpuCores = 1;
        info.totalMemory = 0;
        info.arch = Architecture::Unknown;
        
#else
        info.os = OS::Unknown;
        info.osVersion = "Unknown OS";
        info.kernelVersion = "Unknown Kernel";
        info.cpuCores = 1;
        info.totalMemory = 0;
        info.arch = Architecture::Unknown;
#endif

        initialized = true;
    }
    
    return info;
}

/**
 * @brief 获取平台名称字符串
 * @return std::string 平台名称
 */
std::string PlatformInfo::getPlatformName() const {
    std::ostringstream oss;
    
    // 操作系统名称
    switch (os) {
        case OS::Windows: oss << "Windows"; break;
        case OS::Linux: oss << "Linux"; break;
        case OS::MacOS: oss << "macOS"; break;
        case OS::FreeBSD: oss << "FreeBSD"; break;
        case OS::OpenBSD: oss << "OpenBSD"; break;
        default: oss << "Unknown"; break;
    }
    
    // 架构信息
    oss << " ";
    switch (arch) {
        case Architecture::X86: oss << "x86"; break;
        case Architecture::X64: oss << "x64"; break;
        case Architecture::ARM: oss << "ARM"; break;
        case Architecture::ARM64: oss << "ARM64"; break;
        default: oss << "Unknown"; break;
    }
    
    // CPU核心数和内存信息
    oss << " (" << cpuCores << " cores";
    if (totalMemory > 0) {
        double memoryGB = static_cast<double>(totalMemory) / (1024 * 1024 * 1024);
        oss << ", " << std::fixed << std::setprecision(1) << memoryGB << "GB RAM";
    }
    oss << ")";
    
    return oss.str();
}

/**
 * @brief 全局平台初始化函数
 * @return bool 初始化是否成功
 * 
 * 执行平台特定的初始化工作：
 * - Windows: 初始化Winsock库
 * - Linux: 设置信号处理
 * - macOS: 系统特定初始化
 */
bool initializePlatform() {
    static bool initialized = false;
    if (initialized) return true;
    
#ifdef NETBOX_PLATFORM_WINDOWS
    // Windows平台初始化
    if (!NetBox::Platform::WindowsNetAPI::initialize()) {
        return false;
    }
#elif defined(NETBOX_PLATFORM_LINUX)
    // Linux平台初始化
    // 忽略SIGPIPE信号，避免写入关闭的socket时程序崩溃
    signal(SIGPIPE, SIG_IGN);
#elif defined(NETBOX_PLATFORM_MACOS)
    // macOS平台初始化
    // 忽略SIGPIPE信号
    signal(SIGPIPE, SIG_IGN);
#endif
    
    initialized = true;
    return true;
}

/**
 * @brief 全局平台清理函数
 * 
 * 执行平台特定的清理工作：
 * - Windows: 清理Winsock库
 * - Linux: 恢复信号处理
 * - macOS: 系统特定清理
 */
void cleanupPlatform() {
#ifdef NETBOX_PLATFORM_WINDOWS
    // Windows平台清理
    NetBox::Platform::WindowsNetAPI::cleanup();
#elif defined(NETBOX_PLATFORM_LINUX)
    // Linux平台清理
    // 恢复SIGPIPE信号处理
    signal(SIGPIPE, SIG_DFL);
#elif defined(NETBOX_PLATFORM_MACOS)
    // macOS平台清理
    // 恢复SIGPIPE信号处理
    signal(SIGPIPE, SIG_DFL);
#endif
}

/**
 * @brief 获取平台特定的错误信息
 * @param errorCode 错误码
 * @return std::string 错误描述
 */
std::string getPlatformErrorString(int errorCode) {
#ifdef NETBOX_PLATFORM_WINDOWS
    return NetBox::Platform::WindowsNetAPI::error_to_string(errorCode);
#elif defined(NETBOX_PLATFORM_LINUX)
    return NetBox::Platform::LinuxNetAPI::error_to_string(errorCode);
#elif defined(NETBOX_PLATFORM_MACOS)
    return NetBox::Platform::MacOSNetAPI::error_to_string(errorCode);
#else
    return "Unknown error: " + std::to_string(errorCode);
#endif
}

/**
 * @brief 获取平台特定的最后错误码
 * @return int 错误码
 */
int getPlatformLastError() {
#ifdef NETBOX_PLATFORM_WINDOWS
    return NetBox::Platform::WindowsNetAPI::get_last_error();
#elif defined(NETBOX_PLATFORM_LINUX)
    return NetBox::Platform::LinuxNetAPI::get_last_error();
#elif defined(NETBOX_PLATFORM_MACOS)
    return NetBox::Platform::MacOSNetAPI::get_last_error();
#else
    return 0;
#endif
}

/**
 * @brief 设置socket为非阻塞模式（跨平台）
 * @param sockfd socket文件描述符
 * @return bool 设置是否成功
 */
bool setPlatformNonBlocking(int sockfd) {
#ifdef NETBOX_PLATFORM_WINDOWS
    return NetBox::Platform::WindowsNetAPI::set_nonblocking(sockfd);
#elif defined(NETBOX_PLATFORM_LINUX)
    return NetBox::Platform::LinuxNetAPI::set_nonblocking(sockfd);
#elif defined(NETBOX_PLATFORM_MACOS)
    return NetBox::Platform::MacOSNetAPI::set_nonblocking(sockfd);
#else
    return false;
#endif
}

/**
 * @brief 打印平台信息（调试用）
 */
void printPlatformInfo() {
    auto info = PlatformInfo::getCurrent();
    
    std::cout << "=== NetBox Platform Information ===" << std::endl;
    std::cout << "Platform: " << info.getPlatformName() << std::endl;
    std::cout << "OS Version: " << info.osVersion << std::endl;
    std::cout << "Kernel: " << info.kernelVersion << std::endl;
    std::cout << "CPU Cores: " << info.cpuCores << std::endl;
    
    if (info.totalMemory > 0) {
        double memoryGB = static_cast<double>(info.totalMemory) / (1024 * 1024 * 1024);
        std::cout << "Total Memory: " << std::fixed << std::setprecision(2) 
                  << memoryGB << " GB" << std::endl;
    }
    
    std::cout << "Compiler: ";
#ifdef NETBOX_COMPILER_MSVC
    std::cout << "MSVC " << NETBOX_COMPILER_VERSION;
#elif defined(NETBOX_COMPILER_GCC)
    std::cout << "GCC " << (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__);
#elif defined(NETBOX_COMPILER_CLANG)
    std::cout << "Clang " << (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__);
#else
    std::cout << "Unknown";
#endif
    std::cout << std::endl;
    
    std::cout << "Build Type: ";
#ifdef DEBUG
    std::cout << "Debug";
#else
    std::cout << "Release";
#endif
    std::cout << std::endl;
    std::cout << "===================================" << std::endl;
}

} // namespace Platform
} // namespace NetBox
