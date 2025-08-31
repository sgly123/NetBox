/**
 * @file custom_protocol.cpp
 * @brief 自定义协议示例
 *
 * 展示如何扩展NetBox框架，实现自定义协议
 */

#include "netbox/NetBox.h"
#include <iostream>
#include <cstring>

using namespace NetBox;

/**
 * @brief 自定义消息格式
 *
 * 消息格式：
 * [4字节长度][4字节类型][数据]
 */
class CustomMessage : public Protocol::Message {
private:
    uint32_t m_type;
    std::vector<uint8_t> m_data;

public:
    CustomMessage(uint32_t type = 0) : m_type(type) {}

    void setType(uint32_t type) { m_type = type; }
    uint32_t getType() const { return m_type; }

    void setData(const std::vector<uint8_t>& data) { m_data = data; }
    void setData(const std::string& data) {
        m_data.assign(data.begin(), data.end());
    }
    const std::vector<uint8_t>& getData() const { return m_data; }

    // Protocol::Message 接口实现
    std::string getType() const override {
        return "CustomMessage";
    }

    std::vector<uint8_t> serialize() const override {
        std::vector<uint8_t> result;

        // 长度 (4字节)
        uint32_t length = 8 + m_data.size(); // 4(长度) + 4(类型) + 数据
        result.resize(4);
        std::memcpy(result.data(), &length, 4);

        // 类型 (4字节)
        result.resize(8);
        std::memcpy(result.data() + 4, &m_type, 4);

        // 数据
        result.insert(result.end(), m_data.begin(), m_data.end());

        return result;
    }

    bool deserialize(const std::vector<uint8_t>& data) override {
        if (data.size() < 8) return false;

        // 读取长度
        uint32_t length;
        std::memcpy(&length, data.data(), 4);

        if (data.size() < length) return false;

        // 读取类型
        std::memcpy(&m_type, data.data() + 4, 4);

        // 读取数据
        if (length > 8) {
            m_data.assign(data.begin() + 8, data.begin() + length);
        }

        return true;
    }

    size_t size() const override {
        return 8 + m_data.size();
    }

    uint32_t getId() const override {
        return m_type;
    }
};

/**
 * @brief 自定义协议编解码器
 */
class CustomCodec : public Protocol::Codec {
public:
    std::vector<uint8_t> encode(const Protocol::Message& message) override {
        return message.serialize();
    }

    bool decode(const std::vector<uint8_t>& data, std::unique_ptr<Protocol::Message>& message) override {
        auto customMsg = std::make_unique<CustomMessage>();
        if (customMsg->deserialize(data)) {
            message = std::move(customMsg);
            return true;
        }
        return false;
    }

    int checkIntegrity(const std::vector<uint8_t>& data) override {
        if (data.size() < 4) {
            return 4 - data.size(); // 需要更多字节来读取长度
        }

        uint32_t length;
        std::memcpy(&length, data.data(), 4);

        if (data.size() < length) {
            return length - data.size(); // 需要更多字节
        }

        return 0; // 数据完整
    }
};

/**
 * @brief 自定义协议处理器
 */
class CustomProtocolHandler : public Protocol::ProtocolHandler {
public:
    void onMessage(std::shared_ptr<Protocol::Message> message) override {
        auto customMsg = std::dynamic_pointer_cast<CustomMessage>(message);
        if (customMsg) {
            std::cout << "收到自定义消息:" << std::endl;
            std::cout << "  类型: " << customMsg->getType() << std::endl;
            std::cout << "  数据: " << std::string(customMsg->getData().begin(), customMsg->getData().end()) << std::endl;
        }
    }

    void onConnect() override {
        std::cout << "自定义协议连接建立" << std::endl;
    }

    void onDisconnect() override {
        std::cout << "自定义协议连接断开" << std::endl;
    }

    void onError(const std::string& error) override {
        std::cout << "自定义协议错误: " << error << std::endl;
    }
};

/**
 * @brief 自定义协议工厂
 */
class CustomProtocolFactory : public Protocol::ProtocolFactory {
public:
    std::unique_ptr<Protocol::Codec> createCodec() override {
        return std::make_unique<CustomCodec>();
    }

    std::unique_ptr<Protocol::ProtocolHandler> createHandler() override {
        return std::make_unique<CustomProtocolHandler>();
    }

    std::string getProtocolName() const override {
        return "CustomProtocol";
    }

    std::string getProtocolVersion() const override {
        return "1.0.0";
    }
};

int main() {
    std::cout << "🔧 NetBox自定义协议示例" << std::endl;

    // 初始化框架
    if (!NETBOX_INIT()) {
        std::cerr << "❌ NetBox框架初始化失败" << std::endl;
        return 1;
    }

    try {
        // 创建协议工厂
        auto factory = std::make_unique<CustomProtocolFactory>();

        std::cout << "📋 协议信息:" << std::endl;
        std::cout << "  名称: " << factory->getProtocolName() << std::endl;
        std::cout << "  版本: " << factory->getProtocolVersion() << std::endl;

        // 创建编解码器和处理器
        auto codec = factory->createCodec();
        auto handler = factory->createHandler();

        // 测试消息编解码
        std::cout << "\n🧪 测试消息编解码:" << std::endl;

        // 创建测试消息
        CustomMessage testMsg(1001);
        testMsg.setData("Hello, Custom Protocol!");

        // 编码
        auto encoded = codec->encode(testMsg);
        std::cout << "  编码后大小: " << encoded.size() << " 字节" << std::endl;

        // 解码
        std::unique_ptr<Protocol::Message> decoded;
        if (codec->decode(encoded, decoded)) {
            std::cout << "  ✅ 解码成功" << std::endl;
            handler->onMessage(decoded);
        } else {
            std::cout << "  ❌ 解码失败" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ 异常: " << e.what() << std::endl;
    }

    // 清理框架
    NETBOX_CLEANUP();

    std::cout << "\n✅ 自定义协议示例完成" << std::endl;
    return 0;
}