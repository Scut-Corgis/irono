#include "TimerQueue.h"

#include "../base/Logging.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"

#include <functional>
#include <assert.h>
#include <sys/timerfd.h>
#include "TimerId.h"
namespace irono
{

int createTimerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    LOG_FATAL << "Failed in timerfd_create";
  }
  return timerfd;
}
//返回when距现在的时间，用timespec结构返回
struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
  uint64_t howmany;
  //n的值表示超时次数
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  if (n != sizeof howmany)
  {
    LOG_FATAL << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret)
  {
    LOG_FATAL << "timerfd_settime()";
  }
}

}
using namespace irono;
using namespace std;
TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_(),
    callingExpiredTimers_(false)
{
  timerfdChannel_.setReadCallback(
      bind(&TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  //这行代码意味着从TimerQueue创立之时就已经纳入Poller列表了，是否定时取决你是否设置了定时器
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  for (TimerList::iterator it = timers_.begin();
      it != timers_.end(); ++it)
  {
    delete it->second;
  }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
                             Timestamp when,
                             double interval) {
    Timer* timer = new Timer(cb, when, interval);
    loop_->runInLoop( bind(&TimerQueue::addTimerInLoop, this, timer));
    //必须在自己的I/O线程加，防止竞态条件

    //判断是否最早定时事件改变，timefd是时刻定最早的时，每次返回时再判断所有时间是否超时

    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer* timer) {
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}
void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId));
}
void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, timerId.seq_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end()) {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1); (void)n;
        delete it->first; 
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_) {
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}


void TimerQueue::handleRead() {
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
        it != expired.end(); ++it)
    {
        it->second->run();
    }
    callingExpiredTimers_ = false;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    //返回第一个未到期的定时器，最大指针的手法也很巧妙
    TimerList::iterator it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);

    //将到期的定时器返回，并删除到期的定时器，也很巧妙
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    //删除activeTimers_的对应定时器
    for(Entry& entry : expired) {
        ActiveTimer timer(entry.second, entry.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1); (void)n;
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

//这个函数处理了所有到期定时事件，并重新设置定时为当前最前的定时器
void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;

    for (std::vector<Entry>::const_iterator it = expired.begin();
        it != expired.end(); ++it)
    {
        ActiveTimer timer(it->second, it->second->sequence());
        //如果定时器是不断重复的周期事件并且没有在正在删除的定时器列表中找到他，则重新插入,不然就删除他
        if (it->second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it->second->restart(now);
            insert(it->second);
        }
        else
        {
            //delete时机在此处
            delete it->second;
        }
    }
        //获取下一个定时
    if (!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
    }

bool TimerQueue::insert(Timer* timer) {
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());

    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    //set让排第一个就是最小的
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    {
        std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
        assert(result.second); (void)result;
    }
    
    {
        //set的插入函数，可以返回一个插入位置的迭代器和是否插入成功的boo
        std::pair<ActiveTimerSet::iterator, bool> result = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second); (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

