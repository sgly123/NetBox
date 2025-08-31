# NetBox Echo服务器压力测试工具

本工具套件提供了完整的NetBox Echo服务器性能测试解决方案，包括QPS测试、延迟分析、perf性能分析和火焰图生成。

## 🚀 快速开始

### 一键快速测试
```bash
# 轻量级测试，验证基本功能（推荐首次使用）
./tools/quick_test.sh
```

### 完整压力测试
```bash
# 使用默认参数运行完整测试
./tools/stress_test.sh

# 高并发测试
./tools/stress_test.sh -t 16 -c 50 -d 120

# 纯性能测试（不生成分析报告）
./tools/stress_test.sh --no-perf --no-flamegraph
```

## 📊 工具说明

### 1. 主要脚本

| 脚本 | 功能 | 适用场景 |
|------|------|----------|
| `stress_test.sh` | 完整压力测试套件 | 正式性能测试 |
| `quick_test.sh` | 快速验证测试 | 功能验证、演示 |

### 2. 测试程序

| 程序 | 功能 | 说明 |
|------|------|------|
| `stress_test_echo` | 压力测试专用服务器 | 优化的echo服务器 |
| `benchmark_client` | 高性能客户端 | 支持多线程、统计分析 |

## ⚙️ 参数配置

### 服务器参数
- `-H, --host <ip>`: 服务器IP地址（默认：127.0.0.1）
- `-P, --port <port>`: 服务器端口（默认：8888）
- `-T, --server-threads <n>`: 服务器线程数（默认：4）

### 客户端参数
- `-t, --threads <n>`: 客户端线程数（默认：8）
- `-c, --connections <n>`: 每线程连接数（默认：20）
- `-r, --requests <n>`: 每连接请求数（默认：1000）
- `-s, --size <bytes>`: 消息大小（默认：64）
- `-d, --duration <sec>`: 测试时长（默认：60）

### 性能分析参数
- `--perf-time <sec>`: perf记录时长（默认：30）
- `--no-perf`: 禁用perf分析
- `--no-flamegraph`: 禁用火焰图生成
- `--no-monitor`: 禁用系统监控

## 📈 测试报告

### 输出文件说明

每次测试都会在 `performance_results/YYYYMMDD_HHMMSS/` 目录下生成以下文件：

| 文件 | 内容 | 用途 |
|------|------|------|
| `test_report.md` | 综合测试报告 | 主要查看文件 |
| `benchmark_result.txt` | 性能测试详细结果 | QPS、延迟等指标 |
| `flamegraph.svg` | 火焰图 | 性能热点分析 |
| `perf_report.txt` | perf分析报告 | 函数调用统计 |
| `system_monitor.csv` | 系统监控数据 | CPU、内存使用率 |
| `server.log` | 服务器运行日志 | 调试信息 |

### 关键指标解读

#### 1. QPS指标
```
QPS: 50000 requests/sec
```
- **含义**: 每秒处理的请求数
- **评估**: 
  - \>50k: 优秀
  - 30k-50k: 良好  
  - 10k-30k: 一般
  - <10k: 需要优化

#### 2. 延迟指标
```
平均延迟: 0.5 ms
P95延迟: 1.2 ms
P99延迟: 2.5 ms
```
- **含义**: 请求处理时间分布
- **评估**:
  - P95 < 2ms: 优秀
  - P95 < 5ms: 良好
  - P95 < 10ms: 一般
  - P95 > 10ms: 需要优化

#### 3. 吞吐量指标
```
吞吐量: 3.2 MB/s
```
- **含义**: 网络数据传输速率
- **评估**: 与网络带宽和消息大小相关

## 🔧 使用示例

### 示例1：基础性能测试
```bash
# 测试基本性能，获得QPS和延迟数据
./tools/stress_test.sh -d 30
```

### 示例2：高并发测试
```bash
# 模拟高并发场景
./tools/stress_test.sh \
    -t 32 \              # 32个客户端线程
    -c 100 \             # 每线程100个连接
    -r 500 \             # 每连接500个请求
    -d 180               # 测试3分钟
```

### 示例3：大消息测试
```bash
# 测试大消息处理能力
./tools/stress_test.sh \
    -s 4096 \            # 4KB消息
    -t 8 \               # 减少线程数
    -c 10 \              # 减少连接数
    -d 60
```

### 示例4：内存性能分析
```bash
# 重点分析内存使用和热点函数
./tools/stress_test.sh \
    --perf-time 60 \     # 延长perf记录时间
    -d 90                # 延长测试时间
```

### 示例5：纯性能测试
```bash
# 专注性能数据，不生成分析报告（适合CI/CD）
./tools/stress_test.sh \
    --no-perf \
    --no-flamegraph \
    --no-monitor \
    -d 30
```

## 🎯 测试场景建议

### 1. 开发阶段测试
```bash
# 快速验证功能正常
./tools/quick_test.sh

# 基础性能测试
./tools/stress_test.sh -d 15 --no-flamegraph
```

### 2. 性能调优测试
```bash
# 完整性能分析
./tools/stress_test.sh -d 60

# 专项热点分析
./tools/stress_test.sh --perf-time 120 -d 180
```

### 3. 压力测试
```bash
# 极限压力测试
./tools/stress_test.sh -t 64 -c 200 -d 300

# 长时间稳定性测试
./tools/stress_test.sh -d 3600  # 1小时
```

### 4. 回归测试
```bash
# 自动化回归测试
./tools/stress_test.sh \
    --no-perf \
    --no-flamegraph \
    -d 30 > regression_result.txt
```

## 🔍 性能分析指南

### 1. 查看火焰图
```bash
# 在浏览器中打开火焰图
firefox performance_results/YYYYMMDD_HHMMSS/flamegraph.svg
```

**火焰图分析技巧**：
- 宽度 = CPU使用时间
- 高度 = 调用栈深度
- 点击可放大查看
- 寻找"平顶山"模式（热点函数）

### 2. 分析perf报告
```bash
# 查看热点函数
head -30 performance_results/YYYYMMDD_HHMMSS/perf_report.txt
```

**关注指标**：
- Overhead %: CPU时间占比
- Self %: 函数自身CPU占比
- Command: 进程名
- Symbol: 函数名

### 3. 系统资源分析
```bash
# 用Excel或LibreOffice打开
libreoffice performance_results/YYYYMMDD_HHMMSS/system_monitor.csv
```

**关注指标**：
- CPU使用率趋势
- 内存使用率变化
- 网络I/O模式

## 🚨 故障排除

### 常见问题

#### 1. 编译失败
```bash
# 确保依赖已安装
sudo apt update
sudo apt install build-essential cmake libprotobuf-dev

# 清理重新编译
./tools/stress_test.sh --clean
```

#### 2. 服务器启动失败
- 检查端口是否被占用：`netstat -tlnp | grep 8888`
- 检查防火墙设置
- 查看服务器日志：`cat performance_results/*/server.log`

#### 3. 连接失败
- 确认服务器IP和端口正确
- 检查网络连通性：`ping 127.0.0.1`
- 检查服务器是否正常运行

#### 4. perf权限问题
```bash
# 设置perf权限
echo 0 | sudo tee /proc/sys/kernel/perf_event_paranoid

# 或使用sudo运行
sudo ./tools/stress_test.sh
```

#### 5. 火焰图生成失败
```bash
# 手动安装FlameGraph
git clone https://github.com/brendangregg/FlameGraph.git tools/FlameGraph
export PATH="$PWD/tools/FlameGraph:$PATH"
```

### 性能问题诊断

#### 1. QPS过低
- 检查CPU使用率是否过高
- 分析火焰图寻找热点函数
- 考虑增加服务器线程数
- 检查是否有锁竞争

#### 2. 延迟过高
- 查看P99延迟是否有异常峰值
- 检查系统负载和内存使用
- 分析是否有GC停顿（如果使用）
- 考虑减少并发度

#### 3. 内存使用异常
- 检查系统监控中的内存趋势
- 使用valgrind检查内存泄漏
- 分析服务器日志

## 📋 最佳实践

### 1. 测试前准备
- 关闭不必要的后台程序
- 确保系统负载较低
- 设置合适的文件描述符限制：`ulimit -n 65536`

### 2. 测试参数选择
- 从小参数开始，逐步增加
- 客户端线程数不要超过CPU核心数的2倍
- 连接数根据系统内存合理设置

### 3. 结果分析
- 多次测试取平均值
- 关注P95、P99延迟而非平均延迟
- 结合火焰图和perf报告分析

### 4. 性能调优
- 优先解决最大的性能瓶颈
- 关注CPU密集型函数
- 考虑异步处理和零拷贝优化

## 📞 技术支持

如有问题，请参考：
1. 项目主README文档
2. 生成的测试报告中的故障排除章节
3. 检查项目issue和文档

---

**祝您测试顺利！** 🎉 