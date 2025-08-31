#pragma once

#include "base/IOMultiplexer.h"
#include <sys/epoll.h>

class EpollMultiplexer: public IOMultiplexer
{
private:
    int m_epoll_fd;
    std::vector<epoll_event> m_events;
public:
    EpollMultiplexer();
    bool init() override;
    bool addfd(int fd, EventType events) override;
    bool removefd(int fd) override;
    bool modifyFd(int fd, EventType events)override;
    int wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) override;
    IOType type() const override;
};