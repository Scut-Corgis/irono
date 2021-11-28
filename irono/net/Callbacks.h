#pragma once
#include <functional>
#include <memory>

#include "../base/Timestamp.h"

namespace irono {

// All client visible callbacks go here.

typedef std::function<void()> TimerCallback;

}

