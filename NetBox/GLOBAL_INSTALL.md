# NetBox CLI 全局安装指南

## 🚀 快速开始

### 安装全局命令

```bash
# 在NetBox项目根目录中运行
./install_netbox.sh
```

### 使用全局命令

```bash
# 在任何目录中创建项目
netbox init MyProject

# 进入项目目录
cd MyProject

# 构建项目
netbox build

# 运行项目
netbox run

# 运行测试
netbox test

# 清理构建文件
netbox clean

# 显示帮助
netbox --help
```

## 📋 功能特性

### ✅ 全局可用
- 安装后可在任何目录使用 `netbox` 命令
- 自动查找NetBox项目路径
- 支持多种安装位置

### 🎨 ASCII艺术字
- 启动动画：矩阵效果 + NetBox Logo
- 项目创建：渐入Logo + 进度条动画
- 构建过程：构建进度 + 成功动画
- 运行程序：启动动画 + 程序信息

### 🔧 完整功能
- `init`: 创建新的NetBox项目
- `build`: 构建项目（支持Debug/Release）
- `test`: 运行测试
- `run`: 运行程序
- `info`: 显示项目信息
- `clean`: 清理构建文件

## 🛠️ 安装详情

### 安装位置
- **普通用户**: `$HOME/.local/bin`
- **Root用户**: `/usr/local/bin`

### 自动配置
- 自动添加到PATH环境变量
- 支持bash、zsh等shell
- 自动设置执行权限

## 🗑️ 卸载

如果需要卸载全局命令：

```bash
./uninstall_netbox.sh
```

## 🔍 故障排除

### 命令未找到
```bash
# 重新加载shell配置
source ~/.bashrc

# 或者重新登录
```

### 脚本路径错误
```bash
# 确保在NetBox项目根目录中运行安装脚本
cd /path/to/NetBox
./install_netbox.sh
```

### Python依赖
```bash
# 安装Python 3.7+
sudo apt install python3

# 安装Jinja2（可选）
pip3 install jinja2
```

## 📁 项目结构

创建的项目包含：
```
MyProject/
├── src/                    # 源代码
├── include/netbox/         # 头文件
├── examples/basic/         # 示例代码
├── tests/                  # 测试文件
├── docs/                   # 文档
├── config/                 # 配置文件
├── cmake/                  # CMake模块
├── CMakeLists.txt          # 构建配置
└── README.md              # 项目说明
```

## 🎯 使用示例

### 创建Web服务器项目
```bash
netbox init WebServer
cd WebServer
netbox build
netbox run
```

### 创建游戏服务器项目
```bash
netbox init GameServer
cd GameServer
netbox build
netbox test
```

### 创建微服务项目
```bash
netbox init MicroService
cd MicroService
netbox build
netbox run
```

## 🎉 享受使用NetBox CLI!

现在您可以在任何地方使用简洁的 `netbox` 命令来创建和管理NetBox项目了！ 