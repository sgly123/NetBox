#!/bin/bash

##############################################################################
# NetBox Echo服务器快速压力测试脚本
# 
# 用于快速验证和演示，使用较轻的测试参数
##############################################################################

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}"
cat << "EOF"
╔══════════════════════════════════════╗
║     NetBox 快速压力测试              ║
║     轻量级测试，验证基本功能          ║
╚══════════════════════════════════════╝
EOF
echo -e "${NC}"

echo -e "${GREEN}启动快速压力测试...${NC}"

# 运行压力测试（使用较小的参数）
exec "$SCRIPT_DIR/stress_test.sh" \
    --host 127.0.0.1 \
    --port 8888 \
    --server-threads 2 \
    --threads 4 \
    --connections 5 \
    --requests 50 \
    --size 32 \
    --duration 10 \
    --perf-time 8 \
    "$@" 