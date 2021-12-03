//十分粗略的实现
#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "../base/Logging.h"

#include <functional>

#include <stdio.h>  // snprintf

using namespace irono;
using namespace std;

namespace irono {
namespace other {

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
  loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{
  //connector->
}

}

}

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr)
    : loop_(CHECK_NOTNULL(loop)),
      connector_(new Connector(loop, serverAddr)),
      retry_(false),
      connect_(true),
      nextConnId_(1)
{
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, _1));
    LOG_DEBUG << "TcpClient::TcpClient[" << this
            << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient()
{
    LOG_DEBUG << "TcpClient::~TcpClient[" << this
            << "] - connector " << get_pointer(connector_);
    TcpConnectionPtr conn;
    {
        MutexLockGuard lock(mutex_);
        conn = connection_;
    }
    if (conn) {
        CloseCallback cb = bind(&other::removeConnection, loop_, _1);
        loop_->runInLoop(
            bind(&TcpConnection::setCloseCallback, conn, cb));
    }
    else {
        connector_->stop();
        loop_->runAfter(1, bind(&other::removeConnector, connector_));
    }
}

void TcpClient::connect() {
    LOG_DEBUG << "TcpClient::connect[" << this << "] - connecting to "
            << connector_->serverAddress().toHostPort();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;

    {
        MutexLockGuard lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
    ++nextConnId_;
    string connName = buf;

    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_,
                                            connName,
                                            sockfd,
                                            localAddr,
                                            peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        bind(&TcpClient::removeConnection, this, _1)); // FIXME: unsafe
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_) {
        LOG_DEBUG << "TcpClient::connect[" << this << "] - Reconnecting to "
                << connector_->serverAddress().toHostPort();
        connector_->restart();
    }
}

