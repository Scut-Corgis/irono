#pragma once

#include "../base/copyable.h"

namespace irono {

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public copyable {
public:
    TimerId(Timer* timer = NULL, int64_t seq = 0)
     : timer_(timer),
       seq_(seq)
    {}

  // default copy-ctor, dtor and assignment are okay
    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t seq_;
};

}

