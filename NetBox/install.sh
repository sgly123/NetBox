#!/bin/bash

# NetBox CLI 安装脚本
# 将netbox命令安装到系统PATH中，实现全局访问

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${CYAN}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_header() {
    echo -e "${PURPLE}🏆 NetBox CLI v2.0 安装程序${NC}"
    echo -e "${PURPLE}==============================${NC}"
}

# 检查是否为root用户
check_root() {
    if [ "$EUID" -eq 0 ]; then
        print_warning "检测到root用户，将安装到系统目录"
        INSTALL_DIR="/usr/local/bin"
        NETBOX_HOME="/opt/netbox"
    else
        print_info "检测到普通用户，将安装到用户目录"
        INSTALL_DIR="$HOME/.local/bin"
        NETBOX_HOME="$HOME/.netbox"
        
        # 确保用户bin目录存在
        mkdir -p "$INSTALL_DIR"
        
        # 检查PATH中是否包含用户bin目录
        if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
            print_warning "需要将 $INSTALL_DIR 添加到PATH中"
            echo "请在 ~/.bashrc 或 ~/.zshrc 中添加:"
            echo "export PATH=\"\$PATH:$INSTALL_DIR\""
        fi
    fi
}

# 获取NetBox源码目录
get_source_dir() {
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    
    # 检查是否在NetBox项目根目录
    if [ ! -f "$SCRIPT_DIR/tools/netbox-cli-v2.py" ]; then
        print_error "未找到NetBox CLI脚本"
        print_info "请确保在NetBox项目根目录中运行此安装脚本"
        exit 1
    fi
    
    SOURCE_DIR="$SCRIPT_DIR"
    print_info "NetBox源码目录: $SOURCE_DIR"
}

# 安装NetBox
install_netbox() {
    print_info "开始安装NetBox CLI..."
    
    # 创建NetBox主目录
    print_info "创建NetBox主目录: $NETBOX_HOME"
    mkdir -p "$NETBOX_HOME"
    
    # 复制必要文件
    print_info "复制NetBox文件..."
    cp -r "$SOURCE_DIR/tools" "$NETBOX_HOME/"
    cp -r "$SOURCE_DIR/docs" "$NETBOX_HOME/"
    cp "$SOURCE_DIR/README.md" "$NETBOX_HOME/" 2>/dev/null || true
    
    # 创建全局netbox命令
    print_info "创建全局netbox命令: $INSTALL_DIR/netbox"
    
    cat > "$INSTALL_DIR/netbox" << EOF
#!/bin/bash

# NetBox CLI 全局命令
# 自动生成于 $(date)

# NetBox安装目录
NETBOX_HOME="$NETBOX_HOME"

# NetBox CLI Python脚本路径
NETBOX_CLI="\$NETBOX_HOME/tools/ascii_art.py"

# 检查NetBox是否已安装
if [ ! -f "\$NETBOX_CLI" ]; then
    echo "❌ 错误: NetBox CLI未正确安装"
    echo "📍 期望路径: \$NETBOX_CLI"
    echo "💡 请重新运行安装脚本: curl -sSL https://netbox.dev/install.sh | bash"
    exit 1
fi

# 检查Python是否可用
if ! command -v python3 &> /dev/null; then
    echo "❌ 错误: 未找到python3"
    echo "💡 请安装Python 3.7+:"
    echo "   Ubuntu/Debian: sudo apt install python3"
    echo "   CentOS/RHEL:   sudo yum install python3"
    echo "   macOS:         brew install python3"
    exit 1
fi

# 显示NetBox标题（仅在非help命令时）
if [ \$# -eq 0 ] || [ "\$1" != "--help" ] && [ "\$1" != "-h" ] && [ "\$1" != "help" ]; then
    echo "🏆 NetBox CLI v2.0"
    echo "=================="
fi

# 执行Python CLI脚本，传递所有参数
exec python3 "\$NETBOX_CLI" "\$@"
EOF

    # 设置执行权限
    chmod +x "$INSTALL_DIR/netbox"
    
    print_success "NetBox CLI安装完成!"
}

# 验证安装
verify_installation() {
    print_info "验证安装..."
    
    if [ -x "$INSTALL_DIR/netbox" ]; then
        print_success "netbox命令已安装到: $INSTALL_DIR/netbox"
        
        # 测试命令
        if "$INSTALL_DIR/netbox" --version >/dev/null 2>&1; then
            print_success "netbox命令测试通过"
        else
            print_warning "netbox命令测试失败，但文件已安装"
        fi
    else
        print_error "安装失败: netbox命令不可执行"
        exit 1
    fi
}

# 显示安装后信息
show_post_install_info() {
    echo
    print_success "🎉 NetBox CLI v2.0 安装成功!"
    echo
    print_info "现在您可以在任何地方使用以下命令:"
    echo
    echo "  netbox init MyProject     # 创建新项目"
    echo "  netbox build             # 构建项目"
    echo "  netbox test              # 运行测试"
    echo "  netbox run               # 运行程序"
    echo "  netbox info              # 显示信息"
    echo "  netbox --help            # 显示帮助"
    echo
    
    if [ "$EUID" -ne 0 ] && [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
        print_warning "重要提示:"
        echo "请将以下行添加到您的 ~/.bashrc 或 ~/.zshrc 文件中:"
        echo
        echo "export PATH=\"\$PATH:$INSTALL_DIR\""
        echo
        echo "然后运行: source ~/.bashrc"
        echo
    fi
    
    print_info "快速开始:"
    echo "  netbox init MyAwesomeProject"
    echo "  cd MyAwesomeProject"
    echo "  netbox build"
    echo "  netbox run"
    echo
    print_success "享受NetBox带来的开发体验! 🚀"
}

# 卸载函数
uninstall_netbox() {
    print_info "开始卸载NetBox CLI..."
    
    # 删除全局命令
    if [ -f "$INSTALL_DIR/netbox" ]; then
        rm -f "$INSTALL_DIR/netbox"
        print_success "已删除: $INSTALL_DIR/netbox"
    fi
    
    # 删除NetBox主目录
    if [ -d "$NETBOX_HOME" ]; then
        rm -rf "$NETBOX_HOME"
        print_success "已删除: $NETBOX_HOME"
    fi
    
    print_success "NetBox CLI卸载完成!"
}

# 主函数
main() {
    print_header
    
    # 检查命令行参数
    if [ "$1" = "--uninstall" ] || [ "$1" = "-u" ]; then
        check_root
        uninstall_netbox
        exit 0
    fi
    
    # 执行安装
    check_root
    get_source_dir
    install_netbox
    verify_installation
    show_post_install_info
}

# 运行主函数
main "$@"
