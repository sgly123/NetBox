/**
 * @file simple_server.cpp
 * @brief SimpleEchoServer - 基础TCP服务器示例
 * 
 * 展示如何使用NetBox框架创建一个简单的TCP服务器
 * 
 * @author NetBox Developer
 * @version 1.0.0
 * @date 2025-08-03
 */

#include "netbox/NetBox.h"
#include <iostream>
#include <signal.h>

using namespace NetBox;

/**
 * @brief 简单的连接处理器
 */
class SimpleHandler : public Application::Handler {
public:
    void onConnect(std::shared_ptr<Application::Context> ctx) override {
        std::cout << "🔗 新连接: " << ctx->getRemoteAddress() << std::endl;
        
        ctx->send("欢迎连接到 SimpleEchoServer 服务器!\n");
        
        // 更新连接计数
        m_connectionCount++;
        std::cout << "📊 当前连接数: " << m_connectionCount << std::endl;
    }
    
    void onData(std::shared_ptr<Application::Context> ctx, const std::vector<uint8_t>& data) override {
        std::string message(data.begin(), data.end());
        std::cout << "📨 收到消息: " << message << std::endl;
        
        // 回显模式
        ctx->send("Echo: " + message);
    }
    
    void onDisconnect(std::shared_ptr<Application::Context> ctx) override {
        std::cout << "❌ 连接断开: " << ctx->getRemoteAddress() << std::endl;
        
        m_connectionCount--;
        std::cout << "📊 当前连接数: " << m_connectionCount << std::endl;
    }
    
    void onError(std::shared_ptr<Application::Context> ctx, const std::string& error) override {
        std::cout << "⚠️  连接错误: " << error << std::endl;
    }

private:
    std::atomic<int> m_connectionCount{0};
    
};

/**
 * @brief 简单的应用程序类
 */
class SimpleApplication : public Application::Application {
private:
    std::shared_ptr<Application::Handler> m_handler;
    
public:
    SimpleApplication() : Application("SimpleEchoServer") {
        m_handler = std::make_shared<SimpleHandler>();
    }
    
    bool initialize() override {
        std::cout << "🔧 初始化 SimpleEchoServer 服务器..." << std::endl;
        
        
        // 设置日志
        setupLogging();
        
        return true;
    }
    
    bool start() override {
        std::cout << "🚀 启动 SimpleEchoServer 服务器..." << std::endl;
        // 这里会连接到NetBox的网络层
        return true;
    }
    
    void stop() override {
        std::cout << "🛑 停止 SimpleEchoServer 服务器..." << std::endl;
    }
    
    void cleanup() override {
        std::cout << "🧹 清理 SimpleEchoServer 服务器..." << std::endl;
    }
    
    void setHandler(std::shared_ptr<Application::Handler> handler) override {
        m_handler = handler;
    }
    
    std::shared_ptr<Application::Handler> getHandler() const override {
        return m_handler;
    }

private:
    
    void setupLogging() {
        // 日志设置逻辑
        std::cout << "📝 日志系统已启用" << std::endl;
    }
};

// 全局应用实例
std::unique_ptr<SimpleApplication> g_app;

void signalHandler(int signal) {
    std::cout << "\n🛑 接收到停止信号 (" << signal << ")，正在关闭服务器..." << std::endl;
    if (g_app) {
        g_app->stop();
    }
}

int main() {
    std::cout << "🌟 启动 SimpleEchoServer 基础示例" << std::endl;
    std::cout << "基于 NetBox 跨平台网络框架构建" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // 初始化框架
    if (!NETBOX_INIT()) {
        std::cerr << "❌ NetBox框架初始化失败" << std::endl;
        return 1;
    }
    
    try {
        // 创建应用
        g_app = std::make_unique<SimpleApplication>();
        auto& app = *g_app;
        
        // 初始化并启动
        if (app.initialize() && app.start()) {
            std::cout << "✅ 服务器启动成功!" << std::endl;
            std::cout << "🔌 监听端口: 8080" << std::endl;
            std::cout << "💡 测试命令: telnet localhost 8080" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "按 Enter 键停止服务器..." << std::endl;
            
            std::cin.get();
            
            app.stop();
        }
        
        app.cleanup();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 服务器异常: " << e.what() << std::endl;
    }
    
    // 清理框架
    NETBOX_CLEANUP();
    
    std::cout << "👋 SimpleEchoServer 已安全关闭" << std::endl;
    return 0;
}