#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>
#include "Protocol/ProtocolRouter.h"
#include "base/Logger.h"

class HeartbeatIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::setLevel(Logger::Level::DEBUG);
    }
};

TEST_F(HeartbeatIntegrationTest, HeartbeatWithRedisProtocol) {
    ProtocolRouter router;
    
    // 注册Redis协议处理器
    auto redisProto = std::make_shared<PureRedisProtocol>();
    router.registerProtocol(redisProto->getProtocolId(), redisProto);
    
    // 测试1：发送心跳包
    uint32_t heartbeat_magic = 0x12345678;
    uint32_t network_magic = htonl(heartbeat_magic);
    char heartbeat_data[4];
    std::memcpy(heartbeat_data, &network_magic, sizeof(network_magic));
    
    size_t processed1 = router.onDataReceived(1, heartbeat_data, sizeof(heartbeat_data));
    EXPECT_EQ(processed1, 4); // 心跳包应该被过滤，返回4字节
    
    // 测试2：发送Redis PING命令
    std::string redis_ping = "PING\r\n";
    size_t processed2 = router.onDataReceived(1, redis_ping.c_str(), redis_ping.length());
    EXPECT_GT(processed2, 0); // Redis命令应该被正常处理
    
    // 测试3：发送Redis SET命令
    std::string redis_set = "SET name test\r\n";
    size_t processed3 = router.onDataReceived(1, redis_set.c_str(), redis_set.length());
    EXPECT_GT(processed3, 0); // Redis命令应该被正常处理
}

TEST_F(HeartbeatIntegrationTest, MixedDataStream) {
    ProtocolRouter router;
    
    // 注册Redis协议处理器
    auto redisProto = std::make_shared<PureRedisProtocol>();
    router.registerProtocol(redisProto->getProtocolId(), redisProto);
    
    // 模拟混合数据流：心跳包 + Redis命令 + 心跳包 + Redis命令
    std::vector<std::pair<std::string, size_t>> test_data = {
        // 心跳包
        {std::string(reinterpret_cast<const char*>(&htonl(0x12345678)), 4), 4},
        // Redis命令
        {"PING\r\n", 6},
        // 心跳包
        {std::string(reinterpret_cast<const char*>(&htonl(0x12345678)), 4), 4},
        // Redis命令
        {"SET key value\r\n", 15}
    };
    
    for (const auto& data : test_data) {
        size_t processed = router.onDataReceived(1, data.first.c_str(), data.first.length());
        
        if (data.first.length() == 4) {
            // 心跳包
            EXPECT_EQ(processed, 4);
        } else {
            // Redis命令
            EXPECT_GT(processed, 0);
        }
    }
}

TEST_F(HeartbeatIntegrationTest, HeartbeatDetectionAccuracy) {
    ProtocolRouter router;
    
    // 测试各种数据格式，确保只有真正的心跳包被识别
    std::vector<std::pair<std::string, bool>> test_cases = {
        // 格式: {数据, 是否应该被识别为心跳包}
        {std::string(reinterpret_cast<const char*>(&htonl(0x12345678)), 4), true},  // 正确的心跳包
        {"PING\r\n", false},  // Redis命令
        {"SET key value\r\n", false},  // Redis命令
        {"GET key\r\n", false},  // Redis命令
        {std::string(reinterpret_cast<const char*>(&htonl(0x12345678)), 4) + "EXTRA", false},  // 心跳包魔数+额外数据
        {"1234", false},  // 4字节但不是心跳包魔数
        {"", false},  // 空数据
    };
    
    for (const auto& test_case : test_cases) {
        size_t processed = router.onDataReceived(1, test_case.first.c_str(), test_case.first.length());
        
        if (test_case.second) {
            // 应该被识别为心跳包
            EXPECT_EQ(processed, 4);
        } else {
            // 不应该被识别为心跳包
            EXPECT_NE(processed, 4);
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 