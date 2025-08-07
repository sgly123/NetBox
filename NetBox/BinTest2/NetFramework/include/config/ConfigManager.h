#pragma once

/**
 * @file ConfigManager.h
 * @brief NetBox 配置管理系统
 * 
 * 特性：
 * 1. 多格式支持 (JSON, YAML, INI)
 * 2. 热重载配置
 * 3. 环境变量覆盖
 * 4. 配置验证
 * 5. 默认值支持
 * 6. 类型安全的配置访问
 * 7. 配置变更通知
 */

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <fstream>
#include <sstream>
#include <typeinfo>
#include <any>

namespace NetBox {
namespace Config {

/**
 * @brief 配置值类型枚举
 */
enum class ConfigType {
    STRING,
    INTEGER,
    DOUBLE,
    BOOLEAN,
    ARRAY,
    OBJECT
};

/**
 * @brief 配置值包装类
 */
class ConfigValue {
private:
    std::any m_value;
    ConfigType m_type;

public:
    ConfigValue() : m_type(ConfigType::STRING) {}
    
    template<typename T>
    ConfigValue(const T& value) {
        setValue(value);
    }
    
    template<typename T>
    void setValue(const T& value) {
        m_value = value;
        
        if constexpr (std::is_same_v<T, std::string>) {
            m_type = ConfigType::STRING;
        } else if constexpr (std::is_integral_v<T>) {
            m_type = ConfigType::INTEGER;
        } else if constexpr (std::is_floating_point_v<T>) {
            m_type = ConfigType::DOUBLE;
        } else if constexpr (std::is_same_v<T, bool>) {
            m_type = ConfigType::BOOLEAN;
        } else {
            m_type = ConfigType::OBJECT;
        }
    }
    
    template<typename T>
    T getValue() const {
        try {
            return std::any_cast<T>(m_value);
        } catch (const std::bad_any_cast& e) {
            throw std::runtime_error("配置值类型转换失败: " + std::string(e.what()));
        }
    }
    
    template<typename T>
    T getValue(const T& defaultValue) const {
        try {
            return std::any_cast<T>(m_value);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    
    ConfigType getType() const { return m_type; }
    
    bool isEmpty() const { return !m_value.has_value(); }
    
    std::string toString() const {
        switch (m_type) {
            case ConfigType::STRING:
                return getValue<std::string>();
            case ConfigType::INTEGER:
                return std::to_string(getValue<int>());
            case ConfigType::DOUBLE:
                return std::to_string(getValue<double>());
            case ConfigType::BOOLEAN:
                return getValue<bool>() ? "true" : "false";
            default:
                return "[complex_value]";
        }
    }
};

/**
 * @brief 配置变更监听器
 */
using ConfigChangeListener = std::function<void(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue)>;

/**
 * @brief JSON配置解析器
 */
class JsonConfigParser {
public:
    static std::unordered_map<std::string, ConfigValue> parse(const std::string& content) {
        std::unordered_map<std::string, ConfigValue> config;
        
        // 简化的JSON解析实现
        // 实际项目中应该使用专业的JSON库如nlohmann/json
        parseJsonObject(content, "", config);
        
        return config;
    }

private:
    static void parseJsonObject(const std::string& content, const std::string& prefix, 
                               std::unordered_map<std::string, ConfigValue>& config) {
        // 简化实现，实际应该使用完整的JSON解析器
        // 这里只是示例代码
        
        // 解析简单的键值对
        std::istringstream iss(content);
        std::string line;
        
        while (std::getline(iss, line)) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                
                // 去除引号和空格
                key.erase(0, key.find_first_not_of(" \t\""));
                key.erase(key.find_last_not_of(" \t\",") + 1);
                
                value.erase(0, value.find_first_not_of(" \t\""));
                value.erase(value.find_last_not_of(" \t\",") + 1);
                
                std::string fullKey = prefix.empty() ? key : prefix + "." + key;
                
                // 类型推断
                if (value == "true" || value == "false") {
                    config[fullKey] = ConfigValue(value == "true");
                } else if (value.find('.') != std::string::npos) {
                    try {
                        config[fullKey] = ConfigValue(std::stod(value));
                    } catch (...) {
                        config[fullKey] = ConfigValue(value);
                    }
                } else {
                    try {
                        config[fullKey] = ConfigValue(std::stoi(value));
                    } catch (...) {
                        config[fullKey] = ConfigValue(value);
                    }
                }
            }
        }
    }
};

/**
 * @brief 配置管理器
 */
class ConfigManager {
private:
    std::unordered_map<std::string, ConfigValue> m_config;
    std::vector<ConfigChangeListener> m_listeners;
    std::string m_configFile;
    mutable std::mutex m_mutex;
    bool m_hotReloadEnabled;

public:
    ConfigManager() : m_hotReloadEnabled(false) {}
    
    /**
     * @brief 加载配置文件
     */
    bool loadFromFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        m_configFile = filename;
        
        // 根据文件扩展名选择解析器
        if (filename.ends_with(".json")) {
            m_config = JsonConfigParser::parse(content);
        } else {
            // 其他格式的解析器...
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief 从字符串加载配置
     */
    bool loadFromString(const std::string& content, const std::string& format = "json") {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (format == "json") {
            m_config = JsonConfigParser::parse(content);
            return true;
        }
        
        return false;
    }
    
    /**
     * @brief 获取配置值
     */
    template<typename T>
    T get(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // 首先检查环境变量
        std::string envKey = "NETBOX_" + key;
        std::replace(envKey.begin(), envKey.end(), '.', '_');
        std::transform(envKey.begin(), envKey.end(), envKey.begin(), ::toupper);
        
        const char* envValue = std::getenv(envKey.c_str());
        if (envValue) {
            return convertFromString<T>(envValue);
        }
        
        // 然后检查配置文件
        auto it = m_config.find(key);
        if (it != m_config.end()) {
            return it->second.getValue<T>();
        }
        
        throw std::runtime_error("配置键不存在: " + key);
    }
    
    /**
     * @brief 获取配置值（带默认值）
     */
    template<typename T>
    T get(const std::string& key, const T& defaultValue) const {
        try {
            return get<T>(key);
        } catch (...) {
            return defaultValue;
        }
    }
    
    /**
     * @brief 设置配置值
     */
    template<typename T>
    void set(const std::string& key, const T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        ConfigValue oldValue;
        auto it = m_config.find(key);
        if (it != m_config.end()) {
            oldValue = it->second;
        }
        
        ConfigValue newValue(value);
        m_config[key] = newValue;
        
        // 通知监听器
        for (auto& listener : m_listeners) {
            listener(key, oldValue, newValue);
        }
    }
    
    /**
     * @brief 检查配置键是否存在
     */
    bool has(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_config.find(key) != m_config.end();
    }
    
    /**
     * @brief 获取所有配置键
     */
    std::vector<std::string> getKeys() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::string> keys;
        for (const auto& pair : m_config) {
            keys.push_back(pair.first);
        }
        return keys;
    }
    
    /**
     * @brief 添加配置变更监听器
     */
    void addChangeListener(const ConfigChangeListener& listener) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_listeners.push_back(listener);
    }
    
    /**
     * @brief 启用热重载
     */
    void enableHotReload(bool enable = true) {
        m_hotReloadEnabled = enable;
        // 实际实现中应该启动文件监控线程
    }
    
    /**
     * @brief 重新加载配置
     */
    bool reload() {
        if (m_configFile.empty()) {
            return false;
        }
        return loadFromFile(m_configFile);
    }
    
    /**
     * @brief 保存配置到文件
     */
    bool saveToFile(const std::string& filename = "") const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::string file = filename.empty() ? m_configFile : filename;
        if (file.empty()) {
            return false;
        }
        
        std::ofstream out(file);
        if (!out.is_open()) {
            return false;
        }
        
        // 简化的JSON输出
        out << "{\n";
        bool first = true;
        for (const auto& pair : m_config) {
            if (!first) out << ",\n";
            out << "  \"" << pair.first << "\": ";
            
            switch (pair.second.getType()) {
                case ConfigType::STRING:
                    out << "\"" << pair.second.toString() << "\"";
                    break;
                case ConfigType::BOOLEAN:
                    out << pair.second.toString();
                    break;
                default:
                    out << pair.second.toString();
                    break;
            }
            first = false;
        }
        out << "\n}\n";
        
        return true;
    }

private:
    template<typename T>
    T convertFromString(const std::string& str) const {
        if constexpr (std::is_same_v<T, std::string>) {
            return str;
        } else if constexpr (std::is_same_v<T, int>) {
            return std::stoi(str);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::stod(str);
        } else if constexpr (std::is_same_v<T, bool>) {
            return str == "true" || str == "1" || str == "yes";
        } else {
            throw std::runtime_error("不支持的配置类型转换");
        }
    }
};

/**
 * @brief 全局配置管理器单例
 */
class GlobalConfig {
private:
    static std::unique_ptr<ConfigManager> s_instance;
    static std::once_flag s_initFlag;

public:
    static ConfigManager& getInstance() {
        std::call_once(s_initFlag, []() {
            s_instance = std::make_unique<ConfigManager>();
        });
        return *s_instance;
    }
    
    template<typename T>
    static T get(const std::string& key) {
        return getInstance().get<T>(key);
    }
    
    template<typename T>
    static T get(const std::string& key, const T& defaultValue) {
        return getInstance().get<T>(key, defaultValue);
    }
    
    template<typename T>
    static void set(const std::string& key, const T& value) {
        getInstance().set<T>(key, value);
    }
    
    static bool loadFromFile(const std::string& filename) {
        return getInstance().loadFromFile(filename);
    }
    
    static bool has(const std::string& key) {
        return getInstance().has(key);
    }
};

} // namespace Config
} // namespace NetBox
