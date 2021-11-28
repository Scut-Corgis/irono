#pragma once
#include "../base/Thread.h"
#include "../base/noncopyable.h"
#include "../base/CurrentThread.h"
namespace irono {

class EventLoop : noncopyable {
 public:

  EventLoop();
  ~EventLoop();

  void loop();

  void assertInLoopThread()
  {
    if (!isInLoopThread())
    {
      abortNotInLoopThread();
    }
  }

  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

 private:

  void abortNotInLoopThread();

  bool looping_; /* atomic */
  const pid_t threadId_;
};

}

