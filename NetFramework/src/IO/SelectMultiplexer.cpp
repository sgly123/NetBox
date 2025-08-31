#include "IO/SelectMultiplexer.h"
#include "base/Logger.h"
#include <sys/select.h>
#include <unistd.h>
#include <algorithm>

SelectMultiplexer::SelectMultiplexer() : m_maxFd(0) {}

SelectMultiplexer::~SelectMultiplexer() {}

bool SelectMultiplexer::init() {
    m_fdEvents.clear();
    m_maxFd = 0;
    return true;
}

SelectMultiplexer::IOType SelectMultiplexer::type() const {
    return IOType::SELECT;
}

bool SelectMultiplexer::addfd(int fd, EventType events) {
    if (fd < 0) {
        Logger::error("无效的文件描述符: " + std::to_string(fd));
        return false;
    }
    if (m_fdEvents.find(fd) != m_fdEvents.end()) {
        Logger::error("fd已存在: " + std::to_string(fd));
        return false;
    }
    m_fdEvents[fd] = events;
    if (fd > m_maxFd) m_maxFd = fd;
    return true;
}

bool SelectMultiplexer::removefd(int fd) {
    if (fd < 0) {
        Logger::error("无效的文件描述符: " + std::to_string(fd));
        return false;
    }
    if (m_fdEvents.find(fd) == m_fdEvents.end()) {
        Logger::error("fd不存在: " + std::to_string(fd));
        return false;
    }
    m_fdEvents.erase(fd);
    if (fd == m_maxFd) {
        m_maxFd = m_fdEvents.empty() ? 0 : std::max_element(m_fdEvents.begin(), m_fdEvents.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; })->first;
    }
    return true;
}

bool SelectMultiplexer::modifyFd(int fd, EventType events) {
    if (fd < 0) {
        Logger::error("无效的文件描述符: " + std::to_string(fd));
        return false;
    }
    if (m_fdEvents.find(fd) == m_fdEvents.end()) {
        Logger::error("fd不存在: " + std::to_string(fd));
        return false;
    }
    m_fdEvents[fd] = events;
    return true;
}

int SelectMultiplexer::wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout)
{
    activeEvents.clear();

    if (m_fdEvents.empty()) {
        return 0;
    }

    fd_set readFds, writeFds;
    FD_ZERO(&readFds);
    FD_ZERO(&writeFds);

    for (const auto& pair : m_fdEvents) {
        int fd = pair.first;
        EventType events = pair.second;
        (void)events; // 避免未使用变量警告

        if (events & EventType::READ) {
            FD_SET(fd, &readFds);
        }

        if (events & EventType::WRITE) {
            FD_SET(fd, &writeFds);
        }
    }

    struct timeval tv{};
    if (timeout >= 0) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    // 调用select等待事件就绪
    int ret = select(
        m_maxFd + 1,    
        &readFds,       
        &writeFds,      
        nullptr,        
        timeout >= 0 ? &tv : nullptr  
    );

    if (ret < 0) {
        perror("select失败");
        return -1;
    } else if (ret == 0) {
        return 0;
    }

    for (const auto& pair : m_fdEvents) {
        int fd = pair.first;
        EventType events = pair.second;
        (void)events; // 避免未使用变量警告
        EventType activeEvent = EventType::NONE;

        if (FD_ISSET(fd, &readFds)) {
            activeEvent |= EventType::READ;
        }

        if (FD_ISSET(fd, &writeFds)) {
            activeEvent |= EventType::WRITE;
        }

        if (activeEvent != EventType::NONE) {
            activeEvents.emplace_back(fd, activeEvent);
        }
    }

    return activeEvents.size();
}
