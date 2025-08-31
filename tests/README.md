# NetBox 单元测试

本目录包含NetBox项目的完整单元测试套件，使用Google Test框架实现。

## 快速开始

### 1. 安装依赖
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libgtest-dev libspdlog-dev libfmt-dev libyaml-cpp-dev

# 或使用项目提供的脚本
make -f Makefile.tests install-deps
```

### 2. 构建和运行测试
```bash
# 检查环境
make -f Makefile.tests check-env

# 构建测试
make -f Makefile.tests build-tests

# 运行所有测试
make -f Makefile.tests test
```

## 测试结构

```
tests/
├── CMakeLists.txt              # 测试构建配置
├── README.md                   # 本文件
├── run_tests.sh               # 测试执行脚本
├── include/
│   └── test_utils.h           # 测试工具类
├── unit/                      # 单元测试
│   ├── base/                  # 基础组件测试
│   │   ├── test_logger.cpp
│   │   ├── test_async_logger.cpp
│   │   ├── test_thread_pool.cpp
│   │   └── test_double_lock_thread_pool.cpp
│   ├── util/                  # 工具类测试
│   │   ├── test_config_reader.cpp
│   │   └── test_enhanced_config_reader.cpp
│   ├── io/                    # IO多路复用测试
│   │   ├── test_io_multiplexer.cpp
│   │   ├── test_epoll_multiplexer.cpp
│   │   ├── test_select_multiplexer.cpp
│   │   └── test_poll_multiplexer.cpp
│   ├── protocol/              # 协议层测试
│   ├── server/                # 服务器组件测试
│   └── app/                   # 应用层测试
├── integration/               # 集成测试
├── performance/               # 性能测试
└── data/                      # 测试数据
```

## 测试套件

### 基础组件测试 (test_base)
测试NetFramework/base下的核心组件：
- **Logger**: 基础日志系统
- **AsyncLogger**: 异步日志系统
- **ThreadPool**: 线程池实现
- **DoubleLockThreadPool**: 双锁线程池实现

```bash
make -f Makefile.tests test-base
```

### 工具类测试 (test_util)
测试NetFramework/util下的工具类：
- **ConfigReader**: 配置文件读取器
- **EnhancedConfigReader**: 增强配置读取器

```bash
make -f Makefile.tests test-util
```

### IO多路复用测试 (test_io)
测试NetFramework/IO下的IO多路复用实现：
- **IOMultiplexer**: IO多路复用基类
- **EpollMultiplexer**: Linux epoll实现
- **SelectMultiplexer**: POSIX select实现
- **PollMultiplexer**: POSIX poll实现

```bash
make -f Makefile.tests test-io
```

### 协议层测试 (test_protocol)
测试Protocol层的协议实现：
- **ProtocolBase**: 协议基类
- **ProtocolRouter**: 协议路由器
- **SimpleHeaderProtocol**: 简单头协议
- **HttpProtocol**: HTTP协议
- **RedisProtocol**: Redis协议

```bash
make -f Makefile.tests test-protocol
```

### 服务器组件测试 (test_server)
测试NetFramework/server下的服务器组件：
- **TcpServer**: TCP服务器

```bash
make -f Makefile.tests test-server
```

### 应用层测试 (test_app)
测试app层的应用服务器：
- **ApplicationServer**: 应用服务器基类
- **EchoServer**: Echo服务器
- **RedisApplicationServer**: Redis应用服务器
- **ApplicationRegistry**: 应用注册表

```bash
make -f Makefile.tests test-app
```

### 集成测试 (test_integration)
测试组件间的集成：
- **EchoIntegration**: Echo服务集成测试
- **RedisIntegration**: Redis服务集成测试
- **ProtocolIntegration**: 协议集成测试

```bash
make -f Makefile.tests test-integration
```

### 性能测试 (test_performance)
性能和基准测试：
- **ThreadPoolPerformance**: 线程池性能测试
- **IOPerformance**: IO性能测试
- **ProtocolPerformance**: 协议性能测试

```bash
make -f Makefile.tests test-performance
```

## 测试工具

### TestUtils类
提供丰富的测试辅助功能：
- 临时文件创建和删除
- 随机数据生成
- 执行时间测量
- 条件等待
- 多线程测试支持

### 性能测试基类
`PerformanceTestBase`提供性能测试的基础设施：
- 自动时间测量
- 性能指标报告
- 基准比较

### 多线程测试基类
`MultiThreadTestBase`提供多线程测试支持：
- 多线程任务执行
- 异常处理
- 同步控制

## 常用命令

### 基本测试命令
```bash
# 运行所有测试
make -f Makefile.tests test

# 运行特定测试套件
make -f Makefile.tests test-base
make -f Makefile.tests test-util
make -f Makefile.tests test-io

# 快速测试(跳过性能和多线程测试)
make -f Makefile.tests quick-test
```

### 高级测试命令
```bash
# 生成测试报告
make -f Makefile.tests test-report

# 生成代码覆盖率报告
make -f Makefile.tests test-coverage

# 运行内存检查
make -f Makefile.tests test-valgrind

# 性能基准测试
make -f Makefile.tests benchmark
```

### 调试和开发
```bash
# 调试模式测试
make -f Makefile.tests debug-test

# 发布模式测试
make -f Makefile.tests release-test

# 清理构建文件
make -f Makefile.tests clean-tests
```

## 测试结果解读

### 成功输出示例
```
======================================
    NetBox 单元测试执行报告
======================================

运行 基础组件测试 (test_base)...
✓ 基础组件测试: 44 个测试通过

运行 工具类测试 (test_util)...
✓ 工具类测试: 25 个测试通过

======================================
           测试结果总结
======================================
总测试数: 132
通过测试: 132
失败测试: 0
成功率: 100%
```

### 失败输出示例
```
运行 基础组件测试 (test_base)...
✗ 基础组件测试: 41 个测试通过, 3 个测试失败
  失败的测试:
    [  FAILED  ] ThreadPoolTest.TaskQueueLimit
    [  FAILED  ] ThreadPoolTest.Destruction
    [  FAILED  ] DoubleLockThreadPoolTest.DestructionWithPendingTasks
```

## 编写新测试

### 1. 创建测试文件
```cpp
#include <gtest/gtest.h>
#include "test_utils.h"
#include "your_class.h"

class YourClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的初始化
    }

    void TearDown() override {
        // 测试后的清理
    }
};

TEST_F(YourClassTest, ShouldDoSomething) {
    // Arrange
    YourClass obj;
    
    // Act
    auto result = obj.doSomething();
    
    // Assert
    EXPECT_TRUE(result);
}
```

### 2. 更新CMakeLists.txt
将新的测试文件添加到相应的测试目标中。

### 3. 运行测试
```bash
make -f Makefile.tests build-tests
./build/tests/bin/your_test
```

## 持续集成

项目包含GitHub Actions配置(`.github/workflows/tests.yml`)，会在以下情况自动运行测试：
- 推送到main或develop分支
- 创建Pull Request

CI会运行：
- 多编译器测试(GCC, Clang)
- 多构建类型测试(Debug, Release)
- 内存检查(Valgrind)
- 代码覆盖率分析

## 故障排除

### 常见问题

1. **Google Test未找到**
   ```bash
   # 安装Google Test
   sudo apt-get install libgtest-dev
   cd /usr/src/googletest
   sudo cmake .
   sudo make
   sudo make install
   ```

2. **依赖库缺失**
   ```bash
   # 安装所有依赖
   make -f Makefile.tests install-deps
   ```

3. **测试超时**
   - 检查系统资源使用情况
   - 调整测试中的超时参数
   - 使用`quick-test`跳过耗时测试

4. **内存泄漏**
   ```bash
   # 运行内存检查
   make -f Makefile.tests test-valgrind
   ```

### 获取帮助
```bash
# 查看所有可用命令
make -f Makefile.tests help

# 检查测试环境
make -f Makefile.tests check-env
```

## 贡献指南

1. 新功能必须包含相应的单元测试
2. 测试覆盖率应保持在90%以上
3. 所有测试必须通过才能合并代码
4. 遵循测试命名规范和最佳实践

更多详细信息请参考：
- [单元测试报告](../docs/测试指南/单元测试报告.md)
- [测试最佳实践](../docs/测试指南/测试最佳实践.md)
