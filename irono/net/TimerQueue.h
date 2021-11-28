#pragma once
#include <set>
#include <vector>

#include "../base/noncopyable.h"

#include "../base/Timestamp.h"
#include "../base/Mutex.h"
#include "Callbacks.h"
#include "Channel.h"

namespace irono {

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue : noncopyable {
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads.
    TimerId addTimer(const TimerCallback& cb,
                    Timestamp when,
                    double interval);

    // void cancel(TimerId timerId);

private:

    //用pair来区别相同的时间的定时器
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;

    // called when timerfd alarms
    void handleRead();
    void addTimerInLoop(Timer* timer);
    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;
};

}

