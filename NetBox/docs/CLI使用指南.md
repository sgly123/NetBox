# 🚀 NetBox CLI 使用指南

NetBox CLI是一个功能强大的命令行工具，用于创建和管理基于NetBox框架的项目。它提供了Spring Boot级别的脚手架能力，支持协议层、应用层、网络层的二次开发。

## 📋 **目录**

- [安装和环境要求](#安装和环境要求)
- [基础命令](#基础命令)
- [项目管理](#项目管理)
- [动画和进度条](#动画和进度条)
- [构建和测试](#构建和测试)
- [高级功能](#高级功能)
- [故障排除](#故障排除)
- [示例和最佳实践](#示例和最佳实践)

---

## 🔧 **安装和环境要求**

### **系统要求**
- **操作系统**: Linux, macOS, Windows
- **Python**: 3.7+
- **CMake**: 3.16+
- **编译器**: GCC 7+, Clang 10+, MSVC 2019+

### **依赖安装**
```bash
# Ubuntu/Debian
sudo apt install python3 python3-pip cmake build-essential

# 可选：安装Jinja2模板引擎（推荐）
sudo apt install python3-jinja2
# 或者
pip3 install jinja2

# macOS
brew install python cmake

# Windows
# 安装Visual Studio 2019+和CMake
```

### **验证安装**
```bash
cd NetBox
python3 tools/netbox-cli.py --version
```

---

## 🚀 **基础命令**

### **查看帮助**
```bash
# 显示主帮助（带启动动画）
python3 tools/netbox-cli.py --help

# 显示特定命令帮助
python3 tools/netbox-cli.py init --help
python3 tools/netbox-cli.py build --help
```

### **版本信息**
```bash
# 查看CLI版本
python3 tools/netbox-cli.py --version

# 查看详细信息
python3 tools/netbox-cli.py info
```

---

## 📦 **项目管理**

### **创建新项目**

#### **基础用法**
```bash
# 创建默认项目
python3 tools/netbox-cli.py init

# 创建指定名称的项目
python3 tools/netbox-cli.py init MyFramework
```

#### **项目特性**
生成的项目包含：
- ✅ **完整的框架结构** - 支持协议、应用、网络层扩展
- ✅ **Jinja2模板系统** - 专业的代码生成
- ✅ **示例代码** - 基础和高级示例
- ✅ **构建配置** - CMake配置文件
- ✅ **开发文档** - 详细的开发指南
- ✅ **测试框架** - 单元测试和集成测试

#### **项目结构**
```
MyFramework/
├── src/                    # 框架源码
│   ├── core/              # 核心框架
│   ├── network/           # 网络层
│   ├── protocol/          # 协议层
│   ├── application/       # 应用层
│   └── plugins/           # 插件系统
├── include/netbox/        # 框架头文件
├── protocols/             # 协议扩展
├── applications/          # 应用扩展
├── examples/              # 示例代码
├── tests/                 # 测试用例
├── docs/                  # 开发文档
└── CMakeLists.txt         # 构建配置
```

### **Hello World示例**
```bash
# 为现有项目创建Hello World示例
python3 tools/netbox-cli.py hello MyFramework

# 编译运行
cd MyFramework/examples
g++ -o hello_world hello_world.cpp
./hello_world
```

---

## 🎨 **动画和进度条**

NetBox CLI提供了丰富的视觉效果，让开发体验更加愉悦。

### **动画测试**

#### **启动动画**
```bash
# 测试启动动画（矩阵雨 + NetBox Logo）
python3 tools/netbox-cli.py animation startup
```

#### **项目创建动画**
```bash
# 测试项目创建动画
python3 tools/netbox-cli.py animation create --project "TestProject"
```

### **进度条测试**

#### **静态进度条样式**
```bash
# 方块风格（默认）
python3 tools/netbox-cli.py animation progress --style blocks

# 渐变风格
python3 tools/netbox-cli.py animation progress --style gradient

# 圆点风格
python3 tools/netbox-cli.py animation progress --style dots

# 箭头风格
python3 tools/netbox-cli.py animation progress --style arrows

# 方形风格
python3 tools/netbox-cli.py animation progress --style squares

# 圆形风格
python3 tools/netbox-cli.py animation progress --style circles
```

#### **动态进度条效果**
```bash
# 波浪效果
python3 tools/netbox-cli.py animation progress --style wave

# 脉冲效果
python3 tools/netbox-cli.py animation progress --style pulse

# 彩虹效果
python3 tools/netbox-cli.py animation progress --style rainbow
```

#### **多任务进度条**
```bash
# 显示多个任务的并行进度
python3 tools/netbox-cli.py animation progress --multi
```

#### **完整进度条展示**
```bash
# 运行完整的进度条展示脚本
python3 tools/progress_showcase.py

# 分类展示
python3 tools/progress_showcase.py static   # 静态样式
python3 tools/progress_showcase.py dynamic  # 动态效果
python3 tools/progress_showcase.py multi    # 多任务
```

---

## 🔨 **构建和测试**

### **项目构建**
```bash
# 构建项目
python3 tools/netbox-cli.py build

# Debug构建
python3 tools/netbox-cli.py build --type Debug

# Release构建
python3 tools/netbox-cli.py build --type Release

# 并行构建
python3 tools/netbox-cli.py build --jobs 8
```

### **运行测试**
```bash
# 运行所有测试
python3 tools/netbox-cli.py test

# 运行特定测试
python3 tools/netbox-cli.py test --filter "NetworkTest*"

# 详细测试输出
python3 tools/netbox-cli.py test --verbose
```

### **性能测试**
```bash
# 运行性能基准测试
python3 tools/netbox-cli.py benchmark

# 网络性能测试
python3 tools/netbox-cli.py benchmark --type network

# 内存性能测试
python3 tools/netbox-cli.py benchmark --type memory
```

### **清理构建**
```bash
# 清理构建文件
python3 tools/netbox-cli.py clean

# 深度清理（包括依赖）
python3 tools/netbox-cli.py clean --deep
```

---

## 🎯 **高级功能**

### **项目信息**
```bash
# 查看项目详细信息
python3 tools/netbox-cli.py info

# 查看系统信息
python3 tools/netbox-cli.py info --system

# 查看依赖信息
python3 tools/netbox-cli.py info --deps
```

### **演示程序**
```bash
# 运行内置演示
python3 tools/netbox-cli.py demo

# 运行网络演示
python3 tools/netbox-cli.py demo --type network

# 运行性能演示
python3 tools/netbox-cli.py demo --type performance
```

---

## 🎨 **命令参数详解**

### **init 命令**
```bash
python3 tools/netbox-cli.py init [项目名称] [选项]
```

**参数：**
- `项目名称` - 可选，默认为 "MyNetBoxFramework"

**特性：**
- 自动显示项目创建动画
- 生成完整的框架项目结构
- 包含示例代码和开发文档
- 支持Jinja2模板渲染

### **animation 命令**
```bash
python3 tools/netbox-cli.py animation <类型> [选项]
```

**类型：**
- `startup` - 启动动画
- `create` - 项目创建动画
- `progress` - 进度条测试

**选项：**
- `--project <名称>` - 项目名称（用于create动画）
- `--style <样式>` - 进度条样式
- `--multi` - 多进度条模式

**进度条样式：**
- `blocks` - 方块风格 ████████░░░░
- `dots` - 圆点风格 ●●●●●○○○
- `arrows` - 箭头风格 ▶▶▶▶▶▷▷▷
- `squares` - 方形风格 ■■■■■□□□
- `circles` - 圆形风格 ⬤⬤⬤⬤⬤⬜⬜⬜
- `gradient` - 渐变风格 ████████░░░░
- `wave` - 波浪效果 ▂▁▁▂▃▃▃▃
- `pulse` - 脉冲效果（闪烁）
- `rainbow` - 彩虹效果（变色）

### **hello 命令**
```bash
python3 tools/netbox-cli.py hello <项目名称>
```

**功能：**
- 为现有项目创建Hello World示例
- 展示Jinja2模板功能
- 生成可编译运行的C++代码

### **build 命令**
```bash
python3 tools/netbox-cli.py build [选项]
```

**选项：**
- `--type <类型>` - 构建类型（Debug/Release）
- `--jobs <数量>` - 并行构建任务数
- `--clean` - 构建前清理
- `--verbose` - 详细输出

### **test 命令**
```bash
python3 tools/netbox-cli.py test [选项]
```

**选项：**
- `--filter <模式>` - 测试过滤器
- `--verbose` - 详细输出
- `--parallel` - 并行测试

### **benchmark 命令**
```bash
python3 tools/netbox-cli.py benchmark [选项]
```

**选项：**
- `--type <类型>` - 基准测试类型
- `--iterations <次数>` - 测试迭代次数
- `--output <文件>` - 结果输出文件

---

## 🔍 **故障排除**

### **常见问题**

#### **1. 动画不显示**
```bash
# 检查是否安装了ascii_art模块
ls tools/ascii_art.py

# 如果缺失，动画会自动回退到文本模式
⚠️  ASCII动画模块不可用
💡 请确保 ascii_art.py 文件存在
```

#### **2. 模板渲染失败**
```bash
# 检查Jinja2安装
python3 -c "import jinja2; print('Jinja2可用')"

# 安装Jinja2
sudo apt install python3-jinja2
# 或
pip3 install jinja2
```

#### **3. 构建失败**
```bash
# 检查CMake版本
cmake --version  # 需要3.16+

# 检查编译器
gcc --version    # 需要GCC 7+
clang --version  # 需要Clang 10+
```

#### **4. 权限问题**
```bash
# 确保脚本有执行权限
chmod +x tools/netbox-cli.py

# 使用python3明确调用
python3 tools/netbox-cli.py --help
```

### **调试模式**
```bash
# 启用详细输出
python3 tools/netbox-cli.py --verbose <命令>

# 启用调试模式
export NETBOX_DEBUG=1
python3 tools/netbox-cli.py <命令>
```

---

## 📚 **示例和最佳实践**

### **完整工作流程**

#### **1. 创建新项目**
```bash
# 创建项目（自动显示动画）
python3 tools/netbox-cli.py init MyNetworkApp

# 进入项目目录
cd MyNetworkApp

# 查看项目结构
ls -la
```

#### **2. 查看开发指南**
```bash
# 阅读开发指南
cat docs/开发指南.md

# 查看示例代码
ls examples/
cat examples/basic/simple_server.cpp
```

#### **3. 构建和测试**
```bash
# 构建项目
python3 ../tools/netbox-cli.py build

# 运行测试
python3 ../tools/netbox-cli.py test

# 运行示例
cd examples
g++ -o simple_server basic/simple_server.cpp
./simple_server
```

#### **4. 二次开发**
```bash
# 创建自定义协议
mkdir protocols/my_protocol
# 编写协议实现...

# 创建自定义应用
mkdir applications/my_app
# 编写应用逻辑...

# 重新构建
python3 ../tools/netbox-cli.py build
```

### **最佳实践**

#### **1. 项目命名**
- 使用PascalCase：`MyWebFramework`
- 避免特殊字符和空格
- 保持名称简洁有意义

#### **2. 开发流程**
1. 先阅读生成的开发指南
2. 从示例代码开始学习
3. 逐步扩展协议和应用层
4. 定期运行测试确保质量

#### **3. 性能优化**
```bash
# 定期运行性能测试
python3 tools/netbox-cli.py benchmark

# 使用Release构建进行性能测试
python3 tools/netbox-cli.py build --type Release
```

#### **4. 版本管理**
```bash
# 初始化git仓库
cd MyProject
git init
git add .
git commit -m "Initial NetBox framework project"
```

---

## 🎉 **总结**

NetBox CLI提供了完整的项目管理和开发体验：

- 🚀 **一键项目创建** - Spring Boot级别的脚手架能力
- 🎨 **丰富的视觉效果** - ASCII动画和多样化进度条
- 🔧 **完整的开发工具链** - 构建、测试、基准测试
- 📖 **详细的文档支持** - 开发指南和API参考
- 🎯 **二次开发友好** - 支持协议、应用、网络层扩展

**开始你的NetBox开发之旅：**
```bash
python3 tools/netbox-cli.py init MyAwesomeProject
```

**获取帮助：**
- 📖 查看开发指南：`docs/开发指南.md`
- 💬 运行帮助命令：`python3 tools/netbox-cli.py --help`
- 🎨 体验动画效果：`python3 tools/netbox-cli.py animation startup`

---

*NetBox CLI - 让网络框架开发变得简单而有趣！* 🚀✨
