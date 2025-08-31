#!/bin/bash

# NetBox 火焰图性能分析工具
# 使用 perf + FlameGraph 进行性能分析

set -e

echo "🔥 NetBox 火焰图性能分析工具"
echo "================================"

# 检查必要工具
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo "❌ $1 未安装，正在安装..."
        case $1 in
            "perf")
                sudo apt update
                sudo apt install -y linux-tools-common linux-tools-generic
                ;;
            "git")
                sudo apt update
                sudo apt install -y git
                ;;
            "flamegraph.pl")
                # 检查FlameGraph是否已安装
                if [ ! -f "tools/FlameGraph/flamegraph.pl" ]; then
                    echo "📥 下载 FlameGraph..."
                    cd tools
                    git clone https://github.com/brendangregg/FlameGraph.git
                    cd ..
                fi
                ;;
        esac
    fi
}

# 检查工具
check_tool "perf"
check_tool "git"
check_tool "flamegraph.pl"

# 检查是否在正确的目录
if [ ! -f "build/bin/NetBox" ]; then
    echo "❌ 错误: 找不到 NetBox 可执行文件"
    echo "请确保在项目根目录运行此脚本"
    exit 1
fi

# 创建日志目录
mkdir -p logs
mkdir -p logs/flamegraphs

echo "📋 开始性能分析..."

# 1. CPU 火焰图分析
echo "1️⃣ CPU 火焰图分析..."

# 启动服务器
echo "   启动 NetBox 服务器..."
./build/bin/NetBox config/config_redis_app.yaml &
SERVER_PID=$!

# 等待服务器启动
sleep 3

# 运行性能测试
echo "   运行性能测试..."
for i in {1..50}; do
    echo "ping" | nc -w 1 192.168.88.135 6379 > /dev/null 2>&1 || true
    echo "set key$i value$i" | nc -w 1 192.168.88.135 6379 > /dev/null 2>&1 || true
    echo "get key$i" | nc -w 1 192.168.88.135 6379 > /dev/null 2>&1 || true
done

# 收集性能数据
echo "   收集性能数据..."
sudo perf record -F 99 -p $SERVER_PID -g -- sleep 10

# 生成火焰图
echo "   生成火焰图..."
sudo perf script | tools/FlameGraph/stackcollapse-perf.pl > logs/perf-folded.txt
tools/FlameGraph/flamegraph.pl logs/perf-folded.txt > logs/flamegraphs/cpu-flamegraph.svg

# 停止服务器
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo "✅ CPU 火焰图生成完成: logs/flamegraphs/cpu-flamegraph.svg"

# 2. 内存火焰图分析
echo "2️⃣ 内存火焰图分析..."

# 启动服务器
echo "   启动 NetBox 服务器..."
./build/bin/NetBox config/config_redis_app.yaml &
SERVER_PID=$!

# 等待服务器启动
sleep 3

# 收集内存分配数据
echo "   收集内存分配数据..."
sudo perf record -e malloc -p $SERVER_PID -g -- sleep 10

# 生成内存火焰图
echo "   生成内存火焰图..."
sudo perf script | tools/FlameGraph/stackcollapse-perf.pl > logs/malloc-folded.txt
tools/FlameGraph/flamegraph.pl --title="Memory Allocation Flame Graph" logs/malloc-folded.txt > logs/flamegraphs/memory-flamegraph.svg

# 停止服务器
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo "✅ 内存火焰图生成完成: logs/flamegraphs/memory-flamegraph.svg"

# 3. 系统调用火焰图分析
echo "3️⃣ 系统调用火焰图分析..."

# 启动服务器
echo "   启动 NetBox 服务器..."
./build/bin/NetBox config/config_redis_app.yaml &
SERVER_PID=$!

# 等待服务器启动
sleep 3

# 收集系统调用数据
echo "   收集系统调用数据..."
sudo perf record -e syscalls:sys_enter_* -p $SERVER_PID -g -- sleep 10

# 生成系统调用火焰图
echo "   生成系统调用火焰图..."
sudo perf script | tools/FlameGraph/stackcollapse-perf.pl > logs/syscall-folded.txt
tools/FlameGraph/flamegraph.pl --title="System Call Flame Graph" logs/syscall-folded.txt > logs/flamegraphs/syscall-flamegraph.svg

# 停止服务器
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo "✅ 系统调用火焰图生成完成: logs/flamegraphs/syscall-flamegraph.svg"

# 4. 生成性能分析报告
echo "4️⃣ 生成性能分析报告..."
cat > logs/performance_report.md << 'EOF'
# NetBox 性能分析报告

## 分析时间
$(date)

## 分析工具
- perf (Linux Performance Counters)
- FlameGraph
- 版本: $(perf --version | head -1)

## 分析项目
1. CPU 使用率火焰图
2. 内存分配火焰图
3. 系统调用火焰图

## 火焰图说明

### CPU 火焰图
- **文件**: logs/flamegraphs/cpu-flamegraph.svg
- **说明**: 显示 CPU 使用率的调用栈分布
- **用途**: 识别 CPU 热点和性能瓶颈

### 内存火焰图
- **文件**: logs/flamegraphs/memory-flamegraph.svg
- **说明**: 显示内存分配的调用栈分布
- **用途**: 识别内存分配热点和内存泄漏

### 系统调用火焰图
- **文件**: logs/flamegraphs/syscall-flamegraph.svg
- **说明**: 显示系统调用的调用栈分布
- **用途**: 识别 I/O 瓶颈和系统调用开销

## 性能优化建议

### CPU 优化
1. 识别 CPU 热点函数
2. 优化算法复杂度
3. 减少不必要的计算

### 内存优化
1. 减少内存分配次数
2. 使用内存池
3. 优化数据结构

### I/O 优化
1. 减少系统调用次数
2. 使用批量操作
3. 优化网络 I/O

## 分析配置
- 采样频率: 99Hz
- 采样时间: 10秒
- 分析进程: NetBox 服务器
- 测试场景: Redis 命令处理

## 查看火焰图
1. 在浏览器中打开 SVG 文件
2. 鼠标悬停查看详细信息
3. 点击放大特定区域
4. 搜索特定函数名

## 性能基准
- 测试命令: PING, SET, GET
- 测试次数: 50次
- 并发连接: 单连接
- 网络延迟: 本地测试
EOF

echo "✅ 性能分析报告生成完成: logs/performance_report.md"

# 5. 显示分析结果摘要
echo ""
echo "📊 分析结果摘要:"
echo "=================="
echo "CPU 火焰图: logs/flamegraphs/cpu-flamegraph.svg"
echo "内存火焰图: logs/flamegraphs/memory-flamegraph.svg"
echo "系统调用火焰图: logs/flamegraphs/syscall-flamegraph.svg"
echo "性能报告: logs/performance_report.md"

echo ""
echo "📁 详细文件:"
echo "  - logs/perf-folded.txt (CPU 数据)"
echo "  - logs/malloc-folded.txt (内存数据)"
echo "  - logs/syscall-folded.txt (系统调用数据)"

echo ""
echo "🎉 火焰图性能分析完成！"
echo ""
echo "💡 提示: 在浏览器中打开 SVG 文件查看交互式火焰图" 