#!/bin/bash

# NetBox 快速性能测试脚本
set -e

PROJECT_DIR="/home/sgly/桌面/NetBox"
RESULTS_DIR="$PROJECT_DIR/performance_results"
SERVER_BIN="$PROJECT_DIR/build/bin/NetBox"

# 创建结果目录
mkdir -p "$RESULTS_DIR"
cd "$PROJECT_DIR"

echo "=================================="
echo "    NetBox 快速性能测试"
echo "=================================="
echo "测试时间: $(date)"
echo ""

# 清理函数
cleanup() {
    echo "清理测试环境..."
    pkill -f NetBox || true
    pkill -f wrk || true
    pkill -f ab || true
    sleep 2
}

trap cleanup EXIT INT TERM

# 1. 测试Echo服务器
echo "====== 测试1: Echo服务器性能 ======"

# 启动Echo服务器
echo "启动Echo服务器..."
$SERVER_BIN config/config_echo.yaml > "$RESULTS_DIR/echo_server.log" 2>&1 &
SERVER_PID=$!
echo "服务器PID: $SERVER_PID"

# 等待服务器启动
sleep 3

# 检查服务器是否运行
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "❌ Echo服务器启动失败，查看日志："
    cat "$RESULTS_DIR/echo_server.log"
    exit 1
fi

echo "✅ Echo服务器启动成功"

# 检查端口是否监听
if ! netstat -tln | grep ":8888" > /dev/null; then
    echo "❌ 端口8888未监听，等待更长时间..."
    sleep 5
    if ! netstat -tln | grep ":8888" > /dev/null; then
        echo "❌ 端口8888仍未监听，查看日志："
        cat "$RESULTS_DIR/echo_server.log"
        stop_server $SERVER_PID
        exit 1
    fi
fi

echo "✅ 端口8888正在监听"

# 使用简单的TCP连接测试
echo "进行简单连接测试..."
timeout 5 bash -c 'echo "test" | nc 192.168.88.135 8888' > "$RESULTS_DIR/simple_test.txt" 2>&1 || echo "简单连接测试完成"

# 停止Echo服务器
kill $SERVER_PID 2>/dev/null || true
sleep 2

echo ""

# 2. 测试HTTP功能 (如果支持)
echo "====== 测试2: HTTP性能测试 ======"

# 检查是否有HTTP配置
if [ -f "config/config_http.yaml" ]; then
    echo "启动HTTP服务器..."
    $SERVER_BIN config/config_http.yaml > "$RESULTS_DIR/http_server.log" 2>&1 &
    HTTP_PID=$!
    sleep 3
    
    if kill -0 $HTTP_PID 2>/dev/null; then
        echo "✅ HTTP服务器启动成功"
        
        # 使用wrk测试HTTP性能
        if command -v wrk > /dev/null; then
            echo "使用wrk测试HTTP QPS..."
            wrk -t 4 -c 50 -d 30s --timeout 10s "http://192.168.88.135:8889/" \
                > "$RESULTS_DIR/http_wrk_results.txt" 2>&1 || echo "wrk测试完成"
        fi
        
        # 使用ab测试
        if command -v ab > /dev/null; then
            echo "使用ab测试HTTP QPS..."
            ab -n 1000 -c 20 "http://192.168.88.135:8889/" \
                > "$RESULTS_DIR/http_ab_results.txt" 2>&1 || echo "ab测试完成"
        fi
        
        kill $HTTP_PID 2>/dev/null || true
    else
        echo "❌ HTTP服务器启动失败"
    fi
else
    echo "未找到HTTP配置文件，跳过HTTP测试"
fi

echo ""

# 3. 系统资源使用分析
echo "====== 测试3: 系统资源分析 ======"

# 重新启动Echo服务器进行资源监控
echo "重新启动Echo服务器进行资源监控..."
$SERVER_BIN config/config_echo.yaml > "$RESULTS_DIR/echo_monitor.log" 2>&1 &
MONITOR_PID=$!
sleep 3

if kill -0 $MONITOR_PID 2>/dev/null; then
    echo "✅ 监控服务器启动成功"
    
    # 收集系统信息
    {
        echo "=== 系统信息 ==="
        echo "时间: $(date)"
        echo "内核: $(uname -r)"
        echo "CPU: $(nproc) 核心"
        echo ""
        
        echo "=== CPU信息 ==="
        lscpu | grep -E "(Architecture|CPU|Model|Thread|Core)"
        echo ""
        
        echo "=== 内存信息 ==="
        free -h
        echo ""
        
        echo "=== 网络配置 ==="
        sysctl net.core.somaxconn net.core.netdev_max_backlog 2>/dev/null || echo "无法获取网络参数"
        echo ""
        
        echo "=== 当前连接 ==="
        ss -tln | grep ":8888" || echo "无活动连接"
        echo ""
    } > "$RESULTS_DIR/system_info.txt"
    
    # 监控资源使用
    echo "监控资源使用30秒..."
    {
        for i in {1..30}; do
            echo "=== 时间: $(date) ===" 
            echo "CPU使用率:"
            top -bn1 | grep "Cpu(s)" | head -1
            echo "内存使用:"
            free -m | grep "Mem:"
            echo "进程信息:"
            ps aux | grep NetBox | grep -v grep | head -3
            echo "网络连接:"
            ss -tn | grep ":8888" | wc -l
            echo "---"
            sleep 1
        done
    } > "$RESULTS_DIR/resource_monitor.txt" &
    MONITOR_BG_PID=$!
    
    # 在监控期间进行简单的负载测试
    sleep 5
    echo "在监控期间进行负载测试..."
    
    # 创建简单的并发连接脚本
    for i in {1..20}; do
        (
            for j in {1..50}; do
                echo "test message $i-$j" | timeout 1 nc 192.168.88.135 8888 > /dev/null 2>&1 || true
                sleep 0.1
            done
        ) &
    done
    
    # 等待监控完成
    wait $MONITOR_BG_PID
    
    kill $MONITOR_PID 2>/dev/null || true
else
    echo "❌ 监控服务器启动失败"
fi

echo ""

# 4. 使用perf进行性能分析 (如果可用)
echo "====== 测试4: perf性能分析 ======"

if command -v perf > /dev/null 2>&1; then
    echo "启动服务器进行perf分析..."
    $SERVER_BIN config/config_echo.yaml > "$RESULTS_DIR/perf_server.log" 2>&1 &
    PERF_PID=$!
    sleep 3
    
    if kill -0 $PERF_PID 2>/dev/null; then
        echo "✅ perf分析服务器启动成功"
        
        # 启动负载生成器
        (
            for i in {1..10}; do
                (
                    for j in {1..100}; do
                        echo "perf test $i-$j" | timeout 0.5 nc 192.168.88.135 8888 > /dev/null 2>&1 || true
                        sleep 0.05
                    done
                ) &
            done
            wait
        ) &
        LOAD_PID=$!
        
        sleep 2
        
        # 进行perf采样
        echo "开始perf采样 (20秒)..."
        sudo perf record -F 99 -p $PERF_PID -g -o "$RESULTS_DIR/netbox_perf.data" -- sleep 20 2>/dev/null || {
            echo "⚠️ perf record 需要sudo权限或失败"
        }
        
        # 生成perf报告
        if [ -f "$RESULTS_DIR/netbox_perf.data" ]; then
            echo "生成perf报告..."
            sudo perf report -i "$RESULTS_DIR/netbox_perf.data" --stdio > "$RESULTS_DIR/perf_report.txt" 2>/dev/null || echo "perf report生成失败"
            sudo perf script -i "$RESULTS_DIR/netbox_perf.data" > "$RESULTS_DIR/perf_script.txt" 2>/dev/null || echo "perf script生成失败"
            
            # 修改文件权限
            sudo chown $USER:$USER "$RESULTS_DIR/netbox_perf."* 2>/dev/null || true
        fi
        
        wait $LOAD_PID 2>/dev/null || true
        kill $PERF_PID 2>/dev/null || true
    else
        echo "❌ perf分析服务器启动失败"
    fi
else
    echo "⚠️ perf工具不可用，跳过性能分析"
fi

echo ""

# 5. 生成火焰图 (如果有perf数据)
echo "====== 测试5: 火焰图生成 ======"

if [ -f "$RESULTS_DIR/perf_script.txt" ]; then
    echo "尝试生成火焰图..."
    
    # 下载FlameGraph工具 (如果不存在)
    if [ ! -d "/tmp/FlameGraph" ]; then
        echo "下载FlameGraph工具..."
        git clone https://github.com/brendangregg/FlameGraph.git /tmp/FlameGraph 2>/dev/null || {
            echo "❌ 无法下载FlameGraph工具"
        }
    fi
    
    if [ -d "/tmp/FlameGraph" ]; then
        echo "生成火焰图..."
        /tmp/FlameGraph/stackcollapse-perf.pl "$RESULTS_DIR/perf_script.txt" > "$RESULTS_DIR/netbox_perf.folded" 2>/dev/null
        /tmp/FlameGraph/flamegraph.pl "$RESULTS_DIR/netbox_perf.folded" > "$RESULTS_DIR/netbox_flamegraph.svg" 2>/dev/null
        
        if [ -f "$RESULTS_DIR/netbox_flamegraph.svg" ]; then
            echo "✅ 火焰图生成成功: $RESULTS_DIR/netbox_flamegraph.svg"
        else
            echo "❌ 火焰图生成失败"
        fi
    fi
else
    echo "⚠️ 没有perf数据，跳过火焰图生成"
fi

echo ""

# 6. 生成测试总结报告
echo "====== 生成测试报告 ======"

REPORT_FILE="$RESULTS_DIR/performance_summary.md"

cat > "$REPORT_FILE" << EOF
# NetBox 性能测试总结报告

## 测试概况
- **测试时间**: $(date)
- **测试环境**: $(uname -a)
- **CPU核心数**: $(nproc)
- **内存大小**: $(free -h | grep Mem | awk '{print $2}')

## 测试项目

### 1. Echo服务器基本功能测试
- 配置文件: config/config_echo.yaml
- 监听地址: 192.168.88.135:8888
- 协议类型: SimpleHeader (4字节长度前缀)

### 2. HTTP性能测试
$(if [ -f "$RESULTS_DIR/http_wrk_results.txt" ] || [ -f "$RESULTS_DIR/http_ab_results.txt" ]; then
    echo "- HTTP服务器测试已完成"
else
    echo "- HTTP服务器测试跳过（配置不可用）"
fi)

### 3. 系统资源监控
- 监控时长: 30秒
- 并发连接: 20个客户端，每个发送50条消息
- 资源使用数据: resource_monitor.txt

### 4. 性能分析 (perf)
$(if [ -f "$RESULTS_DIR/perf_report.txt" ]; then
    echo "- perf性能分析已完成"
    echo "- 采样频率: 99Hz"
    echo "- 采样时长: 20秒"
else
    echo "- perf性能分析跳过（需要root权限或工具不可用）"
fi)

### 5. 火焰图分析
$(if [ -f "$RESULTS_DIR/netbox_flamegraph.svg" ]; then
    echo "- 火焰图生成成功: netbox_flamegraph.svg"
    echo "- 可用于CPU热点分析和优化指导"
else
    echo "- 火焰图生成跳过（依赖perf数据）"
fi)

## 结果文件列表

### 服务器日志
EOF

# 列出所有生成的文件
for file in "$RESULTS_DIR"/*.log; do
    if [ -f "$file" ]; then
        echo "- $(basename "$file")" >> "$REPORT_FILE"
    fi
done

cat >> "$REPORT_FILE" << EOF

### 性能测试结果
EOF

for file in "$RESULTS_DIR"/*.txt; do
    if [ -f "$file" ]; then
        echo "- $(basename "$file")" >> "$REPORT_FILE"
    fi
done

cat >> "$REPORT_FILE" << EOF

### 分析文件
EOF

for file in "$RESULTS_DIR"/*.{svg,data,folded}; do
    if [ -f "$file" ]; then
        echo "- $(basename "$file")" >> "$REPORT_FILE"
    fi
done

cat >> "$REPORT_FILE" << EOF

## 快速结果预览

### 系统信息
\`\`\`
$(head -20 "$RESULTS_DIR/system_info.txt" 2>/dev/null || echo "系统信息文件不存在")
\`\`\`

### 性能分析概要
$(if [ -f "$RESULTS_DIR/perf_report.txt" ]; then
    echo "\`\`\`"
    head -30 "$RESULTS_DIR/perf_report.txt" | grep -E "(Samples|Event|Children|Self|Command|Symbol)" | head -10
    echo "\`\`\`"
else
    echo "无perf分析数据"
fi)

## 改进建议

1. **网络优化**:
   - 调整TCP缓冲区大小
   - 优化连接处理逻辑

2. **并发优化**:
   - 分析线程池使用效率
   - 优化锁竞争

3. **内存优化**:
   - 分析内存分配模式
   - 减少不必要的拷贝

4. **协议优化**:
   - 优化协议解析性能
   - 减少系统调用次数

## 下一步操作

1. 分析火焰图识别CPU热点
2. 使用更长时间的压力测试
3. 测试更高并发场景
4. 与其他网络框架对比

---
生成时间: $(date)
EOF

echo "✅ 测试报告生成完成: $REPORT_FILE"

echo ""
echo "=================================="
echo "    性能测试完成"
echo "=================================="
echo "结果目录: $RESULTS_DIR"
echo "主要文件:"
ls -la "$RESULTS_DIR"/ | grep -E "\.(txt|log|svg|md)$" | head -10
echo ""
echo "查看详细报告: cat $REPORT_FILE"
echo "==================================" 