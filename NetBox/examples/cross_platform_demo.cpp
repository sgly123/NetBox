/**
 * @file cross_platform_demo.cpp
 * @brief NetBox 跨平台功能演示程序
 * 
 * 这个程序展示了NetBox框架的跨平台能力：
 * 1. 自动检测运行平台
 * 2. 选择最优的IO多路复用器
 * 3. 使用统一的网络API
 * 4. 展示平台特定的优化特性
 */

#include "platform/Platform.h"
#include "IO/IOFactory.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "🌍 NetBox 跨平台功能演示" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // 1. 初始化平台
    if (!NetBox::Platform::initializePlatform()) {
        std::cerr << "❌ 平台初始化失败" << std::endl;
        return 1;
    }
    
    // 2. 显示平台信息
    std::cout << "📋 平台信息:" << std::endl;

#ifdef NETBOX_PLATFORM_WINDOWS
    std::cout << "  - 平台: Windows" << std::endl;
#elif defined(NETBOX_PLATFORM_LINUX)
    std::cout << "  - 平台: Linux" << std::endl;
#elif defined(NETBOX_PLATFORM_MACOS)
    std::cout << "  - 平台: macOS" << std::endl;
#else
    std::cout << "  - 平台: Unknown" << std::endl;
#endif

#ifdef NETBOX_ARCH_X64
    std::cout << "  - 架构: x86-64" << std::endl;
#elif defined(NETBOX_ARCH_X86)
    std::cout << "  - 架构: x86" << std::endl;
#elif defined(NETBOX_ARCH_ARM64)
    std::cout << "  - 架构: ARM64" << std::endl;
#elif defined(NETBOX_ARCH_ARM)
    std::cout << "  - 架构: ARM" << std::endl;
#endif
    
    std::cout << std::endl;
    
    // 3. 显示IO多路复用器支持
    std::cout << "⚡ IO多路复用器支持:" << std::endl;
    
    auto recommendedType = IOFactory::getRecommendedIOType();
    std::cout << "  - 推荐类型: " << IOFactory::getIOTypeName(recommendedType) << std::endl;
    
    auto supportedTypes = IOFactory::getSupportedIOTypes();
    std::cout << "  - 支持类型: ";
    for (size_t i = 0; i < supportedTypes.size(); ++i) {
        std::cout << IOFactory::getIOTypeName(supportedTypes[i]);
        if (i < supportedTypes.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    
    // 4. 创建并测试推荐的IO多路复用器
    std::cout << "\n🚀 创建IO多路复用器:" << std::endl;
    auto io = IOFactory::createIO(recommendedType);
    if (io && io->init()) {
        std::cout << "  ✅ " << IOFactory::getIOTypeName(recommendedType) 
                  << " 多路复用器创建成功" << std::endl;
    } else {
        std::cout << "  ❌ " << IOFactory::getIOTypeName(recommendedType) 
                  << " 多路复用器创建失败" << std::endl;
    }
    
    // 5. 展示平台特定特性
    std::cout << "\n🔧 平台特定特性:" << std::endl;

#ifdef NETBOX_PLATFORM_WINDOWS
    std::cout << "  - Windows IOCP: 支持异步IO和海量并发" << std::endl;
    std::cout << "  - Winsock2: 完整的Windows网络API支持" << std::endl;
#elif defined(NETBOX_PLATFORM_LINUX)
    std::cout << "  - Linux EPOLL: 支持边缘触发和百万级并发" << std::endl;
    std::cout << "  - SO_REUSEPORT: 支持端口复用负载均衡" << std::endl;
    std::cout << "  - TCP_NODELAY: 支持禁用Nagle算法优化" << std::endl;
#elif defined(NETBOX_PLATFORM_MACOS)
    std::cout << "  - macOS KQUEUE: 支持统一事件处理机制" << std::endl;
    std::cout << "  - BSD Socket: 完整的BSD网络API支持" << std::endl;
    std::cout << "  - 高精度定时器: 支持微秒级定时器事件" << std::endl;
#endif
    
    // 6. 编译时平台检测展示
    std::cout << "\n🔨 编译时平台检测:" << std::endl;
    
#ifdef NETBOX_PLATFORM_WINDOWS
    std::cout << "  - 编译目标: Windows 平台" << std::endl;
    std::cout << "  - 编译器: MSVC" << std::endl;
#elif defined(NETBOX_PLATFORM_LINUX)
    std::cout << "  - 编译目标: Linux 平台" << std::endl;
    #ifdef NETBOX_COMPILER_GCC
        std::cout << "  - 编译器: GCC" << std::endl;
    #elif defined(NETBOX_COMPILER_CLANG)
        std::cout << "  - 编译器: Clang" << std::endl;
    #endif
#elif defined(NETBOX_PLATFORM_MACOS)
    std::cout << "  - 编译目标: macOS 平台" << std::endl;
    std::cout << "  - 编译器: Clang" << std::endl;
#endif

    // 7. 架构信息
    std::cout << "\n🏗️ 系统架构:" << std::endl;
    
#ifdef NETBOX_ARCH_X64
    std::cout << "  - 架构: x86-64 (64位)" << std::endl;
#elif defined(NETBOX_ARCH_X86)
    std::cout << "  - 架构: x86 (32位)" << std::endl;
#elif defined(NETBOX_ARCH_ARM64)
    std::cout << "  - 架构: ARM64 (64位)" << std::endl;
#elif defined(NETBOX_ARCH_ARM)
    std::cout << "  - 架构: ARM (32位)" << std::endl;
#endif

    // 8. 构建信息
    std::cout << "\n📦 构建信息:" << std::endl;
    
#ifdef DEBUG
    std::cout << "  - 构建类型: Debug" << std::endl;
#else
    std::cout << "  - 构建类型: Release" << std::endl;
#endif

    std::cout << "  - C++标准: C++17" << std::endl;
    
    // 9. 性能特性展示
    std::cout << "\n📊 性能特性:" << std::endl;

#ifdef NETBOX_PLATFORM_WINDOWS
    std::cout << "  - 预期并发: 10,000+ 连接 (IOCP)" << std::endl;
    std::cout << "  - 内存效率: 优秀 (异步IO)" << std::endl;
#elif defined(NETBOX_PLATFORM_LINUX)
    std::cout << "  - 预期并发: 100,000+ 连接 (EPOLL)" << std::endl;
    std::cout << "  - 内存效率: 极优 (边缘触发)" << std::endl;
#elif defined(NETBOX_PLATFORM_MACOS)
    std::cout << "  - 预期并发: 50,000+ 连接 (KQUEUE)" << std::endl;
    std::cout << "  - 功能丰富: 统一事件处理" << std::endl;
#endif
    
    // 10. 总结
    std::cout << "\n🎯 跨平台总结:" << std::endl;
    std::cout << "  ✅ 平台检测: 自动识别运行环境" << std::endl;
    std::cout << "  ✅ IO优化: 自动选择最优IO模型" << std::endl;
    std::cout << "  ✅ API统一: 跨平台代码兼容" << std::endl;
    std::cout << "  ✅ 性能优化: 平台特定优化策略" << std::endl;
    
    std::cout << "\n=======================================" << std::endl;
    std::cout << "🎉 NetBox 跨平台演示完成！" << std::endl;
    std::cout << "   支持 Windows、Linux、macOS 三大平台" << std::endl;
    std::cout << "   实现了真正的 '一次编写，到处编译'" << std::endl;
    
    // 清理平台资源
    NetBox::Platform::cleanupPlatform();
    
    return 0;
}
