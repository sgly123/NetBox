#pragma once
#include <vector>
#include <string>
#include <functional>
#include <memory>

/**
 * @brief 协议层抽象基类，负责拆包、封包、流量控制和回调接口
 *        设计目标：支持多种协议的灵活扩展与解耦，提供完整的协议层功能
 */
class ProtocolBase {
public:
    // 回调函数类型定义
    using PacketCallback = std::function<void(const std::vector<char>&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    
    virtual ~ProtocolBase() = default;

    /**
     * @brief 处理接收到的原始数据流（拆包）
     * @param data 原始数据流
     * @param len 数据长度
     * @return 处理的数据长度
     */
    virtual size_t onDataReceived(const char* data, size_t len) = 0;

    /**
     * @brief 封包要发送的数据
     * @param data 要发送的数据
     * @param len 数据长度
     * @param out 封包后的数据
     * @return 封包是否成功
     */
    virtual bool pack(const char* data, size_t len, std::vector<char>& out) = 0;

    /**
     * @brief 设置数据包回调函数
     * @param callback 当拆出完整包时调用的回调函数
     */
    void setPacketCallback(PacketCallback callback) { packetCallback_ = callback; }

    /**
     * @brief 设置错误回调函数
     * @param callback 当发生协议错误时调用的回调函数
     */
    void setErrorCallback(ErrorCallback callback) { errorCallback_ = callback; }

    /**
     * @brief 设置流量控制参数
     * @param maxReceiveRate 最大接收速率（字节/秒），0表示无限制
     * @param maxSendRate 最大发送速率（字节/秒），0表示无限制
     */
    virtual void setFlowControl(size_t maxReceiveRate = 0, size_t maxSendRate = 0) {
        maxReceiveRate_ = maxReceiveRate;
        maxSendRate_ = maxSendRate;
    }

    /**
     * @brief 获取协议类型标识
     * @return 协议类型字符串
     */
    virtual std::string getType() const = 0;

    /**
     * @brief 获取协议ID（用于分发器路由）
     * @return 协议ID（如uint32_t）
     */
    virtual uint32_t getProtocolId() const = 0;

    /**
     * @brief 重置协议状态（清空缓冲区等）
     */
    virtual void reset() = 0;

protected:
    /**
     * @brief 检查流量控制
     * @param bytes 要处理的字节数
     * @return 是否允许处理
     */
    bool checkFlowControl(size_t bytes);

    /**
     * @brief 获取当前时间戳（毫秒）
     * @return 当前时间戳
     */
    size_t getCurrentTime() const;

    PacketCallback packetCallback_;
    ErrorCallback errorCallback_;
    size_t maxReceiveRate_ = 0;
    size_t maxSendRate_ = 0;
    
    // 流量控制相关
    size_t lastReceiveTime_ = 0;
    size_t lastSendTime_ = 0;
    size_t receiveBytes_ = 0;
    size_t sendBytes_ = 0;
}; 