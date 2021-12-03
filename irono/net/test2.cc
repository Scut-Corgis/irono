#include "EventLoop.h"
#include <stdio.h>
#include "TimerId.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include <stdio.h>
#include "Socket.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include "TcpServer.h"

using namespace irono;
using namespace std;
#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <stdio.h>
#include <string>
#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <stdio.h>

#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpClient.h"

#include "../base/Logging.h"

#include <functional>

#include <utility>

#include <stdio.h>
#include <unistd.h>

std::string message = "Hello\n";

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

  printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
}

int main()
{
  EventLoop loop;
  InetAddress serverAddr("127.0.0.1", 9981);
  TcpClient client(&loop, serverAddr);

  client.setConnectionCallback(onConnection);
  client.setMessageCallback(onMessage);
  client.enableRetry();
  client.connect();
  loop.loop();
}




