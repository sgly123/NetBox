# AnimatedProject

> 专门用于Web服务器开发的模板

## 快速开始

### 构建项目

```bash
mkdir build && cd build
cmake ..
make
```

### 运行程序

```bash
# 运行主程序
./bin/AnimatedProject

# 运行示例程序
./bin/AnimatedProject_example

# 运行测试
./bin/AnimatedProject_test
# 或使用ctest
ctest
```

## 项目结构

```
AnimatedProject/
├── src/                    # 源代码
│   └── main.cpp           # 主程序
├── include/netbox/        # 头文件
│   └── NetBox.h          # 框架头文件
├── examples/basic/        # 示例代码
│   └── simple_example.cpp # 简单示例
├── tests/                 # 测试代码
│   └── simple_test.cpp   # 基础测试
├── docs/                  # 文档
├── config/               # 配置文件
└── CMakeLists.txt        # 构建配置
```

## 模板信息

- **模板名称**: MyCustomWebServer
- **模板描述**: 专门用于Web服务器开发的模板

### 包含特性

- http_server
- websocket
- ssl_support
- static_files

## 开发指南

### 添加新功能

1. 在 `src/` 目录下添加源文件
2. 在 `include/netbox/` 目录下添加头文件
3. 更新 `CMakeLists.txt`
4. 编写测试用例

### 扩展应用

继承 `NetBox::Application` 类来创建自定义应用：

```cpp
class MyApp : public NetBox::Application {
public:
    MyApp() : NetBox::Application("MyApp") {}

    bool initialize() override {
        // 初始化逻辑
        return true;
    }

    bool start() override {
        // 启动逻辑
        return true;
    }
};
```

## 构建选项

```bash
# Debug构建
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release构建
cmake .. -DCMAKE_BUILD_TYPE=Release

# 并行构建
make -j$(nproc)
```

## 版本信息

- 项目版本: 1.0.0
- NetBox版本: 1.0.0
- C++标准: C++17

---

*由增强版NetBox CLI生成*
