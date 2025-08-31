# NetBox 模板系统 v2.0

> 简洁、自包含、渐进式的项目模板

## 设计原则

1. **简单优于复杂** - 每个模板都是最小可工作的项目
2. **自包含优于依赖** - 不依赖外部复杂框架
3. **渐进优于一次性** - 支持从简单到复杂的扩展
4. **可靠优于功能** - 确保在任何环境下都能工作

## 模板层次

### Level 1: 基础模板 (basic)
- 最小可工作的项目
- 单个源文件
- 基础CMake配置
- 简单测试

### Level 2: 标准模板 (standard)
- 模块化结构
- 多个源文件
- 完整的测试框架
- 文档和示例

### Level 3: 高级模板 (advanced)
- 插件系统
- 网络功能
- 性能优化
- 企业级特性

### Level 4: 企业模板 (enterprise)
- 完整的框架实现
- 微服务架构
- 监控和日志
- CI/CD集成

## 模板结构

```
templates_v2/
├── basic/          # Level 1: 基础模板
├── standard/       # Level 2: 标准模板
├── advanced/       # Level 3: 高级模板
├── enterprise/     # Level 4: 企业模板
└── components/     # 可复用组件
```

## 使用方式

```bash
# 创建基础项目
netbox init MyProject --template basic

# 创建标准项目
netbox init MyProject --template standard

# 创建高级项目
netbox init MyProject --template advanced

# 创建企业级项目
netbox init MyProject --template enterprise
```

## 模板特性

| 模板 | 文件数 | 复杂度 | 构建时间 | 适用场景 |
|------|--------|--------|----------|----------|
| basic | 3-5个 | 低 | <10s | 学习、原型 |
| standard | 10-15个 | 中 | <30s | 小型项目 |
| advanced | 20-30个 | 高 | <60s | 中型项目 |
| enterprise | 50+个 | 很高 | <120s | 大型项目 |

## 扩展机制

每个模板都支持通过组件扩展：

```bash
# 添加网络组件
netbox add-component network

# 添加数据库组件
netbox add-component database

# 添加Web服务组件
netbox add-component webserver
```
