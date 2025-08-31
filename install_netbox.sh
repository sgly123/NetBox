#!/bin/bash

# NetBox CLI 全局安装脚本
# 将netbox命令安装到系统PATH中

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NETBOX_SCRIPT="$SCRIPT_DIR/netbox"

echo -e "${BLUE}🚀 NetBox CLI 全局安装脚本${NC}"
echo -e "${BLUE}================================${NC}"

# 检查netbox脚本是否存在
if [ ! -f "$NETBOX_SCRIPT" ]; then
    echo -e "${RED}❌ 错误: 找不到netbox脚本${NC}"
    echo -e "${YELLOW}📍 期望路径: $NETBOX_SCRIPT${NC}"
    exit 1
fi

# 检查Python是否可用
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}❌ 错误: 未找到python3${NC}"
    echo -e "${YELLOW}💡 请安装Python 3.7+: sudo apt install python3${NC}"
    exit 1
fi

# 确定安装目录
if [ "$EUID" -eq 0 ]; then
    # 如果是root用户，安装到系统目录
    INSTALL_DIR="/usr/local/bin"
    echo -e "${YELLOW}🔧 检测到root权限，安装到系统目录: $INSTALL_DIR${NC}"
else
    # 普通用户安装到用户目录
    INSTALL_DIR="$HOME/.local/bin"
    echo -e "${YELLOW}🔧 安装到用户目录: $INSTALL_DIR${NC}"
fi

# 创建安装目录
mkdir -p "$INSTALL_DIR"

# 复制netbox脚本到安装目录
echo -e "${BLUE}📦 安装netbox命令...${NC}"
cp "$NETBOX_SCRIPT" "$INSTALL_DIR/netbox"

# 设置执行权限
chmod +x "$INSTALL_DIR/netbox"

# 检查PATH是否包含安装目录
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo -e "${YELLOW}⚠️  安装目录不在PATH中，需要添加到PATH${NC}"
    
    # 确定shell配置文件
    SHELL_PROFILE=""
    if [ -f "$HOME/.bashrc" ]; then
        SHELL_PROFILE="$HOME/.bashrc"
    elif [ -f "$HOME/.bash_profile" ]; then
        SHELL_PROFILE="$HOME/.bash_profile"
    elif [ -f "$HOME/.zshrc" ]; then
        SHELL_PROFILE="$HOME/.zshrc"
    fi
    
    if [ -n "$SHELL_PROFILE" ]; then
        echo -e "${BLUE}📝 添加PATH到 $SHELL_PROFILE${NC}"
        echo "" >> "$SHELL_PROFILE"
        echo "# NetBox CLI PATH" >> "$SHELL_PROFILE"
        echo "export PATH=\"$INSTALL_DIR:\$PATH\"" >> "$SHELL_PROFILE"
        echo -e "${GREEN}✅ PATH已添加到 $SHELL_PROFILE${NC}"
        echo -e "${YELLOW}💡 请重新加载shell配置: source $SHELL_PROFILE${NC}"
    else
        echo -e "${YELLOW}⚠️  请手动将 $INSTALL_DIR 添加到PATH${NC}"
    fi
else
    echo -e "${GREEN}✅ 安装目录已在PATH中${NC}"
fi

# 测试安装
echo -e "${BLUE}🧪 测试安装...${NC}"
if command -v netbox &> /dev/null; then
    echo -e "${GREEN}✅ netbox命令安装成功!${NC}"
    echo -e "${BLUE}🎯 现在可以在任何地方使用netbox命令${NC}"
    echo ""
    echo -e "${GREEN}📋 使用示例:${NC}"
    echo -e "${YELLOW}  netbox init MyProject${NC}"
    echo -e "${YELLOW}  netbox build${NC}"
    echo -e "${YELLOW}  netbox run${NC}"
    echo -e "${YELLOW}  netbox --help${NC}"
    echo ""
    echo -e "${BLUE}🎉 安装完成! 享受使用NetBox CLI!${NC}"
else
    echo -e "${RED}❌ 安装测试失败${NC}"
    echo -e "${YELLOW}💡 请确保 $INSTALL_DIR 在PATH中${NC}"
    exit 1
fi 