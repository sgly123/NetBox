#pragma once

/**
 * @file NetBox.h
 * @brief SimpleEchoServer - NetBox框架主头文件
 * 
 * 这是一个支持二次开发的完整网络框架，提供：
 * 1. 协议层扩展接口 - 自定义协议开发
 * 2. 应用层扩展接口 - 多场景应用开发
 * 3. 网络层优化接口 - 性能调优和扩展
 * 4. 插件化架构 - 模块化功能扩展
 * 
 * @author NetBox Developer
 * @version 1.0.0
 * @date 2025-08-03
 */

// 核心框架
#include "core/Framework.h"
#include "core/EventLoop.h"
#include "core/ThreadPool.h"

// 网络层
#include "network/Connection.h"
#include "network/Server.h"
#include "network/Client.h"
#include "network/Transport.h"

// 协议层
#include "protocol/Protocol.h"
#include "protocol/Codec.h"
#include "protocol/Message.h"

// 应用层
#include "application/Application.h"
#include "application/Handler.h"
#include "application/Context.h"

// 插件系统
#include "plugins/Plugin.h"
#include "plugins/PluginManager.h"

// 工具类
#include "utils/Logger.h"
#include "utils/Config.h"
#include "utils/Metrics.h"

namespace NetBox {

/**
 * @brief 框架版本信息
 */
struct Version {
    static constexpr int MAJOR = 1;
    static constexpr int MINOR = 0;
    static constexpr int PATCH = 0;
    static constexpr const char* STRING = "1.0.0";
};

/**
 * @brief 框架初始化
 */
bool initialize();

/**
 * @brief 框架清理
 */
void cleanup();

/**
 * @brief 获取框架版本
 */
const char* getVersion();

} // namespace NetBox

// 便利宏
#define NETBOX_VERSION_STRING NetBox::Version::STRING
#define NETBOX_INIT() NetBox::initialize()
#define NETBOX_CLEANUP() NetBox::cleanup()

#define NETBOX_DEBUG_LOG(msg) // 发布模式下禁用调试日志

// 启用的特性
#define NETBOX_FEATURE_LOGGING 1
#define NETBOX_FEATURE_METRICS 1
