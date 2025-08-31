# 🚀 NetBox 安装部署指南

> **从零开始，完整的NetBox安装、配置和部署指南**

## 📋 目录

- [系统要求](#系统要求)
- [快速安装](#快速安装)
- [详细安装步骤](#详细安装步骤)
- [环境配置](#环境配置)
- [验证安装](#验证安装)
- [生产环境部署](#生产环境部署)
- [Docker部署](#docker部署)
- [故障排除](#故障排除)

---

## 🔧 系统要求

### 支持的操作系统

| 操作系统 | 版本要求 | 架构支持 | 状态 |
|----------|----------|----------|------|
| **Ubuntu** | 18.04+ | x64/ARM64 | ✅ 完全支持 |
| **CentOS** | 7+ | x64/ARM64 | ✅ 完全支持 |
| **Debian** | 10+ | x64/ARM64 | ✅ 完全支持 |
| **Windows** | 7+ | x86/x64/ARM64 | ✅ 完全支持 |
| **macOS** | 10.9+ | x64/ARM64 | ✅ 完全支持 |

### 硬件要求

| 组件 | 最低要求 | 推荐配置 | 生产环境 |
|------|----------|----------|----------|
| **CPU** | 2核心 | 4核心+ | 8核心+ |
| **内存** | 2GB | 8GB+ | 16GB+ |
| **存储** | 1GB | 10GB+ | 50GB+ |
| **网络** | 100Mbps | 1Gbps+ | 10Gbps+ |

### 软件依赖

```bash
# 必需依赖
- CMake 3.16+
- Python 3.7+
- Git 2.0+
- C++17编译器 (GCC 9+/Clang 10+/MSVC 2019+)

# 可选依赖
- Jinja2 (模板引擎)
- Doxygen (文档生成)
- Valgrind (内存检测)
- Docker (容器化部署)
```

---

## ⚡ 快速安装

### Ubuntu/Debian 一键安装

```bash
# 下载安装脚本
curl -fsSL https://raw.githubusercontent.com/netbox/netbox/main/scripts/install.sh | bash

# 或者手动安装
wget https://raw.githubusercontent.com/netbox/netbox/main/scripts/install.sh
chmod +x install.sh
./install.sh
```

### CentOS/RHEL 一键安装

```bash
# 下载安装脚本
curl -fsSL https://raw.githubusercontent.com/netbox/netbox/main/scripts/install-centos.sh | bash
```

### macOS 一键安装

```bash
# 使用Homebrew
brew tap netbox/netbox
brew install netbox

# 或者手动安装
curl -fsSL https://raw.githubusercontent.com/netbox/netbox/main/scripts/install-macos.sh | bash
```

### Windows 一键安装

```powershell
# 使用PowerShell (管理员权限)
Set-ExecutionPolicy Bypass -Scope Process -Force
iex ((New-Object System.Net.WebClient).DownloadString('https://raw.githubusercontent.com/netbox/netbox/main/scripts/install-windows.ps1'))
```

---

## 📦 详细安装步骤

### Ubuntu/Debian 详细安装

```bash
# 1. 更新系统包
sudo apt update && sudo apt upgrade -y

# 2. 安装基础依赖
sudo apt install -y \
    build-essential \
    cmake \
    python3 \
    python3-pip \
    git \
    curl \
    wget

# 3. 安装可选依赖
sudo apt install -y \
    python3-jinja2 \
    doxygen \
    valgrind \
    docker.io

# 4. 克隆NetBox项目
git clone https://github.com/netbox/netbox.git
cd NetBox

# 5. 设置CLI工具
chmod +x tools/netbox-cli.py
sudo ln -s $(pwd)/tools/netbox-cli.py /usr/local/bin/netbox

# 6. 验证安装
netbox --version
netbox info
```

### CentOS/RHEL 详细安装

```bash
# 1. 安装EPEL仓库
sudo yum install -y epel-release

# 2. 安装基础依赖
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    cmake3 \
    python3 \
    python3-pip \
    git \
    curl \
    wget

# 3. 创建cmake软链接
sudo ln -s /usr/bin/cmake3 /usr/bin/cmake

# 4. 安装可选依赖
sudo yum install -y \
    python3-jinja2 \
    doxygen \
    valgrind \
    docker

# 5. 克隆和配置NetBox
git clone https://github.com/netbox/netbox.git
cd NetBox
chmod +x tools/netbox-cli.py
sudo ln -s $(pwd)/tools/netbox-cli.py /usr/local/bin/netbox

# 6. 验证安装
netbox --version
```

### macOS 详细安装

```bash
# 1. 安装Xcode命令行工具
xcode-select --install

# 2. 安装Homebrew (如果未安装)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 3. 安装依赖
brew install cmake python git

# 4. 安装可选依赖
brew install doxygen
pip3 install jinja2

# 5. 克隆和配置NetBox
git clone https://github.com/netbox/netbox.git
cd NetBox
chmod +x tools/netbox-cli.py
sudo ln -s $(pwd)/tools/netbox-cli.py /usr/local/bin/netbox

# 6. 验证安装
netbox --version
```

### Windows 详细安装

```cmd
# 1. 安装Visual Studio 2019+ (包含C++工具)
# 下载地址: https://visualstudio.microsoft.com/downloads/

# 2. 安装CMake
# 下载地址: https://cmake.org/download/

# 3. 安装Python 3.7+
# 下载地址: https://www.python.org/downloads/

# 4. 安装Git
# 下载地址: https://git-scm.com/download/win

# 5. 克隆NetBox项目
git clone https://github.com/netbox/netbox.git
cd NetBox

# 6. 验证安装
python tools/netbox-cli.py --version
python tools/netbox-cli.py info
```

---

## ⚙️ 环境配置

### 环境变量设置

```bash
# Linux/macOS
export NETBOX_HOME=/path/to/NetBox
export PATH=$NETBOX_HOME/tools:$PATH
export NETBOX_CONFIG_DIR=$NETBOX_HOME/config

# Windows
set NETBOX_HOME=C:\path\to\NetBox
set PATH=%NETBOX_HOME%\tools;%PATH%
set NETBOX_CONFIG_DIR=%NETBOX_HOME%\config
```

### 系统优化配置

```bash
# Linux系统优化
# 增加文件描述符限制
echo "* soft nofile 65535" | sudo tee -a /etc/security/limits.conf
echo "* hard nofile 65535" | sudo tee -a /etc/security/limits.conf

# 优化网络参数
echo "net.core.somaxconn = 65535" | sudo tee -a /etc/sysctl.conf
echo "net.ipv4.tcp_max_syn_backlog = 65535" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p

# 重启生效
sudo reboot
```

### 用户权限配置

```bash
# 创建netbox用户
sudo useradd -r -s /bin/false netbox

# 设置目录权限
sudo chown -R netbox:netbox /opt/netbox
sudo chmod -R 755 /opt/netbox

# 添加当前用户到netbox组
sudo usermod -a -G netbox $USER
```

---

## ✅ 验证安装

### 基础验证

```bash
# 1. 检查CLI工具
netbox --version
# 输出: NetBox CLI v1.0.0

# 2. 检查系统信息
netbox info
# 输出: 详细的系统和项目信息

# 3. 检查依赖
cmake --version    # >= 3.16
python3 --version  # >= 3.7
gcc --version      # >= 9.0 (Linux)
clang --version    # >= 10.0 (macOS)
```

### 功能验证

```bash
# 1. 创建测试项目
netbox init TestProject
cd TestProject

# 2. 构建项目
netbox build

# 3. 运行测试
netbox test

# 4. 性能测试
netbox benchmark

# 5. 运行演示
netbox demo
```

### 动画效果验证

```bash
# 测试启动动画
netbox animation startup

# 测试项目创建动画
netbox animation create --project TestAnimation

# 测试进度条效果
netbox animation progress --style rainbow
```

---

## 🚀 生产环境部署

### 系统服务配置

```bash
# 1. 创建服务文件
sudo tee /etc/systemd/system/netbox.service > /dev/null <<EOF
[Unit]
Description=NetBox Network Framework Server
After=network.target

[Service]
Type=simple
User=netbox
Group=netbox
WorkingDirectory=/opt/netbox
ExecStart=/opt/netbox/bin/netbox_server --config /opt/netbox/config/production.yaml
ExecReload=/bin/kill -HUP \$MAINPID
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

# 2. 启用并启动服务
sudo systemctl daemon-reload
sudo systemctl enable netbox
sudo systemctl start netbox

# 3. 检查服务状态
sudo systemctl status netbox
```

### Nginx反向代理配置

```nginx
# /etc/nginx/sites-available/netbox
server {
    listen 80;
    server_name your-domain.com;
    
    location / {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}

# 启用站点
sudo ln -s /etc/nginx/sites-available/netbox /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

### 监控配置

```bash
# 安装监控工具
sudo apt install -y prometheus node-exporter

# 配置Prometheus
sudo tee /etc/prometheus/prometheus.yml > /dev/null <<EOF
global:
  scrape_interval: 15s

scrape_configs:
  - job_name: 'netbox'
    static_configs:
      - targets: ['localhost:9090']
  
  - job_name: 'node'
    static_configs:
      - targets: ['localhost:9100']
EOF

# 启动监控服务
sudo systemctl enable prometheus node-exporter
sudo systemctl start prometheus node-exporter
```

---

## 🐳 Docker部署

### 基础Docker部署

```dockerfile
# Dockerfile
FROM ubuntu:20.04

# 设置环境变量
ENV DEBIAN_FRONTEND=noninteractive
ENV NETBOX_HOME=/app

# 安装依赖
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    python3 \
    python3-pip \
    git \
    && rm -rf /var/lib/apt/lists/*

# 复制源码
COPY . ${NETBOX_HOME}
WORKDIR ${NETBOX_HOME}

# 构建项目
RUN python3 tools/netbox-cli.py build --type Release

# 创建非root用户
RUN useradd -r -s /bin/false netbox && \
    chown -R netbox:netbox ${NETBOX_HOME}

# 切换用户
USER netbox

# 暴露端口
EXPOSE 8080

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# 启动命令
CMD ["./build/bin/netbox_server", "--config", "config/production.yaml"]
```

### Docker Compose部署

```yaml
# docker-compose.yml
version: '3.8'

services:
  netbox:
    build: .
    ports:
      - "8080:8080"
    volumes:
      - ./config:/app/config
      - ./logs:/app/logs
    environment:
      - NETBOX_ENV=production
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
      interval: 30s
      timeout: 10s
      retries: 3
    
  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf
      - ./ssl:/etc/nginx/ssl
    depends_on:
      - netbox
    restart: unless-stopped
    
  prometheus:
    image: prom/prometheus
    ports:
      - "9090:9090"
    volumes:
      - ./prometheus.yml:/etc/prometheus/prometheus.yml
    restart: unless-stopped
```

### 部署命令

```bash
# 构建镜像
docker build -t netbox:latest .

# 运行容器
docker run -d \
  --name netbox-server \
  -p 8080:8080 \
  -v $(pwd)/config:/app/config \
  -v $(pwd)/logs:/app/logs \
  netbox:latest

# 使用Docker Compose
docker-compose up -d

# 查看日志
docker logs -f netbox-server

# 进入容器
docker exec -it netbox-server /bin/bash
```

---

## 🔧 故障排除

### 常见安装问题

#### 1. 编译器版本过低
```bash
# 问题: GCC版本 < 9.0
# 解决: 升级编译器
sudo apt install gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
```

#### 2. CMake版本过低
```bash
# 问题: CMake版本 < 3.16
# 解决: 安装新版本CMake
wget https://github.com/Kitware/CMake/releases/download/v3.22.0/cmake-3.22.0-linux-x86_64.sh
chmod +x cmake-3.22.0-linux-x86_64.sh
sudo ./cmake-3.22.0-linux-x86_64.sh --prefix=/usr/local --skip-license
```

#### 3. Python依赖问题
```bash
# 问题: Jinja2未安装
# 解决: 安装Python依赖
pip3 install jinja2
# 或
sudo apt install python3-jinja2
```

### 运行时问题

#### 1. 端口占用
```bash
# 检查端口占用
netstat -tlnp | grep 8080
# 或
lsof -i :8080

# 杀死占用进程
sudo kill -9 <PID>
```

#### 2. 权限问题
```bash
# 检查文件权限
ls -la /path/to/netbox

# 修复权限
sudo chown -R $USER:$USER /path/to/netbox
chmod +x tools/netbox-cli.py
```

#### 3. 系统限制
```bash
# 检查文件描述符限制
ulimit -n

# 临时增加限制
ulimit -n 65535

# 永久修改
echo "* soft nofile 65535" | sudo tee -a /etc/security/limits.conf
echo "* hard nofile 65535" | sudo tee -a /etc/security/limits.conf
```

### 获取帮助

```bash
# 查看详细日志
netbox --verbose info

# 启用调试模式
export NETBOX_DEBUG=1
netbox build

# 查看系统信息
netbox info --system

# 检查依赖
netbox info --deps
```

---

## 📞 技术支持

- 📖 **文档**: [完整使用文档](NetBox脚手架完整使用文档.md)
- 🐛 **问题报告**: GitHub Issues
- 💬 **社区讨论**: GitHub Discussions
- 📧 **邮件支持**: support@netbox.dev

---

<div align="center">

**🎉 安装完成！开始你的NetBox开发之旅！**

```bash
netbox init MyFirstProject
```

*NetBox - 企业级网络框架，开箱即用！* 🚀✨

</div>
