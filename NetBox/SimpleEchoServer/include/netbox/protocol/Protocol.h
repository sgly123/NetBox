#pragma once

#include <memory>
#include <string>
#include <vector>

namespace NetBox::Protocol {

/**
 * @brief 协议消息基类
 */
class Message {
public:
    virtual ~Message() = default;

    // 消息类型
    virtual std::string getType() const = 0;

    // 序列化
    virtual std::vector<uint8_t> serialize() const = 0;

    // 反序列化
    virtual bool deserialize(const std::vector<uint8_t>& data) = 0;

    // 消息大小
    virtual size_t size() const = 0;

    // 消息ID
    virtual uint32_t getId() const { return 0; }

    // 消息优先级
    virtual int getPriority() const { return 0; }
};

/**
 * @brief 协议编解码器接口
 */
class Codec {
public:
    virtual ~Codec() = default;

    /**
     * @brief 编码消息
     * @param message 要编码的消息
     * @return 编码后的字节流
     */
    virtual std::vector<uint8_t> encode(const Message& message) = 0;

    /**
     * @brief 解码消息
     * @param data 字节流数据
     * @param message 解码后的消息
     * @return 解码是否成功
     */
    virtual bool decode(const std::vector<uint8_t>& data, std::unique_ptr<Message>& message) = 0;

    /**
     * @brief 检查数据完整性
     * @param data 接收到的数据
     * @return 需要的字节数，0表示完整，-1表示错误
     */
    virtual int checkIntegrity(const std::vector<uint8_t>& data) = 0;
};

/**
 * @brief 协议处理器接口
 */
class ProtocolHandler {
public:
    virtual ~ProtocolHandler() = default;

    /**
     * @brief 处理接收到的消息
     */
    virtual void onMessage(std::shared_ptr<Message> message) = 0;

    /**
     * @brief 处理连接建立
     */
    virtual void onConnect() {}

    /**
     * @brief 处理连接断开
     */
    virtual void onDisconnect() {}

    /**
     * @brief 处理错误
     */
    virtual void onError(const std::string& error) {}
};

/**
 * @brief 协议工厂接口
 */
class ProtocolFactory {
public:
    virtual ~ProtocolFactory() = default;

    /**
     * @brief 创建编解码器
     */
    virtual std::unique_ptr<Codec> createCodec() = 0;

    /**
     * @brief 创建协议处理器
     */
    virtual std::unique_ptr<ProtocolHandler> createHandler() = 0;

    /**
     * @brief 获取协议名称
     */
    virtual std::string getProtocolName() const = 0;

    /**
     * @brief 获取协议版本
     */
    virtual std::string getProtocolVersion() const = 0;
};

} // namespace NetBox::Protocol
