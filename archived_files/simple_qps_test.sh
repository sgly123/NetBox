#!/bin/bash

# 简单的QPS测试脚本，使用现有的echo客户端
set -e

PROJECT_DIR="/home/sgly/桌面/NetBox"
RESULTS_DIR="$PROJECT_DIR/performance_results"
SERVER_BIN="$PROJECT_DIR/build/bin/NetBox"
CLIENT_BIN="$PROJECT_DIR/build/bin/echo_client_fixed"
CONFIG_FILE="$PROJECT_DIR/config/config_echo_local.yaml"

mkdir -p "$RESULTS_DIR"
cd "$PROJECT_DIR"

echo "=================================="
echo "    NetBox 简单QPS测试"
echo "=================================="
echo "测试时间: $(date)"
echo ""

# 清理函数
cleanup() {
    echo "正在清理..."
    pkill -f NetBox || true
    pkill -f echo_client || true
    sleep 2
    echo "清理完成"
}

trap cleanup EXIT INT TERM

# 启动服务器
start_server() {
    echo "启动NetBox服务器..."
    
    $SERVER_BIN "$CONFIG_FILE" > "$RESULTS_DIR/simple_qps_server.log" 2>&1 &
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

# 使用现有客户端进行QPS测试
test_qps_with_client() {
    local test_name=$1
    local num_clients=$2
    local duration=$3
    
    echo ""
    echo "🔥 QPS测试: $test_name"
    echo "客户端数: $num_clients, 持续时间: ${duration}秒"
    
    # 统计变量
    local total_requests=0
    local start_time=$(date +%s.%N)
    
    # 启动多个客户端
    local pids=()
    for ((i=1; i<=num_clients; i++)); do
        {
            # 每个客户端发送消息
            local client_requests=0
            local client_start=$(date +%s.%N)
            
            while true; do
                local current_time=$(date +%s.%N)
                local elapsed=$(echo "$current_time - $client_start" | bc)
                
                if (( $(echo "$elapsed > $duration" | bc -l) )); then
                    break
                fi
                
                # 使用echo客户端发送消息
                echo "Client-$i-Request-$client_requests-$(date +%s%N)" | \
                    timeout 1 $CLIENT_BIN 127.0.0.1 8888 > /dev/null 2>&1 && \
                    ((client_requests++)) || true
                
                # 控制发送频率
                sleep 0.01
            done
            
            echo $client_requests > "/tmp/client_${i}_count.txt"
        } &
        pids+=($!)
    done
    
    # 等待所有客户端完成
    for pid in "${pids[@]}"; do
        wait $pid 2>/dev/null || true
    done
    
    local end_time=$(date +%s.%N)
    local total_duration=$(echo "$end_time - $start_time" | bc)
    
    # 统计总请求数
    for ((i=1; i<=num_clients; i++)); do
        if [ -f "/tmp/client_${i}_count.txt" ]; then
            local client_count=$(cat "/tmp/client_${i}_count.txt")
            total_requests=$((total_requests + client_count))
            rm -f "/tmp/client_${i}_count.txt"
        fi
    done
    
    # 计算QPS
    local qps=$(echo "scale=2; $total_requests / $total_duration" | bc)
    
    # 输出结果
    {
        echo "=== $test_name 测试结果 ==="
        echo "测试时间: $(date)"
        echo "客户端数: $num_clients"
        echo "目标持续时间: ${duration}秒"
        echo "实际持续时间: ${total_duration}秒"
        echo "总请求数: $total_requests"
        echo "QPS: $qps"
        echo "平均每客户端请求数: $((total_requests / num_clients))"
        echo "========================="
    } | tee "$RESULTS_DIR/simple_qps_${test_name}.txt"
    
    echo "✅ $test_name 测试完成，QPS: $qps"
}

# 生成服务器负载进行火焰图分析
test_with_flame_graph() {
    echo ""
    echo "🔥 火焰图性能分析测试"
    
    # 启动负载生成器
    {
        for i in {1..20}; do
            {
                for j in {1..1000}; do
                    echo "FlameTest-$i-$j-$(date +%s%N)" | \
                        timeout 0.5 $CLIENT_BIN 127.0.0.1 8888 > /dev/null 2>&1 || true
                    sleep 0.01
                done
            } &
        done
        wait
    } &
    LOAD_PID=$!
    
    sleep 5  # 等待负载稳定
    
    # 进行perf采样
    if command -v perf > /dev/null; then
        echo "开始perf采样 (30秒)..."
        sudo perf record -F 99 -p $SERVER_PID -g -o "$RESULTS_DIR/flame_perf.data" -- sleep 30 2>/dev/null || {
            echo "⚠️ perf record 需要sudo权限或失败"
        }
        
        # 生成perf报告
        if [ -f "$RESULTS_DIR/flame_perf.data" ]; then
            echo "生成perf报告..."
            sudo perf report -i "$RESULTS_DIR/flame_perf.data" --stdio > "$RESULTS_DIR/flame_perf_report.txt" 2>/dev/null || echo "perf report生成失败"
            sudo perf script -i "$RESULTS_DIR/flame_perf.data" > "$RESULTS_DIR/flame_perf_script.txt" 2>/dev/null || echo "perf script生成失败"
            
            # 修改文件权限
            sudo chown $USER:$USER "$RESULTS_DIR/flame_perf."* 2>/dev/null || true
            
            # 生成火焰图
            if [ -f "$RESULTS_DIR/flame_perf_script.txt" ]; then
                if [ ! -d "/tmp/FlameGraph" ]; then
                    echo "下载FlameGraph工具..."
                    git clone https://github.com/brendangregg/FlameGraph.git /tmp/FlameGraph 2>/dev/null || {
                        echo "❌ 无法下载FlameGraph工具"
                    }
                fi
                
                if [ -d "/tmp/FlameGraph" ]; then
                    echo "生成火焰图..."
                    /tmp/FlameGraph/stackcollapse-perf.pl "$RESULTS_DIR/flame_perf_script.txt" > "$RESULTS_DIR/flame_folded.txt" 2>/dev/null
                    /tmp/FlameGraph/flamegraph.pl "$RESULTS_DIR/flame_folded.txt" > "$RESULTS_DIR/netbox_flame_graph.svg" 2>/dev/null
                    
                    if [ -f "$RESULTS_DIR/netbox_flame_graph.svg" ]; then
                        echo "✅ 火焰图生成成功: $RESULTS_DIR/netbox_flame_graph.svg"
                    else
                        echo "❌ 火焰图生成失败"
                    fi
                fi
            fi
        fi
    else
        echo "⚠️ perf工具不可用，跳过火焰图生成"
    fi
    
    # 停止负载生成器
    kill $LOAD_PID 2>/dev/null || true
    wait $LOAD_PID 2>/dev/null || true
}

# 主测试流程
main() {
    echo "准备简单QPS测试..."
    
    # 检查依赖
    if [ ! -f "$CLIENT_BIN" ]; then
        echo "❌ Echo客户端不存在: $CLIENT_BIN"
        exit 1
    fi
    
    if ! command -v bc &> /dev/null; then
        echo "⚠️ 安装bc工具进行计算"
        sudo apt-get update && sudo apt-get install -y bc || echo "bc安装失败"
    fi
    
    # 启动服务器
    if ! start_server; then
        echo "❌ 无法启动服务器，退出测试"
        exit 1
    fi
    
    echo "等待5秒让服务器稳定..."
    sleep 5
    
    # 运行多个QPS测试
    test_qps_with_client "single" 1 10
    sleep 3
    
    test_qps_with_client "low_concurrency" 5 10
    sleep 3
    
    test_qps_with_client "medium_concurrency" 10 10
    sleep 3
    
    test_qps_with_client "high_concurrency" 20 10
    sleep 3
    
    # 火焰图分析
    test_with_flame_graph
    
    # 生成简单报告
    {
        echo "# NetBox 简单QPS测试报告"
        echo ""
        echo "## 测试环境"
        echo "- 测试时间: $(date)"
        echo "- 操作系统: $(uname -a)"
        echo "- CPU核心: $(nproc)"
        echo "- 内存: $(free -h | grep Mem | awk '{print $2}')"
        echo ""
        echo "## 测试结果"
        echo ""
        
        for result_file in "$RESULTS_DIR"/simple_qps_*.txt; do
            if [ -f "$result_file" ]; then
                echo "### $(basename "$result_file" .txt)"
                echo '```'
                cat "$result_file"
                echo '```'
                echo ""
            fi
        done
        
        if [ -f "$RESULTS_DIR/netbox_flame_graph.svg" ]; then
            echo "## 火焰图分析"
            echo "火焰图文件: netbox_flame_graph.svg"
            echo ""
        fi
        
        if [ -f "$RESULTS_DIR/flame_perf_report.txt" ]; then
            echo "## 性能热点分析"
            echo '```'
            head -30 "$RESULTS_DIR/flame_perf_report.txt"
            echo '```'
        fi
    } > "$RESULTS_DIR/simple_qps_report.md"
    
    # 停止服务器
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null || true
        echo "✅ 服务器已停止"
    fi
    
    echo ""
    echo "=================================="
    echo "    简单QPS测试完成"
    echo "=================================="
    echo "结果目录: $RESULTS_DIR"
    echo "主要报告: $RESULTS_DIR/simple_qps_report.md"
    if [ -f "$RESULTS_DIR/netbox_flame_graph.svg" ]; then
        echo "火焰图: $RESULTS_DIR/netbox_flame_graph.svg"
    fi
    echo "=================================="
}

# 运行主程序
main "$@" 