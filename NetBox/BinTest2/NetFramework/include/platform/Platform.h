#pragma once

#include <string>
#include <cstddef>
#include <vector>

/**
 * @file Platform.h
 * @brief NetBox框架跨平台抽象层
 * 
 * 设计目标：
 * 1. 统一不同操作系统的API差异
 * 2. 提供一致的编程接口
 * 3. 支持条件编译和运行时检测
 * 4. 最小化平台相关代码的影响范围
 * 
 * 支持平台：
 * - Windows (Win32/Win64)
 * - Linux (各发行版)
 * - macOS (Darwin)
 * - FreeBSD/OpenBSD (Unix-like)
 * 
 * 架构设计：
 * Platform.h (统一接口) 
 *   ├── WindowsPlatform.h (Windows实现)
 *   ├── LinuxPlatform.h (Linux实现)
 *   ├── MacOSPlatform.h (macOS实现)
 *   └── UnixPlatform.h (通用Unix实现)
 */

// 平台检测宏定义
#if defined(_WIN32) || defined(_WIN64)
    #define NETBOX_PLATFORM_WINDOWS
    #ifdef _WIN64
        #define NETBOX_PLATFORM_WIN64
    #else
        #define NETBOX_PLATFORM_WIN32
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #define NETBOX_PLATFORM_MACOS
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define NETBOX_PLATFORM_DARWIN
    #endif
#elif defined(__linux__)
    #define NETBOX_PLATFORM_LINUX
#elif defined(__FreeBSD__)
    #define NETBOX_PLATFORM_FREEBSD
    #define NETBOX_PLATFORM_UNIX
#elif defined(__OpenBSD__)
    #define NETBOX_PLATFORM_OPENBSD
    #define NETBOX_PLATFORM_UNIX
#elif defined(__unix__) || defined(__unix)
    #define NETBOX_PLATFORM_UNIX
#else
    #error "Unsupported platform! NetBox supports Windows, Linux, macOS, and Unix-like systems."
#endif

// 编译器检测
#if defined(_MSC_VER)
    #define NETBOX_COMPILER_MSVC
    #define NETBOX_COMPILER_VERSION _MSC_VER
#elif defined(__GNUC__)
    #define NETBOX_COMPILER_GCC
    #define NETBOX_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif defined(__clang__)
    #define NETBOX_COMPILER_CLANG
    #define NETBOX_COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#endif

// 架构检测
#if defined(_M_X64) || defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
    #define NETBOX_ARCH_X64
#elif defined(_M_IX86) || defined(__i386__) || defined(__i386) || defined(i386)
    #define NETBOX_ARCH_X86
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define NETBOX_ARCH_ARM64
#elif defined(_M_ARM) || defined(__arm__)
    #define NETBOX_ARCH_ARM
#endif

// 通用类型定义
namespace NetBox {
namespace Platform {

/**
 * @brief 平台信息结构体
 * 提供运行时平台检测能力
 */
struct PlatformInfo {
    enum class OS {
        Windows,
        Linux, 
        MacOS,
        FreeBSD,
        OpenBSD,
        Unknown
    };
    
    enum class Architecture {
        X86,
        X64,
        ARM,
        ARM64,
        Unknown
    };
    
    OS os;
    Architecture arch;
    std::string osVersion;
    std::string kernelVersion;
    int cpuCores;
    size_t totalMemory;
    
    /**
     * @brief 获取当前平台信息
     * @return PlatformInfo 平台信息结构体
     * 
     * 实现说明：
     * - Windows: 使用GetVersionEx和GetSystemInfo
     * - Linux: 读取/proc/version和/proc/cpuinfo
     * - macOS: 使用sysctl系统调用
     */
    static PlatformInfo getCurrent();
    
    /**
     * @brief 获取平台名称字符串
     * @return std::string 平台名称
     */
    std::string getPlatformName() const;
    
    /**
     * @brief 检查是否为指定平台
     * @param targetOS 目标操作系统
     * @return bool 是否匹配
     */
    bool isOS(OS targetOS) const { return os == targetOS; }
    
    /**
     * @brief 检查是否为指定架构
     * @param targetArch 目标架构
     * @return bool 是否匹配
     */
    bool isArch(Architecture targetArch) const { return arch == targetArch; }
};

/**
 * @brief 平台特定的常量定义
 * 统一不同平台的差异
 */
struct Constants {
#ifdef NETBOX_PLATFORM_WINDOWS
    static constexpr int INVALID_SOCKET_VALUE = -1;  // Windows使用INVALID_SOCKET，但我们统一为-1
    static constexpr char PATH_SEPARATOR = '\\';
    static constexpr const char* LINE_ENDING = "\r\n";
    static constexpr int MAX_PATH_LENGTH = 260;
#else
    static constexpr int INVALID_SOCKET_VALUE = -1;
    static constexpr char PATH_SEPARATOR = '/';
    static constexpr const char* LINE_ENDING = "\n";
    static constexpr int MAX_PATH_LENGTH = 4096;
#endif
    
    // 网络相关常量
    static constexpr int DEFAULT_BACKLOG = 128;
    static constexpr int DEFAULT_BUFFER_SIZE = 8192;
    static constexpr int MAX_EVENTS = 1024;
};

} // namespace Platform
} // namespace NetBox

// 根据平台包含对应的实现头文件
#ifdef NETBOX_PLATFORM_WINDOWS
    #include "platform/WindowsPlatform.h"
#elif defined(NETBOX_PLATFORM_LINUX)
    #include "platform/LinuxPlatform.h"
#elif defined(NETBOX_PLATFORM_MACOS)
    #include "platform/MacOSPlatform.h"
#elif defined(NETBOX_PLATFORM_UNIX)
    #include "platform/UnixPlatform.h"
#endif

// 平台无关的工具宏
#define NETBOX_UNUSED(x) ((void)(x))

// 条件编译辅助宏
#define NETBOX_IF_WINDOWS(code) \
    do { \
        NETBOX_PLATFORM_WINDOWS_ONLY(code) \
    } while(0)

#define NETBOX_IF_UNIX(code) \
    do { \
        NETBOX_PLATFORM_UNIX_ONLY(code) \
    } while(0)

// 平台特定代码块宏
#ifdef NETBOX_PLATFORM_WINDOWS
    #define NETBOX_PLATFORM_WINDOWS_ONLY(code) code
    #define NETBOX_PLATFORM_UNIX_ONLY(code)
#else
    #define NETBOX_PLATFORM_WINDOWS_ONLY(code)
    #define NETBOX_PLATFORM_UNIX_ONLY(code) code
#endif

/**
 * 使用示例：
 * 
 * ```cpp
 * #include "platform/Platform.h"
 * 
 * void example() {
 *     // 运行时平台检测
 *     auto info = NetBox::Platform::PlatformInfo::getCurrent();
 *     if (info.isOS(NetBox::Platform::PlatformInfo::OS::Windows)) {
 *         // Windows特定逻辑
 *     }
 *     
 *     // 编译时平台检测
 *     #ifdef NETBOX_PLATFORM_WINDOWS
 *         // Windows编译时代码
 *     #elif defined(NETBOX_PLATFORM_LINUX)
 *         // Linux编译时代码
 *     #endif
 *     
 *     // 使用平台常量
 *     char separator = NetBox::Platform::Constants::PATH_SEPARATOR;
 * }
 * ```
 */
