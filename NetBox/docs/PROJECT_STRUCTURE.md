# NetBox 网络框架项目结构

## 📁 核心目录结构

```
NetBox/
├── 📁 NetFramework/           # 核心网络框架
│   ├── 📁 include/           # 头文件
│   │   ├── 📁 base/          # 基础组件
│   │   ├── 📁 IO/            # IO多路复用
│   │   ├── 📁 server/        # 服务器组件
│   │   └── 📁 util/          # 工具类
│   └── 📁 src/               # 源文件
│       ├── 📁 base/          # 基础组件实现
│       ├── 📁 IO/            # IO实现
│       ├── 📁 server/        # 服务器实现
│       └── 📁 util/          # 工具类实现
│
├── 📁 Protocol/              # 协议层
│   ├── 📁 include/           # 协议头文件
│   └── 📁 src/               # 协议实现
│
├── 📁 app/                   # 应用层
│   ├── 📁 include/           # 应用头文件
│   └── 📁 src/               # 应用实现
│
├── 📁 config/                # 配置文件
├── 📁 build/                 # 编译输出
├── 📁 docs/                  # 文档
└── 📁 examples/              # 示例代码
```

## 🎯 两大使用场景

### 1. Echo 服务器场景
**文件组成:**
- `config_echo.yaml` - Echo服务器配置
- `EchoServer` - Echo应用服务器
- `SimpleHeaderProtocol` - 简单协议头协议
- `echo_client.cpp` - Echo客户端

**功能:** 回显服务器，客户端发送什么就回显什么

### 2. Redis 服务器场景  
**文件组成:**
- `config_redis_app.yaml` - Redis服务器配置
- `RedisApplicationServer` - Redis应用服务器
- `PureRedisProtocol` - 纯RESP协议
- `smart_netbox_redis_client.cpp` - 智能Redis客户端

**功能:** 完整的Redis数据库服务器，支持标准Redis命令

## 🔧 核心组件说明

### 网络层 (NetFramework)
- **TcpServer**: 通用TCP服务器，支持智能心跳控制
- **IOMultiplexer**: IO多路复用抽象层 (epoll/select/poll)
- **ThreadPool**: 线程池管理

### 协议层 (Protocol)
- **ProtocolRouter**: 协议路由器，自动识别协议类型
- **SimpleHeaderProtocol**: 4字节头部协议 (用于Echo)
- **PureRedisProtocol**: 纯RESP协议 (用于Redis)

### 应用层 (app)
- **ApplicationServer**: 应用服务器基类
- **EchoServer**: Echo应用实现
- **RedisApplicationServer**: Redis应用实现

## 🚀 智能特性

### 智能心跳控制
- **Echo场景**: 启用心跳包，保持连接活跃
- **Redis场景**: 自动禁用心跳包，避免与RESP协议冲突

### 协议自动识别
- 根据数据包头部自动识别协议类型
- 无需手动配置，自动路由到对应处理器

### 多IO模型支持
- 支持 epoll (Linux高性能)
- 支持 select (跨平台兼容)
- 支持 poll (中等性能)

## 📊 性能特点

- **高并发**: 基于epoll的事件驱动架构
- **低延迟**: 零拷贝数据传输
- **高可靠**: 完整的错误处理和日志系统
- **易扩展**: 插件化的协议和应用架构
