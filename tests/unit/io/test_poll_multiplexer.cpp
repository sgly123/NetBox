#include <gtest/gtest.h>
#include "IO/PollMultiplexer.h"
#include "test_utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

class PollMultiplexerTest : public ::testing::Test {
protected:
    void SetUp() override {
        poll_multiplexer_ = std::make_unique<PollMultiplexer>();
        
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
        poll_multiplexer_.reset();
    }

    void setNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        ASSERT_NE(flags, -1);
        ASSERT_NE(fcntl(fd, F_SETFL, flags | O_NONBLOCK), -1);
    }

    std::unique_ptr<PollMultiplexer> poll_multiplexer_;
    int read_fd_ = -1;
    int write_fd_ = -1;
};

// 测试PollMultiplexer基本功能
TEST_F(PollMultiplexerTest, BasicFunctionality) {
    EXPECT_TRUE(poll_multiplexer_->init());
    EXPECT_EQ(poll_multiplexer_->type(), IOMultiplexer::IOType::POLL);
}

// 测试添加文件描述符
TEST_F(PollMultiplexerTest, AddFileDescriptor) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 添加读事件
    EXPECT_TRUE(poll_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 添加写事件
    EXPECT_TRUE(poll_multiplexer_->addfd(write_fd_, IOMultiplexer::EventType::WRITE));
    
    // 尝试添加无效文件描述符
    EXPECT_FALSE(poll_multiplexer_->addfd(-1, IOMultiplexer::EventType::READ));
}

// 测试重复添加文件描述符
TEST_F(PollMultiplexerTest, DuplicateAddFileDescriptor) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 第一次添加应该成功
    EXPECT_TRUE(poll_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 重复添加应该失败
    EXPECT_FALSE(poll_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::WRITE));
    
    // 验证错误类型
    EXPECT_EQ(poll_multiplexer_->lastError(), PollMultiplexer::Error::FD_ALREADY_EXIST);
}

// 测试移除文件描述符
TEST_F(PollMultiplexerTest, RemoveFileDescriptor) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 先添加文件描述符
    EXPECT_TRUE(poll_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 然后移除
    EXPECT_TRUE(poll_multiplexer_->removefd(read_fd_));
    
    // 尝试移除不存在的文件描述符
    EXPECT_FALSE(poll_multiplexer_->removefd(999));
    EXPECT_EQ(poll_multiplexer_->lastError(), PollMultiplexer::Error::FD_NOT_FOUND);
    
    // 尝试移除已经移除的文件描述符
    EXPECT_FALSE(poll_multiplexer_->removefd(read_fd_));
    EXPECT_EQ(poll_multiplexer_->lastError(), PollMultiplexer::Error::FD_NOT_FOUND);
}

// 测试修改文件描述符事件
TEST_F(PollMultiplexerTest, ModifyFileDescriptor) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 先添加文件描述符
    EXPECT_TRUE(poll_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 修改事件类型
    EXPECT_TRUE(poll_multiplexer_->modifyFd(read_fd_, IOMultiplexer::EventType::WRITE));
    
    // 修改为复合事件
    EXPECT_TRUE(poll_multiplexer_->modifyFd(read_fd_, 
        IOMultiplexer::EventType::READ | IOMultiplexer::EventType::WRITE));
    
    // 尝试修改不存在的文件描述符
    EXPECT_FALSE(poll_multiplexer_->modifyFd(999, IOMultiplexer::EventType::READ));
    EXPECT_EQ(poll_multiplexer_->lastError(), PollMultiplexer::Error::FD_NOT_FOUND);
}

// 测试错误字符串
TEST_F(PollMultiplexerTest, ErrorString) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 触发一个错误
    EXPECT_FALSE(poll_multiplexer_->addfd(-1, IOMultiplexer::EventType::READ));
    
    // 获取错误字符串
    std::string error_str = poll_multiplexer_->errorString();
    EXPECT_FALSE(error_str.empty());
    EXPECT_NE(error_str.find("Invalid"), std::string::npos);
}

// 测试等待事件
TEST_F(PollMultiplexerTest, WaitForEvents) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 添加写事件（写事件通常立即就绪）
    EXPECT_TRUE(poll_multiplexer_->addfd(write_fd_, IOMultiplexer::EventType::WRITE));
    
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = poll_multiplexer_->wait(active_events, 100);
    
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
TEST_F(PollMultiplexerTest, ReadEventTrigger) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 添加读事件监听
    EXPECT_TRUE(poll_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 初始状态下，读事件不应该就绪
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = poll_multiplexer_->wait(active_events, 10);
    
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
    const char* test_data = "poll test data";
    ssize_t written = write(write_fd_, test_data, 14);
    EXPECT_EQ(written, 14);
    
    // 现在读事件应该就绪
    active_events.clear();
    result = poll_multiplexer_->wait(active_events, 100);
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
    EXPECT_EQ(bytes_read, 14);
    EXPECT_EQ(std::string(buffer, 14), "poll test data");
}

// 测试超时功能
TEST_F(PollMultiplexerTest, TimeoutFunctionality) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 添加读事件（但不会有数据到来）
    EXPECT_TRUE(poll_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    
    double wait_time = TestUtils::measureExecutionTime([&]() {
        int result = poll_multiplexer_->wait(active_events, 100);
        EXPECT_EQ(result, 0); // 应该超时
    });
    
    // 等待时间应该接近100ms
    EXPECT_GE(wait_time, 90.0);
    EXPECT_LE(wait_time, 150.0);
    EXPECT_TRUE(active_events.empty());
    EXPECT_EQ(poll_multiplexer_->lastError(), PollMultiplexer::Error::TIMEOUT);
}

// 测试多个文件描述符
TEST_F(PollMultiplexerTest, MultipleFileDescriptors) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 创建多个socket对
    std::vector<std::pair<int, int>> socket_pairs;
    const int num_pairs = 20; // poll可以处理更多文件描述符
    
    for (int i = 0; i < num_pairs; ++i) {
        int fds[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
        setNonBlocking(fds[0]);
        setNonBlocking(fds[1]);
        
        socket_pairs.push_back({fds[0], fds[1]});
        
        // 添加读事件监听
        EXPECT_TRUE(poll_multiplexer_->addfd(fds[0], IOMultiplexer::EventType::READ));
    }
    
    // 向所有写端写入数据
    for (int i = 0; i < num_pairs; ++i) {
        std::string data = "data" + std::to_string(i);
        write(socket_pairs[i].second, data.c_str(), data.length());
    }
    
    // 等待事件
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = poll_multiplexer_->wait(active_events, 100);
    
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
        poll_multiplexer_->removefd(pair.first);
        close(pair.first);
        close(pair.second);
    }
}

// 测试错误事件处理
TEST_F(PollMultiplexerTest, ErrorEventHandling) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 添加读事件监听
    EXPECT_TRUE(poll_multiplexer_->addfd(read_fd_, IOMultiplexer::EventType::READ));
    
    // 关闭写端，这应该触发POLLHUP事件
    close(write_fd_);
    write_fd_ = -1;
    
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = poll_multiplexer_->wait(active_events, 100);
    
    if (result > 0) {
        // 应该收到错误事件或读事件（EOF）
        bool error_or_read = false;
        for (const auto& event : active_events) {
            if (event.first == read_fd_) {
                if ((event.second & IOMultiplexer::EventType::ERROR) ||
                    (event.second & IOMultiplexer::EventType::READ)) {
                    error_or_read = true;
                    break;
                }
            }
        }
        EXPECT_TRUE(error_or_read);
    }
}

// 测试动态扩容
TEST_F(PollMultiplexerTest, DynamicResize) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 创建一个较大的文件描述符来测试动态扩容
    std::vector<int> test_fds;
    
    // 创建一些文件描述符
    for (int i = 0; i < 10; ++i) {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0) {
            test_fds.push_back(fds[0]);
            close(fds[1]);
            
            // 添加到poll multiplexer
            EXPECT_TRUE(poll_multiplexer_->addfd(fds[0], IOMultiplexer::EventType::READ));
        }
    }
    
    // 创建一个很大的文件描述符（通过dup多次）
    int large_fd = read_fd_;
    for (int i = 0; i < 100; ++i) {
        int new_fd = dup(large_fd);
        if (new_fd > large_fd) {
            close(large_fd);
            large_fd = new_fd;
        } else {
            close(new_fd);
            break;
        }
    }
    
    // 添加大文件描述符，测试动态扩容
    EXPECT_TRUE(poll_multiplexer_->addfd(large_fd, IOMultiplexer::EventType::READ));
    
    // 验证仍然能正常工作
    std::vector<std::pair<int, IOMultiplexer::EventType>> active_events;
    int result = poll_multiplexer_->wait(active_events, 10);
    EXPECT_GE(result, 0);
    
    // 清理
    poll_multiplexer_->removefd(large_fd);
    close(large_fd);
    
    for (int fd : test_fds) {
        poll_multiplexer_->removefd(fd);
        close(fd);
    }
}

// 测试无效事件类型
TEST_F(PollMultiplexerTest, InvalidEventType) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    // 尝试添加无效的事件类型
    IOMultiplexer::EventType invalid_event = static_cast<IOMultiplexer::EventType>(0xFF);
    EXPECT_FALSE(poll_multiplexer_->addfd(read_fd_, invalid_event));
    EXPECT_EQ(poll_multiplexer_->lastError(), PollMultiplexer::Error::INVALID_EVENT);
}

// 测试性能
TEST_F(PollMultiplexerTest, Performance) {
    ASSERT_TRUE(poll_multiplexer_->init());
    
    const int num_fds = 1000;
    std::vector<int> test_fds;
    
    // 创建测试文件描述符
    for (int i = 0; i < num_fds; ++i) {
        int fds[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0) {
            test_fds.push_back(fds[0]);
            close(fds[1]);
        }
    }
    
    double add_time = TestUtils::measureExecutionTime([&]() {
        for (int fd : test_fds) {
            poll_multiplexer_->addfd(fd, IOMultiplexer::EventType::READ);
        }
    });
    
    double remove_time = TestUtils::measureExecutionTime([&]() {
        for (int fd : test_fds) {
            poll_multiplexer_->removefd(fd);
        }
    });
    
    // 清理
    for (int fd : test_fds) {
        close(fd);
    }
    
    std::cout << "Poll: Added " << test_fds.size() << " fds in " << add_time << " ms" << std::endl;
    std::cout << "Poll: Removed " << test_fds.size() << " fds in " << remove_time << " ms" << std::endl;
    
    // poll性能应该比select好，但比epoll差
    EXPECT_LT(add_time, 1000.0);
    EXPECT_LT(remove_time, 1000.0);
}
