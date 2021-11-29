#include "EventLoop.h"
#include <stdio.h>
#include "TimerId.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <stdio.h>
#include "Socket.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "SocketsOps.h"
using namespace irono;


void newConnection1(int sockfd, const InetAddress& peerAddr)
{
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(sockfd, "第一个！！！！", 13);
  sockets::close(sockfd);
}
void newConnection2(int sockfd, const InetAddress& peerAddr)
{
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(sockfd, "这是第二个啦", 100);
  sockets::close(sockfd);
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  InetAddress listenAddr1(9981);
  InetAddress listenAddr2(9982);
  EventLoop loop;

  Acceptor acceptor1(&loop, listenAddr1);
  Acceptor acceptor2(&loop, listenAddr2);
  acceptor1.setNewConnectionCallback(newConnection1);
  acceptor2.setNewConnectionCallback(newConnection2);
  acceptor1.listen();
  acceptor2.listen();

  loop.loop();
}

