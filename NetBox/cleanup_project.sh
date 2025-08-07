#!/bin/bash

# NetBox 项目文件清理脚本
# 用于整理项目文件，移除不必要的文件，保持项目结构清晰

echo "🧹 NetBox 项目文件清理工具"
echo "================================"

# 创建必要的目录结构
echo "📁 创建标准目录结构..."
mkdir -p docs
mkdir -p examples
mkdir -p logs
mkdir -p config

# 移动配置文件到config目录
echo "📋 整理配置文件..."
if [ -f "config_echo.yaml" ]; then
    mv config_echo.yaml config/
    echo "  ✅ config_echo.yaml -> config/"
fi

if [ -f "config_redis_app.yaml" ]; then
    mv config_redis_app.yaml config/
    echo "  ✅ config_redis_app.yaml -> config/"
fi

# 移动文档到docs目录
echo "📚 整理文档文件..."
if [ -f "README.md" ]; then
    cp README.md docs/
    echo "  ✅ README.md -> docs/"
fi

if [ -f "PROJECT_STRUCTURE.md" ]; then
    mv PROJECT_STRUCTURE.md docs/
    echo "  ✅ PROJECT_STRUCTURE.md -> docs/"
fi

# 移动示例代码到examples目录
echo "🎯 整理示例代码..."
if [ -f "smart_netbox_redis_client.cpp" ]; then
    mv smart_netbox_redis_client.cpp examples/
    echo "  ✅ smart_netbox_redis_client.cpp -> examples/"
fi

if [ -f "redis_integration_test.cpp" ]; then
    mv redis_integration_test.cpp examples/
    echo "  ✅ redis_integration_test.cpp -> examples/"
fi

# 清理build目录中的临时文件
echo "🗑️  清理编译临时文件..."
cd build 2>/dev/null || { echo "  ⚠️  build目录不存在"; }
if [ -d "../build" ]; then
    cd build
    # 保留可执行文件，清理对象文件
    find . -name "*.o" -delete 2>/dev/null
    find . -name "*.d" -delete 2>/dev/null
    find . -name "*.tmp" -delete 2>/dev/null
    echo "  ✅ 已清理编译临时文件"
    cd ..
fi

# 创建项目文件清单
echo "📝 生成项目文件清单..."
cat > PROJECT_FILES.md << 'EOF'
# NetBox 项目文件清单

## 🏗️ 核心框架文件

### 网络层 (NetFramework/)
- `include/server/TcpServer.h` - TCP服务器头文件
- `src/server/TcpServer.cpp` - TCP服务器实现
- `include/IO/IOMultiplexer.h` - IO多路复用接口
- `src/IO/EpollMultiplexer.cpp` - epoll实现

### 协议层 (Protocol/)
- `include/ProtocolRouter.h` - 协议路由器
- `src/ProtocolRouter.cpp` - 协议路由实现
- `include/PureRedisProtocol.h` - Redis RESP协议
- `src/PureRedisProtocol.cpp` - Redis协议实现

### 应用层 (app/)
- `include/ApplicationServer.h` - 应用服务器基类
- `src/ApplicationServer.cpp` - 应用服务器实现
- `include/RedisApplicationServer.h` - Redis应用服务器
- `src/RedisApplicationServer.cpp` - Redis应用实现

## 🎯 使用场景文件

### Echo场景
- `config/config_echo.yaml` - Echo服务器配置
- `build/echo_client` - Echo客户端程序

### Redis场景  
- `config/config_redis_app.yaml` - Redis服务器配置
- `examples/smart_netbox_redis_client.cpp` - 智能Redis客户端源码
- `build/smart_netbox_redis_client` - 智能Redis客户端程序

## 📚 文档和示例

### 文档
- `README.md` - 项目主文档
- `docs/PROJECT_STRUCTURE.md` - 项目结构说明

### 示例代码
- `examples/smart_netbox_redis_client.cpp` - Redis客户端示例
- `examples/redis_integration_test.cpp` - Redis集成测试

## 🔧 构建和配置

### 构建文件
- `CMakeLists.txt` - CMake构建配置
- `build/Makefile` - Make构建文件
- `build/NetBox` - 主程序

### 配置文件
- `config/config_echo.yaml` - Echo场景配置
- `config/config_redis_app.yaml` - Redis场景配置

## 🧹 维护文件
- `cleanup_project.sh` - 项目清理脚本
- `PROJECT_FILES.md` - 本文件清单
EOF

echo "  ✅ 已生成 PROJECT_FILES.md"

# 显示最终的项目结构
echo ""
echo "🎉 项目清理完成！"
echo "================================"
echo "📁 当前项目结构:"
echo ""
tree -I 'build|*.o|*.d|*.tmp' -L 3 2>/dev/null || {
    echo "NetBox/"
    echo "├── 📁 NetFramework/     # 核心网络框架"
    echo "├── 📁 Protocol/         # 协议层"
    echo "├── 📁 app/              # 应用层"
    echo "├── 📁 config/           # 配置文件"
    echo "├── 📁 docs/             # 文档"
    echo "├── 📁 examples/         # 示例代码"
    echo "├── 📁 build/            # 编译输出"
    echo "├── 📄 README.md         # 主文档"
    echo "├── 📄 CMakeLists.txt    # 构建配置"
    echo "└── 📄 main.cpp          # 主程序入口"
}

echo ""
echo "🚀 使用方法:"
echo "  Redis场景: ./build/NetBox config/config_redis_app.yaml"
echo "  Echo场景:  ./build/NetBox config/config_echo.yaml"
echo ""
echo "📖 查看文档: cat README.md"
echo "🧪 运行测试: ./build/smart_netbox_redis_client --port 6380"
