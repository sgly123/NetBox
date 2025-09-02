#!/bin/bash

# NetBox Docker构建脚本
# 用于构建和部署NetBox Docker镜像

set -e

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
    echo -e "${PURPLE}🐳 NetBox Docker 构建脚本${NC}"
    echo -e "${PURPLE}========================${NC}"
}

# 检查Docker是否安装
check_docker() {
    if ! command -v docker &> /dev/null; then
        print_error "Docker未安装，请先安装Docker"
        exit 1
    fi
    
    if ! docker info &> /dev/null; then
        print_error "Docker服务未运行，请启动Docker服务"
        exit 1
    fi
    
    print_success "Docker环境检查通过"
}

# 检查Docker Compose是否安装
check_docker_compose() {
    if ! command -v docker-compose &> /dev/null; then
        print_warning "Docker Compose未安装，将使用docker build命令"
        USE_COMPOSE=false
    else
        print_success "Docker Compose环境检查通过"
        USE_COMPOSE=true
    fi
}

# 构建Docker镜像
build_image() {
    local image_name=${1:-"netbox:latest"}
    local build_args=${2:-""}
    
    print_info "开始构建Docker镜像: $image_name"
    
    if [ "$USE_COMPOSE" = true ]; then
        print_info "使用Docker Compose构建..."
        docker-compose build $build_args
    else
        print_info "使用Docker build构建..."
        docker build -t $image_name .
    fi
    
    print_success "Docker镜像构建完成: $image_name"
}

# 运行容器
run_container() {
    local image_name=${1:-"netbox:latest"}
    local container_name=${2:-"netbox-server"}
    
    print_info "启动NetBox容器: $container_name"
    
    if [ "$USE_COMPOSE" = true ]; then
        print_info "使用Docker Compose启动..."
        docker-compose up -d
    else
        print_info "使用Docker run启动..."
        docker run -d \
            --name $container_name \
            --restart unless-stopped \
            -p 6379:6379 \
            -p 8888:8888 \
            -v $(pwd)/config:/app/config:ro \
            -v $(pwd)/logs:/app/logs \
            -v $(pwd)/data:/app/data \
            $image_name
    fi
    
    print_success "NetBox容器启动完成"
}

# 停止容器
stop_container() {
    local container_name=${1:-"netbox-server"}
    
    print_info "停止NetBox容器: $container_name"
    
    if [ "$USE_COMPOSE" = true ]; then
        docker-compose down
    else
        docker stop $container_name 2>/dev/null || true
        docker rm $container_name 2>/dev/null || true
    fi
    
    print_success "NetBox容器已停止"
}

# 查看容器状态
show_status() {
    print_info "容器状态:"
    
    if [ "$USE_COMPOSE" = true ]; then
        docker-compose ps
    else
        docker ps -a --filter "name=netbox"
    fi
    
    print_info "容器日志:"
    if [ "$USE_COMPOSE" = true ]; then
        docker-compose logs --tail=20
    else
        docker logs --tail=20 netbox-server 2>/dev/null || echo "容器未运行"
    fi
}

# 清理资源
cleanup() {
    print_info "清理Docker资源..."
    
    if [ "$USE_COMPOSE" = true ]; then
        docker-compose down --rmi all --volumes --remove-orphans
    else
        docker stop netbox-server 2>/dev/null || true
        docker rm netbox-server 2>/dev/null || true
        docker rmi netbox:latest 2>/dev/null || true
    fi
    
    print_success "清理完成"
}

# 显示帮助信息
show_help() {
    echo "用法: $0 [命令]"
    echo ""
    echo "命令:"
    echo "  build    构建Docker镜像"
    echo "  run      构建并运行容器"
    echo "  stop     停止容器"
    echo "  status   查看容器状态"
    echo "  cleanup  清理所有资源"
    echo "  help     显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 build    # 构建镜像"
    echo "  $0 run      # 构建并运行"
    echo "  $0 status   # 查看状态"
}

# 主函数
main() {
    print_header
    
    # 检查环境
    check_docker
    check_docker_compose
    
    # 解析命令
    case "${1:-help}" in
        "build")
            build_image
            ;;
        "run")
            build_image
            run_container
            ;;
        "stop")
            stop_container
            ;;
        "status")
            show_status
            ;;
        "cleanup")
            cleanup
            ;;
        "help"|*)
            show_help
            ;;
    esac
}

# 执行主函数
main "$@" 