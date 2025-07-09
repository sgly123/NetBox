#ifndef EPOLLMANGER_H
#define EPOLLMANGER_H
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h> // for fcntl
#include "../../Server/include/Tcp_Server.h"
const int MAX_EVENTS = 1024;

class EpollManger
{
private:
    int _epoll_fd;
    struct epoll_event ev;
    epoll_event events[MAX_EVENTS];
    int nfsd;
public:
    EpollManger(int socket_fd, Tcp_Server& server);
    // ~EpollManger();
    void set_nonblocking(int socket_fd);
    void create_epoll(int socket_fd);
    void create_epoll_ctl(int socket_fd);
    void create_epoll_wait(int socket_fd, Tcp_Server& server);

};





#endif