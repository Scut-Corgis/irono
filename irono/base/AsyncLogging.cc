#include "AsyncLogging.h"
#include "LogFile.h"
#include <assert.h>

namespace irono {

AsyncLogging::AsyncLogging(const std::string& logFileName, int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(logFileName),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_(),
      latch_(1),
      output_(logFileName) 
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

//日志前端工作区，通常有多个线程同时访问
//该类的唯一对象成员的变量如cuurentBuffer_ & nextBuffer_ 提供了前端缓冲区
void AsyncLogging::append(const char* logline, int len) {
    MutexLockGuard lock(mutex_);
    if (currentBuffer_->avail() > len) {
        currentBuffer_->append(logline, len);
    }
    else {
        buffers_.push_back(std::move(currentBuffer_));
        currentBuffer_.reset();
        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline, len);
        cond_.notify();
    }
}


//日志后端线程工作函数，由唯一的logging线程控制
void AsyncLogging::threadFunc()
{
  assert(running_ == true);
  latch_.countDown();
  //LogFile output(basename_);
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);
  while (running_)
  {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      MutexLockGuard lock(mutex_);
      if (buffers_.empty())  // 不使用while是为了能够周期刷盘
      {
        cond_.waitForSeconds(flushInterval_);
      }
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      buffersToWrite.swap(buffers_);
      if (!nextBuffer_)
      {
        nextBuffer_ = std::move(newBuffer2);
      }
    }

    assert(!buffersToWrite.empty());

    if (buffersToWrite.size() > 25)
    {
    //   char buf[256];
    //   snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
    //            Timestamp::now().toFormattedString().c_str(),
    //            buffersToWrite.size()-2);
    //   fputs(buf, stderr);
    //   output.append(buf, static_cast<int>(strlen(buf)));
      buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
    }

    for (const auto& buffer : buffersToWrite)
    {
      output_.append(buffer->data(), buffer->length());
    }

    if (buffersToWrite.size() > 10)
    {
      //防止日志过多内存爆炸，保护措施
      buffersToWrite.resize(2);
    }

    if (!newBuffer1)
    {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2)
    {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output_.flush();
  }
  output_.flush();
}

}


