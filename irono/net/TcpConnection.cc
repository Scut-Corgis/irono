
// excerpts from http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "TcpConnection.h"

#include "../base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include <assert.h>
#include <functional>
#include "poll.h"
#include <errno.h>
#include <stdio.h>
#include "SocketsOps.h"
#include "../base/Logging.h"

using namespace irono;
using namespace std;

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(nameArg),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr)
{
    LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
                << " fd=" << sockfd;
    channel_->setReadCallback(
        bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(
        bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
                << " fd=" << channel_->fd();
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();

//最终析构关闭套接字时，这里会再次回调用户的函数。
        connectionCallback_(shared_from_this());
    }
    loop_->removeChannel(get_pointer(channel_));
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);

    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        LOG_DEBUG<<"Tcpconnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    // 所以只是简单了关闭了关注事件，让Poller不会再返回它
    channel_->disableAll();
    // must be the last line
    // 并且用shared_from_this确保正确析构
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_FATAL << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}



