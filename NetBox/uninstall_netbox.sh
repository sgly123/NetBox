#!/bin/bash

# NetBox CLI 卸载脚本
# 从系统中移除netbox命令

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🗑️  NetBox CLI 卸载脚本${NC}"
echo -e "${BLUE}========================${NC}"

# 查找netbox命令的位置
NETBOX_PATH=""
if command -v netbox &> /dev/null; then
    NETBOX_PATH="$(which netbox)"
    echo -e "${BLUE}📍 找到netbox命令: $NETBOX_PATH${NC}"
else
    echo -e "${YELLOW}⚠️  未找到netbox命令${NC}"
    exit 0
fi

# 确认卸载
echo -e "${YELLOW}❓ 确定要卸载netbox命令吗? (y/N)${NC}"
read -r response
if [[ ! "$response" =~ ^[Yy]$ ]]; then
    echo -e "${BLUE}✅ 取消卸载${NC}"
    exit 0
fi

# 移除netbox命令
echo -e "${BLUE}🗑️  移除netbox命令...${NC}"
rm -f "$NETBOX_PATH"
echo -e "${GREEN}✅ netbox命令已移除${NC}"

# 检查是否需要从PATH中移除
INSTALL_DIR="$(dirname "$NETBOX_PATH")"
SHELL_PROFILES=(
    "$HOME/.bashrc"
    "$HOME/.bash_profile"
    "$HOME/.zshrc"
)

for profile in "${SHELL_PROFILES[@]}"; do
    if [ -f "$profile" ] && grep -q "NetBox CLI PATH" "$profile"; then
        echo -e "${BLUE}📝 从 $profile 中移除PATH配置...${NC}"
        # 创建临时文件，移除NetBox相关的PATH配置
        sed '/^# NetBox CLI PATH$/,+1d' "$profile" > "${profile}.tmp"
        mv "${profile}.tmp" "$profile"
        echo -e "${GREEN}✅ 已从 $profile 中移除PATH配置${NC}"
    fi
done

echo -e "${GREEN}🎉 卸载完成!${NC}"
echo -e "${BLUE}💡 如果需要在其他shell中使用，请重新加载配置${NC}" 