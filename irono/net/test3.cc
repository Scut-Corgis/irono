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
#include "../base/LogFile.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpClient.h"
#include <sys/resource.h>
#include "../base/Logging.h"

#include <functional>

#include <utility>

#include <stdio.h>
#include <unistd.h>



inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

extern unsigned long g_total;
FILE* g_file;
std::unique_ptr<LogFile> g_logFile;


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
  getppid(); // for ltrace and strace

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

