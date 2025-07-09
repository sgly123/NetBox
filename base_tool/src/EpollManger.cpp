#include "../include/EpollManger.h"
#include "../include/thread_pool.h"
#include <memory> // 添加智能指针支持
#include <unordered_set>
void EpollManger::set_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

EpollManger::EpollManger(int socket_fd, Tcp_Server& server) {
    create_epoll(socket_fd);
    create_epoll_ctl(socket_fd);
    create_epoll_wait(socket_fd, server);
}

void EpollManger::create_epoll(int socket_fd) {
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd == -1) {
        perror("epoll_create1");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

void EpollManger::create_epoll_ctl(int socket_fd) {
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP; // 添加边缘触发和对端关闭检测
    ev.data.fd = socket_fd;
    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1) {
        perror("epoll_ctl");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
}

void EpollManger::create_epoll_wait(int socket_fd, Tcp_Server& server) {
    thread_pool thread_pool(4);
    
    // 用于跟踪活动fd的集合
    std::unordered_set<int> active_fds;
    std::mutex active_fds_mutex;
    
    while (true) {
        nfsd = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
        if (nfsd == -1) {
            perror("epoll_wait");
            if (errno == EINTR) continue; // 被信号中断，继续等待
            break;
        }

        for (int i = 0; i < nfsd; i++) {
            int fd = events[i].data.fd;
            uint32_t revents = events[i].events;

            if (fd == socket_fd) {
                if (revents & EPOLLIN) {
                    int client_fd = server.accept_client();
                    if (client_fd == -1) {
                        perror("accept");
                        continue;
                    }
                    set_nonblocking(client_fd);
                    
                    // 设置客户端事件
                    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                    ev.data.fd = client_fd;
                    
                    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                        perror("epoll_ctl for client");
                        close(client_fd);
                        continue;
                    }
                    
                    // 添加到活动fd集合
                    {
                        std::lock_guard<std::mutex> lock(active_fds_mutex);
                        active_fds.insert(client_fd);
                        
                    }
                }
                if (revents & (EPOLLERR | EPOLLHUP)) {
                    fprintf(stderr, "Error on listen socket!\n");
                    close(socket_fd);
                    close(_epoll_fd);
                    exit(EXIT_FAILURE);
                }
            } else {
                if (revents & EPOLLIN) {
                    bool is_active = false;
                    // 检查fd是否仍然有效
                    {
                        std::lock_guard<std::mutex> lock(active_fds_mutex);
                        is_active = (active_fds.find(fd) != active_fds.end());
                    }
                    if (!is_active) {
                        // fd已被关闭，跳过处理
                        continue;
                    }
                    
                    thread_pool.queue([fd, &server] {
                        server.handle_client(fd);
                    });
                } 
                if (revents & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                    // 从epoll中移除
                    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
                        perror("epoll_ctl DEL");
                    }
                    
                    // 关闭fd并从活动集合中移除
                    {
                        std::lock_guard<std::mutex> lock(active_fds_mutex);
                        active_fds.erase(fd);
                    }
                    close(fd);
                    
                    if (revents & EPOLLRDHUP) {
                        printf("Client (fd=%d) closed connection (EPOLLRDHUP)\n", fd);
                    } else {
                        fprintf(stderr, "Error or Hangup on fd=%d (events: 0x%x)\n", fd, revents);
                    }
                }
            }
        }
    }
    
    // 清理所有剩余的文件描述符
    for (int fd : active_fds) {
        close(fd);
    }
    active_fds.clear();
    
    close(socket_fd);
    close(_epoll_fd);
}