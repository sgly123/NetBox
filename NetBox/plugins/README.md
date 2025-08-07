# NetBox 插件目录

## 📁 目录结构

```
plugins/
├── README.md                    # 插件目录说明
├── echo/                        # Echo服务器插件
│   └── EchoServerPlugin.cpp     # Echo插件注册实现
├── sanguosha/                   # 三国杀服务器插件（未来实现）
│   └── SanguoshaPlugin.cpp      # 三国杀插件注册模板
└── redis/                       # Mini Redis服务器插件（未来实现）
    └── RedisPlugin.cpp          # Redis插件注册模板
```

## 🎯 设计原则

### **职责分离**
- **业务逻辑**: 放在 `app/` 目录，专注于功能实现
- **插件注册**: 放在 `plugins/` 目录，专注于框架集成
- **框架核心**: 放在 `NetFramework/` 目录，提供基础设施

### **目录组织**
- 每个应用类型有独立的插件目录
- 插件目录名与配置文件中的 `application.type` 对应
- 便于管理和扩展新的应用类型

## 🚀 添加新插件的步骤

### 1. 创建插件目录
```bash
mkdir plugins/your_app
```

### 2. 创建插件注册文件
```cpp
// plugins/your_app/YourAppPlugin.cpp
#include "app/ApplicationRegistry.h"
#include "YourAppServer.h"
#include "base/Logger.h"

static bool registerYourAppServer() {
    Logger::info("正在注册YourAppServer插件...");
    
    bool success = ApplicationRegistry::getInstance().registerApplication("your_app", 
        [](const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool) {
            return std::make_unique<YourAppServer>(ip, port, io_type, pool);
        });
    
    return success;
}

static bool g_yourAppServerRegistered = registerYourAppServer();
```

### 3. 更新CMakeLists.txt
```cmake
# 应用插件
app/src/EchoServerPlugin.cpp
plugins/echo/EchoServerPlugin.cpp
plugins/your_app/YourAppPlugin.cpp  # 添加新插件
```

### 4. 创建配置文件
```yaml
# config_your_app.yaml
application:
  type: your_app
network:
  ip: 127.0.0.1
  port: 8888
```

### 5. 测试插件
```bash
./NetBox config_your_app.yaml
```

## 📋 当前插件状态

| 插件名称 | 状态 | 配置类型 | 端口 | 描述 |
|---------|------|----------|------|------|
| **echo** | ✅ 已实现 | `echo` | 8888 | TCP回显服务 |
| **sanguosha** | 🚧 模板就绪 | `sanguosha` | 8888 | 三国杀游戏服务器 |
| **mini_redis** | 🚧 模板就绪 | `mini_redis` | 6379 | Redis数据库服务 |

## 🎯 插件开发规范

### **命名规范**
- 插件目录: 小写字母 + 下划线 (如: `mini_redis`)
- 插件文件: 大驼峰 + Plugin.cpp (如: `MiniRedisPlugin.cpp`)
- 注册函数: `register` + 类名 (如: `registerMiniRedisServer`)

### **代码规范**
- 包含详细的注释说明
- 提供插件信息函数
- 使用统一的日志格式
- 遵循错误处理规范

### **配置规范**
- 配置类型与插件目录名一致
- 支持应用特定的配置节
- 提供合理的默认值
- 包含配置示例和说明

## 🔧 维护指南

### **添加新插件时**
1. 在对应目录创建插件文件
2. 更新 CMakeLists.txt
3. 创建配置文件模板
4. 更新此 README 文档

### **删除插件时**
1. 删除插件目录
2. 更新 CMakeLists.txt
3. 删除相关配置文件
4. 更新此 README 文档

### **修改插件时**
1. 保持接口兼容性
2. 更新相关文档
3. 测试所有配置组合
4. 确保向下兼容

## 🎉 插件化的优势

- ✅ **模块化**: 每个应用独立开发和维护
- ✅ **可扩展**: 轻松添加新的应用类型
- ✅ **配置驱动**: 无需重编译即可切换应用
- ✅ **职责分离**: 业务逻辑与框架集成分离
- ✅ **易于测试**: 每个插件可独立测试
- ✅ **团队协作**: 不同团队可并行开发不同插件
