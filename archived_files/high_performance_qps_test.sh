#!/bin/bash

# NetBox 高性能QPS测试脚本 - 目标10万QPS
set -e

PROJECT_DIR="/home/sgly/桌面/NetBox"
RESULTS_DIR="$PROJECT_DIR/performance_results"
SERVER_BIN="$PROJECT_DIR/build/bin/NetBox"
CONFIG_FILE="$PROJECT_DIR/config/config_echo_local.yaml"

mkdir -p "$RESULTS_DIR"
cd "$PROJECT_DIR"

echo "🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀"
echo "    NetBox 高性能QPS测试 - 目标10万QPS"
echo "🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀"
echo "测试时间: $(date)"
echo "目标性能: 100,000 QPS"
echo "硬件配置: 4核8G虚拟机"
echo ""

# 清理函数
cleanup() {
    echo "正在清理..."
    pkill -f NetBox || true
    pkill -f high_perf_client || true
    sleep 2
    echo "清理完成"
}

trap cleanup EXIT INT TERM

# 优化系统参数
optimize_system() {
    echo "⚡ 优化系统参数以达到高性能..."
    
    # 网络参数优化
    sudo sysctl -w net.core.somaxconn=65535 2>/dev/null || echo "需要root权限优化somaxconn"
    sudo sysctl -w net.core.netdev_max_backlog=30000 2>/dev/null || echo "需要root权限优化netdev_max_backlog"
    sudo sysctl -w net.ipv4.tcp_max_syn_backlog=65535 2>/dev/null || echo "需要root权限优化tcp_max_syn_backlog"
    sudo sysctl -w net.core.rmem_max=134217728 2>/dev/null || echo "需要root权限优化rmem_max"
    sudo sysctl -w net.core.wmem_max=134217728 2>/dev/null || echo "需要root权限优化wmem_max"
    
    # 文件描述符限制
    ulimit -n 65535 2>/dev/null || echo "无法设置文件描述符限制"
    
    echo "✅ 系统参数优化完成"
}

# 创建高性能客户端
create_high_perf_client() {
    cat > /tmp/high_perf_client.py << 'EOF'
#!/usr/bin/env python3
import socket
import threading
import time
import sys
import struct
import json
import select
from concurrent.futures import ThreadPoolExecutor

class HighPerfNetBoxClient:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.stats = {
            'requests': 0,
            'responses': 0,
            'errors': 0,
            'timeouts': 0,
            'total_latency': 0.0
        }
        self.lock = threading.Lock()
    
    def create_packet(self, message):
        """创建NetBox协议数据包 - 优化版本"""
        try:
            message_bytes = message.encode('utf-8')
            message_len = len(message_bytes)
            
            # 预计算协议头 - 避免重复计算
            protocol_header = b'\x00\x00\x00\x01'  # 协议ID=1
            body_length = struct.pack('>I', message_len)
            
            return protocol_header + body_length + message_bytes
        except:
            return None
    
    def parse_response_fast(self, data):
        """快速响应解析 - 只检查是否有Echo前缀"""
        try:
            if len(data) >= 13:  # 最小Echo响应长度
                # 跳过协议头，检查是否有"Echo:"
                if data[8:13] == b'Echo:':
                    return True
            return False
        except:
            return False
    
    def high_speed_worker(self, thread_id, duration, target_rate):
        """高速工作线程 - 优化版"""
        local_stats = {'requests': 0, 'responses': 0, 'errors': 0, 'timeouts': 0, 'latency': 0.0}
        
        try:
            # 创建连接
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            sock.settimeout(2.0)  # 减少超时时间
            sock.connect((self.host, self.port))
            
            # 预生成消息包 - 避免运行时计算
            base_msg = f"HighPerf-T{thread_id}-"
            packets = []
            for i in range(100):  # 预生成100个包
                msg = f"{base_msg}R{i}"
                packet = self.create_packet(msg)
                if packet:
                    packets.append(packet)
            
            if not packets:
                local_stats['errors'] += 1
                return local_stats
            
            packet_idx = 0
            start_time = time.time()
            interval = 1.0 / target_rate if target_rate > 0 else 0
            
            # 使用非阻塞IO
            sock.setblocking(False)
            
            while time.time() - start_time < duration:
                loop_start = time.time()
                
                # 发送数据包
                packet = packets[packet_idx % len(packets)]
                packet_idx += 1
                
                try:
                    req_start = time.time()
                    sock.send(packet)
                    local_stats['requests'] += 1
                    
                    # 快速接收检查
                    ready = select.select([sock], [], [], 0.1)  # 100ms超时
                    if ready[0]:
                        response = sock.recv(1024)
                        req_end = time.time()
                        
                        if response and self.parse_response_fast(response):
                            local_stats['responses'] += 1
                            local_stats['latency'] += (req_end - req_start) * 1000
                        else:
                            local_stats['timeouts'] += 1
                    else:
                        local_stats['timeouts'] += 1
                        
                except socket.error as e:
                    if e.errno in (11, 35):  # EAGAIN/EWOULDBLOCK
                        time.sleep(0.001)  # 1ms
                    else:
                        local_stats['errors'] += 1
                
                # 速率控制 - 仅在必要时睡眠
                if interval > 0:
                    elapsed = time.time() - loop_start
                    if elapsed < interval:
                        sleep_time = interval - elapsed
                        if sleep_time > 0.001:  # 只有超过1ms才睡眠
                            time.sleep(sleep_time)
            
            sock.close()
            
        except Exception as e:
            local_stats['errors'] += 1
            print(f"线程 {thread_id} 异常: {e}", file=sys.stderr)
        
        return local_stats
    
    def run_high_perf_test(self, num_threads, duration, rate_per_thread):
        """运行高性能测试"""
        print(f"🔥 启动高性能QPS测试:")
        print(f"   线程数: {num_threads}")
        print(f"   持续时间: {duration}秒") 
        print(f"   每线程速率: {rate_per_thread} msg/s")
        print(f"   目标总QPS: {num_threads * rate_per_thread}")
        
        start_time = time.time()
        
        # 使用线程池
        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            futures = []
            for i in range(num_threads):
                future = executor.submit(self.high_speed_worker, i, duration, rate_per_thread)
                futures.append(future)
            
            # 收集结果
            for future in futures:
                local_stats = future.result()
                with self.lock:
                    self.stats['requests'] += local_stats['requests']
                    self.stats['responses'] += local_stats['responses']
                    self.stats['errors'] += local_stats['errors']
                    self.stats['timeouts'] += local_stats['timeouts']
                    self.stats['total_latency'] += local_stats['latency']
        
        end_time = time.time()
        total_time = end_time - start_time
        
        # 计算性能指标
        actual_qps = self.stats['responses'] / total_time
        success_rate = (self.stats['responses'] / max(self.stats['requests'], 1)) * 100
        avg_latency = self.stats['total_latency'] / max(self.stats['responses'], 1)
        
        # 性能评级
        performance_grade = "🔥"
        if actual_qps >= 100000:
            performance_grade = "🚀🚀🚀 EXCELLENT"
        elif actual_qps >= 50000:
            performance_grade = "🚀🚀 GREAT"
        elif actual_qps >= 20000:
            performance_grade = "🚀 GOOD"
        elif actual_qps >= 10000:
            performance_grade = "✅ OK"
        else:
            performance_grade = "⚠️ NEED OPTIMIZATION"
        
        # 输出结果
        result = {
            'test_config': {
                'threads': num_threads,
                'duration': duration,
                'target_rate_per_thread': rate_per_thread,
                'total_target_qps': num_threads * rate_per_thread
            },
            'results': {
                'total_time': round(total_time, 2),
                'requests_sent': self.stats['requests'],
                'responses_received': self.stats['responses'],
                'errors': self.stats['errors'],
                'timeouts': self.stats['timeouts'],
                'success_rate': round(success_rate, 2),
                'actual_qps': round(actual_qps, 2),
                'avg_latency_ms': round(avg_latency, 2),
                'performance_grade': performance_grade
            }
        }
        
        print("\n" + "="*80)
        print(f"🏆 NetBox 高性能测试结果 {performance_grade}")
        print("="*80)
        print(f"📊 测试配置: {num_threads}线程 x {duration}秒 x {rate_per_thread}msg/s")
        print(f"🎯 目标QPS: {num_threads * rate_per_thread:,}")
        print(f"⏱️  实际运行时间: {total_time:.2f}秒")
        print(f"📤 发送请求: {self.stats['requests']:,}")
        print(f"📥 收到响应: {self.stats['responses']:,}")
        print(f"❌ 错误数: {self.stats['errors']:,}")
        print(f"⏰ 超时数: {self.stats['timeouts']:,}")
        print(f"✅ 成功率: {success_rate:.2f}%")
        print(f"🚀 实际QPS: {actual_qps:,.2f}")
        print(f"⚡ 平均延迟: {avg_latency:.2f}ms")
        
        if actual_qps >= 100000:
            print("🎉 恭喜！达到10万QPS目标！")
        elif actual_qps >= 50000:
            print("👍 很好！达到5万QPS，接近目标！")
        else:
            print("💡 需要继续优化以达到10万QPS目标")
        
        print("="*80)
        
        return result

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("用法: python3 high_perf_client.py <host> <port> <threads> <duration> <rate_per_thread>")
        sys.exit(1)
    
    host = sys.argv[1]
    port = int(sys.argv[2])
    threads = int(sys.argv[3])
    duration = int(sys.argv[4])
    rate = int(sys.argv[5])
    
    client = HighPerfNetBoxClient(host, port)
    result = client.run_high_perf_test(threads, duration, rate)
    
    # 保存结果
    with open(f'/tmp/high_perf_result_{threads}t_{rate}r.json', 'w') as f:
        json.dump(result, f, indent=2)
EOF

    chmod +x /tmp/high_perf_client.py
    echo "⚡ 高性能客户端创建完成"
}

# 启动高性能服务器配置
start_high_perf_server() {
    echo "🚀 启动高性能NetBox服务器..."
    
    # 创建高性能配置文件
    cat > /tmp/high_perf_config.yaml << EOF
# NetBox 高性能配置
application:
  type: echo

network:
  ip: 127.0.0.1
  port: 8888
  io_type: epoll

threading:
  worker_threads: 8        # 增加到8个工作线程
  thread_pool_type: double_lock

logging:
  level: warn              # 减少日志输出
  async: true

performance:
  max_connections: 10000   # 增加最大连接数
  buffer_size: 8192        # 增加缓冲区

echo:
  heartbeat_enabled: false # 关闭心跳以减少开销
  protocol: SimpleHeader

protocol:
  default: SimpleHeader
  timeout: 60
EOF
    
    $SERVER_BIN /tmp/high_perf_config.yaml > "$RESULTS_DIR/high_perf_server.log" 2>&1 &
    SERVER_PID=$!
    
    # 等待服务器启动
    for i in {1..15}; do
        if netstat -tln | grep ":8888" > /dev/null; then
            echo "✅ 高性能服务器启动成功 (PID: $SERVER_PID)"
            return 0
        fi
        echo "等待服务器启动... ($i/15)"
        sleep 1
    done
    
    echo "❌ 服务器启动失败"
    return 1
}

# 运行高性能QPS测试套件
run_high_performance_tests() {
    echo ""
    echo "🔥 开始高性能QPS测试 - 冲击10万QPS！"
    echo ""
    
    create_high_perf_client
    
    # 高性能测试配置: [线程数, 持续时间, 每线程速率, 描述]
    local tests=(
        "10 30 1000 基准测试-1万QPS"
        "20 30 1500 扩展测试-3万QPS" 
        "40 30 1250 高并发-5万QPS"
        "50 30 1600 极限测试-8万QPS"
        "60 30 1667 冲击测试-10万QPS"
        "80 30 1250 超极限-10万QPS"
        "100 30 1000 最大并发-10万QPS"
    )
    
    local test_num=1
    for test in "${tests[@]}"; do
        read -r threads duration rate description <<< "$test"
        target_qps=$((threads * rate))
        
        echo ""
        echo "🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀"
        echo "测试 $test_num: $description"
        echo "配置: ${threads}线程 x ${rate}msg/s = 目标${target_qps}QPS"
        echo "🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀🚀"
        
        # 运行高性能测试
        python3 /tmp/high_perf_client.py 127.0.0.1 8888 $threads $duration $rate \
            > "$RESULTS_DIR/high_perf_test_${test_num}.txt" 2>&1
        
        # 复制结果
        if [ -f "/tmp/high_perf_result_${threads}t_${rate}r.json" ]; then
            cp "/tmp/high_perf_result_${threads}t_${rate}r.json" \
               "$RESULTS_DIR/high_perf_result_${test_num}.json"
        fi
        
        # 显示关键结果
        if [ -f "$RESULTS_DIR/high_perf_result_${test_num}.json" ] && command -v jq > /dev/null; then
            local actual_qps=$(jq -r '.results.actual_qps' "$RESULTS_DIR/high_perf_result_${test_num}.json" 2>/dev/null)
            local success_rate=$(jq -r '.results.success_rate' "$RESULTS_DIR/high_perf_result_${test_num}.json" 2>/dev/null)
            local grade=$(jq -r '.results.performance_grade' "$RESULTS_DIR/high_perf_result_${test_num}.json" 2>/dev/null)
            
            echo "📊 快速结果: QPS=${actual_qps}, 成功率=${success_rate}%, 评级=${grade}"
        fi
        
        echo "✅ 测试 $test_num 完成"
        
        # 让系统恢复
        if [ $test_num -lt ${#tests[@]} ]; then
            echo "等待5秒系统恢复..."
            sleep 5
        fi
        
        ((test_num++))
    done
}

# 生成高性能测试报告
generate_high_perf_report() {
    local report_file="$RESULTS_DIR/high_performance_report.md"
    
    echo ""
    echo "📊 生成高性能测试报告..."
    
    cat > "$report_file" << EOF
# 🚀 NetBox 高性能QPS测试报告 - 冲击10万QPS

## 🎯 测试目标
- **性能目标**: 100,000 QPS (10万每秒查询数)
- **硬件配置**: 4核8G虚拟机
- **测试协议**: NetBox SimpleHeader协议
- **优化策略**: 高并发 + 系统调优 + 协议优化

## 🖥️ 测试环境
- **测试时间**: $(date)
- **操作系统**: $(uname -a)
- **CPU核心**: $(nproc)个
- **内存**: $(free -h | grep Mem | awk '{print $2}')
- **网络**: 本地回环 (127.0.0.1)

## 📊 高性能测试结果

| 测试 | 线程数 | 目标QPS | 实际QPS | 成功率(%) | 延迟(ms) | 性能评级 |
|------|--------|---------|---------|-----------|----------|----------|
EOF
    
    # 解析所有测试结果
    for i in {1..7}; do
        if [ -f "$RESULTS_DIR/high_perf_result_${i}.json" ] && command -v jq > /dev/null; then
            local json_file="$RESULTS_DIR/high_perf_result_${i}.json"
            local threads=$(jq -r '.test_config.threads' "$json_file" 2>/dev/null)
            local target_qps=$(jq -r '.test_config.total_target_qps' "$json_file" 2>/dev/null)
            local actual_qps=$(jq -r '.results.actual_qps' "$json_file" 2>/dev/null)
            local success_rate=$(jq -r '.results.success_rate' "$json_file" 2>/dev/null)
            local latency=$(jq -r '.results.avg_latency_ms' "$json_file" 2>/dev/null)
            local grade=$(jq -r '.results.performance_grade' "$json_file" 2>/dev/null)
            
            echo "| $i | $threads | $target_qps | $actual_qps | $success_rate | $latency | $grade |" >> "$report_file"
        fi
    done
    
    # 添加详细分析
    cat >> "$report_file" << EOF

## 🏆 性能成就分析

### 最高QPS记录
EOF
    
    # 找出最高QPS
    local max_qps=0
    local best_test=""
    for i in {1..7}; do
        if [ -f "$RESULTS_DIR/high_perf_result_${i}.json" ] && command -v jq > /dev/null; then
            local qps=$(jq -r '.results.actual_qps' "$RESULTS_DIR/high_perf_result_${i}.json" 2>/dev/null)
            if (( $(echo "$qps > $max_qps" | bc -l 2>/dev/null || echo 0) )); then
                max_qps=$qps
                best_test="测试$i"
            fi
        fi
    done
    
    cat >> "$report_file" << EOF
- **最高QPS**: $max_qps (在 $best_test 中达成)
- **10万QPS目标**: $(if (( $(echo "$max_qps >= 100000" | bc -l 2>/dev/null || echo 0) )); then echo "✅ 已达成"; else echo "⏳ 接近目标 ($(echo "scale=1; $max_qps/1000" | bc)K QPS)"; fi)

### 性能级别判定
EOF
    
    if (( $(echo "$max_qps >= 100000" | bc -l 2>/dev/null || echo 0) )); then
        echo "🚀🚀🚀 **卓越级** - 成功突破10万QPS大关！" >> "$report_file"
    elif (( $(echo "$max_qps >= 50000" | bc -l 2>/dev/null || echo 0) )); then
        echo "🚀🚀 **优秀级** - 达到5万QPS以上，性能优异！" >> "$report_file"
    elif (( $(echo "$max_qps >= 20000" | bc -l 2>/dev/null || echo 0) )); then
        echo "🚀 **良好级** - 达到2万QPS以上，表现良好！" >> "$report_file"
    else
        echo "⚠️ **需优化** - 未达到预期性能，需要进一步优化" >> "$report_file"
    fi
    
    cat >> "$report_file" << EOF

## 🔧 性能优化分析

### 已实施的优化策略
1. **系统级优化**:
   - 网络参数调优 (somaxconn, netdev_max_backlog)
   - TCP参数优化 (TCP_NODELAY, SO_REUSEADDR)
   - 文件描述符限制提升

2. **应用级优化**:
   - 增加工作线程数至8个
   - 关闭详细日志减少IO开销
   - 关闭心跳机制减少网络开销
   - 增大缓冲区至8KB

3. **客户端优化**:
   - 非阻塞IO模式
   - 预生成数据包避免运行时计算
   - 快速响应解析算法
   - 线程池并发处理

### 进一步优化建议

#### 短期优化 (可立即实施)
1. **协议层优化**:
   - 实现零拷贝数据传输
   - 优化协议解析算法
   - 减少内存分配次数

2. **网络层优化**:
   - 使用SO_REUSEPORT支持
   - 实现连接池复用
   - 优化epoll事件处理

#### 中期优化 (需要代码修改)
1. **架构优化**:
   - 实现无锁队列
   - 采用reactor模式
   - CPU亲和性绑定

2. **内存优化**:
   - 对象池化管理
   - 内存预分配策略
   - NUMA感知优化

#### 长期优化 (架构重构)
1. **异步化改造**:
   - 完全异步IO架构
   - 协程/纤程支持
   - 事件驱动模式

2. **分布式扩展**:
   - 多进程模式
   - 负载均衡支持
   - 水平扩展能力

## 📈 性能对比基准

### 业界QPS基准参考
- **Redis**: ~100K QPS (单实例)
- **Nginx**: ~50K-200K QPS
- **Node.js**: ~20K-50K QPS
- **Go HTTP**: ~100K+ QPS

### NetBox性能定位
NetBox当前性能水平: **$(echo "scale=0; $max_qps/1000" | bc)K QPS**
$(if (( $(echo "$max_qps >= 80000" | bc -l 2>/dev/null || echo 0) )); then
    echo "- 🏆 **行业领先级**: 性能超越大部分开源框架"
elif (( $(echo "$max_qps >= 50000" | bc -l 2>/dev/null || echo 0) )); then
    echo "- 🥇 **行业先进级**: 性能达到主流框架水平"
elif (( $(echo "$max_qps >= 20000" | bc -l 2>/dev/null || echo 0) )); then
    echo "- 🥈 **企业级**: 满足大部分企业应用需求"
else
    echo "- 📚 **学习级**: 适合学习和小规模应用"
fi)

## 🎯 校招展示价值

### 技术深度体现
1. **网络编程精通**: 达到$(echo "scale=0; $max_qps/1000" | bc)K QPS展现深度优化能力
2. **系统调优经验**: 从系统到应用全栈优化
3. **性能分析能力**: 瓶颈定位和针对性优化
4. **工程实践能力**: 完整的测试和分析流程

### 项目亮点总结
- ✅ 企业级网络框架设计
- ✅ 高并发处理架构
- ✅ 系统性能调优
- ✅ 完整测试体系
- ✅ 详细性能分析

---
报告生成时间: $(date)
测试执行者: NetBox Performance Team
框架版本: NetBox v2.0.0
EOF
    
    echo "✅ 高性能测试报告生成完成: $report_file"
}

# 主程序
main() {
    echo "🚀 准备NetBox高性能QPS测试..."
    
    # 系统优化
    optimize_system
    
    # 检查依赖
    if ! command -v python3 &> /dev/null; then
        echo "❌ 需要Python3环境"
        exit 1
    fi
    
    if ! command -v bc &> /dev/null; then
        sudo apt-get update && sudo apt-get install -y bc jq || echo "工具安装可能失败"
    fi
    
    # 启动高性能服务器
    if ! start_high_perf_server; then
        echo "❌ 无法启动高性能服务器"
        exit 1
    fi
    
    echo "⏰ 等待10秒让服务器充分预热..."
    sleep 10
    
    # 运行高性能测试
    run_high_performance_tests
    
    # 生成报告
    generate_high_perf_report
    
    # 停止服务器
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null || true
        echo "✅ 服务器已停止"
    fi
    
    echo ""
    echo "🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆"
    echo "    NetBox 高性能测试完成！"
    echo "🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆"
    echo ""
    echo "📊 结果目录: $RESULTS_DIR"
    echo "📋 高性能报告: $RESULTS_DIR/high_performance_report.md"
    echo "📝 服务器日志: $RESULTS_DIR/high_perf_server.log"
    echo ""
    
    # 显示最佳结果
    if command -v jq > /dev/null; then
        local max_qps=0
        for i in {1..7}; do
            if [ -f "$RESULTS_DIR/high_perf_result_${i}.json" ]; then
                local qps=$(jq -r '.results.actual_qps' "$RESULTS_DIR/high_perf_result_${i}.json" 2>/dev/null)
                if (( $(echo "$qps > $max_qps" | bc -l 2>/dev/null || echo 0) )); then
                    max_qps=$qps
                fi
            fi
        done
        
        echo "🎯 最高QPS记录: $(printf "%'.0f" $max_qps 2>/dev/null || echo $max_qps)"
        
        if (( $(echo "$max_qps >= 100000" | bc -l 2>/dev/null || echo 0) )); then
            echo "🎉 恭喜！成功突破10万QPS大关！🎉"
        else
            echo "💪 继续努力！距离10万QPS目标还有: $(echo "100000 - $max_qps" | bc) QPS"
        fi
    fi
    
    echo "🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆🏆"
}

# 运行主程序
main "$@" 