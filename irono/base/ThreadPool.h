#pragma once
#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include "noncopyable.h"
#include <string>
#include <deque>
#include <vector>
#include "Condition.h"
#include <memory>
namespace irono
{

class ThreadPool : noncopyable {
public:
    typedef std::function<void ()> Task;

    explicit ThreadPool(const std::string& nameArg = std::string("ThreadPool"));
    ~ThreadPool();

    // Must be called before start().
    void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }
    void setThreadInitCallback(const Task& cb)
    { threadInitCallback_ = cb; }

    void start(int numThreads);
    void stop();

    const std::string& name() const
    { return name_; }

    size_t queueSize() const;

    void run(Task f);

private:
    bool isFull() const ;
    void runInThread();
    Task take();

    mutable MutexLock mutex_;
    Condition notEmpty_ ;
    Condition notFull_ ;
    std::string name_;
    Task threadInitCallback_;
    //由这个ThreadPool对象管理其自己其中线程的生命周期
    std::vector<std::unique_ptr<Thread>> threads_;
    //任务队列
    std::deque<Task> queue_ ;
    size_t maxQueueSize_;
    bool running_;
};

}  // namespace irono


