#!/bin/bash

##############################################################################
# NetBox Echo服务器压力测试脚本
# 
# 功能特性：
# 1. 自动编译服务器和客户端
# 2. 启动echo服务器
# 3. 运行压力测试客户端
# 4. 生成perf性能报告
# 5. 生成火焰图
# 6. 系统性能监控
# 7. 生成完整测试报告
##############################################################################

set -euo pipefail

# 脚本配置
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
TOOLS_DIR="$PROJECT_ROOT/tools"
RESULTS_DIR="$PROJECT_ROOT/performance_results/$(date +%Y%m%d_%H%M%S)"

# 默认测试参数
SERVER_IP="127.0.0.1"
SERVER_PORT="8888"
SERVER_THREADS="4"
CLIENT_THREADS="8"
CONNECTIONS_PER_THREAD="20"
REQUESTS_PER_CONNECTION="1000"
MESSAGE_SIZE="64"
TEST_DURATION="60"
PERF_RECORD_TIME="30"
ENABLE_PERF="true"
ENABLE_FLAMEGRAPH="true"
ENABLE_SYSTEM_MONITOR="true"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

log_perf() {
    echo -e "${PURPLE}[PERF]${NC} $1"
}

# 使用说明
usage() {
    cat << EOF
NetBox Echo服务器压力测试脚本

用法: $0 [选项]

服务器选项:
  -H, --host <ip>           服务器IP地址 (默认: $SERVER_IP)
  -P, --port <port>         服务器端口 (默认: $SERVER_PORT)
  -T, --server-threads <n>  服务器线程数 (默认: $SERVER_THREADS)

客户端选项:
  -t, --threads <n>         客户端线程数 (默认: $CLIENT_THREADS)
  -c, --connections <n>     每线程连接数 (默认: $CONNECTIONS_PER_THREAD)
  -r, --requests <n>        每连接请求数 (默认: $REQUESTS_PER_CONNECTION)
  -s, --size <bytes>        消息大小 (默认: $MESSAGE_SIZE)
  -d, --duration <sec>      测试时长 (默认: $TEST_DURATION)

性能分析选项:
  --perf-time <sec>         perf记录时长 (默认: $PERF_RECORD_TIME)
  --no-perf                 禁用perf分析
  --no-flamegraph           禁用火焰图生成
  --no-monitor              禁用系统监控

其他选项:
  -h, --help                显示此帮助信息
  --clean                   清理编译产物并退出

示例:
  $0                                    # 使用默认参数
  $0 -t 16 -c 50 -d 120                # 高并发长时间测试
  $0 --no-perf --no-flamegraph         # 纯性能测试，不生成分析报告
  $0 --clean                           # 清理编译产物

报告输出目录: $RESULTS_DIR
EOF
}

# 解析命令行参数
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -H|--host)
                SERVER_IP="$2"
                shift 2
                ;;
            -P|--port)
                SERVER_PORT="$2"
                shift 2
                ;;
            -T|--server-threads)
                SERVER_THREADS="$2"
                shift 2
                ;;
            -t|--threads)
                CLIENT_THREADS="$2"
                shift 2
                ;;
            -c|--connections)
                CONNECTIONS_PER_THREAD="$2"
                shift 2
                ;;
            -r|--requests)
                REQUESTS_PER_CONNECTION="$2"
                shift 2
                ;;
            -s|--size)
                MESSAGE_SIZE="$2"
                shift 2
                ;;
            -d|--duration)
                TEST_DURATION="$2"
                shift 2
                ;;
            --perf-time)
                PERF_RECORD_TIME="$2"
                shift 2
                ;;
            --no-perf)
                ENABLE_PERF="false"
                shift
                ;;
            --no-flamegraph)
                ENABLE_FLAMEGRAPH="false"
                shift
                ;;
            --no-monitor)
                ENABLE_SYSTEM_MONITOR="false"
                shift
                ;;
            --clean)
                clean_build
                exit 0
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            *)
                log_error "未知选项: $1"
                usage
                exit 1
                ;;
        esac
    done
}

# 清理编译产物
clean_build() {
    log_step "清理编译产物..."
    rm -rf "$BUILD_DIR"
    rm -f "$TOOLS_DIR/stress_test_echo"
    rm -f "$TOOLS_DIR/benchmark_client"
    log_info "清理完成"
}

# 检查依赖
check_dependencies() {
    log_step "检查依赖环境..."
    
    # 检查必要的工具
    local missing_tools=()
    
    if ! command -v cmake &> /dev/null; then
        missing_tools+=("cmake")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_tools+=("make")
    fi
    
    if ! command -v g++ &> /dev/null; then
        missing_tools+=("g++")
    fi
    
    if [[ "$ENABLE_PERF" == "true" ]] && ! command -v perf &> /dev/null; then
        log_warn "perf工具未找到，禁用性能分析"
        ENABLE_PERF="false"
    fi
    
    if [[ "$ENABLE_FLAMEGRAPH" == "true" ]] && ! command -v flamegraph.pl &> /dev/null; then
        log_warn "flamegraph工具未找到，尝试下载..."
        setup_flamegraph
    fi
    
    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        log_error "缺少必要工具: ${missing_tools[*]}"
        log_info "请安装缺少的工具后重试"
        exit 1
    fi
    
    log_info "依赖检查完成"
}

# 设置火焰图工具
setup_flamegraph() {
    local flamegraph_dir="$TOOLS_DIR/FlameGraph"
    
    if [[ ! -d "$flamegraph_dir" ]]; then
        log_info "下载FlameGraph工具..."
        git clone https://github.com/brendangregg/FlameGraph.git "$flamegraph_dir" || {
            log_error "下载FlameGraph失败，禁用火焰图生成"
            ENABLE_FLAMEGRAPH="false"
            return
        }
    fi
    
    # 添加到PATH
    export PATH="$flamegraph_dir:$PATH"
    log_info "FlameGraph工具准备完成"
}

# 编译项目
build_project() {
    log_step "编译NetBox项目..."
    
    cd "$PROJECT_ROOT"
    
    # 创建构建目录
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # CMake配置（Release模式获得最佳性能）
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" || {
        log_error "CMake配置失败"
        exit 1
    }
    
    # 编译
    make -j$(nproc) || {
        log_error "编译失败"
        exit 1
    }
    
    log_info "项目编译完成"
}

# 编译测试工具
build_test_tools() {
    log_step "编译压力测试工具..."
    
    cd "$TOOLS_DIR"
    
    # 编译服务器测试程序
    g++ -std=c++17 -O3 -march=native -I"$PROJECT_ROOT/app/include" \
        -I"$PROJECT_ROOT/NetFramework/include" \
        -I"$PROJECT_ROOT/Protocol/include" \
        stress_test_echo.cpp \
        -L"$BUILD_DIR" -lNetFramework -lProtocol -lpthread \
        -o stress_test_echo || {
        log_error "编译stress_test_echo失败"
        exit 1
    }
    
    # 编译客户端测试程序
    g++ -std=c++17 -O3 -march=native -I"$PROJECT_ROOT/Protocol/include" \
        benchmark_client.cpp \
        -L"$BUILD_DIR" -lProtocol -lpthread \
        -o benchmark_client || {
        log_error "编译benchmark_client失败"
        exit 1
    }
    
    log_info "测试工具编译完成"
}

# 准备结果目录
prepare_results_dir() {
    log_step "准备结果目录..."
    
    mkdir -p "$RESULTS_DIR"
    
    # 保存测试配置
    cat > "$RESULTS_DIR/test_config.txt" << EOF
NetBox Echo服务器压力测试配置
测试时间: $(date)
====================================

服务器配置:
  IP地址: $SERVER_IP
  端口: $SERVER_PORT
  线程数: $SERVER_THREADS

客户端配置:
  线程数: $CLIENT_THREADS
  每线程连接数: $CONNECTIONS_PER_THREAD
  每连接请求数: $REQUESTS_PER_CONNECTION
  消息大小: $MESSAGE_SIZE 字节
  测试时长: $TEST_DURATION 秒

性能分析:
  perf分析: $ENABLE_PERF
  perf记录时长: $PERF_RECORD_TIME 秒
  火焰图生成: $ENABLE_FLAMEGRAPH
  系统监控: $ENABLE_SYSTEM_MONITOR

系统信息:
  操作系统: $(uname -a)
  CPU信息: $(grep "model name" /proc/cpuinfo | head -1 | cut -d: -f2 | sed 's/^ *//')
  CPU核心数: $(nproc)
  内存信息: $(free -h | grep "Mem:" | awk '{print $2}')
EOF
    
    log_info "结果目录准备完成: $RESULTS_DIR"
}

# 启动服务器
start_server() {
    log_step "启动Echo服务器..."
    
    cd "$TOOLS_DIR"
    
    # 启动服务器（后台运行）
    ./stress_test_echo "$SERVER_IP" "$SERVER_PORT" "$SERVER_THREADS" > "$RESULTS_DIR/server.log" 2>&1 &
    SERVER_PID=$!
    
    # 等待服务器启动
    sleep 2
    
    # 检查服务器是否正常运行
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        log_error "服务器启动失败"
        cat "$RESULTS_DIR/server.log"
        exit 1
    fi
    
    log_info "服务器启动成功 (PID: $SERVER_PID)"
}

# 系统监控
start_system_monitor() {
    if [[ "$ENABLE_SYSTEM_MONITOR" != "true" ]]; then
        return
    fi
    
    log_step "启动系统监控..."
    
    # CPU和内存监控
    {
        echo "时间,CPU使用率(%),内存使用率(%),网络接收(KB/s),网络发送(KB/s)"
        while kill -0 $SERVER_PID 2>/dev/null; do
            local timestamp=$(date '+%H:%M:%S')
            local cpu_usage=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | sed 's/%us,//')
            local mem_usage=$(free | grep Mem | awk '{printf "%.1f", $3/$2 * 100.0}')
            local net_stats=$(cat /proc/net/dev | grep -E "(eth0|ens|enp)" | head -1 | awk '{print $2,$10}')
            local rx_bytes=$(echo $net_stats | awk '{print $1}')
            local tx_bytes=$(echo $net_stats | awk '{print $2}')
            
            echo "$timestamp,$cpu_usage,$mem_usage,$rx_bytes,$tx_bytes"
            sleep 1
        done
    } > "$RESULTS_DIR/system_monitor.csv" &
    MONITOR_PID=$!
    
    log_info "系统监控启动完成 (PID: $MONITOR_PID)"
}

# perf性能分析
start_perf_analysis() {
    if [[ "$ENABLE_PERF" != "true" ]]; then
        return
    fi
    
    log_perf "启动perf性能分析..."
    
    # 记录perf数据
    perf record -F 99 -p $SERVER_PID -g -o "$RESULTS_DIR/perf.data" &
    PERF_PID=$!
    
    log_perf "perf记录启动 (PID: $PERF_PID)，将记录 $PERF_RECORD_TIME 秒"
    
    # 设置定时器停止perf
    (
        sleep $PERF_RECORD_TIME
        if kill -0 $PERF_PID 2>/dev/null; then
            kill -INT $PERF_PID
            log_perf "perf记录已停止"
        fi
    ) &
}

# 运行压力测试
run_stress_test() {
    log_step "运行压力测试..."
    
    cd "$TOOLS_DIR"
    
    # 等待服务器完全启动
    sleep 1
    
    # 运行客户端压力测试
    ./benchmark_client \
        -h "$SERVER_IP" \
        -p "$SERVER_PORT" \
        -t "$CLIENT_THREADS" \
        -c "$CONNECTIONS_PER_THREAD" \
        -r "$REQUESTS_PER_CONNECTION" \
        -s "$MESSAGE_SIZE" \
        -d "$TEST_DURATION" \
        > "$RESULTS_DIR/benchmark_result.txt" 2>&1
    
    log_info "压力测试完成"
}

# 生成perf报告
generate_perf_report() {
    if [[ "$ENABLE_PERF" != "true" ]]; then
        return
    fi
    
    log_perf "生成perf分析报告..."
    
    cd "$RESULTS_DIR"
    
    # 等待perf记录完成
    if [[ -n "${PERF_PID:-}" ]]; then
        wait $PERF_PID 2>/dev/null || true
    fi
    
    # 生成perf报告
    if [[ -f "perf.data" ]]; then
        perf report -i perf.data --stdio > perf_report.txt
        perf script -i perf.data > perf_script.txt
        
        log_perf "perf报告生成完成"
    else
        log_warn "perf.data文件未找到"
    fi
}

# 生成火焰图
generate_flamegraph() {
    if [[ "$ENABLE_FLAMEGRAPH" != "true" || "$ENABLE_PERF" != "true" ]]; then
        return
    fi
    
    log_perf "生成火焰图..."
    
    cd "$RESULTS_DIR"
    
    if [[ -f "perf_script.txt" ]]; then
        # 生成火焰图
        stackcollapse-perf.pl perf_script.txt > perf_folded.txt
        flamegraph.pl perf_folded.txt > flamegraph.svg
        
        log_perf "火焰图生成完成: $RESULTS_DIR/flamegraph.svg"
    else
        log_warn "perf_script.txt文件未找到，无法生成火焰图"
    fi
}

# 停止服务器和监控
cleanup() {
    log_step "清理测试环境..."
    
    # 停止服务器
    if [[ -n "${SERVER_PID:-}" ]] && kill -0 $SERVER_PID 2>/dev/null; then
        kill $SERVER_PID
        wait $SERVER_PID 2>/dev/null || true
        log_info "服务器已停止"
    fi
    
    # 停止系统监控
    if [[ -n "${MONITOR_PID:-}" ]] && kill -0 $MONITOR_PID 2>/dev/null; then
        kill $MONITOR_PID
        wait $MONITOR_PID 2>/dev/null || true
        log_info "系统监控已停止"
    fi
    
    # 停止perf
    if [[ -n "${PERF_PID:-}" ]] && kill -0 $PERF_PID 2>/dev/null; then
        kill -INT $PERF_PID
        wait $PERF_PID 2>/dev/null || true
        log_info "perf记录已停止"
    fi
}

# 生成测试报告
generate_report() {
    log_step "生成综合测试报告..."
    
    local report_file="$RESULTS_DIR/test_report.md"
    
    cat > "$report_file" << EOF
# NetBox Echo服务器压力测试报告

生成时间: $(date)

## 测试配置

$(cat "$RESULTS_DIR/test_config.txt")

## 性能测试结果

\`\`\`
$(cat "$RESULTS_DIR/benchmark_result.txt")
\`\`\`

## 服务器日志

\`\`\`
$(tail -50 "$RESULTS_DIR/server.log")
\`\`\`

EOF
    
    # 添加perf分析结果
    if [[ -f "$RESULTS_DIR/perf_report.txt" ]]; then
        cat >> "$report_file" << EOF
## 性能分析概要

### Top 20 热点函数

\`\`\`
$(head -50 "$RESULTS_DIR/perf_report.txt" | grep -A 30 "# Overhead")
\`\`\`

EOF
    fi
    
    # 添加系统监控结果
    if [[ -f "$RESULTS_DIR/system_monitor.csv" ]]; then
        cat >> "$report_file" << EOF
## 系统资源使用

系统监控数据已保存到: system_monitor.csv

EOF
    fi
    
    cat >> "$report_file" << EOF
## 文件说明

- \`test_config.txt\`: 测试配置信息
- \`benchmark_result.txt\`: 压力测试详细结果
- \`server.log\`: 服务器运行日志
- \`perf.data\`: perf原始数据（需要perf工具查看）
- \`perf_report.txt\`: perf分析报告
- \`perf_script.txt\`: perf脚本数据
- \`flamegraph.svg\`: 火焰图（在浏览器中打开查看）
- \`system_monitor.csv\`: 系统资源监控数据

## 查看建议

1. 查看性能结果：\`cat benchmark_result.txt\`
2. 查看火焰图：在浏览器中打开 \`flamegraph.svg\`
3. 查看perf报告：\`cat perf_report.txt\`
4. 分析系统监控：使用Excel或其他工具打开 \`system_monitor.csv\`

EOF
    
    log_info "测试报告生成完成: $report_file"
}

# 显示结果摘要
show_summary() {
    log_step "测试结果摘要"
    
    echo -e "${CYAN}=====================================================${NC}"
    echo -e "${CYAN}           NetBox Echo服务器压力测试完成${NC}"
    echo -e "${CYAN}=====================================================${NC}"
    echo
    
    if [[ -f "$RESULTS_DIR/benchmark_result.txt" ]]; then
        echo -e "${GREEN}性能测试结果:${NC}"
        grep -E "(QPS|延迟|成功率|吞吐量)" "$RESULTS_DIR/benchmark_result.txt" || true
        echo
    fi
    
    echo -e "${GREEN}生成的文件:${NC}"
    ls -la "$RESULTS_DIR/" | grep -v "^total" | awk '{print "  " $9 " (" $5 " bytes)"}'
    echo
    
    echo -e "${GREEN}报告位置:${NC} $RESULTS_DIR"
    echo -e "${GREEN}主报告文件:${NC} $RESULTS_DIR/test_report.md"
    
    if [[ -f "$RESULTS_DIR/flamegraph.svg" ]]; then
        echo -e "${GREEN}火焰图:${NC} file://$RESULTS_DIR/flamegraph.svg"
    fi
    
    echo -e "${CYAN}=====================================================${NC}"
}

# 信号处理
trap cleanup EXIT INT TERM

# 主函数
main() {
    echo -e "${CYAN}"
    cat << "EOF"
 _   _      _   ____             
| \ | | ___| |_| __ )  _____  __ 
|  \| |/ _ \ __|  _ \ / _ \ \/ / 
| |\  |  __/ |_| |_) | (_) >  <  
|_| \_|\___|\__|____/ \___/_/\_\ 
                                 
    Echo服务器压力测试工具
EOF
    echo -e "${NC}"
    
    # 解析参数
    parse_args "$@"
    
    # 检查依赖
    check_dependencies
    
    # 编译项目
    build_project
    
    # 编译测试工具
    build_test_tools
    
    # 准备结果目录
    prepare_results_dir
    
    # 启动服务器
    start_server
    
    # 启动监控
    start_system_monitor
    
    # 启动perf分析
    start_perf_analysis
    
    # 运行压力测试
    run_stress_test
    
    # 生成分析报告
    generate_perf_report
    generate_flamegraph
    
    # 生成综合报告
    generate_report
    
    # 显示结果
    show_summary
}

# 运行主函数
main "$@" 