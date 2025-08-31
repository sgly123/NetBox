#include <gtest/gtest.h>
#include "util/ConfigReader.h"
#include "test_utils.h"
#include <fstream>

class ConfigReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        TestUtils::createTestDataDir();
        test_config_file_ = TestUtils::getTestDataDir() + "test_config.txt";
    }

    void TearDown() override {
        TestUtils::removeTempFile(test_config_file_);
    }

    std::string test_config_file_;
};

// 测试基本配置文件加载
TEST_F(ConfigReaderTest, BasicConfigLoading) {
    // 创建测试配置文件
    std::string config_content = R"(
host=127.0.0.1
port=8888
debug=true
timeout=30
name=test_server
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));
    
    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));
    
    // 测试字符串读取
    EXPECT_EQ(reader.getString("host"), "127.0.0.1");
    EXPECT_EQ(reader.getString("name"), "test_server");
    
    // 测试整数读取
    EXPECT_EQ(reader.getInt("port"), 8888);
    EXPECT_EQ(reader.getInt("timeout"), 30);
    
    // 测试默认值
    EXPECT_EQ(reader.getString("nonexistent", "default"), "default");
    EXPECT_EQ(reader.getInt("nonexistent", 42), 42);
}

// 测试空文件处理
TEST_F(ConfigReaderTest, EmptyFile) {
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, ""));
    
    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));
    
    // 空文件应该返回默认值
    EXPECT_EQ(reader.getString("any_key", "default"), "default");
    EXPECT_EQ(reader.getInt("any_key", 123), 123);
}

// 测试注释处理
TEST_F(ConfigReaderTest, CommentHandling) {
    std::string config_content = R"(
# This is a comment
host=127.0.0.1  # inline comment
# Another comment
port=8888
# debug=false  # commented out config
timeout=30  # timeout setting
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));
    
    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));
    
    EXPECT_EQ(reader.getString("host"), "127.0.0.1");
    EXPECT_EQ(reader.getInt("port"), 8888);
    EXPECT_EQ(reader.getInt("timeout"), 30);
    
    // 被注释的配置不应该存在
    EXPECT_EQ(reader.getString("debug", "not_found"), "not_found");
}

// 测试空行处理
TEST_F(ConfigReaderTest, EmptyLineHandling) {
    std::string config_content = R"(

host=127.0.0.1

port=8888


timeout=30

)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));
    
    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));
    
    EXPECT_EQ(reader.getString("host"), "127.0.0.1");
    EXPECT_EQ(reader.getInt("port"), 8888);
    EXPECT_EQ(reader.getInt("timeout"), 30);
}

// 测试格式错误处理
TEST_F(ConfigReaderTest, MalformedLines) {
    std::string config_content = R"(
host=127.0.0.1
invalid_line_without_equals
port=8888
=value_without_key
key_without_value=
another_key=valid_value
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));
    
    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_)); // 应该成功加载，跳过错误行
    
    // 有效的配置应该被正确读取
    EXPECT_EQ(reader.getString("host"), "127.0.0.1");
    EXPECT_EQ(reader.getInt("port"), 8888);
    EXPECT_EQ(reader.getString("another_key"), "valid_value");
    
    // 空值应该被正确处理
    EXPECT_EQ(reader.getString("key_without_value"), "");
}

// 测试特殊字符处理
TEST_F(ConfigReaderTest, SpecialCharacters) {
    std::string config_content = R"(
path=/home/user/test file with spaces
message=Hello, World! @$%^&*()
unicode=测试中文配置
special_chars=tab	newline
equals_in_value=key=value=more
)";

    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));

    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));

    EXPECT_EQ(reader.getString("path"), "/home/user/test file with spaces");
    EXPECT_EQ(reader.getString("message"), "Hello, World! @$%^&*()");
    EXPECT_EQ(reader.getString("unicode"), "测试中文配置");
    EXPECT_EQ(reader.getString("equals_in_value"), "key=value=more");
}

// 测试数值转换
TEST_F(ConfigReaderTest, NumericConversion) {
    std::string config_content = R"(
positive_int=123
negative_int=-456
zero=0
large_number=2147483647
invalid_number=not_a_number
float_as_int=123.456
hex_number=0xFF
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));
    
    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));
    
    EXPECT_EQ(reader.getInt("positive_int"), 123);
    EXPECT_EQ(reader.getInt("negative_int"), -456);
    EXPECT_EQ(reader.getInt("zero"), 0);
    EXPECT_EQ(reader.getInt("large_number"), 2147483647);
    
    // 无效数字应该返回默认值
    EXPECT_EQ(reader.getInt("invalid_number", 999), 999);
    EXPECT_EQ(reader.getInt("float_as_int", 999), 123); // 应该截断小数部分
}

// 测试文件不存在的情况
TEST_F(ConfigReaderTest, FileNotFound) {
    ConfigReader reader;
    EXPECT_FALSE(reader.load("nonexistent_file.txt"));
    
    // 加载失败后，所有读取都应该返回默认值
    EXPECT_EQ(reader.getString("any_key", "default"), "default");
    EXPECT_EQ(reader.getInt("any_key", 42), 42);
}

// 测试重复键处理
TEST_F(ConfigReaderTest, DuplicateKeys) {
    std::string config_content = R"(
host=first_value
port=8888
host=second_value
host=third_value
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));
    
    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));
    
    // 应该使用最后一个值
    EXPECT_EQ(reader.getString("host"), "third_value");
    EXPECT_EQ(reader.getInt("port"), 8888);
}

// 测试大文件处理
TEST_F(ConfigReaderTest, LargeFile) {
    std::string config_content;
    const int num_entries = 1000;
    
    for (int i = 0; i < num_entries; ++i) {
        config_content += "key" + std::to_string(i) + "=value" + std::to_string(i) + "\n";
    }
    
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));
    
    ConfigReader reader;
    
    double load_time = TestUtils::measureExecutionTime([&]() {
        EXPECT_TRUE(reader.load(test_config_file_));
    });
    
    // 验证所有配置都被正确加载
    for (int i = 0; i < num_entries; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string expected_value = "value" + std::to_string(i);
        EXPECT_EQ(reader.getString(key), expected_value);
    }
    
    std::cout << "Loaded " << num_entries << " config entries in " 
              << load_time << " ms" << std::endl;
    
    // 加载时间应该合理
    EXPECT_LT(load_time, 1000.0); // 应该在1秒内完成
}

// 测试边界情况
TEST_F(ConfigReaderTest, EdgeCases) {
    std::string config_content = R"(
empty_key==value
key_with_spaces=value with spaces
key=
very_long_key_name_that_is_extremely_long_and_might_cause_issues=short_value
short=very_long_value_that_goes_on_and_on_and_might_cause_buffer_issues_in_some_implementations
)";

    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config_content));

    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));

    // 测试各种边界情况
    EXPECT_EQ(reader.getString("empty_key"), "=value");
    EXPECT_EQ(reader.getString("key_with_spaces"), "value with spaces");
    EXPECT_EQ(reader.getString("key"), "");
    EXPECT_EQ(reader.getString("very_long_key_name_that_is_extremely_long_and_might_cause_issues"), "short_value");
    EXPECT_EQ(reader.getString("short"), "very_long_value_that_goes_on_and_on_and_might_cause_buffer_issues_in_some_implementations");
}

// 测试多次加载
TEST_F(ConfigReaderTest, MultipleLoads) {
    // 第一次加载
    std::string config1 = "key1=value1\nkey2=value2\n";
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config1));

    ConfigReader reader;
    EXPECT_TRUE(reader.load(test_config_file_));
    EXPECT_EQ(reader.getString("key1"), "value1");
    EXPECT_EQ(reader.getString("key2"), "value2");

    // 第二次加载不同的配置
    std::string config2 = "key1=new_value1\nkey3=value3\n";
    ASSERT_TRUE(TestUtils::createTempFile(test_config_file_, config2));

    EXPECT_TRUE(reader.load(test_config_file_));
    EXPECT_EQ(reader.getString("key1"), "new_value1");
    EXPECT_EQ(reader.getString("key3"), "value3");

    // 由于ConfigReader不清理旧配置，key2仍然存在
    EXPECT_EQ(reader.getString("key2"), "value2");
}
