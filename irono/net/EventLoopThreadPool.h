#pragma once
#include "../base/Condition.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"


#include <vector>
#include <functional>
#include "../base/noncopyable.h"
#include <memory>

namespace irono
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg = std::string());
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;  // always in loop thread
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

}


