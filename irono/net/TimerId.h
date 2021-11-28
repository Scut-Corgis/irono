#pragma once

#include "../base/copyable.h"

namespace irono {

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public copyable {
public:
    explicit TimerId(Timer* timer)
    : value_(timer) {
  }

  // default copy-ctor, dtor and assignment are okay

private:
    Timer* value_;
};

}

