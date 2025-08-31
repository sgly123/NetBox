#!/bin/bash

# NetBox 内存泄漏检测工具
# 使用 Valgrind 进行内存分析

set -e

echo "🔍 NetBox 内存泄漏检测工具"
echo "================================"

# 检查 Valgrind 是否安装
if ! command -v valgrind &> /dev/null; then
    echo "❌ Valgrind 未安装，正在安装..."
    sudo apt update
    sudo apt install -y valgrind
fi

# 检查是否在正确的目录
if [ ! -f "build/bin/NetBox" ]; then
    echo "❌ 错误: 找不到 NetBox 可执行文件"
    echo "请确保在项目根目录运行此脚本"
    exit 1
fi

# 创建日志目录
mkdir -p logs

echo "📋 开始内存泄漏检测..."

# 1. 基础内存泄漏检测
echo "1️⃣ 基础内存泄漏检测..."
valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file=logs/memcheck_basic.log \
    --suppressions=valgrind.supp \
    ./build/bin/NetBox config/config_redis_app.yaml

echo "✅ 基础检测完成，日志保存到 logs/memcheck_basic.log"

# 2. 压力测试内存泄漏检测
echo "2️⃣ 压力测试内存泄漏检测..."
valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --log-file=logs/memcheck_stress.log \
    --suppressions=valgrind.supp \
    timeout 30s ./build/bin/NetBox config/config_redis_app.yaml &

# 等待服务器启动
sleep 2

# 运行压力测试
echo "   运行压力测试..."
for i in {1..100}; do
    echo "ping" | nc -w 1 192.168.88.135 6379 > /dev/null 2>&1 || true
    echo "set key$i value$i" | nc -w 1 192.168.88.135 6379 > /dev/null 2>&1 || true
    echo "get key$i" | nc -w 1 192.168.88.135 6379 > /dev/null 2>&1 || true
done

# 等待进程结束
wait

echo "✅ 压力测试检测完成，日志保存到 logs/memcheck_stress.log"

# 3. 生成内存分析报告
echo "3️⃣ 生成内存分析报告..."
cat > logs/memory_report.md << 'EOF'
# NetBox 内存泄漏检测报告

## 检测时间
$(date)

## 检测工具
- Valgrind Memcheck
- 版本: $(valgrind --version)

## 检测项目
1. 基础内存泄漏检测
2. 压力测试内存泄漏检测

## 检测结果

### 基础检测
EOF

# 分析基础检测结果
if [ -f "logs/memcheck_basic.log" ]; then
    echo "### 基础检测结果" >> logs/memory_report.md
    echo '```' >> logs/memory_report.md
    grep -A 20 "LEAK SUMMARY" logs/memcheck_basic.log >> logs/memory_report.md 2>/dev/null || echo "无内存泄漏" >> logs/memory_report.md
    echo '```' >> logs/memory_report.md
fi

cat >> logs/memory_report.md << 'EOF'

### 压力测试检测
EOF

# 分析压力测试结果
if [ -f "logs/memcheck_stress.log" ]; then
    echo "### 压力测试结果" >> logs/memory_report.md
    echo '```' >> logs/memory_report.md
    grep -A 20 "LEAK SUMMARY" logs/memcheck_stress.log >> logs/memory_report.md 2>/dev/null || echo "无内存泄漏" >> logs/memory_report.md
    echo '```' >> logs/memory_report.md
fi

cat >> logs/memory_report.md << 'EOF'

## 内存使用统计

### 基础检测内存使用
EOF

# 提取内存使用统计
if [ -f "logs/memcheck_basic.log" ]; then
    echo '```' >> logs/memory_report.md
    grep -E "(total heap usage|allocated|freed)" logs/memcheck_basic.log | tail -5 >> logs/memory_report.md 2>/dev/null || echo "无内存使用统计" >> logs/memory_report.md
    echo '```' >> logs/memory_report.md
fi

cat >> logs/memory_report.md << 'EOF'

### 压力测试内存使用
EOF

if [ -f "logs/memcheck_stress.log" ]; then
    echo '```' >> logs/memory_report.md
    grep -E "(total heap usage|allocated|freed)" logs/memcheck_stress.log | tail -5 >> logs/memory_report.md 2>/dev/null || echo "无内存使用统计" >> logs/memory_report.md
    echo '```' >> logs/memory_report.md
fi

cat >> logs/memory_report.md << 'EOF'

## 建议和优化

### 发现的问题
1. 内存泄漏情况
2. 内存使用效率
3. 潜在的内存错误

### 优化建议
1. 检查智能指针使用
2. 确保资源正确释放
3. 优化内存分配策略

## 检测配置
- 工具: Valgrind Memcheck
- 检测级别: 完整检测
- 泄漏类型: 所有类型
- 来源跟踪: 启用
EOF

echo "✅ 内存分析报告生成完成: logs/memory_report.md"

# 4. 显示检测结果摘要
echo ""
echo "📊 检测结果摘要:"
echo "=================="

if [ -f "logs/memcheck_basic.log" ]; then
    echo "基础检测:"
    grep -A 5 "LEAK SUMMARY" logs/memcheck_basic.log 2>/dev/null || echo "  ✅ 无内存泄漏"
fi

if [ -f "logs/memcheck_stress.log" ]; then
    echo "压力测试:"
    grep -A 5 "LEAK SUMMARY" logs/memcheck_stress.log 2>/dev/null || echo "  ✅ 无内存泄漏"
fi

echo ""
echo "📁 详细日志文件:"
echo "  - logs/memcheck_basic.log (基础检测)"
echo "  - logs/memcheck_stress.log (压力测试)"
echo "  - logs/memory_report.md (分析报告)"

echo ""
echo "🎉 内存泄漏检测完成！" 