#pragma once
#include "ProtocolBase.h"
#include <unordered_map>
#include <string>
#include <vector>

/**
 * @brief 纯RESP协议实现
 * 
 * 设计理念：
 * 1. 完全遵循Redis RESP协议标准
 * 2. 不添加任何协议头或封装
 * 3. 直接传输RESP格式数据
 * 4. 与标准Redis客户端100%兼容
 * 
 * 协议特点：
 * - 输入：纯文本Redis命令（如"PING\r\n"）
 * - 输出：标准RESP响应（如"+PONG\r\n"）
 * - 无协议头：直接socket传输
 * - 完全兼容：支持所有Redis客户端
 */
class PureRedisProtocol : public ProtocolBase {
public:
    static constexpr uint32_t PURE_REDIS_PROTOCOL_ID = 3;
    
    PureRedisProtocol();
    
    // ProtocolBase接口实现
    uint32_t getProtocolId() const override;
    std::string getType() const override;
    void reset() override;
    
    // 数据处理接口
    size_t onDataReceived(const char* data, size_t len) override;
    bool pack(const char* data, size_t len, std::vector<char>& packet) override;
    
    // Redis专用接口
    size_t onClientDataReceived(int clientFd, const char* data, size_t len);
    
private:
    // Redis命令处理
    void processRedisCommand(int clientFd, const std::string& commandLine);
    std::vector<std::string> parseRedisCommand(const std::string& commandLine);
    size_t isCompleteRedisCommand(const std::string& buffer);
    
    // Redis命令执行
    std::string executeRedisCommand(const std::vector<std::string>& args);
    
    // RESP格式化方法
    std::string formatSimpleString(const std::string& str);
    std::string formatError(const std::string& error);
    std::string formatInteger(int64_t value);
    std::string formatBulkString(const std::string& str);
    std::string formatArray(const std::vector<std::string>& array);
    std::string formatNull();
    
    // 直接响应发送（绕过协议封装）
    void sendDirectResponse(int clientFd, const std::string& response);
    
    // 客户端缓冲区管理
    std::unordered_map<int, std::string> m_clientBuffers;
    
    // Redis数据存储（简单实现）
    std::unordered_map<std::string, std::string> m_stringData;
    std::unordered_map<std::string, std::vector<std::string>> m_listData;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_hashData;
};
