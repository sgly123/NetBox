#!/bin/bash

# NetBox 性能基准测试脚本
# 在虚拟机环境中运行框架性能测试

echo "======================================="
echo "    NetBox 性能基准测试"
echo "======================================="
echo "测试环境: 虚拟机环境"
echo "CPU核心数: $(nproc)"
echo "内存信息: $(free -h | grep Mem | awk '{print $2}')"
echo "======================================="
echo

# 检查测试可执行文件是否存在
if [ ! -f "../build/tests/bin/test_performance" ]; then
    echo "❌ 测试可执行文件不存在，请先编译项目"
    echo "运行: cd build && make test_performance"
    exit 1
fi

cd ../build

echo "🧵 运行线程池性能测试..."
./tests/bin/test_performance --gtest_filter="*ThreadPoolBenchmark*" 2>/dev/null
echo

echo "🔌 运行IO多路复用性能测试..."
./tests/bin/test_performance --gtest_filter="*IOMultiplexerBenchmark*" 2>/dev/null
echo

echo "📝 运行异步日志性能测试..."
./tests/bin/test_performance --gtest_filter="*AsyncLoggerBenchmark*" 2>/dev/null
echo

echo "🚀 运行综合性能测试..."
./tests/bin/test_performance --gtest_filter="*ComprehensiveBenchmark*" 2>/dev/null
echo

echo "======================================="
echo "    性能测试完成"
echo "======================================="
echo "📊 详细的性能数据请查看:"
echo "   docs/性能优化/性能基准测试报告.md"
echo
echo "🐛 Bug修复详情请查看:"
echo "   docs/重要更新/EpollMultiplexer_Bug修复报告.md"
echo "======================================="
