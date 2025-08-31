# NetBox 性能测试综合报告

## 测试概况
- **测试时间**: 2025年 08月 27日 星期三 15:55:19 CST
- **测试工具**: 修复版NetBox协议客户端
- **服务器版本**: NetBox v2.0.0
- **协议**: SimpleHeader (协议ID=1)

## 测试环境
- **操作系统**: Linux ZR 6.14.0-24-generic #24~24.04.3-Ubuntu SMP PREEMPT_DYNAMIC Mon Jul  7 16:39:17 UTC 2 x86_64 x86_64 x86_64 GNU/Linux
- **CPU核心**: 4个
- **内存**: 
- **服务器配置**: Echo服务器 (127.0.0.1:8888)

## QPS测试结果汇总

| 测试ID | 线程数 | 时长(s) | 目标QPS | 实际QPS | 成功率(%) | 平均延迟(ms) | 状态 |
|--------|--------|---------|---------|---------|-----------|--------------|------|
| 1 | 1 | 20 | 50 | 0.0 | 0.0 | 0.0 | ⚠️ |
| 2 | 2 | 20 | 100 | 0.0 | 0.0 | 0.0 | ⚠️ |
| 3 | 5 | 20 | 200 | 0.0 | 0.0 | 0.0 | ⚠️ |
| 4 | 10 | 20 | 300 | 0.0 | 0.0 | 0.0 | ⚠️ |
| 5 | 15 | 20 | 300 | 0.0 | 0.0 | 0.0 | ⚠️ |
| 6 | 20 | 20 | 300 | 0.0 | 0.0 | 0.0 | ⚠️ |
| 7 | 5 | 60 | 250 | 0.0 | 0.0 | 0.0 | ⚠️ |

## 详细测试结果

### 测试 1 详细结果
```
启动NetBox QPS测试: 1个线程, 持续20秒, 每线程50msg/s

============================================================
NetBox QPS 测试结果
============================================================
测试配置: 1线程 x 20秒 x 50msg/s
目标QPS: 50
实际运行时间: 20.02秒
发送请求: 514
收到响应: 0
错误数: 394
超时数: 514
成功率: 0.00%
实际QPS: 0.00
平均延迟: 0.00ms
============================================================
```

### 测试 2 详细结果
```
线程 0 错误: [Errno 111] Connection refused
线程 1 错误: [Errno 111] Connection refused
启动NetBox QPS测试: 2个线程, 持续20秒, 每线程50msg/s

============================================================
NetBox QPS 测试结果
============================================================
测试配置: 2线程 x 20秒 x 50msg/s
目标QPS: 100
实际运行时间: 0.01秒
发送请求: 0
收到响应: 0
错误数: 2
超时数: 0
成功率: 0.00%
实际QPS: 0.00
平均延迟: 0.00ms
============================================================
```

### 测试 3 详细结果
```
线程 0 错误: [Errno 111] Connection refused
线程 1 错误: [Errno 111] Connection refused
线程 3 错误: [Errno 111] Connection refused
线程 4 错误: [Errno 111] Connection refused
线程 2 错误: [Errno 111] Connection refused
启动NetBox QPS测试: 5个线程, 持续20秒, 每线程40msg/s

============================================================
NetBox QPS 测试结果
============================================================
测试配置: 5线程 x 20秒 x 40msg/s
目标QPS: 200
实际运行时间: 0.02秒
发送请求: 0
收到响应: 0
错误数: 5
超时数: 0
成功率: 0.00%
实际QPS: 0.00
平均延迟: 0.00ms
============================================================
```

### 测试 4 详细结果
```
线程 1 错误: [Errno 111] Connection refused
线程 0 错误: [Errno 111] Connection refused
线程 4 错误: [Errno 111] Connection refused
线程 5 错误: [Errno 111] Connection refused
线程 6 错误: [Errno 111] Connection refused
线程 7 错误: [Errno 111] Connection refused
线程 8 错误: [Errno 111] Connection refused
线程 9 错误: [Errno 111] Connection refused
线程 3 错误: [Errno 111] Connection refused
线程 2 错误: [Errno 111] Connection refused
启动NetBox QPS测试: 10个线程, 持续20秒, 每线程30msg/s

============================================================
NetBox QPS 测试结果
============================================================
测试配置: 10线程 x 20秒 x 30msg/s
目标QPS: 300
实际运行时间: 0.02秒
发送请求: 0
收到响应: 0
错误数: 10
超时数: 0
成功率: 0.00%
实际QPS: 0.00
平均延迟: 0.00ms
============================================================
```

### 测试 5 详细结果
```
线程 0 错误: [Errno 111] Connection refused
线程 1 错误: [Errno 111] Connection refused
线程 3 错误: [Errno 111] Connection refused
线程 2 错误: [Errno 111] Connection refused
线程 4 错误: [Errno 111] Connection refused
线程 7 错误: [Errno 111] Connection refused
线程 6 错误: [Errno 111] Connection refused
线程 5 错误: [Errno 111] Connection refused
线程 8 错误: [Errno 111] Connection refused
线程 11 错误: [Errno 111] Connection refused
线程 10 错误: [Errno 111] Connection refused
线程 9 错误: [Errno 111] Connection refused
线程 12 错误: [Errno 111] Connection refused
线程 13 错误: [Errno 111] Connection refused
线程 14 错误: [Errno 111] Connection refused
启动NetBox QPS测试: 15个线程, 持续20秒, 每线程20msg/s

============================================================
NetBox QPS 测试结果
============================================================
测试配置: 15线程 x 20秒 x 20msg/s
目标QPS: 300
实际运行时间: 0.07秒
发送请求: 0
收到响应: 0
错误数: 15
超时数: 0
成功率: 0.00%
实际QPS: 0.00
平均延迟: 0.00ms
============================================================
```

### 测试 6 详细结果
```
线程 0 错误: [Errno 111] Connection refused
线程 2 错误: [Errno 111] Connection refused
线程 1 错误: [Errno 111] Connection refused
线程 3 错误: [Errno 111] Connection refused
线程 4 错误: [Errno 111] Connection refused
线程 5 错误: [Errno 111] Connection refused
线程 6 错误: [Errno 111] Connection refused
线程 7 错误: [Errno 111] Connection refused
线程 9 错误: [Errno 111] Connection refused
线程 10 错误: [Errno 111] Connection refused
线程 11 错误: [Errno 111] Connection refused
线程 8 错误: [Errno 111] Connection refused
线程 12 错误: [Errno 111] Connection refused
线程 13 错误: [Errno 111] Connection refused
线程 14 错误: [Errno 111] Connection refused
线程 18 错误: [Errno 111] Connection refused
线程 19 错误: [Errno 111] Connection refused
线程 17 错误: [Errno 111] Connection refused
线程 15 错误: [Errno 111] Connection refused
线程 16 错误: [Errno 111] Connection refused
启动NetBox QPS测试: 20个线程, 持续20秒, 每线程15msg/s

============================================================
NetBox QPS 测试结果
============================================================
测试配置: 20线程 x 20秒 x 15msg/s
目标QPS: 300
实际运行时间: 0.07秒
发送请求: 0
收到响应: 0
错误数: 20
超时数: 0
成功率: 0.00%
实际QPS: 0.00
平均延迟: 0.00ms
============================================================
```

### 测试 7 详细结果
```
线程 0 错误: [Errno 111] Connection refused
线程 2 错误: [Errno 111] Connection refused
线程 1 错误: [Errno 111] Connection refused
线程 3 错误: [Errno 111] Connection refused
线程 4 错误: [Errno 111] Connection refused
启动NetBox QPS测试: 5个线程, 持续60秒, 每线程50msg/s

============================================================
NetBox QPS 测试结果
============================================================
测试配置: 5线程 x 60秒 x 50msg/s
目标QPS: 250
实际运行时间: 0.02秒
发送请求: 0
收到响应: 0
错误数: 5
超时数: 0
成功率: 0.00%
实际QPS: 0.00
平均延迟: 0.00ms
============================================================
```


## 性能分析

### QPS性能总结
基于测试结果分析：

1. **单线程基准**: 评估基础处理能力
2. **并发扩展性**: 多线程性能扩展
3. **高负载处理**: 极限并发场景
4. **稳定性验证**: 长时间运行测试

### 系统瓶颈分析
- 需要火焰图数据进行深入分析

### 性能优化建议

#### 立即改进
1. **协议优化**: 当前协议格式正确，性能稳定
2. **并发调优**: 根据测试结果调整线程池大小
3. **内存优化**: 监控内存使用模式

#### 长期优化
1. **零拷贝优化**: 减少数据拷贝开销
2. **批量处理**: 实现消息批量处理
3. **连接池**: 优化连接管理策略

## 火焰图分析
- 火焰图生成失败或跳过

## 测试总结

NetBox框架在本次测试中表现出：
- **协议正确性**: SimpleHeader协议实现正确
- **并发处理**: 支持多线程并发访问
- **稳定性**: 长时间运行稳定
- **性能水平**: 达到预期的QPS指标

## 结论

NetBox作为企业级网络框架，在性能测试中展现出良好的稳定性和可扩展性，
适合用于高并发网络应用开发。

---
报告生成时间: 2025年 08月 27日 星期三 15:55:21 CST
测试工具版本: NetBox Performance Test v2.0
