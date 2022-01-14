#pragma once
#include "noncopyable.h"
#include "Condition.h"
#include "Mutex.h"
namespace irono
{
// 计数门阀，类似于go的WaitGroup，一种同步机制
class CountDownLatch : noncopyable {
public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
  mutable MutexLock mutex_;
  Condition condition_ ;
  int count_ ;
};

}  // namespace irono

