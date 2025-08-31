# 🎨 NetBox 模板自定义快速指南

## 🚀 快速开始

### 1. 查看可用模板
```bash
python3 tools/template_customizer.py list
```

### 2. 自定义项目信息
```bash
# 自定义默认模板
python3 tools/template_customizer.py customize default

# 自定义Web服务器模板
python3 tools/template_customizer.py customize web_server

# 自定义游戏服务器模板
python3 tools/template_customizer.py customize game_server

# 自定义微服务模板
python3 tools/template_customizer.py customize microservice
```

### 3. 创建自定义模板
```bash
python3 tools/template_customizer.py create
```

### 4. 编辑现有模板
```bash
python3 tools/template_customizer.py edit default
```

## 📋 可自定义的信息

### 🔧 基本信息
- **项目名称**: 您的项目名称
- **作者**: 开发者或团队名称  
- **版本**: 项目版本号
- **许可证**: 项目许可证类型

### 🏢 公司信息
- **公司名称**: 您的公司或组织名称
- **联系邮箱**: 项目联系邮箱
- **仓库地址**: Git仓库URL

### 📝 项目描述
- **项目描述**: 项目的详细描述
- **关键词**: 项目相关的关键词
- **目标平台**: 支持的操作系统

### ⚙️ 技术特性
- **功能特性**: 项目支持的功能列表
- **依赖项**: 项目依赖的库和工具
- **自定义变量**: 模板特定的配置参数

## 🎯 使用示例

### 示例1: 自定义公司信息
```bash
python3 tools/template_customizer.py customize default
```

输入示例:
```
项目名称: MyAwesomeProject
作者: 张三
版本: 1.0.0
许可证: MIT
公司名称: 我的公司
联系邮箱: zhangsan@mycompany.com
项目描述: 基于NetBox框架的高性能网络应用
仓库地址: https://github.com/mycompany/myproject
```

### 示例2: 创建Web服务器项目
```bash
python3 tools/template_customizer.py customize web_server
```

输入示例:
```
项目名称: MyWebServer
服务器端口: 3000
最大连接数: 5000
启用SSL?: Y
静态文件路径: ./static
```

### 示例3: 创建游戏服务器项目
```bash
python3 tools/template_customizer.py customize game_server
```

输入示例:
```
项目名称: MyGameServer
每房间最大玩家数: 20
游戏帧率: 120
启用反作弊?: Y
游戏类型: rpg
```

## 📁 模板类型

### 🔹 默认模板 (default)
适用于一般网络应用开发
- 特性: logging, testing, examples, documentation

### 🔹 Web服务器模板 (web_server)
专门用于Web服务器开发
- 特性: http_server, websocket, ssl_support, static_files
- 自定义变量: server_port, max_connections, enable_ssl, static_path

### 🔹 游戏服务器模板 (game_server)
专门用于游戏服务器开发
- 特性: real_time, room_management, player_sync, game_logic
- 自定义变量: max_players_per_room, tick_rate, enable_cheat_detection, game_type

### 🔹 微服务模板 (microservice)
专门用于微服务开发
- 特性: service_discovery, load_balancing, health_check, metrics
- 自定义变量: service_name, service_port, enable_metrics, enable_tracing

## 🎉 开始使用

1. **查看模板**: `python3 tools/template_customizer.py list`
2. **自定义模板**: `python3 tools/template_customizer.py customize default`
3. **创建项目**: `netbox init MyCustomProject`

享受自定义模板带来的便利！🚀 