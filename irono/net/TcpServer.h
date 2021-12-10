#pragma once
#include "Callbacks.h"
#include "TcpConnection.h"

#include <map>
#include "../base/noncopyable.h"
#include <memory>
#include "InetAddress.h"

namespace irono
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    TcpServer(EventLoop* loop, const InetAddress& listenAddr);
    ~TcpServer();  // force out-line dtor, for unique_ptr members.

    /// Starts the server if it's not listenning.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void start();

    ///此回调每次创建I/O线程时会触发
    void setThreadInitCallback(const ThreadInitCallback& cb)
    { threadInitCallback_ = cb; }
    
    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    /// Set the number of threads for handling input.
    /// 
    /// Always accepts new connection in loop's thread.
    /// Must be called before @c start
    /// @param numThreads
    /// - 0 means all I/O in loop's thread, no thread will created.
    ///   this is the default value.
    /// - 1 means all I/O in another thread.
    /// - N means a thread pool with N threads, new connections
    ///   are assigned on a round-robin basis.
    void setThreadNum(int numThreads);

 private:
  /// Not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);
    /// Thread safe.
    void removeConnection(const TcpConnectionPtr& conn);
    /// Not thread safe, but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* loop_;  // the acceptor loop
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
    std::unique_ptr<EventLoopThreadPool> threadPool_;   //reactors I/O线程池
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool started_;
    ThreadInitCallback threadInitCallback_;
    int nextConnId_;  // always in loop thread
    ConnectionMap connections_;
};

}

