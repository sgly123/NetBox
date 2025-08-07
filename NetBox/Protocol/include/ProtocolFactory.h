#pragma once
#include <memory>
#include <unordered_map>
#include <functional>
#include "ProtocolBase.h"

class ProtocolFactory {
public:
    using Creator = std::function<std::unique_ptr<ProtocolBase>()>;

    // 注册协议类型
    static void registerProtocol(uint32_t protocolId, Creator creator);

    // 创建协议实例
    static std::unique_ptr<ProtocolBase> createProtocol(uint32_t protocolId);

private:
    static std::unordered_map<uint32_t, Creator>& getRegistry();
}; 