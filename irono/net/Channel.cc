#include "Channel.h"
#include "EventLoop.h"
#include "../base/Logging.h"
#include <assert.h>
#include <sstream>

#include <poll.h>

using namespace irono;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fdArg)
    : loop_(loop),
      fd_(fdArg),
      events_(0),
      revents_(0),
      index_(-1),
      eventHandling_(false)
{
}
Channel::~Channel() {
  assert(!eventHandling_);
}

void Channel::update() {
    loop_->updateChannel(this);
}

// void Channel::handleEvent() {
//     eventHandling_ = true;
//     if (revents_ & POLLNVAL) {
//         LOG_DEBUG << "Channel::handle_event() POLLNVAL";
//     }
//     //理论上一定要放到POLLIN前面,不过我后面逻辑修改了一点，所以顺序无所谓了。
//     if (revents_ & (POLLRDHUP)) {
//         LOG_DEBUG << "Channel::handle_event() POLLRDHUP";
//         if (closeCallback_) {
//             closeCallback_();
//         }
//         //防止日志爆炸，强行退出程序
//         else abort();
//     }
//     if (revents_ & (POLLERR | POLLNVAL)) {
//         if (errorCallback_) errorCallback_();
//     }
//     //POLLRDHUP目前测试和POLLHUP差不多,对方关闭触发


//     if ( (revents_ & (POLLIN | POLLPRI)) && !(revents_ & (POLLRDHUP)) ) {
//         if (readCallback_) readCallback_();
//     }
//     if (revents_ & POLLOUT) {
//         if (writeCallback_) writeCallback_();
//     }
//     eventHandling_ = false;
// }


//其实目前只打开了POLLIN，POLLOUT，POLLPRI事件，所以POLLHUP是由read 0 触发的
void Channel::handleEvent(Timestamp receiveTime)
{
    eventHandling_ = true;
    if (revents_ & POLLNVAL) {
        LOG_DEBUG << "Channel::handle_event() POLLNVAL";
    }

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        LOG_DEBUG << "Channel::handle_event() POLLHUP";
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) readCallback_(receiveTime);
    }
    if (revents_ & POLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}