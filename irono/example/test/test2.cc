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
#include "../../net/TcpClient.h"
using namespace irono;
using namespace std;

std::string message = "Hello";

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
    conn->send(message);
  }
  else
  {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onMessage(const TcpConnectionPtr& conn,
               Buffer* buf,
               Timestamp receiveTime)
{
  printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());
    conn->send(buf->retrieveAsString());
}

int main()
{
  EventLoop loop;
  InetAddress serverAddr("localhost", 9981);
  TcpClient client(&loop, serverAddr);

  client.setConnectionCallback(onConnection);
  client.setMessageCallback(onMessage);
  client.enableRetry();
  client.connect();
  loop.loop();
}




