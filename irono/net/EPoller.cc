#include "EPoller.h"

#include "Channel.h"
#include "../base/Logging.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

using namespace irono;

// 在Linux中，这些常量应该相等
static_assert(EPOLLIN == POLLIN);
static_assert(EPOLLPRI == POLLPRI);
static_assert(EPOLLOUT == POLLOUT);
static_assert(EPOLLRDHUP == POLLRDHUP);
static_assert(EPOLLERR == POLLERR);
static_assert(EPOLLHUP == POLLHUP);

namespace
{
//状态常量，说明一个channel是新的、已经增加的、还是增加后被删除的
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

EPoller::EPoller(EventLoop* loop)
    : ownerLoop_(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) 
{
  if (epollfd_ < 0) {
    LOG_FATAL << "EPoller::EPoller";
  }
}

EPoller::~EPoller() {
    ::close(epollfd_);
}

Timestamp EPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    int numEvents = ::epoll_wait(epollfd_,
                                &*events_.begin(),
                                static_cast<int>(events_.size()),
                                timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size()*2);
        }
    }
    else if (numEvents == 0) {
        LOG_TRACE << " nothing happended";
    }
    else {
        LOG_ERROR << "EPoller::poll()";
    }
    return now;
}

void EPoller::fillActiveChannels(int numEvents,
                                 ChannelList* activeChannels) const
{
    assert(static_cast<size_t>(numEvents) <= events_.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);

        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);

        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPoller::updateChannel(Channel* channel) {
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    const int index = channel->index();
    if (index == kNew || index == kDeleted) {
        // EPOLL_CTL_ADD 增加新Channel
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        // index == kDeleted
        else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        // 更新存在的channel 使用 EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::removeChannel(Channel* channel) {
    assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR << "epoll_ctl op=" << operation << " fd=" << fd;
        }
        else {
            LOG_FATAL << "epoll_ctl op=" << operation << " fd=" << fd;
        }
    }
}

