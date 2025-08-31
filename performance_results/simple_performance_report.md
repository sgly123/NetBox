# 🚀 NetBox 简化性能测试报告

## 📋 测试信息
- **测试时间**: 2025年 08月 27日 星期三 17:52:09 CST
- **测试类型**: QPS + Perf分析
- **服务器**: NetBox Echo Server (127.0.0.1:8888)
- **服务器PID**: 62753

## 🔥 QPS测试结果
🔥 开始QPS测试: 4线程 x 10秒
📊 QPS测试结果:
   总请求数: 89249
   测试时长: 10.00秒
   实际QPS: 8921.90

## 📈 性能分析文件
- **Perf数据**: simple_perf.data
- **Perf报告**: simple_perf_report.txt  
- **火焰图**: simple_flamegraph.svg
- **CPU统计**: simple_cpu_stat.txt
- **服务器日志**: simple_server.log

## 🔍 Perf热点分析


## 💻 CPU使用情况
WARNING: perf not found for kernel 6.14.0-24

  You may need to install the following packages for this specific kernel:
    linux-tools-6.14.0-24-generic
    linux-cloud-tools-6.14.0-24-generic

  You may also want to install one of the following packages to keep up to date:
    linux-tools-generic
    linux-cloud-tools-generic

## 🎯 分析总结
1. **QPS性能**: 基于实际连接测试
2. **CPU热点**: 通过perf分析识别性能瓶颈
3. **火焰图**: 可视化函数调用开销
4. **优化建议**: 基于分析结果提供优化方向

## 📁 查看详细结果
- 查看perf详情: `sudo perf report -i performance_results/simple_perf.data`
- 查看火焰图: 在浏览器中打开 `performance_results/simple_flamegraph.svg`
- 查看CPU统计: `cat performance_results/simple_cpu_stat.txt`

---
报告生成时间: 2025年 08月 27日 星期三 17:52:09 CST
