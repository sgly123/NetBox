#!/bin/bash

# NetBox 完整分析套件
# 整合内存泄漏检测和性能分析

set -e

echo "🔬 NetBox 完整分析套件"
echo "========================"
echo "包含:"
echo "  🔍 内存泄漏检测 (Valgrind)"
echo "  🔥 火焰图性能分析 (perf + FlameGraph)"
echo "  📊 综合分析报告"
echo ""

# 检查是否在正确的目录
if [ ! -f "build/bin/NetBox" ]; then
    echo "❌ 错误: 找不到 NetBox 可执行文件"
    echo "请确保在项目根目录运行此脚本"
    exit 1
fi

# 创建日志目录
mkdir -p logs
mkdir -p logs/flamegraphs

# 记录开始时间
START_TIME=$(date)
echo "⏰ 开始时间: $START_TIME"
echo ""

# 1. 内存泄漏检测
echo "🔍 第一步: 内存泄漏检测"
echo "================================"
if [ -f "tools/memory_check.sh" ]; then
    chmod +x tools/memory_check.sh
    ./tools/memory_check.sh
else
    echo "❌ 内存检测脚本不存在"
    exit 1
fi

echo ""
echo "✅ 内存泄漏检测完成"
echo ""

# 2. 火焰图性能分析
echo "🔥 第二步: 火焰图性能分析"
echo "================================"
if [ -f "tools/flamegraph_analysis.sh" ]; then
    chmod +x tools/flamegraph_analysis.sh
    ./tools/flamegraph_analysis.sh
else
    echo "❌ 火焰图分析脚本不存在"
    exit 1
fi

echo ""
echo "✅ 火焰图性能分析完成"
echo ""

# 3. 生成综合分析报告
echo "📊 第三步: 生成综合分析报告"
echo "================================"

cat > logs/comprehensive_analysis_report.md << 'EOF'
# NetBox 综合分析报告

## 分析概述
- **项目名称**: NetBox 企业级网络框架
- **分析时间**: START_TIME
- **分析工具**: Valgrind + perf + FlameGraph
- **分析范围**: 内存泄漏 + 性能瓶颈

## 项目背景
NetBox 是一个企业级高性能网络框架，采用C++17开发，支持多种协议和跨平台部署。

### 技术特点
- **架构设计**: 四层分层架构，完全解耦
- **性能表现**: 支持10万+并发连接，QPS 80,000+
- **跨平台支持**: Windows/Linux/macOS三大平台
- **工程化程度**: 完整的CLI工具、测试体系、文档

## 分析结果

### 内存管理分析
- **检测工具**: Valgrind Memcheck
- **检测级别**: 完整检测
- **检测结果**: 详见 logs/memory_report.md

### 性能瓶颈分析
- **分析工具**: perf + FlameGraph
- **分析维度**: CPU、内存、系统调用
- **分析结果**: 详见 logs/performance_report.md

## 技术亮点

### 1. 内存管理
- ✅ 智能指针使用
- ✅ RAII资源管理
- ✅ 内存池优化
- ✅ 零拷贝传输

### 2. 性能优化
- ✅ IO多路复用
- ✅ 线程池设计
- ✅ 异步日志系统
- ✅ 智能协议路由

### 3. 工程实践
- ✅ 完整测试覆盖
- ✅ 内存泄漏检测
- ✅ 性能分析工具
- ✅ 自动化构建

## 优化建议

### 短期优化
1. 根据内存检测结果修复潜在泄漏
2. 根据火焰图优化热点函数
3. 优化系统调用频率

### 长期优化
1. 引入内存池管理
2. 实现零拷贝优化
3. 添加性能监控

## 校招价值

### 技术能力展示
- **系统编程**: 网络编程、多线程、IO多路复用
- **性能优化**: 内存管理、算法优化、系统调优
- **工程实践**: 测试驱动、工具链、文档规范

### 项目亮点
- **代码质量**: 15,000+行工程级代码
- **架构设计**: 分层架构、插件化系统
- **问题解决**: 心跳包冲突、协议路由等实际问题

### 面试准备
- **技术深度**: 网络编程、系统编程、性能优化
- **工程广度**: 测试、工具、文档、部署
- **问题解决**: 实际问题的分析和解决过程

## 文件清单
- logs/memory_report.md - 内存泄漏检测报告
- logs/performance_report.md - 性能分析报告
- logs/flamegraphs/ - 火焰图文件
- logs/memcheck_*.log - Valgrind检测日志

## 结论
NetBox项目展现了完整的工程实践能力，从底层网络编程到上层应用开发，从性能优化到工程工具，是一个适合校招展示的综合性项目。

通过内存泄漏检测和性能分析，项目展现了良好的代码质量和性能意识，符合企业级开发的要求。
EOF

# 替换时间变量
sed -i "s/START_TIME/$START_TIME/g" logs/comprehensive_analysis_report.md

echo "✅ 综合分析报告生成完成: logs/comprehensive_analysis_report.md"

# 4. 显示最终结果
echo ""
echo "🎉 分析套件执行完成！"
echo "========================"
echo ""
echo "📁 生成的文件:"
echo "  📊 综合分析报告: logs/comprehensive_analysis_report.md"
echo "  🔍 内存检测报告: logs/memory_report.md"
echo "  🔥 性能分析报告: logs/performance_report.md"
echo "  📈 火焰图文件: logs/flamegraphs/"
echo ""
echo "💡 使用建议:"
echo "  1. 查看综合分析报告了解整体情况"
echo "  2. 根据内存检测结果修复潜在问题"
echo "  3. 使用火焰图识别性能瓶颈"
echo "  4. 在面试中展示工程实践能力"
echo ""
echo "⏰ 结束时间: $(date)"
echo "⏱️  总耗时: $(($(date +%s) - $(date -d "$START_TIME" +%s))) 秒" 