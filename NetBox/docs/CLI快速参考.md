# 🚀 NetBox CLI 快速参考

## 📋 **常用命令速查**

### **🎯 项目管理**
```bash
# 创建新项目
netbox init [项目名]

# 创建Hello World示例
netbox hello <项目名>

# 查看项目信息
netbox info
```

### **🎨 动画和进度条**
```bash
# 启动动画
netbox animation startup

# 项目创建动画
netbox animation create --project "MyProject"

# 进度条测试
netbox animation progress --style <样式>
netbox animation progress --multi
```

### **🔨 构建和测试**
```bash
# 构建项目
netbox build [--type Debug|Release]

# 运行测试
netbox test [--verbose]

# 性能测试
netbox benchmark

# 清理构建
netbox clean
```

### **🎪 演示和帮助**
```bash
# 运行演示
netbox demo

# 查看帮助
netbox --help
netbox <命令> --help
```

---

## 🎨 **进度条样式速查**

### **静态样式**
| 样式 | 效果 | 命令 |
|------|------|------|
| blocks | `████████░░░░` | `--style blocks` |
| gradient | `████████░░░░` | `--style gradient` |
| dots | `●●●●●○○○` | `--style dots` |
| arrows | `▶▶▶▶▶▷▷▷` | `--style arrows` |
| squares | `■■■■■□□□` | `--style squares` |
| circles | `⬤⬤⬤⬤⬤⬜⬜⬜` | `--style circles` |

### **动态效果**
| 效果 | 描述 | 命令 |
|------|------|------|
| wave | 波浪动画 | `--style wave` |
| pulse | 脉冲闪烁 | `--style pulse` |
| rainbow | 彩虹变色 | `--style rainbow` |

---

## 🔧 **项目结构速查**

```
MyProject/
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

---

## ⚡ **快速开始**

```bash
# 1. 创建项目
python3 tools/netbox-cli.py init MyFramework

# 2. 进入目录
cd MyFramework

# 3. 构建项目
python3 ../tools/netbox-cli.py build

# 4. 运行示例
cd examples && g++ -o hello hello_world.cpp && ./hello
```

---

## 🆘 **故障排除速查**

| 问题 | 解决方案 |
|------|----------|
| 动画不显示 | 检查 `tools/ascii_art.py` 文件 |
| 模板失败 | 安装 Jinja2: `pip3 install jinja2` |
| 构建失败 | 检查 CMake 3.16+ 和 GCC 7+ |
| 权限错误 | `chmod +x tools/netbox-cli.py` |

---

## 📞 **获取帮助**

- 📖 **完整文档**: `docs/CLI使用指南.md`
- 🔧 **开发指南**: `docs/开发指南.md`
- 💡 **命令帮助**: `netbox --help`
- 🎨 **动画展示**: `python3 tools/progress_showcase.py`

---

*保存此文件作为快速参考！* 📌
