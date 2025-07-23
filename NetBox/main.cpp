#include "server/server.h"
#include <signal.h>

EchoServer* g_server = nullptr;

int main() {
    EchoServer server("192.168.88.135", 8888, IOMultiplexer::IOType::SELECT);
    if (server.start()) {
        server.run();
    }
    return 0;
}