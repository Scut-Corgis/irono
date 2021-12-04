#include "../../net/TcpClient.h"

#include "../../base/Logging.h"
#include "../../base/Thread.h"
#include "../../base/Atomic.h"
#include "../../net/EventLoop.h"
#include "../../net/EventLoopThreadPool.h"
#include "../../net/InetAddress.h"
#include "../../net/TimerId.h"
#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace irono;
using namespace std;
class Client;

//每一个具体的Tcp连接拥有者与管理者
class Session : noncopyable {
public:
    Session(EventLoop* loop,
            const InetAddress& serverAddr,
            const string& name,
            Client* owner)
        : client_(loop, serverAddr),
          owner_(owner),
          bytesRead_(0),
          bytesWritten_(0),
          messagesRead_(0)
    {
        client_.setConnectionCallback(
            std::bind(&Session::onConnection, this, _1));
        client_.setMessageCallback(
            std::bind(&Session::onMessage, this, _1, _2, _3));
    }

    void start() {
        client_.connect();
    }

    void stop() {
        client_.disconnect();
    }

    int64_t bytesRead() const {
        return bytesRead_;
    }

    int64_t messagesRead() const {
        return messagesRead_;
    }

private:

    void onConnection(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
        ++messagesRead_;
        bytesRead_ += buf->readableBytes();
        bytesWritten_ += buf->readableBytes();
        conn->send(buf->retrieveAsString());
    }

    TcpClient client_;
    Client* owner_;
    int64_t bytesRead_;
    int64_t bytesWritten_;
    int64_t messagesRead_;
};

//整个客户端，为所有并发连接的管理者，但不拥有具体的TcpConneciton对象
class Client : noncopyable
{
public:
    Client(EventLoop* loop,
            const InetAddress& serverAddr,
            int blockSize,
            int sessionCount,
            int timeout,
            int threadCount)
        : loop_(loop),
          threadPool_(loop),
          sessionCount_(sessionCount),
          timeout_(timeout)
    {
        loop->runAfter(timeout, std::bind(&Client::handleTimeout, this));
        if (threadCount > 1) {
            threadPool_.setThreadNum(threadCount);
        }
            threadPool_.start();

        //初始化块信息
        for (int i = 0; i < blockSize; ++i) {
            message_.push_back(static_cast<char>(i % 128));
        }

        //创建并发连接并开始
        for (int i = 0; i < sessionCount; ++i) {
            char buf[32];
            snprintf(buf, sizeof buf, "C%05d", i);
            Session* session = new Session(threadPool_.getNextLoop(), serverAddr, buf, this);
            session->start();
            sessions_.emplace_back(session);
        }
    }

    const string& message() const {
        return message_;
    }

    void onConnect() {
        if (numConnected_.incrementAndGet() == sessionCount_) {
            LOG_DEBUG << "all connected";
        }
    }

  void onDisconnect(const TcpConnectionPtr& conn) {
    if (numConnected_.decrementAndGet() == 0) {
        LOG_DEBUG << "all disconnected";

        int64_t totalBytesRead = 0;
        int64_t totalMessagesRead = 0;
        for (const auto& session : sessions_) {
            totalBytesRead += session->bytesRead();
            totalMessagesRead += session->messagesRead();
        }
        LOG_DEBUG << totalBytesRead << " total bytes read";
        LOG_DEBUG << totalMessagesRead << " total messages read";
        LOG_DEBUG << static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead)
                << " average message size";
        LOG_DEBUG << static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024)
                << " MiB/s throughput";
        //注意这里转发给其从reactor，然后再转发给主reactor，因为它没有主reactor的指针
        conn->getLoop()->queueInLoop(std::bind(&Client::quit, this));
    }
  }

private:

    void quit() {
        loop_->queueInLoop(std::bind(&EventLoop::quit, loop_));
    }

    void handleTimeout() {
        LOG_DEBUG << "stop";
        for (auto& session : sessions_) {
            session->stop();
        }
    }

    EventLoop* loop_;
    EventLoopThreadPool threadPool_;
    int sessionCount_;
    int timeout_;
    std::vector<std::unique_ptr<Session>> sessions_;
    string message_;
    AtomicInt32 numConnected_;
};

void Session::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
        conn->send(owner_->message());
        //会原子的增加已成功连接的个数计数
        owner_->onConnect();
    }
    else {
        //原子的减少已成功连接的个数计数，减少至0时触发最后的统计
        owner_->onDisconnect(conn);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
        fprintf(stderr, "<sessions> <time>\n"); 
    }
    else {
        LOG_DEBUG << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
        //对方服务器的ip和端口
        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        //线程数，注意包括主Reactor
        int threadCount = atoi(argv[3]);
        //一次传输的块大小，建议设为4096，当然如果16384为最大块
        int blockSize = atoi(argv[4]);
        //并发连接数
        int sessionCount = atoi(argv[5]);
        //持续时间
        int timeout = atoi(argv[6]);

        EventLoop loop;
        InetAddress serverAddr(ip, port);

        Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount);
        loop.loop();
        sleep(5);
    }
}

