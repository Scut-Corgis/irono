#include "codec.h"

#include "../../base/Logging.h"
#include "../../base/Mutex.h"
#include "../../others/ThreadLocalSingleton.h"
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
          codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
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
        server_.setThreadInitCallback(bind(&ChatServer::threadInit, this, _1));
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << conn->peerAddress().toHostPort() << " -> "
                << conn->localAddress().toHostPort() << " is "
                << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected()) {
            LocalConnections::instance().insert(conn);
        }
        else {
            LocalConnections::instance().erase(conn);
        }
    }

    void onStringMessage(const TcpConnectionPtr&,
                        const string& message,
                        Timestamp)
    {
        EventLoop::Functor f = std::bind(&ChatServer::distributeMessage, this, message);
        LOG_DEBUG;

        //实现了I/O线程级别的转发，唯一的临界区只有短短几行。
        MutexLockGuard lock(mutex_);
        for (std::set<EventLoop*>::iterator it = loops_.begin(); it != loops_.end(); ++it) {
            (*it)->queueInLoop(f);
        }
        LOG_DEBUG;
    }

    typedef std::set<TcpConnectionPtr> ConnectionList;

    void distributeMessage(const string& message) {
        LOG_DEBUG << "begin";
        for (ConnectionList::iterator it = LocalConnections::instance().begin(); it != LocalConnections::instance().end(); ++it) {
            codec_.send(*it, message);
        }
        LOG_DEBUG << "end";
    }

    ///线程初始化回调，每创建一个I/O线程会将其插入loops_进行统一管理
    void threadInit(EventLoop* loop) {
        assert(LocalConnections::pointer() == NULL);
        LocalConnections::instance();
        assert(LocalConnections::pointer() != NULL);
        MutexLockGuard lock(mutex_);
        loops_.insert(loop);
        printf("woaini");
        fflush(stdout);
    }

    TcpServer server_;
    LengthHeaderCodec codec_;
    typedef ThreadLocalSingleton<ConnectionList> LocalConnections;

    MutexLock mutex_;
    std::set<EventLoop*> loops_ ;
};

int main(int argc, char* argv[]) {
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


