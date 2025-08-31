# Mini Redis 快速参考卡片

## 🚀 快速启动
```bash
# 1. 编译项目
cd NetBox/build && make -j$(nproc)

# 2. 启动 Redis 服务器
./NetBox ../config_redis.yaml

# 3. 连接客户端
./redis_client
```

## 📋 命令速查表

### String 命令
```bash
SET key value          # 设置值
GET key               # 获取值
DEL key [key...]      # 删除键
EXISTS key            # 检查存在
EXPIRE key seconds    # 设置过期
```

### List 命令
```bash
LPUSH key value...    # 左侧插入
RPUSH key value...    # 右侧插入
LPOP key             # 左侧弹出
RPOP key             # 右侧弹出
LRANGE key start stop # 范围查询
```

### Hash 命令
```bash
HSET key field value  # 设置字段
HGET key field       # 获取字段
HDEL key field...    # 删除字段
HKEYS key            # 获取所有字段
```

### 通用命令
```bash
KEYS pattern         # 查找键
FLUSHALL            # 清空数据
SAVE                # 保存数据
LOAD                # 加载数据
```

## 🎯 使用示例

### 基础操作
```bash
redis> SET name "张三"
+OK
redis> GET name
+张三
redis> EXPIRE name 60
+1
```

### 列表操作
```bash
redis> LPUSH tasks "写代码" "吃饭"
+2
redis> LRANGE tasks 0 -1
+1) 吃饭
+2) 写代码
```

### 哈希操作
```bash
redis> HSET user:1 name "张三"
+1
redis> HSET user:1 age "25"
+1
redis> HKEYS user:1
+1) name
+2) age
```

## ⚙️ 配置要点

### config_redis.yaml
```yaml
application:
  type: mini_redis    # 必须设置为 mini_redis

network:
  ip: 127.0.0.1      # 监听地址
  port: 6379         # Redis 默认端口

threading:
  worker_threads: 8   # 工作线程数
```

## 🔧 技术特性

### 数据类型
- **String**: 字符串键值对
- **List**: 双端队列，支持 LPUSH/RPUSH
- **Hash**: 哈希表，类似对象属性

### 线程安全
- **读写锁**: 支持多读单写
- **原子操作**: 每个命令都是原子的
- **数据一致性**: 严格保证数据一致性

### 持久化
- **格式**: 简化的文本格式
- **保存**: SAVE 命令手动保存
- **加载**: 启动时自动加载

## 🎯 面试要点

### 核心亮点
1. **数据结构选择**: unordered_map 的 O(1) 性能
2. **并发控制**: 读写锁提升并发性能
3. **内存管理**: TTL 过期机制
4. **协议兼容**: 简化版 RESP 协议

### 技术深度
- **STL 容器**: 高级数据结构应用
- **C++11/14**: 现代 C++ 特性使用
- **系统编程**: 网络 IO + 多线程
- **架构设计**: 插件化 + 配置驱动

### 扩展方向
- **性能优化**: 内存池、零拷贝
- **功能扩展**: Set、ZSet 数据类型
- **分布式**: 主从复制、集群模式
- **监控**: 性能指标、慢查询日志

## 📊 项目数据

### 代码量
- **核心代码**: 850+ 行
- **测试工具**: 200+ 行
- **总计**: 1,150+ 行

### 性能指标
- **数据类型**: 3 种 (String/List/Hash)
- **支持命令**: 18+ 个
- **并发模型**: 多线程 + 读写锁
- **协议**: 简化版 RESP

### 技术栈
- **语言**: C++17
- **容器**: STL unordered_map/list
- **并发**: shared_mutex 读写锁
- **网络**: TCP + Epoll
- **配置**: YAML 格式

## 🚀 快速测试

### 功能测试脚本
```bash
#!/bin/bash
# 启动服务器
./NetBox ../config_redis.yaml &
SERVER_PID=$!

# 等待启动
sleep 2

# 测试命令
echo "SET test_key test_value" | ./redis_client
echo "GET test_key" | ./redis_client
echo "KEYS *" | ./redis_client

# 清理
kill $SERVER_PID
```

### 性能测试
```bash
# 批量插入测试
for i in {1..1000}; do
    echo "SET key$i value$i" | ./redis_client
done

# 批量查询测试
for i in {1..1000}; do
    echo "GET key$i" | ./redis_client
done
```

## 💡 故障排除

### 常见问题
1. **连接失败**: 检查端口是否被占用
2. **命令无响应**: 检查服务器是否正常启动
3. **数据丢失**: 确认是否执行了 SAVE 命令

### 调试技巧
- **查看日志**: 服务器启动日志包含详细信息
- **检查配置**: 确认 application.type 为 mini_redis
- **网络测试**: 使用 telnet 测试连接

---

**💡 提示**: 这个 Mini Redis 实现展示了数据库原理、并发编程、系统设计等多个技术领域的深度理解，是校招面试中的强力加分项！
