#pragma once
#include <map>
#include <vector>

#include "../base/Timestamp.h"
#include "EventLoop.h"

struct pollfd;

namespace irono
{

class Channel;

///
/// IO Multiplexing with poll(2).
///
/// 该类不拥有(管理) channel对象的生命期
class Poller : noncopyable {
public:
    typedef std::vector<Channel*> ChannelList;

    Poller(EventLoop* loop);
    ~Poller();

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
    void fillActiveChannels(int numEvents,
                            ChannelList* activeChannels) const;

    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;
};

}

