# UDP支持文档

## 🚀 概述

NetBox框架现已支持UDP协议，提供高性能的无连接数据报服务。UDP支持是在保持与现有TCP架构一致性的基础上添加的，使得开发者可以轻松构建UDP应用服务器。

## 🏗️ 架构设计

### 核心组件

```
UdpServer (基础UDP服务器)
    ↓
UdpEchoServer (UDP Echo应用服务器)
    ↓
UdpEchoPlugin (插件集成)
    ↓
ApplicationRegistry (框架注册)
```

### 主要特性

- **🔄 事件驱动**: 基于IO多路复用的异步事件处理
- **⚡ 高性能**: 支持Epoll/IOCP/Kqueue多种IO模式
- **📊 统计监控**: 内置数据包统计和客户端管理
- **🔧 易于扩展**: 清晰的接口设计，支持自定义UDP应用
- **🎯 插件化**: 无缝集成到现有框架的插件系统

## 📚 API参考

### UdpServer类

```cpp
class UdpServer {
public:
    // 构造函数
    UdpServer(const std::string& ip, int port, IOMultiplexer::IOType io_type);
    
    // 设置回调函数
    void setOnDatagram(OnDatagramCallback cb);
    void setOnError(OnErrorCallback cb);
    
    // 生命周期管理
    bool start();
    void stop();
    
    // 数据发送
    bool sendTo(const sockaddr_in& addr, const std::string& data);
    bool sendTo(const std::string& ip, int port, const std::string& data);
    
    // 统计信息
    const UdpStats& getStats() const;
    
protected:
    // 可重写的事件处理方法
    virtual void onDatagramReceived(const sockaddr_in& from, const char* data, size_t len);
    virtual void onError(int error_code, const std::string& message);
};
```

### 回调函数类型

```cpp
// 数据报接收回调
using OnDatagramCallback = std::function<void(const sockaddr_in&, const std::string&)>;

// 错误处理回调
using OnErrorCallback = std::function<void(int, const std::string&)>;
```

### 统计信息结构

```cpp
struct UdpStats {
    std::atomic<uint64_t> packets_received{0};
    std::atomic<uint64_t> packets_sent{0};
    std::atomic<uint64_t> bytes_received{0};
    std::atomic<uint64_t> bytes_sent{0};
    std::atomic<uint64_t> send_errors{0};
    std::atomic<uint64_t> recv_errors{0};
};
```

## 🛠️ 使用方法

### 1. 配置UDP服务器

在配置文件中设置UDP服务器：

```json
{
    "server": {
        "ip": "0.0.0.0",
        "port": 8081,
        "io_type": "EPOLL"
    },
    "application": {
        "type": "udp_echo"
    }
}
```

### 2. 启动UDP Echo服务器

```bash
# 使用配置文件启动
./NetBox config/udp_echo_config.json

# 或者直接指定参数
./NetBox --ip 0.0.0.0 --port 8081 --type udp_echo
```

### 3. 测试UDP服务器

```bash
# 编译测试客户端
g++ -o udp_client examples/udp_client_test.cpp

# 运行交互测试
./udp_client 127.0.0.1 8081

# 运行性能测试
./udp_client 127.0.0.1 8081
# 选择模式2，输入测试数量
```

## 🔧 开发自定义UDP应用

### 继承UdpServer

```cpp
class MyUdpServer : public UdpServer {
public:
    MyUdpServer(const std::string& ip, int port, IOMultiplexer::IOType io_type)
        : UdpServer(ip, port, io_type) {}

protected:
    void onDatagramReceived(const sockaddr_in& from, const char* data, size_t len) override {
        // 处理接收到的数据包
        std::string message(data, len);
        
        // 自定义业务逻辑
        std::string response = processMessage(message);
        
        // 发送响应
        sendTo(from, response);
    }
    
    void onError(int error_code, const std::string& message) override {
        // 处理错误
        Logger::error("UDP错误: " + message);
    }
    
private:
    std::string processMessage(const std::string& message) {
        // 实现你的业务逻辑
        return "Processed: " + message;
    }
};
```

### 注册为插件

```cpp
// 在插件文件中注册
static bool registerMyUdpServer() {
    return ApplicationRegistry::getInstance().registerApplication("my_udp", 
        [](const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool) {
            return std::make_unique<MyUdpServerAdapter>(ip, port, io_type, pool);
        });
}

static bool g_registered = registerMyUdpServer();
```

## 📊 性能特性

### 支持的IO模式

| 平台 | 推荐IO模式 | 支持的模式 |
|------|------------|------------|
| Linux | EPOLL | EPOLL, POLL, SELECT |
| Windows | IOCP | IOCP, SELECT |
| macOS/BSD | KQUEUE | KQUEUE, SELECT |

### 性能指标

- **吞吐量**: 支持每秒处理数万个UDP数据包
- **延迟**: 亚毫秒级的数据包处理延迟
- **并发**: 支持数千个并发客户端
- **内存**: 低内存占用，高效的缓冲区管理

## 🔍 错误处理

### 错误类型

```cpp
enum UdpErrorType {
    BIND_FAILED = 1,      // 绑定失败
    SEND_FAILED = 2,      // 发送失败
    RECV_FAILED = 3,      // 接收失败
    PACKET_TOO_LARGE = 4, // 数据包过大
    INVALID_ADDRESS = 5,  // 无效地址
    SOCKET_ERROR = 6      // Socket错误
};
```

### 错误处理最佳实践

```cpp
void onError(int error_code, const std::string& message) override {
    switch (error_code) {
        case UdpErrorType::BIND_FAILED:
        case UdpErrorType::SOCKET_ERROR:
            // 严重错误，考虑重启服务
            Logger::error("严重错误: " + message);
            // 实现重启逻辑
            break;
            
        case UdpErrorType::SEND_FAILED:
        case UdpErrorType::RECV_FAILED:
            // 网络错误，记录并继续
            Logger::warn("网络错误: " + message);
            break;
            
        default:
            Logger::info("其他错误: " + message);
            break;
    }
}
```

## 🎯 最佳实践

### 1. 数据包大小

- UDP最大负载: 65507字节
- 建议数据包大小: < 1472字节 (避免IP分片)
- 局域网环境: < 1500字节
- 广域网环境: < 576字节

### 2. 错误处理

- 始终检查发送和接收的返回值
- 实现合适的超时机制
- 处理网络断开和重连情况

### 3. 性能优化

```cpp
// 批量处理数据包
void handleBatchReceive() {
    while (processRecv()) {
        // 持续处理直到没有数据
    }
}

// 非阻塞发送队列
if (!sendTo(addr, data)) {
    // 加入发送队列，稍后重试
    m_sendQueue.push({addr, data});
}
```

### 4. 监控和统计

```cpp
// 定期打印统计信息
void printPeriodicStats() {
    const auto& stats = getStats();
    Logger::info("UDP统计 - 接收: " + std::to_string(stats.packets_received) + 
                " 发送: " + std::to_string(stats.packets_sent));
}
```

## 🔗 相关链接

- [NetBox主文档](../README.md)
- [TCP服务器文档](TCP_SUPPORT.md)
- [配置文件指南](CONFIG_GUIDE.md)
- [性能优化指南](PERFORMANCE_GUIDE.md)

## 📝 更新日志

### v1.0.0 (2024-XX-XX)
- ✅ 添加基础UDP服务器支持
- ✅ 实现UDP Echo服务器示例
- ✅ 集成到插件化框架
- ✅ 提供完整的API文档和示例 