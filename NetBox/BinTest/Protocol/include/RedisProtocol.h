#pragma once

#include "ProtocolBase.h"
#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief Redis协议实现
 * 
 * 专门为Redis RESP协议设计的协议处理器
 * 负责解析Redis命令和格式化Redis响应
 * 
 * RESP协议特点:
 * 1. 基于文本的协议，易于调试
 * 2. 支持多种数据类型：Simple String, Error, Integer, Bulk String, Array
 * 3. 以\r\n作为行分隔符
 * 4. 每种类型有特定的前缀标识符
 * 
 * 协议格式:
 * - Simple String: +OK\r\n
 * - Error: -Error message\r\n  
 * - Integer: :42\r\n
 * - Bulk String: $6\r\nhello\r\n
 * - Array: *2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n
 * - Null: $-1\r\n
 */
class RedisProtocol : public ProtocolBase {
public:
    /**
     * @brief 构造函数
     */
    RedisProtocol();
    
    /**
     * @brief 析构函数
     */
    ~RedisProtocol() override = default;

    /**
     * @brief 获取协议ID
     */
    uint32_t getProtocolId() const override;

    /**
     * @brief 获取协议类型
     */
    std::string getType() const override;

    /**
     * @brief 重置协议状态
     */
    void reset() override;

    /**
     * @brief 处理接收到的数据（ProtocolBase接口）
     */
    size_t onDataReceived(const char* data, size_t len) override;

    /**
     * @brief 封包数据
     */
    bool pack(const char* data, size_t len, std::vector<char>& packet) override;

    /**
     * @brief 处理客户端数据（扩展接口）
     */
    size_t onClientDataReceived(int clientFd, const char* data, size_t len);

private:
    /**
     * @brief 协议ID (Redis协议使用ID 2)
     */
    static constexpr uint32_t REDIS_PROTOCOL_ID = 2;
    

    
    /**
     * @brief 客户端缓冲区（处理不完整的命令）
     */
    std::unordered_map<int, std::string> m_clientBuffers;

    // ==================== Redis协议解析 ====================
    
    /**
     * @brief 处理完整的Redis命令行
     * @param clientFd 客户端文件描述符
     * @param commandLine 完整的命令行
     */
    void processRedisCommand(int clientFd, const std::string& commandLine);
    
    /**
     * @brief 解析Redis命令参数
     * @param commandLine 命令行字符串
     * @return 解析后的参数列表
     */
    std::vector<std::string> parseRedisCommand(const std::string& commandLine);
    
    /**
     * @brief 检查是否为完整的Redis命令
     * @param buffer 缓冲区数据
     * @return 如果是完整命令返回命令长度，否则返回0
     */
    size_t isCompleteRedisCommand(const std::string& buffer);

    // ==================== RESP协议格式化 ====================
    
    /**
     * @brief 格式化Simple String响应
     */
    std::string formatSimpleString(const std::string& str);
    
    /**
     * @brief 格式化Bulk String响应
     */
    std::string formatBulkString(const std::string& str);
    
    /**
     * @brief 格式化Array响应
     */
    std::string formatArray(const std::vector<std::string>& arr);
    
    /**
     * @brief 格式化Integer响应
     */
    std::string formatInteger(int num);
    
    /**
     * @brief 格式化Error响应
     */
    std::string formatError(const std::string& error);
    
    /**
     * @brief 格式化Null响应
     */
    std::string formatNull();

    // ==================== Redis命令执行 ====================

    /**
     * @brief 执行Redis命令
     */
    std::string executeRedisCommand(const std::vector<std::string>& args);

    /**
     * @brief 直接发送响应到客户端
     */
    void sendDirectResponse(int clientFd, const std::string& response);
};
