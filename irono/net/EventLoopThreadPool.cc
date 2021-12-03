#include "EventLoopThreadPool.h"

#include "EventLoop.h"
#include "EventLoopThread.h"

#include <functional>

using namespace irono;
using namespace std;
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
  // 不要在这里delete EventLoop, 因为它是栈上定义的
}

void EventLoopThreadPool::start() {
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        EventLoopThread* t = new EventLoopThread;
        threads_.push_back(unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
}

//如果没开多级reactor，默认选为baseLoop_,不然用round-robin去依次选
EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}


