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
#include "LogFile.h"
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

    //日志前端工作区，通常有多个线程同时访问
    void append(const char* logline, int len);

    //开启后端日志线程
    void start() {
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop() {
        running_ = false;
        cond_.notify();
        thread_.join();
    }

    LogFile& output() {return output_;}

private:

    //日志后端线程工作函数，由唯一的logging线程控制
    void threadFunc();

    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    //刷新到磁盘的间隔时间，单位s
    const int flushInterval_;
    bool running_;
    //log日志文件名
    const std::string basename_;
    Thread thread_;
    CountDownLatch latch_;
    MutexLock mutex_;
    Condition cond_ ;

    //日志前端Buffer
    BufferPtr currentBuffer_ ;
    BufferPtr nextBuffer_ ;

    //总Buffer，日志后端线程将其写入磁盘
    BufferVector buffers_ ;

    //实际的底层文件操作者
    LogFile output_;
};

}