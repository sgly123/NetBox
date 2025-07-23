#include "IO/SelectMultiplexer.h"
#include <unistd.h>
#include <algorithm>
#include <iostream>

SelectMultiplexer::SelectMultiplexer() : m_maxFd(-1){}

SelectMultiplexer::~SelectMultiplexer() {
    m_fdEvents.clear();
}

bool SelectMultiplexer::init() {
    return true;
}

IOMultiplexer::IOType SelectMultiplexer::type() const {

    return IOType::SELECT;
}

bool SelectMultiplexer::addfd(int fd, EventType events) {
    if (fd < 0) {
        std::cerr << "无效的文件描述符: " << fd << std::endl;
        return false;
    }

    if (m_fdEvents.find(fd) != m_fdEvents.end()) {
        std::cerr << "fd已存在: " << fd << std::endl;
        return false;
    }

    m_fdEvents[fd] = events;

    if (fd > m_maxFd) {
        m_maxFd = fd;
    }

    return true;
}

bool SelectMultiplexer::removefd(int fd)
{
    if (fd < 0) {
        std::cerr << "无效的文件描述符: " << fd << std::endl;
        return false;
    }

    auto it = m_fdEvents.find(fd);
    if (it == m_fdEvents.end()) {
        std::cerr << "fd不存在: " << fd << std::endl;
        return false;
    }
    m_fdEvents.erase(it);

    if (fd == m_maxFd) {
        m_maxFd = -1;
        for (const auto& pair : m_fdEvents) {
            if (pair.first > m_maxFd) {
                m_maxFd = pair.first;
            }
        }
    }

    return true;
}

bool SelectMultiplexer::modifyFd(int fd, EventType events)
{
    if (fd < 0) {
        std::cerr << "无效的文件描述符: " << fd << std::endl;
        return false;
    }

    // 查找fd并更新事件类型
    auto it = m_fdEvents.find(fd);
    if (it == m_fdEvents.end()) {
        std::cerr << "fd不存在: " << fd << std::endl;
        return false;
    }
    it->second = events;
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
