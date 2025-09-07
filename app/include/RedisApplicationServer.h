#pragma once

#include "ApplicationServer.h"
#include <unordered_map>
#include <string>
#include <list>
#include <mutex>

/**
 * @brief Redis应用服务器
 * 
 * 将SimpleRedis的功能集成到NetBox框架中
 * 复用框架的网络层、协议层、线程池等基础设施
 * 
 * 特点：
 * 1. 继承ApplicationServer，融入NetBox架构
 * 2. 使用SimpleHeaderProtocol处理网络数据
 * 3. 保留完整的Redis命令处理逻辑
 * 4. 支持插件化配置和管理
 */
class RedisApplicationServer : public ApplicationServer {
public:
    /**
     * @brief 构造函数
     */
    RedisApplicationServer(const std::string& ip, int port, 
                          IOMultiplexer::IOType io_type, IThreadPool* pool);
    
    ~RedisApplicationServer() override = default;

protected:
    /**
     * @brief 初始化协议路由器
     */
    void initializeProtocolRouter() override;
    
    /**
     * @brief 处理HTTP请求（Redis不需要）
     */
    std::string handleHttpRequest(const std::string& request, int clientFd) override;
    
    /**
     * @brief 处理业务逻辑（主要的Redis命令处理）
     */
    std::string handleBusinessLogic(const std::string& command, const std::vector<std::string>& args) override;
    
    /**
     * @brief 解析请求路径（用于解析Redis命令）
     */
    bool parseRequestPath(const std::string& path, std::string& command, std::vector<std::string>& args) override;
    
    /**
     * @brief 处理网络数据接收
     */
    void onDataReceived(int clientFd, const char* data, size_t len) override;
    
    /**
     * @brief 重写心跳包发送方法，Redis服务器不向客户端发送心跳包
     */
    void sendHeartbeat(int client_fd) override;
    

    
    /**
     * @brief 处理接收到的数据包（核心方法）
     */
    void onPacketReceived(const std::vector<char>& packet);

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
     * @brief 当前处理的客户端FD
     */
    int m_currentClientFd;

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
    void sendRedisResponse(const std::string& response);

    /**
     * @brief 发送原始Redis响应到客户端
     */
    void sendRawRedisResponse(const std::string& response);

    /**
     * @brief 处理PureRedisProtocol的响应回调
     */
    void onPureRedisResponse(const std::vector<char>& packet);

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
