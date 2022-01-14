#pragma once
#include "../base/noncopyable.h"

namespace irono
{

class InetAddress;

/// 套接字文件描述符的封装，析构时关闭套接字
/// 线程安全，所有操作转交给了操作系统
class Socket : noncopyable {
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    { }

    ~Socket();

    int fd() const { return sockfd_; }

    /// abort if address in use
    void bindAddress(const InetAddress& localaddr);
    /// abort if address in use
    void listen();

    /// 成功则返回非负整数表示accepted socket，已经设为非阻塞，*peeraddr is assigned
    /// 错误时返回-1，*peeraddr is untouched
    int accept(InetAddress* peeraddr);

    ///
    /// Enable/disable SO_REUSEADDR
    ///
    void setReuseAddr(bool on);

    //Nagle算法开关
    void setTcpNoDelay(bool on);

    void shutdownWrite();
private:
    const int sockfd_;
};

}
