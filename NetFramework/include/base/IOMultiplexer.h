#ifndef IOMULTIPLEXER_H
#define IOMULTIPLEXER_H

#include <vector>
#include <cstddef>  // for size_t

class IOMultiplexer
{
private:

public:
    enum EventType {
        NONE = 0,
        READ = 1 << 0,
        WRITE = 1 << 1,
        ERROR = 1 << 2
    };

    enum IOType {
        SELECT,     // 跨平台select
        POLL,       // Linux/Unix poll
        EPOLL,      // Linux epoll
        KQUEUE,     // macOS/BSD kqueue
        IOCP        // Windows I/O Completion Port
    };

    // UDP相关常量
    enum SocketType {
        TCP_SOCKET,
        UDP_SOCKET
    };

    static const size_t MAX_UDP_PACKET_SIZE = 65507; // UDP最大负载大小

    friend EventType operator|(EventType a, EventType b) {
        return static_cast<EventType>(static_cast<int>(a) | static_cast<int>(b));
    }
    friend EventType& operator|=(EventType& a, EventType b) {
        a = a | b;
        return a;
    }
    virtual ~IOMultiplexer() = default;
    virtual bool init() = 0;
    virtual IOType type() const = 0;
    virtual bool addfd(int fd, EventType events) = 0;
    virtual bool removefd(int fd) = 0;
    virtual bool modifyFd(int fd, EventType events) = 0;
    virtual int wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) = 0;
};
#endif