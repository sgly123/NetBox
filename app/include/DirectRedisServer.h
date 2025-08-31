#pragma once

#include "ApplicationServer.h"
#include <unordered_map>
#include <string>
#include <list>
#include <mutex>
#include <vector>

/**
 * @brief 直接Redis服务器实现
 *
 * 继承ApplicationServer但不使用协议栈，直接处理原始TCP数据
 * 专门为Redis协议优化，避免协议转换的复杂性
 *
 * 特点：
 * 1. 继承ApplicationServer，融入框架体系
 * 2. 原生处理Redis RESP协议
 * 3. 无协议转换开销，性能更高
 * 4. 专门为Redis优化的数据流
 */
class DirectRedisServer : public ApplicationServer {
public:
    /**
     * @brief 构造函数
     */
    DirectRedisServer(const std::string& ip, int port, IOMultiplexer::IOType io_type, IThreadPool* pool = nullptr);

    virtual ~DirectRedisServer() = default;

protected:
    /**
     * @brief 初始化协议路由器（DirectRedis不使用协议栈）
     */
    void initializeProtocolRouter() override;

    /**
     * @brief 处理HTTP请求（DirectRedis不支持HTTP）
     */
    std::string handleHttpRequest(const std::string& request, int clientFd) override;

    /**
     * @brief 处理业务逻辑（DirectRedis直接处理）
     */
    std::string handleBusinessLogic(const std::string& command, const std::vector<std::string>& args) override;

    /**
     * @brief 解析请求路径（DirectRedis不使用）
     */
    bool parseRequestPath(const std::string& path, std::string& command, std::vector<std::string>& args) override;

    /**
     * @brief 处理客户端数据（核心方法）
     */
    void onDataReceived(int clientFd, const char* data, size_t len) override;

    /**
     * @brief 处理客户端连接
     */
    void onClientConnected(int clientFd) override;

    /**
     * @brief 处理客户端断开
     */
    void onClientDisconnected(int clientFd) override;

private:
    // ==================== 数据存储 ====================
    
    /**
     * @brief String类型数据存储
     */
    std::unordered_map<std::string, std::string> m_stringData;
    
    /**
     * @brief List类型数据存储
     */
    std::unordered_map<std::string, std::list<std::string>> m_listData;
    
    /**
     * @brief Hash类型数据存储
     */
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_hashData;
    
    /**
     * @brief 数据访问锁
     */
    mutable std::mutex m_dataLock;
    
    /**
     * @brief 客户端缓冲区（处理不完整的命令）
     */
    std::unordered_map<int, std::string> m_clientBuffers;

    // ==================== 命令处理 ====================
    
    /**
     * @brief 解析Redis命令字符串
     */
    std::vector<std::string> parseRedisCommand(const std::string& command);
    
    /**
     * @brief 执行Redis命令
     */
    std::string executeRedisCommand(const std::vector<std::string>& args);
    
    /**
     * @brief 发送Redis响应到客户端
     */
    void sendRedisResponse(int clientFd, const std::string& response);
    
    /**
     * @brief 处理完整的命令行
     */
    void processCommandLine(int clientFd, const std::string& commandLine);

    // ==================== Redis命令实现 ====================
    
    std::string cmdPing(const std::vector<std::string>& args);
    std::string cmdSet(const std::vector<std::string>& args);
    std::string cmdGet(const std::vector<std::string>& args);
    std::string cmdDel(const std::vector<std::string>& args);
    std::string cmdKeys(const std::vector<std::string>& args);
    std::string cmdLPush(const std::vector<std::string>& args);
    std::string cmdLPop(const std::vector<std::string>& args);
    std::string cmdLRange(const std::vector<std::string>& args);
    std::string cmdHSet(const std::vector<std::string>& args);
    std::string cmdHGet(const std::vector<std::string>& args);
    std::string cmdHKeys(const std::vector<std::string>& args);
    
    // ==================== RESP协议格式化 ====================
    
    std::string formatSimpleString(const std::string& str);
    std::string formatBulkString(const std::string& str);
    std::string formatArray(const std::vector<std::string>& arr);
    std::string formatInteger(int num);
    std::string formatError(const std::string& error);
    std::string formatNull();
};
