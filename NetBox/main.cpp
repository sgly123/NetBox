#include "./include/server.h"

int main() {
    EchoServer Server("192.168.88.135",8888);
    Server.start();
}