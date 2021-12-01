/*
 * @Author: your name
 * @Date: 2021-11-29 15:53:58
 * @LastEditTime: 2021-12-01 15:29:34
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /irono/irono/net/Acceptor.cc
 */
#include "Acceptor.h"

#include "../base/Logging.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketsOps.h"

#include <functional>

using namespace irono;
using namespace std;
//最大并发连接数,可设置
const int kMaxConnections = 1000;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback( bind(&Acceptor::handleRead, this) );
}

//往Poller注册监听套接字，并开始监听
void Acceptor::listen() {
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptChannel_.enableReading();
    acceptSocket_.listen();
}

//新连接到达回调
void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = 0;
    while ( (connfd = acceptSocket_.accept(&peerAddr)) > 0) {
        // LOG_TRACE<<"new connection from : "<<peerAddr.toHostPort() <<" 现在的connfd:"<<connfd;
        if (connfd < kMaxConnections) {
            if (newConnectionCallback_) {
                newConnectionCallback_(connfd, peerAddr);
        }
        else {
            sockets::close(connfd);
        }
        }
    }

}

