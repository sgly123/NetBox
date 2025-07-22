#ifndef IOMULTIPLEXER_H
#define IOMULTIPLEXER_H

#include <vector>




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

    enum IOType { SELECT, POLL, EPOLL };

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