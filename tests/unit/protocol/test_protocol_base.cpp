#include <gtest/gtest.h>
#include "test_utils.h"

// 协议基类测试占位符
class ProtocolBaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试设置
    }

    void TearDown() override {
        // 测试清理
    }
};

// 基本测试用例
TEST_F(ProtocolBaseTest, BasicTest) {
    // 这是一个占位符测试
    EXPECT_TRUE(true);
}
