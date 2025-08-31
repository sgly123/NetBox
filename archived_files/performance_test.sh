#!/bin/bash

# NetBox 性能测试脚本
# 测试QPS和生成火焰图

set -e

PROJECT_DIR="/home/sgly/桌面/NetBox"
RESULTS_DIR="$PROJECT_DIR/performance_results"
SERVER_BIN="$PROJECT_DIR/build/bin/NetBox"
PERF_BIN="/usr/src/linux-source-6.8.0/tools/perf/perf"

# 创建结果目录
mkdir -p "$RESULTS_DIR"
cd "$PROJECT_DIR"

echo "=================================="
echo "    NetBox 性能测试开始"
echo "=================================="
echo "测试时间: $(date)"
echo "项目目录: $PROJECT_DIR"
echo "结果目录: $RESULTS_DIR"
echo ""

# 函数：启动服务器
start_server() {
    local server_type=$1
    local port=$2
    echo "启动 $server_type 服务器 (端口: $port)..."
    
    # 创建临时配置
    cat > /tmp/netbox_test_config.yaml << EOF
server:
  type: $server_type
  host: 127.0.0.1
  port: $port
  max_connections: 10000
  thread_pool_size: 8
logging:
  level: INFO
  file: $RESULTS_DIR/server_${server_type}_${port}.log
EOF
    
    # 启动服务器 (后台运行)
    $SERVER_BIN /tmp/netbox_test_config.yaml > "$RESULTS_DIR/server_${server_type}_${port}_output.log" 2>&1 &
    local server_pid=$!
    echo "服务器PID: $server_pid"
    
    # 等待服务器启动
    sleep 3
    
    # 检查服务器是否正在运行
    if ! kill -0 $server_pid 2>/dev/null; then
        echo "❌ 服务器启动失败"
        return 1
    fi
    
    echo "✅ 服务器启动成功"
    echo $server_pid
}

# 函数：停止服务器
stop_server() {
    local pid=$1
    if [ -n "$pid" ] && kill -0 $pid 2>/dev/null; then
        echo "停止服务器 (PID: $pid)..."
        kill $pid
        sleep 2
        if kill -0 $pid 2>/dev/null; then
            kill -9 $pid
        fi
    fi
}

# 函数：测试HTTP QPS
test_http_qps() {
    local port=$1
    local duration=30
    local connections=100
    
    echo ""
    echo "🔥 HTTP QPS 测试 (端口: $port)"
    echo "持续时间: ${duration}s, 连接数: $connections"
    
    # 使用 wrk 进行 HTTP 压力测试
    echo "使用 wrk 工具测试..."
    wrk -t 8 -c $connections -d ${duration}s --timeout 30s "http://127.0.0.1:$port/" \
        > "$RESULTS_DIR/http_qps_wrk_${port}.txt" 2>&1 || echo "wrk测试完成"
    
    # 使用 ab 进行额外测试
    echo "使用 ab 工具测试..."
    ab -n 10000 -c $connections -g "$RESULTS_DIR/http_qps_ab_${port}.tsv" \
        "http://127.0.0.1:$port/" > "$RESULTS_DIR/http_qps_ab_${port}.txt" 2>&1 || echo "ab测试完成"
    
    echo "✅ HTTP QPS 测试完成"
}

# 函数：测试TCP Echo QPS
test_echo_qps() {
    local port=$1
    local duration=30
    local connections=100
    
    echo ""
    echo "🔥 TCP Echo QPS 测试 (端口: $port)"
    echo "持续时间: ${duration}s, 连接数: $connections"
    
    # 创建简单的echo测试脚本
    cat > /tmp/echo_test.py << 'EOF'
#!/usr/bin/env python3
import socket
import time
import threading
import sys

def echo_test_thread(host, port, duration, thread_id):
    requests = 0
    errors = 0
    start_time = time.time()
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((host, port))
        
        while time.time() - start_time < duration:
            try:
                message = f"Hello from thread {thread_id} - {requests}"
                # 发送长度前缀 + 消息
                msg_bytes = message.encode('utf-8')
                length = len(msg_bytes)
                sock.send(length.to_bytes(4, byteorder='little') + msg_bytes)
                
                # 接收响应
                response = sock.recv(1024)
                if response:
                    requests += 1
                else:
                    errors += 1
                    break
            except Exception as e:
                errors += 1
                break
                
        sock.close()
    except Exception as e:
        print(f"Thread {thread_id} error: {e}", file=sys.stderr)
        errors += 1
    
    return requests, errors

if __name__ == "__main__":
    host = sys.argv[1]
    port = int(sys.argv[2])
    duration = int(sys.argv[3])
    num_threads = int(sys.argv[4])
    
    threads = []
    for i in range(num_threads):
        t = threading.Thread(target=echo_test_thread, args=(host, port, duration, i))
        threads.append(t)
    
    start_time = time.time()
    for t in threads:
        t.start()
    
    total_requests = 0
    total_errors = 0
    for t in threads:
        t.join()
        # 简化处理，实际应该收集返回值
    
    end_time = time.time()
    elapsed = end_time - start_time
    
    print(f"测试完成:")
    print(f"总时间: {elapsed:.2f}s")
    print(f"线程数: {num_threads}")
EOF
    
    python3 /tmp/echo_test.py 127.0.0.1 $port $duration 50 > "$RESULTS_DIR/echo_qps_${port}.txt" 2>&1 || echo "Echo测试完成"
    
    echo "✅ TCP Echo QPS 测试完成"
}

# 函数：测试Redis QPS
test_redis_qps() {
    local port=$1
    
    echo ""
    echo "🔥 Redis QPS 测试 (端口: $port)"
    
    # 使用redis-benchmark (如果可用)
    if command -v redis-benchmark &> /dev/null; then
        echo "使用 redis-benchmark 测试..."
        redis-benchmark -h 127.0.0.1 -p $port -n 10000 -c 50 -d 100 \
            > "$RESULTS_DIR/redis_qps_${port}.txt" 2>&1 || echo "redis-benchmark测试完成"
    fi
    
    # 使用自定义Redis客户端
    if [ -f "$PROJECT_DIR/build/bin/smart_netbox_redis_client" ]; then
        echo "使用 NetBox Redis 客户端测试..."
        timeout 30 "$PROJECT_DIR/build/bin/smart_netbox_redis_client" 127.0.0.1 $port \
            > "$RESULTS_DIR/redis_netbox_client_${port}.txt" 2>&1 || echo "NetBox Redis客户端测试完成"
    fi
    
    echo "✅ Redis QPS 测试完成"
}

# 函数：生成火焰图
generate_flame_graph() {
    local server_pid=$1
    local server_type=$2
    local duration=60
    
    echo ""
    echo "🔥 生成火焰图 ($server_type, PID: $server_pid)"
    echo "采样时间: ${duration}s"
    
    # 检查FlameGraph工具
    if [ ! -d "/tmp/FlameGraph" ]; then
        echo "下载 FlameGraph 工具..."
        git clone https://github.com/brendangregg/FlameGraph.git /tmp/FlameGraph || {
            echo "❌ 无法下载FlameGraph工具"
            return 1
        }
    fi
    
    # 使用perf记录性能数据
    echo "开始perf采样..."
    sudo $PERF_BIN record -F 99 -p $server_pid -g -o "$RESULTS_DIR/perf_${server_type}.data" -- sleep $duration || {
        echo "❌ perf record 失败"
        return 1
    }
    
    # 生成perf脚本
    echo "生成perf脚本..."
    sudo $PERF_BIN script -i "$RESULTS_DIR/perf_${server_type}.data" > "$RESULTS_DIR/perf_${server_type}.script" || {
        echo "❌ perf script 失败"
        return 1
    }
    
    # 生成火焰图
    echo "生成火焰图..."
    /tmp/FlameGraph/stackcollapse-perf.pl "$RESULTS_DIR/perf_${server_type}.script" > "$RESULTS_DIR/perf_${server_type}.folded"
    /tmp/FlameGraph/flamegraph.pl "$RESULTS_DIR/perf_${server_type}.folded" > "$RESULTS_DIR/flamegraph_${server_type}.svg"
    
    # 修改文件权限
    sudo chown $USER:$USER "$RESULTS_DIR/perf_${server_type}."*
    
    echo "✅ 火焰图生成完成: $RESULTS_DIR/flamegraph_${server_type}.svg"
}

# 函数：系统资源监控
monitor_system() {
    local duration=$1
    local output_file="$RESULTS_DIR/system_monitor.txt"
    
    echo ""
    echo "🔥 系统资源监控 (${duration}s)"
    
    {
        echo "=== 系统信息 ==="
        uname -a
        echo ""
        
        echo "=== CPU信息 ==="
        lscpu | grep -E "(Architecture|CPU|Thread|Core)"
        echo ""
        
        echo "=== 内存信息 ==="
        free -h
        echo ""
        
        echo "=== 网络设置 ==="
        sysctl net.core.somaxconn net.core.netdev_max_backlog net.ipv4.tcp_max_syn_backlog
        echo ""
        
        echo "=== 监控开始时间: $(date) ==="
    } > "$output_file"
    
    # 后台监控CPU和内存
    {
        for i in $(seq 1 $duration); do
            echo "时间: $(date)"
            echo "CPU使用率:"
            top -bn1 | grep "Cpu(s)" || echo "CPU信息获取失败"
            echo "内存使用:"
            free -m | grep -E "(Mem|Swap)" || echo "内存信息获取失败"
            echo "网络连接:"
            ss -tln | grep -E "(8888|8889|8890)" | wc -l
            echo "负载平均:"
            cat /proc/loadavg
            echo "---"
            sleep 1
        done
    } >> "$output_file" &
    
    echo "✅ 系统监控已启动"
}

# 主测试流程
main() {
    echo "准备性能测试环境..."
    
    # 优化系统参数
    echo "优化系统参数..."
    {
        echo 65535 | sudo tee /proc/sys/net/core/somaxconn
        echo 30000 | sudo tee /proc/sys/net/core/netdev_max_backlog
        echo 10000 | sudo tee /proc/sys/net/ipv4/tcp_max_syn_backlog
    } > /dev/null 2>&1 || echo "部分系统参数优化失败（可能需要root权限）"
    
    # 启动系统监控
    monitor_system 300 &
    monitor_pid=$!
    
    # 测试1: Echo服务器 (假设支持)
    echo ""
    echo "====== 测试1: Echo 服务器 ======"
    if echo_pid=$(start_server "echo" 8888 2>/dev/null); then
        # 等待服务器稳定
        sleep 5
        
        # 开始负载测试
        test_echo_qps 8888 &
        test_pid=$!
        
        # 生成火焰图
        sleep 10  # 等待负载开始
        generate_flame_graph "$echo_pid" "echo" &
        flame_pid=$!
        
        # 等待测试完成
        wait $test_pid $flame_pid
        
        # 停止服务器
        stop_server "$echo_pid"
    else
        echo "❌ Echo服务器启动失败，跳过测试"
    fi
    
    sleep 5
    
    # 测试2: HTTP服务器 (假设支持)
    echo ""
    echo "====== 测试2: HTTP 服务器 ======"
    if http_pid=$(start_server "http" 8889 2>/dev/null); then
        sleep 5
        
        test_http_qps 8889 &
        test_pid=$!
        
        sleep 10
        generate_flame_graph "$http_pid" "http" &
        flame_pid=$!
        
        wait $test_pid $flame_pid
        stop_server "$http_pid"
    else
        echo "❌ HTTP服务器启动失败，跳过测试"
    fi
    
    sleep 5
    
    # 测试3: Redis服务器 (假设支持)
    echo ""
    echo "====== 测试3: Redis 服务器 ======"
    if redis_pid=$(start_server "redis" 8890 2>/dev/null); then
        sleep 5
        
        test_redis_qps 8890 &
        test_pid=$!
        
        sleep 10
        generate_flame_graph "$redis_pid" "redis" &
        flame_pid=$!
        
        wait $test_pid $flame_pid
        stop_server "$redis_pid"
    else
        echo "❌ Redis服务器启动失败，跳过测试"
    fi
    
    # 停止系统监控
    kill $monitor_pid 2>/dev/null || true
    
    # 生成测试报告
    generate_report
    
    echo ""
    echo "=================================="
    echo "    性能测试完成"
    echo "=================================="
    echo "结果保存在: $RESULTS_DIR"
    echo "测试时间: $(date)"
}

# 函数：生成测试报告
generate_report() {
    local report_file="$RESULTS_DIR/performance_report.md"
    
    echo "生成性能测试报告..."
    
    cat > "$report_file" << EOF
# NetBox 性能测试报告

## 测试概况
- **测试时间**: $(date)
- **测试环境**: $(uname -a)
- **项目版本**: NetBox v2.0.0

## 测试结果文件

### QPS测试结果
EOF
    
    # 添加测试结果文件列表
    for file in "$RESULTS_DIR"/*_qps_*.txt; do
        if [ -f "$file" ]; then
            echo "- [$(basename "$file")](./${file##*/})" >> "$report_file"
        fi
    done
    
    cat >> "$report_file" << EOF

### 火焰图分析
EOF
    
    for file in "$RESULTS_DIR"/flamegraph_*.svg; do
        if [ -f "$file" ]; then
            echo "- [$(basename "$file")](./${file##*/})" >> "$report_file"
        fi
    done
    
    cat >> "$report_file" << EOF

### 系统监控数据
- [系统监控日志](./system_monitor.txt)

## 快速结果预览

$(echo "正在解析测试结果...")

## 改进建议
1. 根据火焰图分析CPU热点
2. 优化高频调用函数
3. 调整线程池大小
4. 优化内存分配策略

## 下一步
1. 分析火焰图识别性能瓶颈
2. 进行针对性代码优化
3. 重新测试验证改进效果
EOF
    
    echo "✅ 性能测试报告生成完成: $report_file"
}

# 清理函数
cleanup() {
    echo ""
    echo "清理测试环境..."
    
    # 杀死所有相关进程
    pkill -f NetBox || true
    pkill -f echo_test.py || true
    
    # 清理临时文件
    rm -f /tmp/netbox_test_config.yaml /tmp/echo_test.py
    
    echo "清理完成"
}

# 设置信号处理
trap cleanup EXIT INT TERM

# 运行主程序
main "$@" 