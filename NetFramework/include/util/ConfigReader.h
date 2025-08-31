#pragma once
#include <string>
#include <unordered_map>

/**
 * @brief 配置文件读取器：支持key=value格式的配置文件解析
 * 
 * 设计特点：
 * 1. 简单易用：支持标准的key=value格式
 * 2. 类型安全：提供类型转换方法，避免类型错误
 * 3. 默认值支持：所有读取方法都支持默认值
 * 4. 错误容错：解析失败时返回默认值，不中断程序
 * 
 * 配置文件格式：
 * - 每行一个配置项：key=value
 * - 支持注释：以#开头的行会被忽略
 * - 支持空行：空行会被自动跳过
 * - 大小写敏感：key区分大小写
 * 
 * 使用示例：
 * ```cpp
 * ConfigReader config;
 * config.load("config.txt");
 * std::string host = config.getString("host", "127.0.0.1");
 * int port = config.getInt("port", 8080);
 * ```
 * 
 * 扩展建议：
 * - 支持分组配置：[section]key=value
 * - 支持环境变量：${ENV_VAR}
 * - 支持配置文件继承：include other.conf
 * - 支持配置验证：schema validation
 */
class ConfigReader {
public:
    /**
     * @brief 加载配置文件
     * @param filename 配置文件路径
     * @return 加载是否成功
     * 
     * 处理流程：
     * 1. 打开配置文件
     * 2. 逐行读取文件内容
     * 3. 跳过注释行和空行
     * 4. 解析key=value格式
     * 5. 存储到内部映射表
     * 
     * 错误处理：
     * - 文件不存在：返回false
     * - 文件格式错误：跳过错误行，继续处理
     * - 权限不足：返回false
     */
    bool load(const std::string& filename);

    /**
     * @brief 获取字符串配置项
     * @param key 配置项键名
     * @param defaultValue 默认值（当键不存在时返回）
     * @return 配置项值
     * 
     * 特点：
     * - 支持默认值：键不存在时返回默认值
     * - 类型安全：返回std::string类型
     * - 性能优化：O(1)查找复杂度
     */
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;

    /**
     * @brief 获取整数配置项
     * @param key 配置项键名
     * @param defaultValue 默认值（当键不存在或转换失败时返回）
     * @return 配置项值
     * 
     * 类型转换：
     * - 支持十进制整数：123
     * - 支持十六进制：0x7B
     * - 支持八进制：0173
     * - 转换失败时返回默认值
     * 
     * 错误处理：
     * - 键不存在：返回默认值
     * - 格式错误：返回默认值
     * - 溢出错误：返回默认值
     */
    int getInt(const std::string& key, int defaultValue = 0) const;

private:
    std::unordered_map<std::string, std::string> config_;  // 配置项存储：key -> value
}; 