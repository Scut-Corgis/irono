#pragma once
#include <functional>
#include <memory>
#include "Buffer.h"
#include "../base/Timestamp.h"



namespace irono {
    
class TcpConnection;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;



template<typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr)
{
  return ptr.get();
}

template<typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr)
{
  return ptr.get();
}

// 以下都属于外界用户可见的回调



typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

//定时器回调
typedef std::function<void()> TimerCallback;
//新连接到达时的回调
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
//受到客户端信息的回调
typedef std::function<void (const TcpConnectionPtr&, Buffer* buf, Timestamp)> MessageCallback;
//关闭套接字时的回调
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
//缓冲区数据写完了触发的回调
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;

}

