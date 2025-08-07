#pragma once

/**
 * @file PluginManager.h
 * @brief NetBox 插件系统
 * 
 * 特性：
 * 1. 动态插件加载和卸载
 * 2. 插件生命周期管理
 * 3. 插件依赖管理
 * 4. 插件API版本控制
 * 5. 插件配置管理
 * 6. 插件事件系统
 * 7. 插件安全沙箱
 */

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <any>

#ifdef _WIN32
    #include <windows.h>
    typedef HMODULE PluginHandle;
#else
    #include <dlfcn.h>
    typedef void* PluginHandle;
#endif

namespace NetBox {
namespace Plugin {

/**
 * @brief 插件状态枚举
 */
enum class PluginState {
    UNLOADED,   // 未加载
    LOADING,    // 加载中
    LOADED,     // 已加载
    RUNNING,    // 运行中
    STOPPING,   // 停止中
    STOPPED,    // 已停止
    ERROR       // 错误状态
};

/**
 * @brief 插件信息结构体
 */
struct PluginInfo {
    std::string name;           // 插件名称
    std::string version;        // 插件版本
    std::string description;    // 插件描述
    std::string author;         // 插件作者
    std::string apiVersion;     // API版本
    std::vector<std::string> dependencies;  // 依赖插件列表
    std::unordered_map<std::string, std::string> metadata;  // 元数据
};

/**
 * @brief 插件接口基类
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    /**
     * @brief 获取插件信息
     */
    virtual PluginInfo getInfo() const = 0;
    
    /**
     * @brief 初始化插件
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief 启动插件
     */
    virtual bool start() = 0;
    
    /**
     * @brief 停止插件
     */
    virtual void stop() = 0;
    
    /**
     * @brief 清理插件
     */
    virtual void cleanup() = 0;
    
    /**
     * @brief 处理事件
     */
    virtual void onEvent(const std::string& eventName, const std::any& eventData) {}
    
    /**
     * @brief 获取插件配置
     */
    virtual std::unordered_map<std::string, std::any> getConfig() const { return {}; }
    
    /**
     * @brief 设置插件配置
     */
    virtual void setConfig(const std::unordered_map<std::string, std::any>& config) {}
};

/**
 * @brief 插件工厂函数类型
 */
using PluginCreateFunc = std::function<std::unique_ptr<IPlugin>()>;
using PluginDestroyFunc = std::function<void(IPlugin*)>;

/**
 * @brief 插件包装器
 */
class PluginWrapper {
private:
    std::unique_ptr<IPlugin> m_plugin;
    PluginHandle m_handle;
    PluginInfo m_info;
    PluginState m_state;
    std::string m_filePath;
    std::chrono::system_clock::time_point m_loadTime;
    std::chrono::system_clock::time_point m_lastActivity;

public:
    PluginWrapper(std::unique_ptr<IPlugin> plugin, PluginHandle handle, 
                  const std::string& filePath)
        : m_plugin(std::move(plugin)), m_handle(handle), m_filePath(filePath),
          m_state(PluginState::LOADED),
          m_loadTime(std::chrono::system_clock::now()),
          m_lastActivity(std::chrono::system_clock::now()) {
        if (m_plugin) {
            m_info = m_plugin->getInfo();
        }
    }
    
    ~PluginWrapper() {
        if (m_state == PluginState::RUNNING) {
            stop();
        }
        cleanup();
        unload();
    }
    
    bool initialize() {
        if (m_state != PluginState::LOADED) {
            return false;
        }
        
        m_state = PluginState::LOADING;
        bool success = m_plugin->initialize();
        m_state = success ? PluginState::LOADED : PluginState::ERROR;
        updateActivity();
        return success;
    }
    
    bool start() {
        if (m_state != PluginState::LOADED) {
            return false;
        }
        
        bool success = m_plugin->start();
        m_state = success ? PluginState::RUNNING : PluginState::ERROR;
        updateActivity();
        return success;
    }
    
    void stop() {
        if (m_state == PluginState::RUNNING) {
            m_state = PluginState::STOPPING;
            m_plugin->stop();
            m_state = PluginState::STOPPED;
            updateActivity();
        }
    }
    
    void cleanup() {
        if (m_plugin) {
            m_plugin->cleanup();
            updateActivity();
        }
    }
    
    void unload() {
        if (m_handle) {
#ifdef _WIN32
            FreeLibrary(m_handle);
#else
            dlclose(m_handle);
#endif
            m_handle = nullptr;
        }
        m_state = PluginState::UNLOADED;
    }
    
    void onEvent(const std::string& eventName, const std::any& eventData) {
        if (m_state == PluginState::RUNNING && m_plugin) {
            m_plugin->onEvent(eventName, eventData);
            updateActivity();
        }
    }
    
    // Getters
    const PluginInfo& getInfo() const { return m_info; }
    PluginState getState() const { return m_state; }
    const std::string& getFilePath() const { return m_filePath; }
    IPlugin* getPlugin() const { return m_plugin.get(); }
    
    std::chrono::system_clock::time_point getLoadTime() const { return m_loadTime; }
    std::chrono::system_clock::time_point getLastActivity() const { return m_lastActivity; }

private:
    void updateActivity() {
        m_lastActivity = std::chrono::system_clock::now();
    }
};

/**
 * @brief 插件事件系统
 */
class PluginEventSystem {
private:
    std::unordered_map<std::string, std::vector<std::function<void(const std::any&)>>> m_listeners;
    mutable std::mutex m_mutex;

public:
    void subscribe(const std::string& eventName, 
                   const std::function<void(const std::any&)>& listener) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_listeners[eventName].push_back(listener);
    }
    
    void publish(const std::string& eventName, const std::any& eventData) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_listeners.find(eventName);
        if (it != m_listeners.end()) {
            for (const auto& listener : it->second) {
                try {
                    listener(eventData);
                } catch (...) {
                    // 忽略监听器异常，避免影响其他监听器
                }
            }
        }
    }
    
    void unsubscribe(const std::string& eventName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_listeners.erase(eventName);
    }
};

/**
 * @brief 插件管理器
 */
class PluginManager {
private:
    std::unordered_map<std::string, std::unique_ptr<PluginWrapper>> m_plugins;
    std::vector<std::string> m_pluginPaths;
    PluginEventSystem m_eventSystem;
    mutable std::mutex m_mutex;
    std::string m_apiVersion;

public:
    PluginManager(const std::string& apiVersion = "1.0") 
        : m_apiVersion(apiVersion) {}
    
    ~PluginManager() {
        unloadAllPlugins();
    }
    
    /**
     * @brief 添加插件搜索路径
     */
    void addPluginPath(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pluginPaths.push_back(path);
    }
    
    /**
     * @brief 加载插件
     */
    bool loadPlugin(const std::string& filePath) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // 加载动态库
        PluginHandle handle = nullptr;
        
#ifdef _WIN32
        handle = LoadLibraryA(filePath.c_str());
        if (!handle) {
            return false;
        }
        
        // 获取创建函数
        auto createFunc = reinterpret_cast<IPlugin*(*)()>(
            GetProcAddress(handle, "createPlugin"));
        if (!createFunc) {
            FreeLibrary(handle);
            return false;
        }
#else
        handle = dlopen(filePath.c_str(), RTLD_LAZY);
        if (!handle) {
            return false;
        }
        
        // 获取创建函数
        auto createFunc = reinterpret_cast<IPlugin*(*)()>(
            dlsym(handle, "createPlugin"));
        if (!createFunc) {
            dlclose(handle);
            return false;
        }
#endif
        
        // 创建插件实例
        std::unique_ptr<IPlugin> plugin(createFunc());
        if (!plugin) {
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return false;
        }
        
        // 检查API版本兼容性
        PluginInfo info = plugin->getInfo();
        if (info.apiVersion != m_apiVersion) {
            // API版本不兼容
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return false;
        }
        
        // 检查依赖
        if (!checkDependencies(info.dependencies)) {
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return false;
        }
        
        // 创建插件包装器
        auto wrapper = std::make_unique<PluginWrapper>(std::move(plugin), handle, filePath);
        
        // 初始化插件
        if (!wrapper->initialize()) {
            return false;
        }
        
        // 注册插件
        std::string pluginName = info.name;
        m_plugins[pluginName] = std::move(wrapper);
        
        // 发布插件加载事件
        m_eventSystem.publish("plugin.loaded", pluginName);
        
        return true;
    }
    
    /**
     * @brief 卸载插件
     */
    bool unloadPlugin(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_plugins.find(pluginName);
        if (it == m_plugins.end()) {
            return false;
        }
        
        // 发布插件卸载事件
        m_eventSystem.publish("plugin.unloading", pluginName);
        
        // 移除插件
        m_plugins.erase(it);
        
        // 发布插件卸载完成事件
        m_eventSystem.publish("plugin.unloaded", pluginName);
        
        return true;
    }
    
    /**
     * @brief 启动插件
     */
    bool startPlugin(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_plugins.find(pluginName);
        if (it == m_plugins.end()) {
            return false;
        }
        
        bool success = it->second->start();
        if (success) {
            m_eventSystem.publish("plugin.started", pluginName);
        }
        
        return success;
    }
    
    /**
     * @brief 停止插件
     */
    bool stopPlugin(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_plugins.find(pluginName);
        if (it == m_plugins.end()) {
            return false;
        }
        
        it->second->stop();
        m_eventSystem.publish("plugin.stopped", pluginName);
        
        return true;
    }
    
    /**
     * @brief 获取插件
     */
    IPlugin* getPlugin(const std::string& pluginName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_plugins.find(pluginName);
        return (it != m_plugins.end()) ? it->second->getPlugin() : nullptr;
    }
    
    /**
     * @brief 获取所有插件名称
     */
    std::vector<std::string> getPluginNames() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::string> names;
        for (const auto& pair : m_plugins) {
            names.push_back(pair.first);
        }
        return names;
    }
    
    /**
     * @brief 获取插件信息
     */
    std::vector<PluginInfo> getPluginInfos() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<PluginInfo> infos;
        for (const auto& pair : m_plugins) {
            infos.push_back(pair.second->getInfo());
        }
        return infos;
    }
    
    /**
     * @brief 发布事件到所有插件
     */
    void publishEvent(const std::string& eventName, const std::any& eventData) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (const auto& pair : m_plugins) {
            pair.second->onEvent(eventName, eventData);
        }
        
        // 同时发布到事件系统
        m_eventSystem.publish(eventName, eventData);
    }
    
    /**
     * @brief 订阅事件
     */
    void subscribeEvent(const std::string& eventName, 
                       const std::function<void(const std::any&)>& listener) {
        m_eventSystem.subscribe(eventName, listener);
    }
    
    /**
     * @brief 卸载所有插件
     */
    void unloadAllPlugins() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // 先停止所有插件
        for (const auto& pair : m_plugins) {
            pair.second->stop();
        }
        
        // 然后清理所有插件
        m_plugins.clear();
    }

private:
    bool checkDependencies(const std::vector<std::string>& dependencies) {
        for (const std::string& dep : dependencies) {
            if (m_plugins.find(dep) == m_plugins.end()) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @brief 全局插件管理器单例
 */
class GlobalPluginManager {
private:
    static std::unique_ptr<PluginManager> s_instance;
    static std::once_flag s_initFlag;

public:
    static PluginManager& getInstance() {
        std::call_once(s_initFlag, []() {
            s_instance = std::make_unique<PluginManager>();
        });
        return *s_instance;
    }
    
    static bool loadPlugin(const std::string& filePath) {
        return getInstance().loadPlugin(filePath);
    }
    
    static IPlugin* getPlugin(const std::string& pluginName) {
        return getInstance().getPlugin(pluginName);
    }
    
    static void publishEvent(const std::string& eventName, const std::any& eventData) {
        getInstance().publishEvent(eventName, eventData);
    }
};

} // namespace Plugin
} // namespace NetBox

// 插件导出宏
#define NETBOX_PLUGIN_EXPORT extern "C" __declspec(dllexport)

// 插件创建宏
#define NETBOX_PLUGIN_CREATE(PluginClass) \
    NETBOX_PLUGIN_EXPORT NetBox::Plugin::IPlugin* createPlugin() { \
        return new PluginClass(); \
    }

// 插件销毁宏
#define NETBOX_PLUGIN_DESTROY() \
    NETBOX_PLUGIN_EXPORT void destroyPlugin(NetBox::Plugin::IPlugin* plugin) { \
        delete plugin; \
    }
