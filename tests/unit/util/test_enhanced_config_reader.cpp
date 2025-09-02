#include <gtest/gtest.h>
#include "util/EnhancedConfigReader.h"
#include "test_utils.h"
#include <fstream>
#include <algorithm>

class EnhancedConfigReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        TestUtils::createTestDataDir();
        test_txt_file_ = TestUtils::getTestDataDir() + "test_config.txt";
        test_yaml_file_ = TestUtils::getTestDataDir() + "test_config.yaml";
    }

    void TearDown() override {
        TestUtils::removeTempFile(test_txt_file_);
        TestUtils::removeTempFile(test_yaml_file_);
    }

    std::string test_txt_file_;
    std::string test_yaml_file_;
};

// 测试传统格式配置文件加载
TEST_F(EnhancedConfigReaderTest, TraditionalFormatLoading) {
    std::string config_content = R"(
host=127.0.0.1
port=8888
debug=true
timeout=30.5
name=test_server
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_txt_file_, config_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_txt_file_));
    
    EXPECT_EQ(reader.getString("host"), "127.0.0.1");
    EXPECT_EQ(reader.getInt("port"), 8888);
    EXPECT_EQ(reader.getBool("debug"), true);
    EXPECT_DOUBLE_EQ(reader.getDouble("timeout"), 30.5);
    EXPECT_EQ(reader.getString("name"), "test_server");
}

// 测试YAML格式配置文件加载
TEST_F(EnhancedConfigReaderTest, YamlFormatLoading) {
    std::string yaml_content = R"(
application:
  type: echo
  name: test_server
  debug: true

network:
  ip: 127.0.0.1
  port: 8888
  timeout: 30.5

thread_pool:
  size: 4
  max_queue_size: 1000

logging:
  level: info
  async: false
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_yaml_file_));
    
    // 测试分层配置读取
    EXPECT_EQ(reader.getString("application.type"), "echo");
    EXPECT_EQ(reader.getString("application.name"), "test_server");
    EXPECT_EQ(reader.getBool("application.debug"), true);
    
    EXPECT_EQ(reader.getString("network.ip"), "127.0.0.1");
    EXPECT_EQ(reader.getInt("network.port"), 8888);
    EXPECT_DOUBLE_EQ(reader.getDouble("network.timeout"), 30.5);
    
    EXPECT_EQ(reader.getInt("thread_pool.size"), 4);
    EXPECT_EQ(reader.getInt("thread_pool.max_queue_size"), 1000);
    
    EXPECT_EQ(reader.getString("logging.level"), "info");
    EXPECT_EQ(reader.getBool("logging.async"), false);
}

// 测试文件格式自动检测
TEST_F(EnhancedConfigReaderTest, FormatAutoDetection) {
    // 测试.txt文件
    std::string txt_content = "host=127.0.0.1\nport=8888\n";
    ASSERT_TRUE(TestUtils::createTempFile(test_txt_file_, txt_content));
    
    EnhancedConfigReader reader1;
    EXPECT_TRUE(reader1.load(test_txt_file_));
    EXPECT_EQ(reader1.getString("host"), "127.0.0.1");
    
    // 测试.yaml文件
    std::string yaml_content = "network:\n  host: 192.168.1.1\n  port: 9999\n";
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader2;
    EXPECT_TRUE(reader2.load(test_yaml_file_));
    EXPECT_EQ(reader2.getString("network.host"), "192.168.1.1");
    EXPECT_EQ(reader2.getInt("network.port"), 9999);
}

// 测试布尔值转换
TEST_F(EnhancedConfigReaderTest, BooleanConversion) {
    std::string yaml_content = R"(
flags:
  flag1: true
  flag2: false
  flag3: yes
  flag4: no
  flag5: 1
  flag6: 0
  flag7: on
  flag8: off
  flag9: invalid_bool
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_yaml_file_));
    
    EXPECT_TRUE(reader.getBool("flags.flag1"));
    EXPECT_FALSE(reader.getBool("flags.flag2"));
    EXPECT_TRUE(reader.getBool("flags.flag3"));
    EXPECT_FALSE(reader.getBool("flags.flag4"));
    EXPECT_TRUE(reader.getBool("flags.flag5"));
    EXPECT_FALSE(reader.getBool("flags.flag6"));
    EXPECT_TRUE(reader.getBool("flags.flag7"));
    EXPECT_FALSE(reader.getBool("flags.flag8"));
    
    // 无效布尔值应该返回默认值
    EXPECT_TRUE(reader.getBool("flags.flag9", true));
    EXPECT_FALSE(reader.getBool("flags.flag9", false));
}

// 测试数值转换
TEST_F(EnhancedConfigReaderTest, NumericConversion) {
    std::string yaml_content = R"(
numbers:
  int_positive: 123
  int_negative: -456
  int_zero: 0
  double_positive: 123.456
  double_negative: -789.012
  double_zero: 0.0
  invalid_int: not_a_number
  invalid_double: also_not_a_number
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_yaml_file_));
    
    EXPECT_EQ(reader.getInt("numbers.int_positive"), 123);
    EXPECT_EQ(reader.getInt("numbers.int_negative"), -456);
    EXPECT_EQ(reader.getInt("numbers.int_zero"), 0);
    
    EXPECT_DOUBLE_EQ(reader.getDouble("numbers.double_positive"), 123.456);
    EXPECT_DOUBLE_EQ(reader.getDouble("numbers.double_negative"), -789.012);
    EXPECT_DOUBLE_EQ(reader.getDouble("numbers.double_zero"), 0.0);
    
    // 无效数值应该返回默认值
    EXPECT_EQ(reader.getInt("numbers.invalid_int", 999), 999);
    EXPECT_DOUBLE_EQ(reader.getDouble("numbers.invalid_double", 888.0), 888.0);
}

// 测试键存在性检查
TEST_F(EnhancedConfigReaderTest, KeyExistence) {
    std::string yaml_content = R"(
section1:
  key1: value1
  key2: value2

section2:
  key3: value3
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_yaml_file_));
    
    EXPECT_TRUE(reader.hasKey("section1.key1"));
    EXPECT_TRUE(reader.hasKey("section1.key2"));
    EXPECT_TRUE(reader.hasKey("section2.key3"));
    
    EXPECT_FALSE(reader.hasKey("section1.nonexistent"));
    EXPECT_FALSE(reader.hasKey("nonexistent.key"));
    EXPECT_FALSE(reader.hasKey("nonexistent"));
}

// 测试获取所有键
TEST_F(EnhancedConfigReaderTest, GetAllKeys) {
    std::string yaml_content = R"(
app:
  name: test
  version: 1.0

db:
  host: localhost
  port: 5432
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_yaml_file_));
    
    auto keys = reader.getAllKeys();
    EXPECT_EQ(keys.size(), 4);
    
    // 键应该按字母顺序排列
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "app.name") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "app.version") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "db.host") != keys.end());
    EXPECT_TRUE(std::find(keys.begin(), keys.end(), "db.port") != keys.end());
}

// 测试前缀匹配
TEST_F(EnhancedConfigReaderTest, GetKeysWithPrefix) {
    std::string yaml_content = R"(
app:
  name: test
  version: 1.0
  debug: true

database:
  host: localhost
  port: 5432
  name: testdb
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_yaml_file_));
    
    auto app_keys = reader.getKeysWithPrefix("app.");
    EXPECT_EQ(app_keys.size(), 3);
    EXPECT_EQ(app_keys["app.name"], "test");
    EXPECT_EQ(app_keys["app.version"], "1.0");
    EXPECT_EQ(app_keys["app.debug"], "true");
    
    auto db_keys = reader.getKeysWithPrefix("database.");
    EXPECT_EQ(db_keys.size(), 3);
    EXPECT_EQ(db_keys["database.host"], "localhost");
    EXPECT_EQ(db_keys["database.port"], "5432");
    EXPECT_EQ(db_keys["database.name"], "testdb");
    
    auto empty_keys = reader.getKeysWithPrefix("nonexistent.");
    EXPECT_TRUE(empty_keys.empty());
}

// 测试配置清空和大小
TEST_F(EnhancedConfigReaderTest, ClearAndSize) {
    std::string yaml_content = R"(
key1: value1
key2: value2
key3: value3
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_yaml_file_));
    
    EXPECT_EQ(reader.size(), 3);
    EXPECT_TRUE(reader.hasKey("key1"));
    
    reader.clear();
    EXPECT_EQ(reader.size(), 0);
    EXPECT_FALSE(reader.hasKey("key1"));
    
    // 清空后应该返回默认值
    EXPECT_EQ(reader.getString("key1", "default"), "default");
}

// 测试YAML注释处理
TEST_F(EnhancedConfigReaderTest, YamlCommentHandling) {
    std::string yaml_content = R"(
# This is a comment
app:
  name: test  # inline comment
  # version: 2.0  # commented out
  debug: true

# Another section comment
network:
  port: 8888  # port setting
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    EXPECT_TRUE(reader.load(test_yaml_file_));
    
    EXPECT_EQ(reader.getString("app.name"), "test");
    EXPECT_EQ(reader.getBool("app.debug"), true);
    EXPECT_EQ(reader.getInt("network.port"), 8888);
    
    // 被注释的配置不应该存在
    EXPECT_FALSE(reader.hasKey("app.version"));
}

// 测试错误处理
TEST_F(EnhancedConfigReaderTest, ErrorHandling) {
    EnhancedConfigReader reader;
    
    // 测试不存在的文件
    EXPECT_FALSE(reader.load("nonexistent_file.yaml"));
    
    // 测试格式错误的YAML
    std::string invalid_yaml = R"(
app:
  name: test
    invalid_indentation: value
  port: 8888
)";
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, invalid_yaml));
    
    // 应该能够处理部分错误，加载有效的部分
    EXPECT_TRUE(reader.load(test_yaml_file_));
    EXPECT_EQ(reader.getString("app.name"), "test");
}

// 测试默认值
TEST_F(EnhancedConfigReaderTest, DefaultValues) {
    EnhancedConfigReader reader; // 空的配置读取器
    
    EXPECT_EQ(reader.getString("nonexistent", "default_string"), "default_string");
    EXPECT_EQ(reader.getInt("nonexistent", 42), 42);
    EXPECT_EQ(reader.getBool("nonexistent", true), true);
    EXPECT_DOUBLE_EQ(reader.getDouble("nonexistent", 3.14), 3.14);
}

// 测试大型配置文件
TEST_F(EnhancedConfigReaderTest, LargeConfigFile) {
    std::string yaml_content;
    const int num_sections = 100;
    const int keys_per_section = 10;
    
    for (int i = 0; i < num_sections; ++i) {
        yaml_content += "section" + std::to_string(i) + ":\n";
        for (int j = 0; j < keys_per_section; ++j) {
            yaml_content += "  key" + std::to_string(j) + ": value" + std::to_string(i * keys_per_section + j) + "\n";
        }
    }
    
    ASSERT_TRUE(TestUtils::createTempFile(test_yaml_file_, yaml_content));
    
    EnhancedConfigReader reader;
    
    double load_time = TestUtils::measureExecutionTime([&]() {
        EXPECT_TRUE(reader.load(test_yaml_file_));
    });
    
    EXPECT_EQ(reader.size(), num_sections * keys_per_section);
    
    // 验证一些随机配置
    EXPECT_EQ(reader.getString("section0.key0"), "value0");
    EXPECT_EQ(reader.getString("section50.key5"), "value505");
    EXPECT_EQ(reader.getString("section99.key9"), "value999");
    
    std::cout << "Loaded " << (num_sections * keys_per_section) 
              << " config entries in " << load_time << " ms" << std::endl;
    
    EXPECT_LT(load_time, 2000.0); // 应该在2秒内完成
}
