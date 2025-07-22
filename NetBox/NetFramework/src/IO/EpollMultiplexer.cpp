#include "../../include/IO/EpollMultiplexer.h"

EpollMultiplexer::EpollMultiplexer():m_epoll_fd(-1),m_events(1024){};


bool EpollMultiplexer::init() {
    m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    return m_epoll_fd != -1;
}


bool EpollMultiplexer::addfd(int fd, EventType events) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = 0;
    if (events & READ) ev.events |= EPOLLIN;
    if (events & WRITE) ev.events |= EPOLLOUT;
    if (events & ERROR) ev.events |= EPOLLERR;
    ev.events |= EPOLLET;
    return epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,fd, &ev) == 0;
}

bool EpollMultiplexer::removefd(int fd) {
    return epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,fd, nullptr) == 0;
}

bool EpollMultiplexer::modifyFd(int fd, EventType events) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = NONE;
    if (events & READ) ev.events |= EPOLLIN;
    if (events & WRITE) ev.events |= EPOLLOUT;
    if (events & ERROR) ev.events |= EPOLLERR;
    ev.events |= EPOLLET;
    return epoll_ctl(m_epoll_fd,EPOLL_CTL_MOD,fd, &ev) == 0;
}

int EpollMultiplexer::wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) {
    int n = epoll_wait(m_epoll_fd,m_events.data(),m_events.size(),timeout);
    if (n <= 0) return n;
    activeEvents.clear();
    for (size_t i = 0; i < n; i++)
    {
        int fd = m_events[i].data.fd;
        EventType et = NONE;
        if (m_events[i].events & EPOLLIN) et |= READ;
        if (m_events[i].events & EPOLLOUT) et |= WRITE;
        if (m_events[i].events & EPOLLERR) et |= ERROR;
        activeEvents.emplace_back(fd, et);
    }
    if (n == activeEvents.size())
    {
        activeEvents.resize(m_events.size()*2);
    }
    return n;
}
IOMultiplexer::IOType EpollMultiplexer::type() const {
        return IOType::EPOLL;
    }