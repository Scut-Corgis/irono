#include "../../net/EventLoop.h"
#include <stdio.h>
#include "../../net/TimerId.h"
#include "../../net/EventLoopThread.h"
#include "../../net/EventLoopThreadPool.h"
#include "../../net/Socket.h"
#include "../../net/Acceptor.h"
#include "../../net/InetAddress.h"
#include "../../net/SocketsOps.h"
#include <string>
#include "../../net/TcpServer.h"
#include "../../net/EPoller.h"
#include "../../base/Logging.h"
using namespace irono;
using namespace std;

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): tid=%d new connection [%s] from %s\n",
           CurrentThread::tid(),
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
    LOG_INFO<<"`````````````````````````````````````````````````````";
    LOG_ERROR<<"````````````````````````````````````````````````````";
  }
  else
  {
    printf("onConnection(): tid=%d connection [%s] is down\n",
           CurrentThread::tid(),
           conn->name().c_str());
  }
}

void onMessage(const TcpConnectionPtr& conn,
               Buffer* buf,
               Timestamp receiveTime)
{
  printf("onMessage(): tid=%d received %zd bytes from connection [%s] at %s\n",
         CurrentThread::tid(),
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());

    string s = buf->retrieveAsString();
    conn->send(s);
    printf("onMessage(): [%s]\n", s.c_str());
}

int main(int argc, char* argv[])
{
  printf("main(): pid = %d\n", getpid());

  InetAddress listenAddr(9981);
  EventLoop loop;

  TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();

  loop.loop();
}


