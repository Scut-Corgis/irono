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

//线程安全，会放到I/O线程发
void TcpConnection::send(const std::string& message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        }
        else {
            loop_->runInLoop( bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // 如果output Buffer中没有存货，则尝试直接发出去，不然就往output Buffer中继续塞
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote >= 0) {
            if (static_cast<size_t>(nwrote) < message.size()) {
                LOG_TRACE << "I am going to write more data";
            }
            else if (writeCompleteCallback_) {
                loop_->queueInLoop( bind(writeCompleteCallback_, shared_from_this() ) );
            } 
        }
        else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_DEBUG << "TcpConnection::sendInLoop : maybe SIGPIPE";
            }
        }
    }

    assert(nwrote >= 0);
    if (static_cast<size_t>(nwrote) < message.size()) {
        outputBuffer_.append(message.data()+nwrote, message.size()-nwrote);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        // 此处用bind延长了该对象的生命期，所以不会有析构问题
        loop_->runInLoop(bind(&TcpConnection::shutdownInLoop, this));
    }
}

//如果输出缓冲区还有东西，就关不掉,不过因为已经设置了kDisconnecting状态，handleWrite发完后会关闭
void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        // we are not writing
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();

    //用户函数
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();

    //最终析构关闭套接字时，这里会再次回调用户的函数。
    connectionCallback_(shared_from_this());
    
    loop_->removeChannel(get_pointer(channel_));
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);

    if (n > 0) {
        LOG_TRACE<<"onMessage(): tid= "<<CurrentThread::tid()<<" received "<<inputBuffer_.readableBytes()<<" bytes";
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        LOG_DEBUG<<"Tcpconnection::handleRead";
        handleError();
    }
}

//output Buffer有剩余时会调用，确保将发送数据发至对岸
void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = ::write(channel_->fd(),
                            outputBuffer_.peek(),
                            outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop( bind(writeCompleteCallback_, shared_from_this() ) );
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
            else {
                LOG_TRACE << "I am going to write more data";
            }
        }
        else {
            LOG_DEBUG << "TcpConnection::handleWrite";
        }
    }
    else {
        //有可能对面提前关闭了连接，先处理了handleClose()
        LOG_TRACE << "Connection is down, no more writing";
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state_;

    //只可能是正在连接或者已经发送了shutdown
    assert(state_ == kConnected || state_ == kDisconnecting);
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
    LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}

