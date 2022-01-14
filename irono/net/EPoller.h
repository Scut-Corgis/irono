#pragma once

#include <map>
#include <vector>
#include "../base/Timestamp.h"
#include "EventLoop.h"

struct epoll_event;

namespace irono
{

class Channel;

///
/// IO Multiplexing with epoll.
///
/// 该类不负责管理channel
class EPoller : noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

    EPoller(EventLoop* loop);
    ~EPoller();

    /// Polls the I/O events.
    /// Must be called in the loop thread.
    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    /// Changes the interested I/O events.
    /// Must be called in the loop thread.
    void updateChannel(Channel* channel);
    /// Remove the channel, when it destructs.
    /// Must be called in the loop thread.
    void removeChannel(Channel* channel);

    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents,
                            ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);

    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    int epollfd_;
    EventList events_;
    ChannelMap channels_;
};

}