#include<gtest/gtest.h>
#include "../base_tool/include/ReadConfig.h"
#include <fstream>

TEST(ConfigReadTest, NormalFile) {
    // 创建临时测试文件
    const std::string test_file = "test_normal.conf";
    {
        std::ofstream out(test_file);
        out << "# 这是注释\n";
        out << "key1=value1\n";
        out << "  key2  =  value2  \n";  // 带前后空格的键值对
        out << "key3=value3\n";  // 行内注释
        out << "\n";  // 空行
    }

    // 加载配置文件
    ReadConfig reader(test_file);

    // 验证键值对
    EXPECT_EQ(reader.getConfigName("key1"), "value1");
    EXPECT_EQ(reader.getConfigName("key2"), "value2");  // 自动去除键值对的前后空格
    EXPECT_EQ(reader.getConfigName("key3"), "value3");  // 忽略行内注释

    // 清理临时文件
    std::remove(test_file.c_str());
}