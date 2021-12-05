#include "TcpServer.h"
#include "../base/Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "Callbacks.h"
#include <functional>
#include "TcpConnection.h"
#include <assert.h>
#include <stdio.h>  // snprintf
#include "EventLoopThreadPool.h"

using namespace irono;
using namespace std;
TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(CHECK_NOTNULL(loop)),
        name_(listenAddr.toHostPort()),
        acceptor_(new Acceptor(loop, listenAddr)),
        threadPool_(new EventLoopThreadPool(loop)),
        started_(false),
        nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
    bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

//转发函数，线程安全
void TcpServer::start() {
    if (!started_) {
        started_ = true;
        threadPool_->start();
    }

    if (!acceptor_->listenning()) {
        loop_->runInLoop(
            bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_DEBUG << "TcpServer::newConnection [" << name_
            << "] - new connection [" << connName
            << "] from " << peerAddr.toHostPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    //选取EventLoop给新建立的TcpConnection
    EventLoop* ioLoop = threadPool_->getNextLoop();
    TcpConnectionPtr conn(
        new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    //应在放到其对应的I/O线程里面做
    ioLoop->runInLoop(bind(&TcpConnection::connectEstablished, conn));

    conn->setCloseCallback( bind(&TcpServer::removeConnection, this, _1));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {

  loop_->runInLoop(bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::removeConnectionInLoop [" << name_
                << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        bind(&TcpConnection::connectDestroyed, conn));
 }