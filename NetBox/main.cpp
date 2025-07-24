#include "server/server.h"


int main() {
    EchoServer server("192.168.88.135", 8888, IOMultiplexer::IOType::EPOLL);
    if (server.start()) {
        server.run();
    }
    return 0;
}