#pragma once

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief 增强版配置文件读取器
 * 
 * 功能特性：
 * 1. 支持多种格式：key=value 和简化的 YAML 格式
 * 2. 支持分层配置：application.type, network.port 等
 * 3. 支持数组配置：servers[0], servers[1] 等
 * 4. 向下兼容：完全兼容原有的 ConfigReader
 * 5. 类型安全：提供多种类型的读取方法
 * 
 * 支持的配置格式：
 * 
 * 1. 传统格式 (config.txt):
 *    host=127.0.0.1
 *    port=8888
 * 
 * 2. 分层格式 (config.yaml):
 *    application:
 *      type: echo
 *    network:
 *      ip: 127.0.0.1
 *      port: 8888
 * 
 * 使用示例：
 * ```cpp
 * EnhancedConfigReader config;
 * config.load("config.yaml");
 * std::string appType = config.getString("application.type", "echo");
 * std::string ip = config.getString("network.ip", "127.0.0.1");
 * int port = config.getInt("network.port", 8888);
 * ```
 */
class EnhancedConfigReader {
public:
    /**
     * @brief 加载配置文件
     * @param filename 配置文件路径
     * @return 加载是否成功
     * 
     * 自动检测文件格式：
     * - .yaml/.yml 后缀：使用YAML解析器
     * - 其他后缀：使用传统key=value解析器
     */
    bool load(const std::string& filename);

    /**
     * @brief 获取字符串配置项
     * @param key 配置项键名（支持分层：section.key）
     * @param defaultValue 默认值
     * @return 配置项值
     */
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;

    /**
     * @brief 获取整数配置项
     * @param key 配置项键名（支持分层：section.key）
     * @param defaultValue 默认值
     * @return 配置项值
     */
    int getInt(const std::string& key, int defaultValue = 0) const;

    /**
     * @brief 获取布尔配置项
     * @param key 配置项键名（支持分层：section.key）
     * @param defaultValue 默认值
     * @return 配置项值
     */
    bool getBool(const std::string& key, bool defaultValue = false) const;

    /**
     * @brief 获取浮点数配置项
     * @param key 配置项键名（支持分层：section.key）
     * @param defaultValue 默认值
     * @return 配置项值
     */
    double getDouble(const std::string& key, double defaultValue = 0.0) const;

    /**
     * @brief 检查配置项是否存在
     * @param key 配置项键名
     * @return 是否存在
     */
    bool hasKey(const std::string& key) const;

    /**
     * @brief 获取所有配置项的键名
     * @return 键名列表
     */
    std::vector<std::string> getAllKeys() const;

    /**
     * @brief 获取指定前缀的所有配置项
     * @param prefix 前缀（如 "network."）
     * @return 匹配的键值对
     */
    std::unordered_map<std::string, std::string> getKeysWithPrefix(const std::string& prefix) const;

    /**
     * @brief 清空所有配置项
     */
    void clear();

    /**
     * @brief 获取配置项数量
     * @return 配置项数量
     */
    size_t size() const;

private:
    /**
     * @brief 加载传统格式配置文件 (key=value)
     * @param filename 文件路径
     * @return 是否成功
     */
    bool loadTraditionalFormat(const std::string& filename);

    /**
     * @brief 加载简化YAML格式配置文件
     * @param filename 文件路径
     * @return 是否成功
     */
    bool loadYamlFormat(const std::string& filename);

    /**
     * @brief 解析YAML行
     * @param line 配置行
     * @param currentSection 当前节名
     * @return 是否解析成功
     */
    bool parseYamlLine(const std::string& line, std::string& currentSection);

    /**
     * @brief 去除字符串首尾空白字符
     * @param str 输入字符串
     * @return 处理后的字符串
     */
    std::string trim(const std::string& str) const;

    /**
     * @brief 检查文件是否为YAML格式
     * @param filename 文件路径
     * @return 是否为YAML格式
     */
    bool isYamlFile(const std::string& filename) const;

    /**
     * @brief 配置项存储
     * key: 配置项键名（可能包含层级，如 "application.type"）
     * value: 配置项值
     */
    std::unordered_map<std::string, std::string> m_config;
};
