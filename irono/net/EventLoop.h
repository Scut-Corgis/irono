#pragma once
#include "../base/Thread.h"
#include "../base/noncopyable.h"
#include "../base/CurrentThread.h"
#include <vector>
#include <memory>
#include "Channel.h"

namespace irono {

class Poller;

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
  void quit();
  void updateChannel(Channel* channel);

  typedef std::vector<Channel*> ChannleList;

 private:

  void abortNotInLoopThread();

  bool quit_;
  std::unique_ptr<Poller> poller_;
  bool looping_; /* atomic */
  const pid_t threadId_;
  ChannleList activeChannels_;
};

}

