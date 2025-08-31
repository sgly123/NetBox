#include <gtest/gtest.h>
#include <cstring>
#include <arpa/inet.h>
#include "Protocol/ProtocolRouter.h"
#include "base/Logger.h"

class HeartbeatFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置日志级别为DEBUG以便查看心跳包过滤日志
        Logger::setLevel(Logger::Level::DEBUG);
    }
};

TEST_F(HeartbeatFilterTest, HeartbeatPacketDetection) {
    ProtocolRouter router;
    
    // 创建心跳包数据（4字节魔数）
    uint32_t heartbeat_magic = 0x12345678;
    uint32_t network_magic = htonl(heartbeat_magic);
    char heartbeat_data[4];
    std::memcpy(heartbeat_data, &network_magic, sizeof(network_magic));
    
    // 测试心跳包识别和过滤
    size_t processed = router.onDataReceived(1, heartbeat_data, sizeof(heartbeat_data));
    
    // 应该返回4字节（心跳包长度），表示心跳包被正确识别和过滤
    EXPECT_EQ(processed, 4);
}

TEST_F(HeartbeatFilterTest, NonHeartbeatPacketPassThrough) {
    ProtocolRouter router;
    
    // 创建非心跳包数据（Redis PING命令）
    std::string redis_data = "PING\r\n";
    
    // 测试非心跳包数据应该能够通过
    size_t processed = router.onDataReceived(1, redis_data.c_str(), redis_data.length());
    
    // 由于没有注册Redis协议，应该返回0（未处理）
    EXPECT_EQ(processed, 0);
}

TEST_F(HeartbeatFilterTest, MixedDataHandling) {
    ProtocolRouter router;
    
    // 创建心跳包数据
    uint32_t heartbeat_magic = 0x12345678;
    uint32_t network_magic = htonl(heartbeat_magic);
    char heartbeat_data[4];
    std::memcpy(heartbeat_data, &network_magic, sizeof(network_magic));
    
    // 测试心跳包
    size_t processed1 = router.onDataReceived(1, heartbeat_data, sizeof(heartbeat_data));
    EXPECT_EQ(processed1, 4);
    
    // 测试Redis数据
    std::string redis_data = "PING\r\n";
    size_t processed2 = router.onDataReceived(1, redis_data.c_str(), redis_data.length());
    EXPECT_EQ(processed2, 0);
}

TEST_F(HeartbeatFilterTest, InvalidHeartbeatSize) {
    ProtocolRouter router;
    
    // 创建错误大小的心跳包数据（3字节，不是4字节）
    char invalid_data[3] = {0x12, 0x34, 0x56};
    
    // 测试错误大小的心跳包应该不会被识别为心跳包
    size_t processed = router.onDataReceived(1, invalid_data, sizeof(invalid_data));
    
    // 应该返回0（未处理），因为不是有效的心跳包
    EXPECT_EQ(processed, 0);
}

TEST_F(HeartbeatFilterTest, ProtocolIdWithHeartbeatMagic) {
    ProtocolRouter router;
    
    // 创建一个包含心跳包魔数作为协议ID的数据包（8字节：4字节协议ID + 4字节数据）
    uint32_t heartbeat_magic = 0x12345678;
    uint32_t network_magic = htonl(heartbeat_magic);
    char protocol_data[8];
    std::memcpy(protocol_data, &network_magic, sizeof(network_magic));
    std::memcpy(protocol_data + 4, "DATA", 4);
    
    // 测试包含心跳包魔数的协议数据包
    size_t processed = router.onDataReceived(1, protocol_data, sizeof(protocol_data));
    
    // 应该返回0（未处理），因为这不是心跳包而是协议数据包
    EXPECT_EQ(processed, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 