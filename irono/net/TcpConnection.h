#pragma once

#include "Callbacks.h"
#include "InetAddress.h"

//#include <boost/any.hpp>
#include "../base/noncopyable.h"
#include <memory>

namespace irono
{

class Channel;
class EventLoop;
class Socket;

///
/// TCP connection, for both client and server usage.
///
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    /// Constructs a TcpConnection with a connected sockfd
    ///
    /// User should not create this object.
    TcpConnection(EventLoop* loop,
                    const std::string& name,
                    int sockfd,
                    const InetAddress& localAddr,
                    const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() { return localAddr_; }
    const InetAddress& peerAddress() { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    /// Internal use only.
    void setCloseCallback(const CloseCallback& cb)
    {closeCallback_ = cb;}
    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    //当TcpServer从map移除自己时调用
    void connectDestroyed();
private:
    enum StateE { kConnecting, kConnected, kDisconnected, };

    void setState(StateE s) { state_ = s; }
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    EventLoop* loop_;
    std::string name_;
    StateE state_;  // FIXME: use atomic variable
    // we don't expose those classes to client.
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    //监听套接字地址信息（本地地址）
    InetAddress localAddr_;
    //连接套接字地址信息 (对方的地址)
    InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}


