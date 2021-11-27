#pragma once
#include <memory>
#include <vector>
#include <string>
#include "Thread.h"
#include "CountDownLatch.h"
#include "Mutex.h"
#include "Condition.h"
#include "noncopyable.h"
#include "LogStream.h"

namespace irono {

class AsyncLogging : noncopyable {
public:

    AsyncLogging(const std::string& basename,
               int flushInterval = 3);
    ~AsyncLogging() {
        if (running_) {
            stop();
        }
    }

  void append(const char* logline, int len);

  void start()
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }

  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }

 private:

  void threadFunc();

  typedef FixedBuffer<kLargeBuffer> Buffer;
  typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
  typedef BufferVector::value_type BufferPtr;

  const int flushInterval_;
  bool running_;
  const std::string basename_;
  Thread thread_;
  CountDownLatch latch_;
  MutexLock mutex_;
  Condition cond_ ;
  BufferPtr currentBuffer_ ;
  BufferPtr nextBuffer_ ;
  BufferVector buffers_ ;
};

}