#pragma once

#include <sys/select.h>
#include <sys/time.h>
#include <map>
#include "base/IOMultiplexer.h"

class SelectMultiplexer : public IOMultiplexer
{
private:
    int m_maxFd;
    std::map<int, EventType> m_fdEvents;
    

public:
    SelectMultiplexer();
    
    ~SelectMultiplexer() override;
    
    bool init() override;


    IOType type() const override;


    bool addfd(int fd, EventType events) override;


    bool removefd(int fd) override;


    bool modifyFd(int fd, EventType events) override;


    int wait(std::vector<std::pair<int, EventType>>& activeEvents, int timeout) override;
};
