#!/bin/bash

# 修复版性能分析脚本：跟踪服务器所有子进程
# 用法：sudo ./perf_flamegraph.sh [服务器命令] [客户端脚本] [测试时间(秒)]

# 检查权限和参数
if [ "$(id -u)" -ne 0 ]; then
    echo "错误：需要root权限"
    exit 1
fi

if [ $# -lt 3 ]; then
    echo "用法：$0 [服务器命令] [客户端脚本] [测试时间]"
    exit 1
fi

SERVER_CMD=$1
CLIENT_SCRIPT=$2
DURATION=$3
OUTPUT_DIR="flamegraph_$(date +%Y%m%d_%H%M%S)"
PERF_DATA="perf.data"
FLAMEGRAPH_DIR="FlameGraph"
COLLAPSED_FILE="$OUTPUT_DIR/out.perf-folded"

# 检查客户端脚本
if [ ! -f "$CLIENT_SCRIPT" ]; then
    echo "错误：客户端脚本 $CLIENT_SCRIPT 不存在"
    exit 1
fi

# 检查perf
check_perf() {
    echo "检查perf工具..."
    PERF_BIN=$(which perf)
    if [ -z "$PERF_BIN" ]; then
        echo "错误：未找到perf"
        exit 1
    fi
}

# 准备火焰图工具
prepare_flamegraph() {
    echo "准备火焰图工具..."
    [ ! -d "$FLAMEGRAPH_DIR" ] && git clone https://github.com/brendangregg/FlameGraph.git || (cd $FLAMEGRAPH_DIR && git pull)
}

# 启动服务器并获取所有相关进程（包括子进程）
start_server() {
    echo "启动服务器：$SERVER_CMD"
    $SERVER_CMD &
    PARENT_PID=$!
    echo "服务器父进程PID：$PARENT_PID"
    
    # 等待服务器初始化并生成子进程（根据服务器启动速度调整时间）
    sleep 8
    if ! ps -p $PARENT_PID > /dev/null; then
        echo "服务器启动失败"
        exit 1
    fi
    
    # 获取所有子进程ID（服务器可能用子进程处理请求）
    SERVER_PIDS=$(pgrep -P $PARENT_PID | tr '\n' ' ')
    if [ -z "$SERVER_PIDS" ]; then
        echo "未检测到子进程，使用父进程PID"
        SERVER_PIDS=$PARENT_PID
    else
        echo "检测到服务器子进程PID：$SERVER_PIDS"
    fi
}

# 运行客户端（增加请求量，确保perf能捕获数据）
run_client() {
    echo "启动客户端：$CLIENT_SCRIPT"
    # 增加并发数和测试时间，确保服务器有足够负载
    python3 "$CLIENT_SCRIPT" 192.168.88.135 8888 100 $DURATION "test_data" &
    CLIENT_PID=$!
    
    sleep 5  # 等待客户端开始发送请求
    if ! ps -p $CLIENT_PID > /dev/null; then
        echo "客户端启动失败"
        cleanup
        exit 1
    fi
}

# 收集性能数据（跟踪所有服务器进程，提高采样频率）
collect_perf_data() {
    echo "收集性能数据（${DURATION}秒）..."
    # -F 199：提高采样频率（默认99可能太低）
    # -g：记录调用栈
    # --pid $SERVER_PIDS：跟踪所有相关进程
    $PERF_BIN record -F 199 -g --pid $SERVER_PIDS -- sleep $DURATION 2>/dev/null
    
    # 检查perf数据是否有效
    if [ ! -f "$PERF_DATA" ] || [ $(stat -c%s "$PERF_DATA") -lt 2048 ]; then
        echo "警告：perf数据可能不完整（大小：$(stat -c%s "$PERF_DATA" 2>/dev/null)字节）"
        # 尝试直接跟踪进程名（如果PID跟踪失败）
        SERVER_NAME=$(echo $SERVER_CMD | awk '{print $1}' | xargs basename)
        echo "尝试通过进程名跟踪：$SERVER_NAME"
        $PERF_BIN record -F 199 -g --comm $SERVER_NAME -- sleep $DURATION 2>/dev/null
    fi
}

# 生成火焰图（保持数据校验）
generate_flamegraph() {
    echo "生成火焰图..."
    mkdir -p $OUTPUT_DIR
    
    $PERF_BIN script -i $PERF_DATA 2>/dev/null | $FLAMEGRAPH_DIR/stackcollapse-perf.pl > $COLLAPSED_FILE
    
    if [ ! -f "$COLLAPSED_FILE" ] || [ $(stat -c%s "$COLLAPSED_FILE") -eq 0 ]; then
        echo "错误：没有有效的性能数据"
        echo "尝试手动指定进程名运行：sudo perf record -F 199 -g -p \$(pgrep NetBox) -- sleep 30"
        return 1
    fi
    
    $FLAMEGRAPH_DIR/flamegraph.pl $COLLAPSED_FILE > $OUTPUT_DIR/flamegraph.svg
    echo "火焰图生成成功：$OUTPUT_DIR/flamegraph.svg"
    return 0
}

# 清理资源
cleanup() {
    echo "清理资源..."
    [ -n "$PARENT_PID" ] && ps -p $PARENT_PID > /dev/null && kill $PARENT_PID
    [ -n "$CLIENT_PID" ] && ps -p $CLIENT_PID > /dev/null && kill $CLIENT_PID
    rm -f $PERF_DATA
}

# 主流程
check_perf
prepare_flamegraph
start_server
run_client
collect_perf_data
cleanup
generate_flamegraph

echo "流程结束"
