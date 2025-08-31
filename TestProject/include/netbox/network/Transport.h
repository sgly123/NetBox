#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace NetBox::Network {

/**
 * @brief 网络传输层接口
 */
class Transport {
public:
    virtual ~Transport() = default;

    /**
     * @brief 绑定地址
     */
    virtual bool bind(const std::string& address, int port) = 0;

    /**
     * @brief 开始监听
     */
    virtual bool listen(int backlog = 128) = 0;

    /**
     * @brief 接受连接
     */
    virtual std::shared_ptr<Transport> accept() = 0;

    /**
     * @brief 连接到远程地址
     */
    virtual bool connect(const std::string& address, int port) = 0;

    /**
     * @brief 发送数据
     */
    virtual int send(const std::vector<uint8_t>& data) = 0;

    /**
     * @brief 接收数据
     */
    virtual int receive(std::vector<uint8_t>& data) = 0;

    /**
     * @brief 关闭连接
     */
    virtual void close() = 0;

    /**
     * @brief 获取本地地址
     */
    virtual std::string getLocalAddress() const = 0;

    /**
     * @brief 获取远程地址
     */
    virtual std::string getRemoteAddress() const = 0;

    /**
     * @brief 设置选项
     */
    virtual bool setOption(const std::string& name, const std::string& value) = 0;
};

/**
 * @brief 网络过滤器接口
 */
class Filter {
public:
    virtual ~Filter() = default;

    /**
     * @brief 过滤输入数据
     */
    virtual bool filterInput(std::vector<uint8_t>& data) = 0;

    /**
     * @brief 过滤输出数据
     */
    virtual bool filterOutput(std::vector<uint8_t>& data) = 0;

    /**
     * @brief 获取过滤器名称
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief 网络优化器接口
 */
class Optimizer {
public:
    virtual ~Optimizer() = default;

    /**
     * @brief 优化连接参数
     */
    virtual void optimizeConnection(std::shared_ptr<Transport> transport) = 0;

    /**
     * @brief 优化数据传输
     */
    virtual void optimizeTransfer(std::vector<uint8_t>& data) = 0;

    /**
     * @brief 获取优化统计
     */
    virtual std::string getStats() const = 0;
};

/**
 * @brief 负载均衡器接口
 */
class LoadBalancer {
public:
    virtual ~LoadBalancer() = default;

    /**
     * @brief 添加后端服务器
     */
    virtual void addBackend(const std::string& address, int port, int weight = 1) = 0;

    /**
     * @brief 移除后端服务器
     */
    virtual void removeBackend(const std::string& address, int port) = 0;

    /**
     * @brief 选择后端服务器
     */
    virtual std::pair<std::string, int> selectBackend() = 0;

    /**
     * @brief 更新服务器状态
     */
    virtual void updateBackendStatus(const std::string& address, int port, bool healthy) = 0;
};

} // namespace NetBox::Network
