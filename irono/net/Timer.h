#pragma once
#include "../base/noncopyable.h"

#include "../base/Timestamp.h"
#include "Callbacks.h"
#include "../base/Atomic.h"
namespace irono
{

///实际的定时器类对象
class Timer : noncopyable {
public:
    Timer(const TimerCallback& cb, Timestamp when, double interval)
        : callback_(cb),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(s_numCreated_.incrementAndGet())
    {}

    void run() const{
        callback_();
    }

    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }
    void restart(Timestamp now);

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    //类的静态成员，所有对象拥有一个实例，用这个获取自己的sequence_.
    static AtomicInt64 s_numCreated_;
};

}
