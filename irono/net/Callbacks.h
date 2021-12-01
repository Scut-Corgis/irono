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
// 外界用户可见的回调

typedef std::function<void()> TimerCallback;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&, Buffer* buf, Timestamp)> MessageCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
}

