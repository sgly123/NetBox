#!/bin/bash

# NetBox QPS 基准测试脚本
set -e

PROJECT_DIR="/home/sgly/桌面/NetBox"
RESULTS_DIR="$PROJECT_DIR/performance_results"
SERVER_BIN="$PROJECT_DIR/build/bin/NetBox"
CONFIG_FILE="$PROJECT_DIR/config/config_echo_local.yaml"

mkdir -p "$RESULTS_DIR"
cd "$PROJECT_DIR"

echo "=================================="
echo "    NetBox QPS 基准测试"
echo "=================================="
echo "测试时间: $(date)"
echo ""

# 清理函数
cleanup() {
    echo "正在清理..."
    pkill -f NetBox || true
    pkill -f qps_test || true
    sleep 2
    echo "清理完成"
}

trap cleanup EXIT INT TERM

# 创建高性能TCP客户端测试程序
create_tcp_client() {
    cat > /tmp/qps_test_client.py << 'EOF'
#!/usr/bin/env python3
import socket
import threading
import time
import sys
import struct
import json
from concurrent.futures import ThreadPoolExecutor

class NetBoxClient:
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
    
    def send_message(self, sock, message):
        """发送SimpleHeader格式的消息"""
        try:
            msg_bytes = message.encode('utf-8')
            header = struct.pack('<I', len(msg_bytes))  # 4字节长度前缀，小端序
            sock.send(header + msg_bytes)
            return True
        except Exception as e:
            return False
    
    def recv_message(self, sock):
        """接收SimpleHeader格式的消息"""
        try:
            # 读取4字节头部
            header = sock.recv(4)
            if len(header) != 4:
                return None
            
            msg_len = struct.unpack('<I', header)[0]
            if msg_len > 10240:  # 防止过大的消息
                return None
            
            # 读取消息体
            msg_data = b''
            while len(msg_data) < msg_len:
                chunk = sock.recv(msg_len - len(msg_data))
                if not chunk:
                    return None
                msg_data += chunk
            
            return msg_data.decode('utf-8')
        except Exception as e:
            return None
    
    def worker_thread(self, thread_id, duration, messages_per_second):
        """工作线程"""
        local_stats = {'requests': 0, 'responses': 0, 'errors': 0, 'timeouts': 0, 'latency': 0.0}
        
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(5.0)  # 5秒超时
            sock.connect((self.host, self.port))
            
            start_time = time.time()
            interval = 1.0 / messages_per_second if messages_per_second > 0 else 0
            
            while time.time() - start_time < duration:
                loop_start = time.time()
                
                message = f"QPS-Test-{thread_id}-{local_stats['requests']}-{int(time.time() * 1000)}"
                
                # 发送消息
                req_start = time.time()
                if self.send_message(sock, message):
                    local_stats['requests'] += 1
                    
                    # 接收响应
                    response = self.recv_message(sock)
                    req_end = time.time()
                    
                    if response:
                        local_stats['responses'] += 1
                        local_stats['latency'] += (req_end - req_start) * 1000  # 转换为毫秒
                    else:
                        local_stats['timeouts'] += 1
                else:
                    local_stats['errors'] += 1
                
                # 控制发送速率
                if interval > 0:
                    elapsed = time.time() - loop_start
                    if elapsed < interval:
                        time.sleep(interval - elapsed)
            
            sock.close()
            
        except Exception as e:
            local_stats['errors'] += 1
            print(f"线程 {thread_id} 错误: {e}", file=sys.stderr)
        
        # 合并统计数据
        with self.lock:
            self.stats['requests'] += local_stats['requests']
            self.stats['responses'] += local_stats['responses']
            self.stats['errors'] += local_stats['errors']
            self.stats['timeouts'] += local_stats['timeouts']
            self.stats['total_latency'] += local_stats['latency']
    
    def run_test(self, num_threads, duration, messages_per_second_per_thread):
        """运行测试"""
        print(f"启动QPS测试: {num_threads}个线程, 持续{duration}秒, 每线程{messages_per_second_per_thread}msg/s")
        
        start_time = time.time()
        
        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            futures = []
            for i in range(num_threads):
                future = executor.submit(self.worker_thread, i, duration, messages_per_second_per_thread)
                futures.append(future)
            
            # 等待所有线程完成
            for future in futures:
                future.result()
        
        end_time = time.time()
        total_time = end_time - start_time
        
        # 计算结果
        avg_latency = self.stats['total_latency'] / max(self.stats['responses'], 1)
        qps = self.stats['responses'] / total_time
        success_rate = (self.stats['responses'] / max(self.stats['requests'], 1)) * 100
        
        # 输出结果
        result = {
            'test_config': {
                'threads': num_threads,
                'duration': duration,
                'target_rate_per_thread': messages_per_second_per_thread,
                'total_target_rate': num_threads * messages_per_second_per_thread
            },
            'results': {
                'total_time': round(total_time, 2),
                'requests_sent': self.stats['requests'],
                'responses_received': self.stats['responses'],
                'errors': self.stats['errors'],
                'timeouts': self.stats['timeouts'],
                'success_rate': round(success_rate, 2),
                'qps': round(qps, 2),
                'avg_latency_ms': round(avg_latency, 2)
            }
        }
        
        print("\n" + "="*50)
        print("QPS 测试结果")
        print("="*50)
        print(f"测试配置: {num_threads}线程 x {duration}秒 x {messages_per_second_per_thread}msg/s")
        print(f"实际运行时间: {total_time:.2f}秒")
        print(f"发送请求: {self.stats['requests']}")
        print(f"收到响应: {self.stats['responses']}")
        print(f"错误数: {self.stats['errors']}")
        print(f"超时数: {self.stats['timeouts']}")
        print(f"成功率: {success_rate:.2f}%")
        print(f"QPS: {qps:.2f}")
        print(f"平均延迟: {avg_latency:.2f}ms")
        print("="*50)
        
        return result

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("用法: python3 qps_test_client.py <host> <port> <threads> <duration> <rate_per_thread>")
        sys.exit(1)
    
    host = sys.argv[1]
    port = int(sys.argv[2])
    threads = int(sys.argv[3])
    duration = int(sys.argv[4])
    rate = int(sys.argv[5])
    
    client = NetBoxClient(host, port)
    result = client.run_test(threads, duration, rate)
    
    # 保存结果到JSON文件
    with open(f'/tmp/qps_result_{threads}t_{duration}s_{rate}r.json', 'w') as f:
        json.dump(result, f, indent=2)
EOF

    chmod +x /tmp/qps_test_client.py
}

# 启动服务器并等待就绪
start_server() {
    echo "启动NetBox服务器..."
    
    $SERVER_BIN "$CONFIG_FILE" > "$RESULTS_DIR/qps_server.log" 2>&1 &
    SERVER_PID=$!
    
    # 等待服务器启动
    for i in {1..10}; do
        if netstat -tln | grep ":8888" > /dev/null; then
            echo "✅ 服务器启动成功 (PID: $SERVER_PID)"
            return 0
        fi
        echo "等待服务器启动... ($i/10)"
        sleep 1
    done
    
    echo "❌ 服务器启动失败"
    return 1
}

# QPS基准测试套件
run_qps_benchmarks() {
    echo ""
    echo "🔥 开始QPS基准测试套件"
    echo ""
    
    # 创建客户端程序
    create_tcp_client
    
    # 测试配置数组: [线程数, 持续时间(秒), 每线程速率(msg/s)]
    local tests=(
        "1 30 100"      # 单线程基准
        "5 30 100"      # 低并发
        "10 30 100"     # 中等并发
        "20 30 50"      # 高并发，降低速率
        "50 30 20"      # 极高并发，低速率
        "10 60 200"     # 长时间高速率测试
    )
    
    local test_num=1
    for test in "${tests[@]}"; do
        read -r threads duration rate <<< "$test"
        
        echo ""
        echo "────────────────────────────────────"
        echo "测试 $test_num: ${threads}线程 x ${duration}秒 x ${rate}msg/s"
        echo "────────────────────────────────────"
        
        # 运行测试
        python3 /tmp/qps_test_client.py 127.0.0.1 8888 $threads $duration $rate \
            > "$RESULTS_DIR/qps_test_${test_num}_${threads}t_${duration}s_${rate}r.txt" 2>&1
        
        # 复制JSON结果
        if [ -f "/tmp/qps_result_${threads}t_${duration}s_${rate}r.json" ]; then
            cp "/tmp/qps_result_${threads}t_${duration}s_${rate}r.json" \
               "$RESULTS_DIR/qps_result_${test_num}.json"
        fi
        
        echo "✅ 测试 $test_num 完成"
        
        # 测试间隔，让服务器恢复
        if [ $test_num -lt ${#tests[@]} ]; then
            echo "等待5秒后进行下一个测试..."
            sleep 5
        fi
        
        ((test_num++))
    done
}

# 生成QPS测试报告
generate_qps_report() {
    local report_file="$RESULTS_DIR/qps_benchmark_report.md"
    
    echo ""
    echo "📊 生成QPS基准测试报告..."
    
    cat > "$report_file" << EOF
# NetBox QPS 基准测试报告

## 测试环境
- **测试时间**: $(date)
- **操作系统**: $(uname -a)
- **CPU核心**: $(nproc)个
- **内存**: $(free -h | grep Mem | awk '{print $2}')
- **服务器配置**: Echo服务器 (127.0.0.1:8888)
- **协议**: SimpleHeader (4字节长度前缀)

## 测试配置
- **IO模型**: epoll
- **工作线程**: 4个
- **最大连接**: 1000
- **缓冲区**: 4096字节

## 测试结果汇总

| 测试 | 线程数 | 时长(s) | 目标QPS | 实际QPS | 成功率(%) | 平均延迟(ms) |
|------|--------|---------|---------|---------|-----------|--------------|
EOF
    
    # 解析测试结果
    for i in {1..6}; do
        if [ -f "$RESULTS_DIR/qps_result_${i}.json" ]; then
            local json_file="$RESULTS_DIR/qps_result_${i}.json"
            local threads=$(jq -r '.test_config.threads' "$json_file" 2>/dev/null || echo "N/A")
            local duration=$(jq -r '.test_config.duration' "$json_file" 2>/dev/null || echo "N/A")
            local target_qps=$(jq -r '.test_config.total_target_rate' "$json_file" 2>/dev/null || echo "N/A")
            local actual_qps=$(jq -r '.results.qps' "$json_file" 2>/dev/null || echo "N/A")
            local success_rate=$(jq -r '.results.success_rate' "$json_file" 2>/dev/null || echo "N/A")
            local avg_latency=$(jq -r '.results.avg_latency_ms' "$json_file" 2>/dev/null || echo "N/A")
            
            echo "| $i | $threads | $duration | $target_qps | $actual_qps | $success_rate | $avg_latency |" >> "$report_file"
        fi
    done
    
    cat >> "$report_file" << EOF

## 详细测试结果

EOF
    
    # 添加每个测试的详细结果
    for i in {1..6}; do
        if [ -f "$RESULTS_DIR/qps_test_${i}_*.txt" ]; then
            local test_file=$(ls "$RESULTS_DIR"/qps_test_${i}_*.txt 2>/dev/null | head -1)
            if [ -n "$test_file" ]; then
                echo "### 测试 $i 详细结果" >> "$report_file"
                echo '```' >> "$report_file"
                cat "$test_file" >> "$report_file"
                echo '```' >> "$report_file"
                echo "" >> "$report_file"
            fi
        fi
    done
    
    cat >> "$report_file" << EOF

## 性能分析

### QPS性能曲线
基于测试结果，NetBox服务器的QPS性能表现：

1. **单线程性能**: 基准QPS水平
2. **低并发扩展**: 5-10线程的扩展能力
3. **高并发处理**: 20-50线程的并发极限
4. **稳定性测试**: 长时间运行稳定性

### 延迟分析
- **低负载延迟**: 单线程场景下的基准延迟
- **高负载延迟**: 高并发场景下的延迟增长
- **延迟分布**: P95, P99延迟统计（需要更详细的测试）

### 系统瓶颈分析
1. **CPU使用**: 监控CPU使用率与QPS的关系
2. **内存使用**: 连接数与内存消耗
3. **网络IO**: 网络带宽限制
4. **锁竞争**: 多线程环境下的同步开销

## 优化建议

### 短期优化
1. **调整线程池大小**: 根据CPU核心数优化
2. **优化缓冲区**: 根据消息大小调整
3. **减少内存拷贝**: 零拷贝优化

### 长期优化
1. **异步IO**: 完全异步的事件循环
2. **无锁数据结构**: 减少线程同步开销
3. **连接池**: 连接复用机制
4. **NUMA优化**: 多NUMA节点优化

---
报告生成时间: $(date)
EOF
    
    echo "✅ QPS基准测试报告生成完成: $report_file"
}

# 主测试流程
main() {
    echo "准备QPS基准测试..."
    
    # 检查依赖
    if ! command -v python3 &> /dev/null; then
        echo "❌ 需要Python3环境"
        exit 1
    fi
    
    if ! command -v jq &> /dev/null; then
        echo "⚠️ 安装jq工具以获得更好的报告格式"
        sudo apt-get update && sudo apt-get install -y jq || echo "jq安装失败，继续测试"
    fi
    
    # 启动服务器
    if ! start_server; then
        echo "❌ 无法启动服务器，退出测试"
        exit 1
    fi
    
    # 运行QPS测试
    run_qps_benchmarks
    
    # 生成报告
    generate_qps_report
    
    # 停止服务器
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null || true
        echo "✅ 服务器已停止"
    fi
    
    echo ""
    echo "=================================="
    echo "    QPS基准测试完成"
    echo "=================================="
    echo "结果目录: $RESULTS_DIR"
    echo "主要报告: $RESULTS_DIR/qps_benchmark_report.md"
    echo "=================================="
}

# 运行主程序
main "$@" 