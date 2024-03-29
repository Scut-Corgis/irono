
#include "EventLoop.h"
#include "../base/Logging.h"
#include <assert.h>
#include <sys/epoll.h>
#include "TimerQueue.h"
#include <sys/eventfd.h>
#include "TimerId.h"
#include "sys/signal.h"
#include "EPoller.h"
using namespace irono;

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 50000;

static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL << "Failed in eventfd";
    }
    return evtfd;
}

//很重要，一定要忽略SIGPIPE信号，不然服务器有崩溃的风险
class IgnoreSigPipe {
public:
    IgnoreSigPipe() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
//用全局变量的方式帮助自己在程序开始前忽略信号
IgnoreSigPipe initObj;



EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(new EPoller(this)) ,
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()) ,
      wakeupChannel_(new Channel(this, wakeupFd_)) 
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    }
    else {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  // wakeupfd默认开启
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb) {
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb) {
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::cancel(TimerId timerId) {
    return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel* channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->removeChannel(channel);
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_FATAL << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_FATAL << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i) {
        functors[i]();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel* it : activeChannels_) {
            it->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop"<< this<< "stop looping";
    looping_ = false;
}

void EventLoop::abortNotInLoopThread() {
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::runInLoop(const Functor& cb) {
    if (isInLoopThread()) {
        cb();
    }
    else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb) {
    {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}
