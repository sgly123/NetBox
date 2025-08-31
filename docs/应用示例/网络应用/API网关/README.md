# API网关

基于NetBox框架实现的API网关服务器。

## 功能特点

1. **路由功能**
   - 请求转发
   - 负载均衡
   - 服务发现
   - 协议转换

2. **安全功能**
   - 认证授权
   - 限流控制
   - 黑白名单
   - SSL/TLS

3. **监控功能**
   - 请求统计
   - 性能监控
   - 日志追踪
   - 告警通知

## 实现要点

### 1. 网关核心

```cpp
class ApiGateway : public ApplicationServer {
public:
    ApiGateway(const std::string& ip, int port);
    
protected:
    // 初始化
    void initializeProtocolRouter() override;
    
    // 请求处理
    void handleRequest(const HttpRequest& request);
    
    // 路由转发
    void forwardRequest(const HttpRequest& request, const ServiceInfo& service);
    
    // 响应处理
    void handleResponse(const HttpResponse& response);
    
private:
    // 服务注册表
    ServiceRegistry m_registry;
    
    // 负载均衡器
    LoadBalancer m_loadBalancer;
    
    // 限流器
    RateLimiter m_rateLimiter;
};
```

### 2. 服务注册

```cpp
class ServiceRegistry {
public:
    // 服务注册
    void registerService(const ServiceInfo& service);
    
    // 服务发现
    std::vector<ServiceInfo> discoverService(const std::string& name);
    
    // 健康检查
    void checkHealth();
    
    // 服务下线
    void deregisterService(const std::string& id);
    
private:
    std::unordered_map<std::string, std::vector<ServiceInfo>> m_services;
    std::mutex m_mutex;
};
```

### 3. 负载均衡

```cpp
class LoadBalancer {
public:
    // 负载均衡策略
    enum class Strategy {
        ROUND_ROBIN,
        LEAST_CONN,
        RANDOM,
        WEIGHTED
    };
    
    // 选择服务实例
    ServiceInfo selectService(const std::vector<ServiceInfo>& services);
    
    // 更新服务状态
    void updateStatus(const ServiceInfo& service, const ServiceStatus& status);
    
private:
    Strategy m_strategy;
    std::unordered_map<std::string, ServiceStatus> m_status;
};
```

## 使用说明

### 1. 编译和运行

```bash
# 编译
cd build
cmake ..
make api_gateway

# 运行网关
./api_gateway

# 运行管理工具
./gateway_admin
```

### 2. 配置说明

编辑 `config/gateway.json`:
```json
{
    "server": {
        "host": "0.0.0.0",
        "port": 8080,
        "workers": 4
    },
    "services": [
        {
            "name": "user-service",
            "endpoints": [
                "http://localhost:8081",
                "http://localhost:8082"
            ],
            "weight": 100,
            "health_check": "/health"
        }
    ],
    "security": {
        "jwt_secret": "your-secret-key",
        "rate_limit": {
            "requests": 1000,
            "per_seconds": 60
        }
    }
}
```

### 3. API说明

#### 服务注册
```http
POST /api/v1/services
Content-Type: application/json

{
    "name": "user-service",
    "endpoint": "http://localhost:8081",
    "weight": 100,
    "health_check": "/health"
}
```

#### 服务发现
```http
GET /api/v1/services?name=user-service
```

#### 请求转发
```http
# 原始请求
GET /user-service/users/1

# 转发到
GET http://localhost:8081/users/1
```

## 测试方法

### 1. 单元测试

```bash
# 运行所有测试
./test_gateway

# 运行特定测试
./test_gateway --gtest_filter=RoutingTest.*
```

### 2. 性能测试

```bash
# 压力测试
ab -n 100000 -c 100 http://localhost:8080/user-service/users

# 延迟测试
./test_latency.py --duration 3600
```

### 3. 测试用例

1. **功能测试**
   - 路由转发
   - 负载均衡
   - 服务发现
   - 限流控制

2. **性能测试**
   - 并发处理
   - 响应时间
   - 吞吐量
   - CPU使用率

3. **故障测试**
   - 服务故障
   - 网络故障
   - 限流生效
   - 熔断恢复

## 扩展建议

1. **协议扩展**
   - 支持WebSocket
   - 支持gRPC
   - 支持GraphQL
   - 自定义协议

2. **安全扩展**
   - OAuth2集成
   - WAF功能
   - 数据加密
   - 审计日志

3. **监控扩展**
   - Prometheus集成
   - ELK集成
   - APM集成
   - 告警系统

## 性能优化

1. **请求处理**
   - 连接复用
   - 请求合并
   - 响应缓存
   - 异步处理

2. **负载均衡**
   - 动态权重
   - 会话保持
   - 预热机制
   - 平滑切换

3. **资源管理**
   - 连接池化
   - 内存复用
   - 定时清理
   - 资源限制

## 调试技巧

1. **请求追踪**
```bash
# 开启调试模式
./api_gateway --debug

# 查看请求日志
tail -f logs/access.log
```

2. **性能分析**
```bash
# 查看性能指标
./gateway_admin --metrics

# 导出性能报告
./gateway_admin --report
```

3. **故障诊断**
```bash
# 检查服务状态
./gateway_admin --health

# 查看路由表
./gateway_admin --routes
```

## 常见问题

1. **性能问题**
   - 优化路由算法
   - 调整线程配置
   - 启用响应缓存
   - 使用连接池

2. **服务发现**
   - 检查注册中心
   - 验证服务配置
   - 调整健康检查
   - 优化发现策略

3. **安全问题**
   - 更新安全配置
   - 检查认证授权
   - 调整限流策略
   - 分析审计日志

## 最佳实践

1. **部署建议**
   - 使用容器化
   - 配置高可用
   - 实现平滑升级
   - 做好监控告警

2. **安全建议**
   - 启用HTTPS
   - 配置防火墙
   - 实施访问控制
   - 定期安全审计

3. **运维建议**
   - 自动化部署
   - 配置版本控制
   - 做好备份恢复
   - 制定应急预案 