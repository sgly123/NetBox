#!/bin/bash

# NetBox CLI 使用示例脚本
# 展示NetBox脚手架的各种功能

echo "🚀 NetBox CLI 使用示例演示"
echo "================================"

# 设置颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 函数：显示步骤
show_step() {
    echo -e "\n${CYAN}📋 步骤 $1: $2${NC}"
    echo "----------------------------------------"
}

# 函数：执行命令并显示
run_command() {
    echo -e "${YELLOW}💻 执行命令:${NC} $1"
    echo -e "${BLUE}$1${NC}"
    
    # 询问是否执行
    read -p "是否执行此命令? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        eval $1
        echo -e "${GREEN}✅ 命令执行完成${NC}"
    else
        echo -e "${YELLOW}⏭️  跳过此命令${NC}"
    fi
}

# 函数：暂停
pause() {
    echo -e "\n${YELLOW}按任意键继续...${NC}"
    read -n 1 -s
}

# 主演示流程
main() {
    echo -e "${GREEN}欢迎使用NetBox CLI演示！${NC}"
    echo "本脚本将展示NetBox脚手架的主要功能"
    pause

    # 步骤1: 查看帮助
    show_step "1" "查看CLI帮助信息"
    run_command "python3 tools/netbox-cli.py --help"
    pause

    # 步骤2: 测试启动动画
    show_step "2" "测试启动动画效果"
    run_command "python3 tools/netbox-cli.py animation startup"
    pause

    # 步骤3: 测试进度条
    show_step "3" "测试不同风格的进度条"
    echo "可用的进度条样式："
    echo "  - blocks: 方块风格"
    echo "  - gradient: 渐变风格"
    echo "  - wave: 波浪效果"
    echo "  - rainbow: 彩虹效果"
    echo "  - multi: 多任务进度条"
    
    run_command "python3 tools/netbox-cli.py animation progress --style blocks"
    run_command "python3 tools/netbox-cli.py animation progress --style wave"
    run_command "python3 tools/netbox-cli.py animation progress --multi"
    pause

    # 步骤4: 创建项目
    show_step "4" "创建新的NetBox框架项目"
    echo "这将创建一个完整的框架项目，包含："
    echo "  ✅ 完整的扩展接口和基类"
    echo "  ✅ 协议、应用、网络层扩展示例"
    echo "  ✅ 插件系统和示例插件"
    echo "  ✅ 详细的开发指南和API文档"
    
    PROJECT_NAME="DemoFramework"
    run_command "python3 tools/netbox-cli.py init $PROJECT_NAME"
    
    if [ -d "$PROJECT_NAME" ]; then
        echo -e "${GREEN}✅ 项目创建成功！${NC}"
        echo "项目结构："
        run_command "ls -la $PROJECT_NAME"
        pause

        # 步骤5: 查看生成的文档
        show_step "5" "查看生成的开发文档"
        run_command "ls $PROJECT_NAME/docs/"
        run_command "head -20 $PROJECT_NAME/docs/开发指南.md"
        pause

        # 步骤6: 创建Hello World示例
        show_step "6" "创建Hello World示例"
        run_command "python3 tools/netbox-cli.py hello $PROJECT_NAME"
        
        if [ -f "$PROJECT_NAME/examples/hello_world.cpp" ]; then
            echo -e "${GREEN}✅ Hello World示例创建成功！${NC}"
            run_command "head -20 $PROJECT_NAME/examples/hello_world.cpp"
            
            # 步骤7: 编译运行示例
            show_step "7" "编译并运行Hello World示例"
            run_command "cd $PROJECT_NAME/examples && g++ -o hello_world hello_world.cpp"
            
            if [ -f "$PROJECT_NAME/examples/hello_world" ]; then
                echo -e "${GREEN}✅ 编译成功！${NC}"
                echo "运行Hello World程序："
                run_command "cd $PROJECT_NAME/examples && echo '测试用户' | ./hello_world"
            fi
        fi
        pause

        # 步骤8: 查看项目信息
        show_step "8" "查看项目信息"
        run_command "cd $PROJECT_NAME && python3 ../tools/netbox-cli.py info"
        pause

        # 步骤9: 清理演示项目
        show_step "9" "清理演示项目"
        echo -e "${YELLOW}是否删除演示项目 $PROJECT_NAME？${NC}"
        read -p "删除演示项目? (y/n): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            run_command "rm -rf $PROJECT_NAME"
            echo -e "${GREEN}✅ 演示项目已清理${NC}"
        else
            echo -e "${BLUE}📁 演示项目保留在: $PROJECT_NAME${NC}"
        fi
    fi

    # 步骤10: 完整进度条展示
    show_step "10" "完整进度条效果展示"
    run_command "python3 tools/progress_showcase.py static"
    pause

    # 演示结束
    echo -e "\n${GREEN}🎉 NetBox CLI 演示完成！${NC}"
    echo "================================"
    echo -e "${CYAN}📖 更多信息：${NC}"
    echo "  - 完整文档: docs/CLI使用指南.md"
    echo "  - 快速参考: docs/CLI快速参考.md"
    echo "  - 开发指南: 生成项目中的 docs/开发指南.md"
    echo ""
    echo -e "${YELLOW}💡 提示：${NC}"
    echo "  - 使用 'python3 tools/netbox-cli.py --help' 查看所有命令"
    echo "  - 使用 'python3 tools/netbox-cli.py <命令> --help' 查看特定命令帮助"
    echo "  - 尝试不同的进度条样式和动画效果"
    echo ""
    echo -e "${GREEN}感谢使用NetBox CLI！🚀${NC}"
}

# 检查是否在NetBox目录中
if [ ! -f "tools/netbox-cli.py" ]; then
    echo -e "${RED}❌ 错误: 请在NetBox项目根目录中运行此脚本${NC}"
    echo "当前目录: $(pwd)"
    echo "请切换到NetBox目录后重新运行"
    exit 1
fi

# 运行主函数
main
