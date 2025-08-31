# NetBox 模板自定义指南

## 🎨 概述

NetBox提供了强大的模板自定义系统，允许您根据具体需求定制项目模板。本指南将详细介绍如何自定义模板中的相关信息。

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

## 🛠️ 使用方法

### 1. 查看可用模板

```bash
# 列出所有可用模板
python tools/template_customizer.py list
```

### 2. 自定义项目信息

```bash
# 自定义默认模板
python tools/template_customizer.py customize default

# 自定义Web服务器模板
python tools/template_customizer.py customize web_server

# 自定义游戏服务器模板
python tools/template_customizer.py customize game_server

# 自定义微服务模板
python tools/template_customizer.py customize microservice
```

### 3. 创建自定义模板

```bash
# 创建新的自定义模板
python tools/template_customizer.py create
```

### 4. 编辑现有模板

```bash
# 编辑默认模板
python tools/template_customizer.py edit default

# 编辑Web服务器模板
python tools/template_customizer.py edit web_server
```

### 5. 导出/导入模板

```bash
# 导出模板到文件
python tools/template_customizer.py export default my_template.json

# 从文件导入模板
python tools/template_customizer.py import my_template.json
```

## 📁 模板类型详解

### 🔹 默认模板 (default)
适用于一般网络应用开发。

**默认特性:**
- logging: 日志系统
- testing: 测试框架
- examples: 示例代码
- documentation: 文档生成

**自定义变量:**
- company_name: 公司名称
- project_description: 项目描述
- contact_email: 联系邮箱
- repository_url: 仓库地址

### 🔹 Web服务器模板 (web_server)
专门用于Web服务器开发。

**默认特性:**
- http_server: HTTP服务器
- websocket: WebSocket支持
- ssl_support: SSL/TLS支持
- static_files: 静态文件服务

**自定义变量:**
- server_port: 服务器端口 (默认: 8080)
- max_connections: 最大连接数 (默认: 1000)
- enable_ssl: 启用SSL (默认: true)
- static_path: 静态文件路径 (默认: ./public)

### 🔹 游戏服务器模板 (game_server)
专门用于游戏服务器开发。

**默认特性:**
- real_time: 实时通信
- room_management: 房间管理
- player_sync: 玩家同步
- game_logic: 游戏逻辑

**自定义变量:**
- max_players_per_room: 每房间最大玩家数 (默认: 10)
- tick_rate: 游戏帧率 (默认: 60)
- enable_cheat_detection: 启用反作弊 (默认: true)
- game_type: 游戏类型 (默认: action)

### 🔹 微服务模板 (microservice)
专门用于微服务开发。

**默认特性:**
- service_discovery: 服务发现
- load_balancing: 负载均衡
- health_check: 健康检查
- metrics: 指标监控

**自定义变量:**
- service_name: 服务名称 (默认: my-service)
- service_port: 服务端口 (默认: 9090)
- enable_metrics: 启用指标监控 (默认: true)
- enable_tracing: 启用链路追踪 (默认: false)

## 🎯 自定义示例

### 示例1: 自定义公司信息

```bash
python tools/template_customizer.py customize default
```

交互式输入:
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
python tools/template_customizer.py customize web_server
```

交互式输入:
```
项目名称: MyWebServer
服务器端口: 3000
最大连接数: 5000
启用SSL?: Y
静态文件路径: ./static
```

### 示例3: 创建游戏服务器项目

```bash
python tools/template_customizer.py customize game_server
```

交互式输入:
```
项目名称: MyGameServer
每房间最大玩家数: 20
游戏帧率: 120
启用反作弊?: Y
游戏类型: rpg
```

## 📄 配置文件结构

模板配置文件位于 `tools/template_config.json`，结构如下:

```json
{
  "templates": {
    "template_id": {
      "name": "模板名称",
      "description": "模板描述",
      "author": "作者",
      "version": "版本",
      "license": "许可证",
      "features": ["特性1", "特性2"],
      "dependencies": ["依赖1", "依赖2"],
      "custom_variables": {
        "变量名": "变量值"
      }
    }
  },
  "global_settings": {
    "default_template": "default",
    "auto_generate_docs": true,
    "include_tests": true,
    "include_examples": true
  }
}
```

## 🔧 高级自定义

### 添加新的模板类型

1. 编辑 `tools/template_config.json`
2. 在 `templates` 部分添加新的模板配置
3. 在 `template_customizer.py` 中添加相应的自定义逻辑

### 自定义模板变量

1. 在模板配置的 `custom_variables` 部分添加新变量
2. 在 `customize_project_info` 方法中添加相应的用户输入逻辑

### 自定义生成的文件

1. 在 `tools/templates/` 目录下创建新的模板文件
2. 使用 Jinja2 语法进行模板变量替换
3. 在生成逻辑中引用新的模板文件

## 💡 最佳实践

### ✅ 推荐做法
- 为不同类型的项目创建专门的模板
- 使用有意义的变量名称
- 提供合理的默认值
- 添加详细的模板描述

### ❌ 避免的做法
- 在模板中使用硬编码的路径
- 创建过于复杂的模板变量
- 忽略错误处理和验证
- 不提供默认值

## 🎉 开始使用

现在您可以开始自定义您的NetBox项目模板了！

```bash
# 查看可用模板
python tools/template_customizer.py list

# 自定义默认模板
python tools/template_customizer.py customize default

# 创建新项目
netbox init MyCustomProject
```

享受自定义模板带来的便利！🚀 