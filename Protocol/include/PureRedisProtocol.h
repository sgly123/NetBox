#pragma once
#include "ProtocolBase.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

class PureRedisProtocol : public ProtocolBase {
public:
    static constexpr uint32_t PURE_REDIS_PROTOCOL_ID = 3;

    PureRedisProtocol();
    uint32_t getProtocolId() const override;
    std::string getType() const override;
    void reset() override;

    size_t onDataReceived(const char* data, size_t len) override;
    bool pack(const char* data, size_t len, std::vector<char>& packet) override;
    size_t onClientDataReceived(int clientFd, const char* data, size_t len);

private:
    // Redis 命令执行
    void processRedisCommand(int clientFd, const std::vector<std::string>& args);
    std::string executeRedisCommand(const std::vector<std::string>& args);

    // RESP 解码
    std::pair<bool, std::vector<std::string>>
    resp_decode(const std::string& buf, size_t& consumed);

    // RESP 格式化
    std::string formatSimpleString(const std::string& str);
    std::string formatError(const std::string& error);
    std::string formatInteger(int64_t value);
    std::string formatBulkString(const std::string& str);
    std::string formatArray(const std::vector<std::string>& array);
    std::string formatNull();

    std::mutex m_sendMutex;  // 加锁
public:
    // 直接发送
    void sendDirectResponse(int clientFd, const std::string& response);

    // 客户端缓冲区
    std::unordered_map<int, std::string> m_clientBuffers;

    // 简易内存数据库
    std::unordered_map<std::string, std::string> m_stringData;
};