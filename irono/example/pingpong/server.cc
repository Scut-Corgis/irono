#include "../../net/TcpServer.h"

#include "../../base/Atomic.h"
#include "../../base/Logging.h"
#include "../../base/Thread.h"
#include "../../net/EventLoop.h"
#include "../../net/InetAddress.h"

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace irono;
using namespace std;

void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
    conn->send(buf->retrieveAsString());
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: server <address> <port> <threads>\n");
    }
    else {
        LOG_TRACE << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress listenAddr(ip, port);
        int threadCount = atoi(argv[3]);

        EventLoop loop;

        TcpServer server(&loop, listenAddr);

        server.setConnectionCallback(onConnection);
        server.setMessageCallback(onMessage);

        if (threadCount > 1) {
            server.setThreadNum(threadCount);
        }

        server.start();
        
        loop.loop();
    }
} 

 