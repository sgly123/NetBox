#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace NetBox::Plugins {

/**
 * @brief 插件接口
 */
class Plugin {
public:
    virtual ~Plugin() = default;

    // 插件信息
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string getAuthor() const = 0;

    // 插件生命周期
    virtual bool initialize() = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;

    // 插件配置
    virtual void configure(const std::unordered_map<std::string, std::string>& config) {}

    // 插件依赖
    virtual std::vector<std::string> getDependencies() const { return {}; }
};

/**
 * @brief 认证插件接口
 */
class AuthPlugin : public Plugin {
public:
    virtual bool authenticate(const std::string& username, const std::string& password) = 0;
    virtual bool authorize(const std::string& username, const std::string& resource) = 0;
    virtual std::string generateToken(const std::string& username) = 0;
    virtual bool validateToken(const std::string& token) = 0;
};

/**
 * @brief 缓存插件接口
 */
class CachePlugin : public Plugin {
public:
    virtual bool set(const std::string& key, const std::string& value, int ttl = 0) = 0;
    virtual std::string get(const std::string& key) = 0;
    virtual bool exists(const std::string& key) = 0;
    virtual bool remove(const std::string& key) = 0;
    virtual void clear() = 0;
};

/**
 * @brief 数据库插件接口
 */
class DatabasePlugin : public Plugin {
public:
    virtual bool connect(const std::string& connectionString) = 0;
    virtual void disconnect() = 0;
    virtual bool execute(const std::string& sql) = 0;
    virtual std::vector<std::unordered_map<std::string, std::string>> query(const std::string& sql) = 0;
    virtual bool beginTransaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;
};

} // namespace NetBox::Plugins
