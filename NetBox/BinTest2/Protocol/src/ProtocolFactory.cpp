#include "ProtocolFactory.h"

std::unordered_map<uint32_t, ProtocolFactory::Creator>& ProtocolFactory::getRegistry() {
    static std::unordered_map<uint32_t, Creator> registry;
    return registry;
}

void ProtocolFactory::registerProtocol(uint32_t protocolId, Creator creator) {
    getRegistry()[protocolId] = std::move(creator);
}

std::unique_ptr<ProtocolBase> ProtocolFactory::createProtocol(uint32_t protocolId) {
    auto& reg = getRegistry();
    auto it = reg.find(protocolId);
    if (it != reg.end()) {
        return (it->second)();
    }
    return nullptr;
} 