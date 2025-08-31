// #include "app/ApplicationRegistry.h"
// #include "SanguoshaServer.h"  // 未来实现
// #include "base/Logger.h"

/**
 * @brief 三国杀服务器插件注册文件
 * 
 * 功能：
 * 1. 将SanguoshaServer注册到应用注册表
 * 2. 支持通过配置文件动态创建三国杀游戏服务器实例
 * 3. 实现游戏服务器的插件化架构
 * 
 * 使用方法：
 * 在配置文件中设置 application.type = "sanguosha" 即可使用
 * 
 * 配置示例：
 * application:
 *   type: sanguosha
 * network:
 *   port: 8888
 * sanguosha:
 *   max_players_per_room: 8
 *   room_timeout: 1800
 *   enable_ai: true
 */

/*
// 未来实现时取消注释
static bool registerSanguoshaServer() {
    Logger::info("正在注册SanguoshaServer插件...");
    
    bool success = ApplicationRegistry::getInstance().registerApplication("sanguosha", 
        [](const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool) {
            Logger::info("创建SanguoshaServer实例: " + ip + ":" + std::to_string(port));
            return std::make_unique<SanguoshaServer>(ip, port, io_type, pool);
        });
    
    if (success) {
        Logger::info("SanguoshaServer插件注册成功");
    } else {
        Logger::error("SanguoshaServer插件注册失败");
    }
    
    return success;
}

static bool g_sanguoshaServerRegistered = registerSanguoshaServer();

std::string getSanguoshaPluginInfo() {
    return "SanguoshaServer Plugin v1.0 - 提供三国杀游戏服务功能";
}
*/
