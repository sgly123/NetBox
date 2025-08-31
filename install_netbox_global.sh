#!/bin/bash

# NetBox 全局安装脚本
# 将NetBox CLI工具安装到系统路径，支持全局使用

set -e

echo "🚀 NetBox 全局安装脚本"
echo "============================================================"

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 检查是否在NetBox项目目录中
if [ ! -f "$SCRIPT_DIR/tools/netbox-cli-v2.py" ]; then
    echo "❌ 错误: 请在NetBox项目根目录中运行此脚本"
    exit 1
fi

# 检查Python是否可用
if ! command -v python3 &> /dev/null; then
    echo "❌ 错误: 未找到python3"
    echo "💡 请安装Python 3.7+: sudo apt install python3"
    exit 1
fi

# 确定安装路径
INSTALL_DIR="/usr/local/bin"
NETBOX_BIN="$INSTALL_DIR/netbox"

# 检查权限
if [ ! -w "$INSTALL_DIR" ]; then
    echo "🔐 需要管理员权限来安装到 $INSTALL_DIR"
    echo "💡 请使用: sudo ./install_netbox_global.sh"
    exit 1
fi

echo "📁 安装目录: $INSTALL_DIR"
echo "🔧 创建全局命令: netbox"

# 创建全局netbox命令
cat > "$NETBOX_BIN" << 'EOF'
#!/bin/bash

# NetBox CLI 全局命令
# 支持模板选择: netbox init MyProject --template web_server

# 查找NetBox项目目录
NETBOX_DIR=""
COMMON_PATHS=(
    "$HOME/桌面/NetBox"
    "$HOME/Desktop/NetBox"
    "$HOME/NetBox"
    "/opt/NetBox"
    "/usr/local/NetBox"
    "/usr/share/NetBox"
)

for path in "${COMMON_PATHS[@]}"; do
    if [ -f "$path/tools/netbox-cli-v2.py" ]; then
        NETBOX_DIR="$path"
        break
    fi
done

# 如果找不到，尝试从当前目录向上查找
if [ -z "$NETBOX_DIR" ]; then
    CURRENT_DIR="$PWD"
    while [ "$CURRENT_DIR" != "/" ]; do
        if [ -f "$CURRENT_DIR/tools/netbox-cli-v2.py" ]; then
            NETBOX_DIR="$CURRENT_DIR"
            break
        fi
        CURRENT_DIR="$(dirname "$CURRENT_DIR")"
    done
fi

# 检查是否找到NetBox目录
if [ -z "$NETBOX_DIR" ]; then
    echo "❌ 错误: 找不到NetBox项目目录"
    echo "💡 请确保NetBox已正确安装"
    echo "💡 或者运行: ./install_netbox_global.sh"
    exit 1
fi

# 检查Python脚本是否存在
NETBOX_CLI="$NETBOX_DIR/tools/netbox-cli-v2.py"
if [ ! -f "$NETBOX_CLI" ]; then
    echo "❌ 错误: 找不到NetBox CLI脚本"
    echo "📍 期望路径: $NETBOX_CLI"
    exit 1
fi

# 显示帮助信息（如果用户没有提供参数）
if [ $# -eq 0 ]; then
    echo "🚀 NetBox CLI - 企业级跨平台网络框架"
    echo "============================================================"
    echo ""
    echo "📋 可用命令:"
    echo "  init [项目名] [--template 模板名]    创建新项目"
    echo "  list-templates                      列出所有可用模板"
    echo "  build [--type Debug|Release]        构建项目"
    echo "  test                                运行测试"
    echo "  run [目标程序]                      运行程序"
    echo "  info                                显示项目信息"
    echo "  clean                               清理构建文件"
    echo ""
    echo "🎯 模板类型:"
    echo "  default      - 默认模板 (基础功能)"
    echo "  web_server   - Web服务器模板"
    echo "  game_server  - 游戏服务器模板"
    echo "  microservice - 微服务模板"
    echo ""
    echo "💡 示例:"
    echo "  netbox init MyProject                    # 使用默认模板"
    echo "  netbox init MyWebApp --template web_server    # 使用Web服务器模板"
    echo "  netbox init MyGameApp --template game_server  # 使用游戏服务器模板"
    echo "  netbox list-templates                    # 查看所有模板"
    echo ""
    exit 0
fi

# 执行Python CLI脚本，传递所有参数
exec python3 "$NETBOX_CLI" "$@"
EOF

# 设置执行权限
chmod +x "$NETBOX_BIN"

echo "✅ 全局安装完成!"
echo ""
echo "🎉 现在您可以在任何地方使用 netbox 命令:"
echo ""
echo "💡 示例用法:"
echo "  netbox init MyProject                    # 使用默认模板"
echo "  netbox init MyWebApp --template web_server    # 使用Web服务器模板"
echo "  netbox init MyGameApp --template game_server  # 使用游戏服务器模板"
echo "  netbox list-templates                    # 查看所有模板"
echo ""
echo "🔧 卸载方法:"
echo "  sudo rm $NETBOX_BIN"
echo ""
echo "📁 NetBox项目目录: $SCRIPT_DIR"
echo "🔗 全局命令路径: $NETBOX_BIN" 