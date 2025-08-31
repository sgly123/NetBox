#pragma once

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace NetBox::Application {

/**
 * @brief 应用上下文
 */
class Context {
public:
    virtual ~Context() = default;

    // 获取连接信息
    virtual std::string getRemoteAddress() const = 0;
    virtual std::string getLocalAddress() const = 0;

    // 会话管理
    virtual void setAttribute(const std::string& key, const std::string& value) = 0;
    virtual std::string getAttribute(const std::string& key) const = 0;
    virtual bool hasAttribute(const std::string& key) const = 0;

    // 发送数据
    virtual void send(const std::vector<uint8_t>& data) = 0;
    virtual void send(const std::string& data) = 0;

    // 关闭连接
    virtual void close() = 0;
};

/**
 * @brief 应用处理器接口
 */
class Handler {
public:
    virtual ~Handler() = default;

    /**
     * @brief 处理新连接
     */
    virtual void onConnect(std::shared_ptr<Context> ctx) {}

    /**
     * @brief 处理断开连接
     */
    virtual void onDisconnect(std::shared_ptr<Context> ctx) {}

    /**
     * @brief 处理接收数据
     */
    virtual void onData(std::shared_ptr<Context> ctx, const std::vector<uint8_t>& data) {}

    /**
     * @brief 处理错误
     */
    virtual void onError(std::shared_ptr<Context> ctx, const std::string& error) {}

    /**
     * @brief 处理超时
     */
    virtual void onTimeout(std::shared_ptr<Context> ctx) {}
};

/**
 * @brief 应用基类
 */
class Application {
protected:
    std::string m_name;
    std::unordered_map<std::string, std::string> m_config;

public:
    Application(const std::string& name) : m_name(name) {}
    virtual ~Application() = default;

    // 应用生命周期
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;

    // 配置管理
    virtual void setConfig(const std::unordered_map<std::string, std::string>& config) {
        m_config = config;
    }

    virtual std::string getConfig(const std::string& key, const std::string& defaultValue = "") const {
        auto it = m_config.find(key);
        return (it != m_config.end()) ? it->second : defaultValue;
    }

    // 应用信息
    virtual std::string getName() const { return m_name; }
    virtual std::string getVersion() const { return "1.0.0"; }
    virtual std::string getDescription() const { return "NetBox Application"; }

    // 处理器管理
    virtual void setHandler(std::shared_ptr<Handler> handler) = 0;
    virtual std::shared_ptr<Handler> getHandler() const = 0;
};

/**
 * @brief Web应用基类
 */
class WebApplication : public Application {
public:
    WebApplication(const std::string& name) : Application(name) {}

    // HTTP路由
    virtual void addRoute(const std::string& method, const std::string& path,
                         std::function<void(std::shared_ptr<Context>)> handler) = 0;

    // 静态文件服务
    virtual void serveStatic(const std::string& path, const std::string& directory) = 0;

    // 中间件
    virtual void addMiddleware(std::function<bool(std::shared_ptr<Context>)> middleware) = 0;
};

/**
 * @brief 游戏应用基类
 */
class GameApplication : public Application {
public:
    GameApplication(const std::string& name) : Application(name) {}

    // 玩家管理
    virtual void onPlayerJoin(std::shared_ptr<Context> ctx) = 0;
    virtual void onPlayerLeave(std::shared_ptr<Context> ctx) = 0;

    // 游戏逻辑
    virtual void onGameMessage(std::shared_ptr<Context> ctx, const std::string& message) = 0;
    virtual void broadcastMessage(const std::string& message) = 0;

    // 房间管理
    virtual void createRoom(const std::string& roomId) = 0;
    virtual void joinRoom(std::shared_ptr<Context> ctx, const std::string& roomId) = 0;
    virtual void leaveRoom(std::shared_ptr<Context> ctx, const std::string& roomId) = 0;
};

} // namespace NetBox::Application
