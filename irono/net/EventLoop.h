#pragma once
#include "../base/Thread.h"
#include "../base/noncopyable.h"
#include "../base/CurrentThread.h"
#include <vector>
#include <memory>
#include "Channel.h"
#include "TimerQueue.h"
#include <functional>
namespace irono {

class Poller;

class EventLoop : noncopyable {
public:

    EventLoop();
    ~EventLoop();

    void loop();

    void assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
    void quit();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    typedef std::vector<Channel*> ChannleList;
    typedef std::function<void()> Functor;
    void wakeup();

    void runInLoop(const Functor& );
    void queueInLoop(const Functor& );
    TimerId runAt(const Timestamp& time, const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval, const TimerCallback& cb);
private:

    void abortNotInLoopThread();
    void handleRead();
    void doPendingFunctors();
    //一个BUG改一小时，就算因为这个threadId_之前定义在了后面，一直abort，百思不得其解，吸取教训记住了。
    const pid_t threadId_;
    Timestamp pollReturnTime_;
    bool looping_;
    bool quit_;
    bool callingPendingFunctors_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    MutexLock mutex_;
    ChannleList activeChannels_;
    std::vector<Functor> pendingFunctors_;

};

}

