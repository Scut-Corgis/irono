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
#include "../../base/Logging.h"
using namespace irono;
using namespace std;


inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

extern unsigned long g_total;


void bench(const char* type)
{
  Timestamp start(Timestamp::now());

  int n = 1000*1000;
  const bool kLongLog = false;
  string empty = " ";
  string longStr(2890, 'X');
  longStr += " ";
  for (int i = 0; i < n; ++i)
  {
    LOG_TRACE << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
             << (kLongLog ? longStr : empty)
             << i;
  }
  Timestamp end(Timestamp::now());
  double seconds = timeDifference(end, start);
  printf("%12s: %f seconds, %lu bytes, %10.2f msg/s, %.2f MiB/s\n",
         type, seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));
}

int main()
{
  getppid();

  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_TRACE << "Hello";
  LOG_TRACE << "World";
  LOG_ERROR << "Error";
  LOG_TRACE << sizeof(Logger);
  LOG_TRACE << sizeof(LogStream);
  LOG_TRACE << sizeof(LogStream::Buffer);

  sleep(1);
  bench("nop");
}

