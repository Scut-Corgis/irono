
#pragma once
#include <functional>
#include "../base/noncopyable.h"

#include "Channel.h"
#include "Socket.h"

namespace irono
{

class EventLoop;
class InetAddress;

///监听套接字类，理论上一个端口配一个，没有特别需求的话，一个主Reactor才会拥有一个Acceptor对象
class Acceptor : noncopyable {
public:
    typedef std::function<void (int sockfd,
                                const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr);

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};

}
