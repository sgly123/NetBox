#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "../../proto/include/protocol.h"

class Tcp_Server
{
private:
    int port_;
    int socket_fd;
    ssize_t send_data(int client_fd, char* buf, size_t buf_len);
    ssize_t recv_data(int client_fd, char* buf, size_t buf_len);
public:
    Tcp_Server(int port);
    // ~Tcp_Server();
    int get_socket();
    bool start();
    int sockets();
    int binds();
    int listens();
    int accept_client();
    bool send_message(int client_fd, const ProtocolMessage& msg);
    bool receive_message(int client_fd, ProtocolMessage& out_msg);
    void handle_client(int client_fd);
    void close_socket();
    void close_client(int client_fd);
};

#endif // TCP_SERVER_H