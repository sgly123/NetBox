#ifndef POLLMULTIPLEXER_H
#define POLLMULTIPLEXER_H

#include "base/IOMultiplexer.h"
#include <vector>
#include <poll.h>
#include <string>

class PollMultiplexer : public IOMultiplexer {
public:
    // 自定义错误类型
    enum class Error {
        NONE,
        INVALID_FD,
        FD_ALREADY_EXIST,
        FD_NOT_FOUND,
        SYSTEM_ERROR,
        TIMEOUT,
        INVALID_EVENT
    };

    PollMultiplexer();
    ~PollMultiplexer() override = default;

    bool init() override;
    IOType type() const override;
    bool addfd(int fd, EventType events) override;
    bool removefd(int fd) override;
    bool modifyFd(int fd, EventType events) override;
    int wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) override;


    Error lastError() const { return last_error_; }
    
    std::string errorString() const;

private:

    static short toPollEvents(EventType events);
    static EventType fromPollEvents(short events);
    int findFdIndex(int fd) const;

    std::vector<pollfd> m_fds;                
    std::vector<int> m_fd_to_index;           
    Error last_error_;                        
};

#endif // POLLMULTIPLEXER_H
