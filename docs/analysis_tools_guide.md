# NetBox 分析工具使用指南

## 🎯 概述

NetBox 项目提供了完整的内存泄漏检测和性能分析工具套件，帮助开发者进行代码质量检查和性能优化。

## 🔧 工具清单

### 1. 内存泄漏检测 (Valgrind)
- **脚本**: `tools/memory_check.sh`
- **工具**: Valgrind Memcheck
- **功能**: 检测内存泄漏、越界访问、未初始化内存使用

### 2. 火焰图性能分析 (perf + FlameGraph)
- **脚本**: `tools/flamegraph_analysis.sh`
- **工具**: perf + FlameGraph
- **功能**: 生成CPU、内存、系统调用火焰图

### 3. 一键分析套件
- **脚本**: `tools/analysis_suite.sh`
- **功能**: 整合所有分析工具，生成综合报告

## 🚀 快速开始

### 一键运行完整分析
```bash
# 在项目根目录运行
chmod +x tools/analysis_suite.sh
./tools/analysis_suite.sh
```

### 单独运行内存检测
```bash
chmod +x tools/memory_check.sh
./tools/memory_check.sh
```

### 单独运行性能分析
```bash
chmod +x tools/flamegraph_analysis.sh
./tools/flamegraph_analysis.sh
```

## 📊 输出文件

### 内存检测输出
- `logs/memcheck_basic.log` - 基础内存检测日志
- `logs/memcheck_stress.log` - 压力测试内存检测日志
- `logs/memory_report.md` - 内存检测分析报告

### 性能分析输出
- `logs/flamegraphs/cpu-flamegraph.svg` - CPU使用率火焰图
- `logs/flamegraphs/memory-flamegraph.svg` - 内存分配火焰图
- `logs/flamegraphs/syscall-flamegraph.svg` - 系统调用火焰图
- `logs/performance_report.md` - 性能分析报告

### 综合分析输出
- `logs/comprehensive_analysis_report.md` - 综合分析报告

## 🔍 查看火焰图

1. **在浏览器中打开SVG文件**
   ```bash
   # Linux
   xdg-open logs/flamegraphs/cpu-flamegraph.svg
   
   # 或者直接拖拽到浏览器
   ```

2. **火焰图交互操作**
   - **鼠标悬停**: 查看函数详细信息
   - **点击**: 放大特定区域
   - **搜索**: 按F键搜索特定函数
   - **缩放**: 鼠标滚轮缩放

## 📈 分析结果解读

### 内存泄漏检测
- **无泄漏**: 显示 "无内存泄漏"
- **有泄漏**: 显示泄漏类型和位置
- **建议**: 根据报告修复内存管理问题

### 性能火焰图
- **CPU火焰图**: 识别CPU热点函数
- **内存火焰图**: 识别内存分配热点
- **系统调用火焰图**: 识别I/O瓶颈

## 🛠️ 工具配置

### Valgrind配置
- **检测级别**: 完整检测 (`--leak-check=full`)
- **泄漏类型**: 所有类型 (`--show-leak-kinds=all`)
- **来源跟踪**: 启用 (`--track-origins=yes`)
- **抑制文件**: `valgrind.supp`

### perf配置
- **采样频率**: 99Hz (`-F 99`)
- **采样时间**: 10秒
- **调用栈**: 启用 (`-g`)

## 🎯 校招应用

### 面试准备
1. **展示工程实践能力**
   - 内存泄漏检测意识
   - 性能分析工具使用
   - 代码质量保证

2. **技术深度展示**
   - 系统编程知识
   - 性能优化经验
   - 问题解决能力

3. **项目亮点**
   - 完整的工程工具链
   - 专业的分析报告
   - 实际的问题解决

### 使用建议
1. **在简历中提及**
   - "使用Valgrind进行内存泄漏检测"
   - "使用perf+FlameGraph进行性能分析"
   - "建立了完整的代码质量保证体系"

2. **在面试中展示**
   - 展示分析报告
   - 解释火焰图含义
   - 说明优化措施

3. **项目价值体现**
   - 工程化程度高
   - 代码质量有保证
   - 性能优化有依据

## 🔧 故障排除

### 常见问题

1. **Valgrind未安装**
   ```bash
   sudo apt update
   sudo apt install valgrind
   ```

2. **perf权限问题**
   ```bash
   # 临时解决
   sudo sh -c 'echo -1 >/proc/sys/kernel/perf_event_paranoid'
   
   # 永久解决
   sudo sh -c 'echo kernel.perf_event_paranoid = -1 >> /etc/sysctl.conf'
   sudo sysctl -p
   ```

3. **FlameGraph下载失败**
   ```bash
   cd tools
   git clone https://github.com/brendangregg/FlameGraph.git
   cd ..
   ```

### 性能影响
- **Valgrind**: 会显著降低程序性能，仅用于检测
- **perf**: 对程序性能影响很小，可用于生产环境

## 📚 参考资料

- [Valgrind官方文档](https://valgrind.org/docs/manual/manual.html)
- [perf官方文档](https://perf.wiki.kernel.org/)
- [FlameGraph项目](https://github.com/brendangregg/FlameGraph)
- [Linux性能分析](http://www.brendangregg.com/linuxperf.html)

## 🎉 总结

NetBox的分析工具套件提供了专业级的代码质量检查和性能分析能力，不仅有助于项目开发，更是校招面试中的亮点展示。

通过这些工具，你可以：
1. 确保代码质量
2. 优化性能瓶颈
3. 展示工程能力
4. 提升面试竞争力 