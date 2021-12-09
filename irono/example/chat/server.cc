#include "codec.h"
#include "../../base/Logging.h"
#include "../../base/Mutex.h"
#include "../../net/EventLoop.h"
#include "../../net/TcpServer.h"

#include <set>
#include <stdio.h>
#include <unistd.h>

using namespace irono;
using namespace std;

class ChatServer : noncopyable {
public:
    ChatServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr),
      codec_(bind(&ChatServer::onStringMessage, this, _1, _2, _3))
    {
        server_.setConnectionCallback(
            bind(&ChatServer::onConnection, this, _1));
        server_.setMessageCallback(
            bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    }

    void start() {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << conn->peerAddress().toHostPort() << " -> "
                << conn->localAddress().toHostPort() << " is "
                << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected()) {
            connections_.insert(conn);
        }
        else {
            connections_.erase(conn);
        }
    }

    void onStringMessage(const TcpConnectionPtr&,
                        const string& message,
                        Timestamp)
    {
        for (ConnectionList::iterator it = connections_.begin(); it != connections_.end(); ++it) {
            codec_.send(*it, message);
        }
    }

    typedef std::set<TcpConnectionPtr> ConnectionList;
    TcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;
};

int main(int argc, char* argv[]) {
    LOG_INFO << "pid = " << getpid();
    if (argc > 1) {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    }
    else {
        printf("Usage: %s port\n", argv[0]);
    }
}

