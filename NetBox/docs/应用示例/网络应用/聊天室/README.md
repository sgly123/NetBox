# 多人聊天室

基于NetBox框架实现的多人聊天室系统。

## 功能特点

1. **基础功能**
   - 群聊消息
   - 私聊消息
   - 在线状态
   - 历史消息

2. **高级功能**
   - 消息格式化
   - 文件传输
   - 图片分享
   - 表情支持

3. **管理功能**
   - 用户管理
   - 房间管理
   - 消息审核
   - 黑名单

## 实现要点

### 1. 协议设计

```cpp
// 聊天协议
class ChatProtocol : public ProtocolBase {
public:
    // 消息类型
    enum class MsgType {
        TEXT,      // 文本消息
        IMAGE,     // 图片消息
        FILE,      // 文件消息
        SYSTEM,    // 系统消息
        STATUS     // 状态更新
    };
    
    // 消息结构
    struct Message {
        uint32_t id;           // 消息ID
        uint32_t from;         // 发送者ID
        uint32_t to;           // 接收者ID（0表示群发）
        MsgType type;          // 消息类型
        std::string content;   // 消息内容
        uint64_t timestamp;    // 时间戳
    };
    
    // 协议方法
    bool packMessage(const Message& msg, std::vector<char>& packet);
    bool unpackMessage(const std::vector<char>& packet, Message& msg);
};
```

### 2. 服务器实现

```cpp
class ChatServer : public ApplicationServer {
public:
    ChatServer(const std::string& ip, int port);
    
protected:
    // 初始化
    void initializeProtocolRouter() override;
    
    // 消息处理
    void handleMessage(const ChatProtocol::Message& msg);
    
    // 用户管理
    void handleUserLogin(uint32_t userId, const std::string& token);
    void handleUserLogout(uint32_t userId);
    
    // 房间管理
    void createRoom(const std::string& name);
    void joinRoom(uint32_t userId, uint32_t roomId);
    void leaveRoom(uint32_t userId, uint32_t roomId);
    
private:
    // 用户管理
    std::unordered_map<uint32_t, User> m_users;
    std::unordered_map<uint32_t, Room> m_rooms;
    
    // 消息存储
    MessageStore m_msgStore;
};
```

### 3. 消息存储

```cpp
class MessageStore {
public:
    // 存储消息
    void storeMessage(const ChatProtocol::Message& msg);
    
    // 获取历史消息
    std::vector<ChatProtocol::Message> getHistory(uint32_t userId, 
                                                 uint32_t count = 50);
    
    // 搜索消息
    std::vector<ChatProtocol::Message> searchMessages(const std::string& keyword);
    
private:
    // 消息缓存
    std::deque<ChatProtocol::Message> m_cache;
    
    // 持久化存储
    SQLiteStorage m_storage;
};
```

## 使用说明

### 1. 编译和运行

```bash
# 编译
cd build
cmake ..
make chat_server

# 运行服务器
./chat_server

# 运行客户端
./chat_client
```

### 2. 配置说明

编辑 `config/chat.json`:
```json
{
    "server": {
        "host": "127.0.0.1",
        "port": 8888,
        "max_clients": 10000
    },
    "chat": {
        "max_message_length": 1024,
        "history_size": 100,
        "file_size_limit": 10485760
    },
    "storage": {
        "type": "sqlite",
        "path": "chat.db"
    }
}
```

### 3. 客户端命令

```bash
# 登录
/login <username> <password>

# 发送消息
/msg <message>

# 私聊
/pm <user> <message>

# 加入房间
/join <room>

# 离开房间
/leave

# 发送文件
/file <filepath>

# 查看在线用户
/users

# 查看历史消息
/history [count]
```

## 测试方法

### 1. 单元测试

```bash
# 运行所有测试
./test_chat

# 运行特定测试
./test_chat --gtest_filter=MessageTest.*
```

### 2. 压力测试

```bash
# 模拟多用户
python test_chat_stress.py --users 1000 --duration 3600

# 消息压力测试
python test_message_flood.py --rate 1000
```

### 3. 测试用例

1. **功能测试**
   - 消息发送接收
   - 群聊私聊
   - 文件传输
   - 历史记录

2. **性能测试**
   - 并发连接
   - 消息吞吐量
   - 响应时间
   - 资源占用

3. **容错测试**
   - 断线重连
   - 消息重传
   - 异常处理
   - 负载均衡

## 扩展建议

1. **消息类型扩展**
   - 添加新的消息类型
   - 实现消息处理器
   - 更新协议定义
   - 扩展客户端支持

2. **存储方案扩展**
   - 支持分布式存储
   - 实现消息分片
   - 添加缓存层
   - 优化查询性能

3. **功能扩展**
   - 添加语音消息
   - 实现视频聊天
   - 支持消息回复
   - 添加@功能

## 性能优化

1. **消息处理**
   - 消息队列
   - 批量处理
   - 压缩传输
   - 增量同步

2. **连接管理**
   - 连接池
   - 心跳检测
   - 会话管理
   - 负载均衡

3. **存储优化**
   - 索引优化
   - 冷热分离
   - 定期清理
   - 数据压缩

## 调试技巧

1. **日志分析**
```bash
# 查看实时日志
tail -f logs/chat.log

# 分析错误日志
grep "ERROR" logs/chat.log | sort | uniq -c
```

2. **性能监控**
```bash
# 监控连接数
./chat_admin --connections

# 监控消息率
./chat_admin --message-rate
```

3. **问题诊断**
```bash
# 检查网络状态
./chat_admin --network-status

# 检查存储状态
./chat_admin --storage-status
```

## 常见问题

1. **消息延迟**
   - 检查网络状态
   - 优化消息队列
   - 调整处理线程
   - 监控系统负载

2. **连接断开**
   - 检查心跳超时
   - 分析网络状态
   - 优化重连策略
   - 完善错误处理

3. **内存占用**
   - 优化消息缓存
   - 及时清理资源
   - 控制连接数量
   - 监控内存使用 