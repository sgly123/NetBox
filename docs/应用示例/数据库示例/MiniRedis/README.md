# MiniRedis

基于NetBox框架实现的简化版Redis服务器。

## 功能特点

1. **数据类型**
   - String
   - List
   - Hash
   - Set
   - Sorted Set

2. **基本操作**
   - GET/SET
   - LPUSH/RPUSH/LPOP/RPOP
   - HSET/HGET
   - SADD/SREM
   - ZADD/ZRANGE

3. **高级特性**
   - 过期时间
   - 事务支持
   - 持久化
   - 主从复制

## 实现要点

### 1. 协议设计

```cpp
// Redis协议解析器
class RedisProtocol : public ProtocolBase {
public:
    // RESP (Redis Serialization Protocol) 格式
    enum class Type {
        SIMPLE_STRING, // +
        ERROR,        // -
        INTEGER,      // :
        BULK_STRING,  // $
        ARRAY         // *
    };
    
    // 解析RESP格式
    bool parse(const std::string& input, RedisCommand& cmd);
    
    // 构造RESP响应
    std::string buildResponse(const RedisResponse& resp);
};
```

### 2. 命令处理

```cpp
class RedisServer : public ApplicationServer {
public:
    RedisServer(const std::string& ip, int port);
    
protected:
    // 初始化命令表
    void initializeCommands();
    
    // 处理命令
    RedisResponse executeCommand(const RedisCommand& cmd);
    
private:
    // 命令处理函数映射表
    std::unordered_map<std::string, 
        std::function<RedisResponse(const RedisCommand&)>> m_commands;
    
    // 数据存储
    std::shared_ptr<DataStore> m_store;
};
```

### 3. 数据存储

```cpp
// 数据存储接口
class DataStore {
public:
    // String操作
    virtual bool set(const std::string& key, const std::string& value) = 0;
    virtual std::string get(const std::string& key) = 0;
    
    // List操作
    virtual bool lpush(const std::string& key, const std::string& value) = 0;
    virtual std::string lpop(const std::string& key) = 0;
    
    // Hash操作
    virtual bool hset(const std::string& key, const std::string& field, 
                     const std::string& value) = 0;
    virtual std::string hget(const std::string& key, const std::string& field) = 0;
    
    // 过期时间
    virtual void expire(const std::string& key, int seconds) = 0;
    virtual bool isExpired(const std::string& key) = 0;
};
```

## 使用说明

### 1. 编译和运行

```bash
# 编译
cd build
cmake ..
make miniredis

# 运行服务器
./miniredis_server

# 使用客户端
./miniredis_cli
```

### 2. 配置说明

编辑 `config/miniredis.json`:
```json
{
    "server": {
        "host": "127.0.0.1",
        "port": 6379,
        "max_clients": 10000
    },
    "storage": {
        "max_memory": "1gb",
        "persistence": true,
        "db_filename": "dump.rdb"
    }
}
```

### 3. 支持的命令

```bash
# String操作
SET key value
GET key
DEL key
EXISTS key
EXPIRE key seconds

# List操作
LPUSH key value
RPUSH key value
LPOP key
RPOP key
LLEN key

# Hash操作
HSET key field value
HGET key field
HDEL key field
HLEN key

# Set操作
SADD key member
SREM key member
SMEMBERS key
SCARD key

# Sorted Set操作
ZADD key score member
ZRANGE key start stop
ZREM key member
ZCARD key
```

## 测试方法

### 1. 单元测试

```bash
# 运行所有测试
./test_miniredis

# 运行特定测试
./test_miniredis --gtest_filter=StringTest.*
```

### 2. 性能测试

```bash
# 基准测试
./benchmark --clients 50 --requests 100000

# 内存测试
./benchmark --memory-test
```

### 3. 测试用例

1. **基础功能测试**
   - 数据类型操作
   - 过期时间
   - 持久化
   - 事务

2. **并发测试**
   - 多客户端并发
   - 事务并发
   - 主从同步

3. **故障测试**
   - 服务器崩溃恢复
   - 网络断开重连
   - 数据一致性检查

## 扩展建议

1. **新数据类型**
   - 实现新的数据结构
   - 添加命令处理器
   - 更新持久化逻辑
   - 编写测试用例

2. **集群支持**
   - 实现分片机制
   - 添加节点通信
   - 实现数据迁移
   - 处理故障转移

3. **监控系统**
   - 性能指标收集
   - 状态监控
   - 报警机制
   - 可视化界面

## 性能优化

1. **内存优化**
   - 压缩数据结构
   - 共享对象池
   - 惰性删除
   - 内存限制

2. **并发优化**
   - 多线程处理
   - 读写分离
   - 锁优化
   - 事件循环

3. **持久化优化**
   - 增量保存
   - 后台保存
   - 写入缓冲
   - 压缩存储

## 调试技巧

1. **命令行工具**
```bash
# 监控命令
./miniredis_cli monitor

# 慢查询分析
./miniredis_cli slowlog get

# 内存分析
./miniredis_cli memory doctor
```

2. **日志分析**
```bash
# 查看慢查询
grep "SLOW" logs/miniredis.log

# 查看错误
grep "ERROR" logs/miniredis.log
```

3. **性能分析**
```bash
# CPU分析
perf record -g ./miniredis_server
perf report

# 内存分析
valgrind --tool=massif ./miniredis_server
```

## 常见问题

1. **内存占用过高**
   - 检查大key
   - 启用压缩
   - 设置过期时间
   - 使用maxmemory策略

2. **性能下降**
   - 分析慢查询
   - 优化数据结构
   - 调整线程配置
   - 使用pipeline

3. **数据丢失**
   - 检查持久化配置
   - 验证同步状态
   - 增加备份策略
   - 监控磁盘空间 