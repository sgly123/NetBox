#include "IO/PollMultiplexer.h"
#include <algorithm>
#include <string>
#include <cstring>

PollMultiplexer::PollMultiplexer() 
    : m_fd_to_index(512, -1), last_error_(Error::NONE) {
}

bool PollMultiplexer::init() {
    m_fds.clear();
    std::fill(m_fd_to_index.begin(), m_fd_to_index.end(), -1);
    last_error_ = Error::NONE;
    return true;
}

IOMultiplexer::IOType PollMultiplexer::type() const {
    return IOType::POLL;
}

bool PollMultiplexer::addfd(int fd, EventType events) {
    // 检查无效fd
    if (fd < 0) {
        last_error_ = Error::INVALID_FD;
        return false;
    }

    // 检查无效事件（只允许基类定义的事件类型）
    if ((static_cast<int>(events) & ~(static_cast<int>(EventType::READ) | 
                                     static_cast<int>(EventType::WRITE) | 
                                     static_cast<int>(EventType::ERROR))) != 0) {
        last_error_ = Error::INVALID_EVENT;
        return false;
    }

    // 动态扩容映射表
    if (fd >= static_cast<int>(m_fd_to_index.size())) {
        size_t new_size = std::max(static_cast<size_t>(fd) + 1024, m_fd_to_index.size() * 2);
        m_fd_to_index.resize(new_size, -1);
    }

    // 检查fd是否已存在
    if (m_fd_to_index[fd] != -1) {
        last_error_ = Error::FD_ALREADY_EXIST;
        return false;
    }

    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = toPollEvents(events);
    pfd.revents = 0;

    m_fds.push_back(pfd);
    m_fd_to_index[fd] = static_cast<int>(m_fds.size()) - 1;

    last_error_ = Error::NONE;
    return true;
}

bool PollMultiplexer::removefd(int fd) {
    int index = findFdIndex(fd);
    if (index == -1) {
        last_error_ = Error::FD_NOT_FOUND;
        return false;
    }

    // 交换删除优化（避免数组元素移动）
    int last_index = static_cast<int>(m_fds.size()) - 1;
    if (index != last_index) {
        m_fds[index] = m_fds[last_index];
        m_fd_to_index[m_fds[index].fd] = index;
    }

    m_fds.pop_back();
    m_fd_to_index[fd] = -1;

    last_error_ = Error::NONE;
    return true;
}

bool PollMultiplexer::modifyFd(int fd, EventType events) {
    if (fd < 0) {
        last_error_ = Error::INVALID_FD;
        return false;
    }

    // 检查无效事件
    if ((static_cast<int>(events) & ~(static_cast<int>(EventType::READ) | 
                                     static_cast<int>(EventType::WRITE) | 
                                     static_cast<int>(EventType::ERROR))) != 0) {
        last_error_ = Error::INVALID_EVENT;
        return false;
    }

    // 检查fd是否在有效范围内
    if (fd >= static_cast<int>(m_fd_to_index.size())) {
        last_error_ = Error::FD_NOT_FOUND;
        return false;
    }

    int index = findFdIndex(fd);
    if (index == -1) {
        last_error_ = Error::FD_NOT_FOUND;
        return false;
    }

    // 更新事件
    m_fds[index].events = toPollEvents(events);
    m_fds[index].revents = 0;

    last_error_ = Error::NONE;
    return true;
}

int PollMultiplexer::wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) {
    int ret = poll(m_fds.data(), static_cast<nfds_t>(m_fds.size()), timeout);

    if (ret == -1) {
        last_error_ = Error::SYSTEM_ERROR;
        return -1;
    }

    if (ret == 0) {
        activeEvents.clear();
        last_error_ = Error::TIMEOUT;
        return 0;
    }

    // 收集活跃事件
    activeEvents.clear();
    activeEvents.reserve(ret);

    for (const auto& pfd : m_fds) {
        if (pfd.revents != 0) {
            activeEvents.emplace_back(pfd.fd, fromPollEvents(pfd.revents));
            if (--ret == 0) break;
        }
    }

    last_error_ = Error::NONE;
    return static_cast<int>(activeEvents.size());
}

short PollMultiplexer::toPollEvents(EventType events) {
    short poll_events = 0;
    if (events & EventType::READ) {
        poll_events |= POLLIN;
    }
    if (events & EventType::WRITE) {
        poll_events |= POLLOUT;
    }
    return poll_events;
}

IOMultiplexer::EventType PollMultiplexer::fromPollEvents(short events) {
    EventType evt = EventType::NONE;
    if (events & POLLIN) {
        evt |= EventType::READ;
    }
    if (events & POLLOUT) {
        evt |= EventType::WRITE;
    }
    if (events & (POLLERR | POLLHUP | POLLNVAL)) {
        evt |= EventType::ERROR;
    }
    return evt;
}

int PollMultiplexer::findFdIndex(int fd) const {
    if (fd < 0 || fd >= static_cast<int>(m_fd_to_index.size())) {
        return -1;
    }
    return m_fd_to_index[fd];
}

std::string PollMultiplexer::errorString() const {
    switch (last_error_) {
        case Error::NONE:
            return "No error";
        case Error::INVALID_FD:
            return "Invalid file descriptor";
        case Error::FD_ALREADY_EXIST:
            return "File descriptor already exists";
        case Error::FD_NOT_FOUND:
            return "File descriptor not found";
        case Error::SYSTEM_ERROR:
            return "System call failed: " + std::string(strerror(errno));
        case Error::TIMEOUT:
            return "Operation timed out";
        case Error::INVALID_EVENT:
            return "Invalid event type (only READ/WRITE/ERROR allowed)";
        default:
            return "Unknown error";
    }
}
