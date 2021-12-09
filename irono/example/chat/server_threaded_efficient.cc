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
          codec_(bind(&ChatServer::onStringMessage, this, _1, _2, _3)),
          connections_(new ConnectionList)
    {
        server_.setConnectionCallback(
            bind(&ChatServer::onConnection, this, _1));
        server_.setMessageCallback(
            bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    }

    void setThreadNum(int numThreads) {
        server_.setThreadNum(numThreads);
    }

    void start() {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << conn->peerAddress().toHostPort() << " -> "
            << conn->localAddress().toHostPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

        MutexLockGuard lock(mutex_);
        //copy-on-write, 注意connections_是连接集合的shared_ptr
        if (!connections_.unique()) {
            connections_.reset(new ConnectionList(*connections_));
        }
        assert(connections_.unique());

        if (conn->connected()) {
            connections_->insert(conn);
        }
        else {
            connections_->erase(conn);
        }
    }

    typedef std::set<TcpConnectionPtr> ConnectionList;
    typedef std::shared_ptr<ConnectionList> ConnectionListPtr;

    void onStringMessage(const TcpConnectionPtr&, const string& message, Timestamp) {
        //创建shared_ptr，增加引用计数
        ConnectionListPtr connections = getConnectionList();
        for (ConnectionList::iterator it = connections->begin(); it != connections->end(); ++it) {
            codec_.send(*it, message);
        }
    }

    ConnectionListPtr getConnectionList() {
        MutexLockGuard lock(mutex_);
        return connections_;
    }

    TcpServer server_;
    LengthHeaderCodec codec_;
    MutexLock mutex_;
    ConnectionListPtr connections_;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 1) {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        if (argc > 2) {
            server.setThreadNum(atoi(argv[2]));
        }
        server.start();
        loop.loop();
    }
    else {
        printf("Usage: %s port [thread_num]\n", argv[0]);
    }
}

