/*
 * EpollMultiplexer.cpp - epoll IO多路复用实现
 * 基于Linux epoll机制，支持高效的事件驱动IO
 * 采用边缘触发(EPOLLET)模式，仅在状态变化时通知
 */
#include "../../include/IO/EpollMultiplexer.h"

/*
 * 构造函数: 初始化epoll文件描述符和事件数组
 */
EpollMultiplexer::EpollMultiplexer():m_epoll_fd(-1),m_events(1024){};


/*
 * 初始化epoll实例
 * 返回值: true-成功, false-失败
 */
bool EpollMultiplexer::init() {
    m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    return m_epoll_fd != -1;
}


/*
 * 向epoll注册文件描述符和事件
 * 参数: fd-文件描述符, events-关注的事件类型
 * 返回值: true-注册成功, false-失败
 */
bool EpollMultiplexer::addfd(int fd, EventType events) {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = 0;
    if (events & READ) ev.events |= EPOLLIN;
    if (events & WRITE) ev.events |= EPOLLOUT;
    if (events & ERROR) ev.events |= EPOLLERR;
    ev.events = EPOLLIN | EPOLLET;
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

/*
 * 等待epoll事件触发
 * 参数: activeEvents-输出活跃事件列表, timeout-超时时间(ms)
 * 返回值: 触发的事件数量, -1-错误
 */
int EpollMultiplexer::wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) {
    int n = epoll_wait(m_epoll_fd,m_events.data(),static_cast<int>(m_events.size()),timeout);
    if (n <= 0) return n;
    activeEvents.clear();
    for (int i = 0; i < n; i++)
    {
        int fd = m_events[i].data.fd;
        EventType et = NONE;
        if (m_events[i].events & EPOLLIN) et |= READ;
        if (m_events[i].events & EPOLLOUT) et |= WRITE;
        if (m_events[i].events & EPOLLERR) et |= ERROR;
        activeEvents.emplace_back(fd, et);
    }
    return n;
}
IOMultiplexer::IOType EpollMultiplexer::type() const {
        return IOType::EPOLL;
    }