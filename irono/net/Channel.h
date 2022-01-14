
#pragma once
#include <functional>
#include "../base/noncopyable.h"
#include "../base/Timestamp.h"
namespace irono
{

class EventLoop;

/// 连接I/O复用器(Poll等) 和 上层应用(Timer等) 的中间层，上层应用通过它注册回调函数，channel在I/O事件发生时触发对应回调
/// 该类不拥有文件描述符(即不由它管理生命期)， 它的fd可以是套接字，事件fd，timerfd或者信号fd
class Channel : noncopyable {
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(Timestamp)> ReadEventCallback;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(const ReadEventCallback& cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }
    void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }
    int fd() const { return fd_; }
    int& events() { return events_; }
    void set_revents(int revt) { revents_ = revt; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    
    bool isWriting() const { return events_ & kWriteEvent; }
  // for Poller
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop* ownerLoop() { return loop_; }

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int  fd_;
    int        events_;
    int        revents_;
    int        index_; // used by Poller.
    bool eventHandling_;
    
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
};

}

