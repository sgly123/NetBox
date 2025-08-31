#!/bin/bash

echo "正在优化系统参数以提高网络性能..."

# 临时调整文件描述符限制
echo "调整文件描述符限制..."
ulimit -n 65536

# 调整网络参数
echo "调整网络参数..."
# 增加TCP缓冲区大小
echo 1048576 > /proc/sys/net/core/rmem_max
echo 1048576 > /proc/sys/net/core/wmem_max
echo 1048576 > /proc/sys/net/core/rmem_default
echo 1048576 > /proc/sys/net/core/wmem_default

# 调整TCP参数
echo 65536 > /proc/sys/net/ipv4/tcp_rmem
echo 65536 > /proc/sys/net/ipv4/tcp_wmem
echo 1 > /proc/sys/net/ipv4/tcp_tw_reuse
echo 1 > /proc/sys/net/ipv4/tcp_tw_recycle

# 调整连接队列大小
echo 65536 > /proc/sys/net/core/somaxconn
echo 65536 > /proc/sys/net/ipv4/tcp_max_syn_backlog

echo "系统参数优化完成！"
echo "当前文件描述符限制: $(ulimit -n)"
echo "建议在测试前运行此脚本" 