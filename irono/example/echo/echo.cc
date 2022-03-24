#include "echo.h"

#include "../../base/Logging.h"
#include "../../net/EventLoop.h"

#include <assert.h>
#include <stdio.h>

using namespace irono;
using namespace std;

EchoServer::EchoServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       int idleSeconds)
    : server_(loop, listenAddr),
        connectionBuckets_(idleSeconds)
{
    server_.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    loop->runEvery(3.0, std::bind(&EchoServer::onTimer, this));
    dumpConnectionBuckets();
}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << "EchoServer - " << conn->peerAddress().toHostPort() << " -> "
            << conn->localAddress().toHostPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected()) {
        EntryPtr entry(new Entry(conn));
        (*connectionBuckets_.begin()).insert(entry);
        dumpConnectionBuckets();
        WeakEntryPtr weakEntry(entry);
        conn->setContext(weakEntry);
    }
    else {
        assert(!conn->getContext().empty());
        WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
        LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
    }
}

void EchoServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp time)
{
    string msg(buf->retrieveAsString());
    LOG_INFO << conn->name() << " echo " << msg.size()
            << " bytes at " << time.toString();
    conn->send(msg);

    assert(!conn->getContext().empty());
    WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
    EntryPtr entry(weakEntry.lock());
    if (entry) {
        (*connectionBuckets_.begin()).insert(entry);
        dumpConnectionBuckets();
    }
}

// 时间轮指针前移
void EchoServer::onTimer() {
    connectionBuckets_.Tick();
    dumpConnectionBuckets();
}

// 打印当前时间轮所有桶情况
void EchoServer::dumpConnectionBuckets()  {
    LOG_INFO << "size = " << connectionBuckets_.size();
    int idx = 0;
    for (auto bucketI = connectionBuckets_.begin();
        bucketI != connectionBuckets_.end();
        ++bucketI, ++idx)
    {
        const Bucket& bucket = *bucketI;
        printf("[%d] len = %zd : ", idx, bucket.size());
        for (const auto& it : bucket) {
            bool connectionDead = it->weakConn_.expired();
            printf("%p(%ld)%s, ", get_pointer(it), it.use_count(),
                connectionDead ? " DEAD" : "");
        }
        puts("");
    }
    printf("-----------------------\n");
}

