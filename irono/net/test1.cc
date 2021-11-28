#include "EventLoop.h"
#include "../base/Thread.h"
#include <stdio.h>
#include <unistd.h>
#include "../base/Logging.h"
using namespace irono;

EventLoop* g_loop;


int main()
{
  EventLoop loop1;
  EventLoop loop2;
  loop1.loop();
  loop2.loop();
    LOG_TRACE<<"sss";
    sleep(4);
}
