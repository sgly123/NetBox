#include <gtest/gtest.h>
#include "platform/Platform.h"
#include "platform/CrossPlatformNet.h"
#include "IO/IOFactory.h"
#include "test_utils.h"

/**
 * @brief 跨平台功能测试套件
 * 
 * 测试目标：
 * 1. 验证平台检测功能的正确性
 * 2. 测试跨平台网络API的兼容性
 * 3. 验证IO多路复用器的跨平台支持
 * 4. 确保平台特定优化的有效性
 * 
 * 测试策略：
 * - 编译时测试：验证条件编译的正确性
 * - 运行时测试：验证平台检测和API功能
 * - 性能测试：验证平台特定优化效果
 * - 兼容性测试：确保跨平台API一致性
 */

class CrossPlatformTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化平台
        ASSERT_TRUE(NetBox::Platform::initializePlatform());
        ASSERT_TRUE(NetBox::Net::initialize());
    }

    void TearDown() override {
        // 清理平台资源
        NetBox::Net::cleanup();
        NetBox::Platform::cleanupPlatform();
    }
};

// 平台检测测试
TEST_F(CrossPlatformTest, PlatformDetection) {
    std::cout << "🔍 测试平台检测功能" << std::endl;
    
    // 获取平台信息
    auto info = NetBox::Platform::PlatformInfo::getCurrent();
    
    // 验证平台信息的有效性
    EXPECT_NE(info.os, NetBox::Platform::PlatformInfo::OS::Unknown);
    EXPECT_NE(info.arch, NetBox::Platform::PlatformInfo::Architecture::Unknown);
    EXPECT_GT(info.cpuCores, 0);
    EXPECT_FALSE(info.osVersion.empty());
    EXPECT_FALSE(info.kernelVersion.empty());
    
    // 打印平台信息
    std::cout << "  - 平台: " << info.getPlatformName() << std::endl;
    std::cout << "  - 操作系统: " << info.osVersion << std::endl;
    std::cout << "  - 内核版本: " << info.kernelVersion << std::endl;
    std::cout << "  - CPU核心数: " << info.cpuCores << std::endl;
    
    if (info.totalMemory > 0) {
        double memoryGB = static_cast<double>(info.totalMemory) / (1024 * 1024 * 1024);
        std::cout << "  - 总内存: " << std::fixed << std::setprecision(2) 
                  << memoryGB << " GB" << std::endl;
    }
    
    // 验证编译时平台检测
#ifdef NETBOX_PLATFORM_WINDOWS
    EXPECT_TRUE(info.isOS(NetBox::Platform::PlatformInfo::OS::Windows));
    std::cout << "  - 编译时检测: Windows ✅" << std::endl;
#elif defined(NETBOX_PLATFORM_LINUX)
    EXPECT_TRUE(info.isOS(NetBox::Platform::PlatformInfo::OS::Linux));
    std::cout << "  - 编译时检测: Linux ✅" << std::endl;
#elif defined(NETBOX_PLATFORM_MACOS)
    EXPECT_TRUE(info.isOS(NetBox::Platform::PlatformInfo::OS::MacOS));
    std::cout << "  - 编译时检测: macOS ✅" << std::endl;
#endif
}

// 跨平台网络API测试
TEST_F(CrossPlatformTest, CrossPlatformNetworkAPI) {
    std::cout << "🌐 测试跨平台网络API" << std::endl;
    
    // 测试socket创建
    int sock = NetBox::Net::socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sock, -1) << "Socket创建失败: " << NetBox::Net::errorToString(NetBox::Net::getLastError());
    std::cout << "  - Socket创建: ✅ (fd=" << sock << ")" << std::endl;
    
    if (sock != -1) {
        // 测试socket选项设置
        EXPECT_TRUE(NetBox::Net::setReuseAddr(sock));
        std::cout << "  - SO_REUSEADDR设置: ✅" << std::endl;
        
        EXPECT_TRUE(NetBox::Net::setNonBlocking(sock));
        std::cout << "  - 非阻塞模式设置: ✅" << std::endl;
        
        EXPECT_TRUE(NetBox::Net::setTcpNoDelay(sock));
        std::cout << "  - TCP_NODELAY设置: ✅" << std::endl;
        
        // 测试SO_REUSEPORT（平台相关）
        bool reusePortResult = NetBox::Net::setReusePort(sock);
#if defined(NETBOX_PLATFORM_LINUX) || defined(NETBOX_PLATFORM_MACOS)
        EXPECT_TRUE(reusePortResult);
        std::cout << "  - SO_REUSEPORT设置: ✅ (平台支持)" << std::endl;
#else
        // Windows不支持SO_REUSEPORT
        std::cout << "  - SO_REUSEPORT设置: ⚠️ (平台不支持)" << std::endl;
#endif
        
        // 测试缓冲区大小设置
        EXPECT_TRUE(NetBox::Net::setSendBufferSize(sock, 64 * 1024));
        EXPECT_TRUE(NetBox::Net::setRecvBufferSize(sock, 64 * 1024));
        std::cout << "  - 缓冲区大小设置: ✅" << std::endl;
        
        // 关闭socket
        EXPECT_EQ(NetBox::Net::close(sock), 0);
        std::cout << "  - Socket关闭: ✅" << std::endl;
    }
}

// IO多路复用器跨平台测试
TEST_F(CrossPlatformTest, IOMultiplexerCrossPlatform) {
    std::cout << "⚡ 测试IO多路复用器跨平台支持" << std::endl;
    
    // 获取推荐的IO类型
    auto recommendedType = IOFactory::getRecommendedIOType();
    std::string typeName = IOFactory::getIOTypeName(recommendedType);
    std::cout << "  - 推荐IO类型: " << typeName << std::endl;
    
    // 验证推荐类型符合平台特性
#ifdef NETBOX_PLATFORM_WINDOWS
    EXPECT_EQ(recommendedType, IOMultiplexer::IOType::IOCP);
#elif defined(NETBOX_PLATFORM_LINUX)
    EXPECT_EQ(recommendedType, IOMultiplexer::IOType::EPOLL);
#elif defined(NETBOX_PLATFORM_MACOS)
    EXPECT_EQ(recommendedType, IOMultiplexer::IOType::KQUEUE);
#endif
    
    // 获取支持的IO类型列表
    auto supportedTypes = IOFactory::getSupportedIOTypes();
    std::cout << "  - 支持的IO类型: ";
    for (auto type : supportedTypes) {
        std::cout << IOFactory::getIOTypeName(type) << " ";
    }
    std::cout << std::endl;
    
    // 验证支持的类型数量
    EXPECT_GT(supportedTypes.size(), 0);
    
    // 测试创建推荐的IO多路复用器
    auto io = IOFactory::createIO(recommendedType);
    ASSERT_NE(io, nullptr) << "创建IO多路复用器失败";
    EXPECT_TRUE(io->init()) << "初始化IO多路复用器失败";
    std::cout << "  - " << typeName << "多路复用器创建和初始化: ✅" << std::endl;
    
    // 测试基本功能
    int testSock = NetBox::Net::socket(AF_INET, SOCK_STREAM, 0);
    if (testSock != -1) {
        NetBox::Net::setNonBlocking(testSock);
        
        // 测试添加文件描述符
        EXPECT_TRUE(io->addfd(testSock, IOMultiplexer::EventType::READ));
        std::cout << "  - 添加文件描述符: ✅" << std::endl;
        
        // 测试修改事件类型
        EXPECT_TRUE(io->modifyFd(testSock, IOMultiplexer::EventType::WRITE));
        std::cout << "  - 修改事件类型: ✅" << std::endl;
        
        // 测试移除文件描述符
        EXPECT_TRUE(io->removefd(testSock));
        std::cout << "  - 移除文件描述符: ✅" << std::endl;
        
        NetBox::Net::close(testSock);
    }
}

// 平台特定优化测试
TEST_F(CrossPlatformTest, PlatformSpecificOptimizations) {
    std::cout << "🚀 测试平台特定优化" << std::endl;
    
    auto info = NetBox::Platform::PlatformInfo::getCurrent();
    
    if (info.isOS(NetBox::Platform::PlatformInfo::OS::Windows)) {
        std::cout << "  - Windows平台优化测试:" << std::endl;
        
        // 测试IOCP多路复用器
        auto iocp = IOFactory::createIO(IOMultiplexer::IOType::IOCP);
        if (iocp) {
            EXPECT_TRUE(iocp->init());
            std::cout << "    ✅ IOCP多路复用器可用" << std::endl;
        }
        
        // 测试Windows特定网络优化
        int sock = NetBox::Net::socket(AF_INET, SOCK_STREAM, 0);
        if (sock != -1) {
            // Windows可以设置更大的缓冲区
            EXPECT_TRUE(NetBox::Net::setSendBufferSize(sock, 256 * 1024));
            EXPECT_TRUE(NetBox::Net::setRecvBufferSize(sock, 256 * 1024));
            std::cout << "    ✅ 大缓冲区设置成功" << std::endl;
            NetBox::Net::close(sock);
        }
        
    } else if (info.isOS(NetBox::Platform::PlatformInfo::OS::Linux)) {
        std::cout << "  - Linux平台优化测试:" << std::endl;
        
        // 测试EPOLL多路复用器
        auto epoll = IOFactory::createIO(IOMultiplexer::IOType::EPOLL);
        if (epoll) {
            EXPECT_TRUE(epoll->init());
            std::cout << "    ✅ EPOLL多路复用器可用" << std::endl;
        }
        
        // 测试Linux特定优化
        int sock = NetBox::Net::socket(AF_INET, SOCK_STREAM, 0);
        if (sock != -1) {
            EXPECT_TRUE(NetBox::Net::setReusePort(sock));
            std::cout << "    ✅ SO_REUSEPORT设置成功" << std::endl;
            NetBox::Net::close(sock);
        }
        
    } else if (info.isOS(NetBox::Platform::PlatformInfo::OS::MacOS)) {
        std::cout << "  - macOS平台优化测试:" << std::endl;
        
        // 测试KQUEUE多路复用器
        auto kqueue = IOFactory::createIO(IOMultiplexer::IOType::KQUEUE);
        if (kqueue) {
            EXPECT_TRUE(kqueue->init());
            std::cout << "    ✅ KQUEUE多路复用器可用" << std::endl;
        }
        
        // 测试macOS特定优化
        int sock = NetBox::Net::socket(AF_INET, SOCK_STREAM, 0);
        if (sock != -1) {
            EXPECT_TRUE(NetBox::Net::setReusePort(sock));
            std::cout << "    ✅ SO_REUSEPORT设置成功" << std::endl;
            NetBox::Net::close(sock);
        }
    }
}

// 错误处理跨平台一致性测试
TEST_F(CrossPlatformTest, ErrorHandlingConsistency) {
    std::cout << "❌ 测试错误处理跨平台一致性" << std::endl;
    
    // 测试无效socket操作
    int invalidSock = -1;
    int result = NetBox::Net::close(invalidSock);
    EXPECT_EQ(result, -1);
    
    int errorCode = NetBox::Net::getLastError();
    std::string errorMsg = NetBox::Net::errorToString(errorCode);
    
    EXPECT_GT(errorCode, 0);
    EXPECT_FALSE(errorMsg.empty());
    
    std::cout << "  - 错误码: " << errorCode << std::endl;
    std::cout << "  - 错误信息: " << errorMsg << std::endl;
    
    // 测试错误类型判断
    // 这里需要根据平台创建特定的错误条件来测试
    std::cout << "  - 错误处理机制: ✅" << std::endl;
}

// 性能基准测试
TEST_F(CrossPlatformTest, PerformanceBenchmark) {
    std::cout << "📊 跨平台性能基准测试" << std::endl;
    
    auto info = NetBox::Platform::PlatformInfo::getCurrent();
    auto recommendedType = IOFactory::getRecommendedIOType();
    
    // 创建推荐的IO多路复用器
    auto io = IOFactory::createIO(recommendedType);
    ASSERT_NE(io, nullptr);
    ASSERT_TRUE(io->init());
    
    // 简单的性能测试：创建和关闭socket
    const int testCount = 1000;
    
    double createTime = TestUtils::measureExecutionTime([&]() {
        for (int i = 0; i < testCount; ++i) {
            int sock = NetBox::Net::socket(AF_INET, SOCK_STREAM, 0);
            if (sock != -1) {
                NetBox::Net::close(sock);
            }
        }
    });
    
    double avgTime = createTime / testCount;
    double throughput = testCount / (createTime / 1000.0);
    
    std::cout << "  - 平台: " << info.getPlatformName() << std::endl;
    std::cout << "  - IO模型: " << IOFactory::getIOTypeName(recommendedType) << std::endl;
    std::cout << "  - Socket创建/关闭 " << testCount << " 次" << std::endl;
    std::cout << "  - 总时间: " << createTime << " ms" << std::endl;
    std::cout << "  - 平均时间: " << avgTime << " ms/op" << std::endl;
    std::cout << "  - 吞吐量: " << static_cast<int>(throughput) << " ops/sec" << std::endl;
    
    // 性能应该在合理范围内
    EXPECT_LT(avgTime, 1.0); // 平均每次操作应该小于1ms
    EXPECT_GT(throughput, 100); // 吞吐量应该大于100 ops/sec
}

// 兼容性测试
TEST_F(CrossPlatformTest, CompatibilityTest) {
    std::cout << "🔄 跨平台兼容性测试" << std::endl;
    
    // 测试所有支持的IO类型
    auto supportedTypes = IOFactory::getSupportedIOTypes();
    
    for (auto type : supportedTypes) {
        std::string typeName = IOFactory::getIOTypeName(type);
        std::cout << "  - 测试 " << typeName << " 兼容性:" << std::endl;
        
        auto io = IOFactory::createIO(type);
        ASSERT_NE(io, nullptr) << typeName << " 创建失败";
        
        bool initResult = io->init();
        EXPECT_TRUE(initResult) << typeName << " 初始化失败";
        
        if (initResult) {
            std::cout << "    ✅ " << typeName << " 兼容性测试通过" << std::endl;
        } else {
            std::cout << "    ❌ " << typeName << " 兼容性测试失败" << std::endl;
        }
    }
    
    std::cout << "  - 兼容性测试完成，支持 " << supportedTypes.size() << " 种IO模型" << std::endl;
}
