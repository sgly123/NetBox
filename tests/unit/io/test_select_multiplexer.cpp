#include <gtest/gtest.h>
#include "IO/SelectMultiplexer.h"
#include "test_utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

class SelectMultiplexerTest : public ::testing::Test {
protected:
    void SetUp() override {
        select_multiplexer_ = std::make_unique<SelectMultiplexer>();
        
        // 创建测试用的socket对
        int fds[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
        read_fd_ = fds[0];
        write_fd_ = fds[1];
        
        // 设置为非阻塞模式
        setNonBlocking(read_fd_);
        setNonBlocking(write_fd_);
    }

    void TearDown() override {
        if (read_fd_ >= 0) {
            close(read_fd_);
        }
        if (write_fd_ >= 0) {
            close(write_fd_);
        }
        select_multiplexer_.reset();
    }

    void setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        ASSERT_NE(flags, -1);
        ASSERT_NE(fcntl(fd, F_SETFL, flags | O_NONBLOCK), -1);
    }

    std::unique_ptr<SelectMultiplexer> select_multiplexer_;
    int read_fd_ = -1;
    int write_fd_ = -1;
};

// 测试SelectMultiplexer基本功能
TEST_F(SelectMultiplexerTest, BasicFunctionality) {
    EXPECT_TRUE(select_multiplexer_->init());
    EXPECT_EQ(select_multiplexer_->type(), IOMultiplexer::IOType::SELECT);
}

// 测试添加文件描述符
TEST_F(SelectMultiplexerTest, AddFileDescriptor) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 添加读事件
    EXPECT_TRUE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 添加写事件
    EXPECT_TRUE(select_multiplexer_->addfd(write_fd_, IOMultiplexer::EventType::WRITE));
    
    // 尝试添加无效文件描述符
    EXPECT_FALSE(select_multiplexer_->addfd(-1, IOMultiplexer::EventType::READ));
}

// 测试重复添加文件描述符
TEST_F(SelectMultiplexerTest, DuplicateAddFileDescriptor) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 第一次添加应该成功
    EXPECT_TRUE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 重复添加应该失败
    EXPECT_FALSE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::WRITE));
}

// 测试移除文件描述符
TEST_F(SelectMultiplexerTest, RemoveFileDescriptor) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 先添加文件描述符
    EXPECT_TRUE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 然后移除
    EXPECT_TRUE(select_multiplexer_->removefd(read_fd_));
    
    // 尝试移除不存在的文件描述符
    EXPECT_FALSE(select_multiplexer_->removefd(999));
    
    // 尝试移除已经移除的文件描述符
    EXPECT_FALSE(select_multiplexer_->removefd(read_fd_));
}

// 测试修改文件描述符事件
TEST_F(SelectMultiplexerTest, ModifyFileDescriptor) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 先添加文件描述符
    EXPECT_TRUE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 修改事件类型
    EXPECT_TRUE(select_multiplexer_->modifyFd(read_fd_, IOMultiplexer::EventType::WRITE));
    
    // 修改为复合事件
    EXPECT_TRUE(select_multiplexer_->modifyFd(read_fd_, 
        IOMultiplexer::EventType::READ | IOMultiplexer::EventType::WRITE));
    
    // 尝试修改不存在的文件描述符
    EXPECT_FALSE(select_multiplexer_->modifyFd(999, IOMultiplexer::EventType::READ));
}

// 测试等待事件
TEST_F(SelectMultiplexerTest, WaitForEvents) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 添加写事件（写事件通常立即就绪）
    EXPECT_TRUE(select_multiplexer_->addfd(write_fd_, IOMultiplexer::EventType::WRITE));
    
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = select_multiplexer_->wait(active_events, 100);
    
    EXPECT_GT(result, 0);
    EXPECT_FALSE(active_events.empty());
    
    // 检查写事件是否就绪
    bool write_ready = false;
    for (const auto& event : active_events) {
        if (event.first == write_fd_ && (event.second & IOMultiplexer::EventType::WRITE)) {
            write_ready = true;
            break;
        }
    }
    EXPECT_TRUE(write_ready);
}

// 测试读事件触发
TEST_F(SelectMultiplexerTest, ReadEventTrigger) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 添加读事件监听
    EXPECT_TRUE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 初始状态下，读事件不应该就绪
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = select_multiplexer_->wait(active_events, 10);
    
    // 检查是否有读事件就绪
    bool read_ready = false;
    for (const auto& event : active_events) {
        if (event.first == read_fd_ && (event.second & IOMultiplexer::EventType::READ)) {
            read_ready = true;
            break;
        }
    }
    EXPECT_FALSE(read_ready);
    
    // 向写端写入数据
    const char* test_data = "select test";
    ssize_t written = write(write_fd_, test_data, 11);
    EXPECT_EQ(written, 11);
    
    // 现在读事件应该就绪
    active_events.clear();
    result = select_multiplexer_->wait(active_events, 100);
    EXPECT_GT(result, 0);
    
    read_ready = false;
    for (const auto& event : active_events) {
        if (event.first == read_fd_ && (event.second & IOMultiplexer::EventType::READ)) {
            read_ready = true;
            break;
        }
    }
    EXPECT_TRUE(read_ready);
    
    // 读取数据验证
    char buffer[20];
    ssize_t bytes_read = read(read_fd_, buffer, sizeof(buffer));
    EXPECT_EQ(bytes_read, 11);
    EXPECT_EQ(std::string(buffer, 11), "select test");
}

// 测试超时功能
TEST_F(SelectMultiplexerTest, TimeoutFunctionality) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 添加读事件（但不会有数据到来）
    EXPECT_TRUE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    
    double wait_time = TestUtils::measureExecutionTime([&]() {
        int result = select_multiplexer_->wait(active_events, 100);
        EXPECT_EQ(result, 0); // 应该超时
    });
    
    // 等待时间应该接近100ms
    EXPECT_GE(wait_time, 90.0);
    EXPECT_LE(wait_time, 150.0);
    EXPECT_TRUE(active_events.empty());
}

// 测试空文件描述符集合
TEST_F(SelectMultiplexerTest, EmptyFdSet) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 不添加任何文件描述符
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = select_multiplexer_->wait(active_events, 10);
    
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(active_events.empty());
}

// 测试多个文件描述符
TEST_F(SelectMultiplexerTest, MultipleFileDescriptors) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 创建多个socket对
    std::vector<std::pair<int, int>> socket_pairs;
    const int num_pairs = 5; // select有FD_SETSIZE限制，不要创建太多
    
    for (int i = 0; i < num_pairs; ++i) {
        int fds[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
        setNonBlocking(fds[0]);
        setNonBlocking(fds[1]);
        
        socket_pairs.push_back({fds[0], fds[1]});
        
        // 添加读事件监听
        EXPECT_TRUE(select_multiplexer_->addfd(fds[0], IOMultiplexer::EventType::READ));
    }
    
    // 向所有写端写入数据
    for (int i = 0; i < num_pairs; ++i) {
        std::string data = "data" + std::to_string(i);
        write(socket_pairs[i].second, data.c_str(), data.length());
    }
    
    // 等待事件
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = select_multiplexer_->wait(active_events, 100);
    
    EXPECT_EQ(result, num_pairs);
    EXPECT_EQ(active_events.size(), num_pairs);
    
    // 验证所有读事件都就绪
    std::set<int> ready_fds;
    for (const auto& event : active_events) {
        if (event.second & IOMultiplexer::EventType::READ) {
            ready_fds.insert(event.first);
        }
    }
    EXPECT_EQ(ready_fds.size(), num_pairs);
    
    // 清理
    for (const auto& pair : socket_pairs) {
        select_multiplexer_->removefd(pair.first);
        close(pair.first);
        close(pair.second);
    }
}

// 测试读写事件组合
TEST_F(SelectMultiplexerTest, ReadWriteEventsCombination) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 添加读写事件
    EXPECT_TRUE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    EXPECT_TRUE(select_multiplexer_->addfd(write_fd_, IOMultiplexer::EventType::WRITE));
    
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = select_multiplexer_->wait(active_events, 100);
    
    EXPECT_GT(result, 0);
    
    // 写事件应该就绪
    bool write_ready = false;
    bool read_ready = false;
    
    for (const auto& event : active_events) {
        if (event.first == write_fd_ && (event.second & IOMultiplexer::EventType::WRITE)) {
            write_ready = true;
        }
        if (event.first == read_fd_ && (event.second & IOMultiplexer::EventType::READ)) {
            read_ready = true;
        }
    }
    
    EXPECT_TRUE(write_ready);
    EXPECT_FALSE(read_ready); // 初始状态下读事件不应该就绪
}

// 测试maxFd更新
TEST_F(SelectMultiplexerTest, MaxFdUpdate) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 添加一个较大的文件描述符
    int large_fd = dup(read_fd_); // 创建一个新的文件描述符
    ASSERT_GE(large_fd, 0);
    
    EXPECT_TRUE(select_multiplexer_->addfd(large_fd, IOMultiplexer::EventType::READ));
    
    // 移除较大的文件描述符，测试maxFd是否正确更新
    EXPECT_TRUE(select_multiplexer_->removefd(large_fd));
    
    // 添加一个较小的文件描述符
    EXPECT_TRUE(select_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 应该仍然能正常工作
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = select_multiplexer_->wait(active_events, 10);
    EXPECT_GE(result, 0);
    
    close(large_fd);
}

// 测试错误处理
TEST_F(SelectMultiplexerTest, ErrorHandling) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    // 测试添加无效文件描述符
    EXPECT_FALSE(select_multiplexer_->addfd(-1, IOMultiplexer::EventType::READ));
    
    // 测试移除不存在的文件描述符
    EXPECT_FALSE(select_multiplexer_->removefd(999));
    
    // 测试修改不存在的文件描述符
    EXPECT_FALSE(select_multiplexer_->modifyFd(999, IOMultiplexer::EventType::READ));
}

// 测试性能（相对于epoll较低）
TEST_F(SelectMultiplexerTest, Performance) {
    ASSERT_TRUE(select_multiplexer_->init());
    
    const int num_fds = 50; // select性能较低，测试较少的文件描述符
    std::vector<int> test_fds;
    
    // 创建测试文件描述符
    for (int i = 0; i < num_fds; ++i) {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0) {
            test_fds.push_back(fds[0]);
            close(fds[1]); // 只保留一个
        }
    }
    
    double add_time = TestUtils::measureExecutionTime([&]() {
        for (int fd : test_fds) {
            select_multiplexer_->addfd(fd, IOMultiplexer::EventType::READ);
        }
    });
    
    double remove_time = TestUtils::measureExecutionTime([&]() {
        for (int fd : test_fds) {
            select_multiplexer_->removefd(fd);
        }
    });
    
    // 清理
    for (int fd : test_fds) {
        close(fd);
    }
    
    std::cout << "Select: Added " << test_fds.size() << " fds in " << add_time << " ms" << std::endl;
    std::cout << "Select: Removed " << test_fds.size() << " fds in " << remove_time << " ms" << std::endl;
    
    // select性能相对较低，但应该在合理范围内
    EXPECT_LT(add_time, 500.0);
    EXPECT_LT(remove_time, 500.0);
}
