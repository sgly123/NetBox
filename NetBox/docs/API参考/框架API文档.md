# NetBox框架 API参考文档

## 核心类

### ApplicationServer

基础服务器类，所有应用服务器的基类。

```cpp
class ApplicationServer {
public:
    // 构造函数
    ApplicationServer(const std::string& ip, int port, 
                     IOMultiplexer::IOType io_type = IOMultiplexer::IOType::EPOLL,
                     IThreadPool* pool = nullptr);
    
    // 启动服务器
    virtual bool start();
    
    // 停止服务器
    virtual void stop();
    
protected:
    // 初始化协议路由器
    virtual void initializeProtocolRouter() = 0;
    
    // 处理HTTP请求
    virtual std::string handleHttpRequest(const std::string& request, int clientFd) = 0;
    
    // 处理业务逻辑
    virtual std::string handleBusinessLogic(const std::string& command, 
                                          const std::vector<std::string>& args) = 0;
    
    // 解析请求路径
    virtual bool parseRequestPath(const std::string& path, std::string& command,
                                std::vector<std::string>& args) = 0;
};
```

### ProtocolRouter

协议路由器，负责管理和分发不同协议的数据包。

```cpp
class ProtocolRouter {
public:
    // 注册协议
    void registerProtocol(uint32_t protoId, std::shared_ptr<ProtocolBase> protocol);
    
    // 移除协议
    void removeProtocol(uint32_t protoId);
    
    // 数据接收回调
    void onDataReceived(const char* data, size_t len, int client_fd);
};
```

### IOMultiplexer

IO多路复用器接口。

```cpp
class IOMultiplexer {
public:
    enum class IOType {
        SELECT,
        POLL,
        EPOLL
    };
    
    // 添加事件
    virtual bool addEvent(int fd, uint32_t events) = 0;
    
    // 移除事件
    virtual bool removeEvent(int fd) = 0;
    
    // 修改事件
    virtual bool modifyEvent(int fd, uint32_t events) = 0;
    
    // 等待事件
    virtual int wait(std::vector<Event>& activeEvents, int timeout_ms) = 0;
};
```

## 协议类

### ProtocolBase

协议基类，定义了协议的基本接口。

```cpp
class ProtocolBase {
public:
    // 设置包处理回调
    void setPacketCallback(const PacketCallback& cb);
    
    // 设置错误处理回调
    void setErrorCallback(const ErrorCallback& cb);
    
    // 设置流控制
    void setFlowControl(size_t maxReceiveRate, size_t maxSendRate);
    
    // 打包数据
    virtual bool pack(const char* data, size_t len, std::vector<char>& packet) = 0;
    
    // 解包数据
    virtual bool unpack(const char* data, size_t len, std::vector<char>& packet) = 0;
};
```

### SimpleHeaderProtocol

简单头协议实现。

```cpp
class SimpleHeaderProtocol : public ProtocolBase {
public:
    // 包头格式：
    // | Magic Number (4 bytes) | Version (2 bytes) | Length (4 bytes) | Data |
    static const uint32_t MAGIC_NUMBER = 0x12345678;
    static const uint16_t VERSION = 0x0001;
    
    bool pack(const char* data, size_t len, std::vector<char>& packet) override;
    bool unpack(const char* data, size_t len, std::vector<char>& packet) override;
};
```

### HttpProtocol

HTTP协议实现。

```cpp
class HttpProtocol : public ProtocolBase {
public:
    // HTTP请求解析
    bool parseRequest(const std::string& request, HttpRequest& req);
    
    // 构造HTTP响应
    std::string buildResponse(const HttpResponse& resp);
    
    bool pack(const char* data, size_t len, std::vector<char>& packet) override;
    bool unpack(const char* data, size_t len, std::vector<char>& packet) override;
};
```

## 工具类

### Logger

异步日志系统。

```cpp
class Logger {
public:
    // 日志级别
    enum class Level {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };
    
    // 设置日志实例
    static void setInstance(Logger* logger);
    
    // 日志接口
    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
    static void fatal(const std::string& msg);
};
```

### ThreadPool

线程池实现。

```cpp
class ThreadPool {
public:
    // 创建线程池
    ThreadPool(size_t threads);
    
    // 提交任务
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    
    // 获取工作线程数
    size_t getThreadCount() const;
    
    // 获取任务队列大小
    size_t getTaskCount() const;
};
```

## 配置管理

### ConfigManager

配置管理器。

```cpp
class ConfigManager {
public:
    // 加载配置文件
    bool loadConfig(const std::string& filename);
    
    // 获取配置项
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T());
    
    // 设置配置项
    template<typename T>
    void setValue(const std::string& key, const T& value);
    
    // 保存配置
    bool saveConfig(const std::string& filename);
};
```

## 使用示例

### 创建自定义服务器

```cpp
class MyServer : public ApplicationServer {
public:
    MyServer(const std::string& ip, int port) 
        : ApplicationServer(ip, port) {
        // 初始化自定义组件
    }

protected:
    void initializeProtocolRouter() override {
        // 注册协议
        auto proto = std::make_shared<SimpleHeaderProtocol>();
        proto->setPacketCallback([this](const std::vector<char>& packet) {
            handlePacket(packet);
        });
        m_router->registerProtocol(1, proto);
    }
    
    // 实现其他虚函数...
};
```

### 使用线程池

```cpp
// 创建线程池
auto pool = std::make_shared<ThreadPool>(4);

// 提交任务
auto future = pool->submit([](int x) { return x * x; }, 42);

// 获取结果
int result = future.get();
```

### 使用配置管理器

```cpp
// 加载配置
ConfigManager config;
config.loadConfig("config.txt");

// 获取配置项
auto port = config.getValue<int>("server.port", 8888);
auto threads = config.getValue<int>("server.threads", 4);

// 修改配置
config.setValue("server.port", 9999);
config.saveConfig("config.txt");
```

## 注意事项

1. 线程安全
   - 所有公开接口都是线程安全的
   - 回调函数在工作线程中执行
   - 注意避免死锁

2. 错误处理
   - 使用异常处理关键错误
   - 使用错误码处理一般错误
   - 记录详细的错误日志

3. 性能优化
   - 使用零拷贝技术
   - 避免频繁内存分配
   - 合理使用线程池

4. 资源管理
   - 使用RAII管理资源
   - 及时释放不用的连接
   - 避免资源泄漏 