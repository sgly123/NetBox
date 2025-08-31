# HTTP协议扩展快速参考

## 🎯 核心要点

### 1. 问题与解决方案
| 问题 | 解决方案 |
|------|----------|
| HTTP是文本协议，没有二进制协议头 | 智能协议识别：检查前几个字节是否为HTTP方法 |
| 需要同时支持HTTP和SimpleHeader协议 | 协议分发器优先检查HTTP，再处理二进制协议 |
| HTTP响应需要符合框架要求 | 添加4字节协议头，但发送时去掉协议头 |

### 2. 关键代码位置
```
Protocol/include/HttpProtocol.h     # HTTP协议定义
Protocol/src/HttpProtocol.cpp       # HTTP协议实现
Protocol/src/ProtocolRouter.cpp     # 协议分发器修改
app/src/server.cpp                  # 服务器注册HTTP协议
CMakeLists.txt                      # 构建配置
```

## 🔧 实现步骤

### 步骤1：创建HTTP协议类
```cpp
// Protocol/include/HttpProtocol.h
class HttpProtocol : public ProtocolBase {
public:
    static constexpr uint32_t ID = 2;  // 协议ID
    enum class Method { GET, POST, PUT, DELETE, ... };
    enum class StatusCode { OK = 200, BAD_REQUEST = 400, ... };
    
    size_t onDataReceived(const char* data, size_t len) override;
    bool pack(const char* data, size_t len, std::vector<char>& out) override;
    uint32_t getProtocolId() const override { return ID; }
};
```

### 步骤2：修改协议分发器
```cpp
// Protocol/src/ProtocolRouter.cpp
size_t ProtocolRouter::onDataReceived(int client_fd, const char* data, size_t len) {
    // 优先检查HTTP请求
    if (len >= 4) {
        std::string start(reinterpret_cast<const char*>(data), std::min(len, size_t(10)));
        if (start.find("GET ") == 0 || start.find("POST ") == 0 || ...) {
            auto it = protocols_.find(2); // HTTP协议ID
            if (it != protocols_.end()) {
                return it->second->onDataReceived(data, len);
            }
        }
    }
    
    // 处理二进制协议（需要4字节协议头）
    // ... 原有逻辑
}
```

### 步骤3：注册HTTP协议
```cpp
// app/src/server.cpp
EchoServer::EchoServer(...) {
    // 注册SimpleHeader协议
    auto simpleProto = std::make_shared<SimpleHeaderProtocol>();
    m_router.registerProtocol(simpleProto->getProtocolId(), simpleProto);
    
    // 注册HTTP协议
    auto httpProto = std::make_shared<HttpProtocol>();
    httpProto->setPacketCallback([this](const std::vector<char>& packet) {
        this->onHttpPacketReceived(packet);
    });
    m_router.registerProtocol(httpProto->getProtocolId(), httpProto);
}
```

### 步骤4：HTTP业务处理
```cpp
// app/src/server.cpp
void EchoServer::onHttpPacketReceived(const std::vector<char>& packet) {
    // 解析HTTP请求
    std::string httpRequest(packet.begin(), packet.end());
    
    // 构造HTTP响应
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html; charset=utf-8\r\n";
    response += "Server: NetBox/1.0\r\n";
    response += "Connection: close\r\n";
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "\r\n";
    response += body;
    
    // 发送响应
    send(m_currentClientFd, response.data(), response.size(), 0);
}
```

## 🧪 测试命令

### 编译
```bash
cd build && make NetBox
```

### 启动服务器
```bash
./NetBox > server.log 2>&1 &
```

### 开放端口
```bash
sudo ufw allow 8888/tcp
```

### 测试HTTP
```bash
curl -v http://192.168.88.135:8888/
```

## 📊 技术亮点

### 1. 协议自动识别
- HTTP请求：检查前几个字节是否为HTTP方法
- 二进制协议：需要4字节协议头
- 智能路由：自动选择正确的协议处理器

### 2. 架构优势
- **分层设计**：网络层、协议层、应用层分离
- **扩展性强**：易于添加新协议
- **向后兼容**：不影响现有功能

### 3. 实现细节
- **流量控制**：支持请求大小和速率限制
- **错误处理**：完善的错误回调机制
- **内存管理**：使用PooledBuffer优化性能

## 🚀 扩展方向

### 1. 功能扩展
- 静态文件服务
- RESTful API
- WebSocket支持
- HTTPS支持

### 2. 协议扩展
- MQTT协议
- gRPC协议
- 自定义协议

### 3. 性能优化
- 连接池
- 响应缓存
- 负载均衡

## 💡 关键理解点

### 1. 为什么需要智能协议识别？
HTTP是文本协议，没有二进制协议头，但我们的框架期望4字节协议头。通过检查数据开头是否为HTTP方法来解决这个问题。

### 2. 协议分发器的工作流程
1. 检查是否为HTTP请求（文本协议）
2. 如果是HTTP，直接路由到HTTP处理器
3. 如果不是，按原有逻辑处理（需要协议头）

### 3. HTTP响应的处理
- 内部：添加协议头以符合框架要求
- 外部：发送纯HTTP响应给客户端

### 4. 架构设计的好处
- **解耦**：协议层和应用层分离
- **可扩展**：易于添加新协议
- **可维护**：清晰的代码结构

---

*这个快速参考文档帮助您快速理解HTTP协议扩展的核心要点和实现思路。* 