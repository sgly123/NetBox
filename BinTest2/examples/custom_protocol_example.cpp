/**
 * @file custom_protocol_example.cpp
 * @brief BinTest2 - 自定义协议开发示例
 * 
 * 这个示例展示了如何基于NetBox框架开发自定义协议：
 * 1. 继承ProtocolBase类
 * 2. 实现协议解析和封包逻辑
 * 3. 注册到ProtocolRouter
 * 4. 处理协议数据
 */

#include <iostream>
#include <vector>
#include <string>
#include "Protocol/include/ProtocolBase.h"
#include "Protocol/include/ProtocolRouter.h"
#include "Protocol/include/ProtocolFactory.h"

// 自定义协议ID
constexpr uint32_t CUSTOM_PROTOCOL_ID = 1001;

/**
 * @brief 自定义协议实现
 * 
 * 协议格式：
 * - 前4字节：数据长度（大端序）
 * - 后续数据：协议内容
 */
class CustomProtocol : public ProtocolBase {
public:
    CustomProtocol() {
        std::cout << "🔧 创建自定义协议实例" << std::endl;
    }
    
    ~CustomProtocol() override {
        std::cout << "🧹 销毁自定义协议实例" << std::endl;
    }
    
    std::string getType() const override {
        return "CustomProtocol";
    }
    
    uint32_t getProtocolId() const override {
        return CUSTOM_PROTOCOL_ID;
    }
    
    size_t onDataReceived(const char* data, size_t len) override {
        if (len < 4) {
            // 数据不足，等待更多数据
            return 0;
        }
        
        // 解析数据长度（大端序）
        uint32_t dataLen = (static_cast<uint32_t>(data[0]) << 24) |
                          (static_cast<uint32_t>(data[1]) << 16) |
                          (static_cast<uint32_t>(data[2]) << 8) |
                          static_cast<uint32_t>(data[3]);
        
        if (len < dataLen + 4) {
            // 数据不完整，等待更多数据
            return 0;
        }
        
        // 提取协议数据
        std::string protocolData(data + 4, dataLen);
        
        std::cout << "📨 收到自定义协议数据: " << protocolData << std::endl;
        
        // 调用回调函数
        if (packetCallback_) {
            std::vector<char> packet(data, data + dataLen + 4);
            packetCallback_(packet);
        }
        
        return dataLen + 4;  // 返回处理的字节数
    }
    
    bool pack(const char* data, size_t len, std::vector<char>& out) override {
        if (len > 0xFFFFFFFF) {
            // 数据太长
            if (errorCallback_) {
                errorCallback_("Data too long for custom protocol");
            }
            return false;
        }
        
        // 分配输出缓冲区
        out.resize(len + 4);
        
        // 写入数据长度（大端序）
        out[0] = static_cast<char>((len >> 24) & 0xFF);
        out[1] = static_cast<char>((len >> 16) & 0xFF);
        out[2] = static_cast<char>((len >> 8) & 0xFF);
        out[3] = static_cast<char>(len & 0xFF);
        
        // 复制数据
        std::copy(data, data + len, out.begin() + 4);
        
        std::cout << "📤 封包自定义协议数据，长度: " << len << std::endl;
        
        return true;
    }
    
    void reset() override {
        std::cout << "🔄 重置自定义协议状态" << std::endl;
    }
};

int main() {
    std::cout << "🎯 BinTest2 自定义协议开发示例" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // 创建协议路由器
    ProtocolRouter router;
    
    // 创建自定义协议实例
    auto customProtocol = std::make_shared<CustomProtocol>();
    
    // 注册协议
    router.registerProtocol(CUSTOM_PROTOCOL_ID, customProtocol);
    
    // 设置数据包回调
    router.setPacketCallback([](uint32_t protoId, const std::vector<char>& packet) {
        std::cout << "📦 收到协议数据包，ID: " << protoId << ", 大小: " << packet.size() << " 字节" << std::endl;
    });
    
    // 设置错误回调
    router.setErrorCallback([](const std::string& error) {
        std::cerr << "❌ 协议错误: " << error << std::endl;
    });
    
    // 测试协议封包
    std::string testData = "Hello, Custom Protocol!";
    std::vector<char> packedData;
    
    if (customProtocol->pack(testData.c_str(), testData.length(), packedData)) {
        std::cout << "✅ 协议封包成功" << std::endl;
        
        // 测试协议解析
        size_t processed = router.onDataReceived(0, packedData.data(), packedData.size());
        std::cout << "✅ 协议解析成功，处理了 " << processed << " 字节" << std::endl;
    }
    
    // 测试协议工厂
    std::cout << "\n🔧 测试协议工厂..." << std::endl;
    
    // 注册协议到工厂
    ProtocolFactory::registerProtocol(CUSTOM_PROTOCOL_ID, []() {
        return std::make_unique<CustomProtocol>();
    });
    
    // 通过工厂创建协议
    auto factoryProtocol = ProtocolFactory::createProtocol(CUSTOM_PROTOCOL_ID);
    if (factoryProtocol) {
        std::cout << "✅ 协议工厂创建成功，类型: " << factoryProtocol->getType() << std::endl;
    }
    
    std::cout << "\n🎉 自定义协议开发示例完成!" << std::endl;
    std::cout << "按Enter键退出..." << std::endl;
    std::cin.get();
    
    return 0;
}
