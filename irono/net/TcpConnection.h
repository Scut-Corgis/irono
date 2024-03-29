#pragma once

#include "Callbacks.h"
#include "InetAddress.h"
#include "../base/noncopyable.h"
#include <memory>
#include "Buffer.h"
#include <boost/any.hpp>
namespace irono
{

class Channel;
class EventLoop;
class Socket;

/// Tcp连接对象， client 和 server 用该对象进行Tcp连接的管理
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    /// 使用sockfd创建Tcp连接对象
    /// 由框架创建，使用者不应该创建它.
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

    //void send(const void* message, size_t len);
    // 线程安全.
    void send(const std::string& message);
    // this one will swap data
    void send(Buffer* message);  
    // 线程安全.
    void shutdown();

    //是否关闭Nagle算法
    void setTcpNoDelay(bool on);

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    /// 仅供框架内部使用.
    void setCloseCallback(const CloseCallback& cb)
    {closeCallback_ = cb;}
    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    //当TcpServer从map移除自己时调用
    void connectDestroyed();

    void setContext(const boost::any& context)
    { context_ = context; }

    const boost::any& getContext() const
    { return context_; }
private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected, };

    void setState(StateE s) { state_ = s; }
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const std::string& message);
    void shutdownInLoop();
    EventLoop* loop_;
    std::string name_;
    StateE state_; 
    // we don't expose those classes to client.
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    //监听套接字地址信息（本地地址）
    InetAddress localAddr_;
    //连接套接字地址信息 (对方的地址)
    InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    //存放用户的数据，用any可存放任意数据
    boost::any context_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

}


